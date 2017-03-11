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
    char iv[17];
    char key128[17];
    char key256[33];
    
    //command logic
    char* mWRITE = "write";
    char* mREAD = "read";
    char* mCIPHER_NONE = "none";
    char* mCIPHER_AES128 = "aes128";
    char* mCIPHER_AES256 = "aes256";
    int isread = 1;
    char server[1024];
    char port[1024];
    // 0 = none
    // 1 = aes128
    // 2 = aes256
    int cipherNumber = 0;
    int bytesread;
    int acks;

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
    //printf("Program started\n");
    if(argc > 4 && argc < 6){
        //printf("see if read or write\n");

        //see if read or write
        if(strncmp(argv[1], mWRITE, 5) == 0){
            isread = 1;
        }else if(strncmp(argv[1], mREAD, 4)){
            isread = 0;
        }
        printf("it is: %s\n", argv[1]);

        //printf("get filename\n");
        //get file
        strcpy(filename, argv[2]);
        
        
        //check if the file exisits
        if(isread == 1){
            file = fopen(filename,"r");
        }else{
            file = fopen(filename, "w+");
        }
        if(file){
            printf("file found: %s", filename);
            fflush(stdout);
        }

        //find the hostname and port number
        strcpy(tempbuffer, argv[3]);
        i = 0;
        while(1){
            if(tempbuffer[i] == ':')
            {
                break;
            }
            else if(tempbuffer[i] == '\0'){
                error("cannot find port number");
            }else{
                i++;
            }
        }
        strncpy(server, tempbuffer, i);
        
        printf("server: %s\n", server);
        fflush(stdout);
        strncpy(port, &tempbuffer[i+1], (strlen(argv[3]) - i));
        printf("port: %s\n", port);
        fflush(stdout);

        //see which cipher is being used
        //TODO::key currently is hard coded, this need to be changed
        if(strncmp(argv[4], mCIPHER_NONE, sizeof(mCIPHER_NONE)) == 0){
            cipherNumber = 0;
        }
        else if(strncmp(argv[4], mCIPHER_AES128, sizeof(mCIPHER_AES128)) == 0){
            cipherNumber = 1;
            if(argc != 6){
                strcpy(key128, "0000000000000000");
            }else{
                strcpy(key128, argv[5]);
            }
            printf("key: %s\n", key128);
            fflush(stdout);
        }
        else if(strncmp(argv[4], mCIPHER_AES256, sizeof(mCIPHER_AES256)) == 0){
           cipherNumber = 2;
           if(argc != 6){
                strcpy(key256, "00000000000000000000000000000000");
            }else{
                strcpy(key256, argv[5]);
            }
            printf("key: %s\n", key256);
            fflush(stdout);
        }else{
            error("incorrect cipher");
        }

    }else{
        error("invalid input");    int bytesread;
    int acks;
    }

    //set up the socket
    serverHost = gethostbyname(server);
    bzero((char *) &serverAddr, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(atoi(port));
    bcopy((char *)serverHost->h_addr, (char *)&serverAddr.sin_addr.s_addr, serverHost->h_length);
    memset(serverAddr.sin_zero, '\0', sizeof(serverAddr.sin_zero));
    serveraddr_size = sizeof(serverAddr);


    //create local socket
    sockfd = socket(AF_INET,SOCK_STREAM, 0);

    //connect socket
    if(connect(sockfd, (struct sockaddr*) &serverAddr, serveraddr_size) < 0){
        error("ERROR, cannot connect to server socket");
    }

    //generate iv
    for(i = 0; i < 16; i++){
        iv[i] = (unsigned char) (rand()%255);
    }
    iv[16] = '\0';

    //security mode and iv is put together and sent
    sprintf(tempbuffer, "%s:%s", argv[4], iv);
    printf("%s\n", tempbuffer);
    fflush(stdout);
    write(sockfd, tempbuffer, sizeof(tempbuffer));

    //send command and filename to server
    if(isread == 1){
        sprintf(ack, "%s %s", mWRITE, filename);
    }else{
        sprintf(ack, "%s %s", mREAD, filename);
    }
    //
    write(sockfd, ack, sizeof(ack));
    printf("%s\n",ack);
    fflush(stdout);
    //wait for server response
    while(1){}
    while((fread(plan_message, 1, sizeof(plan_message), file)>0)){
        write(sockfd, plan_message, sizeof(plan_message));
        printf("%s\n",plan_message);
        fflush(stdout);
        bzero(plan_message, sizeof(plan_message));
    }

    printf("just finish the loop");
    fflush(stdout);

}