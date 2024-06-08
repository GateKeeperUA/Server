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
#include "IP_conf.h"

//Used to define UPD port
#define PORT    5005
//Used to define encryption key length
#define keyLen  128
//Used to define number of encryption keys
#define numkeys 300
