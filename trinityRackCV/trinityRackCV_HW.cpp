
/*** PERMANENT SETTINGS***/

#include "trinityRackCV_HW_settings.h"


/*** ACTUAL CODE ***/

#include <Arduino.h>
#include "trinityRackCV_HW.h"
#include <shiftRegisterFast.h>
#include <avr/pgmspace.h>
#include <fastAnalogRead.h>
#include <SPI.h>


#define CV_MOVED_TOLERANCE 1




// Declaration of instance (for use in interrupt service routine)
trinityRackCV_HW hardware;


#define UINT16_MAX 65535
#define MAX_ADDR 131067

const uint8_t DACA = 3;
const uint8_t DACB = 15;
const uint8_t DACC = 7;
const uint8_t DACD = 1;
const uint8_t DACE = 5;
const uint8_t DACF = 13;
const uint8_t DACG = 9;
const uint8_t DACH = 11;

const unsigned char dacNumber[8]={DACA,DACB,DACC,DACD,DACE,DACF,DACG,DACH};

const unsigned char analogPin[6]={ANALOG_PIN_1,ANALOG_PIN_2,ANALOG_PIN_3,ANALOG_PIN_4,ANALOG_PIN_5,ANALOG_PIN_6};

void trinityRackCV_HW::init(void(*clockInCallback)()) {

	cli();

	bit_dir_inp(CLOCK_IN_PIN);
	bit_set(CLOCK_IN_PIN);

	bit_dir_outp(LOAD_PIN);
	bit_dir_outp(PIN); //debug
	bit_dir_outp(PIN_2);

	DACInit();

	// store callback pointer for changed buttons
	 this->clockInCallback = clockInCallback;

	// Disable Timer1 interrupt
	//TIMSK1 &= ~_BV(TOIE1);

	// TIMER 2
	TCCR2A = (1 << WGM21);  // turn on CTC mode
	TIMSK2 |= (1 << OCIE2A);// enable interrupt
	TCCR2B = B00000111;	    //prescaler = 1024
	OCR2A = (F_CPU/1024)/(updateFreq);
	TCNT2  = 0;

	// Analog Reads
	fastAnalogRead::init(); // for analog read
	CVCount=0;
	fastAnalogRead::connectChannel(CVCount);
	fastAnalogRead::startConversion();

	CVMovedHash=0;

	sei();

}







/**** CV ****/
void trinityRackCV_HW::setDAC(uint8_t number, uint8_t value){
	dacValues[number]=value;
}

uint8_t trinityRackCV_HW::getCVValue(uint8_t index){
	return CVValues[index];
}

void trinityRackCV_HW::isr_updateADC(){

	if(fastAnalogRead::isConversionFinished()){

		lastCVValues[CVCount]=CVValues[CVCount];
		CVValues[CVCount]=(fastAnalogRead::getConversionResult()>>2);

		int difference=(int)lastCVValues[CVCount]-CVValues[CVCount];
		difference=abs(difference);
		if(difference>CV_MOVED_TOLERANCE) bitWrite(CVMovedHash,CVCount,1);
		else 							  bitWrite(CVMovedHash,CVCount,0);

		CVCount++;
		if(CVCount>=numbCVs) CVCount=0;

		fastAnalogRead::connectChannel(analogPin[CVCount]);
		fastAnalogRead::startConversion();
	}

}


bool trinityRackCV_HW::CVMoved(uint8_t index){
	return bitRead(CVMovedHash,index);
}

void trinityRackCV_HW::isr_updateDAC(){

	//if(dacCount<numbDACs-1) dacCount++;
	//else dacCount=0;
	for(int i=0;i<7;i++){
		dacCount=i;

	// don't send value to DAC if it hasn't changed

	//if (dacValues[dacCount] == dacValuesSent[dacCount]) return;

	bit_set(LOAD_PIN);
	SPI.transfer(dacNumber[dacCount]);
	SPI.transfer(dacValues[dacCount]);
	bit_clear(LOAD_PIN);

	dacValuesSent[dacCount] = dacValues[dacCount];
	}


}

void trinityRackCV_HW::DACInit(){
  SPI.begin();
  SPI.setBitOrder(MSBFIRST);  // Most Significant bit first.
  SPI.setClockDivider(SPI_CLOCK_DIV16);  //16MHz divided by 16 = 1MHz
  SPI.setDataMode(SPI_MODE1);  // zero based clock, data on falling edge, seems like the correct setting
  bit_clear(LOAD_PIN);

  dacCount = 0;
  zeroDACs();
}

void trinityRackCV_HW::zeroDACs(){
  for (int i=0;i< 8; i++){
	dacCount = i;
    dacValues[dacCount] = 0;
    isr_updateDAC();
  }
}






void trinityRackCV_HW::isr_updateClockIn(){
	if(clockInCallback!=0){
		bool newState=!bit_read_in(CLOCK_IN_PIN);
		if(newState && !clockInState) clockInCallback();
		clockInState=newState;
	}
}

bool trinityRackCV_HW::getClockState(){
	return clockInState;
}

/**** TIMING ****/

uint16_t trinityRackCV_HW::getBastlCyclesPerSecond() {
	return (F_CPU/1024)/OCR2A;
}


/**** INTERRUPT ****/

ISR(TIMER2_COMPA_vect) {
	//bit_set(PIN_2);

	hardware.incrementBastlCycles();
	//bit_clear(PIN_2);
}


