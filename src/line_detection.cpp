#include <Arduino.h>
#include <Dezibot.h>
#include <InfraredHandler/InfraredHandler.h>
#include <veml6040.h>
#include <vector>
#include <numeric>
#include <queue>

struct ColorValues
{
    float red, green, blue, white;

    ColorValues() = default;
    ColorValues(float r, float g, float b, float w) : red(r), green(g), blue(b), white(w) {}

    bool in_bound(const ColorValues &expectation, const ColorValues &std_dev)
    {
        float std_dev_tolerance_factor = 2.f;
        return value_in_bound(red, expectation.red, std_dev_tolerance_factor * std_dev.red) &&
               value_in_bound(green, expectation.green, std_dev_tolerance_factor * std_dev.green) &&
               value_in_bound(blue, expectation.blue, std_dev_tolerance_factor * std_dev.blue);// &&
               //value_in_bound(white, expectation.white, std_dev_tolerance_factor * std_dev.white);
    }

    bool value_in_bound(const float value, const float expected_value, const float tolerance)
    {
        float higher_bound = expected_value + tolerance;
        float lower_bound = expected_value - tolerance;
        return lower_bound <= value && higher_bound >= value;
    }

    void toRelative()
    {
        /*float light_f = static_cast<float>(light);
        red /= light_f;
        green /= light_f;
        blue /= light_f;
        white /= light_f;*/
        /*float max = 65535;
        float total = red + green + blue;
        red = red / total * (1/(white/max));
        green = green / total * (1/(white/max));
        blue = blue / total * (1/(white/max));*/
        //white = white / total * (1/(white/max));
        red /= white;
        green /= white;
        blue /= white;
    }

    float distance_to(const ColorValues &other, const ColorValues &std_dev) const
    {
        return std::sqrt(
            std::pow((red - other.red) / (std_dev.red), 2.f) +
            std::pow((green - other.green) / (std_dev.green), 2.f) +
            std::pow((blue - other.blue) / (std_dev.blue), 2.f) +
            std::pow((white - other.white) / (std_dev.white), 2.f)
        );
    }
};

struct ColorBound
{
    ColorValues expectation;
    ColorValues standard_deviation;

    ColorBound() = default;

    void calculate_bounds(std::vector<ColorValues> &color_values_list)
    {
        calculate_expectations(color_values_list);
        calculate_standard_deviations(color_values_list);
    }

private:
    void calculate_expectations(const std::vector<ColorValues> &values)
    {
        expectation.red = static_cast<float>(std::accumulate(values.begin(), values.end(), 0.f,
                                                             [](float sum, const ColorValues &v)
                                                             { return sum + v.red; }) /
                                             values.size());

        expectation.green = static_cast<float>(std::accumulate(values.begin(), values.end(), 0.f,
                                                               [](float sum, const ColorValues &v)
                                                               { return sum + v.green; }) /
                                               values.size());

        expectation.blue = static_cast<float>(std::accumulate(values.begin(), values.end(), 0.f,
                                                              [](float sum, const ColorValues &v)
                                                              { return sum + v.blue; }) /
                                              values.size());

        expectation.white = static_cast<float>(std::accumulate(values.begin(), values.end(), 0.f,
                                                               [](float sum, const ColorValues &v)
                                                               { return sum + v.white; }) /
                                               values.size());
    }

    void calculate_standard_deviations(const std::vector<ColorValues> &values)
    {
        standard_deviation.red = static_cast<float>(std::sqrt(std::accumulate(values.begin(), values.end(), 0.f,
                                                                              [this](float sum, const ColorValues &v)
                                                                              { return sum + std::pow(v.red - expectation.red, 2.f); }) /
                                                              values.size()));

        standard_deviation.green = static_cast<float>(std::sqrt(std::accumulate(values.begin(), values.end(), 0.f,
                                                                                [this](float sum, const ColorValues &v)
                                                                                { return sum + std::pow(v.green - expectation.green, 2.f); }) /
                                                                values.size()));

        standard_deviation.blue = static_cast<float>(std::sqrt(std::accumulate(values.begin(), values.end(), 0.f,
                                                                               [this](float sum, const ColorValues &v)
                                                                               { return sum + std::pow(v.blue - expectation.blue, 2.f); }) /
                                                               values.size()));

        standard_deviation.white = static_cast<float>(std::sqrt(std::accumulate(values.begin(), values.end(), 0.f,
                                                                                [this](float sum, const ColorValues &v)
                                                                                { return sum + std::pow(v.white - expectation.white, 2.f); }) /
                                                                values.size()));
    }
};

