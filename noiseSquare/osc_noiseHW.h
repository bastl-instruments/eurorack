
#ifndef OSC_NOISEHW_H_
#define OSC_NOISEHW_H_

#include <inttypes.h>




class osc_noiseHW  {

public:

	void init();

	void update();

	uint8_t getKnobValue(uint8_t number);
	uint8_t getCVValue(uint8_t number);
	bool getSwitchState();

private:
	static const uint8_t numbKnobs = 3;
	uint8_t knobStates[numbKnobs];

	static const uint8_t numbCVs = 2;
	uint8_t CVStates[numbCVs];

	uint8_t currentAnalogChannel;

	bool switchState;

};




#endif
