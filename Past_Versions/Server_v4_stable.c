// Server side implementation of UDP client-server model
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdbool.h>
   
#define PORT    5005
#define keyLen  128
#define numkeys 500
#define MaxClients 200

volatile char* key[numkeys][keyLen];
volatile char message_cifer[keyLen];
   
struct clients{
    char IP[15];
    int IPsize;
    int counter;
};

volatile struct clients client[MaxClients]={};

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

    for(int i=0;i<MaxClients;i++) {client[i].IPsize = 0;}
}
   
void fill_dummy(int start, char* data) {
    for(int i=start;i<keyLen-1;i++) {data[i] = 97+rand()%20;}
    data[keyLen-1]='\0';
}
   
void XORCipher(char* data, int cifer, int k) {
    if(cifer==1){
        for(int i=0;i<strlen(data);i++) {message_cifer[i]=data[i];}
        message_cifer[strlen(data)]='\0';
        fill_dummy(strlen(data)+1,(char*)message_cifer);
    }
    else {
        for(int i=0;i<keyLen;i++) {message_cifer[i]=data[i];}
    }

	for (int i=0;i<keyLen;++i) {
        message_cifer[i] = message_cifer[i] ^ (long int)key[client[k].counter][i];
        if(message_cifer[i]=='\0' && cifer==0) {break;}
	}

    if(client[k].counter==numkeys-1){client[k].counter = 0;}
    else {client[k].counter++;}
}

// Driver code
int main() {
    int sockfd;
    char buffer[keyLen];
    struct sockaddr_in servaddr, cliaddr;
    int len;    
    bool restart;
    char* message_init="_%&hqt6G+WuXa4oq*uISC?V20k{gpRgcE&#G_0A62rua7vEoc*2+JrZuHaW*ZSr!=LT=yVK)ef-)w5p[gjyI{emT4nk=C*%QKQ#[Tuk}HQ0){ISk#JYrxUJ8UO-m&xq";
    char* message_test = "The temperatue is 25ºC!";

    if(Initialize()==1){return 1;}
       
    // Creating socket file descriptor
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
       
    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));
       
    // Filling server information
    servaddr.sin_family    = AF_INET; // IPv4
    servaddr.sin_addr.s_addr = inet_addr("192.168.0.240");
    servaddr.sin_port = htons(PORT);
   
    // Bind the socket with the server address
    if ( bind(sockfd, (const struct sockaddr *)&servaddr, 
            sizeof(servaddr)) < 0 )
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
       
    len = sizeof(cliaddr);  //len is value/result


    while(1){
        memset(buffer, 0, sizeof(buffer));
        recvfrom(sockfd, buffer, keyLen, MSG_WAITALL, ( struct sockaddr *) &cliaddr, &len);
        if(strcmp(buffer,message_init)==0) {restart = true;}
        else {restart = false;}   
        
        for (int k=0;k<MaxClients;k++) {
            if(client[k].IPsize>0 && strcmp((const char*)client[k].IP,inet_ntoa(cliaddr.sin_addr))==0){
                if (restart == false) {
                    XORCipher(buffer,0,k);
                    printf("Existing client (nº%d) %s:%d -> %s\n", k, inet_ntoa(cliaddr.sin_addr), htons(cliaddr.sin_port), message_cifer);
                    XORCipher((char*)message_test,1,k);  
                      
                }
                else {
                    client[k].counter = 0;
                    printf("Existing client (nº%d) %s:%d -> Has enter\n", k, inet_ntoa(cliaddr.sin_addr), htons(cliaddr.sin_port));
                }
                goto exit;
            }
            else if(client[k].IPsize==0){
                client[k].IPsize=strlen(inet_ntoa(cliaddr.sin_addr));
                for(int i=0;i<client[k].IPsize;i++){
                        client[k].IP[i]=inet_ntoa(cliaddr.sin_addr)[i];
                }
                client[k].counter = 0;
                printf("New client (nº%d) %s:%d -> Has enter\n", k, inet_ntoa(cliaddr.sin_addr), htons(cliaddr.sin_port));
                goto exit;
            }
        }
        exit:
        if(restart == false) {
            sendto(sockfd, (char*)message_cifer, keyLen, MSG_CONFIRM, (const struct sockaddr *) &cliaddr, len);
        }
        else {
            sendto(sockfd, (char*)message_init, keyLen, MSG_CONFIRM, (const struct sockaddr *) &cliaddr, len);   
        }
    }
    return 0;
}