struct ColorData
{
    const String name;
    std::vector<ColorValues> color_values_list;
    ColorBound bound;

    ColorData(String name) : name(name) {}

    void calculate_statistical_data()
    {
        bound.calculate_bounds(color_values_list);
    }
};

VEML6040 color_detection;
Dezibot dezibot;

ColorValues get_cur_color()
{
    return ColorValues(
        static_cast<float>(color_detection.getRed()),
        static_cast<float>(color_detection.getGreen()),
        static_cast<float>(color_detection.getBlue()),
        static_cast<float>(color_detection.getWhite()));
}

ColorValues get_cur_rel_color()
{
    ColorValues values = ColorValues(
        static_cast<float>(color_detection.getRed()),
        static_cast<float>(color_detection.getGreen()),
        static_cast<float>(color_detection.getBlue()),
        static_cast<float>(color_detection.getWhite()));
    values.toRelative();
    return values;
}

void collect_color_data(ColorData *color_data, int &amount_data)
{
    for (int i = 0; i < amount_data; i++)
    {
        dezibot.display.clear();
        dezibot.display.println("add values for\n" + color_data->name);
        const ColorValues cur_color = get_cur_rel_color();
        color_data->color_values_list.push_back(cur_color);
    }
}

ColorData left_side = ColorData("left");
ColorData right_side = ColorData("right");
ColorData middle = ColorData("middle");
ColorData indicator = ColorData("indicator");
ColorData crossing = ColorData("crossing");
ColorData background = ColorData("background");

std::vector<ColorData *> data = {&left_side, &right_side, &middle, /*&indicator,*/ &crossing, &background};

int amount_values = 35;

InfraredHandler ir(dezibot);

void setup()
{
    dezibot.begin();
    color_detection.begin();
    dezibot.multiColorLight.setLed(BOTTOM, 255, 255, 255);

    //ir.begin();

    for (ColorData *color_data : data)
    {
        dezibot.display.clear();
        dezibot.display.println("place robot on\n" + color_data->name);
        delay(3000);
        collect_color_data(color_data, amount_values);
        color_data->calculate_statistical_data();
    }
}

int direction_i = 0;
int last_direction = 0;

