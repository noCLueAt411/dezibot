#include <Arduino.h>
#include <Dezibot.h>
#include <veml6040.h>
#include <vector>
//#include "motion/Motion.h"

VEML6040 color_detection;
Dezibot dezibot;


struct rgbw_values {
    int red;
    int green;
    int blue;
    int white;
};


int white_counter = 0;
int black_counter = 0;
int red_counter = 0;
int green_counter = 0;

int white_ambient_total = 0;
int black_ambient_total = 0;
int red_ambient_total = 0;
int green_ambient_total = 0;

int white_light_total = 0;
int black_light_total = 0;
int red_light_total = 0;
int green_light_total = 0;

int white_total = 0;
int black_total = 0;
int red_total = 0;
int green_total = 0;

int amount_values = 50;

bool waited = false;


std::vector<int> white_values;
std::vector<int> black_values;


void setup() {
    dezibot.begin();
    color_detection.begin();
    dezibot.multiColorLight.setLed(BOTTOM, 255, 255, 255);

    Motion::left.begin();
    Motion::right.begin();
    //dezibot.motion.detection.begin();
} 

void loop() {
    if(!waited) {
        dezibot.display.println("wait for white");
        delay(15000);
        waited = true;
    }
    
    if(white_counter < amount_values) {
        dezibot.display.clear();
        dezibot.display.println("add white values");
        int white_value = color_detection.getWhite();
        white_values.push_back(white_value);
        white_total += white_value;
        white_ambient_total += color_detection.getAmbientLight();
        white_light_total += dezibot.lightDetection.getValue(DL_BOTTOM);
        white_counter++;
    }

    if(white_counter == amount_values && black_counter == 0) {
        dezibot.display.clear();
        dezibot.display.println("white done,");
        dezibot.display.println("place on black");
        delay(3000);        
    }

    if(white_counter == amount_values && black_counter < amount_values) {
        dezibot.display.clear();
        dezibot.display.println("add black values");
        int black_value = color_detection.getWhite();
        black_values.push_back(black_value);
        black_total += black_value;
        black_ambient_total += color_detection.getAmbientLight();
        black_light_total += dezibot.lightDetection.getValue(DL_BOTTOM);
        black_counter++;
    }

    if(white_counter == amount_values && black_counter == amount_values) {
        float white_final = static_cast<float>(white_total) / static_cast<float>(amount_values);
        float black_final = static_cast<float>(black_total) / static_cast<float>(amount_values);

        float white_light_final = static_cast<float>(white_light_total) / static_cast<float>(amount_values);
        float black_light_final = static_cast<float>(black_light_total) / static_cast<float>(amount_values);
        
        float e_x_white = .0f;
        for(auto value : white_values) {
            e_x_white += 1/static_cast<float>(amount_values) * static_cast<float>(value);
        }

        float var_x_white = .0f;

        for(auto value : white_values) {
            var_x_white += (static_cast<float>(value) - e_x_white) * (static_cast<float>(value) - e_x_white) * 1/static_cast<float>(amount_values);
        }

        float sigma_x_white = std::sqrt(var_x_white);


        float e_x_black = .0f;
        for(auto value : black_values) {
            e_x_black += 1/static_cast<float>(amount_values) * static_cast<float>(value);
        }

        float var_x_black = .0f;

        for(auto value : black_values) {
            var_x_black += (static_cast<float>(value) - e_x_black) * (static_cast<float>(value) - e_x_black) * 1/static_cast<float>(amount_values);
        }

        float sigma_x_black = std::sqrt(var_x_black);



        dezibot.display.clear();
        int cur_value = color_detection.getWhite();
        dezibot.display.println(cur_value);
        dezibot.display.println("white range: ");
        dezibot.display.print(e_x_white - 3*sigma_x_white);
        dezibot.display.print("->");
        dezibot.display.print(e_x_white + 3*sigma_x_white);
        dezibot.display.print("\n");
        dezibot.display.println("black range: ");
        dezibot.display.print(e_x_black - 3*sigma_x_black);
        dezibot.display.print("->");
        dezibot.display.print(e_x_black + 3*sigma_x_black);
        dezibot.display.print("\n");

        if(cur_value > e_x_white - 3*sigma_x_white && cur_value < e_x_white + 3*sigma_x_white) {
            dezibot.display.println("White");
        } else if (cur_value > e_x_black - 3*sigma_x_black && cur_value < e_x_black + 3*sigma_x_black) {
            dezibot.display.println("Black");
        } else {
            dezibot.display.println("Unknown");
        }
        delay(333);



        /*long max_value = 65536 * 16348;
        float correction_factor = max_value / (white_final * white_light_final - black_final * black_light_final);

        float correct_white = static_cast<float>(color_detection.getWhite()) * static_cast<float>(color_detection.getWhite()) * correction_factor;

        dezibot.display.clear();
        dezibot.display.println("white: " + String(correct_white));
        dezibot.display.println("ambient: " + String(color_detection.getAmbientLight()));*/
        //dezibot.display.println("ambient: " + String(dezibot.lightDetection.getValue(DL_BOTTOM)));

        // define black as zero and white as 100. for values multiply light and ambient


    }


   /*dezibot.display.println("r: " + String(color_detection.getRed()));
   dezibot.display.println("g: " + String(color_detection.getGreen()));
   dezibot.display.println("b: " + String(color_detection.getBlue()));
   dezibot.display.println("w: " + String(color_detection.getWhite()));
   delay(100);
   dezibot.display.clear();
    
    delay(5000);
    while(true) {
        Motion::left.setSpeed(3900);
        Motion::right.setSpeed(3900);
    }*/
    

    //delay(100);
    //dezibot.display.clear();
}