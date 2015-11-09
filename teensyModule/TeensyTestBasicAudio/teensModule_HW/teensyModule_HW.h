
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

#define MUX_A 22
#define MUX_B 21
#define MUX_C 20

#define DEBUG 1
#define USE_CALLBACKS 1 

#define NUMBER_OF_KNOBS 4
#define NUMBER_OF_CVIN 5
#define NUMBER_OF_BUTTONS 2
#define NUMBER_OF_SWITCHES 1
#define NUMBER_OF_LEDS 5
#define NUMBER_OF_CVOUT 1

// pin definitions //

#define KNOB_A_PIN 5
#define KNOB_B_PIN 4
#define KNOB_C_PIN 6
#define KNOB_D_PIN 7

#define CV_A_PIN 0
#define CV_B_PIN 1
#define CV_C_PIN 2
#define CV_D_PIN 3

#define MUX_PIN A2
#define CV_PIN A5
#define SQUARE_OUT_PIN 2


#define CVOUT_PIN A14

#define BUTTON_PIN 12
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
		void initializeMozzi();
		void update();
		void mozziUpdate();
		
		void setLed(uint8_t _LED, boolean _STATE);
		
		boolean knobFreezed(uint8_t _KNOB);
		boolean knobMoved(uint8_t _KNOB);
		void freezeAllKnobs();
		void unfreezeAllKnobs();
		void freezeKnob(uint8_t _KNOB);
		void unfreezeKnob(uint8_t _KNOB);
		uint16_t knobValue(uint8_t _KNOB);
		uint16_t lastKnobValue(uint8_t _KNOB);
		void setFreezeType(uint8_t _TYPE);
		void setKnobTolerance(uint16_t _tolerance);
		
		uint16_t CVValue(uint8_t _CV);
		boolean CVMoved(uint8_t _CV);
		
		boolean buttonState(uint8_t _BUTTON);
		boolean justPressed(uint8_t _BUTTON);
		boolean justReleased(uint8_t _BUTTON);
		boolean buttonChanged(uint8_t _BUTTON);
		
		void flipSwitch(uint8_t _SWITCH);
		void setSwitch(uint8_t _SWITCH, boolean _STATE);
		boolean switchState(uint8_t _SWITCH);
		void resetSwitches();
		
		// callback functions
		#if USE_CALLBACKS
		void setHandleButtonChange(void (*fptr)(uint8_t button, boolean state));
		void setHandleKnobChange(void (*fptr)(uint8_t knob, uint16_t value));
		void setHandleCVChange(void (*fptr)(uint8_t cv, uint16_t value));
		#endif // USE_CALLBACKS
		
	private:
		void  setMux(uint8_t bits);
		//uint16_t KNOB_TOLERANCE;
		//uint16_t activity;
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
		uint8_t switchStateHash;
		uint8_t justPressedHash; // change
		uint8_t justReleasedHash;
		unsigned long buttonBounceTime[NUMBER_OF_BUTTONS]; //
		void updateButtons();
		void updateMozziButtons();
		boolean mozziDigitalRead(uint8_t _pin);

		boolean mozzi;
		boolean unfreezeExternaly;
		
		// call back pointers
		#if USE_CALLBACKS
		void (*mButtonChangedCallback)(uint8_t button, boolean state);
		void (*mKnobChangedCallback)(uint8_t knob, uint16_t value);
		void (*mCVChangedCallback)(uint8_t cv, uint16_t value);
		#endif // USE_CALLBACKS
		
};
#endif //teensyHWR_h