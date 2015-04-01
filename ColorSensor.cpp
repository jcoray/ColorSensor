//  ColorSensor.cpp - Library for reading color.
//  Created by Jakob Coray, Feb 24, 2015.

#include "Arduino.h"
#include "ColorSensor.h"

ColorSensor::ColorSensor(byte redPin, byte greenPin, byte bluePin, byte sensorPin, byte errorPin, byte selectPin)
{		//  Constructor for a single sensor   
    pinMode(redPin, OUTPUT);
    pinMode(greenPin, OUTPUT);
    pinMode(bluePin, OUTPUT);
    pinMode(sensorPin, INPUT);
    _redPin = redPin;
    _greenPin = greenPin;
    _bluePin = bluePin;
    _sensorPin = sensorPin;
    //  TODO Write code to use these to inputs.
    _errorPin = errorPin;
    _selectPin = selectPin;

    _array = false;
}

ColorSensor::ColorSensor(byte redPin, byte greenPin, byte bluePin, byte sensorPins[8], byte errorPin, byte selectPin)
{   //  Constructor for an array of eight sensors
    pinMode(redPin, OUTPUT);
    pinMode(greenPin, OUTPUT);
    pinMode(bluePin, OUTPUT);

    for(byte sensor = 0; sensor < 8; sensor++) {
    	//  set up and declare sensor array
    	pinMode(sensorPins[sensor], INPUT);
    	_sensorPins[sensor] = sensorPins[sensor];
    }
    
    _redPin = redPin;
    _greenPin = greenPin;
    _bluePin = bluePin;
    _errorPin = errorPin;
    _selectPin = selectPin; //  TODO write code to select with the button.
    _array = true;
}

void ColorSensor::whiteBalanceArray() {
	//  The photocells have a very high tolerance and need to be compared with a 
	//  nomminal value. Placing a white card an equal distance from every sensor
	//  in the array does the trick.
	Serial.println("Place a white card under the sensor then send a zero");
	while(1) {
    	if(Serial.available()) {
    		if(Serial.read() == 48) break;
    	}
   	}
   	for(byte RGB = 0; RGB < 3; RGB++) {
		ColorValues sensorValue = ColorSensor::_findValueArray(RGB);
    	for(byte sensor = 0; sensor < 8; sensor++) {
    		_balanceCoefs[sensor][RGB] = sensorValue.colors[sensor];
    		Serial.println(_balanceCoefs[sensor][RGB]);
    	}
    }
    _balanced = true;
    Serial.println("White Balance Complete.");
    Serial.println();
}

void ColorSensor::calibrate() {
	//  Asks the user to place a color under the sensor, scans it, and stores
	//  those values in _calibratedColors[color][RGB] to compare to when reading
	//  the color.
    for(byte color = 0; color < 6; color++) {
    	//  loop iterates once per color
    	Serial.print("Place a ");
    	Serial.print(_colorNames[color]);
    	Serial.println(" block under sensor and then send a zero");
		while(1) {
    		if(Serial.available()) {
    			if(Serial.read() == 48) break;
    		}
   		}
		Serial.print("block color: ");
		ColorSensor::print(ColorSensor::read());
		ColorSensor::printRGB(); //  show them the RGB once for debuging
    	for(byte RGB = 0; RGB < 3; RGB++) {
    		//  Read and store the RGB values of the block under the sensor as
    		//  the calibration value for that color to use as  a reference.
    		_calibratedColor[color][RGB] = ColorSensor::_findValue(RGB);
    	}
	}
	Serial.println("Calibration Complete");
}

