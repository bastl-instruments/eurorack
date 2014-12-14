/*
 * digitalNoise.h
 *
 *  Created on: 14.12.2014
 *      Author: user
 */

#ifndef DIGITALNOISE_H_
#define DIGITALNOISE_H_

#include <portManipulations.h>
#include <random.h>
#include <Arduino.h>

#define DIGINOISE_PIN B,5


class digitalNoise {
public:
	void init();
	void setTopFreq(uint16_t);
	void checkForBitFlip();
};




#endif /* DIGITALNOISE_H_ */
