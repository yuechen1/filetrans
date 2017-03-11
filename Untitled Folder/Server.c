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
#include <pthread.h>
#include <ctype.h>
#include <netdb.h>
#include <time.h>

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
    char filename[1024];

    //command logic
    char* mWRITE = "write";
    char* mREAD = "read";
    char* mCIPHER_NONE = "none";
    char* mCIPHER_AES128 = "aes128";
    char* mCIPHER_AES256 = "aes256";
    int isread = 1;
    // 0 = none
    // 1 = aes128
    // 2 = aes256
    int cipherNumber = 0;

    //socket comunication information
    struct sockaddr_in serv_addr, cli_addr;
    int sockfd, newsockfd, portno;      
    socklen_t clilen;    
    
    //
    int n;
    int i;

    //File IO
    FILE *file;
    size_t readsize;

    if (argc != 2) {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }else{
        if(argc == 3){
            strcpy(key, argv[2]);
        }else{
            strcpy(key, "00000000000000000000000000000000");
        }
    }
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        error("ERROR opening socket");
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    printf("in main: %s\n", argv[1]);
    fflush(stdout);
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0){
            error("ERROR on binding");
    }
    listen(sockfd,5);
    clilen = sizeof(cli_addr);
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    if (newsockfd < 0) {
        error("ERROR on accept");
    }
    


    int acks;
    bzero(ack, sizeof(ack));
    n = read(newsockfd, ack, 128);
    printf("ack: %s\n", ack);
    fflush(stdout);

    if(n > 0){
        printf("in the for \n");
        fflush(stdout);
        //check for command and filename
        //sperate by space, command is not stored, only the isread is fliped
        i = 0;
        while (1){
            if (ack[i] == ' '){
                break;
            }
            else if (ack[i] == '\0')
            {
                error("cannot find port number");
            }else{
                i++;
            }
        }
        strncpy(plan_message, ack, i);
        i++;
        strncpy(filename, &ack[i], (n - i));

        //debug section
        printf("command: %s\n", plan_message);
        fflush(stdout);
        printf("filename: %s\n", filename);
        fflush(stdout);


        //check for read or write.
        //read is to pull from server
        //write is get from client
        if(strncmp(plan_message, mWRITE, 5) == 0){
            isread = 0;
        }else if(strncmp(plan_message, mREAD, 4)){
            isread = 1;
        }else{
            error("incorect command");
        }

        //check for file based on input
        if(isread == 1){
            file = fopen(filename,"r");
        }else{
            file = fopen(filename, "w+");
        }
        if(file){
            printf("file found: %s", filename);
            fflush(stdout);
        }


        while((n = read(newsockfd, ack, 128)) > 0){
            n = fputs(ack, file);
            if(n < 0){
                error("cannot write to file");
            }
            printf("recive: %s\n", ack);
            fflush(stdout);
            /*n = read(newsockfd, ack, 128);
            if(n > 0){
                printf("recive: %s\n", ack);
                fflush(stdout);
                bzero(ack, 128);
                //write(newsockfd, ack,sizeof(ack));
            }*/
        }
    }

    printf("just finish the loop\n");
    fflush(stdout);

    close(newsockfd);
    close(sockfd);
    return 0; 
}