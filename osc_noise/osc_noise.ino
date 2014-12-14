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


#define DEBUG

extern MultiChannelOscillator oscil;
extern osc_noiseHW hardware;

const uint8_t numbMetallicChannels = 6;

uint16_t minFrequencies[numbMetallicChannels] = {153,185,267,327,465,1023};
uint16_t frequencies[numbMetallicChannels] = {153,185,267,327,465,1023};
uint8_t pinIndices[numbMetallicChannels]  = {2,3,4,5,6,7};

uint16_t testFrequencies[numbMetallicChannels] = {7000,5000,50,327,465,1000};



void setup() {


	TIMSK0 = 0; // disable millis timer to not disturb our interrupt
	TIMSK2 = 0;

	#ifdef DEBUG
	Serial.begin(115200);
	Serial.println("start");
	#endif


	oscil.init(pinIndices);
	oscil.setFrequencies(minFrequencies);
	oscil.printBuffer();
	oscil.start();

	hardware.init();




}



void loop() {


	// get new CV and knob values
	hardware.update();

	oscil.fillBuffer();


	static uint8_t metallicFreq;
	if (hardware.getKnobValue(1) != metallicFreq) {
		metallicFreq = hardware.getKnobValue(1);

		for (uint8_t index=0; index<numbMetallicChannels; index++) {
			frequencies[index] = minFrequencies[index]/2 + ((minFrequencies[index]*metallicFreq)>>7);
		}

		oscil.setFrequencies(frequencies);

	}






}




