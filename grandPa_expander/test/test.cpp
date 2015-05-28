#include <stdio.h>
#include <inttypes.h>


#include "../multiChannelOscillator.h"


MultiChannelOscillator oscil;

uint16_t frequencies[8] =  {153,185,267,327,465,1023,587,845};
uint16_t frequenciesNew[8] = {300,432,498,700,900,2000,587,845};
uint8_t pinIndices[8]  = {0,1,2,3,4,5,6,7};



int main( int argc, const char* argv[] ) {

	oscil.init(pinIndices);
	oscil.setFrequencies(frequencies);

	const uint8_t numbEvents = oscil.eventBufferSize;
	const uint8_t eventsBeforeChange = 50;

	// get first element in buffer. We now have the space until the head would need to wrap
	toggleEvent* buffer = oscil.buffer.getPointer();

	// add some events, then change frequency and add some more events
	uint8_t index=0;
	for (; index<eventsBeforeChange; index++) {
		oscil.queueNextToggle();
	}
	oscil.setFrequencies(frequenciesNew);
	for (;index<numbEvents; index++) {
		oscil.queueNextToggle();
	}

	// CHECK FLIPS

	uint16_t flipTimes[8][numbEvents];
	uint8_t flipTimesIndices[8] = {0,0,0,0,0,0,0,0};

	//clear buffer
	for (uint8_t i=0; i<numbEvents; i++) {
		for (uint8_t bit=0; bit<8; bit++) {
			flipTimes[bit][i]=0;
		}
	}



	// go through buffered events
	for (uint8_t i=0; i<numbEvents; i++) {
		uint8_t bits = buffer->bits;

		// go through actual HW bits
		for (uint8_t bit=0; bit<8; bit++) {

			if (bits & (1<<bit)) {
				flipTimesIndices[bit]++;
			}
			flipTimes[bit][flipTimesIndices[bit]] += buffer->time;
		}

		buffer++; //next event
	}

	// How many events have there been for every pin?
	uint8_t maxEvents = 0;
	for (uint8_t bit=0; bit<8; bit++) {
		//printf("Bit %u: %u events\n",bit,flipTimesIndices[bit]);
		if (flipTimesIndices[bit]>0) flipTimesIndices[bit]--;
		if (maxEvents<flipTimesIndices[bit]) maxEvents= flipTimesIndices[bit];
	}

	//print result
	printf("0\t1\t2\t3\t4\t5\t6\t7\n");
	printf("-------------------------------------------------------------------\n");
	for (uint8_t i=0; i<=maxEvents; i++) {
		for (uint8_t bit=0;bit<8; bit++) {
			if (i<=flipTimesIndices[bit]) {
				printf("%u\t",flipTimes[bit][i]);
			} else {
				printf("#\t");
			}
		}
		printf("\n");
	}
}
