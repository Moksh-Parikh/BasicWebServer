#include "server_functions.h"


int getHTTPMethod(char *request) {
    char* allowedMethods[NUMBER_OF_METHODS] = {
            "GET",
            "HEAD"
    };

    int i;

    char *methodPointer;

    for (i = 0; i < NUMBER_OF_METHODS; i++) {
        if (strstr(request, allowedMethods[i])) {
            break;
        }
    }

    return i + 1;
}


int getFirstLine(char* longString, char** splitString) {
    char* newlinePointer;
    int newlineIndex;

    newlinePointer = strchr(longString, '\n');

    if (newlinePointer == NULL) {
        return BAD_REQUEST;
    }

    newlineIndex = (int)(newlinePointer - longString);
    
    *splitString = calloc(newlineIndex, sizeof(char) );
    strncpy(*splitString, longString, newlineIndex);
    
    return 0;
}


int getFile(int* size, char* fileName, char** fileContent) {
    int fileDescriptor;
    long long fileSize;
    FILE* file;

    fileDescriptor = open(fileName, O_RDONLY);

    if (fileDescriptor < 0) {
        return FILE_NOT_FOUND;
    }

    fileSize = lseek(fileDescriptor, 0, SEEK_END);

    close(fileDescriptor);

    file = fopen(fileName, "rb");

    if (file == NULL) {
        return FILE_NOT_FOUND;
    }

    *fileContent = malloc(fileSize * sizeof(char));

    if (*fileContent == NULL) {
        fprintf(stderr, "failed to malloc(%lld)", fileSize * sizeof(char));

        return MALLOC_ERROR;
    }
    printf("size: %lld\n", fileSize);

    fread(*fileContent, sizeof(char), fileSize, file);
    fclose(file);

    *size = fileSize;

    return 0;
}


void getFileName(char* request, int method, char** outString) {
    char* methodPointer;
    char* HTTPPointer;

    int methodIndex;
    int HTTPIndex;
 
    methodPointer = strchr(request, '/');
    HTTPPointer = strstr(request, " HTTP");
    
    if (methodPointer == NULL || HTTPPointer == NULL) {
        strncpy(*outString, "badrequest.html\0", 16);
        return;
    }

    methodIndex = methodPointer - request + 1;
    HTTPIndex = HTTPPointer - request;

    if (HTTPIndex - methodIndex == 0) {        
        strncpy(*outString, "index.html\0", 11);
        return;
    }

    strncpy(*outString, request + methodIndex, HTTPIndex - methodIndex);
    (*outString)[HTTPIndex - methodIndex] = '\0';
}


int getMIMEType(char* fileName, char** output) {
    int dotIndex;
    char* dotPointer;
    
    char* fileExtensionList[60] = {".aac", ".abw", ".apng", ".arc", ".avif", ".avi", ".azw", ".bin", ".bmp ", ".bz", ".bz2", ".cda", ".csh", ".css", ".csv", ".doc", ".docx", ".eot", ".epub", ".gz", ".gif", ".htm", ".html", ".ico", ".ics", ".jar", ".jpeg", ".jpg", ".js", ".json", ".md", ".mid", ".midi", ".mp3", ".mp4", ".mpeg", ".oga", ".ogv", ".ogx", ".otf", ".png", ".pdf", ".php", ".rar", ".rtf", ".sh", ".svg", ".tar", ".tif", ".tiff", ".ttf", ".txt", ".wav", ".woff", ".woff2", ".xml", ".zip", ".3gp", ".3g2", ".7z"};
    char *MIMETypeList[60] = {"audio/aac", "application/x-abiword", "image/apng", "application/x-freearc", "image/avif", "video/x-msvideo", "application/vnd.amazon.ebook", "application/octet-stream", "image/bmp", "application/x-bzip", "application/x-bzip2", "application/x-cdf", "application/x-csh", "text/css", "text/csv", "application/msword", "application/vnd.openxmlformats-officedocument.wordprocessingml.document", "application/vnd.ms-fontobject", "application/epub+zip", "application/gzip", "image/gif", "text/html", "text/html", "image/x-icon", "text/calendar", "application/java-archive", "image/jpeg", "image/jpeg", "text/javascript", "application/json", "text/markdown", "audio/midi", "audio/midi", "audio/mpeg", "video/mp4", "video/mpeg", "audio/ogg", "video/ogg", "application/ogg", "font/otf", "image/png", "application/pdf", "application/x-httpd-php", "application/vnd.rar", "application/rtf", "application/x-sh", "image/svg+xml", "application/x-tar", "image/tiff", "image/tiff", "font/ttf", "text/plain", "audio/wav", "font/woff", "font/woff2", "application/xml", "application/zip", "video/3gpp", "video/3gpp2", "application/x-7z-compressed"};

    char* fileExtension;

    dotPointer = strrchr(fileName, '.');
    
    if (dotPointer == NULL) {
        // check if the request points to a directory
        struct stat directoryCheck;

        if(stat(fileName, &directoryCheck) == 0 && S_ISDIR(directoryCheck.st_mode)) {
            return IS_A_DIR;
        }
        *output = malloc( (strlen(MIMETypeList[7]) + 1 ) * sizeof(char));
        if (*output == NULL) return -1;

        // application/octet-stream
        strncpy(*output,
                MIMETypeList[7],
                strlen(MIMETypeList[7])
                );
        return 0;
    }

    fileExtension = calloc(10, sizeof(char));
    dotIndex = dotPointer - fileName;
 
    strncpy(fileExtension, fileName + dotIndex, strlen(fileName) - dotIndex);

    for (int i = 0; i < 60; i++) {
        if (strcmp(fileExtension, fileExtensionList[i]) == 0) {
            *output = calloc(
                strlen(MIMETypeList[i]) + 1,
                sizeof(char)
            );
            if (*output == NULL) return -1;
            
            strncpy(*output, MIMETypeList[i], strlen(MIMETypeList[i]));
            free(fileExtension);
            return 0;
        }
    }

    // application/octet-stream
    *output = malloc((strlen(MIMETypeList[7]) + 1 ) * sizeof(char));
    if (*output == NULL) return -1;
    strncpy(*output, MIMETypeList[7], strlen(MIMETypeList[7]));
    free(fileExtension);
    
    return 0;
}


