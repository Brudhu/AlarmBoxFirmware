#include "box.hpp"

Box::Box(uint16_t boxNumber, uint8_t ledPinNumber, uint8_t lidButtonPinNumber)
{
    number = boxNumber;
    ledPin = ledPinNumber;
    lidButtonPin = lidButtonPinNumber;
    
    pinMode(lidButtonPin, INPUT);
    //attachInterrupt(lidButtonPin, resetAlarmState, FALLING); // Needs to be done outsite for now
    pinMode(ledPin, OUTPUT);
    digitalWrite(ledPin, 0);
    
    alarmOn = 0;
    
    pinMode(lidButtonPin, INPUT);
    pinMode(ledPin, OUTPUT);
    
    eepromPosition = (513 + (boxNumber - 1) * 42);
    
    EEPROM.begin(1024);
    EEPROM.get(eepromPosition, lenWEAlarmTimes);
    EEPROM.get(eepromPosition + WD_POS, lenWDAlarmTimes);
    
    if(lenWEAlarmTimes > NUM_TIMES)
    {
        lenWEAlarmTimes = 0;
        EEPROM.put(eepromPosition, 0);
    }
    if(lenWDAlarmTimes > NUM_TIMES)
    {
        lenWDAlarmTimes = 0;
        EEPROM.put(eepromPosition + WD_POS, 0);
    }
    
    for(int i = eepromPosition + 1; i < (eepromPosition + 1) + (2 * lenWEAlarmTimes); i += 2)
    {
        uint8_t hour;
        uint8_t minutes;
        
        EEPROM.get(i, hour);
        EEPROM.get(i + 1, minutes);
        
        //addWEAlarmTime(std::make_pair(hour, minutes));
        weekEndAlarmTimes.push_back(std::make_pair(hour, minutes));
    }
    
    for(int i = eepromPosition + WD_POS + 1; i < (eepromPosition + WD_POS + 1) + (2 * lenWDAlarmTimes); i += 2)
    {
        uint8_t hour;
        uint8_t minutes;
        
        EEPROM.get(i, hour);
        EEPROM.get(i + 1, minutes);
        
        //addWDAlarmTime(std::make_pair(hour, minutes));
        weekDayAlarmTimes.push_back(std::make_pair(hour, minutes));
    }
    
    EEPROM.end();
}

std::vector<std::pair<uint8_t,uint8_t>> Box::getWEAlarmTimes()
{
    return weekEndAlarmTimes;
}

void Box::addWEAlarmTime(std::pair<uint8_t,uint8_t> time)
{
    if(lenWEAlarmTimes < NUM_TIMES)
    {
        weekEndAlarmTimes.push_back(time);
        
        lenWEAlarmTimes++;
        EEPROM.begin(1024);
        EEPROM.put(eepromPosition, lenWEAlarmTimes);
        EEPROM.put(eepromPosition + ((lenWEAlarmTimes - 1) * 2) + 1, std::get<0>(time));
        EEPROM.put(eepromPosition + ((lenWEAlarmTimes - 1) * 2) + 2, std::get<1>(time));
        EEPROM.end();
    }
}

void Box::delWEAlarmTime(uint8_t position)
{
    if(position >= 0 && position < lenWEAlarmTimes)
    {
        weekEndAlarmTimes.erase(weekEndAlarmTimes.begin() + position);
        
        lenWEAlarmTimes--;
        EEPROM.begin(1024);
        EEPROM.put(eepromPosition, lenWEAlarmTimes);
        for(int i = eepromPosition + (position * 2) + 1; i < eepromPosition + (lenWEAlarmTimes * 2) + 1; i += 2)
        {
            uint8_t movAux1;
            uint8_t movAux2;
            EEPROM.get(i + 2, movAux1);
            EEPROM.get(i + 3, movAux2);
            EEPROM.put(i, movAux1);
            EEPROM.put(i + 1, movAux2);
        }
        EEPROM.end();
    }
}

std::vector<std::pair<uint8_t,uint8_t>> Box::getWDAlarmTimes()
{
    return weekDayAlarmTimes;
}

void Box::addWDAlarmTime(std::pair<uint8_t,uint8_t> time)
{
    if(lenWDAlarmTimes < NUM_TIMES)
    {
        weekDayAlarmTimes.push_back(time);
        
        lenWDAlarmTimes++;
        EEPROM.begin(1024);
        EEPROM.put(eepromPosition + WD_POS, lenWDAlarmTimes);
        EEPROM.put(eepromPosition + WD_POS + ((lenWDAlarmTimes - 1) * 2) + 1, std::get<0>(time));
        EEPROM.put(eepromPosition + WD_POS + ((lenWDAlarmTimes - 1) * 2) + 2, std::get<1>(time));
        EEPROM.end();
    }
}

void Box::delWDAlarmTime(uint8_t position)
{
    if(position >= 0 && position < lenWDAlarmTimes)
    {
        weekDayAlarmTimes.erase(weekDayAlarmTimes.begin() + position);
        
        lenWDAlarmTimes--;
        EEPROM.begin(1024);
        EEPROM.put(eepromPosition + WD_POS, lenWDAlarmTimes);
        for(int i = eepromPosition + WD_POS + (position * 2) + 1; i < eepromPosition + WD_POS + (lenWDAlarmTimes * 2) + 1; i += 2)
        {
            uint8_t movAux1;
            uint8_t movAux2;
            EEPROM.get(i + 2, movAux1);
            EEPROM.get(i + 3, movAux2);
            EEPROM.put(i, movAux1);
            EEPROM.put(i + 1, movAux2);
        }
        EEPROM.end();
    }
}

void Box::setAlarmState(bool newState)
{
    alarmOn = newState;
    digitalWrite(ledPin, alarmOn);
}

bool Box::getAlarmState()
{
    return alarmOn;
}

void Box::resetAlarmState()
{
    setAlarmState(0);
}

uint8_t Box::getBoxNumber()
{
    return number;
}

uint8_t Box::getLenWETimes()
{
    return lenWEAlarmTimes;
}

uint8_t Box::getLenWDTimes()
{
    return lenWDAlarmTimes;
}




