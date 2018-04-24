
#include "midiSeqHW_settings.h"

#ifndef MIDISEQHW_H_
#define MIDISEQHW_H_

//#include <IHWLayer.h>
//#include <Arduino.h>
#include <avr/pgmspace.h>

#define NUMBER_OF_TUNING_POINTS 15

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
	uint32_t getPreciseBastlCycles();

	// returns the relation between bastl cycles and seconds
	// this value is dependent on the hardware update frequency that you can set by a define
	// use this to map real time processes (like BMP) to bastlCycles
	uint16_t  getBastlCyclesPerSecond();




	// only called by ISR routine.
	// they would be declared private but this would prevent the ISR from accessing them
	// there are workarounds for this but as they come at a cost I just left it like this

	void isr_updateButtons();
	void isr_updateKnobs();
	void isr_updateShiftRegisters();
	void isr_updateClockIn();
	void isr_updateDisplay();
	void isr_updateAnalogInputs();
	void resetTriggers();
	void setPCMask(uint8_t _number);
	uint32_t getPeriod();
	inline void incrementBastlCycles() {bastlCycles++;}

	/**EEPROM**/

	virtual void readEEPROM(uint8_t pageNumb, uint8_t* byteArray, uint16_t numbBytes){}
	virtual void readEEPROMDirect(uint16_t address, uint8_t* byteArray, uint16_t numbBytes){}
	virtual bool isEEPROMBusy(){return true;}

//	void setLed(uint8_t _led, bool _state);
	void setGate(uint8_t _number, bool _state);
	void setLed(uint8_t _number, bool _state);
	void displayNumber(int _number);
	void setDot( boolean _state);
	void lightNumber(int numberToDisplay);
	void displayChar(char whichChar);
	void setCS(uint8_t _number, bool _state);
	void setClk(bool _state);
	void setRst(bool _state);
	bool getTuneState(uint8_t _number);
	bool getDetectState();
	bool getUpdateState();
	uint16_t getAnalogValue( uint8_t _number);
	bool getMeasurement();
	uint8_t octave;
	uint8_t _lower;
	uint8_t semitone;
	bool above;

	uint32_t getTime();
	uint32_t getAveragePeriod(uint8_t channel);

//	void selectKnob(uint8_t _knob);

	//void setClkOut(bool _state);
	//void setHorLed(uint8_t _number, bool _state);
	//void setVerLed(uint8_t _number, bool _state);
	//void dimLed(uint8_t _led, bool _state);
	//void dimHorLed(uint8_t _led, bool _state);
	//void dimVerLed(uint8_t _led, bool _state);
	//void setClock(bool _master);
	uint8_t _measuredPin;
	uint32_t PCperiod;
	uint32_t PCtime;
	uint32_t lastPCtime;
	uint8_t runningCounter;
	uint32_t runningPCtime;
	uint32_t runningSum;

	uint32_t runningBuffer[8];
	uint8_t T1ADD;
	void isr_updateDAC();
	void setVoltage(uint8_t channel, uint16_t voltage);
	void writeDAC(uint8_t channel, uint16_t voltage);
	void autoCalibrate(uint8_t channel);
	void allDACLow();
	void setNote(uint8_t channel, uint8_t note);
	bool tuning;
	uint16_t getScaling(uint8_t channel);
	uint16_t getOffset(uint8_t channel);
	bool analyseNote(uint8_t channel);
	uint16_t calibrationTable[4][NUMBER_OF_TUNING_POINTS];
	bool getStableMeasurement();
	uint16_t scale[4];
	uint16_t offset[4];
	bool makeAutoTuneTable(uint8_t channel);
	uint16_t getToPeriod(uint8_t channel, uint32_t _destination);
	uint16_t getToNote(uint8_t channel, uint8_t _note);
	void fillTuningTable(uint8_t channel);
	bool periodToNote(uint32_t _period);
	uint32_t noteToPeriod(uint8_t channel, uint8_t _note);
	bool autoTune(uint8_t channel);
	bool reAutoTune(uint8_t channel);
	bool reMakeAutoTuneTable(uint8_t channel);
	uint8_t highestNote[4];
	uint8_t lowestNote[4];

private:
	/**TIMING**/

	uint8_t lowestOctave[4];
	uint8_t readButton;
	uint16_t bastlCycles;
	uint16_t buttonHash;
	uint16_t lastButtonHash;
	uint8_t shiftHash[3];
	uint8_t dimHash[3];
	uint8_t currentAnalogChannel;
	uint8_t updatedDAC;
	uint8_t updateWhich;
	uint8_t gateHash;
	bool scaleOffset[4];
	bool master;
	/**BUTTONS**/
	void setMuxBits(uint8_t _bits);
	void (*buttonChangeCallback)(uint8_t number);
	void (*clockInCallback)(uint8_t number);
	uint8_t buffer595[3];
	uint8_t displayBuffer;
	/**TRIGGERS**/
	uint8_t trigState;
	uint16_t triggerCountdown[8];
	uint16_t analogValues[3];
	uint16_t DACvoltage[4];

	uint16_t autoTuneTable[4][NUMBER_OF_TUNING_POINTS];
	bool autoTuneActive[4];




};




#endif
