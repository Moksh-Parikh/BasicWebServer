#ifndef SERVER_FUNCTIONS
#define SERVER_FUNCTIONS

#include <arpa/inet.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <pthread.h>
#include <regex.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>


#define PORT 300
#define BUFFER_SIZE 104857600
#define NUMBER_OF_METHODS 2

#define METHOD_GET     1
#define METHOD_HEAD    2
#define METHOD_POST    3
#define IS_A_DIR       4
#define MALLOC_ERROR   5
#define FILE_NOT_FOUND 6
#define BAD_REQUEST    7



void* handleClient(void* client_fd);
int getFirstLine(char* longString, char** splitString);
int getHTTPMethod(char *request);

#endif
