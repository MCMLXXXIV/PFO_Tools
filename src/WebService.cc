#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

#include <microhttpd.h>

#include <cassert>

#include <iostream>
#include <vector>
#include <string>

#include <boost/filesystem.hpp>

#include "EntityTypeHelper.h"
#include "OfficialData.h"
#include "Planners.h"
#include "Supply.h"
#include "TrackedResources.h"
#include "Cost.h"
#include "Plan.h"
#include "Utils.h"
#include "CommandLineOptions.h"


#define PORT            8888
#define POSTBUFFERSIZE  512
#define MAXCLIENTS      2
#define MAXPOSTSIZE     8192

#define GET             0
#define POST            1


static unsigned int nr_of_clients = 0;

struct connectionInfo
{
    int ConnectionType;
    char *Buffer;
    int BufferContentLength;
    ulong BufferSize;
    int PostTooLarge;
    struct MHD_PostProcessor *PostProcessor;
};

const char *NotFound =  "<html><head><title>File not found</title></head><body>File not found</body></html>";
const char *ErrorPage = "<html><body>This doesn't seem to be right.</body></html>";
const char *BusyPage = "<html><body>This server is busy, please try again later.</body></html>";
const char *CompletePage = "<html><body>The upload has been completed.</body></html>";
const char *ServererrorPage = "<html><body>An internal server error has occured.</body></html>";

void MicroHttpdTest();

using namespace std;
using namespace boost::filesystem;

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

static int SendPage(struct MHD_Connection *connection, const char *page, int status_code) {
    int ret;
    struct MHD_Response *response;
    response = MHD_create_response_from_buffer(strlen(page), (void*)page, MHD_RESPMEM_MUST_COPY);
    if (!response) {
	return MHD_NO;
    }
    MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE, "text/html");
    ret = MHD_queue_response(connection, status_code, response);
    MHD_destroy_response(response);
    return ret;
}

static int HandleGet(struct MHD_Connection *cxn, const char *url) {

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
	return SendPage(cxn, NotFound, 404);
    }

    printf("requestPath = %s; fsPath = %s\n", url, pagePath.c_str());

    struct MHD_Response *response = MHD_create_response_from_callback(fileStat.st_size, 32 * 1024, &file_reader, fh, &free_callback);
    if (response == NULL) {
	fclose(fh);
	return MHD_NO;
    }

    int ret = MHD_queue_response(cxn, MHD_HTTP_OK, response);
    MHD_destroy_response(response);

    return ret;

}

static int PostIterator(void *coninfo_cls, enum MHD_ValueKind kind, const char *key,
			const char *filename, const char *content_type,
			const char *transfer_encoding, const char *data, uint64_t off,
			size_t size)
{
    struct connectionInfo *conInfo = static_cast<connectionInfo*>(coninfo_cls);

    if (conInfo->PostTooLarge == 0) {
	if (size > 0) {
	    if (size + conInfo->BufferContentLength > conInfo->BufferSize) {
		ulong newSize = conInfo->BufferSize + POSTBUFFERSIZE;
		if (newSize > MAXPOSTSIZE) {
		    printf("Too much post data (%lu bytes; max: %d) - will not process this post\n", newSize, MAXPOSTSIZE);
		    conInfo->PostTooLarge = 1;
		    free(conInfo->Buffer);
		    conInfo->Buffer = NULL;
		    conInfo->BufferSize = 0;
		    conInfo->BufferContentLength = 0;
		} else {
		    if (conInfo->Buffer == NULL) {
			conInfo->Buffer = (char*)malloc(sizeof(char) * newSize);
			memset(conInfo->Buffer, '\0', newSize);
		    } else {
			conInfo->Buffer = (char*)realloc(conInfo->Buffer, sizeof(char) * newSize);
		    }
		    assert(conInfo->Buffer != NULL);
		    conInfo->BufferSize = newSize;
		}
	    }
	    if (conInfo->PostTooLarge == 0) {
		memcpy(conInfo->Buffer + conInfo->BufferContentLength, data, conInfo->BufferSize - conInfo->BufferContentLength);
		conInfo->BufferContentLength += size;
	    }
	}
    } else {
	printf("Post already too large - skipping this portion too\n");
    }

    char buf[POSTBUFFERSIZE + 1];
    memset(buf, '\0', sizeof(buf));
    memcpy(buf, data, sizeof(buf));

    printf("key:[%s]; ", (key != NULL ? key : "NULL"));
    printf("filename: [%s]; ", (filename != NULL ? filename : "NULL"));
    printf("content_type: [%s]; ", (content_type != NULL ? content_type : "NULL"));
    printf("transfer_encoding: [%s]; ", (transfer_encoding != NULL ? transfer_encoding : "NULL"));
    printf("data: [%s]; ", (*data != '\0' ? buf : "NULL"));
    printf("offset: %lu; ", off);
    printf("size: %lu;\n", size);

    return MHD_YES;
}

