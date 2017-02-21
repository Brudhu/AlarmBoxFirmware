#include "command_listener.hpp"
#include "hardware_constants.hpp"

#include <string>

Luvitronics::CommandListener::CommandListener(uint16_t port, std::vector<std::shared_ptr<Box>> *boxesArray, int *systemHour, int *systemMin, int *systemSec)
    : Task(), _serverCom(port)
{
    boxes = boxesArray;
    systemTimeHour = systemHour;
    systemTimeMin = systemMin;
    systemTimeSec = systemSec;
    _serverCom.begin();
}

void Luvitronics::CommandListener::process() {
    //_client = _serverCom.available();
    if(!_client.connected())
      _client = _serverCom.available();

    for (; _client && _client.connected(); delay(10)) {
        if (!_client.available())
            break;
            
        String request = _client.readStringUntil('\n');
        
        char requestChar[40];
        request.toCharArray(requestChar, 40);
        int boxNumber;
        int hour;
        int minute;
        int delIndex;
        char charAux[40];
        if(sscanf(requestChar, "B%dWET.Add=%d:%d", &boxNumber, &hour, &minute) == 3)
        {
            if(boxes->at(boxNumber - 1)->addWEAlarmTime(std::make_pair(hour, minute)))
                _client.println("OK");
            else
                _client.println("Fail");
        }
        
        else if(sscanf(requestChar, "B%dWDT.Add=%d:%d", &boxNumber, &hour, &minute) == 3)
        {
            if(boxes->at(boxNumber - 1)->addWDAlarmTime(std::make_pair(hour, minute)))
                _client.println("OK");
            else
                _client.println("Fail");
        }
        
        else if(sscanf(requestChar, "B%dWET.Del=%d", &boxNumber, &delIndex) == 2)
        {
            if(boxes->at(boxNumber - 1)->delWEAlarmTime(delIndex))
                _client.println("OK");
            else
                _client.println("Fail");
        }
        
        else if(sscanf(requestChar, "B%dWDT.Del=%d", &boxNumber, &delIndex) == 2)
        {
            if(boxes->at(boxNumber - 1)->delWDAlarmTime(delIndex))
                _client.println("OK");
            else
                _client.println("Fail");
        }
        
        else if (request.indexOf("CURTIME?") != -1)
        {
            _client.print(*systemTimeHour);
            _client.print(":");
            _client.print(*systemTimeMin);
            _client.print(":");
            _client.println(*systemTimeSec);
        }
        
        
        break;
        
    }
}
