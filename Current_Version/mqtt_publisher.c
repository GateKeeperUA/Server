#include <stdio.h>
#include <stdlib.h>
#include <mosquitto.h>
#include <unistd.h>
#include <string.h>
#include <sqlite3.h>
#include <time.h>
#include "IP_conf.h"

#define maxclients 200
int last_check_room=0, check=0;

struct Data {
    int last_temperature;
    int last_humidity;
    int last_gas;
    int last_occupation;
    int last_room;
};

struct Data data[maxclients]={0};

struct mosquitto * mosq;

void Send_Data() {
    char publish[128];
    char topic[20];
    int temperature=0, humidity=0, gas=0, occupation=0, room=0;
    sqlite3* db;
    sqlite3_stmt* stmt;

    if (check==0) { 
            last_check_room = 0;
        if(sqlite3_open("SQLite/Data.db", &db)!=0) {
            printf("Error\n");
        }
        sqlite3_prepare_v2(db,"select Temperature,Pressure,Humidity,Gas,Occupation,Room from Data",-1,&stmt,NULL);
        
        while(sqlite3_step(stmt) != SQLITE_DONE){ 
            temperature = sqlite3_column_int(stmt,0);
            humidity = sqlite3_column_int(stmt,2);
            gas = sqlite3_column_int(stmt,3);
            occupation = sqlite3_column_int(stmt,4);
            room = sqlite3_column_int(stmt,5);

            data[last_check_room].last_temperature = temperature;
            data[last_check_room].last_humidity = humidity;
            data[last_check_room].last_gas = gas;
            data[last_check_room].last_occupation = occupation;
            data[last_check_room].last_room = room;

            printf("Data sent temp for topic %d->temp:%d hum:%d gas:%d occupation:%d\n",data[last_check_room].last_room,data[last_check_room].last_temperature,data[last_check_room].last_humidity,data[last_check_room].last_gas,data[last_check_room].last_occupation);
            sprintf(publish,"%d %d %d %d",data[last_check_room].last_temperature,data[last_check_room].last_humidity,data[last_check_room].last_gas,data[last_check_room].last_occupation);
            sprintf(topic,"DETI/room%d",data[last_check_room].last_room);
            mosquitto_publish(mosq,NULL,topic,strlen(publish),publish,0,false);

            last_check_room++;
        }
        ERROR:

        sqlite3_reset(stmt);
        sqlite3_close(db);
        printf("\n");
        check++;
    }

    else {
        for(int i = 0;i<last_check_room;i++){ 
            printf("Data sent temp for topic %d->temp:%d hum:%d gas:%d occupation:%d\n",data[i].last_room,data[i].last_temperature,data[i].last_humidity,data[i].last_gas,data[i].last_occupation);
            sprintf(publish,"%d %d %d %d",data[i].last_temperature,data[i].last_humidity,data[i].last_gas,data[i].last_occupation);
            sprintf(topic,"DETI/room%d",data[i].last_room);
            mosquitto_publish(mosq,NULL,topic,strlen(publish),publish,0,false);
        }
        printf("\n");

        if(check == 4) { 
            check = 0;
        }
        else {
            check++;
        }
    }
}

int main() {
    int rc,sleep_=1;

    mosquitto_lib_init();

    mosq = mosquitto_new("Server_publish_data",true,NULL);

    rc = mosquitto_connect(mosq,IP_server,1883,60);
    if(rc!=0) {
        printf("Error\n");
        mosquitto_destroy(mosq);
        return -1;
    }
    printf("Success\n");

    while(true) {
        Send_Data();
        sleep(sleep_);
    }
    

    mosquitto_disconnect(mosq);
    mosquitto_destroy(mosq);

    mosquitto_lib_cleanup();

    return 0;
}