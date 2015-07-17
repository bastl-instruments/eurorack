/* 
	
pgHW.cpp library
this library is to be used with standuino MicroGranny 2.0 hardware

documentation of the library you find here:
pgHW library

Created by Vaclav Pelousek 2013 www.pelousek.eu
for Standuino wwww.standuino.eu

*/


#include "Arduino.h"
#include <avr/pgmspace.h>
#include <shiftRegisterFast.h>
//#include <portManipulations.h>
#include "pgHW.h"


#define SHIFTREGISTER_SER  D,5
#define SHIFTREGISTER_RCK  D,6
#define SHIFTREGISTER_SRCK D,7

  #define LATCH_PIN 7
#define CLOCK_PIN 6
#define DATA_PIN 5

#define BUTTON_1_PIN 14
#define BUTTON_2_PIN 18
#define BUTTON_3_PIN 16
#define BUTTON_4_PIN 15
#define BUTTON_5_PIN 8
#define BUTTON_6_PIN 9

 #define BUT_PIN_BAM C,0
 #define BUT_PIN_BAM_2 C,2
 #define BUT_PIN_BAM_3 D,1
 #define BUT_PIN_BAM_4 C,1
  #define BUT_PIN_BAM_5 B,0
   #define BUT_PIN_BAM_6 B,1
   
   #define KNOB_PIN_1 3
#define KNOB_PIN_2 4
#define CV_PIN 5

#define NUMBER_OF_KNOBS 2
  
#define DISPLAY 0
#define LEDS 1

//const unsigned char row[5]={ ROW_1_PIN,ROW_2_PIN,ROW_3_PIN,ROW_4_PIN,ROW_5_PIN };
const unsigned char rgbPin[3]={LED_R_PIN,LED_G_PIN,LED_B_PIN};

const uint8_t PROGMEM  KnobPin[3]={KNOB_PIN_1,KNOB_PIN_2, CV_PIN};


/*
Segments
 -  A
 F / / B
 -  G
 E / / C
 -  D
 */
 
const unsigned char segments[8]={
  SEG_A,SEG_B,SEG_C,SEG_D,SEG_E,SEG_F,SEG_G,SEG_DOT};
  
  
#define BLACK_BITS 0
#define RED_BITS 1
#define GREEN_BITS 2
#define BLUE_BITS 4
#define YELLOW_BITS 3
#define MAGENTA_BITS 5
#define CIAN_BITS 6
#define WHITE_BITS 7


const uint8_t PROGMEM  ColorBit[NUMBER_OF_COLORS] PROGMEM = {

BLACK_BITS, RED_BITS,GREEN_BITS,BLUE_BITS,YELLOW_BITS,MAGENTA_BITS,CIAN_BITS,WHITE_BITS

};
enum BitOrder {LSB_FIRST,MSB_FIRST};
const uint8_t PROGMEM Typo[53]={
  B00111111, //0
  B00000110, //1
  B01011011, //2
  B01001111, //3
  B01100110, //4
  B01101101, //5
  B01111101, //6
  B00000111, //7
  B01111111, //8
  B01101111, //9
  B01110111, //A 10
  B01111100, //b
  B01011000, //c
  B01011110, //d
  B01111001, //e
  B01110001, //f
  B00111101, //g
  B01110100, //h
  B00000100, //i
  B00011110, //j 
  B01110000, //k 20
  B00111000, //l
  B01010101, //m
  B01010100, //n
  B01011100, //o 
  B01110011, //p 25
  B01100111, //q
  B01010000, //r
  B01101101, //s //tu memit 
  B01111000, //t 
  B00011100, //u 30
  B00001100, //v 31
  B01101010, //w
  B01001001, //x 
  B01110010, //y 
  B01011011, //z tu menit 35
  B00000000, // void 36
  B01000000, //37
  B01001001, //38
  B01010010, //39 slash
  
  B00010000, //40
  B00011000, //41
  B00001000, //42
  B00001100, //43
  B00000100, //44
  B01000100, //45
  
  B01000000, //46
  
  B01100000, //47
  B00100000, //48
  B00100001, //49
  B00000001, //50
  B00000011, //51
  B00000010, //52
};

/*
Segments
 -  A
 F / / B
 -  G
 E / / C
 -  D
 */
 


pgHW::pgHW(){

}

//############# ROUTINE FUNCTIONS #############

