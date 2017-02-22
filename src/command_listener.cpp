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
        
        bool okStatus = 0;
        if(sscanf(requestChar, "B%dWET.Add=%d:%d", &boxNumber, &hour, &minute) == 3)
        {
            okStatus = boxes->at(boxNumber - 1)->addWEAlarmTime(std::make_pair(hour, minute));
        }
        
        else if(sscanf(requestChar, "B%dWDT.Add=%d:%d", &boxNumber, &hour, &minute) == 3)
        {
            okStatus = boxes->at(boxNumber - 1)->addWDAlarmTime(std::make_pair(hour, minute));
        }
        
        else if(sscanf(requestChar, "B%dWET.Del=%d", &boxNumber, &delIndex) == 2)
        {
            okStatus = boxes->at(boxNumber - 1)->delWEAlarmTime(delIndex);
        }
        
        else if(sscanf(requestChar, "B%dWDT.Del=%d", &boxNumber, &delIndex) == 2)
        {
            okStatus = boxes->at(boxNumber - 1)->delWDAlarmTime(delIndex);
        }
        
        else if (request.indexOf("CURTIME?") != -1)
        {
            char message[10];
            sprintf(message, "%02d:%02d:%02d", *systemTimeHour, *systemTimeMin, *systemTimeSec);
            _client.println(message);
            okStatus = 1;
        }
        
        if (okStatus)
            _client.println("OK\n");
        else
            _client.println("Fail\n");
        
        
        break;
        
    }
}
