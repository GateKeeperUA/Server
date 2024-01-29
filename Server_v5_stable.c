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
#include <sqlite3.h>
   
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

volatile int ID_counter=1;
volatile struct clients client[MaxClients]={};


int Create_DataBase() {
    char* err;
    sqlite3* db;
    sqlite3_stmt* stmt;
    sqlite3_open("SQLite/IP.db", &db);
    if(sqlite3_exec(db,"CREATE TABLE IF NOT EXISTS IP(ID INT,IP1 varchar(3),IP2 varchar(3),IP3 varchar(3),IP4 varchar(3),IPsize varchar(2),counter INT);",NULL,NULL,&err) != SQLITE_OK) {
        printf("\nError %s\n",err);
        return 0;
    }
    sqlite3_close(db);
    printf("\nDataBase creation complete\n");
    return 1;
}

int Find_DataBase(char *IP) {
    char* err;
    sqlite3* db;
    sqlite3_stmt* stmt;
    sqlite3_open("SQLite/IP.db", &db);

    sqlite3_prepare_v2(db,"select ID,IP1,IP2,IP3,IP4,IPsize,counter from IP",-1,&stmt,NULL);
    const unsigned char *aux;
    char *findIP;
    int find=0;
    while(sqlite3_step(stmt) != SQLITE_DONE) {
        aux = sqlite3_column_text(stmt,1);
        findIP = (char*)aux;
        strcat(findIP,".");
        aux = sqlite3_column_text(stmt,2);
        strcat(findIP,aux);
        strcat(findIP,".");
        aux = sqlite3_column_text(stmt,3);
        strcat(findIP,aux);
        strcat(findIP,".");
        aux = sqlite3_column_text(stmt,4);
        strcat(findIP,aux);
        if(strcmp(findIP,IP)==0){
            find = sqlite3_column_int(stmt,0);
            break;
        }
    }
    sqlite3_reset(stmt);
    sqlite3_close(db);
    return find;
}

int Add_DataBase(char* IP) {
    char* err;
    sqlite3* db;
    sqlite3_open("SQLite/IP.db", &db);

    for(int i=0;i<strlen(IP);i++){if(IP[i]=='.')IP[i]=',';}
    char query[100];
    sprintf(query,"insert into IP VALUES(%d,%s,%ld,%d);",ID_counter,IP,strlen(IP),0);

    if(sqlite3_exec(db,query,NULL,NULL,&err) != SQLITE_OK) {
        printf("\nError %s\n",err);
        return 0;
    }
    sqlite3_close(db);
    ID_counter++;
    return 1;
}

int Initialize_DataBase() {
    char* err;
    sqlite3* db;
    sqlite3_stmt* stmt;
    sqlite3_open("SQLite/IP.db", &db);

    sqlite3_prepare_v2(db,"select ID,IP1,IP2,IP3,IP4,IPsize,counter from IP",-1,&stmt,NULL);
    int find=0;
    while(sqlite3_step(stmt) != SQLITE_DONE) {
        find = sqlite3_column_int(stmt,0);
    }
    sqlite3_reset(stmt);
    sqlite3_close(db);
    return find+1;
}

int UpdateCounter_DataBase(int counter,int ID) {
    char* err;
    char query[100];
    sqlite3* db;
    sqlite3_stmt* stmt;
    sqlite3_open("SQLite/IP.db", &db);

    sprintf(query,"UPDATE IP SET counter = %d Where ID = %d",counter,ID);
    if(sqlite3_exec(db,query,NULL,NULL,&err) != SQLITE_OK) {
        printf("\nError %s\n",err);
        return 0;
    }
    sqlite3_close(db);
    return 1;
}

