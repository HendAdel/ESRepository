Read me
Project Name
This project uses ESP8266 module and DHT11 temperature and humidity sensor with Arduino IDE to send temperature and humidity readings to the ThingSpeak website.
External Libraries
Wifi Manager
We use wifi manager library by tzapu to manage the changes in the WiFi settings (SSID and Password). So thanks to tzapu.
To install wifi manager library https://github.com/tzapu/WiFiManager
DHTesp
We use DHTesp library file by “beegee Tokyo” to read the temperature and humidity easily from DHT11 sensor, thanks to beegee Tokyo.
To install DHTesp library https://github.com/beegee-tokyo/DHTesp
How It Works
The DHT11 sensor connected to the ESP8266 reads the temperature and the humidity. The esp8266 connects to the ThingSpeak website through WiFi and sends the readings to the fields channel. We setup a Webserver on the ESP8266 to configure the ThingSpeak logging parameters:
Start/Stop sending the data.
Configure the frequency of sending.
Configure ThingSpeak API key.
All this can be done through the “control settings” HTML page that’s hosted in ESP8266 web server.

Quick Start
To run the code you need to:
1-   Install the wifi manager library from https://github.com/tzapu/WiFiManager  or from the tools menu in the Arduino IDE. choose "Manage libraries" and type wifimanager in the search bar and install it.
2-   Install the DHTesp library from https://github.com/beegee-tokyo/DHTesp or in Manage libraries type DHTesp in the search bar and install it.
3- Upload the html files in the data folder to the SPIFFS by clicking “ESP8266 sketch data upload” from the tools menu – be sure that the serial window is closed- this will add the control settings page to the ESP8266 web server.
Sponsored by
https://www.the-diy-life.co/
