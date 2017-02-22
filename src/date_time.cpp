#include "date_time.hpp"

Luvitronics::DateTime::DateTime(unsigned long *epoch)
{
      dt_epoch = epoch;
      dt_millis = millis();
      
      dt_dWeek = (((*dt_epoch + 3600*Hardware::EstZone) % 604800) / 86400) - 3; // -3 because 1/1/1970 was a Thursday, not Sunday.
      dt_hour = ((*dt_epoch + 3600*Hardware::EstZone) % 86400L) / 3600;
      dt_min = (*dt_epoch  % 3600) / 60;
      dt_sec = *dt_epoch % 60;
      dt_lastMillis = dt_millis;
	
}

uint8_t Luvitronics::DateTime::getDWeek()
{
    return dt_dWeek;
}

uint8_t Luvitronics::DateTime::getHour()
{
    return dt_hour;
}

uint8_t Luvitronics::DateTime::getMinute()
{
    return dt_min;
}

uint8_t Luvitronics::DateTime::getSecond ()
{
    return dt_sec;
}

void Luvitronics::DateTime::setDWeek(uint8_t newValue) {
    dt_dWeek = newValue;
}

void Luvitronics::DateTime::setHour(uint8_t newValue) {
    dt_hour = newValue;
}

void Luvitronics::DateTime::setMinute(uint8_t newValue) {
    dt_min = newValue;
}

void Luvitronics::DateTime::setSecond(uint8_t newValue) {
    dt_sec = newValue;
}

void Luvitronics::DateTime::processEpoch() {
    dt_millis = millis();
    
    dt_dWeek = (((*dt_epoch + 3600*Hardware::EstZone) % 604800) / 86400) - 3; // -3 because 1/1/1970 was a Thursday, not Sunday.
    dt_hour = ((*dt_epoch + 3600*Hardware::EstZone) % 86400L) / 3600;
    dt_min = (*dt_epoch  % 3600) / 60;
    dt_sec = *dt_epoch % 60;
    dt_lastMillis = dt_millis;
    
}

void Luvitronics::DateTime::process() {
    
    unsigned long secsDiff;
    secsDiff = (millis() - dt_lastMillis)/1000;
    dt_lastMillis = millis();
    if(dt_sec + secsDiff >= 60)
    {
        dt_sec = (dt_sec + secsDiff) % 60;
        dt_min++;
    }
    else
        dt_sec = dt_sec + secsDiff;

    if(dt_min >= 60)
    {
        dt_min = dt_min % 60;
        dt_hour++;
        if(dt_hour >= 24)
        {
            dt_hour = dt_hour % 24;
            dt_dWeek++;
    
        if(dt_dWeek >= 7)
            dt_dWeek = dt_dWeek % 7;
        }
    }
    
}
