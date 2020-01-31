#include "http.h"



int ServerRun(){
	int msock;//socket descripter
	struct sockaddr_in addr;//server address
	struct sockaddr_in fsin;//client address
	int fd,nfds;
	fd_set rfds;
  	fd_set afds;

	// make socket
	if (( msock = socket(AF_INET, SOCK_STREAM, 0 )) < 0 ) {
		perror("socket error");
		return -1;
	}
	// port number is 80 
	addr.sin_family = AF_INET;
	addr.sin_port = ntohs(80);
	addr.sin_addr.s_addr = INADDR_ANY;
	//bind socket
	if ( bind( msock, (struct sockaddr *)&addr, sizeof(addr)) < 0 ) {
		perror("bind error");
		return -1;
	}
	//listen
	if ( listen( msock, 1 ) < 0 ) {
		perror("listen error");
		return -1;
	}
	//clear and set to macro
	nfds = getdtablesize();//get descripter table size of your os
	FD_ZERO(&afds);//Clear FD
	FD_SET(msock,&afds);//
	
	//Always
	while ( 1 ){
      bcopy((char *)&afds,(char *)&rfds,sizeof(rfds));
	//wait for request
      if(select(nfds,&rfds,(fd_set *)0,(fd_set *)0,(struct timeval *)0)<0)
		perror("select error");

      if(FD_ISSET(msock,&rfds)){
		int sock;
		int falen = sizeof(fsin);
		sock = accept(msock,(struct sockaddr *)&fsin,&falen);

		if(sock<0)
			printf("accept");
		FD_SET(sock,&afds);
	}
      for(fd=0;fd<nfds;++fd)
		if(fd != msock && FD_ISSET(fd,&rfds)){
	 		fprintf(stderr, "fd = %d\n",fd);
	 		if(http(fd) == 0){
	 			(void)close(fd);
	  			FD_CLR(fd,&afds);
	 		}
		}
	}
}
//Analysis and make response
int http(int fd){
	int length;//length of file requested
	char request[MAX_REQUEST_SIZE];//request buffer
	FILE *fp;//file pointer of file requested
	
	char response[120];//response buffer

	char file_buff [MAX_FILE_SIZE];//send file buffer
	httpheader _header;//header structure,see http.h
	//get request
	if(read(fd,request,sizeof(request)) <= 0){
		perror("error:read requet");
		return 0;
	}else{
		request_analysis(request,&_header);
	}
	//open the file requested
	if ((fp = fopen(_header.uri + 1, "r")) == NULL) {
		send_msg(fd, "404 Not Found");
		return 0;
    	}
	//get the length of the file requested
	if ((length = get_file_size(fp->_fileno)) == -1)
		perror("error:get file size\n");
	//Analyze the content type requested
	format_analysis(response,&length,&_header);
	//send response to client or browser
	send_msg(fd,response);
	
	//send file or payload to client or browser
	fread (file_buff, sizeof(char), length + 1, fp);
    	fclose(fp);
    	write (fd, file_buff, length);
	//return 0 to close the connection
	//return 1 or higher to maintain the connection	
	return 0;
}

void request_analysis(char *request,httpheader *_header){
	char *str;
	char *ptr;//useless
	char token[64];//used to put string of http header
	
	sscanf(request, "%s %s %s", _header->method, _header->uri,_header->type);
	str = strchr(request, (int)'\n');
	str = str + 1;
	sscanf(str, "%s %s", token,_header->host);
	str = strchr(str, (int)'\n');
	str = str + 1;
	sscanf(str, "%s %s", token,_header->user_agent);
	str = strchr(str, (int)'\n');
	str = str + 1;
	sscanf(str, "%s %s", token,_header->accept);
	ptr = strtok(_header->accept,",");//this is to get Accept as "text/html"
	//***********debug http header****************
	/*	
	fprintf(stderr,"method:%s\n",_header->method);
	fprintf(stderr,"file uri:%s\n",_header->uri);
	fprintf(stderr,"http version:%s\n",_header->type);
	fprintf(stderr,"host:%s\n",_header->host);
	fprintf(stderr,"user agent:%s\n",_header->user_agent);
	fprintf(stderr,"accept:%s\n",_header->accept);
	*/
}

void format_analysis(char *response,int *length,httpheader *_header){
	char flen[30];
	
	strcpy(response,"HTTP/1.0 200 OK\r\n");
	if (strcmp(_header->accept, "image/webp") == 0) {
		strcat(response,"Content-Type: image/bmp\r\n");
	}else if(strcmp(_header->accept, "text/css") == 0) {
		strcat(response,"Content-Type: text/css\r\n");
	}else{
		strcat(response,"Content-Type: text/html\r\n");
	}
	sprintf(flen, "Content-Length: %d\r\n\r\n", *length);
	strcat(response,flen);
}

//write message
int send_msg(int fd,char *msg){
	int len;
	len = strlen(msg);

	if(write(fd,msg,len) != len){
		fprintf(stderr,"error:write");
	}
	return len;
}
//get file size
int get_file_size(int fd) {
 struct stat stat_struct;
 if (fstat(fd, &stat_struct) == -1)
 	return 1;
 return (int)stat_struct.st_size;
}
