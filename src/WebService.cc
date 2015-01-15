#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <microhttpd.h>
#include <cassert>

#include <iostream>
#include <vector>
#include <string>
#include <map>

#include <boost/filesystem.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "EntityTypeHelper.h"
#include "OfficialData.h"
#include "Planners.h"
#include "Supply.h"
#include "TrackedResources.h"
#include "Cost.h"
#include "Plan.h"
#include "Utils.h"
#include "CommandLineOptions.h"
#include "Log.h"
#include "WebServerLog.h"
#include "WebClientRequestArgs.h"

using namespace std;
using namespace boost::filesystem;
using namespace boost::posix_time;

#define PORT            8888
#define POSTBUFFERSIZE  512
#define MAXCLIENTS      2
#define MAXPOSTSIZE     8192

#define GET             0
#define POST            1

// arrrg.  I don't know the right way to handle the fact that the microhttpd is c and the rest
// of my code is c++.  I was trying to stay c-ish in this file (even that felt wrong) but as I
// got to handling the post data, I realize that I really need a map of key-values.  Now I feel
// even more dirty.


static unsigned int nr_of_clients = 0;

struct connectionInfo
{
    int ConnectionType;
    WebClientRequestArgs *RequestArgs;
    unsigned PostDataSize;
    int PostHandlingError;
    int CallbackCount;
    string URI;
    string Version;
    string PlanItem;
    int PlanRank;
    int RequestSize;
    int ResponseSize;
    ptime TransactionStartTime;
    struct MHD_PostProcessor *PostProcessor;
};

const char *NotFound =  "<html><head><title>File not found</title></head><body>File not found</body></html>";
const char *ErrorPage = "<html><body>This doesn't seem to be right.</body></html>";
const char *BusyPage = "<html><body>This server is busy, please try again later.</body></html>";
const char *CompletePage = "<html><body>The upload has been completed.</body></html>";
const char *ServererrorPage = "<html><body>An internal server error has occured.</body></html>";

void MicroHttpdTest();

static ssize_t
file_reader (void *cls, uint64_t pos, char *buf, size_t max)
{
    FILE *file = static_cast<FILE*>(cls);

    (void)  fseek (file, pos, SEEK_SET);
    return fread (buf, 1, max, file);
}

static void
free_callback (void *cls)
{
    FILE *file = static_cast<FILE*>(cls);
    fclose (file);
}

int PrintOutKey(void *cls, enum MHD_ValueKind kind, const char *key, const char *value) {
    printf("\t%s: %s\n", key, value);
    return MHD_YES;
}

// thanks http://www.beej.us/guide/bgnet/output/html/multipage/inet_ntopman.html
char *get_ip_str(const struct sockaddr *sa, char *s, size_t maxlen)
{
    switch(sa->sa_family) {
    case AF_INET:
	inet_ntop(AF_INET, &(((struct sockaddr_in *)sa)->sin_addr),
		  s, maxlen);
	break;

    case AF_INET6:
	inet_ntop(AF_INET6, &(((struct sockaddr_in6 *)sa)->sin6_addr),
		  s, maxlen);
	break;

    default:
	strncpy(s, "Unknown AF", maxlen);
	return NULL;
    }

    return s;
}

static void *RecordOriginalUri(void *cls, const char *uri) {
    printf ("RecordOriginalUri: %s\n", uri);

    struct connectionInfo *conInfo = new struct connectionInfo;
    if (NULL == conInfo) {
	return NULL;
    }
    conInfo->CallbackCount = 0;
    conInfo->URI = string(uri);
    conInfo->TransactionStartTime = microsec_clock::local_time();

    return conInfo;
}

