/*
 * multiChannelOscillator.cpp
 *
 *  Created on: 23.10.2014
 *      Author: user
 */


#include "multiChannelOscillator.h"
#include <util/delay.h>

#define PIN B,4

#define DBG

void MultiChannelOscillator::stop() {
	setLow(TIMSK1,OCIE1A);
}

void MultiChannelOscillator::start() {
	setHigh(TIMSK1,OCIE1A);
	OCR1A = eventBufferTime[eventBufferReadIndex];
}


void MultiChannelOscillator::init() {

	cli();

	for (uint8_t index=0; index<numbChannels; index++) {

		compareValues[index] = (F_CPU / 128) / frequencies[index];
		currentCompareValues[index] = compareValues[index];
		#ifdef DBG
		Serial.print(index);
		Serial.print(": ");
		Serial.println(compareValues[index]);
		#endif
	}

	// DEBUG
	bit_dir_outp(PIN);

	// TIMER
	TCCR1A = 0;
	TCCR1B = (1<<WGM12); 	// CTC
	TCCR1B |= (1<<CS10) | (1<<CS11); 	// prescaler = 64

	// INIT
	outputDir = B00111111;
	outputPort = 0;

	// calculate first events to be processed in isr
	uint8_t tmpTime=0;
	uint8_t bits;

	eventBufferBits[0] = 0;
	calcNextToggle(tmpTime,bits);
	eventBufferTime[0] = tmpTime;
	eventBufferBits[1] = bits;

	fillCount = 1;

	eventBufferReadIndex=eventBufferSize-1;
	eventBufferWriteIndex=0;

	fillBuffer();


	// START
	start();
	sei();


}


inline void MultiChannelOscillator::calcNextToggle(uint8_t& time, uint8_t& bits) {

	uint8_t distanceToNext = -1;
	uint8_t nextBits = 0;


	for (uint8_t index=0; index<numbChannels; index++) {


		// new relative toggle distances
		currentCompareValues[index] -= time;
		if (currentCompareValues[index] <= 0) currentCompareValues[index] += compareValues[index];


		// find nearest and bits to flip at next position
		if (currentCompareValues[index] < distanceToNext) {
			nextBits = (1<<index);
			distanceToNext = currentCompareValues[index];
		}

		if (currentCompareValues[index] == distanceToNext) {
			nextBits |= (1<<index);
		}


	}


	// return
	time = distanceToNext;
	bits = nextBits;

}
void MultiChannelOscillator::printSeries() {

	// stop isr
	TIMSK1 = 0;

	const uint16_t steps = 100;
	uint32_t totalTime = 0;
	uint16_t numbFlips=0;

	// load first
	uint8_t timeStep = compareValues[numbChannels-1];
	uint8_t bits = (1<<(numbChannels-1));

	// iterate through steps
	for (uint16_t index = 0; index<steps; index++) {

		totalTime += timeStep;
		numbFlips++;

		Serial.print("Time: "); Serial.println(timeStep);
		Serial.print("Flip: "); Serial.println(bits,BIN);
		Serial.println();

		calcNextToggle(timeStep,bits);
	}

	Serial.print(numbFlips); Serial.print (" Flips in "); Serial.print(totalTime); Serial.println(" ISR intervalls");


	TIMSK1 = (1<<OCIE1A);


}

void MultiChannelOscillator::fillBuffer() {

	Serial.println(eventBufferTime[0]);


	while(fillCount<eventBufferSize-1) {

		uint8_t tmpTime = eventBufferTime[eventBufferWriteIndex];
		uint8_t bits;
		calcNextToggle(tmpTime,bits);

		eventBufferWriteIndex++;
		if (eventBufferWriteIndex==eventBufferSize) eventBufferWriteIndex = 0;

		eventBufferTime[eventBufferWriteIndex] = tmpTime;
		eventBufferBits[eventBufferWriteIndex+1] = bits;

		fillCount++;

	}



}

void MultiChannelOscillator::printBuffer() {
	for (uint8_t index=0; index<eventBufferSize; index++) {
		Serial.print(index); Serial.print(" : "); Serial.println(eventBufferBits[index],BIN);
		Serial.print("    "); Serial.println(eventBufferTime[index]);
	}
}


void MultiChannelOscillator::isr() {

	bit_toggle(PIN);

	eventBufferReadIndex++;
	if (eventBufferReadIndex == eventBufferSize) eventBufferReadIndex = 0;

	fillCount--;
	if (fillCount == 0) stop();

	outputPort = eventBufferBits[eventBufferReadIndex];
	OCR1A = eventBufferTime[eventBufferReadIndex];




}


MultiChannelOscillator oscil;

//max 64 cycles = 4us
ISR(TIMER1_COMPA_vect) {

	oscil.isr();

}



