// Server side implementation of UDP client-server model

#include "Server.h"

char key[numkeys][keyLen];
char message_cipher[keyLen];


int Create_DataBase_IP() {
    char* err;
    sqlite3* db;
    sqlite3_stmt* stmt;
    if (sqlite3_open("SQLite/IP.db", &db)!=0) {
        return 0;
    }
    if(sqlite3_exec(db,"CREATE TABLE IF NOT EXISTS IP(ID INT,IP1 varchar(3),IP2 varchar(3),IP3 varchar(3),IP4 varchar(3),Counter INT,Permission INT);",NULL,NULL,&err) != SQLITE_OK) {
        printf("\nError %s\n",err);
        return 0;
    }
    sqlite3_close(db);
    printf("\nIP DataBase creation complete\n");
    return 1;
}

int Create_DataBase_Data() {
    char* err;
    sqlite3* db;
    sqlite3_stmt* stmt;
    if(sqlite3_open("SQLite/Data.db", &db)!=0) {
        return 0;
    }
    if(sqlite3_exec(db,"CREATE TABLE IF NOT EXISTS Data(Temperature INT,Pressure INT,Humidity INT,Gas INT, ID INT);",NULL,NULL,&err) != SQLITE_OK) {
        printf("\nError %s\n",err);
        return 0;
    }
    sqlite3_close(db);
    printf("Data DataBase creation complete\n");
    return 1;
}

int Find_in_DataBase(char *IP) {
    char* err;
    sqlite3* db;
    sqlite3_stmt* stmt;
    if(sqlite3_open("SQLite/IP.db", &db)!=0) {
        return 0;
    }

    sqlite3_prepare_v2(db,"select ID,IP1,IP2,IP3,IP4,Counter,Permission from IP",-1,&stmt,NULL);
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

int Add_to_DataBase_IP(char* IP, int ID, int permission) {
    char* err;
    sqlite3* db;
    if(sqlite3_open("SQLite/IP.db", &db)!=0) {
        return 0;
    }

    for(int i=0;i<strlen(IP);i++){if(IP[i]=='.')IP[i]=',';}
    char query[100];
    sprintf(query,"insert into IP VALUES(%d,%s,%ld,%d);",ID,IP,0,permission);
    //! 0 - not a student
    //! 1 - DETI setudent
    //! 2 - DETI worker
    //! 3 - admin

    if(sqlite3_exec(db,query,NULL,NULL,&err) != SQLITE_OK) {
        printf("\nError %s\n",err);
        return 0;
    }
    sqlite3_close(db);
    return 1;
}

int Add_to_DataBase_Data(int temperature, int pressure, int humidity, int gas, int ID) {
    char* err;
    sqlite3* db;
    if(sqlite3_open("SQLite/Data.db", &db)!=0) {
        return 0;
    }

    char query[100];
    sprintf(query,"insert into Data VALUES(%d,%d,%d,%d,%d);",temperature,pressure,humidity,gas,ID);

    if(sqlite3_exec(db,query,NULL,NULL,&err) != SQLITE_OK) {
        printf("\nError %s\n",err);
        return 0;
    }
    sqlite3_close(db);
    return 1;
}

/*int Initialize_DataBase() {
    char* err;
    sqlite3* db;
    sqlite3_stmt* stmt;
    if(sqlite3_open("SQLite/Server.db", &db)!=0) {
        return 0;
    }

    sqlite3_prepare_v2(db,"select ID,IP1,IP2,IP3,IP4,Counter,Permission from IP",-1,&stmt,NULL);
    int find=0;
    while(sqlite3_step(stmt) != SQLITE_DONE) {
        find = sqlite3_column_int(stmt,0);
    }
    sqlite3_reset(stmt);
    sqlite3_close(db);
    return find+1;
}*/

int UpdateCounter_DataBase(int counter,int ID) {
    char* err;
    char query[100];
    sqlite3* db;
    sqlite3_stmt* stmt;
    if(sqlite3_open("SQLite/IP.db", &db)!=0) {
        return 0;
    }

    sprintf(query,"UPDATE IP SET Counter = %d Where ID = %d",counter,ID);
    if(sqlite3_exec(db,query,NULL,NULL,&err) != SQLITE_OK) {
        printf("\nError %s\n",err);
        return 0;
    }
    sqlite3_close(db);
    return 1;
}

int UpdatePermission_DataBase(int permission, int ID) {
    char* err;
    char query[100];
    sqlite3* db;
    sqlite3_stmt* stmt;
    if(sqlite3_open("SQLite/IP.db", &db)!=0) {
        return 0;
    }
    sprintf(query,"UPDATE IP SET Permission = %d Where ID = %d",permission,ID);
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
    if(sqlite3_open("SQLite/IP.db", &db)!=0) {
        return 0;
    }

    sqlite3_prepare_v2(db,"select ID,IP1,IP2,IP3,IP4,Counter,Permission from IP",-1,&stmt,NULL);

    int find;
    do {
        sqlite3_step(stmt);
    }while(sqlite3_column_int(stmt,0)!=ID);
    find = sqlite3_column_int(stmt,5);
    sqlite3_reset(stmt);
    sqlite3_close(db);
    return find;    
}

int Initialize(){
    if(Create_DataBase_IP()==0){return 0;}
    if(Create_DataBase_Data()==0){return 0;}

    char *filename = "Encryption_keys.txt";
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("Error: could not open file %s", filename);
        return 0;
    }
    char line[keyLen+1];
    for(int i=0;fgets(line, keyLen+1, fp) != NULL;i++) {
        for(int j=0;j<keyLen;j++) {
            key[i][j] = line[j];
        }
    }

    fclose(fp);
    printf("Keys upload completed\n");

    return 1;
}
   
