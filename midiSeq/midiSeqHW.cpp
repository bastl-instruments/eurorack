
/*** PERMANENT SETTINGS***/

#include "midiSeqHW_settings.h"


/*** ACTUAL CODE ***/

#include <Arduino.h>
#include "midiSeqHW.h"
#include <shiftRegisterFast.h>
#include <avr/pgmspace.h>
#include <fastAnalogRead.h>
#include <SPI.h>
#include <util/atomic.h>

midiSeqHW hw;

#define BUTTON_PIN D,7

#define DISPLAY 2
#define BUT_CS 1
#define LED_GATE 0

//595 3
#define GATE_A 2
#define LED_A 1
#define GATE_B 4
#define LED_B 3
#define GATE_C 6
#define LED_C 5
#define GATE_D 0
#define LED_D 7

const uint8_t gatePin[4]={GATE_A,GATE_B,GATE_C,GATE_D};
const uint8_t ledPin[4]={LED_A,LED_B,LED_C,LED_D};



//595 2
#define CS_1 3
#define CS_2 1
#define RST_PIN 0
#define CLK_PIN 2
#define BUTTON_SET 4
#define BUTTON_TUNE 5
#define BUTTON_UP 6
#define BUTTON_DOWN 7

const uint8_t CSpin[4]={CS_1,CS_2};
const uint8_t buttonPin[4]={BUTTON_SET,BUTTON_TUNE,BUTTON_UP,BUTTON_DOWN};

//595 1
#define SEG_A 3
#define SEG_B 4
#define SEG_C 6
#define SEG_D 5
#define SEG_E 0
#define SEG_F 2
#define SEG_G 1
#define SEG_P 7


//#define DETECT_PIN C,2 -olderHW
#define DETECT_PIN D,6

#define TUNE_PIN_A D,2
#define TUNE_PIN_B D,3
#define TUNE_PIN_C D,4
#define TUNE_PIN_D D,5


//#define UPDATE_PIN D,6 -olderHW
#define UPDATE_PIN C,2

#define CALIBRATION_PIN_A C,0
#define CALIBRATION_PIN_B C,1



const uint8_t analogPin[4]={3,4,5,2};


void midiSeqHW::isr_updateDisplay(){

	bitWrite(buffer595[DISPLAY],SEG_A,!bitRead(displayBuffer,0));
	bitWrite(buffer595[DISPLAY],SEG_B,!bitRead(displayBuffer,1));
	bitWrite(buffer595[DISPLAY],SEG_C,!bitRead(displayBuffer,2));
	bitWrite(buffer595[DISPLAY],SEG_D,!bitRead(displayBuffer,3));
	bitWrite(buffer595[DISPLAY],SEG_E,!bitRead(displayBuffer,4));
	bitWrite(buffer595[DISPLAY],SEG_F,!bitRead(displayBuffer,5));
	bitWrite(buffer595[DISPLAY],SEG_G,!bitRead(displayBuffer,6));
	bitWrite(buffer595[DISPLAY],SEG_P,!bitRead(displayBuffer,7));

}

//const uint8_t buttonPin[9]={BUTTON_PIN_A,BUTTON_PIN_B,BUTTON_PIN_C,BUTTON_PIN_D,BUTTON_PIN_E,BUTTON_PIN_F,BUTTON_MODE,BUTTON_LEFT,BUTTON_RIGHT};






PROGMEM const uint8_t Typo[57]={
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

  B01000000, //53
  B00000100, //54
  B00010000,//55
  B00001000, //56

};


#define VOID 36

