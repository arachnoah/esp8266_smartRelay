#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <LittleFS.h>

#include "settings.html.h"
#include "settings.js.h"

// create config with default values
//#define CREATE_DEFAULT_CONFIG

// SSID: max 31 char
// PSK:  max 63 char / min 8 char
#define AP_SSID "esp8266"
// include AP PSK
#include "AP_PSK.h"
#define HOSTNAME "esp8266"

#define PIN_2 2
#define PIN_0 0


const char json_template[] PROGMEM = R"({"value":{"sta_ssid":"%s","ip0":%hhu,"ip1":%hhu,"ip2":%hhu,"ip3":%hhu,"gw0":%hhu,"gw1":%hhu,"gw2":%hhu,"gw3":%hhu,"sm0":%hhu,"sm1":%hhu,"sm2":%hhu,"sm3":%hhu,"pd0":%hhu,"pd1":%hhu,"pd2":%hhu,"pd3":%hhu,"sd0":%hhu,"sd1":%hhu,"sd2":%hhu,"sd3":%hhu,"interruptOn":"%c","impDur":"%u","impDur_alt":"%u"},"checked":{"sta_dhcp":%s,"flipHighLow":%s,"pin2asIn":%s,"sendImp":%s}}
)";


// declare config variables
char ap_ssid[32];
char ap_psk[64];

char sta_ssid[32];
char sta_psk[64];
bool sta_dhcp;
IPAddress sta_ip = 0u;
IPAddress sta_gateway = 0u;
IPAddress sta_subnet = 0u;
IPAddress sta_priDNS = 0u;
IPAddress sta_secDNS = 0u;

char hostname[32];

bool flipHighLow;
bool pin2asIn;
byte interruptOn;
bool sendImp;
ulong impDur;

ulong nextTryAfter;
ulong retryAfter = 30000;


ESP8266WebServer server(80);

WiFiEventHandler wifiConnectedHandler;
WiFiEventHandler wifiGotIPHandler;
WiFiEventHandler wifiDisconnectedHandler;


void setup() {
    // called once

    // initialize internal LED
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);

    Serial.begin(115200);
    // delay so the Serial terminal is ready?? (min ~ 1800 ms)
    delay(500);
    // blink for 4.5 s
    bool high = true;
    for (uint i = 0; i < 45; i++) {
        delay(100); digitalWrite(LED_BUILTIN, high = !high ? HIGH : LOW);
    }

    Serial.println();
    byte mac[6];
    WiFi.macAddress(mac);
    Serial.printf("MAC-Address: %02x:%02x:%02x:%02x:%02x:%02x\nChip-ID: %06x\n",
            mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], ESP.getChipId());
    Serial.println();

    Serial.println("Initializing ... ");

    // disable automatic credentials saving to flash to reduce unnecessary wear
    WiFi.persistent(false);

    Serial.print("Mounting File System ... ");
    if (!LittleFS.begin()) {
        Serial.println("\nERROR: Failed to mount file system");
        return;
    }
    Serial.println("DONE.");

    // append Chip ID to SSID
    snprintf(ap_ssid, 32, "%s#%06x", AP_SSID, ESP.getChipId());
    strlcpy(ap_psk, AP_PSK, 64);

    snprintf(hostname, 32, "%s-%06x", HOSTNAME, ESP.getChipId());

#ifdef CREATE_DEFAULT_CONFIG
    Serial.print("Creating default config ... ");
    File f = LittleFS.open("/config", "w");
    if (!f) {
        Serial.println("\nERROR: Failed to open `/config` as w");
        return;
    }

    f.println(ap_ssid);
    f.println(ap_psk);
    f.println("true");
    f.println("0.0.0.0");
    f.println("0.0.0.0");
    f.println("0.0.0.0");
    f.println("0.0.0.0");
    f.println("0.0.0.0");
    f.println("false");
    f.println("false");
    f.println("f");
    f.println("false");
    f.println("50");

    f.close();
    Serial.println("DONE.");
