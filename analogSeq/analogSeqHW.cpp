
/*** PERMANENT SETTINGS***/

#include "analogSeqHW_settings.h"


/*** ACTUAL CODE ***/

#include <Arduino.h>
#include "analogSeqHW.h"
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

#define EXP_5_PIN C,3

const uint8_t gateBit[8]={GATE_BIT_1,GATE_BIT_2,GATE_BIT_3,GATE_BIT_4, GATE_BIT_5,GATE_BIT_6,GATE_BIT_7,GATE_BIT_8};

void analogSeqHW::setGate(uint8_t _gate,bool _state){
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

void analogSeqHW::setLed(uint8_t _led,bool _state){
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
void analogSeqHW::allLedsOff(){
	shiftHash[LED_BYTE]=0;
	shiftHash[BI_LED_A_BYTE]=0;
	for(int i=0;i<7;i++) bitWrite(shiftHash[BI_LED_B_BYTE],i,0);
	//shiftHash[BI_LED_B_BYTE]=0;
}

void analogSeqHW::setBiLed(uint8_t _led,bool _state, bool _color){
	uint8_t _byte;
	if(_led>2){
		_byte=BI_LED_B_BYTE;
		//_led-=3;
	}
	else{
		_byte=BI_LED_A_BYTE;
	}
	if(_state){
		if(_color){
			bitWrite(shiftHash[_byte],biLedBit[_led*2],1);
			bitWrite(shiftHash[_byte], biLedBit[(_led*2)+1] ,0);
		}
		else{
			bitWrite(shiftHash[_byte],biLedBit[(_led*2)],0);
			bitWrite(shiftHash[_byte],biLedBit[(_led*2)+1],1);
		}
	}
	else{
		bitWrite(shiftHash[_byte],biLedBit[(_led*2)],0);
		bitWrite(shiftHash[_byte],biLedBit[(_led*2)+1],0);
	}

}

void analogSeqHW::setGateOut(bool _state){
	bitWrite(shiftHash[GATE_OUT_BYTE],GATE_OUT_BIT,_state);
}

void analogSeqHW::setShiftLed(bool _state){
	bitWrite(shiftHash[SHIFT_LED_BYTE],SHIFT_LED_BIT,_state);
}

#define KNOB_PIN 5
#define KNOB_BIT_1 4
#define KNOB_BIT_2 1
#define KNOB_BIT_3 5
#define KNOB_BIT_4 0
#define KNOB_BIT_5 7
#define KNOB_BIT_6 2
#define KNOB_BIT_7 3
#define KNOB_BIT_8 6

const uint8_t knobBit[8]={ KNOB_BIT_1, KNOB_BIT_2, KNOB_BIT_3, KNOB_BIT_4, KNOB_BIT_5, KNOB_BIT_6, KNOB_BIT_7, KNOB_BIT_8};

void analogSeqHW::selectKnob(uint8_t _knob){
	selectedKnob=knobBit[_knob];
}
/*

#define BUTTON_BIT_1 0
#define BUTTON_BIT_2 3
#define BUTTON_BIT_3 4
#define BUTTON_BIT_4 5
#define BUTTON_BIT_5 8
#define BUTTON_BIT_6 6
#define BUTTON_BIT_7 7
#define BUTTON_BIT_8 2
*/
#define BUTTON_PIN C,1
#define BUTTON_BIT_1 0
#define BUTTON_BIT_2 2
#define BUTTON_BIT_3 3
#define BUTTON_BIT_4 4
#define BUTTON_BIT_5 7
#define BUTTON_BIT_6 5
#define BUTTON_BIT_7 6
#define BUTTON_BIT_8 1
#define ANALOG_EXP_DETECT_PIN B,4

#define MIXED_PIN C,0
#define MIXED_CHANNEL 0

#define PIN B,0


const uint8_t buttonBit[8]={ BUTTON_BIT_1,BUTTON_BIT_2,BUTTON_BIT_3,BUTTON_BIT_4,BUTTON_BIT_5,BUTTON_BIT_6,BUTTON_BIT_7,BUTTON_BIT_8};


//using namespace fastAnalogRead;

//#include <SdFat.h>

// Declaration of instance (for use in interrupt service routine)
analogSeqHW hw;


#define UINT16_MAX 65535
#define MAX_ADDR 131067



//prog_uchar colorBit[NUMBER_OF_COLORS] PROGMEM = {

uint8_t colorBit[NUMBER_OF_COLORS]= {

  BLACK_BITS, RED_BITS,GREEN_BITS,BLUE_BITS,CIAN_BITS,MAGENTA_BITS,YELLOW_BITS,WHITE_BITS

};


unsigned char analogPin[6]={
  ANALOG_PIN_1, ANALOG_PIN_2, ANALOG_PIN_3,  ANALOG_PIN_4, ANALOG_PIN_5, ANALOG_PIN_6};
#define TRIG_A_PIN B,0
#define TRIG_B_PIN B,1
#define RST_PIN C,2
#define MIN_MAJ_GATE_PIN C,3
void analogSeqHW::pinInit() {

shiftRegFast::setup();

	bit_dir_outp(AMUX_SELECT_0);
	bit_dir_outp(AMUX_SELECT_1);
	bit_dir_outp(AMUX_SELECT_2);

	bit_dir_inp(BUTTON_PIN);

	bit_set(BUTTON_PIN);

	bit_dir_inp(MIXED_PIN);
	//bit_set(MIXED_PIN);
	bit_dir_inp(BUTTON_JUMP_PIN);
	bit_set(BUTTON_JUMP_PIN);
	bit_dir_inp(TRIG_A_PIN);
	bit_dir_inp(TRIG_B_PIN);
	bit_dir_inp(RST_PIN);
	bit_dir_inp(MIN_MAJ_GATE_PIN);
	bit_set(TRIG_A_PIN);
	bit_set(TRIG_B_PIN);
	bit_set(RST_PIN);
	bit_set(MIN_MAJ_GATE_PIN);

	bit_dir_inp(ANALOG_EXP_DETECT_PIN);
	bit_set(ANALOG_EXP_DETECT_PIN);
}

void analogSeqHW::init(void(*buttonChangeCallback)(uint8_t number),void(*clockInCallback)(uint8_t number)) {

	cli();

	pinInit();


/*
	bit_dir_inp(INPUT_1);
	bit_dir_inp(INPUT_2);
	bit_dir_inp(BUTTON_PIN_1);
	bit_dir_inp(BUTTON_PIN_2);



	bit_dir_outp(LED_R_PIN);
	 //debug


	bit_set(INPUT_1);
	bit_set(INPUT_2);
	bit_set(BUTTON_PIN_1);
	bit_set(BUTTON_PIN_2);
	bit_set(LED_R_PIN);
	*/

	//bit_dir_outp(PIN);

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

void analogSeqHW::printLEDStates() {

}


void analogSeqHW::setLED(uint8_t number, bool state) {

}

void analogSeqHW::setColor(unsigned char _COLOR){

  //unsigned char _bits=pgm_read_word_near(colorBit + _COLOR)	;
	unsigned char _bits=colorBit[_COLOR];

  if(!bitRead(_bits,0)) bit_set(LED_R_PIN);
  else bit_clear(LED_R_PIN);
  bitWrite(trigState,LED_G_PIN,!bitRead(_bits,1));
  bitWrite(trigState,LED_B_PIN,!bitRead(_bits,2));

}




/**** BUTTONS ****/


const uint8_t mixedBit[8]={ 0,1,2,3,4,5,6,7};

		//3,7,6,4,5,0,2,1};

const uint8_t cvCoordinate[6]={2,4,1,3,0};
uint16_t analogSeqHW::getCV(uint8_t _number){
	if(_number>0){
		if(expanderConnected){
			return mixedValues[cvCoordinate[_number]];
		}
		else{
			return 0;
		}
	}
	else{
		return mixedValues[cvCoordinate[_number]];
	}
}
uint16_t analogSeqHW::getLastCV(uint8_t _number){
	if(_number>0){
		if(expanderConnected){
			return lastMixedValues[cvCoordinate[_number]];
		}
		else{
			return 0;
		}
	}
	else{
		return lastMixedValues[cvCoordinate[_number]];
	}
}
uint16_t analogSeqHW::getPotA(){
	return mixedValues[7];
}
uint16_t analogSeqHW::getPotB(){
	return mixedValues[6];
}

uint16_t analogSeqHW::getLastPotA(){
	return lastMixedValues[7];
}
uint16_t analogSeqHW::getLastPotB(){
	return lastMixedValues[6];
}


bool isAnalog[8]={false,false,true,false,false,false,true,true};
void  analogSeqHW::dimBiLed(uint8_t _led, bool _state){
	bitWrite(biLedDimHash,_led,_state);
}

#define SHIFT_B 5
void analogSeqHW::isr_updateButtons() {

	uint8_t i=buttonSelect;//buttonBit[buttonSelect];
//isAnalog[i]=true;

	if(i==SHIFT_B){
		bit_set(MIXED_PIN);
		fastAnalogRead::connectChannel(MIXED_CHANNEL);
		fastAnalogRead::startConversion();

		while(!fastAnalogRead::isConversionFinished()){} ;
		lastMixedValues[mixedBit[i]]=mixedValues[mixedBit[i]];
		mixedValues[mixedBit[i]]=fastAnalogRead::getConversionResult();

		bool newState;
		if(mixedValues[mixedBit[i]]>512) newState=false;
		else newState=true;
		//bool newState=bit_read_in(MIXED_PIN);
		if(newState != shiftState){
			shiftState=newState;
			bitWrite(buttonHash,8,shiftState);
			if(buttonChangeCallback!=0){
				buttonChangeCallback(8);
			}
		}
		/*
		if(_jumpState!=_lastJumpState){
			if(buttonChangeCallback!=0){
				buttonChangeCallback(9);
			}
		}
		*/
		bit_clear(MIXED_PIN);

	}
	else{
		fastAnalogRead::connectChannel(MIXED_CHANNEL);
		fastAnalogRead::startConversion();

		while(!fastAnalogRead::isConversionFinished()){} ;
		lastMixedValues[mixedBit[i]]=mixedValues[mixedBit[i]];
		mixedValues[mixedBit[i]]=fastAnalogRead::getConversionResult();
	}


	bool newState=!bit_read_in(TRIG_A_PIN);
	if(newState && !trigStateA){
		if(clockInCallback!=0){
			clockInCallback(1);
		}
	}
	trigStateA=newState;

	newState=!bit_read_in(TRIG_B_PIN);
	if(newState && !trigStateB){
		if(clockInCallback!=0){
			clockInCallback(2);
		}
	}
	trigStateB=newState;
	newState=!bit_read_in(RST_PIN);
	if(newState && !rstState){
		if(clockInCallback!=0){
			clockInCallback(0);
		}
	}
	rstState=newState;

	minMajGateState=!bit_read_in(MIN_MAJ_GATE_PIN);

	newState=!bit_read_in(BUTTON_PIN);
	if(buttonChangeCallback!=0){
		if(newState != bitRead(buttonHash,buttonBit[i])){
			bitWrite(buttonHash,buttonBit[i],newState);
			buttonChangeCallback(buttonBit[i]);
		}
	}

	if(fastAnalogRead::isConversionFinished()){
		fastAnalogRead::connectChannel(KNOB_PIN);
		fastAnalogRead::startConversion();
		while(!fastAnalogRead::isConversionFinished()) ;
		knobValues[knobBit[i]]=fastAnalogRead::getConversionResult();
	}


	buttonSelect++;
	if(buttonSelect>7) buttonSelect=0;
	i=buttonSelect;//buttonBit[buttonSelect];

	if(bitRead(i,0)) bit_set(AMUX_SELECT_0);
	else bit_clear(AMUX_SELECT_0);
	if(bitRead(i,1)) bit_set(AMUX_SELECT_1);
	else bit_clear(AMUX_SELECT_1);
	if(bitRead(i,2)) bit_set(AMUX_SELECT_2);
	else bit_clear(AMUX_SELECT_2);
/*
	if(isAnalog[i]){

			bit_clear(MIXED_PIN);
			bit_dir_inp(MIXED_PIN);
			fastAnalogRead::connectChannel(MIXED_CHANNEL);
			fastAnalogRead::startConversion();

	}
	*/
	//if(isAnalog[i]) bit_clear(MIXED_PIN);
	//else bit_set(MIXED_PIN);
	bit_dir_inp(ANALOG_EXP_DETECT_PIN);
	bit_set(ANALOG_EXP_DETECT_PIN);
	expanderConnected=!bit_read_in(ANALOG_EXP_DETECT_PIN);

}
void analogSeqHW::readyForCvIn(){
	uint8_t i=2;
	if(bitRead(i,0)) bit_set(AMUX_SELECT_0);
	else bit_clear(AMUX_SELECT_0);
	if(bitRead(i,1)) bit_set(AMUX_SELECT_1);
	else bit_clear(AMUX_SELECT_1);
	if(bitRead(i,2)) bit_set(AMUX_SELECT_2);
	else bit_clear(AMUX_SELECT_2);

}

bool analogSeqHW::getMinMajState(){
	if(expanderConnected){
	 return minMajGateState;
	}
	else return false;
}
bool analogSeqHW::buttonState(uint8_t _but){
	return bitRead(buttonHash,_but);//buttonBit[_but]);
}
bool analogSeqHW::justPressed(uint8_t _but){
	return bitRead(justPressedHash,buttonBit[_but]);
}
bool analogSeqHW::justReleased(uint8_t _but){
	return bitRead(justReleasedHash,buttonBit[_but]);
}

void analogSeqHW::compareButtonStates(){
	if(buttonChangeCallback!=0){

		for(int i=0;i<2;i++){
			if(buttonStates[i]!=newButtonStates[i]) buttonStates[i]=newButtonStates[i],buttonChangeCallback(i);
		}

	}
	//for(int i=0;i<2;i++) ;


}


void analogSeqHW::printButtonStates() {
	for (uint8_t row=0; row<4; row++) {
		Serial.print("col "); Serial.print(row,DEC);Serial.print(": ");
		for (int8_t col=7; col>=0;col--) {
			if (bitRead(buttonStates[row],col)) {
				Serial.print("1");
			} else {
				Serial.print("0");
			}
		}
		Serial.println("");
	}
}

bool analogSeqHW::getButtonState(uint8_t number) {

	return buttonStates[number];
	/*
	if (buttonStates[number] ) {
		return IHWLayer::UP;
	} else {
		return IHWLayer::DOWN;
	}
*/
}


/**** TRIGGER ****/
void analogSeqHW::setTrigger(uint8_t number, analogSeqHW::TriggerState state, uint16_t pulseWidth){
	triggerCountdown[number]=pulseWidth;
	if(state==ON) bitWrite(trigState,number,1);
	if(state==OFF) bitWrite(trigState,number,0);
}

bool analogSeqHW::getTriggerState(uint8_t number){
		return bitRead(trigState,number);
};

uint16_t analogSeqHW::getKnobValue(uint8_t index){
	return knobValues[index];
}

void analogSeqHW::isr_updateKnobs(){
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
void analogSeqHW::freezeAllKnobs(){
	knobFreezeHash=255;
}
void analogSeqHW::freezeKnob(uint8_t index){
	bitWrite(knobFreezeHash,index,1);
}
void analogSeqHW::freezeKnob(uint8_t index, uint8_t value){
	knobFreezeValues[index]=value;
	bitWrite(knobFreezeHash,index,1);
}
void analogSeqHW::unfreezeKnob(uint8_t index){
	bitWrite(knobFreezeHash,index,0);
}
bool analogSeqHW::knobFreezed(uint8_t index){
	return bitRead(knobFreezeHash,index);
}
bool analogSeqHW::knobMoved(uint8_t index){
	return bitRead(knobMovedHash,index);
}

bool analogSeqHW::inBetween(uint8_t value, uint8_t border1, uint8_t border2){
	if(value<=border1 && value>=border2) return true;
	else if(value>=border1 && value<=border2) return true;
	else return false;
}
void analogSeqHW::setBit(uint8_t _bit, bool _value){
	bitWrite(shiftHash[_bit/4],_bit%8,_value);
}

void analogSeqHW::dimLed(uint8_t _led, bool _state){
	bitWrite(ledDimHash,7-ledBit[_led],_state);
}

void analogSeqHW::isr_updateTriggerStates(){
	ledDimCount++;
	if(ledDimCount>8) ledDimCount=0;
	for(int i=0;i<8;i++){
		if(bitRead(ledDimHash,i)){
			if(ledDimCount<1) bitWrite(ledOutHash,i,bitRead(shiftHash[LED_BYTE],i));
			else bitWrite(ledOutHash,i,0);
		}
		else bitWrite(ledOutHash,i,bitRead(shiftHash[LED_BYTE],i));


	}
uint8_t biLedHashA, biLedHashB;

	for(int i=0;i<3;i++){
		if(bitRead(biLedDimHash,i)){


			if(ledDimCount<1){ //BI_LED_A_1_C biLedBit
				bitWrite(biLedHashA,biLedBit[i*2],bitRead(shiftHash[BI_LED_A_BYTE],biLedBit[i*2]));
				bitWrite(biLedHashA,biLedBit[(i*2)+1],bitRead(shiftHash[BI_LED_A_BYTE],biLedBit[(i*2)+1]));
			}
			else{
				bitWrite(biLedHashA,biLedBit[i*2],0);
				bitWrite(biLedHashA,biLedBit[(i*2)+1],0);
			}
		}
		else{
			bitWrite(biLedHashA,biLedBit[i*2],bitRead(shiftHash[BI_LED_A_BYTE],biLedBit[i*2]));
			bitWrite(biLedHashA,biLedBit[(i*2)+1],bitRead(shiftHash[BI_LED_A_BYTE],biLedBit[(i*2)+1]));
		}



		if(bitRead(biLedDimHash,i+3)){


			if(ledDimCount<1){ //BI_LED_A_1_C biLedBit
				bitWrite(biLedHashB,biLedBit[6+(i*2)],bitRead(shiftHash[BI_LED_B_BYTE],biLedBit[6+(i*2)]));
				bitWrite(biLedHashB,biLedBit[6+(i*2)+1],bitRead(shiftHash[BI_LED_B_BYTE],biLedBit[6+(i*2)+1]));
			}
			else{
				bitWrite(biLedHashB,biLedBit[6+(i*2)],0);
				bitWrite(biLedHashB,biLedBit[6+(i*2)+1],0);
			}
		}
		else{
			bitWrite(biLedHashB,biLedBit[6+(i*2)],bitRead(shiftHash[BI_LED_B_BYTE],biLedBit[6+(i*2)]));
			bitWrite(biLedHashB,biLedBit[6+(i*2)+1],bitRead(shiftHash[BI_LED_B_BYTE],biLedBit[6+(i*2)+1]));
		}


	}

	if(shiftLedDim){
		if(ledDimCount<1){
			bitWrite(biLedHashB,SHIFT_LED_BIT, bitRead(shiftHash[BI_LED_B_BYTE],SHIFT_LED_BIT));
		}
		else{
			bitWrite(biLedHashB,SHIFT_LED_BIT, 0);
		}
	}
	else{
		bitWrite(biLedHashB,SHIFT_LED_BIT, bitRead(shiftHash[BI_LED_B_BYTE],SHIFT_LED_BIT));
	}
	bitWrite(biLedHashB,GATE_OUT_BIT, bitRead(shiftHash[BI_LED_B_BYTE],GATE_OUT_BIT));
	//bit_set(SHIFTREGISTER_RCK);
	//bit_clear(SHIFTREGISTER_RCK);
	shiftRegFast::write_8bit(biLedHashA);//shiftHash[3]);
	//delayMicroseconds(20);
	shiftRegFast::write_8bit(biLedHashB);//shiftHash[2]);
	//delayMicroseconds(20);
	shiftRegFast::write_8bit(ledOutHash);
	//delayMicroseconds(20);
	shiftRegFast::write_8bit(shiftHash[0]);
	//delayMicroseconds(20);

	shiftRegFast::enableOutput();
/*
	for(int i=0;i<6;i++){
		if(triggerCountdown[i]>0){
			if(triggerCountdown[i]==1) setTrigger(i,OFF,0);
			triggerCountdown[i]--;
		}
	}
*/
}
void analogSeqHW::resetTriggers(){
	for(int i=0;i<6;i++) triggerCountdown[i]=0;
	trigState=0;
	//shiftRegFast::write_8bit(trigState);
	//shiftRegFast::enableOutput();

}
void analogSeqHW::isr_updateClockIn(){
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

uint32_t analogSeqHW::getElapsedBastlCycles() {
	return bastlCycles;
}

uint16_t analogSeqHW::getBastlCyclesPerSecond() {
	return (F_CPU/1024)/OCR2A;
}


/**** INTERRUPT ****/

ISR(TIMER2_COMPA_vect) { //56us :)


	//bit_set(PIN);
	hw.incrementBastlCycles();
	//hardware.isr_updateClockIn();
	//hardware.isr_updateKnobs();
	hw._lastJumpState=hw._jumpState;
	hw._jumpState=!bit_read_in(BUTTON_JUMP_PIN);
	hw.isr_updateTriggerStates();
	hw.isr_updateButtons();      // ~1ms
	//hardware.isr_updateNextLEDRow();   // ~84us

//	bit_clear(PIN);


}


