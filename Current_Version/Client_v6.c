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
#include <pthread.h>
#include <fcntl.h>
#include <poll.h>
#include <errno.h>
   
#define PORT     5005
#define keyLen   128
#define numkeys  300

int counter, sockfd, len;
char key[numkeys][keyLen];
char message_cipher[keyLen];
char buffer[keyLen];
struct sockaddr_in servaddr;

int Initialize(){
    char *filename = "Encryption_keys.txt";
    char dummy[keyLen];
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("Error: could not open file %s", filename);
        return 1;
    }
    char line[keyLen*2];
    for(int i=0;fgets(line, keyLen*2, fp) != NULL;i++) {
        for(int j=0;j<keyLen;j++) {
            key[i][j] = line[j];
        }
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
            message_cipher[i] = message_cipher[i] ^ key[counter][i-1];
        }
    }
    else {
        for(int i=1;i<keyLen;i++) {message_cipher[i-1]=data[i];}

        for (int i=0;i<keyLen;++i) {
            message_cipher[i] = message_cipher[i] ^ key[counter][i];
            if(message_cipher[i]=='\0') {break;}
        }
    }

    if(counter==numkeys-1){counter = 0;}
    else {counter++;}
}

void send_temperature() {
    sprintf((char*)message_cipher,"2%d %d %d %d \0",1755,103322,70650,4);
    fill_dummy(strlen((char*) message_cipher)+1,(char*)message_cipher);
}

void *Check_Emergency(void *arg) {

    while (1) {
        ssize_t bytesRead = recvfrom(sockfd, buffer, keyLen, MSG_DONTWAIT, (struct sockaddr *) &servaddr, &len);
        if (buffer[0]=='9') {
            printf("Emergency!!\n");
            memset(buffer, 0, sizeof(buffer));
        }
        usleep(1000);
    }

    pthread_exit(NULL);
}

// Driver code
int main() {
    char buffer[keyLen];
    char message[keyLen];
    int try_connect=0;
    char option[2];
    char room[4];
    char message_init_[keyLen];
    char* message_init = "%&hqt6G+WuXa4oq*uISC?V20k{gpRgcE&#G_0A62rua7vEoc*2+JrZuHaW*ZSr!=LT=yVK)ef-)w5p[gjyI{emT4nk=C*%QKQ#[Tuk}HQ0){ISk#JYrxUJ8UO-";
    char* confirmation = "4jqz484yl94neddq0twxugnnyty6imjyc5zdeyyizl636mvk48pi1as8fnyc01a9lj3mamlp4jdcmjfviw48uv7fv4mv52gq75atzpus853ov2n8phy59cy3a77wp"; 
    char* serial_num = "301f26";
    char* emergency_message = "9Emergency";
    pthread_t check_emergency;
    

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
    servaddr.sin_addr.s_addr = inet_addr("192.168.1.100");

    printf("Choose room number (3 numbers):\n");
    fgets(room,4,stdin);
    sprintf(message_init_,"0%s%c%c%c%s",serial_num,room[0],room[1],room[2],message_init);


    CONNECT:
    try_connect++;
    if(try_connect>5) {
        printf("Connection failed\n");
        return 1;
    }
    sendto(sockfd, message_init_, keyLen, MSG_CONFIRM, (const struct sockaddr *) &servaddr, sizeof(servaddr));
    sleep(1);
    recvfrom(sockfd, buffer, keyLen, MSG_DONTWAIT, (struct sockaddr *) &servaddr, &len);
    for(int i=0;i<keyLen-10;i++) {if(buffer[i]!=message_init_[i+10]) {goto CONNECT;}}

    int err = pthread_create(&check_emergency, NULL, Check_Emergency, NULL);
    if (err != 0) {
        perror("Error creating thread");
        return EXIT_FAILURE;
    }

	struct pollfd fds[1];
	fds[0].fd = STDIN_FILENO;
	fds[0].events = POLLIN;

    if(fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFL) | O_NONBLOCK) < 0) {}
    
    printf("\n\rChoose a simulation scenario:\n-1 is ID checkup on Database\n-2 is temperature data\n-9 is emergency\n");
    while(1){
        memset(message, 0, sizeof(message));
        memset(buffer, 0, sizeof(buffer));
        memset((char*) message_cipher, 0, sizeof(buffer));

        int ready = poll(fds, 1, 0);
        if (ready == -1) {
            perror("Erro ao chamar a função poll");
            exit(EXIT_FAILURE);
        }else if (ready > 0) {
            // Dados disponíveis para leitura
            if (fds[0].revents & POLLIN) {
                // Lê a entrada padrão (stdin)
                ssize_t bytesRead = read(STDIN_FILENO, option, 2);
				
                if (bytesRead == -1) {
                    perror("Erro ao ler da entrada padrão");
                    exit(EXIT_FAILURE);
                } else if (bytesRead == 0) {
                    printf("Fim da entrada padrão\n");
                    break;
                }else {
                    switch(option[0]) {
                        case '1':
                            sprintf(message,"05af6486");
                            XORCipher(message,true,'1');
                            sendto(sockfd, (char*)message_cipher, keyLen, MSG_CONFIRM, (const struct sockaddr *) &servaddr, sizeof(servaddr));
                            
                            recvfrom(sockfd, buffer, keyLen, MSG_WAITALL, (struct sockaddr *) &servaddr, &len);

                            XORCipher(buffer,false,'1');

                            if(!strcmp(message_cipher,confirmation)) {
                                printf("Server: Can enter\n");
                            }
                            else {
                                printf("Server: Can't enter\n");
                            }
                            
                            break;
                        case '2':
                            send_temperature();
                            sendto(sockfd, (char*)message_cipher, keyLen, MSG_CONFIRM, (const struct sockaddr *) &servaddr, sizeof(servaddr)); 
                            break;
                        case '9':
                            sendto(sockfd, emergency_message,strlen(emergency_message), MSG_CONFIRM, (const struct sockaddr *) &servaddr, sizeof(servaddr));
                            break;
                        
                    }
                    printf("\n\rChoose a simulation scenario:\n-1 is ID checkup on Database\n-2 is temperature data\n-9 is emergency\n");
                }
            }
        }
    }

    close(sockfd);
    return 0;
}