void ColorSensor::calibrateArray() {
	//  Use one sensor as a relative value to compare the whole array to. 
	if(!_balanced) {
		Serial.println("ERROR: white balance before calibrating.");
		return;
	}

	ColorSensor calibrationSensor(_redPin, _greenPin, _bluePin, _sensorPins[0]);
    for(byte color = 0; color < 6; color++) {
    	//  loop iterates once per color
    	Serial.print("Place a ");
    	Serial.print(_colorNames[color]);
    	Serial.println(" block under sensor 0 and then send a zero.");
    	bool FSM = true; 
		  while(FSM) {
    		if(Serial.available()) {
    			if(Serial.read() == 48) break;
    		}
   		}

//		calibrationSensor.printRGB(); //  show them the RGB once for debuging
    	for(byte RGB = 0; RGB < 3; RGB++) {
    		//  Read and store the RGB values of the block under the sensor as
    		//  the calibration value for that color to use as  a reference.
    		//  Then white balance it so it works with other sensors.
    		_calibratedColor[color][RGB] = calibrationSensor._findValue(RGB) - _balanceCoefs[0][RGB];
    	}
	}
	Serial.println("Calibration Complete");
}

byte ColorSensor::read() {
	//  Return the color. 
	//  0:RED 1:GREEN 2:BLUE 3:WHITE 4:ORANGE 5:YELLOW
	if(_array) {
		Serial.println("ERROR 200: This function is only to be used a single sensor");
		return 200;
	}

	int blockRGB[3] = {0,0,0};
    for(byte RGB = 0; RGB < 3; RGB++) {
    	//  Scan the block in all three colors.
    	int sensorValue = ColorSensor::_findValue(RGB);
    	if(_balanced) blockRGB[RGB] = sensorValue - _balanceCoef;
		else blockRGB[RGB] = blockRGB[RGB] = sensorValue;
    }
    int errorMargin = 150; //TODO adjust this value
    for(byte color = 0; color < 6; color++) {
    	//  See if the color is red, if it's not, try green...
    	for(byte RGB = 0; RGB < 3; RGB++) {
    		//  Compare the block under red light to a RGB calibration value. If
    		//  the value is out of the margin of error, test the block's RGB to
    		//  the next calibration value.
    		Serial.print("Calibration value: ");
    		Serial.println(_calibratedColor[color][RGB]);
    		if(blockRGB[RGB] > _calibratedColor[color][RGB] + errorMargin || \
    		   blockRGB[RGB] < _calibratedColor[color][RGB] - errorMargin) {
    			break;
    		}
    		else if(RGB == 2) {
    			//  If it passes under red, green, & blue light return the color.
    			return color;
    		}
    	}
    }
    Serial.println("ERROR 100: Could not identify color.");
    return 100; //  Return 100 if it cannot identify the color.
}

//     ####################################%%%%#%%%#%###%#
//     ####################################%####%###%%#%%#
//     ############## readArray lies below %%%##%%%#%#%#%#
//     ####################################%######%#%###%#
//     ####################################%####%%%#%###%#
//     ### (these lines are here because I am tired of ###
//     #### editing read(), rather than readArray())  ####
//     ###################################################
//     ###################################################

