#include <Arduino.h>
#include <Dezibot.h>
#include <veml6040.h>
#include <vector>
#include <numeric>



struct ColorValues {
    float red, green, blue, white;

    ColorValues() = default;
    ColorValues(float r, float g, float b, float w) : red(r), green(g), blue(b), white(w) {}

    bool in_bound(const ColorValues& expectation, const ColorValues& std_dev) {
        float std_dev_tolerance_factor = 3.f;
        return value_in_bound(red, expectation.red, std_dev_tolerance_factor*std_dev.red) && 
            value_in_bound(green, expectation.green, std_dev_tolerance_factor*std_dev.green) && 
            value_in_bound(blue, expectation.blue, std_dev_tolerance_factor*std_dev.blue) && 
            value_in_bound(white, expectation.white, std_dev_tolerance_factor*std_dev.white);
    }

    bool value_in_bound(const float value, const float expected_value, const float tolerance) {
        float higher_bound = expected_value + tolerance;
        float lower_bound = expected_value - tolerance;
        return lower_bound <= value && higher_bound >= value;
    }

    void toRelative() {
        float total = red + green + blue + white;
        red = red / total * 100.f;
        green = green / total * 100.f;
        blue = blue / total * 100.f;
        white = white / total * 100.f;
    }
    
    float distance_to(const ColorValues& other, const ColorValues& std_dev) const {
        return std::sqrt(
            std::pow((red - other.red) / std_dev.red, 2.f) +
            std::pow((green - other.green) / std_dev.green, 2.f) +
            std::pow((blue - other.blue) / std_dev.blue, 2.f) +
            std::pow((white - other.white) / std_dev.white, 2.f)
        );
    }
};


struct ColorBound {
    ColorValues expectation;
    ColorValues standard_deviation;

    ColorBound() = default;
    
    void calculate_bounds(std::vector<ColorValues>& color_values_list) {
        calculate_expectations(color_values_list);
        calculate_standard_deviations(color_values_list);
    }

    private:
    void calculate_expectations(const std::vector<ColorValues>& values) {
        expectation.red = static_cast<float>(std::accumulate(values.begin(), values.end(), 0.f,
            [](float sum, const ColorValues& v) { return sum + v.red; }) / values.size());

        expectation.green = static_cast<float>(std::accumulate(values.begin(), values.end(), 0.f,
            [](float sum, const ColorValues& v) { return sum + v.green; }) / values.size());

        expectation.blue = static_cast<float>(std::accumulate(values.begin(), values.end(), 0.f,
            [](float sum, const ColorValues& v) { return sum + v.blue; }) / values.size());

        expectation.white = static_cast<float>(std::accumulate(values.begin(), values.end(), 0.f,
            [](float sum, const ColorValues& v) { return sum + v.white; }) / values.size());
    }

    void calculate_standard_deviations(const std::vector<ColorValues>& values) {
        standard_deviation.red = static_cast<float>(std::sqrt(std::accumulate(values.begin(), values.end(), 0.f,
            [this](float sum, const ColorValues& v) { return sum + std::pow(v.red - expectation.red, 2.f); }) / values.size()));

        standard_deviation.green = static_cast<float>(std::sqrt(std::accumulate(values.begin(), values.end(), 0.f,
            [this](float sum, const ColorValues& v) { return sum + std::pow(v.green - expectation.green, 2.f); }) / values.size()));

        standard_deviation.blue = static_cast<float>(std::sqrt(std::accumulate(values.begin(), values.end(), 0.f,
            [this](float sum, const ColorValues& v) { return sum + std::pow(v.blue - expectation.blue, 2.f); }) / values.size()));

        standard_deviation.white = static_cast<float>(std::sqrt(std::accumulate(values.begin(), values.end(), 0.f,
            [this](float sum, const ColorValues& v) { return sum + std::pow(v.white - expectation.white, 2.f); }) / values.size()));
    }
};

struct ColorData {
    const String name;
    std::vector<ColorValues> color_values_list;
    ColorBound bound;

    ColorData(String name) : name(name) {}