int generateHTTPResponse(int responseCode, char* MIMEtype, char* content, long long contentSize, char** response) {
    // valid HTTP response requires:
    //  - HTTP type
    //  - Content Type
    //  - empty line after
    //  - content

    // plus 9 for the first line, and 3 for response code 
    // and 2 for CRLF
    // 14 for Content-Type,
    // and 4 for the last two CRLFs
    printf("%s\n", MIMEtype);
    int responseSize = (80 + strlen(MIMEtype) + contentSize) * sizeof(char);

    printf("response size: %d, contentSize: %d\n", responseSize, contentSize);

    *response = malloc(responseSize);
    if (*response == NULL) return -1;

    snprintf(*response, responseSize, 
         "HTTP/1.1 %d\r\n"
         "Content-Type: %s\r\n"
         "Content-Length: %lld\r\n"
         "\r\n",
         responseCode,
         MIMEtype,
         contentSize
    );

    memcpy(*response + strlen(*response), content, contentSize);
    printf("passed memcpy\n");

    return responseSize;
}



void* handleClient(void* client_fd) {
    char *response = NULL;
    int responseSize;

    char *request = NULL;
    char *requestFirstLine = NULL;
    int requestMethod;

    char* fileName = NULL;
    char* requestedFileContent = NULL;
    
    int fileSize;
    int fileStatus;

    char* MIMEtype = NULL;

    request = malloc(BUFFER_SIZE);
    requestFirstLine = NULL;

    if (request == NULL) {
        fprintf(stderr, "failed to malloc(%d)", BUFFER_SIZE);

        return (void*)MALLOC_ERROR;
    }

    fileName = malloc(40 * sizeof(char));

    if (recv(*((int*)client_fd), request, BUFFER_SIZE, 0) < 0) {
        fprintf(stderr, "Failed to recv()\n");
        exit(1);
    }

    if (getFirstLine(request, &requestFirstLine) == BAD_REQUEST) {
        
        getFile(&fileSize, "badrequest.html", &requestedFileContent);


        responseSize = 
                     generateHTTPResponse(400,
                                         "text/html", 
                                         requestedFileContent,
                                         fileSize,
                                         &response);
        send(
                // this casts the void pointer to an 
                // integer pointer and gets its value
                *((int*)client_fd),
                response,
                responseSize,
                0
            );

        free(requestedFileContent);
        requestedFileContent = NULL;

        free(MIMEtype);
        MIMEtype = NULL;

        free(response);
        response = NULL;

        free(requestFirstLine);
        requestFirstLine = NULL;

        printf("\n\n");
        return NULL;
    }

    printf("%s\n", requestFirstLine);

    requestMethod = getHTTPMethod(request);
    free(request);
    request = NULL;

    getFileName(requestFirstLine, requestMethod, &fileName);
    free(requestFirstLine);
    requestFirstLine = NULL;

    if (getMIMEType(fileName, &MIMEtype) == IS_A_DIR) {
        responseSize = 
                     generateHTTPResponse(404,
                                         "text/html", 
                                         "<h1>That's A Directory</h1>",
                                         27,
                                         &response);
        send(
                // this casts the void pointer to an 
                // integer pointer and gets its value
                *((int*)client_fd),
                response,
                responseSize,
                0
            );
        
        free(requestedFileContent);
        requestedFileContent = NULL;

        printf("fileContent, ");
        free(response);

        response = NULL;
        printf("response\n");
        
        printf("\n\n");
        return NULL;
    }
    
    fileStatus = getFile(&fileSize, fileName, &requestedFileContent);
    free(fileName);
    fileName = NULL;

    if (fileStatus == FILE_NOT_FOUND) {
        printf("not found\n");
        responseSize =
                     generateHTTPResponse(
                            404,
                            "text/html",
                            "<h1>FileNotFound</h1>",
                            21,
                            &response
                     );
        free(requestedFileContent);
        requestedFileContent = NULL;
        free(MIMEtype);
        MIMEtype = NULL;
    }
    else if (fileStatus == MALLOC_ERROR) {
        free(requestedFileContent);
        requestedFileContent = NULL;
        free(MIMEtype);
        MIMEtype = NULL;
        free(response);
        response = NULL;
        return (void*)MALLOC_ERROR;
    }
    else {
        responseSize = generateHTTPResponse(200, MIMEtype, requestedFileContent, fileSize, &response);
    }
    
    send(*((int*)client_fd), response, responseSize, 0);
    close(*((int*)client_fd));

    free(client_fd);
    client_fd = NULL;
    free(requestedFileContent);
    requestedFileContent = NULL;
    free(MIMEtype);
    MIMEtype = NULL;
    free(response);
    response = NULL;

    printf("\n\n");

    return NULL;
}
