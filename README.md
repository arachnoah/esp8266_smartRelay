esp8266_smartRelay
==================

Relay-switching over WiFi with ESP8266 / ESP-01

Can be uploaded to microcontroller with Arduino IDE


Features:
--------

GPIO2 is always an output, GPIO0 can be input_pullup or output

### Access Point (AP) mode:
- creates AP whenever connection to WiFi is lost or can't be established
- retries connecting to WiFi every 30000 milliseconds
- close AP when connected to WiFi
- SSID: esp8266#*&lt;Chip-ID&gt;*
- PSK: add file `AP_PSK.h` with `#define AP_PSK "super-secred-password"`
- IP-address: `192.168.4.1`

### controlling the GPIO pins:

- **query** state:
  *&lt;IP-address&gt;*/*&lt;pin&gt;*
  (HTTP-GET)
- **switch** state:
  *&lt;IP-address&gt;*/*&lt;pin&gt;*
  (HTTP-POST)
- **set** state:
  *&lt;IP-address&gt;*/*&lt;pin&gt;*/*&lt;state&gt;*
  (HTTP-POST)

where:

- *&lt;pin&gt;* : **GPIO0** `pin`/`pin0`, **GPIO2** `pin2`
- *&lt;state&gt;* : **ON** `1`, **OFF** `0`


### webinterface for configuration:

saved on EEPROM

reachable under: *&lt;IP-address&gt;*/settings in AP and Station Mode

  ![image](https://user-images.githubusercontent.com/50140225/129475386-949c7320-ffaa-4e75-bbfe-646223c4905f.png)

