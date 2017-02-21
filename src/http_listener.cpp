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
        
        for(auto& box : *boxes)
        {
            client.print("Box");
            client.print(box->getBoxNumber());
            client.println(":");
            
            std::vector<std::pair<uint8_t,uint8_t>> currentWETimes = box->getWEAlarmTimes();
            uint8_t i = 0;
            //for(auto i = 0; i < box.getLenWETimes(); ++i)
            for(auto& time : currentWETimes)
            {
                ++i;
                //Serial.print (buffer); std::get<0>(time)
                client.print("<p>");
                client.print("Weekend Time ");
                client.print(i);
                client.print(": ");
                client.print((uint8_t)std::get<0>(time));
                //client.print((int)box1hour);
                client.print(":");
                client.print((uint8_t)std::get<1>(time));
                //client.print((int)box1min);
                client.println("</p>");
                client.print("<p></p>");
            }
            
            std::vector<std::pair<uint8_t,uint8_t>> currentWDTimes = box->getWDAlarmTimes();
            i = 0;
            for(auto& time : currentWDTimes)
            {
                ++i;
                //Serial.print (buffer); std::get<0>(time)
                client.print("<p>");
                client.print("Weekday Time ");
                client.print(i);
                client.print(": ");
                client.print((uint8_t)std::get<0>(time));
                //client.print((int)box1hour);
                client.print(":");
                client.print((uint8_t)std::get<1>(time));
                //client.print((int)box1min);
                client.println("</p>");
            }
            client.print("<p>Alarm Box1 = ");
            client.print(digitalRead(Hardware::LedPin));
            client.println("</p>");
            client.print("<p></p>");
        }
        
        client.println("<br><br>");
        client.println("<a href=\"/\"\"><button>Update </button></a>");
        client.println("</html>");
        
        
        break;
        
    }

    // close the connection:
    client.stop();
}
