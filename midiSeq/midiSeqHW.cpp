
/*** PERMANENT SETTINGS***/

#include "midiSeqHW_settings.h"


/*** ACTUAL CODE ***/

#include <Arduino.h>
#include "midiSeqHW.h"
#include <shiftRegisterFast.h>
#include <avr/pgmspace.h>
#include <fastAnalogRead.h>
//#define PIN B,5
#define BUTTON_JUMP_PIN C,4

#define KNOB_MOVED_TOLERANCE 1
#define GATE_BYTE 0
#define GATE_BIT_1 7
#define GATE_BIT_2 6
#define GATE_BIT_3 5
#define GATE_BIT_4 4
#define GATE_BIT_5 3
#define GATE_BIT_6 2
#define GATE_BIT_7 1
#define GATE_BIT_8 0


const uint8_t gateBit[8]={GATE_BIT_1,GATE_BIT_2,GATE_BIT_3,GATE_BIT_4, GATE_BIT_5,GATE_BIT_6,GATE_BIT_7,GATE_BIT_8};

void midiSeqHW::setGate(uint8_t _gate,bool _state){
	bitWrite(shiftHash[GATE_BYTE],7-gateBit[_gate],_state);
}

#define LED_BYTE 1
#define LED_BIT_1 6
#define LED_BIT_2 4
#define LED_BIT_3 2
#define LED_BIT_4 0
#define LED_BIT_5 7
#define LED_BIT_6 5
#define LED_BIT_7 3
#define LED_BIT_8 1

const uint8_t ledBit[8]={LED_BIT_1,LED_BIT_2,LED_BIT_3,LED_BIT_4, LED_BIT_5,LED_BIT_6,LED_BIT_7,LED_BIT_8};

void midiSeqHW::setLed(uint8_t _led,bool _state){
	bitWrite(shiftHash[LED_BYTE],7-ledBit[_led],_state);
}

#define BI_LED_A_BYTE 3

#define BI_LED_A_1_C 5
#define BI_LED_A_1_A 4
#define BI_LED_A_2_C 2
#define BI_LED_A_2_A 3
#define BI_LED_A_3_C 1
#define BI_LED_A_3_A 0




#define BI_LED_B_BYTE 2
#define BI_LED_B_1_C 5
#define BI_LED_B_1_A 4
#define BI_LED_B_2_C 1
#define BI_LED_B_2_A 0
#define BI_LED_B_3_C 2
#define BI_LED_B_3_A 3

#define GATE_OUT_BYTE 2
#define GATE_OUT_BIT 7

#define SHIFT_LED_BYTE 2
#define SHIFT_LED_BIT 6

const uint8_t biLedBit[12]= { BI_LED_A_1_C , BI_LED_A_1_A , BI_LED_A_2_C , BI_LED_A_2_A , BI_LED_A_3_C , BI_LED_A_3_A , BI_LED_B_1_C , BI_LED_B_1_A , BI_LED_B_2_C , BI_LED_B_2_A , BI_LED_B_3_C , BI_LED_B_3_A};



void midiSeqHW::setGateOut(bool _state){
	bitWrite(shiftHash[GATE_OUT_BYTE],GATE_OUT_BIT,_state);
}

void midiSeqHW::setShiftLed(bool _state){
	bitWrite(shiftHash[SHIFT_LED_BYTE],SHIFT_LED_BIT,_state);
}

void midiSeqHW::selectKnob(uint8_t _knob){
	selectedKnob=knobBit[_knob];
}


//using namespace fastAnalogRead;

//#include <SdFat.h>

// Declaration of instance (for use in interrupt service routine)
midiSeqHW hw;


#define UINT16_MAX 65535
#define MAX_ADDR 131067

