#include "Dezibot.h"
#include "veml6040.h"

// Global calibration variables
uint16_t R_white, G_white, B_white, W_white;
uint16_t R_black, G_black, B_black, W_black;

// Function prototypes
uint16_t normalize(uint16_t rawValue, uint16_t blackRef, uint16_t whiteRef);
String detectSurface(int normRed, int normGreen, int normBlue);

Dezibot dezibot = Dezibot();

void setup() {
    // Initialize Dezibot
    dezibot.begin();
    VEML6040 colorDetectionNew;
    colorDetectionNew.begin();

    // Turn on white LED for calibration visibility
    dezibot.multiColorLight.setLed(BOTTOM, dezibot.multiColorLight.color(255, 255, 255));

    dezibot.display.print("Put on the jacket");
    delay(6000);
    dezibot.display.clear();

    // Prompt for white calibration
    dezibot.display.print("Get ready for white");
    delay(5000);
    dezibot.display.clear();

    dezibot.display.print("Place sensor over a white surface...");
    delay(8000); // Wait for placement
    R_white = dezibot.colorDetection.getColorValue(VEML_RED);
    G_white = dezibot.colorDetection.getColorValue(VEML_GREEN);
    B_white = dezibot.colorDetection.getColorValue(VEML_BLUE);
    W_white = dezibot.colorDetection.getColorValue(VEML_WHITE);
    dezibot.display.clear();

    // Prompt for black calibration
    dezibot.display.print("Get ready for black");
    delay(5000);
    dezibot.display.clear();

    dezibot.display.print("Place sensor over a black surface...");
    delay(8000); // Wait for placement
    R_black = dezibot.colorDetection.getColorValue(VEML_RED);
    G_black = dezibot.colorDetection.getColorValue(VEML_GREEN);
    B_black = dezibot.colorDetection.getColorValue(VEML_BLUE);
    W_black = dezibot.colorDetection.getColorValue(VEML_WHITE);
    dezibot.display.clear();

    // Calibration complete
    dezibot.display.print("Calibration complete!");
    delay(1000);
    dezibot.display.clear();
}

void loop() {
    // Read raw sensor values
    int red = dezibot.colorDetection.getColorValue(VEML_RED);
    int green = dezibot.colorDetection.getColorValue(VEML_GREEN);
    int blue = dezibot.colorDetection.getColorValue(VEML_BLUE);
    int white = dezibot.colorDetection.getColorValue(VEML_WHITE);

    // Normalize sensor values using calibration data
    int normRed = normalize(red, R_black, R_white);
    int normGreen = normalize(green, G_black, G_white);
    int normBlue = normalize(blue, B_black, B_white);
    int normWhite = normalize(white, W_black, W_white);

    // Detect surface type
    String surface = detectSurface(normRed, normGreen, normBlue);

    // Display normalized values and detected surface
    dezibot.display.print("R: ");
    dezibot.display.println(String(normRed));
    dezibot.display.print("G: ");
    dezibot.display.println(String(normGreen));
    dezibot.display.print("B: ");
    dezibot.display.println(String(normBlue));
    dezibot.display.print("Surface: ");
    dezibot.display.println(surface);

    delay(2000); // Adjust delay as needed
    dezibot.display.clear();
}

// Normalization function
uint16_t normalize(uint16_t rawValue, uint16_t blackRef, uint16_t whiteRef) {
    if (whiteRef == blackRef) return rawValue; // Avoid division by zero
    return (uint16_t)((rawValue - blackRef) * 65535 / (whiteRef - blackRef)); // Normalize to 0–65535
}

// Detect surface type based on normalized values
String detectSurface(int normRed, int normGreen, int normBlue) {
    // Thresholds for white and black detection
    const int thresholdWhite = 0.9 * 65535; // 90% of the normalized range
    const int thresholdBlack = 0.1 * 65535; // 10% of the normalized range

    if (normRed > thresholdWhite && normGreen > thresholdWhite && normBlue > thresholdWhite) {
        return "White Surface Detected";
    } else if (normRed < thresholdBlack && normGreen < thresholdBlack && normBlue < thresholdBlack) {
        return "Black Surface Detected";
    } else {
        return "Intermediate Surface Detected";
    }
}