/*
 * multiChannelOscillator.h
 *
 *  Created on: 23.10.2014
 *      Author: user
 */

#ifndef MULTICHANNELOSCILLATOR_H_
#define MULTICHANNELOSCILLATOR_H_

#include <inttypes.h>
#include <portManipulations.h>

#define outputPort PORTC
#define outputDir  DDRC
#define outputPin  PINC


class MultiChannelOscillator {

public:
	MultiChannelOscillator() {
		frequencies[0] = 153;
		frequencies[1] = 185;
		frequencies[2] = 267;
		frequencies[3] = 327;
		frequencies[4] = 465;
		frequencies[5] = 1023;
	};

	void init();
	void start();
	void stop();

	void performToggle();
	void fillBuffer();

	void printBuffer();
	void printSeries();

	volatile uint8_t fillCount;

private:

	void incrementIndex(uint8_t& counter);
	void calcNextToggle(uint8_t& time, uint8_t& bits);

	static const uint8_t numbChannels = 6;
	uint16_t frequencies[numbChannels];
	int16_t compareValues[numbChannels];
	int16_t currentCompareValues[numbChannels];

	static const uint8_t eventBufferSize = 10;
	uint8_t eventBufferBits[eventBufferSize];
	uint8_t eventBufferTime[eventBufferSize];
	uint8_t eventBufferReadIndex;
	uint8_t eventBufferWriteIndex;


};





#endif /* MULTICHANNELOSCILLATOR_H_ */