void midiSeqHW::init(void(*buttonChangeCallback)(uint8_t number),void(*clockInCallback)(uint8_t number)) {

	cli();

	shiftRegFast::setup();

	bit_dir_outp(AMUX_SELECT_0);
	bit_dir_outp(AMUX_SELECT_1);
	bit_dir_outp(AMUX_SELECT_2);

	bit_dir_inp(BUTTON_PIN);

	bit_set(BUTTON_PIN);

	bit_dir_inp(MIXED_PIN);
	bit_set(MIXED_PIN);
	bit_dir_inp(BUTTON_JUMP_PIN);
	bit_set(BUTTON_JUMP_PIN);

	bit_dir_outp(PIN);

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

void midiSeqHW::printLEDStates() {

}


void midiSeqHW::setLED(uint8_t number, bool state) {

}



/**** BUTTONS ****/
#define TRIG_A 3
#define TRIG_B 4
#define RST 0
#define SHIFT_B 5


uint16_t midiSeqHW::getCV(){
	return mixedValues[0];
}


void midiSeqHW::isr_updateButtons() {

	uint8_t i=buttonSelect;//buttonBit[buttonSelect];
//isAnalog[i]=true;

	fastAnalogRead::connectChannel(MIXED_CHANNEL);
	fastAnalogRead::startConversion();
	while(!fastAnalogRead::isConversionFinished()){} ;
	            //=fastAnalogRead::getConversionResult();


	bool newState=!bit_read_in(BUTTON_PIN);
	if(buttonChangeCallback!=0){
		if(newState != bitRead(buttonHash,buttonBit[i])){
			bitWrite(buttonHash,buttonBit[i],newState);
			buttonChangeCallback(buttonBit[i]);
		}
	}


	buttonSelect++;
	if(buttonSelect>7) buttonSelect=0;
	i=buttonSelect;//buttonBit[buttonSelect];

}
bool midiSeqHW::buttonState(uint8_t _but){
	return bitRead(buttonHash,_but);//buttonBit[_but]);
}
bool midiSeqHW::justPressed(uint8_t _but){
	return bitRead(justPressedHash,buttonBit[_but]);
}
bool midiSeqHW::justReleased(uint8_t _but){
	return bitRead(justReleasedHash,buttonBit[_but]);
}

void midiSeqHW::compareButtonStates(){
	if(buttonChangeCallback!=0){

		for(int i=0;i<2;i++){
			if(buttonStates[i]!=newButtonStates[i]) buttonStates[i]=newButtonStates[i],buttonChangeCallback(i);
		}
	}
}


bool midiSeqHW::getButtonState(uint8_t number) {
	return buttonStates[number];
}




void midiSeqHW::setBit(uint8_t _bit, bool _value){
	bitWrite(shiftHash[_bit/4],_bit%8,_value);
}

void midiSeqHW::dimLed(uint8_t _led, bool _state){
	bitWrite(ledDimHash,7-ledBit[_led],_state);
}

void midiSeqHW::isr_updateTriggerStates(){

	shiftRegFast::write_8bit(biLedHashA);//shiftHash[3]);
	shiftRegFast::write_8bit(biLedHashB);//shiftHash[2]);
	shiftRegFast::write_8bit(ledOutHash);
	shiftRegFast::write_8bit(shiftHash[0]);

	shiftRegFast::enableOutput();

}
void midiSeqHW::resetTriggers(){
	for(int i=0;i<6;i++) triggerCountdown[i]=0;
	trigState=0;
	//shiftRegFast::write_8bit(trigState);
	//shiftRegFast::enableOutput();

}
void midiSeqHW::isr_updateClockIn(){
	if(clockInCallback!=0){
		static bool clockInState[2];
		bool newState=bit_read_in(INPUT_1);
		if(newState && !clockInState[0]) clockInCallback(0);
		clockInState[0]=newState;

		newState=bit_read_in(INPUT_2);
		if(newState && !clockInState[1]) clockInCallback(1);
		clockInState[1]=newState;
	}
}

/**** TIMING ****/

uint32_t midiSeqHW::getElapsedBastlCycles() {
	return bastlCycles;
}

uint16_t midiSeqHW::getBastlCyclesPerSecond() {
	return (F_CPU/1024)/OCR2A;
}


/**** INTERRUPT ****/

ISR(TIMER2_COMPA_vect) { //56us :)


	//bit_set(PIN);
	hw.incrementBastlCycles();
	//hardware.isr_updateClockIn();
	hw.isr_updateTriggerStates();
	hw.isr_updateButtons();      // ~1ms
	//hardware.isr_updateNextLEDRow();   // ~84us
	//bit_clear(PIN);


}


