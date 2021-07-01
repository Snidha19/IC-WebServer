#include<sys/types.h>
#include <ctype.h>
#include<sys/socket.h>
#include<sys/stat.h>
#include<netdb.h>
#include<stdio.h>
#include<fcntl.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<time.h>
#include<pthread.h>
#include<stdbool.h>
#include "pcsa_net.h"
#include "parse.h"
#include "myqueue.h"

#define MAXBUF 8192

#define DEFAULT_MIME_TYPE "application/octet-stream"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_p = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t conditional_var = PTHREAD_COND_INITIALIZER;

typedef struct sockaddr SA;

char *strlower(char *s)
{
    for (char *p = s; *p != '\0'; p++) {
        *p = tolower(*p);
    }

    return s;
}

char* setMimeType(char *uri){

    char *extension = strrchr(uri , '.');

    if (extension == NULL) {
        return DEFAULT_MIME_TYPE;
    }
    extension++;

    strlower(extension);

    printf("extension (%s)\n", extension);

    if (strcmp(extension, "html") == 0)
    {
        return "text/html";
    }
    if (strcmp(extension, "css") == 0)
    {
        return "text/css";
    }
    if (strcmp(extension, "txt") == 0)
    {
        return "text/plain";
    }
    if (strcmp(extension, "js") == 0)
    {
        return "text/javascript";
    }
    if (strcmp(extension, "jpg") == 0)
    {
        return "image/jpeg";
    }
    if (strcmp(extension, "png") == 0)
    {
        return "image/png";
    }
    if (strcmp(extension, "gif") == 0)
    {
        return "image/gif";
    }
    return DEFAULT_MIME_TYPE;
}

char* setFilePath(char *root, char *uri)
{
    char* filePath = malloc(100);
    sprintf(filePath, "%s%s", root, uri);
    printf("path (%s)\n", filePath);
    return filePath;
}

struct tm* setLastModified(char *uri){
    struct stat attr;
    struct tm *lmtime;
    stat(uri, &attr);
    lmtime = ctime(&(attr.st_mtime));

    return lmtime;
}

void respond_HEAD(int connFd, char *root, char *uri){
    char buf[MAXBUF];
    char *mimeType = setMimeType(uri);
    char *filePath = setFilePath(root, uri);
    struct stat sb;

    printf("uri (%s)\n", uri);
    printf("mimeType (%s)\n", mimeType);

    // Get current time for the HTTP header
   time_t t1 = time(NULL);
   struct tm *ltime = localtime(&t1);

//    struct stat attr;
//    struct tm *lmtime;
//    stat(uri, &attr);
//    lmtime = ctime(&(attr.st_mtime));

    struct tm *lmtime = setLastModified(uri);
    int inputFd = open(filePath, O_RDONLY);
    if (inputFd == -1)
    {
        printf("404 File not found\n");
        return;
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
            "Content-type: %s\r\n"
            "Last-Modified: %s\r\n",
            asctime(ltime), sizeOfFile, mimeType, lmtime);
    printf("buf (%s)\n", buf);
    write_all(connFd, buf, strlen(buf));

    close(inputFd);
}

void respond_GET(int connFd, char *root, char *uri)
{
    char buf[MAXBUF];
    char *mimeType = setMimeType(uri);
    char *filePath = setFilePath(root, uri);
    struct stat sb;
    
    printf("uri (%s)\n",uri);
    printf("mimeType (%s)\n",mimeType);

    // Get current time for the HTTP header
   time_t t1 = time(NULL);
   struct tm *ltime = localtime(&t1);

   struct tm *lmtime = setLastModified(uri);

    int inputFd = open(filePath, O_RDONLY);
    if (inputFd == -1)
    {
        printf("404 File not found\n");
        return;
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
            "Content-type: %s\r\n\r\n"
            "Last-Modified: %s\r\n",
            asctime(ltime), sizeOfFile, mimeType,lmtime);
    printf("buf (%s)\n", buf);
    write_all(connFd, buf, strlen(buf));

    char content[MAXBUF];
    ssize_t numRead;
    while ((numRead = read(inputFd, content, MAXBUF)) > 0)
    {
        write_all(connFd, content, MAXBUF);
    }
    close(inputFd);
}