static void LogRequest(struct MHD_Connection *connection, int http_code, struct connectionInfo *conInfo, int queueRetVal) {

    struct sockaddr *so;
    so = MHD_get_connection_info(connection, MHD_CONNECTION_INFO_CLIENT_ADDRESS)->client_addr;
    char remoteHost[128];
    memset(remoteHost, 128, '\0');
    get_ip_str(so, remoteHost, 128);

    const char *referer = MHD_lookup_connection_value(connection, MHD_HEADER_KIND, MHD_HTTP_HEADER_REFERER);
    const char *userAgent = MHD_lookup_connection_value(connection, MHD_HEADER_KIND, MHD_HTTP_HEADER_USER_AGENT);
    
    char timeStr[128];
    memset(timeStr, 128, '\0');
    const time_t now = time(NULL);
    struct tm *timeInfo = gmtime(&now);
    strftime(timeStr, 127, "%d/%b/%Y:%T %z", timeInfo);
    
    string identity("-");
    string userName("-");

    string connectionType = "-";
    if (conInfo->ConnectionType == POST) {
	connectionType = "POST";
    } else if (conInfo->ConnectionType == GET) {
	connectionType = "GET";
    }

    time_duration dur = microsec_clock::local_time() - conInfo->TransactionStartTime;
    char durStr[512];
    memset(durStr, 512, '\0');
    long seconds = dur.total_seconds();
    long milliseconds = dur.fractional_seconds() / 1000;
    snprintf(durStr, 511, "%ld.%03ld", seconds, milliseconds);

    string logline = remoteHost;
    logline += " " + identity;
    logline += " " + userName;
    logline += " [" + string(timeStr) + "]";
    logline += " \"" + connectionType;
    logline += " " + conInfo->URI;
    logline += " " + conInfo->Version + "\"";
    logline += " " + to_string(conInfo->RequestSize);
    logline += " [" + conInfo->PlanItem + "]";
    logline += " " + to_string(conInfo->PlanRank);
    logline += " " + to_string(http_code);
    logline += " " + to_string(conInfo->ResponseSize);
    logline += " " + string(durStr);
    logline += " " + to_string(queueRetVal);
    logline += " \"" + string(referer != NULL ? referer : "-") + "\"";
    logline += " \"" + string(userAgent != NULL ? userAgent : "-") + "\"";
    
    WebServerLog::Instance()->EmitLog(logline);
}

static int SendPage(struct MHD_Connection *connection, const char *page, int status_code, struct connectionInfo *conInfo) {
    struct MHD_Response *response;
    response = MHD_create_response_from_buffer(strlen(page), (void*)page, MHD_RESPMEM_MUST_COPY);
    if (!response) {
	return MHD_NO;
    }
    conInfo->ResponseSize = strlen(page);

    MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE, "text/html");
    int ret = MHD_queue_response(connection, status_code, response);
    MHD_destroy_response(response);

    LogRequest(connection, status_code, conInfo, ret);

    return ret;
}

static int HandleGet(struct MHD_Connection *cxn, const char *url, struct connectionInfo *conInfo) {

    string pagePath = "demo_site";
    pagePath += url;

    struct stat fileStat;
    FILE *fh;
    if (0 == stat(pagePath.c_str(), &fileStat)) {
	if (S_ISREG(fileStat.st_mode)) {
	    fh = fopen(pagePath.c_str(), "rb");
	} else {
	    fh = NULL;
	    printf("directory requests not supported\n");
	}
    } else {
	fh = NULL;
    }

    if (fh == NULL) {
	printf("Current working directory: %s\n", initial_path().string().c_str());
	printf("Can't open requested file: %s\n", pagePath.c_str());
	return SendPage(cxn, NotFound, 404, conInfo);
    }

    printf("requestPath = %s; fsPath = %s\n", url, pagePath.c_str());

    conInfo->ResponseSize = fileStat.st_size;

    struct MHD_Response *response = MHD_create_response_from_callback(fileStat.st_size, 32 * 1024, &file_reader, fh, &free_callback);
    if (response == NULL) {
	fclose(fh);
	return MHD_NO;
    }

    int ret = MHD_queue_response(cxn, MHD_HTTP_OK, response);
    MHD_destroy_response(response);

    LogRequest(cxn, MHD_HTTP_OK, conInfo, ret);

    return ret;

}

static int PostIterator(void *coninfo_cls, enum MHD_ValueKind kind, const char *key,
			const char *filename, const char *content_type,
			const char *transfer_encoding, const char *data, uint64_t off,
			size_t size)
{
    struct connectionInfo *conInfo = static_cast<connectionInfo*>(coninfo_cls);

    printf("key:[%s]; ", (key != NULL ? key : "NULL"));
    printf("filename: [%s]; ", (filename != NULL ? filename : "NULL"));
    printf("content_type: [%s]; ", (content_type != NULL ? content_type : "NULL"));
    printf("transfer_encoding: [%s]; ", (transfer_encoding != NULL ? transfer_encoding : "NULL"));
    printf("data: [%s]; ", (*data != '\0' ? data : "NULL"));
    printf("offset: %lu; ", off);
    printf("size: %lu;\n", size);

    if (conInfo->PostHandlingError == 0) {
	if (size > 0) {
	    assert (conInfo->RequestArgs != NULL);

	    string keyStr = key;
	    string valStr = data;
	    
	    ulong additionalBytes = keyStr.size() + valStr.size();
	    ulong newSize = conInfo->PostDataSize + additionalBytes;
	    
	    if (newSize > MAXPOSTSIZE) {
		printf("Too much post data (%lu bytes; max: %d) - will not process this post\n", newSize, MAXPOSTSIZE);
		conInfo->PostHandlingError = 1;
		delete(conInfo->RequestArgs);
		conInfo->RequestArgs = NULL;
		conInfo->PostDataSize = 0;
	    } else {
		printf("adding map[%s] = '%s'\n", keyStr.c_str(), valStr.c_str());
		conInfo->PostDataSize = newSize;
		bool appendedValue = off > 0;
		conInfo->RequestArgs->AddKeyVal(keyStr, valStr, appendedValue);
	    }
	} else {
	    printf("PostIterator called with less than one bytes\n");
	}
    } else {
	printf("Post already too large - skipping this portion too\n");
    }

    return MHD_YES;
}

