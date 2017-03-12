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

//code from wiki.openssl.org/index.php/EVP_Symmetric_Encryption_and_Decryption
void handleErrors(void){
    ERR_print_errors_fp(stderr);
    abort();
}

int main(int argc, char *argv[])
{
    //encryption
    int len;
    EVP_CIPHER_CTX *ctx;
    EVP_CIPHER_CTX *ctxd;

    //character buffers
    char hash_message[128];
    char plan_message[128];
    char ivd[17];
    char iv[17];
    char key256[33];
    char key128[17];
    char filename[128];
    char tempbuffer[1024];

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

    if (argc < 2) {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }else{
        if(argc == 3){
            strcpy(key256, argv[2]);
            while(strlen(key256) < sizeof(key256)){
                strcat(key256, argv[2]);
            }
        }else{
            strcpy(key256, "00000000000000000000000000000000");
        }
        strncpy(key128, key256, sizeof(key128));
        key128[16] = '\0';
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
    

    //get encryption and iv from client
    bzero(tempbuffer, sizeof(tempbuffer));
    n = read(newsockfd, tempbuffer, sizeof(tempbuffer));
    if(n > 0){
        i = 0;
        while (1){
            if (tempbuffer[i] == ':'){
                break;
            }
            else if (tempbuffer[i] == '\0')
            {
                error("cannot find iv");
            }else{
                i++;
            }
        }
        bzero(plan_message, sizeof(plan_message));
        printf("%s\ni = %d\nn = %d\n", tempbuffer, i, n);
        fflush(stdout);
        strncpy(plan_message, tempbuffer, i);
        i++;
        strncpy(iv, &tempbuffer[i], 16);
        iv[16] = '\0';
        strcpy(ivd, iv);
        printf("cipher: %s\niv: %s\n", plan_message, ivd);
        fflush(stdout);
        //see if its none
        if(strncmp(plan_message, mCIPHER_NONE, 4) == 0){
            cipherNumber = 0;
        }
        //see if its aes128
        else if(strncmp(plan_message, mCIPHER_AES128, 6) == 0){
            cipherNumber = 1;
        }
        //see if its aes 256
        else if(strncmp(plan_message, mCIPHER_AES256, 6) == 0){
            cipherNumber = 2;
        }else{
            error("incorrect cipher");
        }
    }else{
        error("no input detected");
    }

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




    bzero(tempbuffer, sizeof(tempbuffer));
    bzero(plan_message, sizeof(plan_message));
    //get filename, and command and file transfer
    if(cipherNumber > 0){
        n = read(newsockfd, hash_message, sizeof(hash_message));    
        if(1 != EVP_DecryptUpdate(ctxd, plan_message, &n, hash_message, sizeof(hash_message))){
            handleErrors();
        }
        BIO_dump_fp(stdout, (const char *)hash_message, n);
        printf("len: %d\n", n);
        fflush(stdout);  
    }else{
        n = read(newsockfd, plan_message, sizeof(plan_message));
    }
    printf("plan_message: %s\n", plan_message);
    fflush(stdout);
    if(n > 0){

        //check for command and filename
        //sperate by space, command is not stored, only the isread is fliped
        i = 0;
        while (1){
            if (plan_message[i] == ' '){
                break;
            }
            else if (plan_message[i] == '\0')
            {
                error("cannot find command");
            }else{
                i++;
            }
        }
        strncpy(tempbuffer, plan_message, i);
        i++;
        strncpy(filename, &plan_message[i], (n - i));

        //debug section
        printf("command: %s\n", tempbuffer);
        fflush(stdout);
        printf("filename: %s\n", filename);
        fflush(stdout);

        //check for read or write.
        //read is to pull from server
        //write is get from client
        if(strncmp(tempbuffer, mWRITE, 5) == 0){
            isread = 1;
        }else if(strncmp(tempbuffer, mREAD, 4) == 0){
            isread = 0;
        }else{
            error("incorect command");
        }

        //check for file based on input
        if(isread == 0){
            file = fopen(filename,"r");
        }else{
            file = fopen(filename, "w+");
        }
        if(file){
            printf("file found: %s\n", filename);
            fflush(stdout);
        }

        //send filename back to client
        if(cipherNumber > 0){
            if(1 != EVP_EncryptUpdate(ctx, hash_message, &len, filename, sizeof(filename))){
                handleErrors();
            }
            write(newsockfd, hash_message, sizeof(hash_message));
        }else{
            write(newsockfd, filename, strlen(filename));
        }

        bzero(plan_message, sizeof(plan_message));
        bzero(hash_message, sizeof(hash_message));
        if(isread == 1){
            printf("reading from socket");
            while((n = read(newsockfd, hash_message, sizeof(hash_message))) > 0){
                if(cipherNumber > 0){
                    if(1 != EVP_DecryptUpdate(ctxd, plan_message, &n, hash_message, sizeof(hash_message))){
                        handleErrors();
                    }
                    BIO_dump_fp(stdout, (const char *)hash_message, n);
                    printf("len: %d\nplan_message: %s", n, plan_message);
                    n = fputs(plan_message, file);
                }else{    
                    n = fputs(hash_message, file);
                }
                if(n < 0){
                    error("cannot write to file");
                }
            }
        }else{
            printf("reading from file");
            while((fread(plan_message, 1, sizeof(plan_message), file)>0)){
                if(cipherNumber > 0){
                    if(1 != EVP_EncryptUpdate(ctx, hash_message, &len, plan_message, sizeof(plan_message))){
                        handleErrors();
                    }
                    write(newsockfd, hash_message, len);
                    BIO_dump_fp(stdout, (const char *)hash_message, len);
                    printf("len: %d\n", len);
                    fflush(stdout);
                }else{
                    write(newsockfd, plan_message, sizeof(plan_message));
                    printf("%s\n",plan_message);
                    fflush(stdout);
                }
                bzero(plan_message, sizeof(plan_message));
                bzero(hash_message, sizeof(hash_message));
            }
        }
    }

    printf("just finish the loop\n");
    fflush(stdout);

    if(cipherNumber > 0){
        EVP_CIPHER_CTX_free(ctx);
        EVP_CIPHER_CTX_free(ctxd);
        EVP_cleanup();
        ERR_free_strings();
    }


    close(newsockfd);
    close(sockfd);
    return 0; 
}