#endif

    Serial.print("Reading configs ... ");
    File configFile = LittleFS.open("/config", "r");
    if (!configFile) {
        Serial.println("\nERROR: Failed to open `/config` as r");
        return;
    }

    //Serial.println();
    //Serial.println(" ==== BEGIN read from `/config`");
    bool err = false;
    char buf[16];
    err = err || readLine(configFile, sta_ssid, 32);
    //Serial.println(sta_ssid);
    err = err || readLine(configFile, sta_psk, 64);
    //Serial.println(sta_psk);
    err = err || readLine(configFile, buf, 6);
    //Serial.println(buf);
    if (strncmp(buf, "true", 5) == 0) {
        sta_dhcp = true;
    } else if (strncmp(buf, "false", 6) == 0) {
        sta_dhcp = false;
    } else {
        err = true;
    }
    err = err || readLine(configFile, buf, 16);
    //Serial.println(buf);
    sta_ip.fromString(buf);
    err = err || readLine(configFile, buf, 16);
    //Serial.println(buf);
    sta_gateway.fromString(buf);
    err = err || readLine(configFile, buf, 16);
    //Serial.println(buf);
    sta_subnet.fromString(buf);
    err = err || readLine(configFile, buf, 16);
    //Serial.println(buf);
    sta_priDNS.fromString(buf);
    err = err || readLine(configFile, buf, 16);
    //Serial.println(buf);
    sta_secDNS.fromString(buf);
    err = err || readLine(configFile, buf, 6);
    //Serial.println(buf);
    if (strncmp(buf, "true", 5) == 0) {
        flipHighLow = true;
    } else if (strncmp(buf, "false", 6) == 0) {
        flipHighLow = false;
    } else {
        err = true;
    }
    err = err || readLine(configFile, buf, 6);
    //Serial.println(buf);
    if (strncmp(buf, "true", 5) == 0) {
        pin2asIn = true;
    } else if (strncmp(buf, "false", 6) == 0) {
        pin2asIn = false;
    } else {
        err = true;
    }
    err = err || readLine(configFile, buf, 2);
    //Serial.println(buf);
    if (strncmp(buf, "r", 2) == 0) {
        interruptOn = RISING;
    } else if (strncmp(buf, "f", 2) == 0) {
        interruptOn = FALLING;
    } else if (strncmp(buf, "c", 2) == 0) {
        interruptOn = CHANGE;
    } else {
        err = true;
    }
    err = err || readLine(configFile, buf, 6);
    //Serial.println(buf);
    if (strncmp(buf, "true", 5) == 0) {
        sendImp = true;
    } else if (strncmp(buf, "false", 6) == 0) {
        sendImp = false;
    } else {
        err = true;
    }
    err = err || readLine(configFile, buf, 11);
    //Serial.println(buf);
    impDur = strtoul(buf, NULL, 0);
    //Serial.println(" ==== END");

    if (err) {
        Serial.println("\nERROR: Failed to parse config file");
        configFile.seek(0);
        Serial.println(" ==== START OF FILE");
        Serial.println(configFile.readString());
        Serial.println(" ==== END OF FILE");
        return;
    }
    configFile.close();
    Serial.println("DONE.");

    Serial.println();
    Serial.println(" ==== BEGIN internal data:");
    Serial.println(sta_ssid);
    Serial.println(sta_psk);
    Serial.println(sta_dhcp ? "true" : "false");
    Serial.println(sta_ip.toString());
    Serial.println(sta_gateway.toString());
    Serial.println(sta_subnet.toString());
    Serial.println(sta_priDNS.toString());
    Serial.println(sta_secDNS.toString());
    Serial.println(flipHighLow ? "true" : "false");
    Serial.println(pin2asIn ? "true" : "false");
    Serial.println(interruptOn);
    Serial.println(sendImp ? "true" : "false");
    Serial.println(impDur);
    Serial.println(" ==== END");

    // configure WiFi
    Serial.print("Configuring WiFi ... ");
    WiFi.hostname(hostname);
    //WiFi.softAPConfig(IPAddress(192, 168, 4, 1), IPAddress(192, 168, 4, 254), IPAddress(255, 255, 255, 0));
    WiFi.config(sta_ip, sta_gateway, sta_subnet, sta_priDNS, sta_secDNS);
    WiFi.setAutoReconnect(false);
    WiFi.mode(WIFI_STA);
    Serial.println("DONE.");
    //Serial.printf("auto reconnect set to: %s\n", WiFi.getAutoReconnect() ? "true" : "false");

    // add Event Handlers for WiFi
    Serial.print("Adding event handlers ... ");
    wifiConnectedHandler = WiFi.onStationModeConnected(&handleStationModeConnected);
    wifiGotIPHandler = WiFi.onStationModeGotIP(&handleStationModeGotIP);
    wifiDisconnectedHandler = WiFi.onStationModeDisconnected(&handleStationModeDisconnected);
    Serial.println("DONE.");

    Serial.print("Starting up WiFi ... ");
    WiFi.begin(sta_ssid, sta_psk);
    Serial.println("DONE.");

    Serial.print("Starting mDNS ... ");
    MDNS.begin(hostname);
    Serial.println("DONE.");

    Serial.print("Starting up webserver ... ");
    server.begin(80);
    Serial.println("DONE.");

    // add Event Handlers for webserver
    Serial.print("Adding path handlers ... ");
    // ping
    server.on("/", HTTP_GET, handlePing);
    // settings
    server.on("/settings", HTTP_GET, handleSettings);
    server.on("/settings", HTTP_POST, handleSettingsPOST);
    server.on("/settings.js", HTTP_GET, handleSettingsJS);
    server.on("/config", HTTP_GET, handleConfig);
    // gpio (default mapped to gpio0)
    server.on("/pin", HTTP_GET, getGPIO0);
    server.on("/pin", HTTP_POST, switchGPIO0);
    server.on("/pin/1", HTTP_POST, setGPIO0_1);
    server.on("/pin/0", HTTP_POST, setGPIO0_0);
    // gpio0
    server.on("/pin0", HTTP_GET, getGPIO0);
    server.on("/pin0", HTTP_POST, switchGPIO0);
    server.on("/pin0/1", HTTP_POST, setGPIO0_1);
    server.on("/pin0/0", HTTP_POST, setGPIO0_0);
    // gpio2 if defined as output
    if (!pin2asIn) {
        server.on("/pin2", HTTP_GET, getGPIO2);
        server.on("/pin2", HTTP_POST, switchGPIO2);
        server.on("/pin2/1", HTTP_POST, setGPIO2_1);
        server.on("/pin2/0", HTTP_POST, setGPIO2_0);
    }
    server.onNotFound(handleNotFound);
    Serial.println("DONE.");

    // initialize GPIO pins
    Serial.print("Initializing GPIO pins ... ");
    if (pin2asIn) {
        pinMode(PIN_2, INPUT_PULLUP);
        attachInterrupt(digitalPinToInterrupt(PIN_2), onInterruptPin2, interruptOn);
    } else {
        pinMode(PIN_2, OUTPUT);
        digitalWrite(PIN_2, flipHighLow ? HIGH : LOW);
    }
    pinMode(PIN_0, OUTPUT);
    digitalWrite(PIN_0, flipHighLow ? HIGH : LOW);
    Serial.println("DONE.");

    Serial.println("Initialization completed.");
}

