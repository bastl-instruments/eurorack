#ifdef EXTERNAL_IDE

#include <Arduino.h>
#define PIN B,4


int main(void) {

  init();
  setup();

  while(true) {
    loop();
  }
}

#endif

//////////////////////////////////////////////////

#include <portManipulations.h>
#include "multiChannelOscillator.h"
#include "osc_noiseHW.h"
#include "digitalNoise.h"


//#define DEBUG

extern MultiChannelOscillator oscil;
extern osc_noiseHW hardware;
digitalNoise digiNoise;

const uint8_t numbMetallicChannels = 6;
const uint8_t numbCowbellChannels = 2;

uint16_t minFrequencies[numbMetallicChannels + numbCowbellChannels] = {153,185,267,327,465,1023,377,293};//587,754}; //
uint8_t pinIndices[numbMetallicChannels + numbCowbellChannels]  = {2,3,4,5,6,7,0,1};
uint16_t frequencies[numbMetallicChannels + numbCowbellChannels];






void setup() {


	TIMSK2 = 0;

	#ifdef DEBUG
	Serial.begin(115200);
	Serial.println("start");
	#endif


	oscil.init(pinIndices);
	oscil.setFrequencies(minFrequencies);

	oscil.start();

	digiNoise.init();
	digiNoise.setTopFreq(1000);

	hardware.init();


}



void loop() {


	// get new CV and knob values
	hardware.update();

	digiNoise.checkForBitFlip();

	oscil.fillBuffer();



	digiNoise.checkForBitFlip();

	// set metallic frequencies
	uint8_t index=0;
	uint16_t metalicOffset=0;
	if(hardware.getSwitchState()){
		metalicOffset=hardware.getKnobValue(1);
	}
	else{
		metalicOffset=hardware.getKnobValue(1)+hardware.getCVValue(1);
	}
	for (; index<numbMetallicChannels; index++) {
		frequencies[index] = minFrequencies[index]/8 + (((uint32_t)minFrequencies[index]*metalicOffset)>>9);
		digiNoise.checkForBitFlip();
	}




	// set cowbell frequencies
	uint16_t freq=hardware.getKnobValue(2)+(hardware.getCVValue(0)>>1);
	for (; index<numbMetallicChannels+numbCowbellChannels; index++) {
		frequencies[index] = minFrequencies[index]/16 + (((uint32_t)minFrequencies[index]*freq)>>6); // WAS >>8 changed here
		//frequencies[index] = lookUp[index-numbMetallicChannels][freq];
		digiNoise.checkForBitFlip();
	}

	digiNoise.checkForBitFlip();
	oscil.setFrequencies(frequencies);
	digiNoise.checkForBitFlip();


	// calculate digital noise top frequency
	if(hardware.getSwitchState()){
		freq=hardware.getKnobValue(0)+(hardware.getCVValue(1)>>1);
	}
	else{
		freq=hardware.getKnobValue(0);
	}

	digiNoise.checkForBitFlip();
	digiNoise.setTopFreq((freq<<5)+62);


	digiNoise.checkForBitFlip();




}




