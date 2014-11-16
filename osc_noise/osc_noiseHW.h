
#ifndef OSC_NOISEHW_H_
#define OSC_NOISEHW_H_

#include <inttypes.h>




class osc_noiseHW  {

public:

	void init();

	void update();

	uint8_t getKnobValue(uint8_t number);
	void freezeKnob(uint8_t number, uint8_t value);

	uint8_t getCVValue(uint8_t number);

private:
	static const uint8_t numbKnobs = 3;
	uint8_t knobStates[numbKnobs];
	uint8_t knobSatesOld[numbKnobs];
	uint8_t knobFreezeBits;

	static const uint8_t numbCVs = 2;
	uint8_t CVStates[numbCVs];

	uint8_t currentAnalogChannel;



};




#endif