void serve_http(int connFd, char *root)
{
    char buf[MAXBUF], line[MAXBUF];
    int bufSize = 0, bytesRead=0;
    printf("path (%s)\n", root);

    while ((bytesRead = read_line(connFd, line, MAXBUF)) != 0)
    {
        printf("byteReadPerTime: %d\n" , bytesRead);
        bufSize += bytesRead;
        strcat(buf,line);
        if (!strcmp(line, "\r\n"))
            break;
    }

    if(!bytesRead)
        return ;

    printf("buf (%s)\n", buf);
    printf("bufSize : %d\n", bufSize);
    printf("bytesRead : %d\n", bytesRead);

    Request *request = parse(buf, bufSize, connFd);
    strcpy(buf,"");

    if(strcasecmp(request->http_version, "HTTP/1.1") != 0)
    {
        printf("505: Bad Version Number");
        return;
    }
    else if (strcasecmp(request->http_method, "GET") == 0 &&
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

void serve_http2(int *p_connFd, char  *root)
{
    int connFd = *p_connFd;
    printf("connFD : %d\n" , connFd);
    char buf[MAXBUF], line[MAXBUF];
    int bufSize = 0, bytesRead=0;
    printf("path (%s)\n", root);

    while ((bytesRead = read_line(connFd, line, MAXBUF)) > 0)
    {
        printf("byteReadPerTime: %d\n" , bytesRead);
        bufSize += bytesRead;
        strcat(buf,line);
        if (!strcmp(line, "\r\n"))
            break;
    }

    if(!bytesRead)
        return ;

    //bytesRead = read(connFd, line, MAXBUF);

    printf("buf (%s)\n", buf);
    printf("bufSize : %d\n", bufSize);
    printf("bytesRead : %d\n", bytesRead);

    pthread_mutex_lock(&mutex_p);
    Request *request = parse(buf, bufSize, connFd);
    pthread_mutex_unlock(&mutex_p);
    strcpy(buf,"");

    if(strcasecmp(request->http_version, "HTTP/1.1") != 0)
    {
        printf("505: Bad Version Number");
        return ;
    }
    else if (strcasecmp(request->http_method, "GET") == 0 &&
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

void* thread_function(void* arg){
    while (true) {
        t_arg *args;
        pthread_mutex_lock(&mutex);
        pthread_cond_wait(&conditional_var,&mutex);
        args = dequeue();
        pthread_mutex_unlock(&mutex);
        if (args->connFd != NULL){
            serve_http2(args->connFd, args->root);
        }
    }

}


int main(int argc, char* argv[]) {
    printf("argc: %d\n", argc);
    //cmd line arg
    /* ./icws --port <listenPort> --root <wwwRoot> */

    // flags to check if port and root exist
    int portFlag = 0,rootFlag = 0,threadFlag=0;

    if (strcmp("--port", argv[1]) == 0){
        portFlag = 1;
    }
    // if (strcmp("--root", argv[3]) == 0){
    //     rootFlag = 1;
    // }
    

    if (argc > 5){
        if (strcmp("--numThreads", argv[5]) == 0){
        threadFlag = 1;
        }
        if(threadFlag != 0){
            int num = atoi(argv[6]);
            //printf(" num threads : %d\n", num);
            pthread_t thread_pool[num];
            for (int i=0; i<num; i++){
                pthread_create(&thread_pool[i], NULL, thread_function, NULL);
            }
        }
    }
    
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
            
            
            
            
            //creating threads 
            // pthread_t t;
            if(threadFlag != 0){
            int *pclient = malloc(sizeof(int));
            *pclient = connFd;
            pthread_mutex_lock(&mutex);
            enqueue(pclient, argv[4]);
            pthread_cond_signal(&conditional_var);
            pthread_mutex_unlock(&mutex);
            }else{
                serve_http(connFd, argv[4]);
            }
            
            // serve_http(connFd, argv[4]);

            close(connFd);
        }

    }
    if (threadFlag != 0){
    pthread_mutex_destroy(&mutex);
    pthread_mutex_destroy(&mutex_p);
    }
    return 0;
}