/*
 * LITTLE NERD
 * eurorack module by Bastl Instruments
 * www.bastl-instruments.com
 * Vaclav Pelousek
 * licensed under cc-by-sa
 *
 *TODO
 *finish visual testing
 *do audioble testing
 *implement production code = testing + eeprom reset
 *implement midi ?
 *
 */

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

#include <portManipulations.h>
#include "multiChannelOscillator.h"

extern MultiChannelOscillator oscil;



void setup() {


	Serial.begin(115200);
	Serial.println("start");

	oscil.init();

}



void loop() {

	oscil.fillBuffer();
	Serial.println(oscil.fillCount);





}