void loop() {
    // called indefinitely
    server.handleClient();
    // give server time to do WiFi stuff!!
    delay(20);

    if (WiFi.status() != WL_CONNECTED && millis() > nextTryAfter) {
        WiFi.begin(sta_ssid, sta_psk);
        nextTryAfter = millis() + retryAfter;
    }

    if ((WiFi.getMode() == WIFI_AP || WiFi.getMode() == WIFI_AP_STA) && WiFi.status() == WL_CONNECTED) {
        WiFi.softAPdisconnect(true);
    }

    //debug:
    //Serial.printf("Free heap: %d\n", ESP.getFreeHeap());
    //delay(500);

    //delay(1000);
    //Serial.printf("mode: %d\n", WiFi.getMode());
    //Serial.printf("status: %d\n", WiFi.status());
}

bool readLine(Stream &stream, char *buf, int len) {
    bool err = false;
    char c;
    int idx = 0;
    --len;// decrement length to make room for NULL-terminator
    while (true) {
        c = stream.read();
        if (c == '\n') {
            buf[idx] = 0;
            break;
        } else if (c == '\r') {
            if (stream.peek() == '\n') {
                stream.read();
            }
            buf[idx] = 0;
            break;
        } else if (idx >= len) {
            buf[idx] = 0;
            err = true;
        } else {
            buf[idx++] = c;
        }
    }
    return err;
}


