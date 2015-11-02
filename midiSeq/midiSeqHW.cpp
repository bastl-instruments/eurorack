
/*** PERMANENT SETTINGS***/

#include "midiSeqHW_settings.h"


/*** ACTUAL CODE ***/

#include <Arduino.h>
#include "midiSeqHW.h"
#include <shiftRegisterFast.h>
#include <avr/pgmspace.h>
//#include <fastAnalogRead.h>
midiSeqHW hw;

//#define PIN B,5
#define CLK_IN_PIN C,5


#define BUTTON_PIN B,0

#define BUTT_CO_B 0
#define BUTT_CO_D 1
#define BUTT_CO_LEFT 2
#define BUTT_CO_C 3
#define BUTT_CO_MODE 4
#define BUTT_CO_E 5
#define BUTT_CO_RIGHT 6
#define BUTT_CO_F 7

#define MUX_PIN_0 D,6
#define MUX_PIN_1 D,5
#define MUX_PIN_2 C,0

#define BUTTON_PIN_A D,7

#define BUTTON_PIN_B D,6
#define BUTTON_PIN_C D,5

#define BUTTON_PIN_D C,4
#define BUTTON_PIN_E C,3
#define BUTTON_PIN_F C,2

#define BUTTON_MODE C,0
#define BUTTON_LEFT B,0
#define BUTTON_RIGHT C,1

//const uint8_t buttonPin[9]={BUTTON_PIN_A,BUTTON_PIN_B,BUTTON_PIN_C,BUTTON_PIN_D,BUTTON_PIN_E,BUTTON_PIN_F,BUTTON_MODE,BUTTON_LEFT,BUTTON_RIGHT};



#define LED_GATE_A 16
#define LED_GATE_B 20
#define LED_GATE_C 22
#define LED_CLK 23

#define GATE_A 18
#define GATE_B 19
#define GATE_C 21

#define CLK 17

#define LED_BIT_A 14
#define LED_BIT_B 9
#define LED_BIT_C 0
#define LED_BIT_D 13
#define LED_BIT_E 10
#define LED_BIT_F 1

#define LED_BIT_1 6
#define LED_BIT_2 5
#define LED_BIT_3 4
#define LED_BIT_4 3
#define LED_BIT_5 2

#define LED_BIT_V_1 7
#define LED_BIT_V_2 8
#define LED_BIT_V_3 11
#define LED_BIT_V_4 12

#define H_LED_SHIFT 6
#define V_LED_SHIFT 11
#define GATE_LED_SHIFT 15
#define GATE_SHIFT 18
#define CLK_SHIFT 22
#define CLK_LED_SHIFT 21
#define CLK_PIN C,5
#define RST_PIN C,1


const uint8_t outBit[24]={
		LED_BIT_A,LED_BIT_B,LED_BIT_C,LED_BIT_D,LED_BIT_E,LED_BIT_F,
		LED_BIT_1,LED_BIT_2,LED_BIT_3,LED_BIT_4,LED_BIT_5,
		LED_BIT_V_1,LED_BIT_V_2,LED_BIT_V_3,LED_BIT_V_4,
		LED_GATE_A,LED_GATE_B,LED_GATE_C,
		GATE_A,GATE_B,GATE_C,
		LED_CLK,CLK
};



void midiSeqHW::setLed(uint8_t _led,bool _state){
	bitWrite(shiftHash[outBit[_led]/8],7-(outBit[_led]%8),_state);
}
void midiSeqHW::dimLed(uint8_t _led,bool _state){
	bitWrite(dimHash[outBit[_led]/8],7-(outBit[_led]%8),_state);
}
void midiSeqHW::setGateOut(uint8_t _number, bool _state){
	setLed(GATE_SHIFT+_number,_state);
	setLed(GATE_LED_SHIFT+_number,_state);
}
void midiSeqHW::setClkOut(bool _state){
	setLed(CLK_SHIFT,_state);
	setLed(CLK_LED_SHIFT,_state);
}
void midiSeqHW::setHorLed(uint8_t _number, bool _state){
	setLed(H_LED_SHIFT+_number,_state);
}
void midiSeqHW::setVerLed(uint8_t _number, bool _state){
	setLed(V_LED_SHIFT+_number,_state);
}
void midiSeqHW::dimHorLed(uint8_t _number, bool _state){
	dimLed(H_LED_SHIFT+_number,_state);
}
void midiSeqHW::dimVerLed(uint8_t _number, bool _state){
	dimLed(V_LED_SHIFT+_number,_state);
}



//using namespace fastAnalogRead;

//#include <SdFat.h>

// Declaration of instance (for use in interrupt service routine)


#define UINT16_MAX 65535
#define MAX_ADDR 131067

