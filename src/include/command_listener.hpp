#ifndef COMMAND_LISTENER_HPP
#define COMMAND_LISTENER_HPP

#include <vector>
#include <ESP8266WiFi.h>
#include "task.hpp"
#include "box.hpp"


namespace Luvitronics
{
    class CommandListener : public Task
    {
    public: 
        CommandListener(uint16_t port, std::vector<std::shared_ptr<Box>> *boxesArray, int *systemHour, int *systemMin, int *systemSec);
        
        virtual void process() override;
        
    private:
        WiFiServer _serverCom;
        WiFiClient _client;
        std::vector<std::shared_ptr<Box>>* boxes;
        
        int *systemTimeHour;
        int *systemTimeMin;
        int *systemTimeSec;
    };
}

#endif //DIMMER_LISTENER_HPP
