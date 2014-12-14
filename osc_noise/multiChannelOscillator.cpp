/*
 * multiChannelOscillator.cpp
 *
 *  Created on: 23.10.2014
 *      Author: user
 */


#include "multiChannelOscillator.h"

#include <util/atomic.h>












#ifndef TESTING
#include <Arduino.h>
#define DBG
#define PIN B,4
#endif

#ifdef TESTING
#define F_CPU 16000000L
#include <stdio.h>
volatile uint8_t pinRegister = 0;
#endif



void MultiChannelOscillator::stop() {
#ifndef TESTING
	setLow(TIMSK1,OCIE1A);
#else
	printf("stopped\n");
#endif
}

void MultiChannelOscillator::start() {

	// first, fill Buffer in case it was started from empty buffer
	fillBuffer();

	// get next event, set it to timer and start interrupt
	toggleEvent event;
	buffer.get(event);
	#ifndef TESTING
	OCR1A = event.time;
	setHigh(TIMSK1,OCIE1A);
	#endif
}

void MultiChannelOscillator::setFrequencies(uint16_t* frequencies) {

	// save settings
	for (uint8_t index=0; index<numbChannels; index++) {
		this->frequencies[index] = frequencies[index];
	}

	// calculate time distances for frequencies
	calcCompareValues();

}


void MultiChannelOscillator::init(uint8_t* pinIndices) {

	#ifndef TESTING
		cli();
	#endif

	// save pin Indices
	for (uint8_t index=0; index<numbChannels; index++) {
		this->channelMappings[index] = (1<<pinIndices[index]);
	}


	#ifndef TESTING

	// DEBUG
	bit_dir_outp(PIN);

	// TIMER
	TCCR1A = 0;
	TCCR1B = (1<<WGM12); 	// CTC
	TCCR1B |= (1<<CS10) | (1<<CS11); 	// prescaler = 64

	// INIT
	for (uint8_t bit=0; bit<numbChannels; bit++) {
		REG_DIR(OSCIL_PORT) |= channelMappings[bit];
		REG_PORT(OSCIL_PORT) &= ~(channelMappings[bit]);
	}
	#endif


	#ifndef TESTING
	sei();
	#endif


}

void MultiChannelOscillator::calcCompareValues() {
	for (uint8_t index=0; index<numbChannels; index++) {
		//ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
		{
			compareValues[index] = (F_CPU / 128) / frequencies[index];
			currentCompareValues[index] = compareValues[index];
		}
		#ifdef TESTING
		printf("Channel %u: %u\n",index,compareValues[index]);
		#endif
	}
	#ifdef TESTING
		printf("\n");
	#endif
}


inline void MultiChannelOscillator::queueNextToggle() {

	// the last step that as been calculated
	static uint16_t lastTime = 0;
	static uint8_t lastPins = 0;

	// add pin action that has been calculated in previous call
	toggleEvent event;
	event.bits = lastPins;


	// get new time and pins
	uint16_t distanceToNext = -1;




	for (uint8_t index=0; index<numbChannels; index++) {

		// new relative toggle distances
		currentCompareValues[index] -= lastTime;
		if (currentCompareValues[index] <= 0) currentCompareValues[index] += compareValues[index];


		// find nearest and bits to flip at next position
		if (currentCompareValues[index] < distanceToNext) {
			lastPins = channelMappings[index];
			distanceToNext = currentCompareValues[index];
		}

		if (currentCompareValues[index] == distanceToNext) {
			lastPins |= channelMappings[index];
		}


	}

	// correct time it takes to call ISR and function it calls
	//if (distanceToNext != 0) distanceToNext-=1;

	// store calculated time and keep it for next run
	event.time = distanceToNext;
	if (event.time>2) event.time--;


	lastTime = distanceToNext;


	// add event to buffer
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
	buffer.add(event);
	}



}

void MultiChannelOscillator::fillBuffer() {

	while(!buffer.isFull()) {
		queueNextToggle();
	}

}

void MultiChannelOscillator::printBuffer() {
	#ifdef TESTING
	volatile toggleEvent* eventPtr = buffer.getPointer();
	for (uint8_t index=0; index<eventBufferSize; index++) {
		printf("do %u then wait %u\n",eventPtr->bits,eventPtr->time);
		eventPtr++;
	}
	#endif
}

inline void MultiChannelOscillator::performToggle() {

		toggleEvent event;
		if (buffer.get(event)) {
	#ifndef TESTING
			OCR1A = event.time;
			REG_PIN(OSCIL_PORT) = event.bits;
	#else
			printf("Flipping %u and setting timer to %u\n",event.bits,event.time);
	#endif

		} else {
			//Serial.println("BUFFER EMPTY!");
			stop();
		}

}



#ifndef TESTING
MultiChannelOscillator oscil;

//max 64 cycles = 4us
ISR(TIMER1_COMPA_vect) {
	//bit_set(PIN);
	//sei();
	oscil.performToggle();
	//bit_clear(PIN);

}

#endif

