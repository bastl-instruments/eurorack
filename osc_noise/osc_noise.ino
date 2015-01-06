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

uint16_t minFrequencies[numbMetallicChannels + numbCowbellChannels] = {153,185,267,327,465,1023,587,845};
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

	oscil.fillBuffer();



	digiNoise.checkForBitFlip();

	// set metallic frequencies
	uint8_t index=0;

	for (; index<numbMetallicChannels; index++) {
		frequencies[index] = minFrequencies[index]/8 + (((uint32_t)minFrequencies[index]*hardware.getKnobValue(1))>>6);
	}

	// set cowbell frequencies
	uint16_t freq=hardware.getKnobValue(2)+(hardware.getCVValue(1)>>1);
	for (; index<numbMetallicChannels+numbCowbellChannels; index++) {
		frequencies[index] = minFrequencies[index]/16 + (((uint32_t)minFrequencies[index]*freq)>>7); //changed here
	}

	oscil.setFrequencies(frequencies);


	// calculate digital noise top frequency
	 freq=hardware.getKnobValue(0)+(hardware.getCVValue(0)>>1);

	digiNoise.setTopFreq((freq<<6)+62);


	digiNoise.checkForBitFlip();




}




