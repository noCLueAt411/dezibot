#include <Arduino.h>
#include <Dezibot.h>
#include <veml6040.h>
#include <vector>
#include <numeric>



struct ColorValues {
    int red, green, blue, white;

    ColorValues() = default;
    ColorValues(int r, int g, int b, int w) : red(r), green(g), blue(b), white(w) {}

    bool in_bound(ColorValues& expectation, ColorValues& std_dev) {
        int std_dev_tolerance_factor = 3;
        return value_in_bound(red, expectation.red, std_dev_tolerance_factor*std_dev.red) && 
            value_in_bound(green, expectation.green, std_dev_tolerance_factor*std_dev.green) && 
            value_in_bound(blue, expectation.blue, std_dev_tolerance_factor*std_dev.blue) && 
            value_in_bound(white, expectation.white, std_dev_tolerance_factor*std_dev.white);
    }

    bool value_in_bound(int& value, int& expected_value, int tolerance) {
        int higher_bound = expected_value + tolerance;
        int lower_bound = expected_value - tolerance;
        return lower_bound <= value && higher_bound >= value;
    }

    int total_dist(ColorValues& expectation) {
         return std::abs(std::abs(red) - std::abs(expectation.red)) + std::abs(std::abs(green) - std::abs(expectation.green)) + std::abs(std::abs(blue) - std::abs(expectation.blue)) + std::abs(std::abs(white) - std::abs(expectation.white));
    }
};

struct ColorBound {
    ColorValues expectation;
    ColorValues standard_deviation;

    ColorBound() = default;
    ColorBound(std::vector<ColorValues> color_values_list) {
        calculate_expectations(color_values_list);
        calculate_standard_deviations(color_values_list);
    }

    private:
    void calculate_expectations(std::vector<ColorValues> values) {
        std::vector<int> red_values, green_values, blue_values, white_values;
        expectation.red = static_cast<int>(std::accumulate(values.begin(), values.end(), 0,
            [](int sum, const ColorValues& v) { return sum + v.red; }) / values.size());

        expectation.green = static_cast<int>(std::accumulate(values.begin(), values.end(), 0,
            [](int sum, const ColorValues& v) { return sum + v.green; }) / values.size());

        expectation.blue = static_cast<int>(std::accumulate(values.begin(), values.end(), 0,
            [](int sum, const ColorValues& v) { return sum + v.blue; }) / values.size());

        expectation.white = static_cast<int>(std::accumulate(values.begin(), values.end(), 0,
            [](int sum, const ColorValues& v) { return sum + v.white; }) / values.size());
    }

    void calculate_standard_deviations(std::vector<ColorValues> values) {
        std::vector<int> red_values, green_values, blue_values, white_values;
        standard_deviation.red = static_cast<int>(std::sqrt(std::accumulate(values.begin(), values.end(), 0,
            [this](float sum, const ColorValues& v) { return sum + std::pow(v.red - expectation.red, 2); }) / values.size()));

        standard_deviation.green = static_cast<int>(std::sqrt(std::accumulate(values.begin(), values.end(), 0,
            [this](float sum, const ColorValues& v) { return sum + std::pow(v.green - expectation.green, 2); }) / values.size()));

        standard_deviation.blue = static_cast<int>(std::sqrt(std::accumulate(values.begin(), values.end(), 0,
            [this](float sum, const ColorValues& v) { return sum + std::pow(v.blue - expectation.blue, 2); }) / values.size()));

        standard_deviation.white = static_cast<int>(std::sqrt(std::accumulate(values.begin(), values.end(), 0,
            [this](float sum, const ColorValues& v) { return sum + std::pow(v.white - expectation.white, 2); }) / values.size()));
    }
};

struct ColorData {
    String name;
    std::vector<ColorValues> color_values_list;
    ColorBound bound;

    ColorData(String name) : name(name) {}

    void calculate_statistical_data() {
        bound = ColorBound(color_values_list);
    }
};





VEML6040 color_detection;
Dezibot dezibot;


ColorValues get_cur_color() {
    return ColorValues(
        color_detection.getRed(),
        color_detection.getGreen(),
        color_detection.getBlue(),
        color_detection.getWhite()
    );
}

void collect_color_data(ColorData* color_data, int& amount_data) {
    for(int i=0; i<amount_data; i++) {
        dezibot.display.clear();
        dezibot.display.println("add " + color_data->name + " values");
        ColorValues cur_color = get_cur_color();
        color_data->color_values_list.push_back(cur_color);
    }
}


ColorData white_data = ColorData("white");
ColorData black_data = ColorData("black");
ColorData red_data = ColorData("red");
ColorData green_data = ColorData("green");

std::vector<ColorData*> data = {&white_data, &black_data, &red_data, &green_data};

int amount_values = 50;
bool initialized = false;

void setup() {
    dezibot.begin();
    color_detection.begin();
    dezibot.multiColorLight.setLed(BOTTOM, 255, 255, 255);
}

void loop() {
    if(!initialized) {
        for(ColorData* color_data : data) {
            dezibot.display.clear();
            dezibot.display.println("place robot on\n" + color_data->name);
            delay(3000);
            collect_color_data(color_data, amount_values);
            color_data->calculate_statistical_data();
        }
        initialized = true;
    } 

    ColorValues cur_color = get_cur_color();

    int total_red, total_green, total_white, total_black;

    bool identified = false;
    dezibot.display.clear();
    for(ColorData* color_data : data) {
        if(cur_color.in_bound(color_data->bound.expectation, color_data->bound.standard_deviation)) {
            dezibot.display.println(color_data->name);
            identified = true;
            break;
        }
    }
    if(!identified) {
        dezibot.display.println("Unknown");
    }
    delay(333);
}