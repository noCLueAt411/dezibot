#include "line_detection.h"


Dezibot dezibot;
LineDetection lineDetection(dezibot);

void setup() {
    dezibot.begin();
    lineDetection.begin();
}


void loop() {
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