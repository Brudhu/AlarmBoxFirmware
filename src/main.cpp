/*
   1MB flash sizee

   sonoff header
   1 - vcc 3v3
   2 - rx
   3 - tx
   4 - gnd
   5 - gpio 14

   esp8266 connections
   gpio  0 - button
   gpio 12 - relay
   gpio 13 - green led - active low
   gpio 14 - pin 5 on header

*/

#include <vector>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Ticker.h>
#include <DNSServer.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
#include <EEPROM.h>
//#include <ArduinoOTA.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include "box.hpp"
#include "fw_updater.hpp"
#include "http_listener.hpp"
#include "command_listener.hpp"
#include "hardware_constants.hpp"
#include "date_time.hpp"

#define EEPROM_SALT 12663
typedef struct {
  int   salt = EEPROM_SALT;
} WMSettings;

WMSettings settings;

WiFiUDP udp;
unsigned int localUdpPort = 2115;
unsigned int remoteUdpPort = 123;
IPAddress timeServerIP;
const char* ntpServerName = "pool.ntp.org";//"time.nist.gov";
#define NTP_PACKET_SIZE 48
byte packetBuffer[NTP_PACKET_SIZE];
uint8_t gotTime = 0;

unsigned long epoch = 0;
unsigned long lastEpoch = 0;
unsigned long lastMillis = 0;
Luvitronics::DateTime dateTime(&epoch);
int lastMinutes = -1;

//for LED status
Ticker ticker;
Ticker tickerOTA;
Ticker tickerTCP;
Ticker tickerPB;
Ticker tickerPWM;
Ticker tickerGetTime;
Ticker tickerCheckUDP;
Ticker tickerCheckAlarm;

bool ota = 0;
bool pb = 0;
bool tcp = 0;
bool pwmState = 0;
bool getTime = 0;
bool checkUDP = 0;
bool checkAlarm = 0;

std::vector<std::shared_ptr<Box>> boxes;
Luvitronics::FWUpdater* fwUpdater = new Luvitronics::FWUpdater();
Luvitronics::HttpListener* httpListener = new Luvitronics::HttpListener(80, &boxes);
Luvitronics::CommandListener* commandListener = new Luvitronics::CommandListener(2211, &boxes, &dateTime);

const int CMD_WAIT = 0;
const int CMD_BUTTON_CHANGE = 1;

int cmd = CMD_WAIT;
int relayState = HIGH;

//inverted button state
int buttonState = HIGH;

static long startPress = 0;

String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete

void tick()
{
  //toggle state
  int state = digitalRead(Hardware::SonoffLed);  // get the current state of GPIO1 pin
  digitalWrite(Hardware::SonoffLed, !state);     // set pin to the opposite state
}

void tickOTA()
{
  ota = 1;
}

void tickTCP()
{
  tcp = 1;
}

void tickPB()
{
  pb = 1;
}

void tickPWM()
{
  pwmState = !pwmState;
  analogWrite(Hardware::BoxBuzzer, 512 * pwmState);
}

void tickGetTime()
{
    getTime = 1;
    if(gotTime == 1)
    {
        gotTime++;
        tickerGetTime.attach(300, tickGetTime);
    }
}

void tickCheckUDP()
{
  checkUDP = 1;
}

void tickCheckAlarm()
{
  checkAlarm = 1;
}

//gets called when WiFiManager enters configuration mode
void configModeCallback (WiFiManager *myWiFiManager) {
  ticker.attach(0.2, tick);
}


void setState(int s) {
  digitalWrite(Hardware::SonoffRelay, s);
  digitalWrite(Hardware::SonoffLed, (s + 1) % 2); // led is active low
}

void turnOn() {
  relayState = HIGH;
  setState(relayState);
}

void turnOff() {
  relayState = LOW;
  setState(relayState);
}

void toggleState() {
  cmd = CMD_BUTTON_CHANGE;
}

template<int i>
void resetBox(){
    auto& box = boxes.at(i - 1);
    box->resetAlarmState();
  
    for(auto& box : boxes)
    {
        if(box->getAlarmState())
            return;
    }
    pwmState = 0;
    analogWrite(Hardware::BoxBuzzer, 512 * pwmState);
    tickerPWM.detach();
  //digitalWrite(i, 0);
}

//flag for saving data
bool shouldSaveConfig = false;

//callback notifying us of the need to save config
void saveConfigCallback () {
  //Serial.println("Should save config");
  shouldSaveConfig = true;
}


void toggle() {
  //Serial.println("toggle state");
  //Serial.println(relayState);
  relayState = relayState == HIGH ? LOW : HIGH;
  setState(relayState);
}

void restart() {
  ESP.reset();
  delay(1000);
}

void reset() {
  WiFi.disconnect();
  delay(1000);
  ESP.reset();
  delay(1000);
}

// send an NTP request to the time server at the given address
unsigned long sendNTPpacket(IPAddress& address)
{
  //Serial.println("sending NTP packet...");
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  udp.beginPacket(address, 123); //NTP requests are to port 123
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();
}

