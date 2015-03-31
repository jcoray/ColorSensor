
  //  ColorSensor.h - Library for reading color.
  //  Created by Jakob Coray, Feb 24, 2015.
  
#ifndef ColorSensor_h
#define ColorSensor_h

#include "Arduino.h"

struct ColorValues {
    int colors[8];
};

struct Colors {
    byte colors[8];
};


class ColorSensor {
  public:
    ColorSensor(byte redPin, byte greenPin, byte bluePin, byte sensorPin);
    ColorSensor(byte redPin, byte greenPin, byte bluePin, byte sensorPins[8]);
    void whiteBalanceArray();
    void calibrateArray();
    void calibrate();
    byte read();
    Colors readArray();
    void print(byte color);
    void printArray(Colors colors);
    void printRGB();

  private:
    int _balanceCoef = 0;
    int _balanceCoefs[8][3];
    bool _balanced = false;
    byte _sensorPin;
    byte _sensorPins[8];

    char *_colorNames[6] = {"red", "blue", "orange", "white", "yellow", "green",};
    char *_colorNamesAbbr[6] = {"R", "B", "O", "W", "Y", "G",};

    int _calibratedColor[6][3];

    int _findValue(byte color);
    ColorValues _findValueArray(byte RGB);

    byte _redPin;
    byte _greenPin;
    byte _bluePin;
  
    byte _errorPin;
    byte _selectPin;
    bool _array;
};

#endif
