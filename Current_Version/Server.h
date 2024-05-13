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
#include <math.h>
#include <time.h>
#include <ctype.h>
#include <mosquitto.h>
   
#define PORT    5005
#define keyLen  128
#define numkeys 300
#define IP_server "192.168.1.100"