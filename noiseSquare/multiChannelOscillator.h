/*
 * multiChannelOscillator.h
 *
 *  Created on: 23.10.2014
 *      Author: Lennart Schierling for bastl-instruments
 *
 *
 *  \brief Up to 8 channel square wave oscillator, tunable during runtime.
 *
 *  All channels have to be on the same port. On atmega328p, there are 8 usable pins only on port D
 *  and using all of them will disable the hardware serial port. If you need less channels, you can also
 *  use a different port.
 *
 *  With every channel there is a bit array associated that defines the pins that this channel is mapped to.
 *  This way you can map a channel to several pins and also have different channels (with different frequencies)
 *  work on the same pin, probably creating some interesting effects.
 *
 *  As the pin toggle events are calculated in advance and then buffered, you need to make sure that this buffer
 *  does not run empty. Calling fillBuffer() regularly does the job. You can also change the size of the buffer
 *  via a define letting you trade off more infrequent buffer filling, memory consumption and response time.
 *  If the buffer runs empty, the oscillator is stopped.
 *
 */

#ifndef MULTICHANNELOSCILLATOR_H_
#define MULTICHANNELOSCILLATOR_H_

/// *** SETTINGS *** ///


// which port should be used to toggle
#define OSCIL_PORT D

// The length of the event buffer in number of events
// * The delay introduced by the buffer depends on speed of events.
//   Assuming events are spaced on average 127 ticks with a prescaler of 64 and 20MHz crystal,
//   the average delay is EVENT_BUFFER_SIZE * 0.4 ms
// * Increasing the oscillator frequency has a higher delay than decreasing it
// * Larger buffers increase the delay, shorter buffers increase the risk of an underrun
#define EVENT_BUFFER_SIZE 20

// The numbers of channels to handle.
// This could be derived from the argument of init() but is hardcoded to avoid dynamic allocation
// of FiFoBuffer inside of init()
#define NUMB_CHANNELS 8





/// **** CODE *** ///


#include <inttypes.h>
#include <portManipulations.h>
#include <FiFoBuffer.h>

#ifndef TESTING
#include <avr/io.h>
#endif






struct toggleEvent {
	uint16_t time;
	uint8_t bits;
};




class MultiChannelOscillator {

public:
	MultiChannelOscillator() {
	};

	// initialize the hardware and set the pins that are controlled by each channel
	void init(uint8_t* pinIndices);

	// set the frequencies of all channels at the same time
	void setFrequencies(uint16_t* frequencies);

	// set frequency of individual channel
	void setFrequency(uint8_t channel, uint16_t frequency);

	// start the oscillator
	void start();

	// stop the oscillator
	void stop();

	// fill up the event buffer
	void fillBuffer();

	static const uint8_t eventBufferSize = EVENT_BUFFER_SIZE;
	static const uint8_t numbChannels = NUMB_CHANNELS;


public:
	// print the next events inside the buffer via printf(). used for external testing only
	void printBuffer();


public:
	// called in ISR. This checks if a toggle event is present for the current call tick
	void performToggle();


private:

	FiFoBuffer<eventBufferSize,toggleEvent> buffer;

	uint8_t channelMappings[numbChannels];
	uint16_t frequencies[numbChannels];
	int16_t compareValues[numbChannels];
	int16_t currentCompareValues[numbChannels];

	uint8_t channelEnabled;

	void calcCompareValues();
	void queueNextToggle();

};





#endif /* MULTICHANNELOSCILLATOR_H_ */
