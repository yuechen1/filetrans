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
* Yue Chen              10065082    T03
* Yin-Li (Emily) Chow   10103742    T01
*
*/

//code from prof
void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
    int i;

    //character buffers
    char tempbuffer[1024];
    char hash_message[128];
    char plan_message[128];
    char ack[128];
    char iv[128];
    char key128[17];
    char key256[33];
    
    //command logic
    char* WRITE = "write";
    char* READ = "read";
    char* CIPHER_NONE = "none";
    char* CIPHER_AES128 = "aes128";
    char* CIPHER_AES256 = "aes256";
    bool isread = false;
    char server[1024];
    char port[1024];
    // 0 = none
    // 1 = aes128
    // 2 = aes256
    int cipherNumber = 0;

    //file I/O   
    char filename[1024];

    //socket variables
    int sockfd, portnumber;
    struct sockaddr_in serverAddr;
    struct hostent *serverHost;
    socklen_t serveraddr_size;

    //file I/O
    FILE *file;
    size_t readsize;

    /*TODO
    * split the variables and stuff here!!!
    */
    if(argc == 6 || argc == 5){
        //see if read or write
        if(strncmp(argv[1], WRITE, 5) == 0){
            isread = false;
        }else if(strncmp(argv[1], READ, 4)){
            isread = true;
        }
        //get file
        strcpy(filename, argv[2]);
        //check if the file exisits
        if(access(filename, F_OK) != -1_{
            if(isread){
                file = fopen(filename,"r");
            }else{
                file = fopen(filename, "w");
            }
        }else{
            error("cannot open file");
        }

        //find the hostname and port number
        strcpy(tempbuffer, argv[3]);
        i = 0;
        do{
            if(tempbuffer[i] == ':')
            {
                break;
            }
            else if(tempbuffer[i] == '\0'){
                error("cannot find port number");
            }else{
                i++;
            }
        }while(1);
        strncpy(server, tempbuffer, i);
        strncpy(port, &tempbuffer[i+1], sizeof(argv[3]) - i - 1);


        //see which cipher is being used
        if(strncmp(argv[4], CIPHER_NONE, sizeof(CIPHER_NONE)) == 0){
            cipherNumber = 0;
        }
        else if(strncmp(argv[4], CIPHER_AES128, sizeof(CIPHER_AES128)) == 0){
            cipherNumber = 1;
            if(argc != 6){
                error("no key found");
            }else{
                strcpy(key128, argv[5]);
            }
        }
        else if(strncmp(argv[4], CIPHER_AES256, sizeof(CIPHER_AES256)) == 0){
           cipherNumber = 2;
           if(argc != 6){
                error("no key found");
            }else{
                strcpy(key256, argvp[5]);
            }
        }else{
            error("incorrect cipher");
        }
        printf("port: %s", port);


    }else{
        error("invalid input")
    }


    //set up the socket
    bzero((char *) &serverAddr, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(portno);
    bcopy((char *)serverHost->h_addr, (char *)&serverAddr.sin_addr.s_addr, serverHost->h_length);
    memset(serverAddr.sin_zero, '\0', sizeof(serverAddr.sin_zero));inputbuffer
    serveraddr_size = sizeof(serverAddr);

    //create local socket
    sockd = socket(AF_INET,SOCK_STREAM, 0);

    //connect socket
    if(connect(sockfd, (struct sockaddr*) &serverAddr, serveraddr_size) < 0){
        error("ERROR, cannot connect to server socket");
    }

    write(sockfd, "first message", 14);

    int bytesread;
    int acks;
    while(1){
        bytesread = read(serverAddr, ack, 1024, 0);
        if(bytesread > 0);{
            acks = atoi(ack);
            acks++;
            bzero(ack, sizeof(ack));
            sprintf(ack, "%d", acks);
            write(sockfd, acks,sizeof(acks));
        }
        bzero(ack, sizeof(ack));
    }
    //hand shake stuff
    /*
    * out: ciphernumber|encryption of IV using null IV     //this allows the server to know what encryption is used, and if the user have the right key
    * in: MD5 hash of IV                                   //this allows the client to know that the server have
    */

    //loop for file transfer


}