/*
 * midiSeqHW-settings.h
 *
 *  Created on: 15.07.2014
 *      Author: user
 */
#include <Arduino.h>

#ifndef ANALOGSEQHW_SETTINGS_H_
#define ANALOGSEQHW_SETTINGS_H_

// used pins
#define SHIFTREGISTER_SER D,3
#define SHIFTREGISTER_RCK D,4
#define SHIFTREGISTER_SRCK D,2




// the frequency at which the leds are updated.
// when set too high, resources are wasted, when set too low flikering occurs
// the value is in hertz and can be set from 16 to 255

static const uint16_t updateFreq = 1500;  // because its cool dude ! - please update next year



// properties of the led blinking
// total defines the frequency of one blink cycle
// duty defines when the led state is switched
// the values are relative to the frequency, so when you want dimming instead of blinking, you will need to increase that value
//  -------
//        |
//        |
//        -------------
//  0    duty         total

#define blinkTotal 2
#define blinkDuty  1



//
#define BUFFER_DEPTH 40


// debug Pin
#define PIN D,8

#endif /* ANALOGSEQHW_SETTINGS_H_ */