Colors ColorSensor::readArray() {
	//  Return the color. 0:RED 1:GREEN 2:BLUE 3:WHITE 4:ORANGE 5:YELLOW
  if(!_array) {
    Colors error;
    Serial.println("ERROR: This function is only to be used an array of sensors.");
		return error;
	}
	if(!_balanced) {
	  Colors error;
		Serial.println("ERROR: This function will return garbage if the sensor is not white balanced.");
		return error;
	}

  ColorValues faceRGB[3] = {{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0}};
	for(byte RGB = 0; RGB < 3; RGB++) {
		//  Scan the block in all three colors.
		ColorValues sensorValue = ColorSensor::_findValueArray(RGB);
		for(byte sensor = 0; sensor < 8; sensor++) {
			/*
			Serial.print(sensorValue.colors[sensor]);
			Serial.print(" sensorValue.colors[sensor] ");
			Serial.println(sensor);
			*/
			faceRGB[RGB].colors[sensor] = sensorValue.colors[sensor] - _balanceCoefs[sensor][RGB];
			/*
			Serial.print(faceRGB[RGB].colors[sensor]);
			Serial.print(":");
			Serial.print(_calibratedColor[sensor][RGB] );
			Serial.println(" faceRGB[RGB].colors[sensor] after - balance");
			*/
		}
	}
	
	Colors faceColor;
	for(byte sensor = 0; sensor < 8; sensor++) {
		int errorValue = analogRead(_errorPin);
		int errorMargin = map(errorValue,0,1023,20,400); //  These can be adjusted.
		Serial.print("\nError value: "); 
		Serial.print(errorValue);
		Serial.print("    Error margin: "); 
		Serial.println(errorMargin);
		
		//  default to error margin so matches will be less than it when checking difference
		int possibleMatches[6] = {errorMargin,errorMargin,errorMargin,errorMargin,errorMargin,errorMargin};
		for(byte color = 0; color < 6; color++) {
			//  Test if the block fit within a certain margin of the white balanced 
			//  calibration values for a color. If it doesn't, test the nest color.
			//  If it does, add it to the list of possibilities. Then try to fit it
			//  to the next color.
			for(byte RGB = 0; RGB < 3; RGB++) {
				//  Compare the block under red light to the red calibration value of
				//  the color being tested. If the value is out of the margin of error, 
				//  quit testing the current color, and skip to the next one. Else, 
				//  continue testing with the green and then the blue values. If the
				//  the block is within the margin of error (if it is a match) add it
				//  to the list of possible matches.
				if( faceRGB[RGB].colors[sensor] > _calibratedColor[color][RGB] + errorMargin || faceRGB[RGB].colors[sensor] < _calibratedColor[color][RGB] - errorMargin) {
					break;
				}
				else if(RGB == 2) {
				//  If it passes under red, green, & blue light add it to the list of 
				//  possible matches.
					int totalDiff = 0;
					for(byte matchRGB = 0; matchRGB < 3; matchRGB++) {
						totalDiff += faceRGB[matchRGB].colors[sensor] - _calibratedColor[color][matchRGB];
					}
					totalDiff /= 3;
					possibleMatches[color] = totalDiff;
				}
			}
		}
		
		int bestMatch = errorMargin;
		byte bestColor = 0;
		for(byte ccolor = 0; ccolor < 6; ccolor++) {
		  int diff = abs(possibleMatches[ccolor]);
		  Serial.println("Difference b/w colors: ");
		  Serial.println(diff);
			if(diff < errorMargin) {
				bestMatch = possibleMatches[ccolor];
				bestColor = ccolor;
				Serial.print("Possible color match: ");
				Serial.println(bestColor);
			} 
		}
		Serial.println("");
		if(bestMatch != errorMargin) {
			faceColor.colors[sensor] = bestColor;
			Serial.print("identified color as: ");
			Serial.println(bestColor);
		}
		else { //  This should only run when something goes wrong.
		  Serial.println("ERROR 150: readArray() failed to identify the color");
		  faceColor.colors[sensor] = 150;
		}
	}
	return faceColor;
}

ColorValues ColorSensor::_findValueArray(byte RGB) { 
	//  RED = 0; GREEN = 1; BLUE = 2;
  switch(RGB) {
  case 0: //  red
      digitalWrite(_bluePin, LOW);
      digitalWrite(_greenPin, LOW);
      digitalWrite(_redPin, HIGH);
      break;
	case 1: //  green
      digitalWrite(_redPin, LOW);
      digitalWrite(_bluePin, LOW);
      digitalWrite(_greenPin, HIGH);
      break;
  case 2: //  blue
      digitalWrite(_greenPin, LOW);
      digitalWrite(_redPin, LOW);
      digitalWrite(_bluePin, HIGH);
      break;
  }
  //  The photo resistor datasheet says it takes 100 millis to update. It lies. 
  //  It takes 100 millis to get 80% of the way there. I tested it with an
  //  oscilloscope, and it takes about 250 millis to get 98% of the way there. 
  //  I chose 300 to be on the safe side. A 100 or 200 millis delay might 
  //  work, but there is no reason (as of yet) to risk it not working.
  int lightDelay = 300;
  delay(lightDelay);
  ColorValues sensorValue = {0,0,0,0,0,0,0,0};
	for(byte averages = 0; averages < 5; averages++) {
		for(byte sensor = 0; sensor < 8; sensor++) {
			sensorValue.colors[sensor] += analogRead(_sensorPins[sensor]);
			//  I have not tried it, but I think that testing other values would be 
			//  futile. But, not taking an average of the readings is not as reliable.
			delay(1);  
		}
	}
	for(byte sensor = 0; sensor < 8; sensor++) {
			sensorValue.colors[sensor] /= 5;
	}
  digitalWrite(_bluePin, LOW);
  digitalWrite(_greenPin, LOW);
  digitalWrite(_redPin, LOW);
  return sensorValue;
}

