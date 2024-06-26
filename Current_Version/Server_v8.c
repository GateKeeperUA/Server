#include "Server.h"

char key[numkeys][keyLen];
char message_cipher[keyLen];
char buffer[keyLen];
struct sockaddr_in servaddr, cliaddr;
int sockfd;
struct mosquitto * mosq;
char UID_recon[8];
// Confirmation string
char* confirmation = "4jqz484yl94neddq0twxugnnyty6imjyc5zdeyyizl636mvk48pi1as8fnyc01a9lj3mamlp4jdcmjfviw48uv7fv4mv52gq75atzpus853ov2n8phy59cy3a77wp"; 
// Denial string
char* denial = "931ghxbwti34tq3fzyc0wqxjbq92v9hrjlzndm3xdbgjc2131ouyxxx7dm4rt7tzbd0x9ij6lq5wbm2n1nq0x7ikoavivpu34sditd3i3opuxsfi2r1gzkojgjo96";
// Initialization strong
char* message_init = "%&hqt6G+WuXa4oq*uISC?V20k{gpRgcE&#G_0A62rua7vEoc*2+JrZuHaW*ZSr!=LT=yVK)ef-)w5p[gjyI{emT4nk=C*%QKQ#[Tuk}HQ0){ISk#JYrxUJ";


// Struct to store room, UID and time received by each room related to facial recognition
struct rooms_recognition {
    int room;
    char last_UID[9];
    int last_time;
};

struct rooms_recognition recog[300];
int room_recog_received=0;


// Function to create Table IP (responsible to store IP and info related to the clients) if not created when called. Table with the following collums
// Serial TEXT
// Room INT
// IP TEXT
// Port INT
// Counter INT
// Permission INT
// Ocupation INT
int Create_DataBase_IP() {
    char* err;
    sqlite3* db;
    sqlite3_stmt* stmt;
    if (sqlite3_open("SQLite/IP.db", &db)!=0) {
        return 0;
    }
    if(sqlite3_exec(db,"CREATE TABLE IF NOT EXISTS IP(Serial TEXT,Room INT,IP TEXT,Port INT,Counter INT,Permission INT, Ocupation INT);",NULL,NULL,&err) != SQLITE_OK) {
        printf("\nError creating IP %s\n",err);
        return 0;
    }
    sqlite3_close(db);
    printf("\nIP DataBase creation complete\n");
    return 1;
}

// Function to create Table Data (responsible to store rooms environmental data and occupation) if not created when called. Table with the following collums
// Temperature INT
// Pressure INT
// Humidity INT
// Gas INT
// Occupation INT
// Room INT
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

// Function to create Table ID (responsible to store card UID, struden NMEC and level of permission) if not created when called. Table with the following collums
// UID TEXT
// NMEC INT
// Permission INT
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

// Function to create Table Log (responsible to store every entrance in any door with the day and time) if not created when called. Table with the following collums
// Room INT
// NMEC INT
// Date DATETIME
int Create_DataBase_Log() {
    char* err;
    sqlite3* db;
    sqlite3_stmt* stmt;
    if(sqlite3_open("SQLite/Log.db", &db)!=0) {
        return 0;
    }
    if(sqlite3_exec(db,"CREATE TABLE IF NOT EXISTS LOG(Room INT,NMEC INT,Date DATETIME);",NULL,NULL,&err) != SQLITE_OK) {
        printf("\nError creating Log %s\n",err);
        return 0;
    }
    sqlite3_close(db);
    printf("Log DataBase creation complete\n");
    return 1;
}

