cd Current_Version
gcc key_generator.c -o key_generator -lmosquitto -lsqlite3 && ./key_generator