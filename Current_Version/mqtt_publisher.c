#include <stdio.h>
#include <stdlib.h>
#include <mosquitto.h>
#include <unistd.h>
#include <string.h>
#include <sqlite3.h>

#define maxclients 200
int last_check_ID=0, check=0;

struct Data {
    int last_temperature;
    int last_humidity;
    int last_gas;
    int last_ID;
};

struct Data data[maxclients]={0};

struct mosquitto * mosq;

void Send_Data() {
    char publish[128];
    char topic[20];
    int temperature=0, humidity=0, gas=0, ID=0;
    sqlite3* db;
    sqlite3_stmt* stmt;

    if (check==0) { 
            last_check_ID = 0;
        if(sqlite3_open("SQLite/Data.db", &db)!=0) {
            printf("Error\n");
        }
        sqlite3_prepare_v2(db,"select Temperature,Pressure,Humidity,Gas,ID from Data",-1,&stmt,NULL);
        
        while(sqlite3_step(stmt) != SQLITE_DONE){ 
            temperature = sqlite3_column_int(stmt,0);
            humidity = sqlite3_column_int(stmt,2);

            gas = sqlite3_column_int(stmt,3);
            ID = sqlite3_column_int(stmt,4);

            data[last_check_ID].last_temperature = temperature;
            data[last_check_ID].last_humidity = humidity;
            data[last_check_ID].last_gas = gas;
            data[last_check_ID].last_ID = ID;

            printf("Data sent temp for topic %d->temp:%d hum:%d gas:%d\n",data[last_check_ID].last_ID,data[last_check_ID].last_temperature,data[last_check_ID].last_humidity,data[last_check_ID].last_gas);
            sprintf(publish,"%d %d %d",data[last_check_ID].last_temperature,data[last_check_ID].last_humidity,data[last_check_ID].last_gas);
            sprintf(topic,"DETI/room%d",data[last_check_ID].last_ID);
            mosquitto_publish(mosq,NULL,topic,strlen(publish),publish,0,false);

            last_check_ID++;
        }
        ERROR:

        sqlite3_reset(stmt);
        sqlite3_close(db);
        printf("\n");
        check++;
    }

    else {
        for(int i = 0;i<last_check_ID;i++){ 
            printf("Data sent temp for topic %d->temp:%d hum:%d gas:%d\n",data[i].last_ID,data[i].last_temperature,data[i].last_humidity,data[i].last_gas);
            sprintf(publish,"%d %d %d",data[i].last_temperature,data[i].last_humidity,data[i].last_gas);
            sprintf(topic,"DETI/room%d",data[i].last_ID);
            mosquitto_publish(mosq,NULL,topic,strlen(publish),publish,0,false);
        }
        printf("\n");

        if(check == 9) { 
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

    mosq = mosquitto_new("Server",true,NULL);

    rc = mosquitto_connect(mosq,"192.168.1.86",1883,60);
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