void  midiSeqHW::displayChar(char whichChar) {
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

void  midiSeqHW::lightNumber(int numberToDisplay) {

  for(uint8_t i=0;i<7;i++){
    bitWrite(displayBuffer,i,bitRead(pgm_read_word_near(Typo+numberToDisplay),i));
  }

}

void  midiSeqHW::displayNumber(int _number){
  //lightNumber(_number/100,1);
  //lightNumber((_number%100)/10,2);
  lightNumber(_number%10);
//  if(_number>9) setDot(true);
//  else setDot(false);

}
void midiSeqHW::setPitchBend(uint8_t channel, int _pitchBend){
	int semitoneSize=scale[channel]/12;
	pitchBend[channel] = map(_pitchBend,-8192,8192,-(semitoneSize*pitchBendRange),semitoneSize*pitchBendRange);
}
void  midiSeqHW::setDot( boolean _state){
  bitWrite(displayBuffer,7,_state);
}


void midiSeqHW::setCS(uint8_t _number, bool _state){
	bitWrite(buffer595[BUT_CS],CSpin[_number],_state);
}
void midiSeqHW::setClk(bool _state){
	bitWrite(buffer595[BUT_CS],CLK_PIN,_state);
}
void midiSeqHW::setRst(bool _state){
	bitWrite(buffer595[BUT_CS],RST_PIN,_state);
}


void midiSeqHW::setGate(uint8_t _number, bool _state){
	bitWrite(gateHash,_number,_state); //to synchronise cv and gate update - updates only when DAC gets updated
}
#define HIGH_VOLTAGE 3000
#define LOW_VOLTAGE 100
void midiSeqHW::setDacGate(uint8_t _number, bool _state){
	if(_state) setVoltage(_number,HIGH_VOLTAGE);
	else setVoltage(_number,LOW_VOLTAGE);
}


void midiSeqHW::setLed(uint8_t _number, bool _state){
	bitWrite(buffer595[LED_GATE],ledPin[_number],_state);
}

bool midiSeqHW::getDetectState(){
	return bit_read_in(DETECT_PIN);
}

bool midiSeqHW::getUpdateState(){
	return analogValues[3]<350;// !bit_read_in(UPDATE_PIN);
}

bool midiSeqHW::getTuneState(uint8_t _number){
	bool _state=false;
	switch(_number){
		case 0:
			_state=!bit_read_in(TUNE_PIN_A);
			break;
		case 1:
			_state=!bit_read_in(TUNE_PIN_B);
			break;
		case 2:
			_state=!bit_read_in(TUNE_PIN_C);
			break;
		case 3:
			_state=!bit_read_in(TUNE_PIN_D);
			break;
	}
	return _state;

}

uint16_t midiSeqHW::getAnalogValue( uint8_t _number){
	return analogValues[_number];
}

//using namespace fastAnalogRead;

//#include <SdFat.h>

// Declaration of instance (for use in interrupt service routine)


#define UINT16_MAX 65535
#define MAX_ADDR 131067

void midiSeqHW::setPCMask(uint8_t _number) {
	PCMSK2=0;
	_measuredPin=_number;
	switch(_number){
		case 0:
			bitWrite(PCMSK2,PCINT18,1); //TUNE A
			break;
		case 1:
			bitWrite(PCMSK2,PCINT19,1); //TUNE B
			break;
		case 2:
			bitWrite(PCMSK2,PCINT20,1); //TUNE C
			break;
		case 3:
			bitWrite(PCMSK2,PCINT21,1); //TUNE D
			break;
		default: //any other number will disable the mask
			break;
	}
}

void  midiSeqHW::setVoltage(uint8_t channel, uint16_t voltage)
{
	if(channel<4 && voltage<4096){
		DACvoltage[channel]=voltage;
		uint16_t difference=abs((int)DACslewed[updatedDAC]-(int)DACvoltage[updatedDAC]);
		slewIncrement[channel]=((difference)<<5)/slewRate;
		if(slewIncrement[channel]>>5==0) slewIncrement[channel]+=1<<5;
		//if(slewIncrement[channel]=((difference)<<5)/slewRate;)
	}
}

void midiSeqHW::setSlew(uint16_t _slew){
	slewRate=_slew;
	uint16_t difference=abs((int)DACslewed[updatedDAC]-(int)DACvoltage[updatedDAC]);
	for(uint8_t i=0;i<4;i++){
		slewIncrement[i]=((difference)<<5)/slewRate;
		if(slewIncrement[i]>>5==0) slewIncrement[i]+=1<<5;
	}

}

const uint8_t updateSequence[4]={0,2,1,3};

void  midiSeqHW::isr_updateDAC()
{

	uint16_t voltage=0;


	if((DACslewed[updatedDAC]!=DACvoltage[updatedDAC]) && slewIncrement[updatedDAC]==0){ //voltage moved - establish new increment

		//map(,DACslewed[updatedDAC],DACvoltage[updatedDAC],0,slewRate)
	}
	uint16_t difference=abs((int)DACslewed[updatedDAC]-(int)DACvoltage[updatedDAC]);
	//if(difference<<5slewRate)
	//slewIncrement[updatedDAC]=((difference)<<3)/slewRate;

	if(DACslewed[updatedDAC]==DACvoltage[updatedDAC]) ;// slewIncrement[updatedDAC]=0;

	else if(DACslewed[updatedDAC]>DACvoltage[updatedDAC]){
		if((slewIncrement[updatedDAC]>>5)>=difference) DACslewed[updatedDAC]=DACvoltage[updatedDAC];//, slewIncrement[updatedDAC]=0;
		else DACslewed[updatedDAC]-=(slewIncrement[updatedDAC]>>5);
	}
	else if(DACslewed[updatedDAC]<DACvoltage[updatedDAC]){
		if((slewIncrement[updatedDAC]>>5)>=difference) DACslewed[updatedDAC]=DACvoltage[updatedDAC];//, slewIncrement[updatedDAC]=0;
		else DACslewed[updatedDAC]+=(slewIncrement[updatedDAC]>>5);
	}

	voltage=DACslewed[updatedDAC];

	//voltage=DACvoltage[updatedDAC];

	uint16_t command=0;
	if((updatedDAC%2==0)) command = 0x0000;
	else command = 0x8000;

	command |= 0x1000;
	command |= 0x0000 ;
	command |= (voltage & 0x0FFF);

//	digitalWrite(_csPin,LOW);
	SPI.setDataMode(SPI_MODE0);
	SPI.transfer(command>>8);
	SPI.transfer(command&0xFF);

	bitWrite(buffer595[LED_GATE],gatePin[updatedDAC],bitRead(gateHash,updatedDAC)); //to synchronise cv and gate update


	//digitalWrite(_csPin,HIGH);

	//bitWrite(buffer595[BUT_CS],CSpin[0],1);
	//bitWrite(buffer595[BUT_CS],CSpin[1],1);

	updateWhich++;
	if(updateWhich>3) updateWhich=0;
	updatedDAC=updateSequence[updateWhich];

	if(updatedDAC/2){
		bitWrite(buffer595[BUT_CS],CSpin[0],1);
		bitWrite(buffer595[BUT_CS],CSpin[1],0);

	}
	else{
		bitWrite(buffer595[BUT_CS],CSpin[0],0);
		bitWrite(buffer595[BUT_CS],CSpin[1],1);
	}
}

uint16_t inputCalibrationTable[4][12];
void  midiSeqHW::calibrateInputs(){
	for(uint8_t j=0;j<12;j++){
		for(uint8_t i=0;i<4;i++) writeDAC(i,calibrationTable[i][j]);

		for(uint8_t i=0;i<12;i++) isr_updateAnalogInputs(), delay(1);

		for(uint8_t i=0;i<4;i++) inputCalibrationTable[i][j]=getAnalogValue(i);
	}
}

void  midiSeqHW::writeDAC(uint8_t channel, uint16_t voltage)
{
	if(channel<4 && voltage<4096){

		DACvoltage[channel]=voltage;
		uint16_t command=0;
		if((channel%2==0)) command = 0x0000;
		else command = 0x8000;
		command |= 0x1000;
		command |= 0x0000 ;
		command |= (voltage & 0x0FFF);


		if(channel/2){
			bitWrite(buffer595[BUT_CS],CSpin[0],1);
			bitWrite(buffer595[BUT_CS],CSpin[1],0);

		}
		else{
			bitWrite(buffer595[BUT_CS],CSpin[0],0);
			bitWrite(buffer595[BUT_CS],CSpin[1],1);
		}
		isr_updateShiftRegisters();

		SPI.setDataMode(SPI_MODE0);
		SPI.transfer(command>>8);
		SPI.transfer(command&0xFF);

		bitWrite(buffer595[BUT_CS],CSpin[0],1);
		bitWrite(buffer595[BUT_CS],CSpin[1],1);
		isr_updateShiftRegisters();

	}
}


const uint8_t calPin[4]={3,3,3,4};
const uint8_t channelUsedToCalibrate[4]={3,3,3,0};
const uint16_t tuningVoltage[9]={0,500,1000,1500,2000,2500,3000,3500,4000};
const uint16_t defaultCalibration[15]={40, 300, 600, 900, 1200, 1500, 1800, 2100, 2400, 2700, 3000, 3300, 3600, 3900, 4200};



uint8_t microTonalTable[12]={127,127,127,127,127 ,127,127,127,127,127,127,127};

#define COMPARTOR_PIN  B,0
void midiSeqHW::setMicroTonalTable(uint8_t _note, uint8_t _value){
	microTonalTable[_note]=_value;
}
uint8_t midiSeqHW::getMicroTonalTable(uint8_t _note){
	return microTonalTable[_note];
}
void midiSeqHW::allDACLow(){
	for(uint8_t i=0;i<4;i++) writeDAC(i,0);
}

void midiSeqHW::autoCalibrate(uint8_t channel){
//	cli();
	tuning=true; //make sure interrupts do not come
	allDACLow();
	//digitalWrite(calPin[channel],LOW);init
	bool ledBlink=false;

	for(uint8_t i=0;i<9;i++){
		calibrationTable[channel][i]=defaultCalibration[i];

		//allDACLow();
		writeDAC(channelUsedToCalibrate[channel],tuningVoltage[i]);
		writeDAC(channel,calibrationTable[channel][i]);
		//delay(500);
		if(channel==3){
			while(!bit_read_in(CALIBRATION_PIN_B)){
				ledBlink=!ledBlink;
				setLed(channel,ledBlink);
				calibrationTable[channel][i]++;
				if(calibrationTable[channel][i]>4095) calibrationTable[channel][i]=0;
				writeDAC(channel,calibrationTable[channel][i]);
				//Serial.print(".");
			}
		}
		else{
			while(!bit_read_in(CALIBRATION_PIN_A)){
				ledBlink=!ledBlink;
				setLed(channel,ledBlink);
				calibrationTable[channel][i]++;
				if(calibrationTable[channel][i]>4095) calibrationTable[channel][i]=0;
				writeDAC(channel,calibrationTable[channel][i]);
			//	Serial.print(".");
			//	Serial.print(calibrationTable[channel][i]);
			}
		}
		if(abs(calibrationTable[channel][i]-defaultCalibration[i])>100)  autoCalibrate(channel); // Serial.println(),Serial.println(), if grounded cable was released - calibrete from scratch
		calibrationTable[channel][i]--;
	//	Serial.print(calibrationTable[channel][i]);
	//	Serial.print(", ");

	}
	//find average half octave
	uint16_t differenceSum=0;
	for(uint8_t i=0;i<8;i++){
		uint16_t difference=calibrationTable[channel][i+1]-calibrationTable[channel][i];
		differenceSum+=difference;
	}
	uint16_t halfOctave=differenceSum/8;
	//this is questionable but ok...
	//for frequency measurement method there should be the 1% impedance error compensation detection

	for(uint8_t i=9;i<15;i++){
		calibrationTable[channel][i]=calibrationTable[channel][i-1]+halfOctave;
		//Serial.print(calibrationTable[channel][i]);
	//	Serial.print(", ");
	}
	//Serial.println(", ");
	for(uint8_t i=0;i<4;i++) setLed(i,0), writeDAC(i,calibrationTable[i][0]); //go to 0volts everywhere
//	tuning=false;
//	sei();
	//digitalWrite(calPin[channel],HIGH);
}
bool midiSeqHW::makeAutoTuneTable(uint8_t channel){
	bool _success=true;
	autoTuneTable[channel][0]=offset[channel];
	for(uint8_t i=1;i<15;i++){
		uint16_t _tuningPoint=getToNote(channel,i*6);
		if(_tuningPoint==0){
			if(i>1 && autoTuneTable[channel][i-1]!=0) autoTuneTable[channel][i]=autoTuneTable[channel][i-1]+(autoTuneTable[channel][i-1]-autoTuneTable[channel][i-2]);
		}
		else autoTuneTable[channel][i]=_tuningPoint;
		//Serial.print(autoTuneTable[channel][i]);
		//Serial.print(", ");
	}
	autoTuneActive[channel]=true;
	return _success;
}

bool midiSeqHW::offsetAutoTuneTable(uint8_t channel,int _offset){
	bool _success=true;
	offset[channel]+=_offset;
	for(uint8_t i=1;i<15;i++){
		if(autoTuneTable[channel][i]+_offset>0) autoTuneTable[channel][i]+=_offset;
		else _success=false;
	//	if() negative ?
		//Serial.print(autoTuneTable[channel][i]);
		//Serial.print(", ");
	}
	autoTuneActive[channel]=true;
	return _success;
}
bool midiSeqHW::reMakeAutoTuneTable(uint8_t channel){
	bool _success=true;
	int _difference=offset[channel]-autoTuneTable[channel][0];
	autoTuneTable[channel][0]=offset[channel];
	for(uint8_t i=1;i<15;i++){
		if(autoTuneTable[channel][i]+_difference>0) autoTuneTable[channel][i]+=_difference;
		else _success=false;
	//	if() negative ?
		//Serial.print(autoTuneTable[channel][i]);
		//Serial.print(", ");
	}
	autoTuneActive[channel]=true;
	return _success;
}
void midiSeqHW::setNote(uint8_t channel, uint8_t note){


	uint8_t _noteOffset=12+(lowestOctave[channel]*12);

	if(autoTuneActive[channel]){ //make the right pitch - overflow the lowest and the highest tone to nearest octave
		while(note<_noteOffset){
			note+=12;
		}
		while(note>=(highestNote[channel]+_noteOffset)){ //
			note-=12;
		}

		note-=_noteOffset;
	}

	uint8_t tableSegment=note/6;
	uint8_t numberOfSemitones=note%6;

	currentNote[channel]=note;

	if(autoTuneActive[channel]){
		int semiToneSize=(autoTuneTable[channel][tableSegment+1]-autoTuneTable[channel][tableSegment]) /6;
		uint16_t DACoutput=pitchBend[channel]+autoTuneTable[channel][tableSegment]+ (semiToneSize*numberOfSemitones)+map( microTonalTable[note%12],0,255,-semiToneSize,+semiToneSize) ;

		writeDAC(channel,DACoutput);
		//Serial.println("a");
	}
	else if(scaleOffset[channel]){
		int semiToneSize=scale[channel]/12;
		uint8_t _octave=note/12;
		//uint16_t DACoutput=offset[channel]+_octave*offset[channel]+(note%12)*semiToneSize;
		uint16_t DACoutput=offset[channel]+note*semiToneSize+map( microTonalTable[note%12],0,255,-semiToneSize,+semiToneSize);
		setVoltage(channel,DACoutput);
	//	Serial.println("s");
	}

	else{
		int semiToneSize=(calibrationTable[channel][tableSegment+1]-calibrationTable[channel][tableSegment]) /6;
		uint16_t DACoutput=calibrationTable[channel][tableSegment]+ (semiToneSize*numberOfSemitones)+map( microTonalTable[note%12],0,255,-semiToneSize,+semiToneSize);
		setVoltage(channel,DACoutput);
		//Serial.println("n");
	}

}

const uint16_t notePeriod[13]={61270,57879,54574,51559,48669,45892, 43323,40940, 38603, 36480,34417,32517,30635};
#define TIME_OUT 100000

uint32_t midiSeqHW::getAveragePeriod(uint8_t channel){
	setPCMask(channel);
	bool _success=getMeasurement();
	setPCMask(255);
	if(_success) return runningPCtime; //some waiting
	else return 0;
}

bool midiSeqHW::getStableMeasurement(){ //not fully tested
	bool _success=getMeasurement();

	uint32_t _periodCheck=runningPCtime;
	_success=getMeasurement();
	uint8_t precision=13;
	while((hw.PCperiod>>precision)==0) precision--;
	precision--;
	while(abs(_periodCheck-runningPCtime)>2){//runningPCtime>>precision){
		precision=13;
		while((hw.PCperiod>>precision)==0) precision--;
		precision--;
		_periodCheck=runningPCtime;
		_success=getMeasurement();
	}
	return _success;

}
bool midiSeqHW::getMeasurement(){
	tuning=true;
	runningCounter=0;
	uint32_t timeoutTime=getTime();
	bool _timeout=false;
	while(runningCounter==0 && _timeout==false) if(getTime()-timeoutTime>TIME_OUT) _timeout=true; //timeout dedělat
	while(runningCounter%8!=0 && _timeout==false) if(getTime()-timeoutTime>(TIME_OUT<<3)) _timeout=true; //timeout
	while(runningCounter%8!=1 && _timeout==false) if(getTime()-timeoutTime>(TIME_OUT<<3)) _timeout=true;
	while(runningCounter%8!=2 && _timeout==false) if(getTime()-timeoutTime>(TIME_OUT<<3)) _timeout=true;
	//tuning=false;
	return !_timeout;
	//should be ready
}
bool midiSeqHW::periodToNote(uint32_t _period){
	semitone=0;
	octave=0;
	above=true;
	_lower=0;
	/*
	if(_period >=notePeriod[0]<<_lower){

	}
*/
	/*
	Serial.print(_period);
	Serial.print("<");
	Serial.print((notePeriod[0]<<_lower));
*/
	if(_period!=0 && _period <(notePeriod[0]<<_lower)){


		while(!(_period<((notePeriod[0]<<_lower)>>octave) && _period>=((notePeriod[12]<<_lower)>>octave))){
			octave++;
		}

		while(!(_period<((notePeriod[semitone]<<_lower)>>octave) && _period>=((notePeriod[semitone+1]<<_lower)>>octave))){
			semitone++;
		}

		if(_period<((notePeriod[semitone]<<_lower)>>octave)-(((notePeriod[semitone]<<_lower)>>octave)-((notePeriod[semitone+1]<<_lower)>>octave))/2) semitone++, above=false;

		return true;
	}
	else return false;
}
bool midiSeqHW::analyseNote(uint8_t channel){
	octave=0;
	semitone=0;
	above=true;
	uint32_t _period=getAveragePeriod(channel);
	_lower=0;
	return periodToNote(_period);


}
#define LOW_POINT 0
#define HIGH_POINT 2
#define MAX_NO_OF_APPROACHES 100
uint16_t midiSeqHW::getScaling(uint8_t channel){
	writeDAC(channel,calibrationTable[channel][HIGH_POINT]);
	delay(1);
	uint32_t _period=getAveragePeriod(channel);
	//informed guessing
	//start with scale of one volt exactly
	scale[channel]=calibrationTable[channel][HIGH_POINT+2]-calibrationTable[channel][HIGH_POINT];
	//guess the rootNote
	periodToNote(_period);
	lowestOctave[channel]=octave;
	uint16_t _semitoneSize=scale[channel]/12;
	offset[channel]=calibrationTable[channel][HIGH_POINT]-(semitone*_semitoneSize);
	//SCALE
	return scale[channel]=getToPeriod(channel, _period/2)-calibrationTable[channel][HIGH_POINT]; //get precise scale
	//OFFSET
	//offset[channel]=getToPeriod(channel,notePeriod[rootNote]>>lowestOctave[channel]); //get precise offset

}

#define rootNote 0

uint32_t midiSeqHW::noteToPeriod(uint8_t channel, uint8_t _note){
	uint8_t _octave=_note/12;
	uint8_t _semi=_note%12;
	return notePeriod[_semi]>>(_octave+lowestOctave[channel]);
}

uint16_t midiSeqHW::getToNote(uint8_t channel, uint8_t _note){
	if(_note>highestNote[channel]){
		return 0;
	}
	else return getToPeriod(channel,noteToPeriod(channel,_note));
}

bool midiSeqHW::reAutoTune(uint8_t channel){
	bool _success=true;
	if(autoTuneActive[channel]){

		//
		uint16_t previousNoteVoltage=DACvoltage[channel];

		uint16_t _valueDAC = getToNote(channel, currentNote[channel]);

		if(_valueDAC!=0) offsetAutoTuneTable(channel,(int)_valueDAC-previousNoteVoltage);
		else _success=false;
/*

		//1 volt measurement informs us a lot
		writeDAC(channel,calibrationTable[channel][HIGH_POINT]);
		delay(1);
		uint32_t _period=getAveragePeriod(channel);
		if(_period==0) _success=false;//, Serial.println("invalid");
		//informed guessing
		//start with scale of one volt exactly

		//scale[channel]=calibrationTable[channel][HIGH_POINT+2]-calibrationTable[channel][HIGH_POINT];

		//guess the rootNote
		if(!periodToNote(_period)){
			_success=false;//, Serial.println("too low");
		}
		else{
			//if(_lower>0) Serial.print(_lower), Serial.println("too low");
			lowestOctave[channel]=octave;
			lowestNote[channel]=lowestOctave[channel]*12;
			uint16_t _semitoneSize=scale[channel]/12;
			//rootNote - zabudovat zde
			//semitone=(semitone+rootNote)%12;
			uint16_t oldOffset=offset[channel];
			offset[channel]=calibrationTable[channel][HIGH_POINT]-(semitone*_semitoneSize);
			if(!getOffset(channel)) autoTune(channel);
			if(abs(offset[channel]-oldOffset)>1) reMakeAutoTuneTable(channel);
		}
 */

		//getOffset(channel);
	}

	else _success=autoTune( channel);
	return _success;
}

bool midiSeqHW::autoTune(uint8_t channel){
	bool _success=true;
	//1 volt measurement informs us a lot
	writeDAC(channel,calibrationTable[channel][HIGH_POINT]);
	delay(1);
	uint32_t _period=getAveragePeriod(channel);
	if(_period==0) _success=false;//, Serial.println("invalid");
	//informed guessing
	//start with scale of one volt exactly
	scale[channel]=calibrationTable[channel][HIGH_POINT+2]-calibrationTable[channel][HIGH_POINT];
	//guess the rootNote
	if(!periodToNote(_period)){
		_success=false;//, Serial.println("too low");
	}
	else{
		//if(_lower>0) Serial.print(_lower), Serial.println("too low");
		lowestOctave[channel]=octave;
		lowestNote[channel]=lowestOctave[channel]*12;
		uint16_t _semitoneSize=scale[channel]/12;
		//rootNote - zabudovat zde
		//semitone=(semitone+rootNote)%12;
		offset[channel]=calibrationTable[channel][HIGH_POINT]-(semitone*_semitoneSize);
	}
	/*
	Serial.println();
	Serial.print(" scale guess: ");
	Serial.print(scale[channel]);
	Serial.print(" offset gues: ");
	Serial.println(offset[channel]);
	*/
	//SCALE
	uint16_t _valueDAC=getToPeriod(channel, _period/2);
	if(_valueDAC==0) _success=false;
	else scale[channel]=_valueDAC-calibrationTable[channel][HIGH_POINT]; //get precise scale
	if(scale[channel]<50 || scale[channel]>2000) _success=false; //check if correct
	//OFFSET
	if(_success){
		//Serial.print(notePeriod[rootNote]>>lowestOctave[channel]);
		_valueDAC=getToPeriod(channel,notePeriod[rootNote]>>lowestOctave[channel]);
		if(_valueDAC==0) _success=false;
		else offset[channel]=_valueDAC; //get precise offset
	}
	if(offset[channel]<1 || offset[channel]>2000) _success=false; //offset 0-2000

	//what is the highest note?
	if(_success){
		highestNote[channel]=60;
		uint16_t _semitoneSize=scale[channel]/12;
		while(((highestNote[channel]*_semitoneSize)+scale[channel])<4095){
			highestNote[channel]++;
			//Serial.print(".");
		}
		highestNote[channel]--;
	}
/*
	Serial.print("highestnote: ");
	Serial.println(highestNote[channel]);
	Serial.println();
	Serial.print(" scale measured: ");
	Serial.print(scale[channel]);
	Serial.print(" offset measured: ");
	Serial.println(offset[channel]);
*/
	if(_success) _success=makeAutoTuneTable(channel);
	return _success;

}
#define MAX_NO_OF_UPDOWNS 8
uint16_t midiSeqHW::getToPeriod(uint8_t channel, uint32_t _destination){

	tuning=true;
	uint16_t dacValue=0;
	bool success=true;
	int _increment=0;
	uint32_t _period=0;
	//Serial.print("E1");
	periodToNote(_destination);
	//Serial.print("E2");
	uint16_t _dacValue=offset[channel]+(scale[channel]/12)*(((octave-lowestOctave[channel])*12)+semitone); //informed guess
	//Serial.print("E3: ");
	//Serial.print(_dacValue);
	writeDAC(channel,_dacValue);
	delay(1);
	//Serial.print(" E4");
	_period=getAveragePeriod(channel);
	//Serial.print("E5");
	uint8_t _tries=0;
	uint16_t tenCents=((_destination)/12)/10;
	bool upDown;
	uint8_t upDownCount=0;

	while(abs(_period-_destination)>tenCents){ //rough approach

		_tries++;
		if(_destination<=_period){
			_increment+=4;
			if(!upDown) upDownCount++;
			upDown=true;
		//	Serial.print("+4,");
		}
		else{
			_increment-=4;
			if(upDown) upDownCount++;
			upDown=false;
		//	Serial.print("-4,");
		}

		if(_tries>MAX_NO_OF_APPROACHES){
			success=false;
		//	Serial.println("<.");
			break;//getScaling(channel), getRoot(channel); //restart - there also needs to be complete escape from the loop
		}
		if(upDownCount>MAX_NO_OF_UPDOWNS) break;
		if(abs(_period-_destination)>(((notePeriod[semitone]>>octave)-(notePeriod[semitone]>>(octave+1)))>>1)){ //bigger error than half octave? give up dude
		//	success=false;
			//break;
		}
		writeDAC(channel,_dacValue+_increment);
		delay(1);
		_period=getAveragePeriod(channel);

		tenCents=((_period)/12)/10;
	}

	_tries=0;
	upDownCount=0;
	if(success){
		uint16_t twoCents=((_destination)/12)/50;
		while(abs(_period-_destination)>twoCents){ //how precise you need to go
			_tries++;
			if(_destination<=_period){
				_increment++;
				if(!upDown) upDownCount++;
				upDown=true;
			//	Serial.print("+,");
			}
			else{
				_increment--;
				if(upDown) upDownCount++;
				upDown=false;
			//	Serial.print("-,");
			}
			if(_tries>MAX_NO_OF_APPROACHES){
				success=false;
				//Serial.println("<.");
				break;//getScaling(channel), getRoot(channel); //restart - maybe get new scaling?
			}

			if(upDownCount>MAX_NO_OF_UPDOWNS) break;
			if(abs(_period-_destination)>(((notePeriod[semitone]>>octave)-(notePeriod[semitone]>>(octave+1)))>>1)){ //bigger error than half octave? give up dude
				//success=false;
				//break;
			}

			writeDAC(channel,_dacValue+_increment);
			//delay(1);
			_period=getAveragePeriod(channel);
		}
	}

	if(success){

		return _dacValue+_increment;
	}
	else return 0;

}

uint16_t midiSeqHW::getOffset(uint8_t channel){

	if(scale[channel]==0){
		getScaling(channel);
	//OFFSET
	}
	return offset[channel]=getToPeriod(channel,notePeriod[rootNote]>>lowestOctave[channel]); //get precise offset

}


void midiSeqHW::init(void(*buttonChangeCallback)(uint8_t number),void(*clockInCallback)(uint8_t number)) {

	cli();

	shiftRegFast::setup();

	//bit_dir_outp(PIN);

	bit_dir_inp(BUTTON_PIN);
	bit_set(BUTTON_PIN);

	bit_dir_inp(DETECT_PIN);
	bit_clear(DETECT_PIN);

	bit_dir_inp(TUNE_PIN_A);
	bit_set(TUNE_PIN_A);

	bit_dir_inp(TUNE_PIN_B);
	bit_set(TUNE_PIN_B);

	bit_dir_inp(TUNE_PIN_C);
	bit_set(TUNE_PIN_C);

	bit_dir_inp(TUNE_PIN_D);
	bit_set(TUNE_PIN_D);

	bit_dir_inp(UPDATE_PIN);
	bit_set(UPDATE_PIN);

	bit_dir_inp(CALIBRATION_PIN_A);
	bit_clear(CALIBRATION_PIN_A);

	bit_dir_inp(CALIBRATION_PIN_B);
	bit_clear(CALIBRATION_PIN_B);


	// store callback pointer for changed buttons
	 this->buttonChangeCallback = buttonChangeCallback;

	 this->clockInCallback = clockInCallback;

	// Disable Timer1 interrupt
	//TIMSK1 &= ~_BV(TOIE1);

	// TIMER 2
	TCCR2A = 0; //(1 << WGM21);  // turn on CTC mode
	TIMSK2 |= (1 << OCIE2A);// enable interrupt
	//TCCR2B = B00000111;	    //prescaler = 1024

	TCCR2B = B0000111; //prescaler = 32
	OCR2A =255;// call on overflow (F_CPU/1024)/(updateFreq); // call
	TCNT2  = 0;

	//external interrupts

	PCICR=0, bitWrite(PCICR,PCIE2,1); //PCINT18,19,20,21
	setPCMask(255); //disable
	PCMSK2=0;
	//(TCNT1H<<8)|TCNT1L

	TIMSK1=0, bitWrite(TIMSK1,0,1);
	TCCR1A=0;
	TCCR1B = B00000010;


	//TIMSK1=;
	//fastAnalogRead::init(); // for analog read
	//knobCount=0;
	//fastAnalogRead::connectChannel(knobCount);
	//fastAnalogRead::startConversion();
	sei();
	SPI.begin();
	fastAnalogRead::init();
	currentAnalogChannel = 0;
	fastAnalogRead::connectChannel(analogPin[currentAnalogChannel]);
	fastAnalogRead::startConversion();


}






/**** BUTTONS ****/



void midiSeqHW::isr_updateButtons() {


	lastButtonHash=buttonHash;

	bitWrite(buttonHash,readButton,!bit_read_in(BUTTON_PIN));

	if(buttonChangeCallback!=0){
		for(uint8_t i=0;i<3;i++){
			if(bitRead(lastButtonHash,i) != bitRead(buttonHash,i)){
				buttonChangeCallback(i);
			}
		}
	}

	readButton++;
	if(readButton>3) readButton=0;
	for(uint8_t i=0;i<4;i++) bitWrite(buffer595[BUT_CS], buttonPin[i],1);
	bitWrite(buffer595[BUT_CS], buttonPin[readButton],0);

	//hw.isr_updateShiftRegisters();
}
bool midiSeqHW::buttonState(uint8_t _but){
	return bitRead(buttonHash,_but);//buttonBit[_but]);
}

bool midiSeqHW::justPressed(uint8_t _but){
	return bitRead(buttonHash,_but) && !bitRead(lastButtonHash,_but);//buttonBit[_but]);
}
bool midiSeqHW::justReleased(uint8_t _but){
	return !bitRead(buttonHash,_but) && bitRead(lastButtonHash,_but);//buttonBit[_but]);
}

void midiSeqHW::isr_updateShiftRegisters(){

	shiftRegFast::write_8bit(buffer595[LED_GATE],shiftRegFast::MSB_FIRST);
	shiftRegFast::write_8bit(buffer595[BUT_CS],shiftRegFast::MSB_FIRST);
	shiftRegFast::write_8bit(buffer595[DISPLAY],shiftRegFast::MSB_FIRST);
	shiftRegFast::enableOutput();

}


/**** TIMING ****/

uint32_t midiSeqHW::getElapsedBastlCycles() {
	return bastlCycles;
}

uint16_t midiSeqHW::getBastlCyclesPerSecond() { //0.5mS
	return (F_CPU/1024)/OCR2A;
}

uint32_t midiSeqHW::getPreciseBastlCycles(){ //times 1.875uS
	uint8_t _counterValue=TCNT2;
	return (bastlCycles<<8)|_counterValue;
}


/**** INTERRUPT ****/
void midiSeqHW::isr_updateAnalogInputs(){
	if(fastAnalogRead::isConversionFinished()){

		analogValues[currentAnalogChannel]=fastAnalogRead::getConversionResult();

		currentAnalogChannel++;
		if(currentAnalogChannel>3) currentAnalogChannel=0;

		fastAnalogRead::connectChannel(analogPin[currentAnalogChannel]);
		fastAnalogRead::startConversion();
	}


}

uint32_t midiSeqHW::getPeriod(){
	return PCperiod;
}

ISR(TIMER1_OVF_vect){
	hw.T1ADD++;
}

uint32_t midiSeqHW::getTime(){
	uint32_t _time=word(TCNT1H,TCNT1L);
	_time=(uint32_t)((uint32_t)T1ADD<<16 | _time);
	return _time;
}

/*
uint32_t midiSeqHW::update(){
	hw.isr_updateDisplay();
	hw.isr_updateButtons();
	hw.isr_updateDAC();
	hw.isr_updateShiftRegisters();
	hw.isr_updateAnalogInputs();
}
*/

void PCinterrupt(){
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){

			hw.PCtime=word(TCNT1H,TCNT1L);
			hw.PCtime=(uint32_t)((uint32_t)hw.T1ADD<<16 | hw.PCtime);//hw.getPreciseBastlCycles(); //((TCNT1H<<8)|TCNT1L);//word(TCNT1H,TCNT1L);//((TCNT1H<<8)|TCNT1L);//hw.getPreciseBastlCycles(); (hw.T1ADD<<16)|((TCNT1H<<8)|TCNT1L)
			if(hw.getTuneState(hw._measuredPin)){ //update period only on rising edge
				if(hw.PCtime>hw.lastPCtime){
					hw.PCperiod=hw.PCtime-hw.lastPCtime;

				//	uint8_t precision=13;
				//	while((hw.PCperiod>>precision)==0) precision--;
					//precision--;
				//	if(abs(hw.runningBuffer[hw.runningCounter%8]-hw.PCperiod)<hw.PCperiod>>precision) hw.runningCounter++; //ensure stability
					hw.runningCounter++;
					hw.runningBuffer[hw.runningCounter%8]=hw.PCperiod;
					hw.runningSum=0;
					for(uint8_t i=0;i<8;i++) hw.runningSum+=hw.runningBuffer[i];
					hw.runningPCtime=hw.runningSum>>3;
				}
				hw.lastPCtime=hw.PCtime;
			}
		}
}
ISR(PCINT2_vect){
	//cli();
	PCinterrupt();
//	sei();
}

