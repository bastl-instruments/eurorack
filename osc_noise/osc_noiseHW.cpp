
#include "osc_noiseHW.h"
#include <fastAnalogRead.h>


#define PIN B,5

osc_noiseHW hardware;


void osc_noiseHW::init() {

	fastAnalogRead::init();

	currentAnalogChannel = 0;
	fastAnalogRead::connectChannel(currentAnalogChannel);
	fastAnalogRead::startConversion();

}


void osc_noiseHW::update(){
	if(fastAnalogRead::isConversionFinished()){

		uint8_t reading = fastAnalogRead::getConversionResult()>>2;

		// KNOB
		if (currentAnalogChannel < numbKnobs) {
			knobStates[currentAnalogChannel] = reading;

		// CV
		} else {
			CVStates[currentAnalogChannel-numbKnobs] = reading;
		}

		// Start next conversion
		currentAnalogChannel++;
		if(currentAnalogChannel == numbKnobs+numbCVs) currentAnalogChannel = 0;

		fastAnalogRead::connectChannel(currentAnalogChannel);
		fastAnalogRead::startConversion();
	}
}





uint8_t osc_noiseHW::getKnobValue(uint8_t number) {
	if (number < numbKnobs)	return knobStates[number];
	else 					return 0;

}

uint8_t osc_noiseHW::getCVValue(uint8_t number) {
	if (number < numbCVs) return CVStates[number];
	else				  return 0;
}



