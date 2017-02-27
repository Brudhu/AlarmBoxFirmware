#ifndef BOX_HPP
#define BOX_HPP

#include <vector>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <EEPROM.h>

#define NUM_TIMES   10
#define WD_POS      1 + (2 * NUM_TIMES)

class Box
{
public: 
    Box(uint16_t boxNumber, uint8_t ledPinNumber, uint8_t lidButtonPinNumber);
    
    std::vector<std::pair<uint8_t,uint8_t>> getWEAlarmTimes();
    bool addWEAlarmTime(std::pair<uint8_t,uint8_t> time);
    bool delWEAlarmTime(uint8_t position);
    
    std::vector<std::pair<uint8_t,uint8_t>> getWDAlarmTimes();
    bool addWDAlarmTime(std::pair<uint8_t,uint8_t> time);
    bool delWDAlarmTime(uint8_t position);
    
    void setAlarmState(bool newState);
    bool getAlarmState();
    
    void resetAlarmState();
    
    uint8_t getBoxNumber();
    uint8_t getLedPin();
    uint8_t getLidButtonPin();
    uint8_t getLenWETimes();
    uint8_t getLenWDTimes();
    
private:
    uint8_t number;
    uint8_t ledPin;
    uint8_t lidButtonPin;
    bool alarmOn;
    std::vector<std::pair<uint8_t,uint8_t>> weekEndAlarmTimes;
    std::vector<std::pair<uint8_t,uint8_t>> weekDayAlarmTimes;
    uint16_t eepromPosition;
    uint8_t lenWEAlarmTimes;
    uint8_t lenWDAlarmTimes;
    
    //constexpr unsigned MaxNumTimes = 10;
    //constexpr unsigned WdPos = 1 + 2*MaxNumTimes;
};

#endif //DIMMER_LISTENER_HPP
