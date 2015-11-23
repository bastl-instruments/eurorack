
#include "analogSeqHW_settings.h"

#ifndef ANALOGSEQHW_H_
#define ANALOGSEQHW_H_

//#include <IHWLayer.h>
//#include <Arduino.h>
#include <avr/pgmspace.h>

#define BLACK_BITS 0
#define RED_BITS 1
#define GREEN_BITS 2
#define BLUE_BITS 4
#define YELLOW_BITS 3
#define MAGENTA_BITS 5
#define CIAN_BITS 6
#define WHITE_BITS 7


#define NUMBER_OF_COLORS 8


#define BLACK 0
#define RED 1
#define GREEN 2
#define BLUE 3
#define YELLOW 6
#define MAGENTA 5
#define CIAN 4
#define WHITE 7



class analogSeqHW { //: public IHWLayer


public:
	enum TriggerState{ON,OFF};


	// sets up all the pins, timers and SPI interface
	// call this before using any other method from this class
	void init(void(*buttonChangeCallback)(uint8_t number), void(*clockInCallback)(uint8_t number));

	/***KNOBS***/

	//
	virtual uint16_t getKnobValue(uint8_t index);
	uint8_t getLastKnobValue(uint8_t index){return lastKnobValues[index];}
	bool knobMoved(uint8_t index);
	bool knobFreezed(uint8_t index);
	void freezeAllKnobs();
	void freezeKnob(uint8_t index);
	void freezeKnob(uint8_t index, uint8_t value);
	void unfreezeKnob(uint8_t index);



	/***LEDS***/

	// disabled
	virtual void setLED(uint8_t number, bool state);

	// disabled
	virtual void setLED(uint8_t number, uint8_t number2, uint8_t number3 = 0) {}

	// print the state arrays to the Serial terminal
	void printLEDStates();

	// set color of RGB led
	void setColor(uint8_t _COLOR);

	void allLedsOff();


	/***BUTTONS***/

	// the the state of a button identified by its id
	//virtual IHWLayer::ButtonState getButtonState(uint8_t number);
//	virtual IHWLayer::ButtonState
	bool getButtonState(uint8_t number);

	// print the read button states to serial terminal
	void printButtonStates();
	/***TRIGGER***/
	void setTrigger(uint8_t number, TriggerState state, uint16_t pulseWidth=0);

	bool getTriggerState(uint8_t number);




	/***RAM***/

	// write a byte to the given address
	virtual void writeSRAM(long address, uint8_t data){}

	// write a number of bytes starting at the given address
	virtual void writeSRAM(long address, uint8_t* buf, uint16_t len){}

	// read the byte stored at the given address
	virtual uint8_t readSRAM(long address){return 0;}

	// read a number of bytes starting from the given address
	virtual void readSRAM(long address, uint8_t* buf, uint16_t len){}




	/**TIMING**/

	// the number of bastl cycles elapsed since startup
	// this number will overflow after some minutes; you have to deal with that in the layer above
	// using a longer datatype would prevent this but at the cost of longer computation time
	uint32_t getElapsedBastlCycles();

	// returns the relation between bastl cycles and seconds
	// this value is dependent on the hardware update frequency that you can set by a define
	// use this to map real time processes (like BMP) to bastlCycles
	uint16_t  getBastlCyclesPerSecond();




	// only called by ISR routine.
	// they would be declared private but this would prevent the ISR from accessing them
	// there are workarounds for this but as they come at a cost I just left it like this

	void isr_updateButtons();
	void isr_updateKnobs();
	void isr_updateTriggerStates();
	void isr_updateClockIn();
	void resetTriggers();

	inline void incrementBastlCycles() {bastlCycles++;}

	/**EEPROM**/

	virtual void readEEPROM(uint8_t pageNumb, uint8_t* byteArray, uint16_t numbBytes){}
	virtual void readEEPROMDirect(uint16_t address, uint8_t* byteArray, uint16_t numbBytes){}
	virtual bool isEEPROMBusy(){return true;}
	void setBit(uint8_t _bit, bool _value);
	void setGate(uint8_t _gate, bool _state);
	void setLed(uint8_t _led, bool _state);
	void dimLed(uint8_t _led, bool _state);
	void setBiLed(uint8_t _led,bool _state, bool _color);
	void setGateOut(bool _state);
	void setShiftLed(bool _state);
	void dimBiLed(uint8_t _led, bool _state);
	void dimShiftLed(bool _state){shiftLedDim=_state;};
	void selectKnob(uint8_t _knob);
	uint8_t shiftHash[4];
	bool justPressed(uint8_t _but);
	bool justReleased(uint8_t _but);
	bool buttonState(uint8_t _but);
	bool jumpState(){return _jumpState;};
	bool lastJumpState(){return _lastJumpState;};
	uint16_t getCV(uint8_t _number);
	uint16_t getLastCV(uint8_t _number);
	uint16_t getPotA();
	uint16_t getPotB();
	uint16_t getLastPotA();
	uint16_t getLastPotB();
	bool getMinMajState();

	bool trigAState(){return trigStateA;};
	bool trigBState(){return trigStateB;};
	bool resetState(){return rstState;};
	bool _jumpState;
	bool _lastJumpState;

private:
	/**TIMING**/

	uint32_t bastlCycles;
	bool inBetween(uint8_t value, uint8_t border1, uint8_t border2);


	uint8_t buttonSelect;
	/**BUTTONS**/
	uint8_t knobFreezeHash;
	uint8_t knobMovedHash;
	bool shiftLedDim;
	uint16_t buttonHash;
	uint8_t justPressedHash;
	uint8_t justReleasedHash;

	uint8_t ledDimHash;
	uint8_t biLedDimHash;
	uint8_t ledOutHash;
	uint8_t ledDimCount;

	uint8_t selectedKnob;
	bool newButtonStates[2];
	bool buttonStates[2];
	bool trigStateA;
	bool trigStateB;
	bool rstState;
	bool shiftState;
	bool minMajGateState;
	uint16_t knobValues[8];
	uint16_t mixedValues[9];
	uint16_t lastMixedValues[9];

	uint16_t lastKnobValues[6];
	uint8_t knobFreezeValues[6];
	uint8_t knobCount;
	void compareButtonStates();
	void (*buttonChangeCallback)(uint8_t number);
	void (*clockInCallback)(uint8_t number);

	/**TRIGGERS**/
	uint8_t trigState;
	uint16_t triggerCountdown[8];





};




#endif
