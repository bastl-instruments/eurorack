
/*** PERMANENT SETTINGS***/

#include "osc_noiseHW_settings.h"


/*** ACTUAL CODE ***/

#include <Arduino.h>
#include "osc_noiseHW.h"
#include <shiftRegisterFast.h>
#include <avr/pgmspace.h>
#include <fastAnalogRead.h>
#define PIN B,5
#define KNOB_MOVED_TOLERANCE 1
//using namespace fastAnalogRead;

//#include <SdFat.h>

// Declaration of instance (for use in interrupt service routine)
osc_noiseHW hardware;


#define UINT16_MAX 65535
#define MAX_ADDR 131067

unsigned char analogPin[6]={
  ANALOG_PIN_1, ANALOG_PIN_2, ANALOG_PIN_3,  ANALOG_PIN_4, ANALOG_PIN_5, ANALOG_PIN_6};


//prog_uchar colorBit[NUMBER_OF_COLORS] PROGMEM = {


void osc_noiseHW::init(void(*buttonChangeCallback)(uint8_t number),void(*clockInCallback)(uint8_t number)) {

	cli();

	shiftRegFast::setup();




	//bit_dir_inp(INPUT_1);



	//bit_dir_outp(LED_R_PIN);

	bit_dir_outp(PIN_0);
	bit_dir_outp(PIN_1);
	bit_dir_outp(PIN_2);
	bit_dir_outp(PIN_3);
	bit_dir_outp(PIN_4);
	bit_dir_outp(PIN_5);
	bit_dir_outp(PIN_6);
	bit_dir_outp(PIN_7);
	bit_dir_outp(PIN_8);
	bit_dir_outp(PIN_9);
	bit_dir_outp(PIN_10);

	bit_dir_inp(PIN_11);
	bit_dir_inp(PIN_12);
	bit_dir_inp(PIN_13);
	bit_dir_inp(PIN_14);

	bit_set(PIN_11);
	bit_set(PIN_12);
	bit_set(PIN_13);
	bit_set(PIN_14);


	// store callback pointer for changed buttons
	 this->buttonChangeCallback = buttonChangeCallback;

	 this->clockInCallback = clockInCallback;

	// Disable Timer1 interrupt
	//TIMSK1 &= ~_BV(TOIE1);

	// TIMER 2
	TCCR2A = (1 << WGM21);  // turn on CTC mode
	TIMSK2 |= (1 << OCIE2A);// enable interrupt
	TCCR2B = B00000111;	    //prescaler = 1024
	OCR2A = (F_CPU/1024)/(updateFreq);
	TCNT2  = 0;

	fastAnalogRead::init(); // for analog read
	knobCount=0;
	fastAnalogRead::connectChannel(knobCount);
	fastAnalogRead::startConversion();
	knobFreezeHash=255;
	knobMovedHash=0;


	sei();
}


/**** LEDS ****/






/**** BUTTONS ****/


void osc_noiseHW::isr_updateButtons() {

/*
	newButtonStates[0]=bit_read_in(BUTTON_PIN_1);
	newButtonStates[1]=bit_read_in(BUTTON_PIN_2);

	compareButtonStates();
*/
}
/*
void osc_noiseHW::compareButtonStates(){
	if(buttonChangeCallback!=0){

		for(int i=0;i<2;i++){
			if(buttonStates[i]!=newButtonStates[i]) buttonStates[i]=newButtonStates[i],buttonChangeCallback(i);
		}

	}
	//for(int i=0;i<2;i++) ;


}
*/


IHWLayer::ButtonState osc_noiseHW::getButtonState(uint8_t number) {

	if (buttonStates[number] ) {
		return IHWLayer::UP;
	} else {
		return IHWLayer::DOWN;
	}

}


/**** TRIGGER ****/
void osc_noiseHW::setTrigger(uint8_t number, osc_noiseHW::TriggerState state, uint16_t pulseWidth){
	triggerCountdown[number]=pulseWidth;
	if(state==ON) bitWrite(trigState,number,1);
	if(state==OFF) bitWrite(trigState,number,0);
}

bool osc_noiseHW::getTriggerState(uint8_t number){
		return bitRead(trigState,number);
};

uint8_t osc_noiseHW::getKnobValue(uint8_t index){
	return knobValues[index];
}

