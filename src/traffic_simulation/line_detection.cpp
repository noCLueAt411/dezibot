#include "line_detection.h"
#include <numeric>

bool LineDetection::ColorValues::valueInBound(const float value, const float expected_value, const float tolerance) {
    float higher_bound = expected_value + tolerance;
    float lower_bound = expected_value - tolerance;
    return lower_bound <= value && higher_bound >= value;
}

bool LineDetection::ColorValues::inBound(const ColorValues &expectation, const ColorValues &std_dev) {
    float std_dev_tolerance_factor = 2.f;
    return valueInBound(red, expectation.red, std_dev_tolerance_factor * std_dev.red) &&
        valueInBound(green, expectation.green, std_dev_tolerance_factor * std_dev.green) &&
        valueInBound(blue, expectation.blue, std_dev_tolerance_factor * std_dev.blue);
}

float LineDetection::ColorValues::distanceToMean(const ColorValues &expectation, const ColorValues &std_dev) {
    return std::sqrt(
        std::pow((red - expectation.red) / (std_dev.red), 2.f) +
        std::pow((green - expectation.green) / (std_dev.green), 2.f) +
        std::pow((blue - expectation.blue) / (std_dev.blue), 2.f)
    );
}

void LineDetection::ColorBound::calculateExpectations(const std::vector<ColorValues> &values) {
    expectation.red = static_cast<float>(std::accumulate(values.begin(), values.end(), 0.f,
                                                         [](float sum, const ColorValues &v) {
                                                             return sum + v.red;
                                                         }) /
                                         values.size());

    expectation.green = static_cast<float>(std::accumulate(values.begin(), values.end(), 0.f,
                                                           [](float sum, const ColorValues &v) {
                                                               return sum + v.green;
                                                           }) /
                                           values.size());

    expectation.blue = static_cast<float>(std::accumulate(values.begin(), values.end(), 0.f,
                                                          [](float sum, const ColorValues &v) {
                                                              return sum + v.blue;
                                                          }) /
                                          values.size());
}

void LineDetection::ColorBound::calculateStandardDeviations(const std::vector<ColorValues> &values) {
    standard_deviation.red = static_cast<float>(std::sqrt(std::accumulate(values.begin(), values.end(), 0.f,
                                                                      [this](float sum, const ColorValues &v) {
                                                                          return sum + std::pow(v.red - expectation.red, 2.f);
                                                                      }) /
                                                      values.size()));

    standard_deviation.green = static_cast<float>(std::sqrt(std::accumulate(values.begin(), values.end(), 0.f,
                                                                        [this](float sum, const ColorValues &v) {
                                                                            return sum + std::pow(v.green - expectation.green, 2.f);
                                                                        }) /
                                                    values.size()));

    standard_deviation.blue = static_cast<float>(std::sqrt(std::accumulate(values.begin(), values.end(), 0.f,
                                                                       [this](float sum, const ColorValues &v) {
                                                                           return sum + std::pow(v.blue - expectation.blue, 2.f);
                                                                       }) /
                                                   values.size()));
}

void LineDetection::begin() {
    colorDetection.begin();

    dezibot.multiColorLight.setLed(BOTTOM, 255, 255, 255);
    calibrateColors();
}

void LineDetection::calibrateColors() {
    for (ColorData *colorData : data) {
        dezibot.display.clear();
        dezibot.display.println("place robot on\n" + colorData->name);
        delay(3000);
        collectColorData(colorData);
        colorData->calculate_statistical_data();
    }
}

void LineDetection::collectColorData(ColorData *colorData) {
    for (int i = 0; i < amountCalibrationValues; i++) {
        dezibot.display.clear();
        dezibot.display.println("add values for\n" + colorData->name);
        const ColorValues cur_color = getCurRelColor();
        colorData->color_values_list.push_back(cur_color);
    }
}

LineDetection::ColorValues LineDetection::getCurRelColor()
{
    ColorValues values = ColorValues(
        static_cast<float>(colorDetection.getRed()),
        static_cast<float>(colorDetection.getGreen()),
        static_cast<float>(colorDetection.getBlue()),
        static_cast<float>(colorDetection.getWhite()));
    values.toRelative();
    return values;
}

void LineDetection::proceed() {
    ColorValues cur_color = getCurRelColor();

    float distance_left = cur_color.distanceToMean(left.bound.expectation, left.bound.standard_deviation);
    float distance_right = cur_color.distanceToMean(right.bound.expectation, right.bound.standard_deviation);
    float distance_middle = cur_color.distanceToMean(middle.bound.expectation, middle.bound.standard_deviation);
    float distance_indicator = cur_color.distanceToMean(indicator.bound.expectation, indicator.bound.standard_deviation);
    float distance_crossing = cur_color.distanceToMean(crossing.bound.expectation, crossing.bound.standard_deviation);
    float distance_background = cur_color.distanceToMean(background.bound.expectation, background.bound.standard_deviation);

    /*dezibot.display.println("left: " + String(distance_left));
    dezibot.display.println("right: " + String(distance_right));
    dezibot.display.println("middle: " + String(distance_middle));
    dezibot.display.println("cross: " + String(distance_crossing));
    dezibot.display.println("back: " + String(distance_background));*/

    std::vector<float> distances = {distance_left, distance_right, distance_middle, distance_indicator, distance_crossing, distance_background};
    float min_distance = *std::min_element(distances.begin(), distances.end());

    if(min_distance == distance_left) {
        linePart = LEFT;
    } else if(min_distance == distance_right) {
        linePart = RIGHT;
    } else if(min_distance == distance_middle) {
        linePart = MIDDLE;
    } else if(min_distance == distance_indicator) {
        linePart = INDICATOR;
    } else if(min_distance == distance_crossing) {
        linePart = CROSSING;
    } else if(min_distance == distance_background) {
        linePart = BACKGROUND;
    }
}