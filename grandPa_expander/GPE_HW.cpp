
#include "osc_noiseHW.h"
#include <fastAnalogRead.h>


#define DEBUGPIN B,5
#define SWITCHPIN B,0

osc_noiseHW hardware;


void osc_noiseHW::init() {

	fastAnalogRead::init();

	currentAnalogChannel = 1;
	fastAnalogRead::connectChannel(currentAnalogChannel);
	fastAnalogRead::startConversion();

	bit_dir_inp(SWITCHPIN);
	bit_set(SWITCHPIN); // enable pullup

}


void osc_noiseHW::update(){
	if(fastAnalogRead::isConversionFinished()){

		uint8_t reading = fastAnalogRead::getConversionResult()>>2;

		// KNOB
		if (currentAnalogChannel < numbKnobs) {
			knobStates[currentAnalogChannel-1] = reading;

		// CV
		} else {
			CVStates[currentAnalogChannel-numbKnobs-1] = reading;
		}

		// Start next conversion
		currentAnalogChannel++;
		if(currentAnalogChannel == numbKnobs+numbCVs+1) currentAnalogChannel = 1;

		fastAnalogRead::connectChannel(currentAnalogChannel);
		fastAnalogRead::startConversion();
	}

	// read cv routing switch
	if (bit_read_in(SWITCHPIN)) switchState = true;
	else						switchState = false;
}





uint8_t osc_noiseHW::getKnobValue(uint8_t number) {
	if (number < numbKnobs)	return knobStates[number];
	else 					return 0;

}

uint8_t osc_noiseHW::getCVValue(uint8_t number) {
	if (number < numbCVs) return CVStates[number];
	else				  return 0;
}

bool osc_noiseHW::getSwitchState() {
	return switchState;
}



