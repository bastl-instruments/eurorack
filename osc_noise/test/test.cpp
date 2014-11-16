#include <stdio.h>
#include <inttypes.h>


#include "../multiChannelOscillator.h"


MultiChannelOscillator oscil;



int main( int argc, const char* argv[] ) {

	oscil.init();
	//oscil.printBuffer();

	for (uint8_t f=0; f<100; f++) {
		oscil.performToggle();
	}

	// check flips
	toggleEvent* buffer = oscil.buffer.getPointer();
	uint16_t channelTimes[8] = {0,0,0,0,0,0};

	for (uint8_t i=0; i<100; i++) {
		uint8_t bits = buffer->bits;

		for (uint8_t bit=0; bit<8; bit++) {

			if (bits & (1<<bit)) {
				//printf("Flip %u after %u\n",bit,channelTimes[bit]);
				if (channelTimes[bit] == oscil.compareValues[bit]) ;
				else printf("error\n");
				channelTimes[bit] = 0;
			}

			channelTimes[bit] += buffer->time;
		}

		buffer++;
	}
}