void fill_dummy(int start, char* data) {
    for(int i=start;i<keyLen-1;i++) {data[i] = rand()%126;}
    data[keyLen-1]='\0';
}
   
int XORCipher(char* data, bool send, int ID, char type) {
    int counter = ReadCounter_DataBase(ID);
    if(send==true){
        message_cipher[0] = type;
        for(int i=0;i<strlen(data);i++) {message_cipher[i+1]=data[i];}
        message_cipher[strlen(data)+1]='\0';
        fill_dummy(strlen(data)+2,(char*)message_cipher);
        
        if(counter==numkeys-1){counter = 0;}
        else {counter++;}

        for (int i=1;i<keyLen-1;i++) {
            message_cipher[i] = message_cipher[i] ^ key[counter][i-1];
        }
	}

    else {
        for(int i=1;i<keyLen;i++) {message_cipher[i-1]=data[i];}

        for (int i=0;i<keyLen-1;i++) {
            message_cipher[i] = message_cipher[i] ^ key[counter][i];
            if(message_cipher[i]=='\0') {break;}
        }
    }	

    if(counter==numkeys-1){counter = 0;}
    else {counter++;}

    return counter;
}

int receive_data(char* data, int ID) {
    int temperature=0,pressure=0,humidity=0,gas=0,i=1,j=0,k=0;
    char aux[10];
    for(int i=1;data[i-1]!='\0';i++,k++) {
        if(data[i]!=' ') {
            aux[k]=data[i];
        }
        else {
            switch(j) {
                case 0:
                    temperature = atoi(aux);
                    printf("%dºC ",temperature);
                    break;
                case 1:
                    pressure = atoi(aux);
                    printf("%dhPA ",pressure);
                    
                    break;
                case 2:
                    humidity = atoi(aux);
                    printf("%d%% ",humidity);
                    break;
                case 3:
                    gas = atoi(aux);
                    printf("%dgas\n",gas);
                    break;
            }
            j++;
            k=-1;
            memset(aux, 0, 10);
        }
    }

    if(Add_to_DataBase_Data(temperature,pressure,humidity,gas,ID)==0) {return 0;}
    

    return 1;
}

int main() {
    int sockfd, ID, key_counter;
    char buffer[keyLen];
    struct sockaddr_in servaddr, cliaddr;
    int len;    
    bool restart;
    char ID_[3];
    char* message_init="%&hqt6G+WuXa4oq*uISC?V20k{gpRgcE&#G_0A62rua7vEoc*2+JrZuHaW*ZSr!=LT=yVK)ef-)w5p[gjyI{emT4nk=C*%QKQ#[Tuk}HQ0){ISk#JYrxUJ8UO-";
    char* message_ID = "In DataBase";

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
    servaddr.sin_addr.s_addr = inet_addr("192.168.1.86");
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
        memset((char*) message_cipher, 0, sizeof(message_cipher));
        memset(buffer, 0, sizeof(buffer));
        recvfrom(sockfd, buffer, keyLen, MSG_WAITALL, ( struct sockaddr *) &cliaddr, &len); 
        
        //! 0 is new connection
        //! 1 is ID checkup on Database
        //! 2 is temperature data

        ID = Find_in_DataBase(inet_ntoa(cliaddr.sin_addr));
        switch (buffer[0]){
            case '0':
                sprintf(ID_,"%c%c%c",buffer[1],buffer[2],buffer[3]);
                for(int i=0;i<keyLen-5;i++) {if(buffer[i+5]!=message_init[i]) {goto NOT_RESET;}}
                    sendto(sockfd, (char*)message_init, keyLen, MSG_CONFIRM, (const struct sockaddr *) &cliaddr, len);
                    if (ID==0){
                        if(Add_to_DataBase_IP(inet_ntoa(cliaddr.sin_addr),atoi(ID_),buffer[4]-'0')==0){return 1;};
                        printf("Client (nº%d) %s:%d -> Has entered\n", atoi(ID_), inet_ntoa(cliaddr.sin_addr), htons(cliaddr.sin_port));
                    }
                    else {
                        if(UpdateCounter_DataBase(0,ID)==0) {return 1;}
                        if(UpdatePermission_DataBase(buffer[4]-'0',ID)==0) {return 1;}    
                        printf("Client (nº%d) %s:%d -> Has entered\n", atoi(ID_), inet_ntoa(cliaddr.sin_addr), htons(cliaddr.sin_port));
                    }
                                
                break;
                NOT_RESET:
                sendto(sockfd, "Not initialized", 15, MSG_CONFIRM, (const struct sockaddr *) &cliaddr, len);
                break;

            case '1':
                if (ID>0) {
                    XORCipher(buffer,false,ID,'1');
                    printf("Client (nº%d) %s:%d -> %s\n", ID, inet_ntoa(cliaddr.sin_addr), htons(cliaddr.sin_port), message_cipher);
                    key_counter = XORCipher((char*)message_ID,true,ID,'1');

                    sendto(sockfd, (char*)message_cipher, keyLen, MSG_CONFIRM, (const struct sockaddr *) &cliaddr, len);
                    if(UpdateCounter_DataBase(key_counter,ID)==0) {return 1;};
                }
                break;

            case '2':
                if (ID>0) {
                    printf("Data in room of client (nº%d) %s:%d is ", ID, inet_ntoa(cliaddr.sin_addr), htons(cliaddr.sin_port));
                    if(receive_data(buffer,ID)==0) {return 1;}
                }
        }
    }
    return 0;
}