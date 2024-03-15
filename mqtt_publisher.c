#include <stdio.h>
#include <stdlib.h>
#include <mosquitto.h>
#include <unistd.h>
#include <string.h>
#include <sqlite3.h>

int last_check=0, last_temperature, last_humidity, last_gas;

int Find_in_DataBase() {
    int temperature, humidity, gas, check = 0;
    char* err;
    sqlite3* db;
    sqlite3_stmt* stmt;
    if(sqlite3_open("SQLite/Server.db", &db)!=0) {
        return 1;
    }
    sqlite3_prepare_v2(db,"select Temperature,Pressure,Humidity,Gas from Data",-1,&stmt,NULL);
    while(sqlite3_step(stmt) != SQLITE_DONE){ 
        check++;
        if(last_check+10<check){
            goto ERROR;
            printf("Error reading\n");
        }
        temperature = sqlite3_column_int(stmt,0);
        humidity = sqlite3_column_int(stmt,2);
        gas = sqlite3_column_int(stmt,3);
    }
    last_check = check;
    last_temperature = temperature;
    last_humidity = humidity;
    last_gas = gas;
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
    sqlite3_prepare_v2(db,"select Temperature,Pressure,Humidity,Gas from Data",-1,&stmt,NULL);
    while(sqlite3_step(stmt) != SQLITE_DONE){ 
        last_check++;
    }
    sqlite3_reset(stmt);
    sqlite3_close(db);
    
    return 0;
}

int main() {
    int rc;
    char publish[128];
    struct mosquitto * mosq;
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
        while(Find_in_DataBase()!=0);

        printf("Data sent temp:%d hum:%d gas:%d\n",last_temperature,last_humidity,last_gas);
        sprintf(publish,"%d %d %d",last_temperature,last_humidity,last_gas);
        mosquitto_publish(mosq,NULL,"DETI/1",strlen(publish),publish,0,false);
        sleep(1);
    }
    

    mosquitto_disconnect(mosq);
    mosquitto_destroy(mosq);

    mosquitto_lib_cleanup();

    return 0;
}