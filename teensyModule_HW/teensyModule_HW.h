
/*
teensyModule hardware library
this library is to be used with Bastl Instruments eurorack hardware and Teensy 3.1
MOZZI library compatible
documentation of the library you find here:

Created by Tomash Ghzegowsky 2015 http://tomashg.com/
for Bastl Instruments http://www.bastl-instruments.com/
*/

#ifndef teensyHWR_h
#define teensyHWR_h

#include "Arduino.h"
#include <avr/pgmspace.h>

#define DEBUG 1
#define USE_CALLBACKS 1 

#define NUMBER_OF_KNOBS 4
#define NUMBER_OF_CVIN 5
#define NUMBER_OF_BUTTONS 2
#define NUMBER_OF_SWITCHES 1
#define NUMBER_OF_LEDS 5
#define NUMBER_OF_CVOUT 1

// pin definitions //

#define CV_PIN_A 0
#define CV_PIN_B 1
#define CV_PIN_C 2
#define CV_PIN_D 3

#define KNOB_PIN_A 5 
#define KNOB_PIN_B 4
#define KNOB_PIN_C 6
#define KNOB_PIN_D 7

#define MUX_PIN A2

#define MUX_PIN_A 22
#define MUX_PIN_B 21
#define MUX_PIN_C 20


#define CV_PIN_E A5

#define SQUARE_OUT_PIN 2
#define CV_OUT_PIN A14

#define BUTTON_A_PIN 12
#define SWITCH_PIN_A 11
#define SWITCH_PIN_B 14

#define LED_A_PIN 10
#define LED_B_PIN 9
#define LED_C_PIN 8
#define LED_D_PIN 7
#define LED_E_PIN 6

#define ZERO 0
#define DEFAULT 0
#define UNFREEZE_EXTERNALY 1

#define ACTIVITY_LIMIT 25
#define KNOB_FREEZE_DISTANCE 32

class teensyHWR {
	public:
		teensyHWR(); // constructor
		void initialize();
		void update();
		
		void setLed(uint8_t _LED, boolean _STATE);
		
		boolean knobFreezed(uint8_t _KNOB);
		boolean knobMoved(uint8_t _KNOB);
		
		void freezeAllKnobs();
		void unfreezeAllKnobs();
		void freezeKnob(uint8_t _KNOB);
		void unfreezeKnob(uint8_t _KNOB);
		
		
		uint16_t knobValue(uint8_t _KNOB);
		uint16_t lastKnobValue(uint8_t _KNOB);
		
		void setKnobTolerance(uint16_t _tolerance);
		
		uint16_t cvValue(uint8_t _CV);
		boolean cvMoved(uint8_t _CV);
		bool isConnected(uint8_t _CV);
		
		boolean buttonState(uint8_t _BUTTON);
		boolean justPressed(uint8_t _BUTTON);
		boolean justReleased(uint8_t _BUTTON);
		boolean buttonChanged(uint8_t _BUTTON);
		void setSquareOut(bool highLow);
		
		uint8_t switchState();
		uint8_t lastSwitchState();
		bool switchChanged();
		void readCVE(bool yesNo);
		
		
		// callback functions
		#if USE_CALLBACKS
		void setHandleButtonChange(void (*fptr)(uint8_t button, boolean state));
		void setHandleKnobChange(void (*fptr)(uint8_t knob, uint16_t value));
		void setHandleCVChange(void (*fptr)(uint8_t cv, uint16_t value));
		#endif // USE_CALLBACKS
		
	private:
	bool readCV_E;
	uint8_t switchHash;
	uint8_t lastSwitchHash;
		void setMux(uint8_t bits);
		uint16_t KNOB_TOLERANCE;
		
		//leds
		void writeToLeds();
		uint8_t ledStateHash;
		//knobs
		uint8_t knobFreezedHash;
		uint8_t knobChangedHash;
		uint16_t knobValues[NUMBER_OF_KNOBS];
		uint16_t lastKnobValues[NUMBER_OF_KNOBS];
		void updateMozziKnobs();
		void updateKnobs();
		//cv
		uint8_t CVChangedHash;
		uint16_t CVValues[NUMBER_OF_CVIN];
		uint16_t lastCVValues[NUMBER_OF_CVIN];
		void updateMozziCVs();
		void updateCVs();
		//buttons
		uint8_t buttonStateHash;
		uint8_t buttonChangedHash;
		uint8_t lastButtonStateHash;
		uint8_t justPressedHash; // change
		uint8_t justReleasedHash;
		
		void updateButtons();
		void updateMozziButtons();
		
		// call back pointers
		#if USE_CALLBACKS
		void (*mButtonChangedCallback)(uint8_t button, boolean state);
		void (*mKnobChangedCallback)(uint8_t knob, uint16_t value);
		void (*mCVChangedCallback)(uint8_t cv, uint16_t value);
		#endif // USE_CALLBACKS
		
};
#endif //teensyHWR_h