static void RequestCompleted(void *cls, struct MHD_Connection *con, void **con_cls, enum MHD_RequestTerminationCode toe)
{
    printf("RequestCompleted() - deleting conInfo\n");
    struct connectionInfo *conInfo = static_cast<connectionInfo*>(*con_cls);
    if (NULL == conInfo) {
	return;
    }

    if (conInfo->ConnectionType == POST) {
	if (NULL != conInfo->PostProcessor) {
	    MHD_destroy_post_processor(conInfo->PostProcessor);
	    nr_of_clients--;
	}
	if (conInfo->RequestArgs != NULL) {
	    delete conInfo->RequestArgs;
	}
    }
    delete conInfo;
    *con_cls = NULL;
}

static int HandlePost(struct MHD_Connection *connection, const char *url, struct connectionInfo *conInfo)
{
    printf("HandlePost(%s)\n", url);
    if (strncmp("/entities", url, strlen("/entities")) == 0) {
	// search
	auto keyValEntry = conInfo->RequestArgs->ArgMap.find("search");
	if (keyValEntry == conInfo->RequestArgs->ArgMap.end()) {
	    printf("bad search - no value for key 'search'\n");
	    //  TODO send error page here
	    char buffer[1024];
	    snprintf(buffer, sizeof(buffer), "[ \"foo\", \"orange\" ]");
	    return SendPage(connection, buffer, MHD_HTTP_OK, conInfo);
	}
	
	string val = keyValEntry->second;
	printf("looking for matches to: '%s'\n", val.c_str());

	OfficialData* rulesGraph = OfficialData::Instance();
	vector<string> entities = rulesGraph->SearchForEntitiesMatchingStrings(val.c_str());

	unsigned maxReturnCount = 47;
	char buf[64];
	memset(buf, '\0', 64);
	if (entities.size() > maxReturnCount) {
	    snprintf(buf, 63, ", but only returning %u of them", maxReturnCount);
	}
	printf("Got %lu entities from \"%s\"%s\n", entities.size(), val.c_str(), buf);

	int addSep = 0;
	string result = "[";
	auto itr = entities.begin();
	unsigned entryCount = 0;
	for(; itr != entities.end(); ++itr) {
	    if (addSep != 0) { result += ","; } else { addSep = 1; }
	    result += " \"" + *itr + "\"";
	    if (++entryCount >= maxReturnCount) { break; }	    
	}
	result += " ]";
	printf("%s\n", result.c_str());
	return SendPage(connection, result.c_str(), MHD_HTTP_OK, conInfo);
    }

    if (strncmp("/plan", url, strlen("/plan")) == 0) {
	auto keyValEntry = conInfo->RequestArgs->ArgMap.find("Entity");
	if (keyValEntry == conInfo->RequestArgs->ArgMap.end()) {
	    printf("bad plan request - no value for key 'Entity'\n");
	    //  TODO send error page here
	    char buffer[1024];
	    snprintf(buffer, sizeof(buffer), "[ \"foo\", \"orange\" ]");
	    return SendPage(connection, buffer, MHD_HTTP_OK, conInfo);
	}
	string entityName = keyValEntry->second;
	conInfo->PlanItem = entityName;
	printf("Getting plan for entity: [%s]\n", entityName.c_str());

	keyValEntry = conInfo->RequestArgs->ArgMap.find("Store");
	string storeSerialized = "default";
	if (keyValEntry != conInfo->RequestArgs->ArgMap.end()) {
	    storeSerialized = keyValEntry->second;
	}

	Supply *store = Supply::Deserialize(storeSerialized.c_str());

	keyValEntry = conInfo->RequestArgs->ArgMap.find("Tracked");
	string trackedSerialized = "default";
	if (keyValEntry != conInfo->RequestArgs->ArgMap.end()) {
	    trackedSerialized = keyValEntry->second;
	}

	TrackedResources *tracked = TrackedResources::Deserialize(trackedSerialized.c_str());

	unsigned rank = 0;
	keyValEntry = conInfo->RequestArgs->ArgMap.find("Rank");
	if (keyValEntry != conInfo->RequestArgs->ArgMap.end()) {
	    rank = atoi(keyValEntry->second.c_str());
	    conInfo->PlanRank = rank;
	}

	// returns json
	Logger::Instance()->SetLoggingLevel(Logger::Level::Verbose);
	string result = Planners::CreatePlanForItemGoalForWeb(entityName.c_str(), rank, store, tracked);
	printf("returning for plan: %s\n", result.c_str());

	return SendPage(connection, result.c_str(), MHD_HTTP_OK, conInfo);
    }

    char buffer[1024];
    snprintf(buffer, sizeof(buffer), "[ \"foo\", \"orange\" ]");
    return SendPage(connection, buffer, MHD_HTTP_OK, conInfo);
}

