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

extern MultiChannelOscillator oscil;
extern osc_noiseHW hardware;

uint16_t frequencies[6] = {153,185,267,327,465,1023};
uint8_t pinIndices[6]  = {2,3,4,5,6,7};



void setup() {


	TIMSK0 = 0; // disable millis timer to not disturb our interrupt
	TIMSK2 = 0;

	Serial.begin(115200);
	Serial.println("start");




	oscil.init(frequencies,pinIndices);


	oscil.printBuffer();


	oscil.start();

	hardware.init();





}



void loop() {


	oscil.fillBuffer();



}




