#include "line_detection.h"
#include "InfraredHandler/InfraredHandler.h"


Dezibot dezibot;
LineDetection lineDetection(dezibot);
InfraredHandler ir(dezibot);

void setup() {
    dezibot.begin();
    lineDetection.begin();
    ir.begin(); 
}


void loop() {

    ir.obstacleInFront();

    lineDetection.proceed();
    LinePart linePart = lineDetection.getLinePart();

    if(linePart == LEFT) {
        dezibot.display.println("LEFT");
    } else if(linePart == RIGHT) {
        dezibot.display.println("RIGHT");
    } else if(linePart == MIDDLE) {
        dezibot.display.println("MIDDLE");
    } else if(linePart == INDICATOR) {
        dezibot.display.println("INDICATOR");
    } else if(linePart == CROSSING) {
        dezibot.display.println("CROSSING");
    } else if(linePart == BACKGROUND) {
        dezibot.display.println("BACKGROUND");
    }

    delay(200);
}