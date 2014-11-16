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
#include <FiFoBuffer.h>
#include <avr/io.h>

//#define TESTING // switch to test class with stdout
#define OSCIL_PORT D
#define PIN_0 2
#define PIN_1 3
#define PIN_2 4
#define PIN_3 5
#define PIN_4 6
#define PIN_5 7


// macros for accessing port register
#define REG_PIN(...) REG_PIN_(__VA_ARGS__)
#define REG_PIN_(L) PIN ## L

#define REG_PORT(...) REG_PORT_(__VA_ARGS__)
#define REG_PORT_(L) PORT ## L

#define REG_DIR(...) REG_DIR_(__VA_ARGS__)
#define REG_DIR_(L) DDR ## L




struct toggleEvent {
	uint8_t time;
	uint8_t bits;
};




class MultiChannelOscillator {

public:
	MultiChannelOscillator() {
		frequencies[0] = 153;
		frequencies[1] = 185;
		frequencies[2] = 267;
		frequencies[3] = 327;
		frequencies[4] = 465;
		frequencies[5] = 1023;

		channelMappings[0] = 1<<PIN_0;
		channelMappings[1] = 1<<PIN_1;
		channelMappings[2] = 1<<PIN_2;
		channelMappings[3] = 1<<PIN_3;
		channelMappings[4] = 1<<PIN_4;
		channelMappings[5] = 1<<PIN_5;
	};

	void init();
	void start();
	void stop();

	void performToggle();
	void fillBuffer();

	void printBuffer();

	static const uint8_t eventBufferSize = 10;
	FiFoBuffer<eventBufferSize,toggleEvent> buffer;

	static const uint8_t numbChannels = 6;
	uint8_t channelMappings[numbChannels];
	uint16_t frequencies[numbChannels];
	int16_t compareValues[numbChannels];
	int16_t currentCompareValues[numbChannels];


private:


	void queueNextToggle();

};





#endif /* MULTICHANNELOSCILLATOR_H_ */
