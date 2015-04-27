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
		10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 143, 148, 153, 158, 164, 169, 174, 179, 184, 190, 195, 200, 205, 210, 216, 221, 226, 231, 237, 242, 247, 252, 257, 263, 268, 273, 278, 283, 289, 294, 299, 304, 310, 315, 320, 325, 330, 336, 341, 346, 351, 356, 362, 367, 372, 377, 382, 388, 393, 398, 403, 409, 414, 419, 424, 429, 435, 440, 445, 450, 455, 461, 466, 471, 476, 482, 487, 492, 497, 502, 508, 513, 518, 523, 528, 534, 539, 544, 549, 555, 560, 565, 570, 575, 581, 586, 591, 596, 601, 607, 612, 617, 622, 627, 633, 638, 643, 648, 654, 659, 664, 669, 674, 680, 685, 690, 695, 700, 706, 711, 716, 721, 727, 732, 737, 742, 747, 753, 758, 763, 768, 773, 779, 784, 789, 794, 800
};

const uint16_t cowbellTableLow[] PROGMEM = {
		8, 8, 9, 10, 11, 11, 12, 13, 14, 14, 15, 16, 17, 18, 18, 19, 20, 21, 21, 22, 23, 24, 25, 25, 26, 27, 28, 28, 29, 30, 31, 31, 32, 33, 34, 35, 35, 36, 37, 38, 38, 39, 40, 41, 42, 42, 43, 44, 45, 45, 46, 47, 48, 48, 49, 50, 51, 52, 52, 53, 54, 55, 55, 56, 57, 58, 59, 59, 60, 61, 62, 62, 63, 64, 65, 66, 66, 67, 68, 69, 69, 70, 71, 72, 72, 73, 74, 75, 76, 76, 77, 78, 79, 79, 80, 81, 82, 83, 83, 84, 85, 86, 86, 87, 88, 89, 89, 90, 91, 92, 93, 93, 94, 95, 96, 96, 97, 98, 99, 100, 100, 101, 102, 103, 103, 104, 105, 106, 107, 111, 115, 119, 123, 127, 131, 135, 139, 143, 147, 151, 155, 159, 163, 167, 171, 175, 179, 183, 187, 191, 196, 200, 204, 208, 212, 216, 220, 224, 228, 232, 236, 240, 244, 248, 252, 256, 260, 264, 268, 272, 276, 281, 285, 289, 293, 297, 301, 305, 309, 313, 317, 321, 325, 329, 333, 337, 341, 345, 349, 353, 357, 361, 366, 370, 374, 378, 382, 386, 390, 394, 398, 402, 406, 410, 414, 418, 422, 426, 430, 434, 438, 442, 446, 451, 455, 459, 463, 467, 471, 475, 479, 483, 487, 491, 495, 499, 503, 507, 511, 515, 519, 523, 527, 531, 536, 540, 544, 548, 552, 556, 560, 564, 568, 572, 576, 580, 584, 588, 592, 596, 600, 604, 608, 612, 616, 621
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
	frequencies[7] = mapProgmemU8U16(cowbellIn,cowbellTableHigh);
	frequencies[6] = mapProgmemU8U16(cowbellIn,cowbellTableLow);

	digiNoise.checkForBitFlip();
	oscil.setFrequencies(frequencies);
	digiNoise.checkForBitFlip();


	//** Set digital noise top frequency
	digiNoise.setTopFreq((digiNoiseIn<<5)+62);


	digiNoise.checkForBitFlip();

}




