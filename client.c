#include "client.h"
#define MAX_MSG_SIZE 1024

struct param {
    int sockfd;
    char* folder;
    char* subRequest;
    int PORT;
};

int child(void* arg);

/*
 * The function get_sockaddr converts the server's address and port into a form usable to create a
 * scoket
*/
struct addrinfo* get_sockaddr(const char* hostname, const char *port)
{

    struct addrinfo hints;
    struct addrinfo* results;
    int rv;

    memset(&hints, 0, sizeof(struct addrinfo));

    hints.ai_family = AF_INET;          //Return socket address for the server's IPv4 addresses
    hints.ai_socktype = SOCK_STREAM;    //Return TCP socket addresses

    /* Use getaddrinfo will get address information for the host specified by hostnae */

    rv = getaddrinfo(hostname, port, &hints, &results);
    if (rv != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        exit(1);
    }

    return results;
}

/*
 * The function open_connection establishes a connection to the server
*/
int open_connection(struct addrinfo* addr_list)
{

    struct addrinfo* p;
    int sockfd;
    //Iterate through each addr info in the list; Stop when we successully connect to one

    for (p = addr_list; p != NULL; p = p->ai_next) {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);

        // Try the next address since the socket operation failed
        if (sockfd == -1) continue;

        //Stop iterating of we are able to connect to the server

        if (connect(sockfd,p->ai_addr, p->ai_addrlen) != -1) break;
    }

    freeaddrinfo(addr_list);

    if (p == NULL)
        err(EXIT_FAILURE, "%s", "Unable to connect");
    else
        return sockfd;

}

int getmsg(int socketfd, char* path, int PORT)
{

    char recvBuff[1024];
    recv(socketfd, recvBuff, sizeof(recvBuff), 0);
    printf("%s\n",recvBuff);

    char secPart[1024]="",returnString[1024] = "", bla[1024]="";
    char* cutTrash, *remain;
    // char* deli = " ";
    remain = strtok(recvBuff,"\r\n");
    remain = strtok(NULL,"\r\n");
    strcpy(bla,remain);
    cutTrash = strtok(NULL," ");
    cutTrash = strtok(NULL," ");
    if(strcmp(cutTrash,"directory")==0) {
        remain = strtok(bla, "\r\n");
        strcpy(secPart,remain);
        // remain = strstr(recvBuff, "\r\n\r\n");
// printf("%s!!\n",secPart);
        remain = strtok(secPart, " ");
        // printf("%s++\n",remain);
        struct param parameter = {socketfd, path, remain, PORT};
        pthread_t t; // pthread variable
        pthread_create(&t, NULL, child, &parameter); // new thread

        pthread_join(t, NULL); // 等待子執行緒執行完成
        while(remain != NULL) {
            remain = strtok(NULL, " ");
            if(remain == NULL)break;
            // printf("%s++\n",remain);

            struct param parameter = {socketfd, path, remain, PORT};

            // printf("%d\n",parameter.sockfd);
            pthread_t t; // pthread variable
            pthread_create(&t, NULL, child, &parameter); // new thread

            pthread_join(t, NULL); // 等待子執行緒執行完成
        }
    }
}

