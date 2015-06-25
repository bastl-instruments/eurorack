
#include "midiSeqHW_settings.h"

#ifndef MIDISEQHW_H_
#define MIDISEQHW_H_

//#include <IHWLayer.h>
//#include <Arduino.h>
#include <avr/pgmspace.h>



class midiSeqHW { //: public IHWLayer


public:


	// sets up all the pins, timers and SPI interface
	// call this before using any other method from this class
	void init(void(*buttonChangeCallback)(uint8_t number), void(*clockInCallback)(uint8_t number));



	bool buttonState(uint8_t _but);


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

	void setLed(uint8_t _led, bool _state);
	void setGateOut(uint8_t _number, bool _state);
	void selectKnob(uint8_t _knob);

	void setClkOut(bool _state);
	void setHorLed(uint8_t _number, bool _state);
	void setVerLed(uint8_t _number, bool _state);
	void dimLed(uint8_t _led, bool _state);
	void dimHorLed(uint8_t _led, bool _state);
	void dimVerLed(uint8_t _led, bool _state);

private:
	/**TIMING**/

	uint32_t bastlCycles;
	uint16_t buttonHash;
	uint16_t lastButtonHash;
	uint8_t shiftHash[3];
	uint8_t dimHash[3];

	/**BUTTONS**/

	void (*buttonChangeCallback)(uint8_t number);
	void (*clockInCallback)(uint8_t number);

	/**TRIGGERS**/
	uint8_t trigState;
	uint16_t triggerCountdown[8];





};




#endif
