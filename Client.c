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
#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>

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


//code from wiki.openssl.org/index.php/EVP_Symmetric_Encryption_and_Decryption
void handleErrors(void){
    ERR_print_errors_fp(stderr);
    abort();
}

int main(int argc, char *argv[])
{
    int i;
    //encryption
    int len;
    EVP_CIPHER_CTX *ctx;
    EVP_CIPHER_CTX *ctxd;

    //character buffers
    char tempbuffer[1024];
    char hash_message[128];
    char plan_message[128];
    char ack[128];
    char iv[17];
    char ivd[17];
    char key128[17];
    char key256[33];
    
    //command logic
    char* mWRITE = "write";
    char* mREAD = "read";
    char* mCIPHER_NONE = "none";
    char* mCIPHER_AES128 = "aes128";
    char* mCIPHER_AES256 = "aes256";
    int isread = 1;
    char server[128];
    char port[1024];
    // 0 = none
    // 1 = aes128
    // 2 = aes256
    int cipherNumber = 0;
    int bytesread;
    int n;

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
    if(argc > 4 && argc <= 6){
        //printf("see if read or write\n");

        //see if read or write
        if(strncmp(argv[1], mWRITE, 5) == 0){
            isread = 1;
        }else if(strncmp(argv[1], mREAD, 4) == 0){
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
            printf("file found: %s\n", filename);
            fflush(stdout);
        }else{
	    error("Cannot get File");
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
	bzero(server, sizeof(server));        
	strncpy(server, tempbuffer, i);
        
        printf("server: %s\ni: %d\n", server, i);
        strncpy(port, &tempbuffer[i+1], (strlen(argv[3]) - i));
        printf("port: %s\n", port);
        fflush(stdout);

        //see which cipher is being used
        if(strncmp(argv[4], mCIPHER_NONE, sizeof(mCIPHER_NONE)) == 0){
            cipherNumber = 0;
        }
        else if(strncmp(argv[4], mCIPHER_AES128, sizeof(mCIPHER_AES128)) == 0){
            cipherNumber = 1;
            if(argc != 6){
                error("No Key Input");
		//strcpy(key128, "0000000000000000");
            }else{
                strcpy(key128, argv[5]);
		n = strlen(key128);
	    	while(n < 16){
		    if((16-n) < strlen(argv[5])){
		        strcat(key256, argv[5]);
		    }else{
		        strncat(key256, argv[5], 16 - n);
		    }
		    n = strlen(key128);
	        }
	        key128[16] = '\0';
            }
            printf("key: %s\n", key128);
            fflush(stdout);
        }
        else if(strncmp(argv[4], mCIPHER_AES256, sizeof(mCIPHER_AES256)) == 0){
           cipherNumber = 2;
           if(argc != 6){
                error("No Key Input");                
		//strcpy(key256, "00000000000000000000000000000000");
            }else{
                strcpy(key256, argv[5]);
		n = strlen(key256);
	    	while(n < 32){
		    if((32-n) < strlen(argv[5])){
		        strcat(key256, argv[5]);
		    }else{
		        strncat(key256, argv[5], 32 - n);
		    }
		    n = strlen(key256);
	        }
	        key128[32] = '\0';
            }
            printf("key: %s\n", key256);
            fflush(stdout);
        }else{
            error("incorrect cipher");
        }

    }else{
        error("invalid input");
    }



    //set up the socket
    serverHost = gethostbyname(server);
    bzero((char *) &serverAddr, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(atoi(port));
    bcopy((char *)serverHost->h_addr, (char *)&serverAddr.sin_addr.s_addr, serverHost->h_length);
    memset(serverAddr.sin_zero, '\0', sizeof(serverAddr.sin_zero));
    serveraddr_size = sizeof(serverAddr);

    printf("key: %s\n", key256);
    fflush(stdout);

    //create local socket
    sockfd = socket(AF_INET,SOCK_STREAM, 0);

    //generate iv
    for(i = 0; i < 16; i++){
        do
        {
            n = rand()%74 + 48;
        }while(n > 122 || (n > 57 && n < 65) || (n > 90 && n < 97) );
        iv[i] = (unsigned char) n;
    }
    iv[16] = '\0';
    strcpy(ivd, iv);

    printf("iv: %s\n", ivd);
    fflush(stdout);

    //start encryption
    if(cipherNumber > 0){
        ERR_load_crypto_strings();
        OpenSSL_add_all_algorithms();
        OPENSSL_config(NULL);

        //create encryption and decrytion context
        if(!(ctx = EVP_CIPHER_CTX_new())){
            handleErrors();
        }
        if(!(ctxd = EVP_CIPHER_CTX_new())){
            handleErrors();
        }

        //start the encrypt context based on cipher selected
        if(cipherNumber == 1){
            if(1 != EVP_EncryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, key128, iv)){
                handleErrors();
            }
            if(1 != EVP_DecryptInit_ex(ctxd, EVP_aes_128_cbc(), NULL, key128, ivd)){
                handleErrors();
            }
        }
        else if(cipherNumber == 2){
            if(1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key256, iv)){
                handleErrors();
            }
            if(1 != EVP_DecryptInit_ex(ctxd, EVP_aes_256_cbc(), NULL, key256, ivd)){
                handleErrors();
            }
        }
    }


    //connect socket
    if(connect(sockfd, (struct sockaddr*) &serverAddr, serveraddr_size) < 0){
        error("ERROR, cannot connect to server socket");
    }

    //security mode and iv is put together and sent
    bzero(tempbuffer, sizeof(tempbuffer));
    sprintf(tempbuffer, "%s:%s", argv[4], iv);
    printf("%s\n", tempbuffer);
    fflush(stdout);
    write(sockfd, tempbuffer, sizeof(tempbuffer));

    //create command and filename string
    bzero(plan_message, sizeof(plan_message));
    if(isread == 1){
        sprintf(plan_message, "%s %s", mWRITE, filename);
    }else{
        sprintf(plan_message, "%s %s", mREAD, filename);
    }
    
    //encrypt the message
    if(cipherNumber > 0){
        if(1 != EVP_EncryptUpdate(ctx, hash_message, &len, plan_message, sizeof(plan_message))){
            handleErrors();
        }
        write(sockfd, hash_message, sizeof(hash_message));
        BIO_dump_fp(stdout, (const char *)hash_message, len);
        printf("len: %d\n", len);
        fflush(stdout);
    }else{
        write(sockfd, plan_message, sizeof(plan_message));
        printf("%s\n",plan_message);
        fflush(stdout);
    }
    
    //get filename from Server, if encrypted filename does not match then the Server have wrong key
    while(read(sockfd, hash_message, sizeof(hash_message)) < 0){
        if(cipherNumber > 0){
            if(1 != EVP_DecryptUpdate(ctxd, plan_message, &n, hash_message, sizeof(hash_message))){
                    handleErrors();
            }
            if(strncmp(filename, plan_message, strlen(filename)) != 0){
                error("Error, incorrect key\n");
            }
        }
    }

    //send content of file
    bzero(plan_message, sizeof(plan_message));
    bzero(hash_message, sizeof(hash_message));
    if(isread == 1){
        printf("reading from file\n");
	    n = fread(plan_message, 1, sizeof(plan_message), file);     
	    do{
            if(cipherNumber > 0){
                if(1 != EVP_EncryptUpdate(ctx, hash_message, &len, plan_message, sizeof(plan_message))){
                    handleErrors();
                }
                write(sockfd, hash_message, sizeof(hash_message));
                BIO_dump_fp(stdout, (const char *)hash_message, sizeof(hash_message));
                //printf("len: %d\n", len);
                //fflush(stdout);
            }else{
                write(sockfd, plan_message, n);
                //printf("%s\n",plan_message);
                //fflush(stdout);
            }
            bzero(plan_message, sizeof(plan_message));
            bzero(hash_message, sizeof(hash_message));
	        n = fread(plan_message, 1, sizeof(plan_message), file);
        }while(n > 0);
    }else{
        printf("reading from socket\n");
	    n = read(sockfd, hash_message, sizeof(hash_message));
        do{
            if(cipherNumber > 0){
                if(1 != EVP_DecryptUpdate(ctxd, plan_message, &n, hash_message, sizeof(hash_message))){
                    handleErrors();
                }
                BIO_dump_fp(stdout, (const char *)hash_message, sizeof(hash_message));
                printf("len: %d\nplan_message: %s", n, plan_message);
                i = fputs(plan_message, file);
            }else{    
                i = fputs(hash_message, file);
                printf("%s\n",hash_message);
                fflush(stdout);
            }
            if(n < 0){
                error("cannot write to file");
            }
            bzero(plan_message, sizeof(plan_message));
            bzero(hash_message, sizeof(hash_message));
	        n = read(sockfd, hash_message, sizeof(hash_message));
        }while(n > 0);
    }

    if(cipherNumber > 0){
        EVP_CIPHER_CTX_free(ctx);
        EVP_CIPHER_CTX_free(ctxd);
        EVP_cleanup();
        ERR_free_strings();
    }
    printf("just finish the loop");
    fflush(stdout);

}
