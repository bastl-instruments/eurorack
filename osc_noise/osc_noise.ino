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


#define DEBUG

extern MultiChannelOscillator oscil;
extern osc_noiseHW hardware;
digitalNoise digiNoise;

const uint8_t numbMetallicChannels = 6;

uint16_t minFrequencies[numbMetallicChannels] = {153,185,267,327,465,1023};
uint16_t frequencies[numbMetallicChannels] = {153,185,267,327,465,1023};
uint8_t pinIndices[numbMetallicChannels]  = {2,3,4,5,6,7};





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

	// calculate metallic noise frequencies
	static uint8_t metallicFreq = 0;
	if (hardware.getKnobValue(1) != metallicFreq) {
		metallicFreq = hardware.getKnobValue(1);

		for (uint8_t index=0; index<numbMetallicChannels; index++) {
			frequencies[index] = minFrequencies[index]/2 + ((minFrequencies[index]*metallicFreq)>>7);
		}

		oscil.setFrequencies(frequencies);
	}

	// calculate digital noise top frequency
	static uint8_t noiseFreq = 0;
	if (hardware.getKnobValue(0) != noiseFreq) {
		noiseFreq = hardware.getKnobValue(0);

		digiNoise.setTopFreq((noiseFreq<<6)+62);
	}

	digiNoise.checkForBitFlip();




}




