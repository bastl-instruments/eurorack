
#include "trinityRackCV_HW_settings.h"

#ifndef TRINITYRACKCV_HW_H_
#define TRINITYRACKCV_HW_H_

#include <IHWLayer.h>
#include <Arduino.h>
#include <avr/pgmspace.h>


class trinityRackCV_HW : public IHWLayer {


public:
	// sets up all the pins, timers and SPI interface
	// call this before using any other method from this class
	void init(void(*clockInCallback)());

	/*** CV in***/
	uint8_t getCVValue(uint8_t index);
	uint8_t getLastCVValue(uint8_t index){return lastCVValues[index];}
	bool CVMoved(uint8_t index);

	/*** CV out ***/
	void setDAC(uint8_t number, uint8_t value);
	bool getClockState();

	/**TIMING**/

	// the number of bastl cycles elapsed since startup
	// this number will overflow after some minutes; you have to deal with that in the layer above
	// using a longer datatype would prevent this but at the cost of longer computation time
	uint32_t getElapsedBastlCyclesLong() {return bastlCycles;}
	uint16_t  getElapsedBastlCycles() {return bastlCycles>>16;}

	// returns the relation between bastl cycles and seconds
	// this value is dependent on the hardware update frequency that you can set by a define
	// use this to map real time processes (like BMP) to bastlCycles
	uint16_t  getBastlCyclesPerSecond();


	// Buttons //
	virtual ButtonState getButtonState(unsigned char index) {return UP;}

	// LEDs //
	virtual void setLED(unsigned char index, LedState state) {}
	//virtual void setLED(unsigned char index, unsigned char brightness, unsigned char blinkTime = 0) = 0;

	// Knobs //
	virtual unsigned char getKnobValue(unsigned char index) {return 0;}
	virtual void freezeKnob(uint8_t number, uint8_t value) {};
	virtual void setKnobLED(uint8_t number, uint8_t value) {};
	virtual void setKnobLEDTracking(uint8_t number, bool) {};


	// RAM //
	virtual unsigned char readSRAM(long address) {return 0;}
	virtual void readSRAM(long address, unsigned char * data, unsigned int size) {};
	virtual void writeSRAM(long address, unsigned char data) {};
	virtual void writeSRAM(long address, unsigned char * data, unsigned int size) {};

	// EEPROM //

    virtual bool writeEEPROM(uint16_t address, uint8_t* byteArray, uint16_t numbBytes) {return false;}
    virtual void readEEPROM(uint16_t address, uint8_t* byteArray, uint16_t numbBytes) {}

    // general operation
    virtual void giveSomeTime() {};
    virtual void giveAllTime() {};

	// only called by ISR routine.
	// they would be declared private but this would prevent the ISR from accessing them
	// there are workarounds for this but as they come at a cost I just left it like this
	void isr_updateADC();
	void isr_updateDAC();
	void isr_updateClockIn();

	inline void incrementBastlCycles() {bastlCycles++;}




private:
	/**TIMING**/
	uint32_t bastlCycles;

	/** CLOCK IN **/
	bool clockInState;
	void (*clockInCallback)();

	/** CV out */
	static const uint8_t numbDACs = 8;
	uint8_t dacCount;
	uint8_t dacValues[numbDACs];
	uint8_t dacValuesSent[numbDACs];
	void DACInit();
	void zeroDACs();

	/** CV in */
	static const uint8_t numbCVs = 6;
	uint8_t CVValues[numbCVs];
	uint8_t lastCVValues[numbCVs];
	uint8_t CVCount;
	uint8_t CVMovedHash;


};




#endif
