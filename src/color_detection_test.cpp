#include <Arduino.h>
#include <Dezibot.h>
#include <veml6040.h>
#include <vector>

VEML6040 color_detection;
Dezibot dezibot;

struct rgbw_values {
    int red;
    int green;
    int blue;
    int white;
};

int white_counter = 0, black_counter = 0, red_counter = 0, green_counter = 0;
int white_total = 0, black_total = 0, red_total = 0, green_total = 0;
int white_ambient_total = 0, black_ambient_total = 0, red_ambient_total = 0, green_ambient_total = 0;
int white_light_total = 0, black_light_total = 0, red_light_total = 0, green_light_total = 0;

int amount_values = 50;
bool waited = false;

std::vector<int> white_values, black_values, red_values, green_values;

void setup() {
    dezibot.begin();
    color_detection.begin();
    dezibot.multiColorLight.setLed(BOTTOM, 255, 255, 255);
}

void loop() {
    if (!waited) {
        dezibot.display.println("wait for white");
        delay(5000);
        waited = true;
    }
    
    if (white_counter < amount_values) {
        dezibot.display.clear();
        dezibot.display.println("add white values");
        int white_value = color_detection.getWhite();
        white_values.push_back(white_value);
        white_total += white_value;
        white_counter++;
        if (white_counter >= amount_values) {
            dezibot.display.clear();
            dezibot.display.println("get ready for black");
            delay(3000);
        }
    } else if (black_counter < amount_values) {
        dezibot.display.clear();
        dezibot.display.println("add black values");
        int black_value = color_detection.getWhite();
        black_values.push_back(black_value);
        black_total += black_value;
        black_counter++;
        if (black_counter >= amount_values) {
            dezibot.display.clear();
            dezibot.display.println("get ready for red");
            delay(3000);
        }
    } else if (red_counter < amount_values) {
        dezibot.display.clear();
        dezibot.display.println("add red values");
        int red_value = color_detection.getRed();
        red_values.push_back(red_value);
        red_total += red_value;
        red_counter++;
        if (red_counter >= amount_values) {
            dezibot.display.clear();
            dezibot.display.println("get ready for green");
            delay(3000);
        }
    } else if (green_counter < amount_values) {
        dezibot.display.clear();
        dezibot.display.println("add green values");
        int green_value = color_detection.getGreen();
        green_values.push_back(green_value);
        green_total += green_value;
        green_counter++;
    } else {
        float e_x_white = static_cast<float>(white_total) / static_cast<float>(amount_values);
        float e_x_black = static_cast<float>(black_total) / static_cast<float>(amount_values);
        float e_x_red = static_cast<float>(red_total) / static_cast<float>(amount_values);
        float e_x_green = static_cast<float>(green_total) / static_cast<float>(amount_values);

        float sigma_x_white = sqrt(e_x_white);
        float sigma_x_black = sqrt(e_x_black);
        float sigma_x_red = sqrt(e_x_red);
        float sigma_x_green = sqrt(e_x_green);

        dezibot.display.clear();
        int cur_white = color_detection.getWhite();
        int cur_red = color_detection.getRed();
        int cur_green = color_detection.getGreen();
        
        if (cur_white >= e_x_white - sigma_x_white && cur_white <= e_x_white + sigma_x_white) {
            dezibot.display.println("White");
        } else if (cur_white >= e_x_black - sigma_x_black && cur_white <= e_x_black + sigma_x_black) {
            dezibot.display.println("Black");
        } else if (cur_red >= e_x_red - sigma_x_red && cur_red <= e_x_red + sigma_x_red) {
            dezibot.display.println("Red");
        } else if (cur_green >= e_x_green - sigma_x_green && cur_green <= e_x_green + sigma_x_green) {
            dezibot.display.println("Green");
        } else {
            dezibot.display.println("Unknown");
        }
        delay(333);
    }
}