    void calculate_statistical_data() {
        bound.calculate_bounds(color_values_list);
    }
};





VEML6040 color_detection;
Dezibot dezibot;


ColorValues get_cur_color() {
    return ColorValues(
        static_cast<float>(color_detection.getRed()),
        static_cast<float>(color_detection.getGreen()),
        static_cast<float>(color_detection.getBlue()),
        static_cast<float>(color_detection.getWhite())
    );
}

ColorValues get_cur_rel_color() {
    ColorValues values = ColorValues(
        static_cast<float>(color_detection.getRed()),
        static_cast<float>(color_detection.getGreen()),
        static_cast<float>(color_detection.getBlue()),
        static_cast<float>(color_detection.getWhite())
    );
    values.toRelative();
    return values;
}

void collect_color_data(ColorData* color_data, int& amount_data) {
    for(int i=0; i<amount_data; i++) {
        dezibot.display.clear();
        dezibot.display.println("add values for\n" + color_data->name);
        const ColorValues cur_color = get_cur_rel_color();
        color_data->color_values_list.push_back(cur_color);
    }
}


ColorData left_side = ColorData("left");
ColorData right_side = ColorData("right");
ColorData middle = ColorData("middle");
ColorData crossing = ColorData("crossing");
ColorData background = ColorData("background");

std::vector<ColorData*> data = {&left_side, &right_side, &middle, &crossing, &background};

int amount_values = 50;

void setup() {
    dezibot.begin();
    color_detection.begin();
    dezibot.multiColorLight.setLed(BOTTOM, 255, 255, 255);

    for(ColorData* color_data : data) {
        dezibot.display.clear();
        dezibot.display.println("place robot on\n" + color_data->name);
        delay(3000);
        collect_color_data(color_data, amount_values);
        color_data->calculate_statistical_data();
    }
}

void loop() {
    ColorValues cur_color = get_cur_rel_color();

    bool identified = false;
    dezibot.display.clear();

   
    float distance_left = cur_color.distance_to(left_side.bound.expectation, left_side.bound.standard_deviation);
    float distance_right = cur_color.distance_to(right_side.bound.expectation, right_side.bound.standard_deviation);
    float distance_middle = cur_color.distance_to(middle.bound.expectation, middle.bound.standard_deviation);
    float distance_crossing = cur_color.distance_to(crossing.bound.expectation, crossing.bound.standard_deviation);
    float distance_background = cur_color.distance_to(background.bound.expectation, background.bound.standard_deviation);

    if(distance_left < distance_right && distance_left < distance_middle && distance_left < distance_crossing && distance_left < distance_background) {
        dezibot.display.println("left");
        dezibot.multiColorLight.setLed(TOP, 10, 0, 0);
        dezibot.motion.rotateClockwise();
    } else if(distance_right < distance_left && distance_right < distance_middle && distance_right < distance_crossing && distance_right < distance_background) {
        dezibot.display.println("right");
        dezibot.multiColorLight.setLed(TOP, 0, 0, 10);
        dezibot.motion.rotateAntiClockwise();
    } else if(distance_middle < distance_left && distance_middle < distance_right && distance_middle < distance_crossing && distance_middle < distance_background) {
        dezibot.display.println("middle");
        dezibot.multiColorLight.setLed(TOP, 10, 0, 5);
        dezibot.motion.move();
    } else if(distance_crossing < distance_left && distance_crossing < distance_right && distance_crossing < distance_middle && distance_crossing < distance_background) {
        dezibot.display.println("crossing");
        dezibot.multiColorLight.setLed(TOP, 0, 10, 0);
        dezibot.motion.stop();
    } else if(distance_background < distance_left && distance_background < distance_right && distance_background < distance_middle && distance_background < distance_crossing) {
        dezibot.display.println("background");
        dezibot.multiColorLight.setLed(TOP, 10, 10, 10);
        dezibot.motion.stop();
    } else {
        dezibot.display.println("Unknown");
        dezibot.multiColorLight.setLed(TOP, 0, 0, 0);
        dezibot.motion.stop();
    }   
    
    
    delay(333);
}