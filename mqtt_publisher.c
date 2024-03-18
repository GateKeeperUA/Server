#include <stdio.h>
#include <stdlib.h>
#include <mosquitto.h>
#include <unistd.h>
#include <string.h>
#include <sqlite3.h>

#define maxclients 200
int last_check_ID=0, last_check_data=0;

struct Data {
    int last_temperature;
    int last_humidity;
    int last_gas;
    int last_ID;
};

struct Data data[maxclients]={0};

struct mosquitto * mosq;

int Find_in_DataBase(int client) {
    int temperature=0, humidity=0, gas=0, check = 0;
    char* err;
    sqlite3* db;
    sqlite3_stmt* stmt;
    if(sqlite3_open("SQLite/Server.db", &db)!=0) {
        return 1;
    }
    sqlite3_prepare_v2(db,"select Temperature,Pressure,Humidity,Gas,ID from Data",-1,&stmt,NULL);
    while(sqlite3_step(stmt) != SQLITE_DONE){ 
        check++;
        if(last_check_data+10<check){
            printf("Error reading\n");
            goto ERROR;
        }
        if(sqlite3_column_int(stmt,4)==data[client].last_ID){
            temperature = sqlite3_column_int(stmt,0);
            humidity = sqlite3_column_int(stmt,2);
            gas = sqlite3_column_int(stmt,3);
        }
    }
    last_check_data = check;
    data[client].last_temperature = temperature;
    data[client].last_humidity = humidity;
    data[client].last_gas = gas;
    ERROR:
    sqlite3_reset(stmt);
    sqlite3_close(db);
    
    return 0;
}

int Find_IP_in_Database() {
    int IDs[maxclients]={0};
    int check = 0;
    char* err;
    sqlite3* db;
    sqlite3_stmt* stmt;
    if(sqlite3_open("SQLite/Server.db", &db)!=0) {
        return 1;
    }

    sqlite3_prepare_v2(db,"select ID,IP1,IP2,IP3,IP4,Counter,Permission from IP",-1,&stmt,NULL);
    for(int i=0;sqlite3_step(stmt) != SQLITE_DONE;i++) {
        check++;
        if(last_check_ID+10<check){
            printf("Error reading\n");
            goto ERROR;
        }
        
        IDs[i]=sqlite3_column_int(stmt,0);
    }

    last_check_ID = check;
    for(int i=0;i<check;i++) {
        data[i].last_ID=IDs[i];
    }
    ERROR:

    sqlite3_reset(stmt);
    sqlite3_close(db);
    return 0;
}

int Check_DataBase() {
    char* err;
    sqlite3* db;
    sqlite3_stmt* stmt;
    if(sqlite3_open("SQLite/Server.db", &db)!=0) {
        return 1;
    }
    sqlite3_prepare_v2(db,"select Temperature,Pressure,Humidity,Gas,ID from Data",-1,&stmt,NULL);
    while(sqlite3_step(stmt) != SQLITE_DONE){ 
        last_check_data++;
    }
    sqlite3_reset(stmt);
    sqlite3_prepare_v2(db,"select ID,IP1,IP2,IP3,IP4,Counter,Permission from IP",-1,&stmt,NULL);
    while(sqlite3_step(stmt) != SQLITE_DONE){ 
        last_check_ID++;
    }
    sqlite3_reset(stmt);
    sqlite3_close(db);
    
    return 0;
}

void Send_Data() {
    char publish[128];
    char topic[20];
    for(int i=0;i<last_check_ID;i++) {
        memset(topic,0,sizeof(topic));
        memset(publish,0,sizeof(publish));
        while(Find_in_DataBase(i)!=0);

        printf("Data sent temp for topic %d:%d hum:%d gas:%d\n",data[i].last_ID,data[i].last_temperature,data[i].last_humidity,data[i].last_gas);
        sprintf(publish,"%d %d %d",data[i].last_temperature,data[i].last_humidity,data[i].last_gas);
        sprintf(topic,"DETI/%d",data[i].last_ID);
        mosquitto_publish(mosq,NULL,topic,strlen(publish),publish,0,false);
    }
    printf("\n");
}

int main() {
    int rc;

    Check_DataBase();
    mosquitto_lib_init();

    mosq = mosquitto_new("Server",true,NULL);

    rc = mosquitto_connect(mosq,"192.168.1.86",1883,60);
    if(rc!=0) {
        printf("Error\n");
        mosquitto_destroy(mosq);
        return -1;
    }
    printf("Success\n");

    while(true) {
        while(Find_IP_in_Database()!=0);
        Send_Data();
        sleep(1);
    }
    

    mosquitto_disconnect(mosq);
    mosquitto_destroy(mosq);

    mosquitto_lib_cleanup();

    return 0;
}