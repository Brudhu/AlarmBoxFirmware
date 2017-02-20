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
    
    std::vector<std::pair<int,int>> getWEAlarmTimes();
    void addWEAlarmTime(std::pair<int,int> time);
    void delWEAlarmTime(uint8_t position);
    
    std::vector<std::pair<int,int>> getWDAlarmTimes();
    void addWDAlarmTime(std::pair<int,int> time);
    void delWDAlarmTime(uint8_t position);
    
    void setAlarmState(bool newState);
    bool getAlarmState();
    
    void resetAlarmState();
    
private:
    uint16_t number;
    uint8_t ledPin;
    uint8_t lidButtonPin;
    bool alarmOn;
    std::vector<std::pair<int,int>> weekEndAlarmTimes;
    std::vector<std::pair<int,int>> weekDayAlarmTimes;
    uint16_t eepromPosition;
    uint16_t lenWEAlarmTimes;
    uint16_t lenWDAlarmTimes;
};

#endif //DIMMER_LISTENER_HPP