void loop()
{

    //int light = dezibot.lightDetection.getAverageValue(DL_BOTTOM, 40, 5) * 16;
    ColorValues cur_color = get_cur_rel_color();
    dezibot.display.clear();
    /*dezibot.display.println("red: " + String(cur_color.red));
    dezibot.display.println("green: " + String(cur_color.green));
    dezibot.display.println("blue: " + String(cur_color.blue));
    dezibot.display.println("white: " + String(cur_color.white));*/
    //dezibot.display.println("light: " + String(light));

    float distance_left = cur_color.distance_to(left_side.bound.expectation, left_side.bound.standard_deviation);
    float distance_right = cur_color.distance_to(right_side.bound.expectation, right_side.bound.standard_deviation);
    float distance_middle = cur_color.distance_to(middle.bound.expectation, middle.bound.standard_deviation);
    // distance_indicator = cur_color.distance_to(indicator.bound.expectation, indicator.bound.standard_deviation);
    float distance_crossing = cur_color.distance_to(crossing.bound.expectation, crossing.bound.standard_deviation);
    float distance_background = cur_color.distance_to(background.bound.expectation, background.bound.standard_deviation);

    

    dezibot.display.println("left: " + String(distance_left));
    dezibot.display.println("right: " + String(distance_right));
    dezibot.display.println("middle: " + String(distance_middle));
    //dezibot.display.println("indi: " + String(distance_indicator));
    dezibot.display.println("cross: " + String(distance_crossing));
    dezibot.display.println("backgr: " + String(distance_background));

    /*dezibot.display.println("left: " + String(cur_color.in_bound(left_side.bound.expectation, left_side.bound.standard_deviation)));
    dezibot.display.println("right: " + String(cur_color.in_bound(right_side.bound.expectation, right_side.bound.standard_deviation)));
    dezibot.display.println("middle: " + String(cur_color.in_bound(middle.bound.expectation, middle.bound.standard_deviation)));
    //dezibot.display.println("indicator: " + String(cur_color.in_bound(indicator.bound.expectation, indicator.bound.standard_deviation)));
    dezibot.display.println("crossing: " + String(cur_color.in_bound(crossing.bound.expectation, crossing.bound.standard_deviation)));
    dezibot.display.println("background: " + String(cur_color.in_bound(background.bound.expectation, background.bound.standard_deviation)));*/

    std::vector<float> distances = {distance_left, distance_right, distance_middle, /*distance_indicator,*/ distance_crossing, distance_background};
    float min_distance = *std::min_element(distances.begin(), distances.end());

    direction_i = 0;

    /*if(cur_color.in_bound(left_side.bound.expectation, left_side.bound.standard_deviation)) {
        dezibot.multiColorLight.setLed(TOP, 0, 0, 10);
        direction_i = 2;
    } else if(cur_color.in_bound(right_side.bound.expectation, right_side.bound.standard_deviation)) {
        dezibot.multiColorLight.setLed(TOP, 10, 0, 0);
        direction_i = 3;
    } else if(cur_color.in_bound(middle.bound.expectation, middle.bound.standard_deviation)) {
        dezibot.multiColorLight.setLed(TOP, 10, 5, 0);
        direction_i = 1;
    } else if(cur_color.in_bound(indicator.bound.expectation, indicator.bound.standard_deviation)) {
        dezibot.multiColorLight.setLed(TOP, 0, 10, 0);
    } else if(cur_color.in_bound(crossing.bound.expectation, crossing.bound.standard_deviation)) {
        dezibot.multiColorLight.setLed(TOP, 10, 10, 0);
    } else if(cur_color.in_bound(background.bound.expectation, background.bound.standard_deviation)) {
        dezibot.multiColorLight.setLed(TOP, 10, 10, 10);
    } else {
        direction_i = last_direction;
        dezibot.multiColorLight.setLed(TOP, 0, 0, 0);
    }*/

    if(min_distance == distance_left) {
        //dezibot.display.println("left");
        dezibot.multiColorLight.setLed(TOP, 0, 0, 10);
        direction_i = 2;
    } else if(min_distance == distance_right) {
        //dezibot.display.println("right");
        dezibot.multiColorLight.setLed(TOP, 10, 0, 0);
        direction_i = 3;
    } else if(min_distance == distance_middle) {
        //dezibot.display.println("middle");
        dezibot.multiColorLight.setLed(TOP, 10, 5, 0);
        direction_i = 1;
    } /*else if(min_distance == distance_indicator) {
        //dezibot.display.println("indicator");
        dezibot.multiColorLight.setLed(TOP, 0, 10, 0);
    }*/ else if(min_distance == distance_crossing) {
        //dezibot.display.println("crossing");
        dezibot.multiColorLight.setLed(TOP, 10, 10, 0);
    } else if(min_distance == distance_background) {
        //dezibot.display.println("background");
        dezibot.multiColorLight.setLed(TOP, 10, 10, 10);
    } else {
        //dezibot.display.println("Unknown");
        dezibot.multiColorLight.setLed(TOP, 0, 0, 0);
    }
    
    if(last_direction != direction_i) {
        dezibot.motion.stop();
        if(direction_i == 1) {
            dezibot.motion.move();
        } else if (direction_i == 2) {
            dezibot.motion.rotateClockwise();
        } else if (direction_i == 3) {
            dezibot.motion.rotateAntiClockwise();
        }
    }
    last_direction = direction_i;
    

    delay(200);
}