void osc_noiseHW::isr_updateKnobs(){
	if(fastAnalogRead::isConversionFinished()){
	lastKnobValues[knobCount]=knobValues[knobCount];
	knobValues[knobCount]=fastAnalogRead::getConversionResult()>>2;
	int difference=(int)lastKnobValues[knobCount]-knobValues[knobCount];
	difference=abs(difference);
	if(difference>KNOB_MOVED_TOLERANCE) bitWrite(knobMovedHash,knobCount,1), unfreezeKnob(knobCount);
	else bitWrite(knobMovedHash,knobCount,0);
	if(inBetween(knobFreezeValues[knobCount],lastKnobValues[knobCount],knobValues[knobCount])) unfreezeKnob(knobCount);
	knobCount++;
	if(knobCount>=6) knobCount=0;
	fastAnalogRead::connectChannel(analogPin[knobCount]);
	fastAnalogRead::startConversion();
	}
}
void osc_noiseHW::freezeAllKnobs(){
	knobFreezeHash=255;
}
void osc_noiseHW::freezeKnob(uint8_t index){
	bitWrite(knobFreezeHash,index,1);
}
void osc_noiseHW::freezeKnob(uint8_t index, uint8_t value){
	knobFreezeValues[index]=value;
	bitWrite(knobFreezeHash,index,1);
}
void osc_noiseHW::unfreezeKnob(uint8_t index){
	bitWrite(knobFreezeHash,index,0);
}
bool osc_noiseHW::knobFreezed(uint8_t index){
	return bitRead(knobFreezeHash,index);
}
bool osc_noiseHW::knobMoved(uint8_t index){
	return bitRead(knobMovedHash,index);
}

bool osc_noiseHW::inBetween(uint8_t value, uint8_t border1, uint8_t border2){
	if(value<=border1 && value>=border2) return true;
	else if(value>=border1 && value<=border2) return true;
	else return false;
}
void osc_noiseHW::isr_updateTriggerStates(){

	//shiftRegFast::write_8bit(trigState);
	//shiftRegFast::enableOutput();

	for(int i=0;i<NUMBER_OF_OUTPUT_PINS;i++){
		setPin(i,bitRead(trigState,i));
	//	bit_set(pin[i]);
		if(triggerCountdown[i]>0){
			if(triggerCountdown[i]==1) setTrigger(i,OFF,0);
			triggerCountdown[i]--;
		}
	}

}
void osc_noiseHW::setPin(uint8_t pin, bool state){
	switch(pin){
	case 0:
		if(state) bit_set(PIN_0);
		else bit_clear(PIN_0);
		break;
	case 1:
			if(state) bit_set(PIN_1);
			else bit_clear(PIN_1);
			break;
	case 2:
			if(state) bit_set(PIN_2);
			else bit_clear(PIN_2);
			break;
	case 3:
			if(state) bit_set(PIN_3);
			else bit_clear(PIN_3);
			break;
	case 4:
			if(state) bit_set(PIN_4);
			else bit_clear(PIN_4);
			break;
	case 5:
			if(state) bit_set(PIN_5);
			else bit_clear(PIN_5);
			break;
	case 6:
			if(state) bit_set(PIN_6);
			else bit_clear(PIN_6);
			break;
	case 7:
			if(state) bit_set(PIN_7);
			else bit_clear(PIN_7);
			break;
	case 8:
			if(state) bit_set(PIN_8);
			else bit_clear(PIN_8);
			break;

	case 9:
			if(state) bit_set(PIN_9);
			else bit_clear(PIN_9);
			break;
	case 10:
			if(state) bit_set(PIN_10);
			else bit_clear(PIN_10);
			break;
	case 11:
			if(state) bit_set(PIN_11);
			else bit_clear(PIN_11);
			break;
	case 12:
				if(state) bit_set(PIN_12);
				else bit_clear(PIN_12);
				break;
	case 13:
				if(state) bit_set(PIN_13);
				else bit_clear(PIN_13);
				break;
	case 14:
				if(state) bit_set(PIN_14);
				else bit_clear(PIN_14);
				break;
	case 15:
				if(state) bit_set(PIN_15);
				else bit_clear(PIN_15);
				break;

	}

}
void osc_noiseHW::resetTriggers(){
	for(int i=0;i<6;i++) triggerCountdown[i]=0;
	trigState=0;
	//shiftRegFast::write_8bit(trigState);
	//shiftRegFast::enableOutput();

}
/*
void osc_noiseHW::isr_updateClockIn(){
	if(clockInCallback!=0){
		static bool clockInState[2];
		bool newState=!bit_read_in(INPUT_1);
		if(newState && !clockInState[0]) clockInCallback(0);
		clockInState[0]=newState;

		newState=!bit_read_in(INPUT_2);
		if(newState && !clockInState[1]) clockInCallback(1);
		clockInState[1]=newState;
	}
}
*/
/**** TIMING ****/

uint32_t osc_noiseHW::getElapsedBastlCycles() {
	return bastlCycles;
}

uint16_t osc_noiseHW::getBastlCyclesPerSecond() {
	return (F_CPU/1024)/OCR2A;
}


/**** INTERRUPT ****/

ISR(TIMER2_COMPA_vect) { //56us :)


//	bit_set(PIN_0);
	hardware.incrementBastlCycles();
//	hardware.isr_updateClockIn();
	hardware.isr_updateKnobs();
	hardware.isr_updateTriggerStates();
	//hardware.isr_updateButtons();      // ~1ms
	//hardware.isr_updateNextLEDRow();   // ~84us

	//bit_clear(PIN_0);


}