void pgHW::initialize(){ 
	pinMode(DATA_PIN,OUTPUT);
	pinMode(CLOCK_PIN,OUTPUT);
	pinMode(LATCH_PIN,OUTPUT);
	pinMode(BUTTON_1_PIN,INPUT_PULLUP);
	pinMode(BUTTON_2_PIN,INPUT_PULLUP);
	pinMode(BUTTON_3_PIN,INPUT_PULLUP);
	pinMode(BUTTON_4_PIN,INPUT_PULLUP);
	pinMode(BUTTON_5_PIN,INPUT_PULLUP);
	pinMode(BUTTON_6_PIN,INPUT_PULLUP);
	//pinMode(A2,OUTPUT);

}

void pgHW::update(){
	updateKnobs();
	updateButtons();
	updateDisplay(); 
}



//############# KNOB RELATED FUNCTIONS #############
/*
void pgHW::setFreezeType(unsigned char _TYPE){

//	if(_TYPE==DEFAULT) unfreezeExternaly=false;
//	else if(_TYPE==UNFREEZE_EXTERNALY) unfreezeExternaly=true;

}
*/
//update values and hashes of knobs

 
  
  
void pgHW::updateKnobs(){

	knobChangedHash = 0;
  for (int i = 0; i < 3; i++) {
  	lastKnobValues[i]=knobValues[i];
  	
    int newValue = analogRead(pgm_read_word_near(KnobPin + i));
    int distance = abs(newValue - knobValues[i]); 
    
    
   	if (abs(newValue - knobValues[i]) > KNOB_TOLERANCE) {
      bitWrite(knobChangedHash, i, true);
    }
    else bitWrite(knobChangedHash, i, false);
   // lastKnobValues[i]=knobValues[i];
    knobValues[i] = newValue;
    //delay(10); ///
    updateDisplay();
  }
  
}


//returns the freezing state of knob	
boolean pgHW::knobFreezed(unsigned char _KNOB){ 
	return bitRead(knobFreezedHash,_KNOB);
}

boolean pgHW::knobMoved(unsigned char _KNOB){ 
	return bitRead(knobChangedHash,_KNOB);
}

// freeze all knobs
void pgHW::freezeAllKnobs(){ 
	for(int i=0;i<NUMBER_OF_KNOBS;i++){
		bitWrite(knobFreezedHash,i,true);
	}
}

// unfreeze all knobs
void pgHW::unfreezeAllKnobs(){ 
	for(int i=0;i<NUMBER_OF_KNOBS;i++){
		bitWrite(knobFreezedHash,i,false);
	}
}


// freeze one knob
void pgHW::freezeKnob(unsigned char _KNOB){ 
	bitWrite(knobFreezedHash,_KNOB,true);
}

// unfreeze one knob
void pgHW::unfreezeKnob(unsigned char _KNOB){ 
		bitWrite(knobFreezedHash,_KNOB,false);
}


// get knob value
int pgHW::knobValue(unsigned char _KNOB){ 

	return knobValues[_KNOB];

}

int pgHW::getCvValue(){
	return knobValues[2];
}

// get last knob value
int pgHW::lastKnobValue(unsigned char _KNOB){ 

	return lastKnobValues[_KNOB];

}

void pgHW::setLastKnobValue(unsigned char _KNOB,int _val){ 

	lastKnobValues[_KNOB]=_val;

}

//############# LED RELATED FUNCTIONS #############

// write the values from the hash to the pins
//bitRead(ledStateHash,i)


// set state of led

void pgHW::setLed(unsigned char _LED,boolean _STATE){ 
	if(_LED>=6) bitWrite(displayBuffer[LEDS],_LED,!_STATE);
	else bitWrite(displayBuffer[LEDS],_LED,_STATE);


}

void pgHW::setColor(unsigned char _COLOR){ 

	unsigned char _bits=pgm_read_word_near(ColorBit + _COLOR)	;
	
	for(int i=0;i<3;i++){
		setLed(rgbPin[i],bitRead(_bits,i));
	}

}

void pgHW::displayText(char *text){
 // for(int i=0;i<NUMBER_OF_DIGITS;i++) displayChar(text[i],i);
}

void pgHW::displayChar(char whichChar) {
  if(whichChar==32)
  { 
    lightNumber(VOID);
  }
  else if(whichChar<65){
    lightNumber(whichChar-48);
  }
  else if(whichChar<97){
    lightNumber(whichChar-65+10);
  }
  else {
    lightNumber(whichChar-97+10);
  }
  
}


void pgHW::lightNumber(int numberToDisplay) {

  for(int i=0;i<7;i++){
    bitWrite(displayBuffer[DISPLAY],segments[i],bitRead(pgm_read_word_near(Typo+numberToDisplay),i)); 
  }

}

void pgHW::displayNumber(int _number){
 
 // lightNumber(_number/100,1);
  //lightNumber((_number%100)/10,2);
  lightNumber(_number%10);

}

