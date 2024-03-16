// Client side implementation of UDP client-server model
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <time.h>
   
#define PORT     5005
#define keyLen   128
#define numkeys  500

int counter;
char* key[numkeys][keyLen];
char message_cipher[keyLen];

int Initialize(){
    char *filename = "Encryption_keys.txt";
    char dummy[keyLen];
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("Error: could not open file %s", filename);
        return 1;
    }
    for(int i=0;i<numkeys;i++){
        fgets((char *)key[i],keyLen,fp);
        fgets(dummy,keyLen,fp);
    }
    fclose(fp);
    printf("\nKeys upload completed\n");

    counter = 0;
}

void fill_dummy(int start, char* data) {
    for(int i=start;i<keyLen;i++) {data[i] = rand()%126;}
    data[keyLen-1]='\0';
}
   
void XORCipher(char* data, bool send, char type) {
    if(send==true){
        message_cipher[0] = type;
        for(int i=0;i<strlen(data);i++) {message_cipher[i+1]=data[i];}
        message_cipher[strlen(data)+1]='\0';
        fill_dummy(strlen(data)+2,(char*)message_cipher);

        for (int i=1;i<keyLen;++i) {
            message_cipher[i] = message_cipher[i] ^ (long int)key[counter][i-1];
        }
    }
    else {
        for(int i=1;i<keyLen;i++) {message_cipher[i-1]=data[i];}

        for (int i=0;i<keyLen;++i) {
            message_cipher[i] = message_cipher[i] ^ (long int)key[counter][i];
            if(message_cipher[i]=='\0') {break;}
        }
    }

    if(counter==numkeys-1){counter = 0;}
    else {counter++;}
}

void send_temperature() {
    sprintf((char*)message_cipher,"2%d %d %d %d \0",175,1033,70,4);
    fill_dummy(strlen((char*) message_cipher)+1,(char*)message_cipher);
}

// Driver code
int main() {
    int sockfd;
    char buffer[keyLen];
    char message[keyLen];
    struct sockaddr_in servaddr;
    int len, try_connect=0;
    char option[2];
    char* message_init="03143%&hqt6G+WuXa4oq*uISC?V20k{gpRgcE&#G_0A62rua7vEoc*2+JrZuHaW*ZSr!=LT=yVK)ef-)w5p[gjyI{emT4nk=C*%QKQ#[Tuk}HQ0){ISk#JYrxUJ8UO-";
    

   if(Initialize()==1){return 1;}

    // Creating socket file descriptor
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
   
    memset(&servaddr, 0, sizeof(servaddr));
       
    // Filling server information
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = inet_addr("192.168.1.86");

    CONNECT:
    try_connect++;
    if(try_connect>5) {
        printf("Connection failed\n");
        return 1;
    }
    sendto(sockfd, message_init, keyLen, MSG_CONFIRM, (const struct sockaddr *) &servaddr, sizeof(servaddr));
    sleep(1);
    recvfrom(sockfd, buffer, keyLen, MSG_DONTWAIT, (struct sockaddr *) &servaddr, &len);
    for(int i=0;i<keyLen-5;i++) {if(buffer[i]!=message_init[i+5]) {goto CONNECT;}}

    while(1){
        memset(message, 0, sizeof(message));
        memset(buffer, 0, sizeof(buffer));
        memset((char*) message_cipher, 0, sizeof(buffer));

        //! 0 is new connection
        //! 1 is ID checkup on Database
        //! 2 is temperature data
        
        fflush(stdin);
        printf("\n\rChoose a simulation scenario:\n-1 is ID checkup on Database\n-2 is temperature data\n");
        fgets(option, 2, stdin);

        switch(option[0]) {
            case '1':
                sprintf(message,"[ID]");
                XORCipher(message,true,'1');
                sendto(sockfd, (char*)message_cipher, keyLen, MSG_CONFIRM, (const struct sockaddr *) &servaddr, sizeof(servaddr));
                
                recvfrom(sockfd, buffer, keyLen, MSG_WAITALL, (struct sockaddr *) &servaddr, &len);

                XORCipher(buffer,false,'1');
                printf("Server: %s\n", message_cipher);
                break;
            case '2':
                send_temperature();
                sendto(sockfd, (char*)message_cipher, keyLen, MSG_CONFIRM, (const struct sockaddr *) &servaddr, sizeof(servaddr)); 
                break;
        }
    }

    close(sockfd);
    return 0;
}
