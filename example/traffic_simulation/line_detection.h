#include <Dezibot.h>
#include <veml6040.h>
#include <vector>


enum LinePart {
    LEFT,
    RIGHT,
    MIDDLE,
    INDICATOR,
    CROSSING,
    BACKGROUND
};

class LineDetection {
public:
    LineDetection(Dezibot &dezibot) : dezibot(dezibot) {}

    void begin();

    void proceed();

    LinePart getLinePart() { return linePart; }

private:
    struct ColorValues {
        float red, green, blue, white;

        ColorValues() = default;
        ColorValues(float r, float g, float b, float w) : red(r), green(g), blue(b), white(w) {}

        bool inBound(const ColorValues &expectation, const ColorValues &std_dev);

        bool valueInBound(const float value, const float expected_value, const float tolerance);

        void toRelative() {
            red /= white;
            green /= white;
            blue /= white;
        }

        float distanceToMean(const ColorValues &expectation, const ColorValues &std_dev);
    };

    struct ColorBound {
    public:
        ColorValues expectation;
        ColorValues standard_deviation;

        ColorBound() = default;

        void calculateBounds(const std::vector<ColorValues> &values) {
            calculateExpectations(values);
            calculateStandardDeviations(values);
        }
    private:
        void calculateExpectations(const std::vector<ColorValues> &values);
        
        void calculateStandardDeviations(const std::vector<ColorValues> &values);
    };

    struct ColorData {
        String name;
        std::vector<ColorValues> color_values_list;
        ColorBound bound;

        ColorData(String name) : name(name) {}

        void calculate_statistical_data() {
            bound.calculateBounds(color_values_list);
        }
    };


    Dezibot &dezibot;
    VEML6040 colorDetection;
    LinePart linePart;

    int amountCalibrationValues = 35;

    ColorData left = ColorData("left");
    ColorData right = ColorData("right");
    ColorData middle = ColorData("middle");
    ColorData indicator = ColorData("indicator");
    ColorData crossing = ColorData("crossing");
    ColorData background = ColorData("background");

    std::vector<ColorData *> data = {&left, &right, &middle, /*&indicator,*/ &crossing, &background};

    void calibrateColors();
    void collectColorData();


    ColorValues getCurRelColor();
};