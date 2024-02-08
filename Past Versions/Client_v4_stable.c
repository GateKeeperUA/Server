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

volatile int counter;
volatile char* key[numkeys][keyLen];
volatile char message_cifer[keyLen];

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
    printf("\nKeys upload complete\n");

    counter = 0;
}

void fill_dummy(int start, char* data) {
    for(int i=start;i<keyLen;i++) {data[i] = 97+rand()%20;}
    data[keyLen-1]='\0';
}
   
void XORCipher(char* data, int cifer) {
    if(cifer==1){
        for(int i=0;i<strlen(data);i++) {message_cifer[i]=data[i];}
        message_cifer[strlen(data)]='\0';
        fill_dummy(strlen(data)+1,(char*)message_cifer);
    }
    else {
        for(int i=0;i<keyLen;i++) {message_cifer[i]=data[i];}
    }

	for (int i=0;i<keyLen-1;++i) {
        message_cifer[i] = message_cifer[i] ^ (long int)key[counter][i];        
        if(message_cifer[i]=='\0' && cifer==0) {break;}
	}
    if(counter==numkeys-1){counter = 0;}
    else {counter++;}
}


// Driver code
int main() {
    int sockfd;
    char buffer[keyLen];
    char message[keyLen] = "Test";
    struct sockaddr_in servaddr;
    int len, counter;

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
    servaddr.sin_addr.s_addr = inet_addr("192.168.0.240");

    char* message_init="_%&hqt6G+WuXa4oq*uISC?V20k{gpRgcE&#G_0A62rua7vEoc*2+JrZuHaW*ZSr!=LT=yVK)ef-)w5p[gjyI{emT4nk=C*%QKQ#[Tuk}HQ0){ISk#JYrxUJ8UO-m&xq";
    sendto(sockfd, message_init, keyLen, MSG_CONFIRM, (const struct sockaddr *) &servaddr, sizeof(servaddr));
    recvfrom(sockfd, buffer, keyLen, MSG_WAITALL, (struct sockaddr *) &servaddr, &len);
    if(strcmp(message_init,buffer)==0) { printf("Conection established\n");}

    while(1){
        memset(message, 0, sizeof(message));
        memset(buffer, 0, sizeof(buffer));
        sprintf(message,"Test %d",counter);
        printf("\nMessage to send: %s\n",message);

        XORCipher(message,1);
        sendto(sockfd, (char*)message_cifer, keyLen, MSG_CONFIRM, (const struct sockaddr *) &servaddr, sizeof(servaddr));
        
        recvfrom(sockfd, buffer, keyLen, MSG_WAITALL, (struct sockaddr *) &servaddr, &len);

        XORCipher(buffer,0);

        printf("Server: %s\n", message_cifer);

        sleep(2);
        counter++;
    }

    close(sockfd);
    return 0;
}
