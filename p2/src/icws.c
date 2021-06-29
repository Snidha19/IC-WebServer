#include<sys/types.h>
#include<sys/socket.h>
#include<sys/stat.h>
#include<netdb.h>
#include<stdio.h>
#include<fcntl.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<time.h>
#include "pcsa_net.h"
#include "parse.h"

#define MAXBUF 8192

typedef struct sockaddr SA;

char* setMimeType(char *uri){
    // char tmpUri[MAXBUF];
    // strcpy(tmpUri, uri);

    char *mimeType = malloc(30);
    char *extension;
    strtok(uri, ".");
    extension = strtok(NULL, " ");
    printf("extension (%s)\n", extension);

    if (strcmp(extension, "html") == 0)
    {
        strcpy(mimeType, "text/html");
    }
    if (strcmp(extension, "css") == 0)
    {
        strcpy(mimeType, "text/css");
    }
    if (strcmp(extension, "txt") == 0)
    {
        strcpy(mimeType, "text/plain");
    }
    if (strcmp(extension, "js") == 0)
    {
        strcpy(mimeType, "text/javascript");
    }
    if (strcmp(extension, "jpg") == 0)
    {
        strcpy(mimeType, "image/jpeg");
    }
    if (strcmp(extension, "png") == 0)
    {
        strcpy(mimeType, "image/png");
    }
    if (strcmp(extension, "gif") == 0)
    {
        strcpy(mimeType, "image/gif");
    }
    return mimeType ;
}

char* setFilePath(char *root, char *uri)
{
    char* filePath = malloc(100);
    sprintf(filePath, "%s%s", root, uri);
    printf("path (%s)\n", filePath);
    return filePath;
}

void respond_HEAD(int connFd, char *root, char *uri){
    char buf[MAXBUF];
    char *mimeType = setMimeType(uri);
    char *filePath = setFilePath(root, uri);
    struct stat sb;

    printf("uri (%s)\n", uri);
    // strcp(mimeType, setMimeType(uri));
    // mimeType = setMimeType(uri, mimeType);
    printf("mimeType (%s)\n", mimeType);

    // Get current time for the HTTP header
   time_t t1 = time(NULL);
   struct tm *ltime = localtime(&t1);
    
    // sprintf(newBuf, "%s%s", filePath, uri);
    // printf("path (%s)\n",newBuf);
    // strcp(filePath, setFilePath(root, uri));
    int inputFd = open(filePath, O_RDONLY);
    if (inputFd == -1)
    {
        printf("404 File not found\n");
    }

    fstat(inputFd, &sb);
    size_t sizeOfFile = sb.st_size;
    printf("%lu\n",sizeOfFile);

    sprintf(buf,
            "HTTP/1.1 200 OK\r\n"
            "Date: %s"
            "Server: ICWS\r\n"
            "Connection: close\r\n"
            "Content-length: %lu\r\n"
            "Content-type: %s\r\n\r\n",
            asctime(ltime), sizeOfFile, mimeType);
    printf("buf (%s)\n", buf);
    write_all(connFd, buf, strlen(buf));

    close(inputFd);
    free(mimeType);
}

void respond_GET(int connFd, char *root, char *uri)
{
    char buf[MAXBUF];
    char *mimeType = setMimeType(uri);
    char *filePath = setFilePath(root, uri);
    struct stat sb;
    
    printf("uri (%s)\n",uri);
    // strcp(mimeType,setMimeType(uri));
    printf("mimeType (%s)\n",mimeType);

    // Get current time for the HTTP header
   time_t t1 = time(NULL);
   struct tm *ltime = localtime(&t1);

    // char filePath[MAXBUF];
    // sprintf(newBuf, "%s%s", filePath, uri);
    // printf("path (%s)\n",newBuf);
    // strcp(filePath, setFilePath(root, uri));
    int inputFd = open(filePath, O_RDONLY);
    if (inputFd == -1)
    {
        printf("404 File not found\n");
    }
    
    fstat(inputFd, &sb);
    size_t sizeOfFile = sb.st_size;
    printf("%lu\n",sizeOfFile);
    
    sprintf(buf,
            "HTTP/1.1 200 OK\r\n"
            "Date: %s"
            "Server: ICWS\r\n"
            "Connection: close\r\n"
            "Content-length: %lu\r\n"
            "Content-type: %s\r\n\r\n",
            asctime(ltime), sizeOfFile, mimeType);
    printf("buf (%s)\n", buf);
    write_all(connFd, buf, strlen(buf));

    char content[MAXBUF];
    ssize_t numRead;
    while ((numRead = read(inputFd, content, MAXBUF)) > 0)
    {
        write_all(connFd, content, numRead);
    }
    close(inputFd);
}

void serve_http(int connFd, char *root)
{
    char buf[MAXBUF], line[MAXBUF];
    int readPerLine = 0, bufSize = 0;
    printf("path (%s)\n", root);

    while ((readPerLine = read_line(connFd, line, MAXBUF))> 0)
    {
        bufSize += readPerLine;
        // printf(" bufSIZE : %d\n", bufSize);
        strcat(buf, line);
        if (!strcmp(line, "\r\n"))
            break;
    }

    if(!readPerLine)
        return ;

    printf("buf (%s)\n", buf);
    printf("bufSize : %d\n", bufSize);

    Request *request = parse(buf, bufSize, connFd);

    if(strcasecmp(request->http_version, "HTTP/1.1") != 0){
        printf("505: Bad Version Number");
    }

    if (strcasecmp(request->http_method, "GET") == 0 &&
        request->http_uri[0] == '/')
    {
        printf("LOG: GET \n");
        respond_GET(connFd, root, request->http_uri);
    }
    else if (strcasecmp(request->http_method, "HEAD") == 0 &&
        request->http_uri[0] == '/')
    {
        printf("LOG: HEAD \n");
        respond_HEAD(connFd, root, request->http_uri);
    }
    else
    {
        printf("501: Unsupported Methods\n");
    }
}

int main(int argc, char* argv[]) {
    //cmd line arg
    /* ./icws --port <listenPort> --root <wwwRoot> */

    // flags to check if port and root exist
    int portFlag = 0,rootFlag = 0;

    if (strcmp("--port", argv[1]) == 0){
        portFlag = 1;
    }
    // if (strcmp("--root", argv[3]) == 0){
    //     rootFlag = 1;
    // }

    if(portFlag != 0){
        int listenFd = open_listenfd(argv[2]);
        for (;;) {
            struct sockaddr_storage clientAddr;
            socklen_t clientLen = sizeof(struct sockaddr_storage);

            int connFd = accept(listenFd, (SA *)&clientAddr, &clientLen);
            if (connFd < 0)
            {
                fprintf(stderr, "Failed to accept\n");
                continue;
            }

            char hostBuf[MAXBUF], svcBuf[MAXBUF];
            if (getnameinfo((SA *)&clientAddr, clientLen,
                            hostBuf, MAXBUF, svcBuf, MAXBUF, 0) == 0)
                printf("Connection from %s:%s\n", hostBuf, svcBuf);
            else
                printf("Connection from ?UNKNOWN?\n");
            
            serve_http(connFd, argv[4]);
            close(connFd);
        }

    }
    
    return 0;
}