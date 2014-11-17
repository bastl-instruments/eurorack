
#include "osc_noiseHW.h"
#include <avr/io.h>
#include <stdlib.h>
#include <fastAnalogRead.h>

const uint8_t KNOB_UNFREEZE_THRES = 10;


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

			// frozen
			if (knobFreezeBits & (1<<currentAnalogChannel)) {
				uint8_t move = abs(reading - knobStatesOld[currentAnalogChannel]);
				if ((move > KNOB_UNFREEZE_THRES) || (reading == knobStatesOld[currentAnalogChannel])) {
					setLow(knobFreezeBits,currentAnalogChannel);
					knobStates[currentAnalogChannel] = reading;
				}
				knobStatesOld[currentAnalogChannel] = reading;

			// not frozen
			} else {
				knobStates[currentAnalogChannel] = reading;
			}

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



void osc_noiseHW::freezeKnob(uint8_t index, uint8_t value){

	// if newly frozen --> reset tracking
	if (!(knobFreezeBits & (1<<index))) knobStatesOld[index] = value;

	knobStates[index] = value;
	setHigh(knobFreezeBits,index);

}

uint8_t osc_noiseHW::getKnobValue(uint8_t number) {
	if (number < numbKnobs)	return knobStates[number];
	else 					return 0;

}

uint8_t osc_noiseHW::getCVValue(uint8_t number) {
	if (number < numbCVs) return CVStates[number];
	else				  return 0;
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