void pgHW::setDot( boolean _state){
	bitWrite(displayBuffer[DISPLAY],SEG_DOT,_state);
}

//############# BUTTON RELATED FUNCTIONS #############
/*
void pgHW::updateButtons(){ 
updateButtons();
}
*/
void pgHW::updateDisplay(){
//inline void updateDisplay(){
/*
shiftRegFast::write_8bit(~displayBuffer[1]);
shiftRegFast::write_8bit(~displayBuffer[0]);
shiftRegFast::enableOutput();
*/
if(displayBuffer[0]!=lastDisplayBuffer[0] || displayBuffer[1]!=lastDisplayBuffer[1]){
	digitalWrite(LATCH_PIN,LOW); 
    shiftOut(DATA_PIN,CLOCK_PIN,MSBFIRST,~displayBuffer[0]);//analogRead(4)>>_row);//);
    shiftOut(DATA_PIN,CLOCK_PIN,MSBFIRST,~displayBuffer[1]);
    digitalWrite(LATCH_PIN,HIGH);
    }
    lastDisplayBuffer[0]=displayBuffer[0];
    lastDisplayBuffer[1]=displayBuffer[1];

}




// updates all button related hashes
void pgHW::updateButtons(){  

	pinMode(BUTTON_1_PIN,INPUT_PULLUP);
	pinMode(BUTTON_2_PIN,INPUT_PULLUP);
	pinMode(BUTTON_3_PIN,INPUT_PULLUP);
	pinMode(BUTTON_4_PIN,INPUT_PULLUP);
	pinMode(BUTTON_5_PIN,INPUT_PULLUP);
	pinMode(BUTTON_6_PIN,INPUT_PULLUP);

	bitWrite(buttonStateHash,0,!bit_read_in(BUT_PIN_BAM));//bit_read_in(BUT_PIN)); 
	bitWrite(buttonStateHash,1,!bit_read_in(BUT_PIN_BAM_2));
	bitWrite(buttonStateHash,2,!bit_read_in(BUT_PIN_BAM_3));
	bitWrite(buttonStateHash,3,!bit_read_in(BUT_PIN_BAM_4));
	bitWrite(buttonStateHash,4,bit_read_in(BUT_PIN_BAM_5));
	bitWrite(buttonStateHash,5,bit_read_in(BUT_PIN_BAM_6));
		
//	justReleasedHash=0;
//    justPressedHash=0;
    
	for(int i=0;i<6;i++){ // first read the buttons and update button states
	bitWrite(justPressedHash,i,false); 
	bitWrite(justReleasedHash,i,false);
	if(bitRead(buttonStateHash,i)==true && bitRead(lastButtonStateHash,i)==false)  bitWrite(justPressedHash,i,true);
	if(bitRead(buttonStateHash,i)==false && bitRead(lastButtonStateHash,i)==true)  bitWrite(justReleasedHash,i,true);
	bitWrite(lastButtonStateHash,i,bitRead(buttonStateHash,i));  
	}
		
		
	
 
}



//returns current state of a button
boolean pgHW::buttonState(unsigned char _BUTTON){ 
	return bitRead(buttonStateHash,_BUTTON);
}

//returns true if the button was just pressed
boolean pgHW::justPressed(unsigned char _BUTTON){

	return bitRead(justPressedHash,_BUTTON);

}

//returns true if the button was just released
boolean pgHW::justReleased(unsigned char _BUTTON){
	return bitRead(justReleasedHash,_BUTTON);
}



//flips the software switch
void pgHW::flipSwitch(unsigned char _SWITCH){ 
	bitWrite(switchStateHash,_SWITCH,!bitRead(switchStateHash,_SWITCH));
}

// sets switch state
void pgHW::setSwitch(unsigned char _SWITCH, boolean _STATE){ 
	bitWrite(switchStateHash,_SWITCH,_STATE);
}

//returns switch state
boolean pgHW::switchState(unsigned char _SWITCH){ 
	return bitRead(switchStateHash,_SWITCH);
}

//resetsSwitches
void pgHW::resetSwitches(){
	for(int i=0;i<NUMBER_OF_BUTTONS;i++){
		bitWrite(switchStateHash,i,false);
	}
}

//use switch states as bits of one number - sound
unsigned char pgHW::soundFromSwitches(){
	unsigned char val=0;
	for(int i=0;i<4;i++){
		bitWrite(val,i,bitRead(switchStateHash,i));
	}
	return val;

}

//use button states as bits of one number - sound
unsigned char pgHW::soundFromButtons(){
	unsigned char val=0;
	for(int i=0;i<4;i++){
		bitWrite(val,i,bitRead(buttonStateHash,i));
	}
	return val;
}
