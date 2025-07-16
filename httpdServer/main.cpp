/* Feel free to use this example code in any way
   you see fit (Public Domain) */

#include <sys/types.h>
#ifndef _WIN32
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/stat.h>
#else
#include <winsock2.h>
#include <direct.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <map>
#include <microhttpd.h>
#include <string.h>
#include <vector>
#include <fstream>


#ifdef _MSC_VER
#ifndef strcasecmp
#define strcasecmp(a,b) _stricmp ((a),(b))
#endif /* !strcasecmp */
#endif /* _MSC_VER */

#if defined(_MSC_VER) && _MSC_VER + 0 <= 1800
   /* Substitution is OK while return value is not used */
#define snprintf _snprintf
#endif

// #define PORT            8888
#define POSTBUFFERSIZE  512
#define MAXCLIENTS      1

// define separator
#ifndef _WIN32
#define PATH_SEP '/'
#define PATH_SEP_STR "/"
#else
#define PATH_SEP '\\'
#define PATH_SEP_STR "\\"
#endif

static std::map<std::string, std::string> s_mParams;

enum ConnectionType
{
    GET = 0,
    POST = 1
};

static unsigned int nr_of_uploading_clients = 0;

/**
 * Information we keep per connection.
 */
struct connection_info_struct
{
    enum ConnectionType connectiontype;

    /**
     * Handle to the POST processing state.
     */
    struct MHD_PostProcessor* postprocessor;

    /**
     * File handle where we write uploaded data.
     */
    FILE* fp;

    /**
     * HTTP response body we will return, NULL if not yet known.
     */
    const char* answerstring;

    /**
     * HTTP status code we will return, 0 for undecided.
     */
    unsigned int answercode;

    const char* filename;

    const char* sessionId;
};

const char* response_busy = "This server is busy, please try again later.";
const char* response_error = "This doesn't seem to be right.";
const char* response_servererror = "Invalid request.";
const char* response_fileioerror = "IO error writing to disk.";
const char* const response_postprocerror = "Error processing POST data";
const char* response_conversionerror = "Conversion error";
const char* response_success = "success";

static enum MHD_Result
sendResponseSuccess(struct MHD_Connection* connection)
{
    struct MHD_Response* response;
    MHD_Result ret = MHD_NO;

    response = MHD_create_response_from_buffer(0, (void*)NULL, MHD_RESPMEM_MUST_COPY);
    MHD_add_response_header(response, MHD_HTTP_HEADER_ACCESS_CONTROL_ALLOW_ORIGIN, "*");
    ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);

    return ret;
}

static enum MHD_Result
sendResponseText(struct MHD_Connection* connection, const char* responseText, int status_code)
{
    struct MHD_Response* response;
    MHD_Result ret = MHD_NO;

    response = MHD_create_response_from_buffer(strlen(responseText), (void*)responseText, MHD_RESPMEM_PERSISTENT);

    MHD_add_response_header(response, MHD_HTTP_HEADER_ACCESS_CONTROL_ALLOW_ORIGIN, "*");
    ret = MHD_queue_response(connection, status_code, response);
    MHD_destroy_response(response);

    return ret;
}

static enum MHD_Result
sendResponseFloatArr(struct MHD_Connection *connection, std::vector<float> &floatArray)
{
	struct MHD_Response *response;
	MHD_Result ret = MHD_NO;

	if (0 < floatArray.size())
	{
		response = MHD_create_response_from_buffer(floatArray.size() * sizeof(float), (void*)&floatArray[0], MHD_RESPMEM_MUST_COPY);
		std::vector<float>().swap(floatArray);
	}
	else
	{
		return MHD_NO;
	}

	MHD_add_response_header(response, MHD_HTTP_HEADER_ACCESS_CONTROL_ALLOW_ORIGIN, "*");

	ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
	MHD_destroy_response(response);

	return ret;
}

