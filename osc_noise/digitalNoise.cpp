/*
 * digitalNoise.cpp
 *
 *  Created on: 14.12.2014
 *      Author: user
 */


#include "digitalNoise.h"


void digitalNoise::init() {
	bit_dir_outp(DIGINOISE_PIN);
	bit_set(DIGINOISE_PIN);

	TCCR0A = (1<<WGM01);
	TCCR0B = 0;
	TIMSK0 = 0;

}


void digitalNoise::setTopFreq(uint16_t freq) {
	uint16_t totalCompare = F_CPU/freq/2;

	uint16_t minimumPrescaler = totalCompare>>8;


	uint16_t prescaler = 1;

	if (minimumPrescaler>=1) prescaler = 8;
	if (minimumPrescaler>=8) prescaler = 64;
	if (minimumPrescaler>=64) prescaler = 256;
	if (minimumPrescaler>=256) prescaler = 1024;

	switch (prescaler) {
	case 1: TCCR0B = 1; break;
	case 8: TCCR0B = 2; break;
	case 64: TCCR0B = 3; break;
	case 256: TCCR0B = 4; break;
	case 1024: TCCR0B = 5; break;
	default: break;
	}

	OCR0A = totalCompare/prescaler;
}

void digitalNoise::checkForBitFlip() {
	if (TIFR0 & (1<<OCF0A)) {
		if (bastlRandom::byte()>127) bit_toggle(DIGINOISE_PIN);
		TIFR0 |= (1<<OCF0A);
	}

}

