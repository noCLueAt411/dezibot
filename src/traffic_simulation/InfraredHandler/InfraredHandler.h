#ifndef INFRARED_HANDLER_H
#define INFRARED_HANDLER_H

#include <Dezibot.h>

class InfraredHandler {
public:
    InfraredHandler(Dezibot& robot);  // Konstruktor mit Referenz auf den Dezibot

    void begin();
    bool obstacleInFront();  // Gibt true zurück, wenn gestoppt wurde
    bool dezibotOnRightSide();

private:
    Dezibot& dezibot;
    const uint16_t threshold = 4050;  // Anpassen je nach Sensorverhalten
    const uint16_t thresholdDezibotOnRightSide = 4010;
};

#endif