#ifndef HTTP_LISTENER_HPP
#define HTTP_LISTENER_HPP

#include <vector>
#include <ESP8266WiFi.h>
#include "task.hpp"
#include "box.hpp"


namespace Luvitronics
{
    class HttpListener : public Task
    {
    public: 
        HttpListener(uint16_t port, std::vector<std::shared_ptr<Box>> *boxesArray);
        
        virtual void process() override;
        
    private:
        WiFiServer _server;
        std::vector<std::shared_ptr<Box>>* boxes;
    };
}

#endif //DIMMER_LISTENER_HPP