int ColorSensor::_findValue(byte RGB) { //  RED = 0; GREEN = 1; BLUE = 2;
  switch(RGB) {
  case 0: //  red
      digitalWrite(_bluePin, LOW);
      digitalWrite(_greenPin, LOW);
      digitalWrite(_redPin, HIGH);
      break;
  case 1: //  green
      digitalWrite(_redPin, LOW);
      digitalWrite(_bluePin, LOW);
      digitalWrite(_greenPin, HIGH);
      break;
  case 2: //  blue
      digitalWrite(_greenPin, LOW);
      digitalWrite(_redPin, LOW);
      digitalWrite(_bluePin, HIGH);
      break;
  default:
      return -1; //  Just in case something goes wrong, we want to be alerted
  }
  //  See related comments in function ColorSensor::_findValueArray(byte RGB)
  int lightDelay = 300;
  delay(lightDelay);
	int sensorValue;
	for(byte i = 0; i < 5; i++) {
		sensorValue += analogRead(_sensorPin);
		delay(1);
	}
	sensorValue /= 5;
  digitalWrite(_bluePin, LOW);
  digitalWrite(_greenPin, LOW);
  digitalWrite(_redPin, LOW);
  return sensorValue;
}

void ColorSensor::print(byte color) {
	if(_array) {
		Serial.println("ERROR: This function is only to be used with a single sensor");
		return;
	}

	for(byte i = 0; i < 6; i++) {
	//  Try to match the block color number to the color name
		if(color == i) {
			Serial.println(_colorNames[color]);
			return;
		}
		else if(i == 5) {
			Serial.println("did not recognize");
		}
	}
}

void ColorSensor::printArray(Colors colors) {
	//  Prints out a pretty diagram of the face.
	if(!_array) {
    Serial.println("ERROR: This function is only to be used an array of sensors.");
		return;
	}
	char *faceColors[8] = {"#", "#", "#", "#", "#", "#", "#", "#",};
	for(byte sensor = 0; sensor < 8; sensor++) {
		Serial.println(colors.colors[sensor]);

		for(byte color = 0; color < 6; color++) {
		//  Try to match the block color number to the color name.
			if(colors.colors[sensor] == color) {
				faceColors[sensor] = _colorNamesAbbr[color];
				break;
			}
			else if(color == 5) {
				Serial.print("ERROR: did not recognize color in spot: ");
				Serial.println(sensor);
			}
		}
	}
	//  It may be better to use to Arduino String library to make one big, long 
	//  string and to use one print statement. However, if it ain't broke; don't
	//  fix it. 
	Serial.println();
 	Serial.print(faceColors[0]); Serial.print(" "); Serial.print(faceColors[1]); Serial.print(" "); Serial.println(faceColors[2]);
  Serial.print(faceColors[7]); Serial.print(" "); Serial.print("#"); Serial.print(" "); Serial.println(faceColors[3]);
  Serial.print(faceColors[6]); Serial.print(" "); Serial.print(faceColors[5]); Serial.print(" "); Serial.println(faceColors[4]);
}

void ColorSensor::printRGB() {
	int redValue, greenValue, blueValue;
  redValue   = ColorSensor::_findValue(0); //  Red
  greenValue = ColorSensor::_findValue(1); //  Green
  blueValue  = ColorSensor::_findValue(2); //  Blue
	Serial.print("  Red = ");
  Serial.println(redValue, DEC);
	Serial.print("Green = ");
  Serial.println(greenValue, DEC);
	Serial.print(" Blue = ");
  Serial.println(blueValue, DEC);
  Serial.println();
}