int ReadCounter_DataBase(int ID) {
    char* err;
    sqlite3* db;
    sqlite3_stmt* stmt;
    sqlite3_open("SQLite/IP.db", &db);

    sqlite3_prepare_v2(db,"select ID,IP1,IP2,IP3,IP4,IPsize,counter from IP",-1,&stmt,NULL);

    int find;
    for (int i=1;i<=ID;i++) {
        sqlite3_step(stmt);
    }
    find = sqlite3_column_int(stmt,6);
    sqlite3_reset(stmt);
    sqlite3_close(db);
    return find;    
}
int Initialize(){
    char *filename = "Encryption_keys.txt";
    char dummy[keyLen];
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("Error: could not open file %s", filename);
        return 0;
    }
    for(int i=0;i<numkeys;i++){
        fgets((char *)key[i],keyLen,fp);
        fgets(dummy,keyLen,fp);
    }
    fclose(fp);
    printf("Keys upload complete\n");

    ID_counter = Initialize_DataBase();
    return 1;
}
   
void fill_dummy(int start, char* data) {
    for(int i=start;i<keyLen-1;i++) {data[i] = 97+rand()%20;}
    data[keyLen-1]='\0';
}
   
int XORCipher(char* data, int cifer, int ID) {
    int counter;
    counter = ReadCounter_DataBase(ID);
    if(cifer==1){
        for(int i=0;i<strlen(data);i++) {message_cifer[i]=data[i];}
        message_cifer[strlen(data)]='\0';
        fill_dummy(strlen(data)+1,(char*)message_cifer);
    }
    else {
        for(int i=0;i<keyLen;i++) {message_cifer[i]=data[i];}
    }

	for (int i=0;i<keyLen;++i) {
        message_cifer[i] = message_cifer[i] ^ (long int)key[counter][i];
        if(message_cifer[i]=='\0' && cifer==0) {break;}
	}

    if(counter==numkeys-1){counter = 0;}
    else {counter++;}

    if(UpdateCounter_DataBase(counter,ID)==0) {return 0;};
    return 1;
}


int main() {
    int sockfd,ID;
    char buffer[keyLen];
    struct sockaddr_in servaddr, cliaddr;
    int len;    
    bool restart;
    char* message_init="_%&hqt6G+WuXa4oq*uISC?V20k{gpRgcE&#G_0A62rua7vEoc*2+JrZuHaW*ZSr!=LT=yVK)ef-)w5p[gjyI{emT4nk=C*%QKQ#[Tuk}HQ0){ISk#JYrxUJ8UO-m&xq";
    char* message_test = "The temperatue is 25ºC!";

    if(Create_DataBase()==0){return 1;}
    if(Initialize()==0){return 1;}    
       
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
    printf("Socket creation complete\n\n");
       
    len = sizeof(cliaddr);

    while(1){
        memset(buffer, 0, sizeof(buffer));
        recvfrom(sockfd, buffer, keyLen, MSG_WAITALL, ( struct sockaddr *) &cliaddr, &len);
        if(strcmp(buffer,message_init)==0) {restart = true;}
        else {restart = false;}   
        
        
        ID = Find_DataBase(inet_ntoa(cliaddr.sin_addr));       
        if(ID==0) {
            if(Add_DataBase(inet_ntoa(cliaddr.sin_addr))==0){return 1;};
            printf("New client (nº%d) %s:%d -> Has enter\n", ID_counter-1, inet_ntoa(cliaddr.sin_addr), htons(cliaddr.sin_port));
        }
        else {
            if(restart==false) {
                if(XORCipher(buffer,0,ID)==0) {return 1;}
                printf("Existing client (nº%d) %s:%d -> %s\n", ID, inet_ntoa(cliaddr.sin_addr), htons(cliaddr.sin_port), message_cifer);
                if(XORCipher((char*)message_test,1,ID)==0) {return 1;}
            }
            else {
                if(UpdateCounter_DataBase(0,ID)==0) {return 1;}
                printf("Existing client (nº%d) %s:%d -> Has enter\n", ID, inet_ntoa(cliaddr.sin_addr), htons(cliaddr.sin_port));
            }
        }

        if(restart == false) {
            sendto(sockfd, (char*)message_cifer, keyLen, MSG_CONFIRM, (const struct sockaddr *) &cliaddr, len);
        }
        else {
            sendto(sockfd, (char*)message_init, keyLen, MSG_CONFIRM, (const struct sockaddr *) &cliaddr, len);   
        }
    }
    return 0;
}