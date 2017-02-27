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
    //character buffers
    char hash_message[1024];
    char plan_message[1024];
    char iv[128];
    char key[1024];
    
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
    struct sockaddr_in serv_addr, serverAddr;
    struct hostent *serverHost;
    socklen_t serveraddr_size;


    /*TODO
    * split the variables and stuff here!!!
    */


    //set up the socket
    bzero((char *) &serverAddr, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(portno);
    bcopy((char *)serverHost->h_addr, (char *)&serverAddr.sin_addr.s_addr, serverHost->h_length);
    memset(serverAddr.sin_zero, '\0', sizeof(serverAddr.sin_zero));
    serveraddr_size = sizeof(serverAddr);

    //connect socket
    if(connect(sockfd, (struct sockaddr*) &serverAddr, serveraddr_size) < 0){
        error("ERROR, cannot connect to server socket");
    }

    //hand shake stuff
    /*
    * out: ciphernumber|encryption of IV using null IV     //this allows the server to know what encryption is used, and if the user have the right key
    * in: MD5 hash of IV                                   //this allows the client to know that the server have
    */

    //loop for file transfer


}