#include <string.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <microhttpd.h>

#include <cassert>

#include <iostream>
#include <vector>
#include <string>

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

#define GET             0
#define POST            1


static unsigned int nr_of_clients = 0;

struct connectionInfo
{
    int ConnectionType;
    char **Buffer;
    int BufferSize;
    struct MHD_PostProcessor *PostProcessor;
};

const char *ErrorPage = "<html><body>This doesn't seem to be right.</body></html>";
const char *BusyPage = "<html><body>This server is busy, please try again later.</body></html>";
const char *CompletePage = "<html><body>The upload has been completed.</body></html>";
const char *ServererrorPage = "<html><body>An internal server error has occured.</body></html>";
const char *GetPage = "\
<html>\
<head>\
  <script src=\"//code.jquery.com/jquery-1.11.0.min.js\"></script>\
  <script>\
    $(document).ready(function(){\
       $('#fetch').click(function(){\
          $.ajax({\
              url: '/Item/',\
              type: 'POST',\
              data: { zipcode: 94070, items: [ 'grob', 'reed' ], store: [ { 'clay': { 'qty': 5 }, 'wood': { 'qty': 10 } } ] },\
              success: function(data) {\
                  $('#Reply').html(data + ' degrees');\
              }\
          });\
       });\
    });\
  </script>\
</head>\
<body>\
<div id='Reply'></div>\
<button id='fetch'>Fetch</button>\
</body>\
</html>";

void MicroHttpdTest();

using namespace std;

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

static int PostIterator(void *coninfo_cls, enum MHD_ValueKind kind, const char *key,
			const char *filename, const char *content_type,
			const char *transfer_encoding, const char *data, uint64_t off,
			size_t size)
{
    // struct connectionInfo *conInfo = static_cast<connectionInfo*>(coninfo_cls);

    printf("key:[%s]; ", (key != NULL ? key : "NULL"));
    printf("filename: [%s]; ", (filename != NULL ? filename : "NULL"));
    printf("content_type: [%s]; ", (content_type != NULL ? content_type : "NULL"));
    printf("transfer_encoding: [%s]; ", (transfer_encoding != NULL ? transfer_encoding : "NULL"));

    char buff[1024];
    memset(buff, '\0', sizeof(buff));
    if (size > 0) {
	memcpy(buff, data, sizeof(buff));
    }

    printf("data: [%s]; ", (*data != NULL ? buff : "NULL"));
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

static int AnswerToConnection(void *cls, struct MHD_Connection *connection,
			      const char *url,
			      const char *method,
			      const char *version,
			      const char *upload_data,
			      size_t *upload_data_size,
			      void **con_cls)
{
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
	return SendPage(connection, GetPage, MHD_HTTP_OK);
    }

    if (0 == strcmp(method, "POST")) {
	printf("AnswerToConnection - handling POST\n");
	struct connectionInfo *conInfo = static_cast<connectionInfo*>(*con_cls);
	if (0 != *upload_data_size) {
	    MHD_post_process(conInfo->PostProcessor, upload_data, *upload_data_size);
	    *upload_data_size = 0;
	    return MHD_YES;
	} else {
	    char buffer[1024];
	    snprintf(buffer, sizeof(buffer), "AnswerToConnection called; POST, and no upload_data");
	    return SendPage(connection, buffer, MHD_HTTP_OK);
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

    MicroHttpdTest();
    cout << "test over" << endl;

    return 0;
}


