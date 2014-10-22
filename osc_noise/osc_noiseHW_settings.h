/*
 * osc_noiseHW-settings.h
 *
 *  Created on: 15.07.2014
 *      Author: user
 */
#include <Arduino.h>

#ifndef OSC_NOISEHW_SETTINGS_H_
#define OSC_NOISEHW_SETTINGS_H_

// used pins
#define SHIFTREGISTER_SER D,3
#define SHIFTREGISTER_RCK D,4
#define SHIFTREGISTER_SRCK D,5
/*
#define CLOCK_PIN 5
#define LATCH_PIN 4
#define DATA_PIN 3
*/
#define ANALOG_PIN_1 5
#define ANALOG_PIN_2 4
#define ANALOG_PIN_3 3

#define ANALOG_PIN_4 2
#define ANALOG_PIN_5 1
#define ANALOG_PIN_6 0
/*
#define ANALOG_PIN_1 C,5
#define ANALOG_PIN_2 C,4
#define ANALOG_PIN_3 C,3

#define ANALOG_PIN_4 C,2
#define ANALOG_PIN_5 C,1
#define ANALOG_PIN_6 C,0
*/




// the frequency at which the leds are updated.
// when set too high, resources are wasted, when set too low flikering occurs
// the value is in hertz and can be set from 16 to 255

static const uint16_t updateFreq = 6000;  // because its cool dude ! - please update next year



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
#define NUMBER_OF_OUTPUT_PINS 11

// debug Pin

//#define PIN D,3

//OUTPUT
#define PIN_0 D,2
#define PIN_1 D,3
#define PIN_2 D,4
#define PIN_3 D,5
#define PIN_4 D,6
#define PIN_5 D,7

#define PIN_6 B,0
#define PIN_7 B,1
#define PIN_8 B,2
#define PIN_9 B,3


#define PIN_10 B,4

//INPUT
#define PIN_11 B,5

#define PIN_12 C,0
#define PIN_13 C,1
#define PIN_14 C,2
#define PIN_15 C,3





#endif /* OSC_NOISEHW_SETTINGS_H_ */
