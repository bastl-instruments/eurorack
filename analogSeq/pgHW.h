/* 
	
pgHW.cpp library
this library is to be used with standuino MicroGranny hardware
MOZZI library compatible
documentation of the library you find here:
pgHW library

Created by Vaclav Pelousek 2013 www.pelousek.eu
for Standuino wwww.standuino.eu

*/

#ifndef pgHW_h
#define pgHW_h

#include "Arduino.h"
#include <avr/pgmspace.h>
#include <shiftRegisterFast.h>
#include <portManipulations.h>

#define DEFAULT 0
#define UNFREEZE_EXTERNALY 1

#define SEG_A 0 //Display pin 14
#define SEG_B 1 //Display pin 16
#define SEG_C 2 //Display pin 13
#define SEG_D 3 //Display pin 3
#define SEG_E 4 //Display pin 5
#define SEG_F 7 //Display pin 11
#define SEG_G 6 //Display pin 15
#define SEG_DOT 5

#define LED_R_PIN 5
#define LED_G_PIN 4
#define LED_B_PIN 3

#define LED_1_PIN 6
#define LED_2_PIN 7

//const unsigned char bigButton[NUMBER_OF_BIG_BUTTONS]={BIG_1,BIG_2,BIG_3,BIG_4,BIG_5,BIG_6};

#define KNOB_TOLERANCE 11
#define KNOB_FREEZE_DISTANCE 128

#define NUMBER_OF_KNOBS 2

#define NUMBER_OF_BUTTONS 6

//#define NUMBER_OF_DIGITS 4


#define FACTORY_CLEAR_PIN 2
#define FACTORY_CLEAR_SIGNAL_PIN 13


#define VOID 36
#define MINUS 37
#define LINES 38
#define SLASH 39

#define ZERO 0

//const unsigned char knobLed[NUMBER_OF_KNOBS]={KNOB_LED_1_PIN,KNOB_LED_2_PIN,KNOB_LED_3_PIN,KNOB_LED_4_PIN};

#define LED_R 9
#define LED_G 12
#define LED_B 8
#define NUMBER_OF_COLORS 8




#define BLACK 0
#define RED 1
#define GREEN 2
#define BLUE 3
#define YELLOW 4
#define MAGENTA 5
#define CIAN 6
#define WHITE 7




/*
Segments
 -  A
 F / / B
 -  G
 E / / C
 -  D
 */


	

class pgHW
{
  public: 
  
  
  	

    pgHW(); // constructor
	void initialize();
	void update();
	//void updateButtons();
	void dimForRecord(unsigned char _BUTTON);

    void setLed(unsigned char _LED, boolean _STATE);
    void setColor(unsigned char _COLOR);
    
    boolean buttonState(unsigned char _BUTTON); 
	boolean justPressed(unsigned char _BUTTON);
	boolean justReleased(unsigned char _BUTTON);
	
	boolean knobFreezed(unsigned char _KNOB);
	boolean knobMoved(unsigned char _KNOB);
	
	void freezeAllKnobs();
	void unfreezeAllKnobs();
	void freezeKnob(unsigned char _KNOB);
	void unfreezeKnob(unsigned char _KNOB);
	int getCvValue();
	int knobValue(unsigned char _KNOB);
	
	int lastKnobValue(unsigned char _KNOB);
	void setLastKnobValue(unsigned char _KNOB,int _val);
//	void setFreezeType(unsigned char _TYPE);
	
	
	void flipSwitch(unsigned char _SWITCH);
	void setSwitch(unsigned char _SWITCH, boolean _STATE);
	boolean switchState(unsigned char _SWITCH);
	void resetSwitches();
	
	void displayText(char *text);
	void displayChar(char whichChar) ;
	void lightNumber(int numberToDisplay);
	void displayNumber(int _number);
	void setDot( boolean _state);

	unsigned char soundFromSwitches();
	unsigned char soundFromButtons();
	
//	boolean factoryClear();
//	void factoryCleared();
	void updateKnobs();
	void updateButtons();
	void updateDisplay();
	
	int _row;
unsigned char displayBuffer[2];
unsigned char lastDisplayBuffer[2];


  private: 
  	
	int buttonStateHash;
	int lastButtonStateHash;
	int switchStateHash;
	int justPressedHash;
	int justReleasedHash;
	int ledStateHash;
	unsigned char knobFreezedHash;
	unsigned char knobChangedHash;
	int knobValues[3];
	int lastKnobValues[3];
	
	
	
	
	
	
	
	
	boolean mozzi;
	boolean unfreezeExternaly;

	
};

#endif