static int AnswerToConnection(void *cls, struct MHD_Connection *connection,
			      const char *url,
			      const char *method,
			      const char *version,
			      const char *upload_data,
			      size_t *upload_data_size,
			      void **con_cls)
{
    printf("AnswerToConnection - ENTRY\n");

    // con_cls will be created by the logging callback, RecordOriginalUri
    assert(NULL != *con_cls);
    
    struct connectionInfo *conInfo = static_cast<connectionInfo*>(*con_cls);
    int timesWeveBeenCalledBack = conInfo->CallbackCount;
    printf("AnswerToConnection - callback count = %d\n", timesWeveBeenCalledBack);

    if (timesWeveBeenCalledBack == 0) {
	printf("AnswerToConnection - first callback\n");
	// first time calling this for the connection

	if (nr_of_clients >= MAXCLIENTS) {
	    return SendPage(connection, BusyPage, MHD_HTTP_SERVICE_UNAVAILABLE, conInfo);
	}

	conInfo->RequestArgs = new WebClientRequestArgs();
	conInfo->PostDataSize = 0;
	conInfo->PostHandlingError = 0;
	conInfo->RequestSize = -1;
	conInfo->CallbackCount = 1;
	conInfo->Version = version;
	conInfo->ResponseSize = 0;

	if (NULL != upload_data_size) {
	    conInfo->RequestSize = *upload_data_size;
	}

	//MHD_get_connection_values(connection, MHD_HEADER_KIND, PrintOutKey, NULL);
	//MHD_get_connection_values(connection, MHD_GET_ARGUMENT_KIND, PrintOutKey, NULL);

	if (0 == strcmp(method, "POST")) {
	    conInfo->PostProcessor = MHD_create_post_processor(connection, POSTBUFFERSIZE, PostIterator, (void*)conInfo);
	    if (NULL == conInfo->PostProcessor) {
		delete conInfo;
		return MHD_NO;
	    }

	    nr_of_clients++;
	    conInfo->ConnectionType = POST;

	} else {
	    conInfo->ConnectionType = GET;
	}

	return MHD_YES;
    }

    if (NULL != upload_data_size) {
	int uploadSize = *upload_data_size;
	int updatedSize = conInfo->RequestSize + uploadSize;
	if (conInfo->RequestSize != updatedSize) {
	    printf("updating uploadSize from %d to %d\n", conInfo->RequestSize, updatedSize);
	    conInfo->RequestSize = updatedSize;
	}
    }


    if (0 == strcmp(method, "GET")) {
	conInfo->CallbackCount++;
	printf("AnswerToConnection - handling GET\n");
	return HandleGet(connection, url, conInfo);
    }

    if (0 == strcmp(method, "POST")) {
	conInfo->CallbackCount++;
	if (0 != *upload_data_size) {
	    printf("AnswerToConnection - processing POST\n");
	    MHD_post_process(conInfo->PostProcessor, upload_data, *upload_data_size);
	    *upload_data_size = 0;
	    return MHD_YES;
	} else {
	    printf("AnswerToConnection - replying to POST\n");
	    struct connectionInfo *conInfo = static_cast<connectionInfo*>(*con_cls);
	    printf("post body %d bytes\n", conInfo->PostDataSize);
	    return HandlePost(connection, url, conInfo);
	}
    }
    return SendPage(connection, ErrorPage, MHD_HTTP_BAD_REQUEST, conInfo);
}

void MicroHttpdTest() {
 
    struct MHD_Daemon *daemon;
    // daemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY, 8888, NULL, NULL, &AnswerToConnection, NULL, MHD_OPTION_END);
    daemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY, PORT,
			      NULL, NULL,  // policy callback - none here
			      &AnswerToConnection, NULL,
			      MHD_OPTION_NOTIFY_COMPLETED, RequestCompleted, NULL, // FxnPtr MHD_RequestCompletedCallback() and ptr to callback arg (may be NULL)
			      MHD_OPTION_URI_LOG_CALLBACK, RecordOriginalUri, NULL,
			      MHD_OPTION_END);

    if (NULL == daemon) {
	printf("failed...\n");
    }
    getchar();
    
    MHD_stop_daemon(daemon);
    return;

}

int main(int argc, char **argv) {

    OfficialData* rulesGraph = OfficialData::Instance();
    rulesGraph->ProcessSpreadsheetDir("official_data");

    MicroHttpdTest();
    cout << "test over" << endl;
    WebServerLog::Instance()->CloseLog();

    return 0;
}


