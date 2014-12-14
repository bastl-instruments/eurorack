/*
 * multiChannelOscillator.h
 *
 *  Created on: 23.10.2014
 *      Author: user
 */

#ifndef MULTICHANNELOSCILLATOR_H_
#define MULTICHANNELOSCILLATOR_H_

/// *** SETTINGS *** ///

//#define TESTING // switch to test class with stdout

// which port and which pins should be used to toggle
#define OSCIL_PORT D


/// **** CODE *** ///


#include <inttypes.h>
#include <portManipulations.h>
#include <FiFoBuffer.h>
#include <avr/io.h>

// macros for accessing port register
#define REG_PIN(...) REG_PIN_(__VA_ARGS__)
#define REG_PIN_(L) PIN ## L

#define REG_PORT(...) REG_PORT_(__VA_ARGS__)
#define REG_PORT_(L) PORT ## L

#define REG_DIR(...) REG_DIR_(__VA_ARGS__)
#define REG_DIR_(L) DDR ## L




struct toggleEvent {
	uint16_t time;
	uint8_t bits;
};




class MultiChannelOscillator {

public:
	MultiChannelOscillator() {
	};

	void init(uint8_t* pinIndices);
	void setFrequencies(uint16_t* frequencies);
	void start();
	void stop();

	void performToggle();
	void fillBuffer();

	void printBuffer();

	static const uint8_t eventBufferSize = 100;
	FiFoBuffer<eventBufferSize,toggleEvent> buffer;

	static const uint8_t numbChannels = 6;
	uint8_t channelMappings[numbChannels];
	uint16_t frequencies[numbChannels];
	int16_t compareValues[numbChannels];
	int16_t currentCompareValues[numbChannels];


private:


	void queueNextToggle();
	void calcCompareValues();

};





#endif /* MULTICHANNELOSCILLATOR_H_ */