void midiSeqHW::init(void(*buttonChangeCallback)(uint8_t number),void(*clockInCallback)(uint8_t number)) {

	cli();

	shiftRegFast::setup();

	//bit_dir_outp(PIN);



	bit_dir_inp(BUTTON_PIN_A);
	bit_set(BUTTON_PIN_A);

	bit_dir_inp(BUTTON_PIN);
	bit_set(BUTTON_PIN);


	bit_dir_outp(MUX_PIN_0);
	bit_clear(MUX_PIN_0);
	bit_dir_outp(MUX_PIN_1);
	bit_clear(MUX_PIN_1);
	bit_dir_outp(MUX_PIN_2);
	bit_clear(MUX_PIN_2);

	/*
	bit_dir_inp(BUTTON_PIN_B);
	bit_dir_inp(BUTTON_PIN_C);

	bit_dir_inp(BUTTON_PIN_D);
	bit_dir_inp( BUTTON_PIN_E);
	bit_dir_inp(BUTTON_PIN_F);

	bit_dir_inp(BUTTON_MODE);
	bit_dir_inp(BUTTON_LEFT);
	bit_dir_inp(BUTTON_RIGHT);

	bit_set(BUTTON_PIN_A);
	bit_set(BUTTON_PIN_B);
	bit_set(BUTTON_PIN_C);

	bit_set(BUTTON_PIN_D);
	bit_set(BUTTON_PIN_E);
	bit_set(BUTTON_PIN_F);

	bit_set(BUTTON_MODE);
	bit_set(BUTTON_LEFT);
	bit_set(BUTTON_RIGHT);
*/

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

	//fastAnalogRead::init(); // for analog read
	//knobCount=0;
	//fastAnalogRead::connectChannel(knobCount);
	//fastAnalogRead::startConversion();
	sei();
}

void midiSeqHW::setClock(bool _master){
	master=_master;
	if(master){
		bit_dir_outp(CLK_PIN);
		bit_dir_outp(RST_PIN);
		bit_clear(CLK_PIN);
		bit_clear(RST_PIN);
	}
	else{
		bit_dir_inp(CLK_PIN);
		bit_dir_inp(RST_PIN);
		bit_clear(CLK_PIN);
		bit_clear(RST_PIN);
	}
}





/**** BUTTONS ****/
#define TRIG_A 3
#define TRIG_B 4
#define RST 0
#define SHIFT_B 5

void midiSeqHW::setMuxBits(uint8_t _bits){

	if(bitRead(_bits,0)) bit_set(MUX_PIN_0);
	else bit_clear(MUX_PIN_0);

	if(bitRead(_bits,1)) bit_set(MUX_PIN_1);
	else bit_clear(MUX_PIN_1);

	if(bitRead(_bits,2)) bit_set(MUX_PIN_2);
	else bit_clear(MUX_PIN_2);
	//delay(1);
}
uint8_t readOut=0;
void midiSeqHW::isr_updateButtons() {
//buttonBit[buttonSelect];
//isAnalog[i]=true;

	//fastAnalogRead::connectChannel(MIXED_CHANNEL);
	//fastAnalogRead::startConversion();
	lastButtonHash=buttonHash;
	if(readOut<8) readOut++;
	else readOut=0;
switch(readOut){
case 0:
	bitWrite(buttonHash,0,!bit_read_in(BUTTON_PIN_A));
	setMuxBits(BUTT_CO_B);
	break;
	case 1:
	bitWrite(buttonHash,1,!bit_read_in(BUTTON_PIN));
	setMuxBits(BUTT_CO_C);
	break;
	case 2:
	bitWrite(buttonHash,2,!bit_read_in(BUTTON_PIN));
	setMuxBits(BUTT_CO_D);
	break;
	case 3:
	bitWrite(buttonHash,3,!bit_read_in(BUTTON_PIN));
	setMuxBits(BUTT_CO_E);
	break;
	case 4:
	bitWrite(buttonHash,4,!bit_read_in(BUTTON_PIN));
	setMuxBits(BUTT_CO_F);
	break;
	case 5:
	bitWrite(buttonHash,5,!bit_read_in(BUTTON_PIN));
	setMuxBits(BUTT_CO_MODE);
	break;
	case 6:
	bitWrite(buttonHash,6,!bit_read_in(BUTTON_PIN));
	setMuxBits(BUTT_CO_LEFT);
	break;
	case 7:
	bitWrite(buttonHash,7,!bit_read_in(BUTTON_PIN));
	setMuxBits(BUTT_CO_RIGHT);
	break;
	case 8:
	bitWrite(buttonHash,8,!bit_read_in(BUTTON_PIN));
	break;
}

	if(buttonChangeCallback!=0){
		for(uint8_t i=0;i<9;i++){
			if(bitRead(lastButtonHash,i) != bitRead(buttonHash,i)){
				buttonChangeCallback(i);
			}
		}
	}


//	while(!fastAnalogRead::isConversionFinished()){} ;
	            //=fastAnalogRead::getConversionResult();



}
bool midiSeqHW::buttonState(uint8_t _but){
	return bitRead(buttonHash,_but);//buttonBit[_but]);
}


uint8_t dimCycle=0;
void midiSeqHW::isr_updateTriggerStates(){
	if(dimCycle<8) dimCycle++;
	else dimCycle=0;
	uint8_t outHash[3];
	for(uint8_t i=0;i<3;i++) outHash[i]=shiftHash[i];
	if(dimCycle==0){

	}
	else{
		for(uint8_t i=0;i<3;i++){
			//outHash[i]=~(dimHash[i]|shiftHash[i]);
			for(uint8_t j=0;j<8;j++){
				if(bitRead(dimHash[i],j)) bitWrite(outHash[i],j,false);
			}
		}
	}


	shiftRegFast::write_8bit(outHash[2]);
	shiftRegFast::write_8bit(outHash[1]);
	shiftRegFast::write_8bit(outHash[0]);

	shiftRegFast::enableOutput();

}


void midiSeqHW::isr_updateClockIn(){
	if(clockInCallback!=0){
		static bool clockInState;
		bool newState=bit_read_in(CLK_IN_PIN);
		if(newState && !clockInState) clockInCallback(0);
		clockInState=newState;
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


