#include "InfraredHandler.h"

InfraredHandler::InfraredHandler(Dezibot &robot) : dezibot(robot) {}

void InfraredHandler::begin()
{
    dezibot.infraredLight.begin();
}

bool InfraredHandler::obstacleInFront()
{
    dezibot.infraredLight.front.turnOn();
    uint16_t distance = dezibot.lightDetection.getValue(IR_FRONT);
    dezibot.infraredLight.front.turnOff();
    if (distance > threshold)
    {
        dezibot.display.println("STOP");
        return true;
    }
    return false;
}

bool InfraredHandler::dezibotOnRightSide()
{
    dezibot.infraredLight.front.turnOn();
    uint16_t distanceRight = dezibot.lightDetection.getValue(IR_RIGHT);
    dezibot.infraredLight.front.turnOff();

    if(distanceRight > thresholdDezibotOnRightSide){
        return true;
    }

    return false;
}