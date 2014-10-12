#include "sshttp.h"

static char buff[SSHTTP_BUFF_SIZE]={0};

static int parse_responce(char* responce)
{
    char code[4];
    char* s;
    size_t bp=0;
    
    code[0]=buff[9];
    code[1]=buff[10];
    code[2]=buff[11];
    code[3]='\0';
    
    s=strstr(buff,"\r\n\r\n");
    if(s==NULL)
    {
        return -6;
    }
    s+=4;
    
    if(responce!=NULL)
    {
        strcpy(responce,s);
    }
    
    return atoi(code);
}

int sshttp_request
(
    const char* host,
    const char* target,
    int method,
    const char* header[],
    int header_num,
    const char* body,
    size_t body_len,
    char* responce
)
{
    struct addrinfo* ai;
	int fd;
    int i;
    size_t bp=0;
    const char* s;
    struct pollfd pfd;
	
    errno=0;
    
    /*-----open socket-----*/
	if(getaddrinfo(host,"http",NULL,&ai)!=0)
	{
		perror("[sshttp]Error: getaddrinfo()");
        return -1;
	}

	fd=socket(ai->ai_family,ai->ai_socktype,ai->ai_protocol);
	if(fd<0)
	{
		freeaddrinfo(ai);
        perror("[sshttp]Error: socket()");
		return -2;
	}
    
	if(connect(fd,ai->ai_addr,ai->ai_addrlen)==-1)
	{
		freeaddrinfo(ai);
        perror("[sshttp]Error: connect()");
		return -3;
	}
    
    /*-----build request-----*/
    memset(buff,0,SSHTTP_BUFF_SIZE);
    
    switch(method)
    {
        case GET:
            buff[bp++]='G';
            buff[bp++]='E';
            buff[bp++]='T';
            break;
        case PUT:
            buff[bp++]='P';
            buff[bp++]='U';
            buff[bp++]='T';
            break;
        case POST:
            buff[bp++]='P';
            buff[bp++]='O';
            buff[bp++]='S';
            buff[bp++]='T';
            break;
        case DELETE:
            buff[bp++]='D';
            buff[bp++]='E';
            buff[bp++]='L';
            buff[bp++]='E';
            buff[bp++]='T';
            buff[bp++]='E';
            break;
    }
    
    buff[bp++]=' ';
    
    strcpy(buff+bp,target);
    bp+=strlen(target);
    
    buff[bp++]=' ';
    
    strcpy(buff+bp,"HTTP/1.1");
    bp+=8;
    
    buff[bp++]='\r';
    buff[bp++]='\n';
    
    strcpy(buff+bp,"Host:");
    bp+=5;
    
    strcpy(buff+bp,host);
    bp+=strlen(host);
    
    buff[bp++]='\r';
    buff[bp++]='\n';
    
    for(i=0;header!=NULL&&i<header_num;++i)
    {
        strcpy(buff+bp,header[i]);
        bp+=strlen(header[i]);
        
        buff[bp++]='\r';
        buff[bp++]='\n';
    }
    
    if(body_len>0)
    {
        s="Content-Length";
        strcpy(buff+bp,s);
        bp+=strlen(s);

        buff[bp++]=':';
        bp+=sprintf(buff+bp,"%ld",body_len);
        
        buff[bp++]='\r';
        buff[bp++]='\n';
    }
    
    buff[bp++]='\r'; //blank line
    buff[bp++]='\n';
    
    if(body!=NULL&&body_len>0)
    {
        memcpy(buff+bp,body,body_len);
        bp+=body_len;
    }
    
    /*-----send request-----*/
    if(send(fd,buff,bp,0)==-1)
    {
        freeaddrinfo(ai);
        close(fd);
        perror("[sshttp]Error: send()");
        return -4;
    }
    
    /*-----wait responce-----*/
    memset(buff,0,bp);
    
    pfd.fd=fd;
    pfd.events=POLLIN;
    pfd.revents=0;
    
    if(poll(&pfd,1,TIMEOUT*1000)<=0)
    {
        freeaddrinfo(ai);
        close(fd);
        perror("[sshttp]Error: poll()");
        return -5;
    }
    
    bp=recv(fd,buff,SSHTTP_BUFF_SIZE,0);
    if(bp<=0)
    {
        freeaddrinfo(ai);
        close(fd);
        perror("[sshttp]Error: recv()");
        return -6;
    }
    buff[bp]='\0';
    
    freeaddrinfo(ai);
    close(fd);
    
    return parse_responce(responce);
}
