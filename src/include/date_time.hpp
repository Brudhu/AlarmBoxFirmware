#ifndef DATE_TIME_HPP
#define DATE_TIME_HPP

#include "task.hpp"
#include "hardware_constants.hpp"
#include <stdint.h>
#include <Arduino.h>


namespace Luvitronics
{
    class DateTime : public Task
    {
    public: 
        DateTime(unsigned long *epoch);
        
        uint8_t getDWeek();
        char* getDWeekStr(uint8_t dWeek);
        uint8_t getHour();
        uint8_t getMinute();
        uint8_t getSecond();
        
        void setDWeek(uint8_t newValue);
        void setHour(uint8_t newValue);
        void setMinute(uint8_t newValue);
        void setSecond(uint8_t newValue);
        
        void processEpoch();
        virtual void process() override;
        
    private:
        uint8_t dt_dWeek;
        uint8_t dt_hour;
        uint8_t dt_min;
        uint8_t dt_sec;
        
        unsigned long dt_lastEpoch;
        unsigned long *dt_epoch;
        unsigned long dt_lastMillis;
        unsigned long dt_millis;
    };
}

#endif //DATE_TIME_HPP