ICACHE_RAM_ATTR void onInterruptPin2() {
    Serial.println("Interrupt!");
    bool isHigh = digitalRead(PIN_0) == HIGH;
    digitalWrite(PIN_0, !isHigh ? HIGH : LOW);
    if (sendImp) {
        delay(impDur);
        digitalWrite(PIN_0, isHigh ? HIGH : LOW);
    }
}


void handleStationModeConnected(const WiFiEventStationModeConnected& e) {
    Serial.printf("connected to SSID `%s` \n", WiFi.SSID().c_str());
}

void handleStationModeGotIP(const WiFiEventStationModeGotIP& e) {
    Serial.print("got IP; reachable under ");
    Serial.println(WiFi.localIP());
}

void handleStationModeDisconnected(const WiFiEventStationModeDisconnected& e) {
    Serial.println("disconnected");
    if (WiFi.getMode() != WIFI_AP_STA) {
        WiFi.mode(WIFI_AP_STA);
        bool success = WiFi.softAP(ap_ssid, ap_psk);
        Serial.printf("AP %s: SSID `%s`, PSK `%s`, IP %s\n",
                success ? "up" : "failed",
                WiFi.softAPSSID().c_str(),
                WiFi.softAPPSK().c_str(),
                WiFi.softAPIP().toString().c_str());
    }
}


void handleNotFound() {
    server.send(404, "text/plain", "Resource not found.");
}

void handlePing() {
    server.send(204);
}


void getGPIO2() {
    bool isHigh = digitalRead(PIN_2) == HIGH;
    server.send(200, "text/plain", isHigh != flipHighLow ? "1" : "0");
}

void switchGPIO2() {
    bool isHigh = digitalRead(PIN_2) == HIGH;
    digitalWrite(PIN_2, !isHigh ? HIGH : LOW);
    server.send(200, "text/plain", !isHigh != flipHighLow ? "1" : "0");
    if (sendImp) {
        delay(impDur);
        digitalWrite(PIN_2, isHigh ? HIGH : LOW);
    }
}

void setGPIO2_1() {
    digitalWrite(PIN_2, !flipHighLow ? HIGH : LOW);
    server.send(200, "text/plain", "1");
}

void setGPIO2_0() {
    digitalWrite(PIN_2, !flipHighLow ? LOW : HIGH);
    server.send(200, "text/plain", "0");
}


void getGPIO0() {
    bool isHigh = digitalRead(PIN_0) == HIGH;
    server.send(200, "text/plain", isHigh != flipHighLow ? "1" : "0");
}

void switchGPIO0() {
    bool isHigh = digitalRead(PIN_0) == HIGH;
    digitalWrite(PIN_0, !isHigh ? HIGH : LOW);
    server.send(200, "text/plain", !isHigh != flipHighLow ? "1" : "0");
    if (sendImp) {
        delay(impDur);
        digitalWrite(PIN_0, isHigh ? HIGH : LOW);
    }
}

void setGPIO0_1() {
    digitalWrite(PIN_0, !flipHighLow ? HIGH : LOW);
    server.send(200, "text/plain", "1");
}

void setGPIO0_0() {
    digitalWrite(PIN_0, !flipHighLow ? LOW : HIGH);
    server.send(200, "text/plain", "0");
}


void handleSettings() {
    server.send_P(200, "text/html", settings_html);
}

void handleSettingsJS() {
    server.send_P(200, "application/javascript", settings_js);
}

