#include <stdio.h>
#include <stdlib.h>
#include <mosquitto.h>
#include <unistd.h>
#include <string.h>

int main() {
    int rc, counter=0;
    char *counter_;
    struct mosquitto * mosq;

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
        sprintf(counter_,"%d",counter);
        mosquitto_publish(mosq,NULL,"DETI/Rooms/1",strlen(counter_),counter_,0,false);
        sleep(1);
        counter++;
    }
    

    mosquitto_disconnect(mosq);
    mosquitto_destroy(mosq);

    mosquitto_lib_cleanup();

    return 0;
}