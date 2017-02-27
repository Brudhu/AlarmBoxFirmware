#include "http_listener.hpp"

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
        request = request.substring(request.indexOf("/AlarmBox") + 10, request.indexOf("HTTP"));
        request.replace("%3A", ":");
        
        char requestChar[200];
        request.toCharArray(requestChar, 200);
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
        
        /*else if(sscanf(requestChar, "B%dAlarm=%d", &boxNumber, &delIndex) == 2)
        {
            boxes->at(boxNumber - 1)->setAlarmState((bool)delIndex);
            okStatus = 1;
        }*/
        
        // Return the response
        client.println("HTTP/1.1 200 OK");
        client.println("Content-Type: text/html");
        client.println(""); //  do not forget this one
        client.println("<!DOCTYPE HTML>");
        client.println("<html>");
        char message[120];
        
        for(auto& box : *boxes)
        {
            sprintf(message, "Box%d:\n", box->getBoxNumber());
            client.print(message);
            
            std::vector<std::pair<uint8_t,uint8_t>> currentWETimes = box->getWEAlarmTimes();
            uint8_t i = 0;
            //for(auto i = 0; i < box.getLenWETimes(); ++i)
            client.print("<p>Weekend:</p>\n<ul style='list-style-type:disc'>\n");
            for(auto& time : currentWETimes)
            {
                ++i;
                sprintf(message, "<li>Time %d: %02d:%02d     ", i, (uint8_t)std::get<0>(time), (uint8_t)std::get<1>(time));
                client.print(message);
                sprintf(message, "<a href=\"/AlarmBox?B%dWET.Del=%d\"\"><button>Del</button></a>\n</li>", box->getBoxNumber(), i);
                client.print(message);
            }
            client.print("</ul>\n<form action='/AlarmBox'>\n<input type='text' name='B1WET.Add' maxlength='5' size='5'><br>\n<input type='submit' value='Add'>\n</form>");
            
            std::vector<std::pair<uint8_t,uint8_t>> currentWDTimes = box->getWDAlarmTimes();
            i = 0;
            client.print("<br><p>Weekday:</p>\n<ul style='list-style-type:disc'>\n");
            for(auto& time : currentWDTimes)
            {
                ++i;
                sprintf(message, "<li>Time %d: %02d:%02d     ", i, (uint8_t)std::get<0>(time), (uint8_t)std::get<1>(time));
                client.print(message);
                sprintf(message, "<a href=\"/AlarmBox?B%dWDT.Del=%d\"\"><button>Del</button></a>\n</li>", box->getBoxNumber(), i);
                client.print(message);
            }
            client.print("</ul>\n<form action='/AlarmBox'>\n<input type='text' name='B1WDT.Add' maxlength='5' size='5'><br>\n<input type='submit' value='Add'>\n</form>");
            sprintf(message, "<p>Alarm Box%d = %d</p>\n<p></p>", box->getBoxNumber(), digitalRead(box->getLedPin()));
            client.print(message);
            /*sprintf(message, "<p><a href=\"/AlarmBox/B%dAlarm=1\"\"><button>Turn on alarm </button></a>", box->getBoxNumber());
            client.print(message);
            sprintf(message, "     <a href=\"/AlarmBox/B%dAlarm=0\"\"><button>Turn off alarm </button></a></p>", box->getBoxNumber());
            client.print(message);*/
        }
        
        client.println("<br><br><br>");
        client.println("<a href=\"/AlarmBox\"\"><button>Update </button></a>");
        client.println("</html>");
        
        
        break;
        
    }

    // close the connection:
    client.stop();
}
