

#ifdef EXTERNAL_IDE
#include <Arduino.h>
int main(void) {

  init();

  setup();

  while(true) {
    loop();
  }
}

#endif

#include <EEPROM.h>
#include <MIDI.h>
#include <SPI.h>
MIDI_CREATE_INSTANCE(HardwareSerial, Serial, MIDI) ;
#include <MIDInoteBuffer.h>
//#include<spiShared.h>
#include "MCPDAC.h"
#include "midiSeqHW.h"
#define DAC_CONFIG 0x30
#define SS_DAC B,2
MIDInoteBuffer buffer;
extern midiSeqHW hw;
MCPDACClass dac;
bool noteOn[127];

bool anyNoteOn(){
	for(int i=0;i<128;i++){
		if(noteOn[i]) return true;
	}
	return false;
}
void setGate(bool state){
 digitalWrite(8,state);
 digitalWrite(7,state);
}
bool calibrating=false;
uint8_t selectedNote;
//void loa

uint16_t tuneTable[22]={0,6, 12,18, 24,30, 36,42, 48,54, 60, 0,410, 819,1229, 1638,2048, 2458,2867, 3277,3686, 4095};
#define TUNE_POINTS 11
void loadTable(){
	for(int i=0;i<22;i++){
		tuneTable[i]=word(EEPROM.read(i*2),EEPROM.read(1+(i*2)));
	}
}
void saveTable(){
	for(int i=0;i<22;i++){
		EEPROM.write(i*2,highByte(tuneTable[i]));
		EEPROM.write(1+(i*2),lowByte(tuneTable[i]));
	}
}



uint32_t curveMap(uint8_t value, uint8_t numberOfPoints, uint16_t * tableMap){
	uint32_t inMin=0, inMax=255, outMin=0, outMax=255;
	for(int i=0;i<numberOfPoints-1;i++){
		if(value >= tableMap[i] && value <= tableMap[i+1]) {
			inMax=tableMap[i+1];
			inMin=tableMap[i];
			outMax=tableMap[numberOfPoints+i+1];
			outMin=tableMap[numberOfPoints+i];
			i=numberOfPoints+10;
		}
	}
	return map(value,inMin,inMax,outMin,outMax);
}




void handleNoteOn(byte channel, byte pitch, byte velocity)
{
	noteOn[pitch]=true;
	setGate(true);

		buffer.addNoteToBuffer(pitch,velocity);
		//if(pitch>=36 && pitch<=96) dac.writeDAC(channel,curveMap(pitch-36,TUNE_POINTS, tuneTable));
		//digitalWrite(8,LOW);
	//digitalWrite(8,HIGH);
}

void handleNoteOff(byte channel, byte pitch, byte velocity)
{
	buffer.removeNoteFromBuffer(pitch);
	//if(buffer.getNumberOfNotesInBuffer()>0) dac.writeDAC(curveMap(buffer.getNoteFromBuffer(0)-36,TUNE_POINTS, tuneTable));
//	else setGate(false);
	noteOn[pitch]=false;
}

uint8_t mode=0;
uint8_t parameter[4];
uint8_t selectedChannel=0;
void buttonCall(uint8_t number){

	if(number<3){
		if(hw.buttonState(number)){
			selectedChannel=number;
		}
	}
	else if(number<6){
		if(hw.buttonState(number)){

		}
	}
	else if(number==7){
		if(hw.buttonState(number)){
			if(parameter[mode]>0) parameter[mode]--;
			else parameter[mode]=4;
		}
	}
	else if(number==8){
		if(hw.buttonState(number)){
			if(parameter[mode]<4) parameter[mode]++;
			else parameter[mode]=0;
		}
	}
	else if(number==6){
		if(hw.buttonState(number)){
			if(mode<3) mode++;
			else mode=0;
		}
	}
}
bool flop=false;
void clockCall(uint8_t number){
	flop=!flop;
}

void setup(){
	//dacInit();



	MIDI.begin(MIDI_CHANNEL_OMNI);
    MIDI.setHandleNoteOn(handleNoteOn);
    MIDI.setHandleNoteOff(handleNoteOff);

    //saveTable();
  //  loadTable();
    hw.init(&buttonCall,&clockCall);
    buffer.init();
    buffer.setPolyphony(1);

    dac.init(10,9);
   	for(int i=0;i<4;i++){
   		 dac.writeDAC(i,2000);
   	}
}
#define MIDI_CV 0
#define MIDI_SEQ 1
#define MIDI_LOOP 2
#define ARP 3
uint16_t counter=0;
void loop()
{
	/*
	for(int i=0;i<4096;i++){
	writeDAC(i);
	delay(1);
	}*/

	if(counter<4096) counter++;
	else counter=0;
	for(int i=0;i<4;i++){
		 dac.writeDAC(i,counter);
	}
	delay(1);

    MIDI.read();
    hw.setClkOut(hw.buttonState(4));
	for(int i=0;i<3;i++){
		hw.setGateOut(i,hw.buttonState(i));
		if(i==selectedChannel) hw.setLed(i,true);
		else hw.setLed(i,false);
	//	hw.setLed(i,true);
	//	if(i==selectedChannel) hw.dimLed(i,true);
	//	else hw.dimLed(i,false);
	}
	for(int i=3;i<6;i++){
		hw.setLed(i,hw.buttonState(i));
	//	hw.setLed(i,true);
	//	hw.dimLed(i,hw.buttonState(i));
	}
	//hw.setLed(5,flop);
	for(int i=0;i<5;i++){
		if(i==parameter[mode]) hw.setHorLed(i,true);
		else hw.setHorLed(i,false);

		//hw.setHorLed(i,true);
		//if(i==parameter[mode]) hw.dimHorLed(i,true);
		//		else hw.dimHorLed(i,false);
	}
	for(int i=0;i<4;i++){
		if(i==mode) hw.setVerLed(i,true);
		else hw.setVerLed(i,false);

	//	hw.setVerLed(i,true);
	//	if(i==mode) hw.dimVerLed(i,true);
	//	else hw.dimVerLed(i,false);
	}


	switch(mode){
	case MIDI_CV:
		//horizontal=midi2cv mode
		break;
	case MIDI_SEQ:
		//horizontal=pattern
		break;
	case MIDI_LOOP:
		//horizontal=pattern
		break;
	case ARP:
		//horizontal=arp_type
		break;
	}

}




