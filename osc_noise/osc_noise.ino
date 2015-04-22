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
#include <mapping.h>
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

const uint16_t cowbellTableHigh[] PROGMEM = {
		10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 141, 143, 145, 147, 149, 151, 153, 154, 156, 158, 160, 162, 164, 166, 168, 169, 171, 173, 175, 177, 179, 181, 183, 185, 186, 188, 190, 192, 194, 196, 198, 200, 201, 203, 205, 207, 209, 211, 213, 215, 217, 218, 220, 222, 224, 226, 228, 230, 232, 233, 235, 237, 239, 241, 243, 245, 247, 249, 250, 252, 254, 256, 258, 260, 262, 264, 265, 267, 269, 271, 273, 275, 277, 279, 281, 282, 284, 286, 288, 290, 292, 294, 296, 297, 299, 301, 303, 305, 307, 309, 311, 313, 314, 316, 318, 320, 322, 324, 326, 328, 329, 331, 333, 335, 337, 339, 341, 343, 345, 346, 348, 350, 352, 354, 356, 358, 360, 361, 363, 365, 367, 369, 371, 373, 375, 377
};

const uint16_t cowbellTableLow[] PROGMEM = {
		8, 8, 9, 10, 11, 11, 12, 13, 14, 14, 15, 16, 17, 18, 18, 19, 20, 21, 21, 22, 23, 24, 25, 25, 26, 27, 28, 28, 29, 30, 31, 31, 32, 33, 34, 35, 35, 36, 37, 38, 38, 39, 40, 41, 42, 42, 43, 44, 45, 45, 46, 47, 48, 48, 49, 50, 51, 52, 52, 53, 54, 55, 55, 56, 57, 58, 59, 59, 60, 61, 62, 62, 63, 64, 65, 66, 66, 67, 68, 69, 69, 70, 71, 72, 72, 73, 74, 75, 76, 76, 77, 78, 79, 79, 80, 81, 82, 83, 83, 84, 85, 86, 86, 87, 88, 89, 89, 90, 91, 92, 93, 93, 94, 95, 96, 96, 97, 98, 99, 100, 100, 101, 102, 103, 103, 104, 105, 106, 107, 108, 109, 111, 112, 114, 115, 117, 118, 120, 121, 123, 124, 126, 127, 128, 130, 131, 133, 134, 136, 137, 139, 140, 142, 143, 145, 146, 148, 149, 150, 152, 153, 155, 156, 158, 159, 161, 162, 164, 165, 167, 168, 169, 171, 172, 174, 175, 177, 178, 180, 181, 183, 184, 186, 187, 189, 190, 191, 193, 194, 196, 197, 199, 200, 202, 203, 205, 206, 208, 209, 210, 212, 213, 215, 216, 218, 219, 221, 222, 224, 225, 227, 228, 230, 231, 232, 234, 235, 237, 238, 240, 241, 243, 244, 246, 247, 249, 250, 251, 253, 254, 256, 257, 259, 260, 262, 263, 265, 266, 268, 269, 271, 272, 273, 275, 276, 278, 279, 281, 282, 284, 285, 287, 288, 290, 291, 293
};




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


	// Read basic knobs
	uint8_t digiNoiseIn = hardware.getKnobValue(0);
	uint8_t metallicIn = hardware.getKnobValue(1);
	uint8_t cowbellIn = hardware.getKnobValue(2);

	// Add CV
	uint16_t tmp = cowbellIn + hardware.getCVValue(0);
	if (tmp > 255) tmp = 255;
	cowbellIn = tmp;

	if (hardware.getSwitchState()) {
		uint16_t tmp = digiNoiseIn + (hardware.getCVValue(1)>>1);
		if (tmp > 255) tmp = 255;
		digiNoiseIn = tmp;
	} else {
		uint16_t tmp = metallicIn + hardware.getCVValue(1);
		if (tmp > 255) tmp = 255;
		metallicIn = tmp;
	}

	digiNoise.checkForBitFlip();

	//** Set metallic frequencies
	for (uint8_t index=0; index<numbMetallicChannels; index++) {
		frequencies[index] = minFrequencies[index]/8 + (((uint32_t)minFrequencies[index]*metallicIn)>>9);
		digiNoise.checkForBitFlip();
	}


	//** Set cowbell frequencies
	frequencies[6] = mapProgmemU8U16(metallicIn,cowbellTableHigh);
	frequencies[7] = mapProgmemU8U16(metallicIn,cowbellTableLow);

	digiNoise.checkForBitFlip();
	oscil.setFrequencies(frequencies);
	digiNoise.checkForBitFlip();


	//** Set digital noise top frequency
	digiNoise.setTopFreq((digiNoiseIn<<5)+62);


	digiNoise.checkForBitFlip();

}




