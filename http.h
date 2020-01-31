#ifndef HTTP_H
#define HTTP_H

#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>//perror

#include <sys/stat.h>//to get file size
#include <fcntl.h>

#define MAX_REQUEST_SIZE 1024
#define MAX_FILE_SIZE 100000

#define RUN(); ServerRun();

typedef struct
{
	char user_agent[60];
	char method[60];
	char uri[60];
	char type[60];
	char host[60];
	char connection[60];
	char cache_control[60];
	char accept_language[60];
	char accept_encoding[60];
	char accept[60];
} httpheader;


int ServerRun();
int send_msg(int fd,char *msg);
int http(int fd);
int get_file_size(int fd);
void request_analysis(char *request,httpheader *_header);
void format_analysis(char *response,int *length,httpheader *_header);

#endif