bool paramStrToStr(const char *key, std::string &sVal)
{
	sVal = s_mParams[std::string(key)];
	if (sVal.empty()) return false;

	return true;
}

bool paramStrToInt(const char *key, int &iVal)
{
	std::string sVal = s_mParams[std::string(key)];
	if (sVal.empty()) return false;

	iVal = std::atoi(sVal.c_str());

	return true;
}

bool paramStrToDbl(const char *key, double &dVal)
{
	std::string sVal = s_mParams[std::string(key)];
	if (sVal.empty()) return false;

	dVal = std::atof(sVal.c_str());

	return true;
}

bool paramStrToChr(const char *key, char &cha)
{
	std::string sVal = s_mParams[std::string(key)];
	if (sVal.empty()) return false;

	cha = sVal.c_str()[0];

	return true;
}

void addFile(std::ofstream& ofs, const char* inFile, bool addEndl)
{
    std::ifstream ifs;
    std::string str;

    ifs.open(inFile);
    while (getline(ifs, str))
    {
        ofs << str;
        if (addEndl)
            ofs << std::endl;
    }
    ifs.close();
}

static enum MHD_Result
iterate_post(void* coninfo_cls,
    enum MHD_ValueKind kind,
    const char* key,
    const char* filename,
    const char* content_type,
    const char* transfer_encoding,
    const char* data,
    uint64_t off,
    size_t size)
{
    struct connection_info_struct* con_info = (connection_info_struct*)coninfo_cls;
    FILE* fp;
    (void)kind;               /* Unused. Silent compiler warning. */
    (void)content_type;       /* Unused. Silent compiler warning. */
    (void)transfer_encoding;  /* Unused. Silent compiler warning. */
    (void)off;                /* Unused. Silent compiler warning. */

    if (0 == strcmp(key, "file"))
    {
        if (!con_info->fp)
        {
            if (0 != con_info->answercode)   /* something went wrong */
                return MHD_YES;

            char filePath[FILENAME_MAX];

#ifndef _WIN32
            sprintf(filePath, "/tmp/%s/%s", con_info->sessionId, filename);
#else
            sprintf(filePath, "C:\\temp\\%s%s\\%s", con_info->sessionId, filename);
#endif

            /* NOTE: This is technically a race with the 'fopen()' above,
            but there is no easy fix, short of moving to open(O_EXCL)
            instead of using fopen(). For the example, we do not care. */
            con_info->fp = fopen(filePath, "ab");

            if (!con_info->fp)
            {
                con_info->answerstring = response_fileioerror;
                con_info->answercode = MHD_HTTP_INTERNAL_SERVER_ERROR;
                return MHD_YES;
            }
        }

        if (size > 0)
        {
            if (!fwrite(data, sizeof(char), size, con_info->fp))
            {
                con_info->answerstring = response_fileioerror;
                con_info->answercode = MHD_HTTP_INTERNAL_SERVER_ERROR;
                return MHD_YES;
            }
        }
    }
    else if (0 == strcmp(key, "script"))
    {
        if (!con_info->fp)
        {
            if (0 != con_info->answercode)   /* something went wrong */
                return MHD_YES;

            char filePath[FILENAME_MAX];
            sprintf(filePath, "..%s..%sHtmlConverter%stemplate%s%s.json", PATH_SEP_STR, PATH_SEP_STR, PATH_SEP_STR, PATH_SEP_STR, con_info->sessionId);
            con_info->fp = fopen(filePath, "w");

            if (!con_info->fp)
            {
                con_info->answerstring = response_fileioerror;
                con_info->answercode = MHD_HTTP_INTERNAL_SERVER_ERROR;
                return MHD_YES;
            }
        }

        if (size > 0)
        {
            if (!fwrite(data, sizeof(char), size, con_info->fp))
            {
                con_info->answerstring = response_fileioerror;
                con_info->answercode = MHD_HTTP_INTERNAL_SERVER_ERROR;
                return MHD_YES;
            }
        }

    }
    else if (size > 0)
    {
        s_mParams.insert(std::make_pair(std::string(key), std::string(data)));
        printf("key: %s, data: %s\n", key, data);
    }

    return MHD_YES;
}