int child(void* arg)
{
    // struct param *parameter = (struct param *) arg;
    // char sendBuff[1024]="",recvBuff[1024]="", request[1024]="";
    // strcpy(request,parameter->folder);
    // strcat(request,"/");
    // strcat(request,parameter->subRequest);
    // sprintf(sendBuff, "GET %s HTTP/1.x\r\nHOST: 127.0.0.1:%d \r\n\r\n",request,parameter->PORT);
    // // printf("~~\n%d\n~~\n",parameter->PORT);
    // struct addrinfo* results = get_sockaddr("127.0.0.1", parameter->PORT);
    // int sockfd = open_connection(results);
    // write(sockfd, sendBuff, strlen(sendBuff));

    // int numbytes = recv(sockfd,recvBuff,sizeof(recvBuff)-1,0);
    // if (numbytes == -1) {
    //     perror("recv");
    //     exit(1);
    // }

    // // recvBuff[numbytes] = '\0';
    // printf("%s\n", recvBuff);
    // pthread_exit(NULL);

    // char *filepath = (char*) name;
    struct param *parameter = (struct param *) arg;
    char sendBuff[200];
    char recvBuff[1024];
    char request[1024]="";
    strcpy(request,parameter->folder);
    strcat(request,"/");
    strcat(request,parameter->subRequest);

    struct sockaddr_in addr;

    int socketfd = socket(PF_INET, SOCK_STREAM, 0);

    addr.sin_family = AF_INET;
    addr.sin_port = htons(parameter->PORT);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    memset(addr.sin_zero, '\0', sizeof(addr.sin_zero));

    connect(socketfd, (struct sockaddr*)&addr, sizeof(addr));

    memset(recvBuff, '\0',sizeof(recvBuff));
    memset(sendBuff, '\0', sizeof(sendBuff));

    sprintf(sendBuff, "GET %s HTTP/1.x\r\nHOST: 127.0.0.1:%d \r\n\r\n",request,parameter->PORT);
    send(socketfd, sendBuff, strlen(sendBuff), 0);

    char sendPath[300];
    memset(sendPath,'\0',sizeof(sendPath));

    // getmsg(socketfd, request, parameter->PORT);
    recv(socketfd, recvBuff, sizeof(recvBuff), 0);
    printf("%s\n",recvBuff);

    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    int numbytes;
    char sendBuff[MAX_MSG_SIZE];
    char recvBuff[MAX_MSG_SIZE];
    struct sockaddr_in serv_addr;
    struct hostent *server;
    int rv;
    int sum;

    int PORT = atoi(argv[6]);
    char* ibuffer = argv[2];
    char QUERY_FILE_OR_DIR[1025] = {""};
    strcpy(QUERY_FILE_OR_DIR,ibuffer);
    // printf("%d::%s\n", PORT, QUERY_FILE_OR_DIR);

    // if(argc != 4)
    // {
    //     printf("\n Usage: %s hostname port number_to_be_multiplied\n",argv[0]);
    //     return 1;
    // }

    struct addrinfo* results = get_sockaddr(argv[4], argv[6]);
    int sockfd = open_connection(results);

    memset(sendBuff, '0',sizeof(sendBuff));
    memset(recvBuff, '0',sizeof(recvBuff));

    // snprintf(sendBuff, sizeof(sendBuff), "%d", 1024);
    sprintf(sendBuff, "GET %s HTTP/1.x\r\nHOST: 127.0.0.1:%d \r\n\r\n",QUERY_FILE_OR_DIR,PORT);
    write(sockfd, sendBuff, strlen(sendBuff));

    numbytes = recv(sockfd,recvBuff,sizeof(recvBuff)-1,0);
    if (numbytes == -1) {
        perror("recv");
        exit(1);
    }

    recvBuff[numbytes] = '\0';
    printf("%s\n", recvBuff);

//"HTTP/1.x 200 OK\nContent-type: directory\nServer: httpserver/1.x\n\n"
    char secPart[1024]="",returnString[1024] = "", bla[1024]="";
    char* cutTrash, *remain;
    // char* deli = " ";
    remain = strtok(recvBuff,"\r\n");
    remain = strtok(NULL,"\r\n");
    strcpy(bla,remain);
    cutTrash = strtok(NULL," ");
    cutTrash = strtok(NULL," ");
    if(strcmp(cutTrash,"directory")==0) {
        remain = strtok(bla, "\r\n");
        strcpy(secPart,remain);

        // while(remain != NULL) {
        //     // printf("%s\n",remain);
        //     if(strcmp(remain,"Server: httpserver/1.x")==0) {
        //         remain = strtok(NULL, "\r\n");
        //         strcpy(secPart,remain);
        //         break;
        //     }
        //     remain = strtok(NULL, "\r\n");
        // }
        // remain = strstr(recvBuff, "\r\n\r\n");
// printf("%s!!\n",secPart);
        remain = strtok(secPart, " ");
        // printf("%s++\n",remain);
        struct param parameter = {sockfd, QUERY_FILE_OR_DIR, remain, PORT};
        pthread_t t; // pthread variable
        pthread_create(&t, NULL, child, &parameter); // new thread

        pthread_join(t, NULL); // 等待子執行緒執行完成
        while(remain != NULL) {
            remain = strtok(NULL, " ");
            if(remain == NULL)break;
            // printf("%s++\n",remain);

            struct param parameter = {sockfd, QUERY_FILE_OR_DIR, remain, PORT};

            // printf("%d\n",parameter.sockfd);
            pthread_t t; // pthread variable
            pthread_create(&t, NULL, child, &parameter); // new thread

            pthread_join(t, NULL); // 等待子執行緒執行完成
        }
    }
    return 0;
}