ISR(TIMER2_COMPA_vect) { //56us :)


	//bit_set(PIN);
	hw.incrementBastlCycles();
	if(!hw.tuning){
		//TCCR2B = B0001000;

		hw.isr_updateDisplay();
			hw.isr_updateButtons();
			hw.isr_updateDAC();
			hw.isr_updateShiftRegisters();
			hw.isr_updateAnalogInputs();

	}
	else{
		//TCCR2B = B0001111;
		//TCCR2B = B0000010;
	}


	//bit_clear(PIN);


}



/* oldGetScale
	uint16_t octaveResponse=0;
	bool success=true;
	writeDAC(channel,calibrationTable[channel][LOW_POINT]); //1 volt
	delay(1);
	uint32_t _period=getAveragePeriod(channel);
	tuning=true;
	if(_period!=0){
		uint32_t _lowPeriod=_period;
		int _increment=-10;
		writeDAC(channel,calibrationTable[channel][HIGH_POINT]+_increment); //less than 2 volt
		delay(1);
		_period=getAveragePeriod(channel);
		uint8_t _tries=0;
		uint16_t tenCents=((_period)/12)/10;
		while(abs((_lowPeriod>>1)-_period)>tenCents){ //rough approach
			_tries++;
			if(_lowPeriod<=_period<<1){
				_increment+=4;
				Serial.print("+4 ");
			}
			else{
				_increment-=4;
				Serial.print("-4 ");
			}
			if(_tries>MAX_NO_OF_APPROACHES){
				success=false;
				Serial.println("< ");
				break;//getScaling(channel), getRoot(channel); //restart - there also needs to be complete escape from the loop
			}
			writeDAC(channel,calibrationTable[channel][HIGH_POINT]+_increment);
			delay(1);
			_period=getAveragePeriod(channel);
		}
		uint8_t precision=9;
		if(((_lowPeriod-_period)>>precision)==0) precision=8;
		_tries=0;
		uint16_t twoCents=((_period>>1)/12)/50;
		if(success){

			while(abs((_lowPeriod>>1)-_period)>twoCents){ //how precise you need to go
				_tries++;
				if(_lowPeriod<=_period<<1){
					_increment++;
					Serial.print("+ ");
				}
				else{
					_increment--;
					Serial.print("- ");
				}
				if(_tries>MAX_NO_OF_APPROACHES){
					success=false;
					Serial.println("< ");
					break;//getScaling(channel), getRoot(channel); //restart - there also needs to be complete escape from the loop
				}
				writeDAC(channel,calibrationTable[channel][HIGH_POINT]+_increment);
				delay(1);
				_period=getAveragePeriod(channel);
			}
		}
		octaveResponse=abs((calibrationTable[channel][HIGH_POINT]+_increment)-calibrationTable[channel][LOW_POINT]);
		Serial.print("error: ");
		Serial.println(abs((_lowPeriod>>1)-_period));
	}
	else Serial.println("period 0");
	tuning=false;
	if(success){
		scale[channel]=octaveResponse;
		return octaveResponse;
	}
	else return 0;
	*/