static void
request_completed(void* cls,
    struct MHD_Connection* connection,
    void** con_cls,
    enum MHD_RequestTerminationCode toe)
{
    struct connection_info_struct* con_info = (connection_info_struct*)*con_cls;
    (void)cls;         /* Unused. Silent compiler warning. */
    (void)connection;  /* Unused. Silent compiler warning. */
    (void)toe;         /* Unused. Silent compiler warning. */

    if (NULL == con_info)
        return;

    if (con_info->connectiontype == POST)
    {
        if (NULL != con_info->postprocessor)
        {
            MHD_destroy_post_processor(con_info->postprocessor);
            nr_of_uploading_clients--;
        }

        if (con_info->fp)
            fclose(con_info->fp);
    }

    free(con_info);
    *con_cls = NULL;
}


static enum MHD_Result
answer_to_connection(void* cls,
    struct MHD_Connection* connection,
    const char* url,
    const char* method,
    const char* version,
    const char* upload_data,
    size_t* upload_data_size,
    void** con_cls)
{
    (void)cls;               /* Unused. Silent compiler warning. */
    (void)url;               /* Unused. Silent compiler warning. */
    (void)version;           /* Unused. Silent compiler warning. */
    
    if (NULL == *con_cls)
    {
        /* First call, setup data structures */
        struct connection_info_struct* con_info;
        
        if (nr_of_uploading_clients >= MAXCLIENTS)
            return sendResponseText(connection, response_busy, MHD_HTTP_OK);

        con_info = (connection_info_struct*)malloc(sizeof(struct connection_info_struct));
        if (NULL == con_info)
            return MHD_NO;
        con_info->answercode = 0;   /* none yet */
        con_info->fp = NULL;
        s_mParams.clear();

        printf("--- New %s request for %s using version %s\n", method, url, version);
        con_info->sessionId = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "session_id");
        printf("Session ID: %s\n", con_info->sessionId);

        if (0 == strcasecmp(method, MHD_HTTP_METHOD_POST))
        {
            con_info->postprocessor =
                MHD_create_post_processor(connection,
                    POSTBUFFERSIZE,
                    &iterate_post,
                    (void*)con_info);

            if (NULL == con_info->postprocessor)
            {
                free(con_info);
                return MHD_NO;
            }

            nr_of_uploading_clients++;

            con_info->connectiontype = POST;
        }
        else
        {
            con_info->connectiontype = GET;
        }

        *con_cls = (void*)con_info;

        return MHD_YES;
    }

    if (0 == strcasecmp(method, MHD_HTTP_METHOD_GET))
    {
        return sendResponseSuccess(connection);
    }

    if (0 == strcasecmp(method, MHD_HTTP_METHOD_POST))
    {
        struct connection_info_struct* con_info = (connection_info_struct*)*con_cls;

        if (0 != *upload_data_size)
        {
            /* Upload not yet done */
            if (0 != con_info->answercode)
            {
                /* we already know the answer, skip rest of upload */
                *upload_data_size = 0;
                return MHD_YES;
            }
            if (MHD_YES !=
                MHD_post_process(con_info->postprocessor,
                    upload_data,
                    *upload_data_size))
            {
                con_info->answerstring = response_postprocerror;
                con_info->answercode = MHD_HTTP_INTERNAL_SERVER_ERROR;
            }
            *upload_data_size = 0;

            return MHD_YES;
        }
        /* Upload finished */

        if (0 == strcmp(url, "/FileUpload"))
        {
            if (NULL != con_info->fp)
            {
                fclose(con_info->fp);
                con_info->fp = NULL;
            }
            if (0 == con_info->answercode)
            {
                /* No errors encountered, declare success */

                std::vector<float> floatArr;
                con_info->answerstring = response_success;
                con_info->answercode = MHD_HTTP_OK;

                return sendResponseFloatArr(connection, floatArr);
            }
        }
        else if (0 == strcmp(url, "/Download"))
        {
            if (NULL != con_info->fp)
            {
                fclose(con_info->fp);
                con_info->fp = NULL;
            }
            if (0 == con_info->answercode)
            {
                std::string modelName;
                if (!paramStrToStr("modelName", modelName)) return MHD_NO;

                std::string options;
                if (!paramStrToStr("options", options)) return MHD_NO;

                char filePath[FILENAME_MAX];
                sprintf(filePath, "..%s..%sHtmlConverter%stemplate%s17_custom_script_before_start_viewer.js", 
                    PATH_SEP_STR, PATH_SEP_STR, PATH_SEP_STR, PATH_SEP_STR);

                std::ofstream ofs;
                ofs.open(filePath, std::ios::out | std::ios::trunc);

                ofs << "const animDef = '";

                sprintf(filePath, "..%s..%sHtmlConverter%stemplate%s%s.json", 
                    PATH_SEP_STR, PATH_SEP_STR, PATH_SEP_STR, PATH_SEP_STR, con_info->sessionId);
                addFile(ofs, filePath, false);
                
                remove(filePath);

                ofs << "';" << std::endl;

                sprintf(filePath, "..%s..%sHtmlConverter%stemplate%s17_custom_script_before_start_viewer_temp.js", 
                    PATH_SEP_STR, PATH_SEP_STR, PATH_SEP_STR, PATH_SEP_STR);
                addFile(ofs, filePath, true);

                ofs.close();

                // Start HTML converter
                char command[1024];
                sprintf(command, "..%s..%sHtmlConverter%sbin%sExport3DToHtml ..%s..%sdata%s%s.prc ..%s..%sHtmlConverter%stemplate %s ..%s%s.html",
                    PATH_SEP_STR, PATH_SEP_STR, PATH_SEP_STR, PATH_SEP_STR, 
                    PATH_SEP_STR, PATH_SEP_STR, PATH_SEP_STR, modelName.data(),
                    PATH_SEP_STR, PATH_SEP_STR, PATH_SEP_STR, 
                    options.data(), PATH_SEP_STR, con_info->sessionId);
     
                printf("HTML exporting ...\n");
                system(command);
                printf("\nHTML export was done.\n");

                con_info->answerstring = response_success;
                con_info->answercode = MHD_HTTP_OK;
                return sendResponseText(connection, con_info->answerstring, con_info->answercode);
            }

            return MHD_NO;
        }
        else if (0 == strcmp(url, "/Downloaded"))
        {
            char filePath[FILENAME_MAX];
            sprintf(filePath, "..%s%s.html", PATH_SEP_STR, con_info->sessionId);
            remove(filePath);

            con_info->answerstring = response_success;
            con_info->answercode = MHD_HTTP_OK;
            return sendResponseText(connection, con_info->answerstring, con_info->answercode);
        }
    }

    return MHD_NO;
}


int
main(int argc, char** argv)
{
    if (argc != 2) {
        printf("%s PORT\n",
            argv[0]);
        return 1;
    }

    int iPort = atoi(argv[1]);
    printf("Bind to %d port\n", iPort);

    struct MHD_Daemon* daemon;

    daemon = MHD_start_daemon(MHD_USE_INTERNAL_POLLING_THREAD,
        iPort, NULL, NULL,
        &answer_to_connection, NULL,
        MHD_OPTION_NOTIFY_COMPLETED, &request_completed,
        NULL,
        MHD_OPTION_END);
    if (NULL == daemon)
    {
        fprintf(stderr,
            "Failed to start daemon.\n");
        return 1;
    }

    bool bFlg = true;
    while (bFlg)
    {
        if (getchar())
            bFlg = false;

    }
    MHD_stop_daemon(daemon);

    return 0;
}
