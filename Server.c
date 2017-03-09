#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <dirent.h>

/*
*	Yin-Li (Emily) Chow		10103742		T01
* 	Yue Chen				10065082		T03
* 
* 	Note: base socket code provided by the professor, Pavol Federl
*/

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
    //character buffers
    char hash_message[128];
    char plan_message[128];
    char ack[1024];
    char iv[128];
    char key[32];

    //socket comunication information
    struct sockaddr_in serv_addr, cli_addr;
    int sockfd, newsockfd, portno;      
    socklen_t clilen;    
    
    //
    int n;

    //File IO
    FILE *file;
    size_t readsize;

    if (argc != 2) {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }else{
        strcpy(key, argv[2]);
    }
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if (bind(sockfd, (struct sockaddr *) &serv_addr,
            sizeof(serv_addr)) < 0) 
            error("ERROR on binding");
    listen(sockfd,5);
    clilen = sizeof(cli_addr);
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    if (newsockfd < 0) {
        error("ERROR on accept");
    }
    n = write(newsockfd, "Welcome to this backdoor\n", 26);
    int acks;
    do {
        bzero(ack, sizeof(ack));
        n = read(sockfd,ack, 1024);
        printf("recive: %s\n", ack);
        if(n > 0);{
            acks = atoi(ack);
            acks++;
            bzero(ack, sizeof(ack));
            sprintf(ack, "%d", acks);
            write(sockfd, ack,sizeof(ack));
        }
    }while (1);
    
    close(newsockfd);
    close(sockfd);
    return 0; 
}