static void RequestCompleted(void *cls, struct MHD_Connection *con, void **con_cls, enum MHD_RequestTerminationCode toe)
{
    printf("RequestCompleted()\n");
    struct connectionInfo *conInfo = static_cast<connectionInfo*>(*con_cls);
    if (NULL == conInfo) {
	return;
    }

    if (conInfo->ConnectionType == POST) {
	if (NULL != conInfo->PostProcessor) {
	    MHD_destroy_post_processor(conInfo->PostProcessor);
	    nr_of_clients--;
	}
	if (conInfo->Buffer != NULL) {
	    free(conInfo->Buffer);
	}
    }

    delete conInfo;
    *con_cls = NULL;
}

static int HandlePost(struct MHD_Connection *connection, const char *url, struct connectionInfo *conInfo)
{
    printf("HandlePost(%s, %s)\n", url, conInfo->Buffer);
    if (strncmp("/entities", url, strlen("/entities")) == 0) {
	OfficialData* rulesGraph = OfficialData::Instance();
	vector<string> entities = rulesGraph->SearchForEntitiesMatchingStrings(conInfo->Buffer);

	printf("Got %lu entities from \"%s\"\n", entities.size(), conInfo->Buffer);

	int addSep = 0;
	string result = "[";
	vector<string>::iterator itr = entities.begin();
	for(; itr != entities.end(); ++itr) {
	    if (addSep != 0) { result += ","; } else { addSep = 1; }
	    result += " \"" + *itr + "\"";
	}
	result += " ]";
	printf("%s\n", result.c_str());
	return SendPage(connection, result.c_str(), MHD_HTTP_OK);
    }

    char buffer[1024];
    snprintf(buffer, sizeof(buffer), "[ \"foo\", \"orange\" ]");
    return SendPage(connection, buffer, MHD_HTTP_OK);
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
    if (NULL == *con_cls) {
	printf("AnswerToConnection - no con_cls yet\n");
	// fisrt time calling this for the connection
	struct connectionInfo *conInfo;

	if (nr_of_clients >= MAXCLIENTS) {
	    return SendPage(connection, BusyPage, MHD_HTTP_SERVICE_UNAVAILABLE);
	}

	conInfo = new struct connectionInfo;
	if (NULL == conInfo) {
	    return MHD_NO;
	}
	conInfo->Buffer = NULL;
	conInfo->BufferSize = 0;
	conInfo->BufferContentLength = 0;
	conInfo->PostTooLarge = 0;

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

	*con_cls = (void*)conInfo;
	return MHD_YES;
    }

    if (0 == strcmp(method, "GET")) {
	printf("AnswerToConnection - handling GET\n");
	return HandleGet(connection, url);
    }

    if (0 == strcmp(method, "POST")) {
	struct connectionInfo *conInfo = static_cast<connectionInfo*>(*con_cls);
	if (0 != *upload_data_size) {
	    printf("AnswerToConnection - processing POST\n");
	    MHD_post_process(conInfo->PostProcessor, upload_data, *upload_data_size);
	    *upload_data_size = 0;
	    return MHD_YES;
	} else {
	    printf("AnswerToConnection - replying to POST\n");
	    struct connectionInfo *conInfo = static_cast<connectionInfo*>(*con_cls);
	    printf("post body %d bytes: %s\n", conInfo->BufferContentLength, conInfo->BufferContentLength > 0 ? conInfo->Buffer : "NULL");
	    return HandlePost(connection, url, conInfo);
	}
    }
    return SendPage(connection, ErrorPage, MHD_HTTP_BAD_REQUEST);
}

void MicroHttpdTest() {
 
    struct MHD_Daemon *daemon;
    // daemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY, 8888, NULL, NULL, &AnswerToConnection, NULL, MHD_OPTION_END);
    daemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY, PORT,
			      NULL, NULL,  // policy callback - none here
			      &AnswerToConnection, NULL,
			      MHD_OPTION_NOTIFY_COMPLETED, RequestCompleted, NULL, // FxnPtr MHD_RequestCompletedCallback() and ptr to callback arg (may be NULL)
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

    return 0;
}