void handleConfig() {
    char c;
    if (interruptOn == RISING) {
        c = 'r';
    } else if (interruptOn == FALLING) {
        c = 'f';
    } else {
        c = 'c';
    }
    char json[512];
    snprintf_P(json, 512, json_template, sta_ssid,
            sta_ip[0], sta_ip[1], sta_ip[2], sta_ip[3],
            sta_gateway[0], sta_gateway[1], sta_gateway[2], sta_gateway[3],
            sta_subnet[0], sta_subnet[1], sta_subnet[2], sta_subnet[3],
            sta_priDNS[0], sta_priDNS[1], sta_priDNS[2], sta_priDNS[3],
            sta_secDNS[0], sta_secDNS[1], sta_secDNS[2], sta_secDNS[3],
            c, impDur, impDur, sta_dhcp ? "true" : "false",
            flipHighLow ? "true" : "false",
            pin2asIn ? "true" : "false",
            sendImp ? "true" : "false");

    Serial.println(json);

    server.send(200, "application/json", json);
}

void handleSettingsPOST() {
    char sta_ssid_new[32];
    char sta_psk_new[64];
    bool sta_dhcp_new;
    IPAddress sta_ip_new = 0u;
    IPAddress sta_gateway_new = 0u;
    IPAddress sta_subnet_new = 0u;
    IPAddress sta_priDNS_new = 0u;
    IPAddress sta_secDNS_new = 0u;
    bool flipHighLow_new;
    bool pin2asIn_new;
    byte interruptOn_new;
    bool sendImp_new;
    ulong impDur_new;
    // get and check form params
    bool err = true;
    do {
        strlcpy(sta_ssid_new, server.arg("sta_ssid").c_str(), 32);
        if (strlen(sta_ssid_new) < 1) {
            continue;
        }
        // when input is empty, don't change psk
        strlcpy(sta_psk_new, strlen(server.arg("sta_psk").c_str()) != 0 ? server.arg("sta_psk").c_str() : sta_psk, 64);
        if (strlen(sta_psk_new) < 8) {
            continue;
        }
        sta_dhcp_new = server.hasArg("sta_dhcp");
        String ip = server.arg("ip0")+"."+server.arg("ip1")+"."+server.arg("ip2")+"."+server.arg("ip3");
        if (!sta_ip_new.fromString(ip.c_str())) {
            continue;
        }
        String gw = server.arg("gw0")+"."+server.arg("gw1")+"."+server.arg("gw2")+"."+server.arg("gw3");
        if (!sta_gateway_new.fromString(gw.c_str())) {
            continue;
        }
        String sm = server.arg("sm0")+"."+server.arg("sm1")+"."+server.arg("sm2")+"."+server.arg("sm3");
        if (!sta_subnet_new.fromString(sm.c_str())) {
            continue;
        }
        String pd = server.arg("pd0")+"."+server.arg("pd1")+"."+server.arg("pd2")+"."+server.arg("pd3");
        if (!sta_priDNS_new.fromString(pd.c_str())) {
            continue;
        }
        String sd = server.arg("sd0")+"."+server.arg("sd1")+"."+server.arg("sd2")+"."+server.arg("sd3");
        if (!sta_secDNS_new.fromString(sd.c_str())) {
            continue;
        }
        flipHighLow_new = server.hasArg("flipHighLow");
        pin2asIn_new = server.hasArg("pin2asIn");
        char c = server.arg("interruptOn").charAt(0);
        if (c == 'r') {
            interruptOn_new = RISING;
        } else if (c == 'f') {
            interruptOn_new = FALLING;
        } else if (c == 'c') {
            interruptOn_new = CHANGE;
        } else {
            continue;
        }
        sendImp_new = server.hasArg("sendImp");
        impDur_new = strtoul(server.arg("impDur").c_str(), NULL, 0);

        err = false;

    } while (false);

    char cfg[512];
    snprintf(cfg, 512, "%s\nSSID: %s\nPSK: %s\nDHCP: %s\nIP: %s\nGW: %s\nSN: %s\npDNS: %s\nsDNS: %s\nflipHighLow: %s\npin2asIn: %s\ninterruptOn: %d\nsendImp: %s\nimpDur: %u",
            err ? "invalid data received:" : "",
            sta_ssid_new,
            sta_psk_new,
            sta_dhcp_new ? "true" : "false",
            sta_ip_new.toString().c_str(),
            sta_gateway_new.toString().c_str(),
            sta_subnet_new.toString().c_str(),
            sta_priDNS_new.toString().c_str(),
            sta_secDNS_new.toString().c_str(),
            flipHighLow_new ? "true" : "false",
            pin2asIn_new ? "true" : "false",
            interruptOn_new,
            sendImp_new ? "true" : "false",
            impDur_new);
    if (err) {
        server.send(400, "text/plain", cfg);
        return;
    }

    bool reloadWiFi = false;
    if (strcmp(sta_ssid, sta_ssid_new) != 0
            || strcmp(sta_psk, sta_psk_new) != 0
            || sta_dhcp != sta_dhcp_new
            || sta_ip != sta_ip_new
            || sta_gateway != sta_gateway_new
            || sta_subnet != sta_subnet_new
            || sta_priDNS != sta_priDNS_new
            || sta_secDNS != sta_secDNS_new) {
        reloadWiFi = true;
    }

    // adopt config
    strlcpy(sta_ssid, sta_ssid_new, 32);
    strlcpy(sta_psk, sta_psk_new, 64);
    sta_dhcp = sta_dhcp_new;
    sta_ip = sta_ip_new;
    sta_gateway = sta_gateway_new;
    sta_subnet = sta_subnet_new;
    sta_priDNS = sta_priDNS_new;
    sta_secDNS = sta_secDNS_new;
    flipHighLow = flipHighLow_new;
    pin2asIn = pin2asIn_new;
    interruptOn = interruptOn_new;
    sendImp = sendImp_new;
    impDur = impDur_new;

    File configFile = LittleFS.open("/config", "w");
    if (!configFile) {
        Serial.println("Failed to open `/config` as w");
        server.send(500, "text/plain", "Failed to open file");
        return;
    }

    configFile.println(sta_ssid);
    configFile.println(sta_psk);
    configFile.println(sta_dhcp ? "true" : "false");
    configFile.println(sta_ip.toString());
    configFile.println(sta_gateway.toString());
    configFile.println(sta_subnet.toString());
    configFile.println(sta_priDNS.toString());
    configFile.println(sta_secDNS.toString());
    configFile.println(flipHighLow ? "true" : "false");
    configFile.println(pin2asIn ? "true" : "false");
    char c;
    if (interruptOn == RISING) {
        c = 'r';
    } else if (interruptOn == FALLING) {
        c = 'f';
    } else {
        c = 'c';
    }
    configFile.println(c);
    configFile.println(sendImp ? "true" : "false");
    configFile.println(impDur);

    configFile.close();
    Serial.println("Config changed.");

    // (re)bind handlers if neccessary
    detachInterrupt(digitalPinToInterrupt(PIN_2));
    if (pin2asIn) {
        pinMode(PIN_2, INPUT_PULLUP);
        attachInterrupt(digitalPinToInterrupt(PIN_2), onInterruptPin2, interruptOn);
        server.on("/gpio2", HTTP_GET, handleNotFound);
        server.on("/gpio2", HTTP_POST, handleNotFound);
        server.on("/gpio2/1", HTTP_POST, handleNotFound);
        server.on("/gpio2/0", HTTP_POST, handleNotFound);
    } else {
        pinMode(PIN_2, OUTPUT);
        bool isHigh = digitalRead(PIN_2) == HIGH;
        digitalWrite(PIN_2, isHigh == !flipHighLow ? HIGH : LOW);
        server.on("/gpio2", HTTP_GET, getGPIO2);
        server.on("/gpio2", HTTP_POST, switchGPIO2);
        server.on("/gpio2/1", HTTP_POST, setGPIO2_1);
        server.on("/gpio2/0", HTTP_POST, setGPIO2_0);
    }
    bool isHigh = digitalRead(PIN_0) == HIGH;
    digitalWrite(PIN_0, isHigh == !flipHighLow ? HIGH : LOW);

    // send response to client
    server.send(200, "text/plain", cfg);
    // retry WiFi connection if any WiFi changed
    if (reloadWiFi) {
        WiFi.config(sta_ip, sta_gateway, sta_subnet, sta_priDNS, sta_secDNS);
        WiFi.begin(sta_ssid, sta_psk);
        Serial.println("reconnecting WiFi ...");
    }
}
