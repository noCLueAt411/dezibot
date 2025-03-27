#include "InfraredHandler.h"

InfraredHandler::InfraredHandler(Dezibot &robot) : dezibot(robot) {}

void InfraredHandler::begin()
{
    dezibot.infraredLight.begin();
    dezibot.infraredLight.front.turnOn(); // Dauerbetrieb
}

bool InfraredHandler::obstacleInFront()
{
    uint16_t distance = dezibot.lightDetection.getValue(IR_FRONT);
    if (distance > threshold)
    {
        dezibot.display.println("STOP");
        return true;
    }
    return false;
}

bool InfraredHandler::dezibotOnRightSide()
{
    uint16_t distanceRight = dezibot.lightDetection.getValue(IR_RIGHT);

    if(distanceRight > thresholdDezibotOnRightSide){
        return true;
    }

    return false;
}