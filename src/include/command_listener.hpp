#ifndef COMMAND_LISTENER_HPP
#define COMMAND_LISTENER_HPP

#include <vector>
#include <ESP8266WiFi.h>
#include "task.hpp"
#include "box.hpp"
#include "date_time.hpp"


namespace Luvitronics
{
    class CommandListener : public Task
    {
    public: 
        CommandListener(uint16_t port, std::vector<std::shared_ptr<Box>> *boxesArray, Luvitronics::DateTime *dt);
        
        virtual void process() override;
        
    private:
        WiFiServer _serverCom;
        WiFiClient _client;
        std::vector<std::shared_ptr<Box>>* boxes;
        
        Luvitronics::DateTime *dateTime;
    };
}

#endif //DIMMER_LISTENER_HPP
