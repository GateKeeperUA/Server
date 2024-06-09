# Server
Raspberry server code repository

To run it just call server.sh first and then mqtt.sh in parallel

The client.sh is to simulate a door and the key_generator.sh is to generate news keys and store in the file Encryption_keys

# Dependencies
sudo apt install libsqlite3-dev libmosquitto-dev mosquitto mosquitto-clients

sudo nano /etc/mosquitto/mosquitto.conf file and add the following to the file
listener 1883
allow_anonymous true

sudo systemctl start mosquitto
sudo systemctl enable mosquitto
