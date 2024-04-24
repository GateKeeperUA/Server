// Server side implementation of UDP client-server model

#include "Server.h"

char key[numkeys][keyLen];
char message_cipher[keyLen];
struct sockaddr_in servaddr, cliaddr;
int sockfd;
char* emergency_message = "9Emergency";

int Create_DataBase_IP() {
    char* err;
    sqlite3* db;
    sqlite3_stmt* stmt;
    if (sqlite3_open("SQLite/IP.db", &db)!=0) {
        return 0;
    }
    if(sqlite3_exec(db,"CREATE TABLE IF NOT EXISTS IP(Serial TEXT,Room INT,IP TEXT,Port INT,Counter INT,Permission INT);",NULL,NULL,&err) != SQLITE_OK) {
        printf("\nError creating IP %s\n",err);
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
    if(sqlite3_exec(db,"CREATE TABLE IF NOT EXISTS Data(Temperature INT,Pressure INT,Humidity INT,Gas INT,Occupation INT,Room INT);",NULL,NULL,&err) != SQLITE_OK) {
        printf("\nError creating Data%s\n",err);
        return 0;
    }
    sqlite3_close(db);
    printf("Data DataBase creation complete\n");
    return 1;
}

int Create_DataBase_ID() {
    char* err;
    sqlite3* db;
    sqlite3_stmt* stmt;
    if(sqlite3_open("SQLite/ID.db", &db)!=0) {
        return 0;
    }
    if(sqlite3_exec(db,"CREATE TABLE IF NOT EXISTS ID(UID TEXT,NMEC INT,Permission INT);",NULL,NULL,&err) != SQLITE_OK) {
        printf("\nError creating ID %s\n",err);
        return 0;
    }
    sqlite3_close(db);
    printf("ID DataBase creation complete\n");
    return 1;
}

int Create_DataBase_Log() {
    char* err;
    sqlite3* db;
    sqlite3_stmt* stmt;
    if(sqlite3_open("SQLite/Log.db", &db)!=0) {
        return 0;
    }
    if(sqlite3_exec(db,"CREATE TABLE IF NOT EXISTS LOG(Room INT,NMEC INT,Date TEXT);",NULL,NULL,&err) != SQLITE_OK) {
        printf("\nError creating Log %s\n",err);
        return 0;
    }
    sqlite3_close(db);
    printf("Log DataBase creation complete\n");
    return 1;
}

int Find_IP_in_DataBase_IP(char *IP) {
    char* err;
    sqlite3* db;
    sqlite3_stmt* stmt;
    if(sqlite3_open("SQLite/IP.db", &db)!=0) {
        return 0;
    }

    sqlite3_prepare_v2(db,"select Serial,Room,IP,Port,Counter,Permission from IP",-1,&stmt,NULL);
    const unsigned char *findIP;
    int room=0;
    while(sqlite3_step(stmt) != SQLITE_DONE) {
        findIP = sqlite3_column_text(stmt,2);
        if(strcmp(findIP,IP)==0){
            room = sqlite3_column_int(stmt,1);
            break;
        }
    }
    sqlite3_reset(stmt);
    sqlite3_close(db);
    return room;
}

int Find_Serial_in_DataBase_IP(char* serial) {
    int room=0;
    char* err;
    sqlite3* db;
    sqlite3_stmt* stmt;
    if(sqlite3_open("SQLite/IP.db", &db)!=0) {
        return 0;
    }

    sqlite3_prepare_v2(db,"select Serial,Room,IP,Port,Counter,Permission from IP",-1,&stmt,NULL);
    const unsigned char* find;
    while(sqlite3_step(stmt) != SQLITE_DONE) {
        find = sqlite3_column_text(stmt,0);
        if(strcmp(find,serial)==0){
            room = sqlite3_column_int(stmt,1);
            break;
        }
    }
    sqlite3_reset(stmt);
    sqlite3_close(db);
    return room;
}

int Add_to_DataBase_IP(char* serial_num, int room, char* IP,int port) {
    char* err;
    sqlite3* db;
    if(sqlite3_open("SQLite/IP.db", &db)!=0) {
        return 0;
    }

    char query[100];
    sprintf(query,"insert into IP VALUES('%s',%d,'%s',%d,%d,%d);",serial_num,room,IP,port,0,3);
    //! 0 - not a DETI student
    //! 1 - DETI setudent
    //! 2 - DETI worker
    //! 3 - admin

    if(sqlite3_exec(db,query,NULL,NULL,&err) != SQLITE_OK) {
        printf("\nError adding info %s\n",err);
        if(strcmp(err,"disk I/O error")!=0){
            return 0;
        }  
    }
    sqlite3_close(db);
    return 1;
}

int Add_to_DataBase_Data(int temperature, int pressure, int humidity, int gas, int occupation, int room) {
    int find=0;
    char query[100];
    char* err;
    sqlite3* db;
    sqlite3_stmt* stmt;
    if(sqlite3_open("SQLite/Data.db", &db)!=0) {
        return 0;
    }
    sqlite3_prepare_v2(db,"select Temperature,Pressure,Humidity,Gas,Occupation,Room from Data",-1,&stmt,NULL);
    while(sqlite3_step(stmt) != SQLITE_DONE) {
        find = sqlite3_column_int(stmt,5);
        if(find==room){
            sprintf(query,"UPDATE DATA SET Temperature = %d, Pressure = %d, Humidity = %d, Gas = %d Where Room = %d",temperature,pressure,humidity,gas,room);
            if(sqlite3_exec(db,query,NULL,NULL,&err) != SQLITE_OK) {
                printf("\nError %s\n",err);
                return 0;
            }
            sprintf(query,"UPDATE DATA SET Occupation = %d Where Room = %d",occupation,room);
            if(sqlite3_exec(db,query,NULL,NULL,&err) != SQLITE_OK) {
                printf("\nError %s\n",err);
                return 0;
            }
            sqlite3_reset(stmt);
            goto DONE;
        }
    }
    sqlite3_reset(stmt);
    sprintf(query,"insert into Data VALUES(%d,%d,%d,%d,%d,%d);",temperature,pressure,humidity,gas,occupation,room);
    if(sqlite3_exec(db,query,NULL,NULL,&err) != SQLITE_OK) {
        printf("\nError %s\n",err);
        return 0;
    }
    
    DONE:
    sqlite3_close(db);
    return 1;
}

int Add_to_DataBase_Log(int room, int NMEC) {
    int month_,space_counter = 0;
    char hour[2], minute[2], second[2], day[2], month[3], year[4];
    char* err;
    sqlite3* db;
    time_t t;
    if(sqlite3_open("SQLite/Log.db", &db)!=0) {
        return 0;
    }
    time(&t);
    char query[100];

    sprintf(query,"insert into Log VALUES(%d,%d,'%s');",room,NMEC,ctime(&t));
    if(sqlite3_exec(db,query,NULL,NULL,&err) != SQLITE_OK) {
        printf("\nError %s\n",err);
        return 0;
    }
    sqlite3_close(db);
    return 1;
}

int Update_Counter_in_DataBase_IP(int counter,int room) {
    char* err;
    char query[100];
    sqlite3* db;
    sqlite3_stmt* stmt;
    if(sqlite3_open("SQLite/IP.db", &db)!=0) {
        return 0;
    }

    sprintf(query,"UPDATE IP SET Counter = %d Where Room = %d",counter,room);
    if(sqlite3_exec(db,query,NULL,NULL,&err) != SQLITE_OK) {
        printf("\nError updating counter %s\n",err);
        if(strcmp(err,"disk I/O error")!=0){
            return 0;
        }  
    }
    sqlite3_close(db);
    return 1;
}

int Update_Info_in_DataBase_IP(char* serial, int room, char* IP, int port) {
    char IP_[4];
    int IP1=0,IP2=0,IP3=0;
    char* err;
    char query[100];
    sqlite3* db;
    if(sqlite3_open("SQLite/IP.db", &db)!=0) {
        return 0;
    }
    sprintf(query,"UPDATE IP SET Room = %d, IP = '%s', Port = %d, Counter = %d Where Serial = '%s'",room,IP,port,0,serial);
    if(sqlite3_exec(db,query,NULL,NULL,&err) != SQLITE_OK) {
        printf("\nError updating info %s\n",err);
        if(strcmp(err,"disk I/O error")!=0){
            return 0;
        }  
    }
    sqlite3_close(db);
    return 1;
}

int Read_Counter_in_DataBase_IP(int room) {
    int counter;
    char* err;
    sqlite3* db;
    sqlite3_stmt* stmt;
    if(sqlite3_open("SQLite/IP.db", &db)!=0) {
        return 0;
    }

    sqlite3_prepare_v2(db,"select Serial,Room,IP,Port,Counter,Permission from IP",-1,&stmt,NULL);
    do {
        sqlite3_step(stmt);
    }while(sqlite3_column_int(stmt,1)!=room);
    counter = sqlite3_column_int(stmt,4);
    sqlite3_reset(stmt);
    sqlite3_close(db);
    return counter;    
}

int Check_UID_in_DataBase_ID(char* UID, int room) {
    char* err;
    sqlite3* db;
    sqlite3_stmt* stmt;

    if(sqlite3_open("SQLite/IP.db", &db)!=0) {
        return 0;
    }

    sqlite3_prepare_v2(db,"select Serial,Room,IP,Port,Counter,Permission from IP",-1,&stmt,NULL);

    int find_room, permission;
    while(sqlite3_step(stmt) != SQLITE_DONE) {
        find_room=sqlite3_column_int(stmt,1);
        if(find_room==room){
            permission=sqlite3_column_int(stmt,5);
        }
    }

    sqlite3_reset(stmt);
    sqlite3_close(db);


    if(sqlite3_open("SQLite/ID.db", &db)!=0) {
        return 0;
    }

    sqlite3_prepare_v2(db,"select UID,NMEC,Permission from ID",-1,&stmt,NULL);

    const unsigned char *find_UID="";
    while(sqlite3_step(stmt) != SQLITE_DONE) {
        find_UID = sqlite3_column_text(stmt,0);
        if(strcmp(find_UID,UID)==0){
            if(sqlite3_column_int(stmt,2)>=permission) {
                goto ALLOWED;
            }
            else {
                goto NOT_ALLOWED;
            }
            
        }
    }

    NOT_ALLOWED:
    sqlite3_reset(stmt);
    sqlite3_close(db);
    return 2;
    
    ALLOWED:
    sqlite3_reset(stmt);
    sqlite3_close(db);
    return 1;    
}

int Read_NMEC_in_DataBase_ID(char* UID) {
    char* err;
    sqlite3* db;
    sqlite3_stmt* stmt;
    int NMEC;

    if(sqlite3_open("SQLite/ID.db", &db)!=0) {
        return 0;
    }

    sqlite3_prepare_v2(db,"select UID,NMEC,Permission from ID",-1,&stmt,NULL);

    const unsigned char *find_UID="";
    while(sqlite3_step(stmt) != SQLITE_DONE) {
        find_UID = sqlite3_column_text(stmt,0);
        if(strcmp(find_UID,UID)==0){
            NMEC = sqlite3_column_int(stmt,1);
            
        }
    }
    sqlite3_reset(stmt);
    sqlite3_close(db);
    return NMEC;    
}

int Initialize(){
    if(Create_DataBase_IP()==0){return 0;}
    if(Create_DataBase_ID()==0){return 0;}
    if(Create_DataBase_Data()==0){return 0;}
    if(Create_DataBase_Log()==0){return 0;}

    char *filename = "Encryption_keys.txt";
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("Error: could not open file %s", filename);
        return 0;
    }
    char line[keyLen*2];
    for(int i=0;fgets(line, keyLen*2, fp) != NULL;i++) {
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
   
int XORCipher(char* data, bool send, int room, char type) {
    int counter = Read_Counter_in_DataBase_IP(room);
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

int Receive_Data(char* data, int room) {
    int temperature=0,pressure=0,humidity=0,gas=0,occupation=0,i=1,j=0,k=0;
    char aux[10];
    for(int i=1;data[i-1]!='\0';i++,k++) {
        if(data[i]!=' ') {
            aux[k]=data[i];
        }
        else {
            switch(j) {
                case 0:
                    temperature = atoi(aux);
                    printf("%.2fºC ",(float)temperature/100);
                    break;
                case 1:
                    pressure = atoi(aux);
                    printf("%.2fhPA ",(float)pressure/100);
                    
                    break;
                case 2:
                    humidity = atoi(aux);
                    printf("%.2f%% ",(float)humidity/1000);
                    break;
                case 3:
                    gas = atoi(aux);
                    printf("%dIAQ ",gas);
                    break;
                case 4:
                    occupation = atoi(aux);
                    printf("%d Students\n",occupation);
                    break;
            }
            j++;
            k=-1;
            memset(aux, 0, 10);
        }
    }

    if(Add_to_DataBase_Data(temperature,pressure,humidity,gas,occupation,room)==0) {return 0;}   
    return 1;
}

void Send_Emergency(){
    char* err;
    sqlite3* db;
    sqlite3_stmt* stmt;
    int NMEC;

    RETRY:
    if(sqlite3_open("SQLite/IP.db", &db)!=0) {
        goto RETRY;
    }
    sqlite3_prepare_v2(db,"select Serial,Room,IP,Port,Counter,Permission from IP",-1,&stmt,NULL);
    while(sqlite3_step(stmt) != SQLITE_DONE) {
        cliaddr.sin_family      = AF_INET;
        cliaddr.sin_addr.s_addr = inet_addr(sqlite3_column_text(stmt,2));
        cliaddr.sin_port        = htons(sqlite3_column_int(stmt,3));
        sendto(sockfd, emergency_message, strlen(emergency_message), MSG_CONFIRM, (const struct sockaddr *) &cliaddr, sizeof(cliaddr));
    }
    sqlite3_reset(stmt);
    sqlite3_close(db);
    printf("Emergency warning sent for every Room\n");
}

int main() {
    int find, room, key_counter, permission_UID, nmec;
    char buffer[keyLen];
    int len;    
    bool restart;
    char serial_num[6], room_[3];
    char* message_init = "%&hqt6G+WuXa4oq*uISC?V20k{gpRgcE&#G_0A62rua7vEoc*2+JrZuHaW*ZSr!=LT=yVK)ef-)w5p[gjyI{emT4nk=C*%QKQ#[Tuk}HQ0){ISk#JYrxUJ8UO-";
    char* confirmation = "4jqz484yl94neddq0twxugnnyty6imjyc5zdeyyizl636mvk48pi1as8fnyc01a9lj3mamlp4jdcmjfviw48uv7fv4mv52gq75atzpus853ov2n8phy59cy3a77wp"; 
    char* denial = "931ghxbwti34tq3fzyc0wqxjbq92v9hrjlzndm3xdbgjc2131ouyxxx7dm4rt7tzbd0x9ij6lq5wbm2n1nq0x7ikoavivpu34sditd3i3opuxsfi2r1gzkojgjo96";

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
    servaddr.sin_addr.s_addr = inet_addr(IP_server);
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
        memset(serial_num,0,sizeof(serial_num));
        recvfrom(sockfd, buffer, keyLen, MSG_WAITALL, ( struct sockaddr *) &cliaddr, &len); 
        
        //! 0 is new connection
        //! 1 is UID checkup on Database
        //! 2 is temperature data
        //! 9 is emergency

        
        switch (buffer[0]){
            case '0':
                for(int i=1;i<7;i++){
                    sprintf(serial_num,"%s%c",serial_num,buffer[i]);
                }
                sprintf(room_,"%c%c%c",buffer[7],buffer[8],buffer[9]);
                room = atoi(room_);
                find = Find_Serial_in_DataBase_IP(serial_num);
                
                for(int i=0;i<keyLen-10;i++) {if(buffer[i+10]!=message_init[i]) {goto NOT_RESET;}}
                    sendto(sockfd, (char*)message_init, keyLen, MSG_CONFIRM, (const struct sockaddr *) &cliaddr, len);
                    if (find==0){
                        if(Add_to_DataBase_IP(serial_num,room,inet_ntoa(cliaddr.sin_addr),htons(cliaddr.sin_port))==0){return 1;};
                        printf("Client (nº%d) %s:%d -> Has entered\n", room, inet_ntoa(cliaddr.sin_addr), htons(cliaddr.sin_port));
                    }
                    else {
                        if(Update_Info_in_DataBase_IP(serial_num,room,inet_ntoa(cliaddr.sin_addr),htons(cliaddr.sin_port))==0) {return 1;}    
                        printf("Client (nº%d) %s:%d -> Has entered\n", room, inet_ntoa(cliaddr.sin_addr), htons(cliaddr.sin_port));
                    }
                                
                break;
                NOT_RESET:
                sendto(sockfd, denial, strlen(denial), MSG_CONFIRM, (const struct sockaddr *) &cliaddr, len);
                break;

            case '1':
                room = Find_IP_in_DataBase_IP(inet_ntoa(cliaddr.sin_addr));
                if (room>0) {
                    XORCipher(buffer,false,room,'1');
                    permission_UID = Check_UID_in_DataBase_ID(message_cipher,room);
                    nmec = Read_NMEC_in_DataBase_ID(message_cipher);
                    if(permission_UID==0) {
                        return 1;
                    }
                    else if(permission_UID==1){
                        printf("Client (nº%d) %s:%d -> %s can enter\n", room, inet_ntoa(cliaddr.sin_addr), htons(cliaddr.sin_port), message_cipher);
                        key_counter = XORCipher(confirmation,true,room,'1');
                        if(Add_to_DataBase_Log(room,nmec)==0){return 1;}
                    }
                    else{
                        printf("Client (nº%d) %s:%d -> %s can't enter\n", room, inet_ntoa(cliaddr.sin_addr), htons(cliaddr.sin_port), message_cipher);
                        key_counter = XORCipher(denial,true,room,'1'); 
                    }

                    sendto(sockfd, (char*)message_cipher, keyLen, MSG_CONFIRM, (const struct sockaddr *) &cliaddr, len);
                    if(Update_Counter_in_DataBase_IP(key_counter,room)==0) {return 1;};
                }
                break;

            case '2':
                room = Find_IP_in_DataBase_IP(inet_ntoa(cliaddr.sin_addr));
                if (room>0) {
                    printf("%s\n",buffer);
                    printf("Data in room of client (nº%d) %s:%d is ", room, inet_ntoa(cliaddr.sin_addr), htons(cliaddr.sin_port));
                    if(Receive_Data(buffer,room)==0) {return 1;}
                }
                break;

            case '9':
                Send_Emergency();
                break;
        }
    }
    return 0;
}