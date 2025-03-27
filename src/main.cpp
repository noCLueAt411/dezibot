#include "traffic_simulation/line_detection.h"
#include "traffic_simulation/InfraredHandler/InfraredHandler.h"

Dezibot dezibot;
LineDetection lineDetection(dezibot);
InfraredHandler irHandler(dezibot);

LinePart lastLinePart = MIDDLE;

void setup() {
    dezibot.begin();
    lineDetection.begin();
    irHandler.begin();
}


void loop() {
    dezibot.display.clear();
    
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

    if(linePart != lastLinePart) {
        dezibot.motion.stop();
        if(linePart == LEFT) {
            dezibot.motion.rotateClockwise();
        } else if(linePart == RIGHT) {
            dezibot.motion.rotateAntiClockwise();
        } else if(linePart == MIDDLE) {
            dezibot.motion.move();
        } else if(linePart == INDICATOR) {
            dezibot.motion.rotateClockwise();
            delay(1500);
            dezibot.motion.stop();
            if(irHandler.dezibotOnRightSide()) {
                dezibot.display.println("DETECTED");
                delay(1000);
            }
            dezibot.motion.rotateAntiClockwise();
            delay(1500);
            dezibot.motion.stop();
        } else if(linePart == CROSSING) {
            dezibot.motion.move();
        }
    }
    lastLinePart = linePart;

    delay(200);
}