// Fnction to find room number based on IP on Table IP
int Find_room_with_IP_in_DataBase_IP(char *IP) {
    char* err;
    sqlite3* db;
    sqlite3_stmt* stmt;
    if(sqlite3_open("SQLite/IP.db", &db)!=0) {
        return 0;
    }

    sqlite3_prepare_v2(db,"select Serial,Room,IP,Port,Counter,Permission,Ocupation from IP",-1,&stmt,NULL);
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

// Function to find room number based on serial number of rooms' boards
int Find_room_with_serial_in_DataBase_IP(char* serial) {
    int room=0;
    char* err;
    sqlite3* db;
    sqlite3_stmt* stmt;
    if(sqlite3_open("SQLite/IP.db", &db)!=0) {
        return 0;
    }

    sqlite3_prepare_v2(db,"select Serial,Room,IP,Port,Counter,Permission,Ocupation from IP",-1,&stmt,NULL);
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

// Function to add to IP table a new entrance (new serial board number)
int Add_to_DataBase_IP(char* serial_num, int room, char* IP,int port) {
    char* err;
    sqlite3* db;
    if(sqlite3_open("SQLite/IP.db", &db)!=0) {
        return 0;
    }

    char query[100];
    sprintf(query,"insert into IP VALUES('%s',%d,'%s',%d,%d,%d,%d);",serial_num,room,IP,port,0,3,0);
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

// Function to add (if new room) or change (if existent already) data related to a certain room
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

// Function to add to table Log a new entrance in a door
int Add_to_DataBase_Log(int room, int NMEC) {
    int month_,space_counter = 0;
    char hour[2], minute[2], second[2], day[2], month[3], year[4];
    char* err;
    sqlite3* db;
    time_t t;
    if(sqlite3_open("SQLite/Log.db", &db)!=0) {
        return 0;
    }
    char query[100];

    sprintf(query,"insert into Log VALUES(%d,%d,DATETIME('now'));",room,NMEC);
    if(sqlite3_exec(db,query,NULL,NULL,&err) != SQLITE_OK) {
        printf("\nError %s\n",err);
        return 0;
    }
    sqlite3_close(db);
    return 1;
}

// Function to update keys counter in table IP
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

// Function to change IP, room and port in table IP based on the serial number (which is constant). ALso updates ocupation and counter to 0
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
    sprintf(query,"UPDATE IP SET Ocupation = %d Where Serial = '%s'",0,serial);
    if(sqlite3_exec(db,query,NULL,NULL,&err) != SQLITE_OK) {
        printf("\nError updating info %s\n",err);
        if(strcmp(err,"disk I/O error")!=0){
            return 0;
        }  
    }
    sqlite3_close(db);
    return 1;
}

// Function to change occupation in table IP based on the room number
int Update_Ocupation_in_DataBase_IP(int ocupation, int room) {
    char* err;
    char query[100];
    sqlite3* db;
    if(sqlite3_open("SQLite/IP.db", &db)!=0) {
        return 0;
    }

    sprintf(query,"UPDATE IP SET Ocupation = %d Where Room = %d",ocupation,room);
    if(sqlite3_exec(db,query,NULL,NULL,&err) != SQLITE_OK) {
        printf("\nError updating info %s\n",err);
        if(strcmp(err,"disk I/O error")!=0){
            return 0;
        }  
    }
    sqlite3_close(db);
    return 1;
}

// Function to read keys counter of table IP based on the room number
int Read_Counter_in_DataBase_IP(int room) {
    int counter;
    char* err;
    sqlite3* db;
    sqlite3_stmt* stmt;
    if(sqlite3_open("SQLite/IP.db", &db)!=0) {
        return 0;
    }

    sqlite3_prepare_v2(db,"select Serial,Room,IP,Port,Counter,Permission,Ocupation from IP",-1,&stmt,NULL);
    do {
        sqlite3_step(stmt);
    }while(sqlite3_column_int(stmt,1)!=room);
    counter = sqlite3_column_int(stmt,4);
    sqlite3_reset(stmt);
    sqlite3_close(db);
    return counter;    
}

// Function to check if the UID (in table ID) provided has permission to enter the room specified (in table IP). If it has, returns 1, if not returns 2
int Check_UID_in_DataBase_ID(char* UID, int room) {
    char* err;
    sqlite3* db;
    sqlite3_stmt* stmt;

    if(sqlite3_open("SQLite/IP.db", &db)!=0) {
        return 0;
    }

    sqlite3_prepare_v2(db,"select Serial,Room,IP,Port,Counter,Permission,Ocupation from IP",-1,&stmt,NULL);

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

// Function to check if a room is in table IP. If it is, returns 1, if not returns 2
int Check_Room_in_DataBase_IP(int room) {
    char* err;
    sqlite3* db;
    sqlite3_stmt* stmt;

    if(sqlite3_open("SQLite/IP.db", &db)!=0) {
        return 0;
    }

    sqlite3_prepare_v2(db,"select Serial,Room,IP,Port,Counter,Permission,Ocupation from IP",-1,&stmt,NULL);

    int find_room;
    while(sqlite3_step(stmt) != SQLITE_DONE) {
        find_room=sqlite3_column_int(stmt,1);
        if(find_room==room){
            goto IN_DATABASE;
        }
    }

    sqlite3_reset(stmt);
    sqlite3_close(db);
    return 2;
    
    IN_DATABASE:
    sqlite3_reset(stmt);
    sqlite3_close(db);
    return 1;    
}

// Function to return NMEC from table ID based on the UID
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

// Function to fill encrypted message with characters untill the size specified in Server.h
void fill_dummy(int start, char* data) {
    for(int i=start;i<keyLen-1;i++) {data[i] = rand()%126;}
    data[keyLen-1]='\0';
}

// Function respossible for encryption/decryption. The encryption/decryption is based on xor between the message we want to send/receive and the key indicated by the key counter
int XORCipher(char* data, bool send, int room, char type) {
    int counter = Read_Counter_in_DataBase_IP(room);
    if(send==true){
        message_cipher[0] = type;
        for(int i=0;i<strlen(data);i++) {message_cipher[i+1]=data[i];}
        message_cipher[strlen(data)+1]='\0';
        fill_dummy(strlen(data)+2,(char*)message_cipher);

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

// Function to return UID from table ID based on NMEC
int Read_UID_in_DataBase_ID(int NMEC) {
    char* err;
    sqlite3* db;
    sqlite3_stmt* stmt;
    const unsigned char* UID;

    if(sqlite3_open("SQLite/ID.db", &db)!=0) {
        return 0;
    }

    sqlite3_prepare_v2(db,"select UID,NMEC,Permission from ID",-1,&stmt,NULL);

    int find_NMEC;
    while(sqlite3_step(stmt) != SQLITE_DONE) {
        find_NMEC = sqlite3_column_int(stmt,1);
        if(find_NMEC==NMEC){
            UID = sqlite3_column_text(stmt,0);
            break;
        }
    }
    strncpy(UID_recon,UID,8);
    sqlite3_reset(stmt);
    sqlite3_close(db);
    return 1;    
}

// Function to send emergency message to every room in table IP
void Send_Emergency(){
    char* emergency_message = "9Emergency";
    char* err;
    sqlite3* db;
    sqlite3_stmt* stmt;
    int NMEC;

    RETRY:
    if(sqlite3_open("SQLite/IP.db", &db)!=0) {
        goto RETRY;
    }
    sqlite3_prepare_v2(db,"select Serial,Room,IP,Port,Counter,Permission,Ocupation from IP",-1,&stmt,NULL);
    while(sqlite3_step(stmt) != SQLITE_DONE) {
        cliaddr.sin_family      = AF_INET;
        cliaddr.sin_addr.s_addr = inet_addr(sqlite3_column_text(stmt,2));
        cliaddr.sin_port        = htons(sqlite3_column_int(stmt,3));
        sendto(sockfd, emergency_message, strlen(emergency_message), MSG_CONFIRM, (const struct sockaddr *) &cliaddr, sizeof(cliaddr));
    }
    sqlite3_reset(stmt);
    sqlite3_close(db);
    printf("Emergency warning sent to every Room\n");
}

// Function to find IP in table IP and port based on room number. ALso fills the cliaddr struct so the message can be sent via UDP to that IP
void Find_IP_with_room_in_DataBase_IP(int room){
    char* err;
    sqlite3* db;
    sqlite3_stmt* stmt;

    if(sqlite3_open("SQLite/IP.db", &db)!=0) {
        return 0;
    }
    sqlite3_prepare_v2(db,"select Serial,Room,IP,Port,Counter,Permission,Ocupation from IP",-1,&stmt,NULL);
    while(sqlite3_step(stmt) != SQLITE_DONE) {
        if(sqlite3_column_int(stmt,1)==room) {
            cliaddr.sin_family      = AF_INET;
            cliaddr.sin_addr.s_addr = inet_addr(sqlite3_column_text(stmt,2));
            cliaddr.sin_port        = htons(sqlite3_column_int(stmt,3));
            break;
        }
    }

    sqlite3_reset(stmt);
    sqlite3_close(db);
}

// Function to handle new messages received by MQTT. IT sorts by topics
// Topic Emergency: sends emergency warning to every room in table IP
// Topic DETI/Authenticate/Enter: checks if the UID received was registered in the last 5 seconds by any room. If it was, sends via MQTT confirmation, if not, sends a denial
// Topic DETI/Authenticate/Recognition: stores the room, UID and time of the message received
void Receive_MQTT(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg) {
    int len;
    time_t t;
    time(&t);

    if(strcmp((char *)msg->payload,"Emergency")==0 && strcmp((char *)msg->topic,"DETI/Emergency")==0) {
        Send_Emergency();
        sleep(1);
        Send_Emergency();
    }
    else if(strcmp((char *)msg->topic,"DETI/Authenticate/Enter")==0) {
        char confirm[9];
        for(int i=0;i<room_recog_received;i++) {
            if(!strcmp(recog[i].last_UID,msg->payload) && t-5<=recog[i].last_time){
                if(Check_UID_in_DataBase_ID(recog[i].last_UID,recog[i].room)==1){
                    printf("%s can enter\n",msg->payload);
                    memset(confirm,0,sizeof(confirm));
                    strcat(confirm,"1");
                    strcat(confirm,msg->payload);
                    mosquitto_publish(mosq,NULL,"DETI/Authenticate/Confirm",9,confirm,2,false);
                    int key_counter = XORCipher(confirmation,true,recog[i].room,'1'); 
                    Find_IP_with_room_in_DataBase_IP(recog[i].room);
                    sendto(sockfd, (char*)message_cipher, keyLen, MSG_CONFIRM, (const struct sockaddr *) &cliaddr, sizeof(cliaddr));
                    Update_Counter_in_DataBase_IP(key_counter,recog[i].room)==0;
                goto CAN_ENTER;
                }
                
            }
        } 
        memset(confirm,0,sizeof(confirm));
        strcat(confirm,"0");
        strcat(confirm,msg->payload);
        mosquitto_publish(mosq,NULL,"DETI/Authenticate/Confirm",9,confirm,2,false);
        printf("%s can't enter\n",msg->payload);

        CAN_ENTER:
    }
    else if(strcmp((char *)msg->topic,"DETI/Authenticate/Recognition")==0) {
        char room_recog[4];
        char NMEC[9];

        strncpy(room_recog,msg->payload,3);
        memcpy(NMEC,msg->payload+3,sizeof(msg->payload));
        Read_UID_in_DataBase_ID(atoi(NMEC));
        if(Check_Room_in_DataBase_IP(atoi(room_recog))==2){
            goto NOT_IN_DATABASE;
        }

        for (int i=0;i<room_recog_received;i++){
            if(recog->room==atoi(room_recog)) {
                memcpy(recog[i].last_UID,UID_recon,sizeof(UID_recon));
                recog[i].last_time = t; 
                goto CHANGED_UID;  
            }
        }

        recog[room_recog_received].room = atoi(room_recog);
        memcpy(recog[room_recog_received].last_UID,UID_recon,sizeof(UID_recon));
        recog[room_recog_received].last_time = t;
        room_recog_received++;

        CHANGED_UID:
        
        printf("Room:%d NMEC:%s Time:%d\n",recog[0].room,recog[0].last_UID,recog[0].last_time);

        NOT_IN_DATABASE:
    }
}

// Function to connect and subscribe topics MQTT. ALso creates a thread to receive messages on that topics
int Create_Thread_MQTT() {
    mosquitto_lib_init();
    mosq = mosquitto_new("Server",true,NULL);
    
    mosquitto_message_callback_set(mosq,Receive_MQTT);

    int rc = mosquitto_connect(mosq,"127.0.0.1",1883,60);
    if(rc!=0) {
        printf("Error\n");
        mosquitto_destroy(mosq);
        return 0;
    }
    mosquitto_subscribe(mosq,NULL,"DETI/Emergency",2);
    mosquitto_subscribe(mosq,NULL,"DETI/Authenticate/Enter",2);
    mosquitto_subscribe(mosq,NULL,"DETI/Authenticate/Recognition",2);
    mosquitto_loop_start(mosq);


    printf("MQTT thread created successfully\n\n");
}

// Function to create UDP server socket
void Create_Socket() {
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
    printf("Socket creation complete\n");
}

// Function to read and store encryption keys from txt file
int Read_Keys() {
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
}

// Function which calls all initializations need when running the code
int Initialize(){

    if(Create_DataBase_IP()==0){return 0;}
    if(Create_DataBase_ID()==0){return 0;}
    if(Create_DataBase_Data()==0){return 0;}
    if(Create_DataBase_Log()==0){return 0;}
    if(Read_Keys()==0){return 0;}
    Create_Socket();
    if(Create_Thread_MQTT()==0){return 0;}

    return 1;
}

// Function to treat and store the data received correspondent to a room
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
                    Update_Ocupation_in_DataBase_IP(occupation,room);
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

int main() {
    int find, room, key_counter, permission_UID, nmec, len;  
    bool restart;
    char serial_num[6], room_[4];  

    if(Initialize()==0){return 1;}    

    while(1){
        memset((char*) message_cipher, 0, sizeof(message_cipher));
        memset(buffer, 0, sizeof(buffer));
        memset(serial_num,0,sizeof(serial_num));

        // Waits to receive a UDP message
        recvfrom(sockfd, buffer, keyLen, MSG_WAITALL, ( struct sockaddr *) &cliaddr, &len);
        
        //! 0 is new connection
        //! 1 is UID checkup permission
        //! 2 is room data
        //! 9 is emergency
        
        // Checks which is the first digit of the message to redirect to the correct action
        switch (buffer[0]){
            // Happens when a client connects for the first time. Checks if the message is equal to the message_init. 
            // If it is saves the data and sends the message_init back, else just sends denial message
            case '0':
                strncpy(serial_num,buffer+1,6);
                strncpy(room_,buffer+7,3);
                
                room = atoi(room_);
                find = Find_room_with_serial_in_DataBase_IP(serial_num);
                
                for(int i=0;i<keyLen-10;i++) {if(buffer[i+10]!=message_init[i]) {goto NOT_RESET;}}

                if(strcmp(inet_ntoa(cliaddr.sin_addr),"0.0.0.0")){
                    sendto(sockfd, (char*)message_init, keyLen, MSG_CONFIRM, (const struct sockaddr *) &cliaddr, len);
                    if (find==0){
                        if(Add_to_DataBase_IP(serial_num,room,inet_ntoa(cliaddr.sin_addr),htons(cliaddr.sin_port))==0){return 1;};
                        printf("Client (nº%d) %s:%d -> Has entered\n", room, inet_ntoa(cliaddr.sin_addr), htons(cliaddr.sin_port));
                    }
                    else {
                        if(Update_Info_in_DataBase_IP(serial_num,room,inet_ntoa(cliaddr.sin_addr),htons(cliaddr.sin_port))==0) {return 1;}    
                        printf("Client (nº%d) %s:%d -> Has entered\n", room, inet_ntoa(cliaddr.sin_addr), htons(cliaddr.sin_port));
                    }
                }
                                
                break;
                NOT_RESET:
                sendto(sockfd, denial, strlen(denial), MSG_CONFIRM, (const struct sockaddr *) &cliaddr, len);
                break;

            // Happens when a client has a card trying to enter. It decrypts the message, checks if the card has authorization and sends 
            // confirmation/denial message encrypted
            case '1':
                room = Find_room_with_IP_in_DataBase_IP(inet_ntoa(cliaddr.sin_addr));
                if (room>0) {
                    key_counter = XORCipher(buffer,false,room,'1');
                    if(Update_Counter_in_DataBase_IP(key_counter,room)==0) {return 1;};
                    message_cipher[8]='\0';
                    permission_UID = Check_UID_in_DataBase_ID(message_cipher,room);
                    nmec = Read_NMEC_in_DataBase_ID(message_cipher);
                    if(permission_UID==0) {
                        return 1;
                    }
                    else if(permission_UID==1){
                        printf("Client (nº%d) %s:%d -> %s can enter\n", room, inet_ntoa(cliaddr.sin_addr), htons(cliaddr.sin_port), message_cipher);
                        key_counter = XORCipher(confirmation,true,room,'1');
                        if(Add_to_DataBase_Log(room,nmec)==0){return 1;}
                        if(Update_Counter_in_DataBase_IP(key_counter,room)==0) {return 1;};
                    }
                    else{
                        printf("Client (nº%d) %s:%d -> %s can't enter\n", room, inet_ntoa(cliaddr.sin_addr), htons(cliaddr.sin_port), message_cipher);
                        key_counter = XORCipher(denial,true,room,'1'); 
                        if(Update_Counter_in_DataBase_IP(key_counter,room)==0) {return 1;};
                    }

                    sendto(sockfd, (char*)message_cipher, keyLen, MSG_CONFIRM, (const struct sockaddr *) &cliaddr, len);
                }
                break;

            // Happens when a client sends envrironmental data from a room. It just receives the data and stores it
            case '2':
                room = Find_room_with_IP_in_DataBase_IP(inet_ntoa(cliaddr.sin_addr));
                if (room>0) {
                    printf("Data in room of client (nº%d) %s:%d is ", room, inet_ntoa(cliaddr.sin_addr), htons(cliaddr.sin_port));
                    if(Receive_Data(buffer,room)==0) {return 1;}
                }
                break;

            // Happens when an emergency is sent by a room. It sends the emergency to every room
            case '9':
                Send_Emergency();
                sleep(1);
                Send_Emergency();
                break;
        }
    }
    return 0;
}