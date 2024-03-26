cd Current_Version
gcc mqtt_publisher.c -o mqtt -lmosquitto -lsqlite3 && ./mqtt