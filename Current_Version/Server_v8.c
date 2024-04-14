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
    if(sqlite3_exec(db,"CREATE TABLE IF NOT EXISTS IP(Room INT,IP1 varchar(3),IP2 varchar(3),IP3 varchar(3),IP4 varchar(3),Counter INT,Permission INT);",NULL,NULL,&err) != SQLITE_OK) {
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
    if(sqlite3_exec(db,"CREATE TABLE IF NOT EXISTS Data(Temperature INT,Pressure INT,Humidity INT,Gas INT,Room INT);",NULL,NULL,&err) != SQLITE_OK) {
        printf("\nError %s\n",err);
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
    if(sqlite3_exec(db,"CREATE TABLE IF NOT EXISTS ID(UID varchar(23),NMEC INT,Permission INT);",NULL,NULL,&err) != SQLITE_OK) {
        printf("\nError %s\n",err);
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
    if(sqlite3_exec(db,"CREATE TABLE IF NOT EXISTS LOG(Room INT,NMEC INT,Hour INT,Minute INT,Second INT,Day INT,Month INT,Year INT);",NULL,NULL,&err) != SQLITE_OK) {
        printf("\nError %s\n",err);
        return 0;
    }
    sqlite3_close(db);
    printf("Log DataBase creation complete\n");
    return 1;
}

int Find_IP_in_DataBase(char *IP) {
    char* err;
    sqlite3* db;
    sqlite3_stmt* stmt;
    if(sqlite3_open("SQLite/IP.db", &db)!=0) {
        return 0;
    }

    sqlite3_prepare_v2(db,"select Room,IP1,IP2,IP3,IP4,Counter,Permission from IP",-1,&stmt,NULL);
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

int Find_ID_in_DataBase(int ID) {
    char* err;
    sqlite3* db;
    sqlite3_stmt* stmt;
    if(sqlite3_open("SQLite/IP.db", &db)!=0) {
        return 0;
    }

    sqlite3_prepare_v2(db,"select Room,IP1,IP2,IP3,IP4,Counter,Permission from IP",-1,&stmt,NULL);
    int find=0;
    while(sqlite3_step(stmt) != SQLITE_DONE) {
        find = sqlite3_column_int(stmt,0);
        if(find==ID){
            break;
        }
    }
    sqlite3_reset(stmt);
    sqlite3_close(db);
    return find;
}

int Add_to_DataBase_IP(char* IP, int room, int permission) {
    char* err;
    sqlite3* db;
    if(sqlite3_open("SQLite/IP.db", &db)!=0) {
        return 0;
    }

    for(int i=0;i<strlen(IP);i++){if(IP[i]=='.')IP[i]=',';}
    char query[100];
    sprintf(query,"insert into IP VALUES(%d,%s,%d,%d);",room,IP,0,permission);
    //! 0 - not a DETI student
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

int Add_to_DataBase_Data(int temperature, int pressure, int humidity, int gas, int room) {
    int find=0;
    char query[100];
    char* err;
    sqlite3* db;
    sqlite3_stmt* stmt;
    if(sqlite3_open("SQLite/Data.db", &db)!=0) {
        return 0;
    }
    sqlite3_prepare_v2(db,"select Temperature,Pressure,Humidity,Gas,Room from Data",-1,&stmt,NULL);
    while(sqlite3_step(stmt) != SQLITE_DONE) {
        find = sqlite3_column_int(stmt,4);
        if(find==room){
            sprintf(query,"UPDATE DATA SET Temperature = %d, Pressure = %d, Humidity = %d, Gas = %d Where ID = %d",temperature,pressure,humidity,gas,room);
            goto DONE;
        }
        printf("a\n");
    }
    sqlite3_reset(stmt);

    sprintf(query,"insert into Data VALUES(%d,%d,%d,%d,%d);",temperature,pressure,humidity,gas,room);
    if(sqlite3_exec(db,query,NULL,NULL,&err) != SQLITE_OK) {
        printf("\nError %s\n",err);
        return 0;
    }
    
    DONE:
    sqlite3_close(db);
    return 1;
}

int Match_Month(char* month) {
    if (strcmp(month,"Jan")==0)
        return 1;
    else if (strcmp(month,"Feb")==0)
        return 2;
    else if (strcmp(month,"Mar")==0)
        return 3;
    else if (strcmp(month,"Apr")==0)
        return 4;
    else if (strcmp(month,"May")==0)
        return 5;
    else if (strcmp(month,"Jun")==0)
        return 6;
    else if (strcmp(month,"Jul")==0)
        return 7;
    else if (strcmp(month,"Ago")==0)
        return 8;
    else if (strcmp(month,"Sep")==0)
        return 9;
    else if (strcmp(month,"Oct")==0)
        return 10;
    else if (strcmp(month,"Nov")==0)
        return 11;
    else if (strcmp(month,"Dec")==0)
        return 12;    
}

int Add_to_DataBase_Log(int Room, int NMEC) {
    int month_,space_counter = 0;
    char hour[2], minute[2], second[2], day[2], month[3], year[4];
    char* err;
    sqlite3* db;
    time_t t;
    if(sqlite3_open("SQLite/Log.db", &db)!=0) {
        return 0;
    }
    time(&t);
    char* t_string = ctime(&t);
    for(int i=0;i<strlen(t_string);i++) {
        switch(space_counter) {
            case 1:
                for(int j=0;t_string[i]!=' ';j++,i++) {
                    month[j] = t_string[i];
                }
                break;
            case 2:
                for(int j=0;t_string[i]!=' ';j++,i++) {
                    day[j] = t_string[i];
                }
                break;

            case 3:
                for(int j=0;t_string[i]!=':';j++,i++) {
                    hour[j] = t_string[i];
                }
                break;

            case 4:
                for(int j=0;t_string[i]!=':';j++,i++) {
                    minute[j] = t_string[i];
                }
                break;
            case 5:
                for(int j=0;t_string[i]!=' ';j++,i++) {
                    second[j] = t_string[i];
                }
                break;
            case 6:
                for(int j=0;t_string[i]!='\0';j++,i++) {
                    year[j] = t_string[i];
                }
                break;
        }
        if(t_string[i]==' ' || t_string[i]==':'){
            space_counter++;
        }
    }
    sprintf(month,"%c%c%c",month[0],month[1],month[2]);
    month_ = Match_Month(month);
    char query[100];
    sprintf(query,"insert into Log VALUES(%d,%d,%d,%d,%d,%d,%d,%d);",Room,NMEC,atoi(hour),atoi(minute),atoi(second),atoi(day),month_,atoi(year));

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
        printf("\nError %s\n",err);
        return 0;
    }
    sqlite3_close(db);
    return 1;
}

int Update_Permission_in_DataBase_IP(int permission, int room, char IP[20]) {
    char IP_[4];
    int IP1=0,IP2=0,IP3=0;
    char* err;
    char query[100];
    sqlite3* db;
    sqlite3_stmt* stmt;
    if(sqlite3_open("SQLite/IP.db", &db)!=0) {
        return 0;
    }
    sprintf(query,"UPDATE IP SET Permission = %d Where Room = %d",permission,room);
    if(sqlite3_exec(db,query,NULL,NULL,&err) != SQLITE_OK) {
        printf("\nError %s\n",err);
        return 0;
    }
    memset(IP_,0,strlen(IP_));
    for(int i=0;IP[i]!='.';i++){
        IP_[i]=IP[i];
        IP1++;
    }
    sprintf(query,"UPDATE IP SET IP1 = %s Where Room = %d",IP_,room);
    if(sqlite3_exec(db,query,NULL,NULL,&err) != SQLITE_OK) {
        printf("\nError %s\n",err);
        return 0;
    }
    memset(IP_,0,strlen(IP_));
    for(int i=IP1+1;IP[i]!='.';i++){
        IP_[i-IP1-1]=IP[i];
        IP2++;
    }
    sprintf(query,"UPDATE IP SET IP2 = %s Where Room = %d",IP_,room);
    if(sqlite3_exec(db,query,NULL,NULL,&err) != SQLITE_OK) {
        printf("\nError %s\n",err);
        return 0;
    }
    memset(IP_,0,strlen(IP_));
    for(int i=IP1+IP2+2;IP[i]!='.';i++){
        IP_[i-IP1-IP2-2]=IP[i];
        IP3++;
    }
    sprintf(query,"UPDATE IP SET IP3 = %s Where Room = %d",IP_,room);
    if(sqlite3_exec(db,query,NULL,NULL,&err) != SQLITE_OK) {
        printf("\nError %s\n",err);
        return 0;
    }
    memset(IP_,0,strlen(IP_));
    for(int i=IP1+IP2+IP3+3;IP[i]!='.';i++){
        IP_[i-IP1-IP2-IP3-3]=IP[i];
    }
    sprintf(query,"UPDATE IP SET IP4 = %s Where Room = %d",IP_,room);
    if(sqlite3_exec(db,query,NULL,NULL,&err) != SQLITE_OK) {
        printf("\nError %s\n",err);
        return 0;
    }
    sprintf(query,"UPDATE IP SET IP4 = %s Where Room = %d",IP_,room);
    if(sqlite3_exec(db,query,NULL,NULL,&err) != SQLITE_OK) {
        printf("\nError %s\n",err);
        return 0;
    }
    sqlite3_close(db);

    return 1;
}

int Read_Counter_in_DataBase_IP(int room) {
    char* err;
    sqlite3* db;
    sqlite3_stmt* stmt;
    if(sqlite3_open("SQLite/IP.db", &db)!=0) {
        return 0;
    }

    sqlite3_prepare_v2(db,"select Room,IP1,IP2,IP3,IP4,Counter,Permission from IP",-1,&stmt,NULL);

    int find;
    do {
        sqlite3_step(stmt);
    }while(sqlite3_column_int(stmt,0)!=room);
    find = sqlite3_column_int(stmt,5);
    sqlite3_reset(stmt);
    sqlite3_close(db);
    return find;    
}

int Check_UID_in_DataBase_ID(char* UID, int room) {
    char* err;
    sqlite3* db;
    sqlite3_stmt* stmt;

    if(sqlite3_open("SQLite/IP.db", &db)!=0) {
        return 0;
    }

    sqlite3_prepare_v2(db,"select Room,IP1,IP2,IP3,IP4,Counter,Permission from IP",-1,&stmt,NULL);

    int find_room, permission;
    while(sqlite3_step(stmt) != SQLITE_DONE) {
        find_room=sqlite3_column_int(stmt,0);
        if(find_room==room){
            permission=sqlite3_column_int(stmt,6);
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
   
int XORCipher(char* data, bool send, int ID, char type) {
    int counter = Read_Counter_in_DataBase_IP(ID);
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

int Receive_Data(char* data, int ID) {
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
                    printf("%dIAQ\n",gas);
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
    int sockfd, ID, key_counter, permission_UID;
    char buffer[keyLen];
    struct sockaddr_in servaddr, cliaddr;
    int len;    
    bool restart;
    char ID_[3];
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
        recvfrom(sockfd, buffer, keyLen, MSG_WAITALL, ( struct sockaddr *) &cliaddr, &len); 
        
        //! 0 is new connection
        //! 1 is ID checkup on Database
        //! 2 is temperature data
        //! 9 is emergency

        
        switch (buffer[0]){
            case '0':
                sprintf(ID_,"%c%c%c",buffer[1],buffer[2],buffer[3]);
                ID = Find_ID_in_DataBase(atoi(ID_));
                for(int i=0;i<keyLen-5;i++) {if(buffer[i+5]!=message_init[i]) {goto NOT_RESET;}}
                    sendto(sockfd, (char*)message_init, keyLen, MSG_CONFIRM, (const struct sockaddr *) &cliaddr, len);
                    if (ID==0){
                        if(Add_to_DataBase_IP(inet_ntoa(cliaddr.sin_addr),atoi(ID_),buffer[4]-'0')==0){return 1;};
                        printf("Client (nº%d) %s:%d -> Has entered\n", atoi(ID_), inet_ntoa(cliaddr.sin_addr), htons(cliaddr.sin_port));
                    }
                    else {
                        if(Update_Counter_in_DataBase_IP(0,ID)==0) {return 1;}
                        if(Update_Permission_in_DataBase_IP(buffer[4]-'0',ID,inet_ntoa(cliaddr.sin_addr))==0) {return 1;}    
                        printf("Client (nº%d) %s:%d -> Has entered\n", atoi(ID_), inet_ntoa(cliaddr.sin_addr), htons(cliaddr.sin_port));
                    }
                                
                break;
                NOT_RESET:
                sendto(sockfd, "Not initialized", 15, MSG_CONFIRM, (const struct sockaddr *) &cliaddr, len);
                break;

            case '1':
                ID = Find_IP_in_DataBase(inet_ntoa(cliaddr.sin_addr));
                if (ID>0) {
                    XORCipher(buffer,false,ID,'1');
                    permission_UID = Check_UID_in_DataBase_ID(message_cipher,ID);
                    if(permission_UID==0) {
                        return 1;
                    }
                    else if(permission_UID==1){
                        printf("Client (nº%d) %s:%d -> %s can enter\n", ID, inet_ntoa(cliaddr.sin_addr), htons(cliaddr.sin_port), message_cipher);
                        key_counter = XORCipher(confirmation,true,ID,'1');

                        if(Add_to_DataBase_Log(ID,107465)==0){return 1;}
                    }
                    else{
                        printf("Client (nº%d) %s:%d -> %s can't enter\n", ID, inet_ntoa(cliaddr.sin_addr), htons(cliaddr.sin_port), message_cipher);
                        key_counter = XORCipher(denial,true,ID,'1'); 
                    }

                    sendto(sockfd, (char*)message_cipher, keyLen, MSG_CONFIRM, (const struct sockaddr *) &cliaddr, len);
                    if(Update_Counter_in_DataBase_IP(key_counter,ID)==0) {return 1;};
                }
                break;

            case '2':
                ID = Find_IP_in_DataBase(inet_ntoa(cliaddr.sin_addr));
                if (ID>0) {
                    printf("Data in room of client (nº%d) %s:%d is ", ID, inet_ntoa(cliaddr.sin_addr), htons(cliaddr.sin_port));
                    if(Receive_Data(buffer,ID)==0) {return 1;}
                }
        }
    }
    return 0;
}