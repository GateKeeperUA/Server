# Server
Raspberry server code repository

To run it just call server.sh and mqtt.sh in parallel

The client.sh is to simulate a door and the key_generator.sh is to generate news keys and store in the file Encryption_keys

# Dependencies
sudo apt install -libsqlite3 -libmosquitto mosquitto mosquitto-clients

Configure /etc/mosquitto/mosquitto.conf file

sudo systemctl start mosquitto
sudo systemctl enable mosquitto
