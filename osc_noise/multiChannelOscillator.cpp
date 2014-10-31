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

	eventBufferWriteIndex=0;
	fillCount = 0;
	fillBuffer();

	// load ISR
	eventBufferReadIndex=0;

	sei();


}

inline void MultiChannelOscillator::incrementIndex(uint8_t& counter) {
	counter++;
	if (counter == eventBufferSize)  counter = 0;
}


inline void MultiChannelOscillator::calcNextToggle(uint8_t& time, uint8_t& bits) {

	// the last step that as been calculated
	static uint8_t lastTime = 0;

	uint8_t distanceToNext = -1;
	uint8_t nextBits = 0;


	for (uint8_t index=0; index<numbChannels; index++) {


		// new relative toggle distances
		currentCompareValues[index] -= lastTime;
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

	// keep old time step value for next run
	lastTime = distanceToNext;

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


	while(fillCount<eventBufferSize-1) {

		//bit_set(PIN);

		calcNextToggle(eventBufferTime[eventBufferWriteIndex],eventBufferBits[eventBufferWriteIndex]);

		incrementIndex(eventBufferWriteIndex);

		fillCount++;


	}

	//bit_clear(PIN);


}

void MultiChannelOscillator::printBuffer() {
	for (uint8_t index=0; index<eventBufferSize; index++) {
		Serial.print(index); Serial.print(" wait "); Serial.print(eventBufferTime[index]);
		Serial.print(" then "); Serial.println(eventBufferBits[index],BIN);
	}
}


void MultiChannelOscillator::isr() {

	static uint8_t track5;
	if (eventBufferBits[eventBufferReadIndex] & (1<<5)) {
		//if (fillCount == 9) bit_set(PIN);
		//else bit_clear(PIN);

	}



	outputPin = eventBufferBits[eventBufferReadIndex];

	incrementIndex(eventBufferReadIndex);

	OCR1A = eventBufferTime[eventBufferReadIndex];

	fillCount--;
	bit_toggle(PIN);
	bit_toggle(PIN);
	if (fillCount == 1) stop();



}


MultiChannelOscillator oscil;

//max 64 cycles = 4us
ISR(TIMER1_COMPA_vect) {

	oscil.isr();

}



