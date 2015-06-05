
#include "GPE_HW.h"
#include <fastAnalogRead.h>


#define DEBUGPIN B,5
#define SWITCHPIN B,0

GPE_HW hardware;


void GPE_HW::init() {
/*
	fastAnalogRead::init();

	currentAnalogChannel = 1;
	fastAnalogRead::connectChannel(currentAnalogChannel);
	fastAnalogRead::startConversion();
*/
	bit_dir_inp(SWITCHPIN);
	bit_set(SWITCHPIN); // enable pullup

}


void GPE_HW::update(){
	for(int i=0;i<6;i++){
		lastCVStates[i]=CVStates[i];
		CVStates[i] = analogRead(i)>>2;
	}
	/*
	lastCVStates[5]=CVStates[5];
	if(digitalRead(19)) CVStates[5]=255;
	*/
	/*
	if(fastAnalogRead::isConversionFinished()){
		lastCVStates[currentAnalogChannel]=CVStates[currentAnalogChannel];
		uint8_t reading = fastAnalogRead::getConversionResult()>>2;


		CVStates[currentAnalogChannel] = reading;
		// Start next conversion
		currentAnalogChannel++;
		if(currentAnalogChannel >5) currentAnalogChannel = 0;
		fastAnalogRead::connectChannel(currentAnalogChannel);
		fastAnalogRead::startConversion();
	}
*/
	// read cv routing switch
	if (bit_read_in(SWITCHPIN)) switchState = true;
	else						switchState = false;
}

const uint8_t cvMap[6]={0,1,2,3,5,4};
uint8_t GPE_HW::getCVValue(uint8_t number) {
	return CVStates[cvMap[number]];
}
uint8_t GPE_HW::getLastCVValue(uint8_t number) {
	return lastCVStates[cvMap[number]];
}
bool GPE_HW::getSwitchState() {
	return switchState;
}