/* old getRoot
tuning=true;
uint16_t rootNoteValue=0;
bool success=true;
int _increment=0;
writeDAC(channel,0);//calibrationTable[channel][LOW_POINT]); //1 volt
delay(1);
if(analyseNote(channel)){
	uint16_t _oneSemitone=scale[channel]/12;

	rootNoteValue=(12-semitone)*_oneSemitone;

	writeDAC(channel,rootNoteValue+_increment);
	delay(1);
	uint32_t _period=getAveragePeriod(channel);
	lowestOctave[channel]=octave+1;
	uint16_t _destination = notePeriod[rootNote]>>lowestOctave[channel];
	uint8_t _tries=0;
	uint16_t tenCents=((_destination)/12)/10;
	while(abs(_period-_destination)>tenCents){ //rough approach

		_tries++;
		if(_destination<=_period){
			_increment+=4;
			Serial.print("+4.");
		}
		else{
			_increment-=4;
			Serial.print("-4.");
		}

		if(_tries>MAX_NO_OF_APPROACHES){
			success=false;
			Serial.println("<.");
			break;//getScaling(channel), getRoot(channel); //restart - there also needs to be complete escape from the loop
		}

		writeDAC(channel,rootNoteValue+_increment);
		//Serial.println(rootNoteValue+_increment);
		//delay(1);
		_period=getAveragePeriod(channel);
		tenCents=((_period)/12)/10;
	}
	uint8_t precision=9;
	if((_destination>>precision)==0) precision=8;
//	precision-=2;
	_tries=0;
	if(success){
		uint16_t twoCents=((_destination)/12)/50;
		//Serial.print(twoCents);
		while(abs(_period-_destination)>twoCents){ //how precise you need to go
			_tries++;
			if(_destination<=_period){
				_increment++;
				Serial.print("+.");
			}
			else{
				_increment--;
				Serial.print("-.");
			}
			if(_tries>MAX_NO_OF_APPROACHES){
				success=false;
				Serial.println("<.");
				break;//getScaling(channel), getRoot(channel); //restart - maybe get new scaling?
			}

			writeDAC(channel,rootNoteValue+_increment);
			//delay(1);
			_period=getAveragePeriod(channel);
		}


	}
}
else{
	//Serial.print(_lower);
//	Serial.println("out of range");

}
//uint32_t _lowestPeriod=getAveragePeriod(channel);
//if(_lowestPeriod)
//	scale[channel]=rootNoteValue+_increment;
if(success){
	offset[channel]=rootNoteValue+_increment;
	scaleOffset[channel]=true;
	return rootNoteValue+_increment;
}
else return 0;
*/
