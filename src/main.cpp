#include "traffic_simulation/line_detection.h"
#include "traffic_simulation/InfraredHandler/InfraredHandler.h"

Dezibot dezibot;
LineDetection lineDetection(dezibot);
InfraredHandler irHandler(dezibot);

LinePart lastLinePart = MIDDLE;

bool checked = false;
bool crossed = false;

void setup() {
    dezibot.begin();
    lineDetection.begin();
    irHandler.begin();
}

int time_counter = 0;
int data_counter = 0;

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
    } else if(linePart == TRAFFIC_LIGHT_STOP) {
        dezibot.display.println("TRAFFIC_LIGHT_STOP");
    } else if(linePart == TRAFFIC_LIGHT_GO) {
        dezibot.display.println("TRAFFIC_LIGHT_GO");
    }

    if(linePart != lastLinePart) {
        lastLinePart = linePart;
        dezibot.motion.stop();
        if(linePart == LEFT) {
            dezibot.motion.rotateClockwise();
        } else if(linePart == RIGHT) {
            dezibot.motion.rotateAntiClockwise();
        } else if(linePart == MIDDLE || linePart == TRAFFIC_LIGHT_GO) {
            dezibot.motion.move();
        } else if(linePart == INDICATOR && !checked) {
            dezibot.motion.rotateClockwise();
            delay(1500);
            dezibot.motion.stop();
            if(irHandler.dezibotOnRightSide()) {
                dezibot.display.println("DETECTED");
                delay(1000);
            }
            while(irHandler.dezibotOnRightSide()) {
                delay(200);
            }
            dezibot.motion.rotateAntiClockwise();
            delay(1500);
            dezibot.motion.stop();
            while(irHandler.dezibotOnRightSide()) {
                delay(200);
            }
            checked = true;
        } else if(linePart == INDICATOR && crossed) {
            crossed = false;
            checked = false;
            lineDetection.proceed();
            while(lineDetection.getLinePart() == INDICATOR) {
                lineDetection.proceed();
                dezibot.motion.move();
                delay(200);
            }
        } else if(linePart == CROSSING) {
            crossed = true;
            dezibot.motion.move();
        }
    }

    /*lineDetection.printData(data_counter);
    if(time_counter % 5 == 0) {
        data_counter++;
        if(data_counter == 7) {
            data_counter = 0;
        }
    }
    time_counter++;*/

    delay(200);
}

