#include "server.h"

#define MAX_MSG_SIZE 1024

// extn extensions[] = {
// 	{"htm", "text/html"},
// 	{"html", "text/html"},
// 	{"css", "text/css"},
// 	{"h", "text/x-h"},
// 	{"hh", "text/x-h"},
// 	{"c", "text/x-c"},
// 	{"cc", "text/x-c"},
// 	{"json", "application/json"},
// 	{0, 0},
// };

enum {
    OK = 0,
    BAD_REQUEST,
    NOT_FOUND,
    METHOD_NOT_ALLOWED,
    UNSUPPORT_MEDIA_TYPE
};

const int status_code[] = {
    200,
    400,
    404,
    405,
    415,
};

/*Declare global variables*/
pthread_mutex_t mutex;
pthread_cond_t cond;
queue* q;
char processRoot[1025] = {""};


/*
 * This function receives receives one number from the client and multiplies
 * it by 10 and sends it back
 */
void doProcessing (int connfd)
{
    char recvBuff[MAX_MSG_SIZE]="", sendBuff[MAX_MSG_SIZE]="";

    // memset(recvBuff, '0', sizeof(recvBuff));
    // memset(sendBuff, '0', sizeof(sendBuff));

    /*receive data from the client*/
    int numbytes = recv(connfd,recvBuff, sizeof(recvBuff)-1,0);
    if (numbytes == -1) {
        perror("recv");
        exit(1);
    }
    recvBuff[numbytes] = '\0';

    // char method[32]="", trash[64]="", host[32]="", QUERY_FILE_OR_DIR[2048]="";
    char *method="", *trash="", *host="", *QUERY_FILE_OR_DIR="", *directory = "",*extn = "", *contentType= "";
    int PORT = 0;
    // printf("%d\n",strlen(recvBuff));
    // sscanf(recvBuff,"GET %s HTTP/1.x\r\nHOST: 127.0.0.1:%d \r\n\r\n",QUERY_FILE_OR_DIR,&PORT);
    // sscanf(recvBuff,"%s %s %s: %s:%d \r\n\r\n",method,QUERY_FILE_OR_DIR,trash,host,&PORT);
    char* deli = " ", *deli2 = ":", *deli3 = ".";
    method = strtok(recvBuff, deli);
    QUERY_FILE_OR_DIR = strtok(NULL, deli);
    trash = strtok(NULL, deli);
    host = strtok(NULL, deli);
    host = strtok(host, deli2);
    PORT = atoi(strtok(NULL, deli2));
    // printf("method:%s\nQUERY_FILE_OR_DIR:%s\ntrash:%s\nhost:%s\nPORT:%d\n",method,QUERY_FILE_OR_DIR,trash,host,PORT);
    char temp[150]="";
    strcpy(temp,QUERY_FILE_OR_DIR);
    directory = strtok(temp, deli3);
    extn = strtok(NULL, deli3);
    contentType = "bla";
    // printf("extn::%s::contentType:::%s\n",extn,contentType);
    if(extn == NULL) {
        extn = "";
    }
    if(strcmp(extn, "")==0) {
        contentType = "directory";
    } else if(strcmp(extn, "htm")==0||strcmp(extn, "html")==0) {
        contentType = "text/html";
    } else if(strcmp(extn, "css")==0) {
        contentType = "text/css";
    } else if(strcmp(extn, "h")==0||strcmp(extn, "hh")==0) {
        contentType = "text/x-h";
    } else if(strcmp(extn, "c")==0||strcmp(extn, "cc")==0) {
        contentType = "text/x-c";
    } else if(strcmp(extn, "json")==0) {
        contentType = "application/json";
    } else {
        contentType = "falseType";
    }
    // printf("extn::%s::contentType:::%s\n",extn,contentType);


    if(QUERY_FILE_OR_DIR[0]!='/') {
        sprintf(sendBuff,"HTTP/1.x 400 BAD_REQUEST\r\nContent-type:\r\nServer: httpserver/1.x\r\n\r\n");
    } else if(strcmp(method, "GET")!=0) {
        sprintf(sendBuff,"HTTP/1.x 405 \r\nContent-type:\r\nServer: httpserver/1.x\r\n\r\n");
        //sprintf(sendBuff,"HTTP/1.x 405 METHOD_NOT_ALLOWED\nContent-type: %s\nServer: httpserver/1.x", contentType);
    } else if(strcmp(contentType, "falseType")==0) {
        sprintf(sendBuff,"HTTP/1.x 415 UNSUPPORT_MEDIA_TYPE\r\nContent-type:\r\nServer: httpserver/1.x\r\n\r\n");
    } else {
        // printf("!!cT%s\n",contentType);
        char file_path[2048] = "";
        strcat(file_path, processRoot);
        strcat(file_path, QUERY_FILE_OR_DIR);
        // printfile(file_path);

        if(strcmp(extn, "")==0) { //directory
            struct dirent *de;  // Pointer for directory entry

            // opendir() returns a pointer of DIR type.
            DIR *dr = opendir(file_path);

            if (dr == NULL) { // opendir returns NULL if couldn't open directory
                printf("Could not open current directory" );
            } else {
                sprintf(sendBuff,"HTTP/1.x 200 OK\r\nContent-type: directory\r\nServer: httpserver/1.x\r\n\r\n");
                while ((de = readdir(dr)) != NULL) {
                    if(strcmp(de->d_name,".")==0||strcmp(de->d_name,"..")==0)continue;
                    strcat(sendBuff, de->d_name);

                    strcat(sendBuff, " ");
                    // printf("%s\n",sendBuff);
                }

                strcat(sendBuff,"\n");
                closedir(dr);
            }
        } else if(strcmp(extn, "")!=0) {
            FILE *fp;
            char str[1024];

            /* opening file for reading */
            fp = fopen(file_path, "r");
            if(fp == NULL) {
                // perror("Error opening file");
                // return(-1);
                sprintf(sendBuff,"HTTP/1.x 404 NOT_FOUND\r\nContent-type:\r\nServer: httpserver/1.x\r\n\r\n",contentType);
            } else {
                sprintf(sendBuff,"HTTP/1.x 200 OK\r\nContent-type: %s\r\nServer: httpserver/1.x\r\n\r\n",contentType);
                while( fgets (str, 1024, fp)!=NULL ) {
                    /* writing content to stdout */
                    strcat(sendBuff, str);
                    // printf("%s",str);
                }
                fclose(fp);
            }
        } else {
            sprintf(sendBuff,"WRONG_INPUT\r\n",contentType);
        }
    }

    printf("%s",sendBuff);

    send(connfd, sendBuff, strlen(sendBuff),0);

    close(connfd);
}

