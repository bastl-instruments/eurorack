
#ifndef GPE_HW_H_
#define GPE_HW_H_

#include <inttypes.h>




class GPE_HW  {

public:

	void init();

	void update();


	uint8_t getCVValue(uint8_t number);
	uint8_t getLastCVValue(uint8_t number);
	bool getSwitchState();

private:


	static const uint8_t numbCVs = 6;
	uint8_t CVStates[numbCVs];
	uint8_t lastCVStates[numbCVs];

	uint8_t currentAnalogChannel;

	bool switchState;

};




#endif
