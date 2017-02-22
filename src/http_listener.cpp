#include "http_listener.hpp"
#include "hardware_constants.hpp"

#include <string>

Luvitronics::HttpListener::HttpListener(uint16_t port, std::vector<std::shared_ptr<Box>> *boxesArray)
    : Task(), _server(port)
{
    boxes = boxesArray;
    _server.begin();
}

void Luvitronics::HttpListener::process() {
    auto client = _server.available();

    for (; client && client.connected(); delay(10)) {
        if (!client.available())
            continue;
            
        String request = client.readStringUntil('\r');

        // Return the response
        client.println("HTTP/1.1 200 OK");
        client.println("Content-Type: text/html");
        client.println(""); //  do not forget this one
        client.println("<!DOCTYPE HTML>");
        client.println("<html>");
        char message[60];
        
        for(auto& box : *boxes)
        {
            sprintf(message, "Box%d:\n", box->getBoxNumber());
            client.print(message);
            
            std::vector<std::pair<uint8_t,uint8_t>> currentWETimes = box->getWEAlarmTimes();
            uint8_t i = 0;
            //for(auto i = 0; i < box.getLenWETimes(); ++i)
            for(auto& time : currentWETimes)
            {
                ++i;
                sprintf(message, "<p>Weekend Time %d: %02d:%02d</p>\n<p></p>", i, (uint8_t)std::get<0>(time), (uint8_t)std::get<1>(time));
                client.print(message);
            }
            
            std::vector<std::pair<uint8_t,uint8_t>> currentWDTimes = box->getWDAlarmTimes();
            i = 0;
            for(auto& time : currentWDTimes)
            {
                ++i;
                sprintf(message, "<p>Weekday Time %d: %02d:%02d</p>\n", i, (uint8_t)std::get<0>(time), (uint8_t)std::get<1>(time));
                client.print(message);
            }
            sprintf(message, "<p>Alarm Box%d = %d</p>\n<p></p>", box->getBoxNumber(), digitalRead(Hardware::LedPin));
            client.print(message);
        }
        
        client.println("<br><br>");
        client.println("<a href=\"/\"\"><button>Update </button></a>");
        client.println("</html>");
        
        
        break;
        
    }

    // close the connection:
    client.stop();
}