/*
 * This method locks down the connection queue then utilizes the queue.h push function
 * to add a connection to the queue. Then the mutex is unlocked and cond_signal is set
 * to alarm threads in cond_wait that a connection as arrived for reading
 */
void queue_add(int value)
{
    /*Locks the mutex*/
    pthread_mutex_lock(&mutex);

    push(q, value);

    /*Unlocks the mutex*/
    pthread_mutex_unlock(&mutex);

    /* Signal waiting threads */
    pthread_cond_signal(&cond);
}

/*
 * This method locks down the connection queue then utilizes pthread_cond_wait() and waits
 * for a signal to indicate that there is an element in the queue. Then it proceeds to pop the
 * connection off the queue and return it
 */
int queue_get()
{
    /*Locks the mutex*/
    pthread_mutex_lock(&mutex);

    /*Wait for element to become available*/
    while(empty(q) == 1) {
        //printf("Thread %lu: \tWaiting for Connection\n", pthread_self());
        if(pthread_cond_wait(&cond, &mutex) != 0) {
            perror("Cond Wait Error");
        }
    }

    /*We got an element, pass it back and unblock*/
    int val = peek(q);
    pop(q);

    /*Unlocks the mutex*/
    pthread_mutex_unlock(&mutex);

    return val;
}

static void* connectionHandler()
{
    int connfd = 0;

    /*Wait until tasks is available*/
    while(1) {
        connfd = queue_get();
        //printf("Handler %lu: \tProcessing\n", pthread_self());
        /*Execute*/
        doProcessing(connfd);
    }
}

int main(int argc, char *argv[])
{
    /*Command line argument: port number*/
    // if(argc != 4)
    // {
    //         printf("\n Usage: %s port number_of_threads size_of_connections_array\n",argv[0]);
    //         return 1;
    // }

    q = createQueue(1024);

    int listenfd = 0, connfd = 0;
    struct sockaddr_in serv_addr;
    int rv;

    // get processRoot
    char* ibuffer = argv[2];
    strcpy(processRoot,ibuffer);

    /*Initialize the mutex global variable*/
    pthread_mutex_init(&mutex,NULL);

    /*Declare the thread pool array*/
    pthread_t threadPool[atoi(argv[6])];

    /*Socket creation and binding*/
    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    if (listenfd <  0) {
        perror("Error in socket creation");
        exit(1);
    }

    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[4]));

    rv = bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

    if (rv <  0) {
        perror("Error in binding");
        exit(1);
    }

    /*Make Thread Pool*/
    for(int i = 0; i < atoi(argv[6]); i++) {
        pthread_create(&threadPool[i], NULL, connectionHandler, (void *) NULL);
    }

    listen(listenfd, 10);

    /*Accept connection and push them onto the stack*/
    while(1) {
        //printf("\nMain: \t\t\t\tAccepting Connections\n");

        /*The accept call blocks until a connection is found
            * then the connection is pushed onto the queue by queue_add*/
        queue_add(accept(listenfd, (struct sockaddr*)NULL, NULL));

        //printf("Main: \t\t\t\tConnection Completed\n\n");
    }
}