void setup()
{
    Serial.begin(115200);

    inputString.reserve(200);

    //set led pin as output
    pinMode(Hardware::SonoffLed, OUTPUT);
    // start ticker with 0.5 because we start in AP mode and try to connect
    ticker.attach(0.6, tick);


    const char *hostname = "ESP8266";

    WiFiManager wifiManager;
    //set callback that gets called when connecting to previous WiFi fails, and enters Access Point mode
    wifiManager.setAPCallback(configModeCallback);

    //timeout - this will quit WiFiManager if it's not configured in 3 minutes, causing a restart
    wifiManager.setConfigPortalTimeout(180);

    //custom params
    EEPROM.begin(512);
    EEPROM.get(0, settings);
    EEPROM.end();

    if (settings.salt != EEPROM_SALT) {
        Serial.println("Invalid settings in EEPROM, trying with defaults");
        WMSettings defaults;
        settings = defaults;
    }

    //set config save notify callback
    wifiManager.setSaveConfigCallback(saveConfigCallback);

    if (!wifiManager.autoConnect(hostname)) {
        Serial.println("failed to connect and hit timeout");
        //reset and try again, or maybe put it to deep sleep
        ESP.reset();
        delay(1000);
    }

    //save the custom parameters to FS
    if (shouldSaveConfig) {
        Serial.println("Saving config");

        EEPROM.begin(512);
        EEPROM.put(0, settings);
        EEPROM.end();
    }

    //if you get here you have connected to the WiFi
    Serial.println("connected...yeey :)");
    ticker.detach();

    // Print the IP address
    Serial.print("Use this URL to connect: ");
    Serial.print("http://");
    Serial.print(WiFi.localIP());
    Serial.println("/");


    udp.begin(localUdpPort);
    WiFi.hostByName(ntpServerName, timeServerIP); 
    sendNTPpacket(timeServerIP);

    //setup button
    pinMode(Hardware::SonoffButton, INPUT);
    attachInterrupt(Hardware::SonoffButton, toggleState, CHANGE);
    //setup relay
    pinMode(Hardware::SonoffRelay, OUTPUT);
    //setup box IOs
    Box* bxs[] = {
        new Box(1, Hardware::LedPinBox1, Hardware::LidButtonPinBox1)
    };
    for (auto box : bxs) boxes.emplace_back(box);
    attachInterrupt(Hardware::LidButtonPinBox1, resetBox<Hardware::LidButtonPinBox1>, RISING);
    
    pinMode(Hardware::BoxBuzzer, OUTPUT);
    analogWriteFreq(1000);
    analogWrite(Hardware::BoxBuzzer, 0);

    turnOff();
    //Serial.println("done setup");

    tickerOTA.attach(0.1, tickOTA);
    tickerTCP.attach(0.05, tickTCP);
    tickerPB.attach(0.1, tickPB);
    //tickerPWM.attach(0.5, tickPWM);
    tickerGetTime.attach(5, tickGetTime); // every 5 seconds, then 5 minutes (get ntp time)
    tickerCheckUDP.attach(1, tickCheckUDP);
    tickerCheckAlarm.attach(10, tickCheckAlarm);
  
}

void loop()
{
  delay(20);
  
  if(ota)
  {
    ota = 0;
    fwUpdater->process();
  }

  if(tcp)
  {
    tcp = 0;
    httpListener->process();
    commandListener->process();
  }

  if(pb)
  {
    pb = 0;
    switch (cmd) {
      case CMD_WAIT:
        break;
      case CMD_BUTTON_CHANGE:
        int currentState = digitalRead(Hardware::SonoffButton);
        if (currentState != buttonState) {
          if (buttonState == LOW && currentState == HIGH) {
            long duration = millis() - startPress;
            if (duration < 1000) {
              //Serial.println("short press - toggle relay");
              toggle();
            } else if (duration < 5000) {
              //Serial.println("medium press - reset");
              restart();
            } else if (duration < 60000) {
              //Serial.println("long press - reset settings");
              reset();
            }
          } else if (buttonState == HIGH && currentState == LOW) {
            startPress = millis();
          }
          buttonState = currentState;
        }
        break;
    }
  }
  
  if(getTime)
  {
    getTime = 0;
    
    WiFi.hostByName(ntpServerName, timeServerIP); 
    sendNTPpacket(timeServerIP);
    
  }
  
  if(checkUDP)
  {
    checkUDP = 0;
    
    int cb = udp.parsePacket();
    if (cb)
    {
      if(!gotTime)
        gotTime++;
      // We've received a packet, read the data from it
      udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

      //the timestamp starts at byte 40 of the received packet and is four bytes,
      // or two words, long. First, esxtract the two words:

      unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
      unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
      // combine the four bytes (two words) into a long integer
      // this is NTP time (seconds since Jan 1 1900):
      unsigned long secsSince1900 = highWord << 16 | lowWord;

      const unsigned long seventyYears = 2208988800UL;
      // subtract seventy years:
      lastEpoch = epoch;
      epoch = secsSince1900 - seventyYears;
      dateTime.processEpoch();
      
      lastMillis = millis();
    }
  }
  
  if(checkAlarm)
  {
    checkAlarm = 0;
      
    dateTime.process();
    
    if(lastMinutes != dateTime.getMinute())
    {
        bool timeEqual = 0;
        for(auto& box : boxes)
        {

            std::vector<std::pair<uint8_t,uint8_t>> currentWETimes = box->getWEAlarmTimes();
            std::vector<std::pair<uint8_t,uint8_t>> currentWDTimes = box->getWDAlarmTimes();
            
            //TODO: Find a way to turn on only that box' alarm
            if(dateTime.getDWeek() == 0 || dateTime.getDWeek() == 6)
                for(auto& time : currentWETimes)
                {
                    if(std::get<0>(time) == dateTime.getHour() && std::get<1>(time) == dateTime.getMinute())
                        timeEqual = 1;
                }
            else
                for(auto time : currentWDTimes)
                {
                    if(std::get<0>(time) == dateTime.getHour() && std::get<1>(time) == dateTime.getMinute())
                        timeEqual = 1;
                }
            if(timeEqual)
            {
                tickerPWM.attach(0.5, tickPWM);
                box->setAlarmState(1);
            }
        }
        lastMinutes = dateTime.getMinute();
    }
    
  }

}





