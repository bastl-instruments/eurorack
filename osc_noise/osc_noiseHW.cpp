
#include "osc_noiseHW.h"
#include <avr/io.h>
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

		if (currentAnalogChannel < numbKnobs) {
			// save knob State
			// check unfreeze
		} else {
			CVStates[currentAnalogChannel-numbKnobs] = fastAnalogRead::getConversionResult()>>2;
		}

		currentAnalogChannel++;
		if(currentAnalogChannel == numbKnobs+numbCVs) currentAnalogChannel = 0;

		fastAnalogRead::connectChannel(currentAnalogChannel);
		fastAnalogRead::startConversion();
	}
}

void osc_noiseHW::freezeKnob(uint8_t index, uint8_t value){


}

/*

uint32_t osc_noiseHW::getElapsedBastlCycles() {
	return bastlCycles;
}

uint16_t osc_noiseHW::getBastlCyclesPerSecond() {
	return (F_CPU/1024)/OCR2A;
}




ISR(TIMER2_COMPA_vect) {


//	bit_set(PIN_0);
	hardware.incrementBastlCycles();

}
*/

