/*
 done

 -hw layer
 -auto calibration
 -frequency measurement method
 	 -check timer1 method
 	 -fix time scale
 	 -running average on period
 	 -tuning table for absolute tuning

 todo


-listening calibration
	-establish voltage to frequency response (scaling) - find how much voltage is one octave - than it is easy to move around
	-read only when stable - needs to be tested more
	-find offset

	-make the approach algorhythm bulletproof

-make precision tests

 -make midi adaptor befor input is fixed
 -fix midi input

 -UI-menu
 -note buffer stucturing
 -polyphony handling
 -cv input implementations

 */

#ifdef EXTERNAL_IDE
#include <Arduino.h>
#include <avr/pgmspace.h>
#include <shiftRegisterFast.h>
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
//MIDI_CREATE_INSTANCE(HardwareSerial, Serial, MIDI) ;
#include "MIDInoteBuffer.h"
//#include<spiShared.h>
//#include "MCPDAC.h"
#include "midiSeqHW.h"
#define DAC_CONFIG 0x30
#define SS_DAC B,2
MIDInoteBuffer buffer;
extern midiSeqHW hw;
//MCPDACClass dac;
bool noteOn[127];
uint8_t currentPreset;
uint8_t clockDivider;
uint8_t priority;
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

//uint16_t tuneTable[22]={0,6, 12,18, 24,30, 36,42, 48,54, 60, 0,410, 819,1229, 1638,2048, 2458,2867, 3277,3686, 4095};
#define TUNE_POINTS 11
void loadTable(){
	for(int i=0;i<22;i++){
		//tuneTable[i]=word(EEPROM.read(i*2),EEPROM.read(1+(i*2)));
	}
}
void saveTable(){
	for(int i=0;i<22;i++){
		//EEPROM.write(i*2,highByte(tuneTable[i]));
	//	EEPROM.write(1+(i*2),lowByte(tuneTable[i]));
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


void updateVoices(){
	for(uint8_t i=0;i<4;i++){
		//Serial.println(buffer.getVoiceNote(i));
		hw.setNote(i,buffer.getVoiceNote(i)); //
		hw.setGate(i,buffer.getVoiceGate(i));
		hw.setLed(i,buffer.getVoiceGate(i));
	}
}

void handleNoteOn(byte channel, byte pitch, byte velocity)
{
	//hw.setLed(0,true);
	hw.displayChar('o');
	//hw.setNote(0,pitch);

	noteOn[pitch]=true;
	//hw.setGate(0,true);
	//hw.setGate(1,false);

	buffer.addNoteToBuffer(pitch,velocity);
	updateVoices();
/*
	for(uint8_t i=0;i<4;i++){
		uint8_t _note=buffer.getVoiceNote(i);
		if(_note!=255) hw.setNote(i,_note), hw.setGate(i,true),hw.setLed(i,true);
		else  hw.setGate(i,false),hw.setLed(i,false); // dac.setNote(i,0),
	}
*/
		//if(pitch>=36 && pitch<=96) dac.writeDAC(channel,curveMap(pitch-36,TUNE_POINTS, tuneTable));
		//digitalWrite(8,LOW);
	//digitalWrite(8,HIGH);
}

void handleNoteOff(byte channel, byte pitch, byte velocity)
{
	hw.displayChar('f');
	//hw.setLed(0,false);
	//hw.setGate(0,false);
	//hw.setGate(1,true);

	buffer.removeNoteFromBuffer(pitch);
	updateVoices();
	//if(buffer.getNumberOfNotesInBuffer()>0) dac.writeDAC(curveMap(buffer.getNoteFromBuffer(0)-36,TUNE_POINTS, tuneTable));
//	else setGate(false);
/*
	noteOn[pitch]=false;
	for(uint8_t i=0;i<4;i++){
		uint8_t _note=buffer.getVoiceNote(i);
		if(_note!=255){
			if(i==3){
				hw.setNote(i,_note),
				hw.setGate(i,true),hw.setLed(i,true);
			}
		}
		else  hw.setGate(i,false),hw.setLed(i,false); // dac.setNote(i,0),
	}

*/

}



uint8_t mode=0;
uint8_t parameter[4];
uint8_t selectedChannel=0;
#define UP 0
#define DOWN 1
#define SET 2
uint8_t count;
bool edit,layoutMode, learn;
uint32_t longPressTime;
bool lookingForEdit=false;
uint8_t layout;
#define NUMBER_OF_LAYOUTS 12
char layoutSign[12]={'p','p','4','4','2','2','1','1','8','8','t','t'};
bool renderClock=false;
uint8_t channelLearn=0;
#define NUMBER_OF_CHANNELS 4
#define NUMBER_OF_EDIT_PAGES 4
uint8_t editPage;
void reflectLayoutChange(){

}
void save(){

}
void load(){

}
#define DIVIDER_PAGE 0
#define BANK_PAGE 1
#define PRIORITY_PAGE 2
#define TUNING_PAGE 3
uint8_t divider;
#define NUMBER_OF_DIVIDERS 5
#define NUMBER_OF_PRESETS 10
#define NUMBER_OF_PRIORITIES 3
void tune(){
	for(uint8_t i=0;i<4;i++){
	//	dac.autoCalibrate(i);
	}
}
void buttonCall(uint8_t number){
	/*
	if(edit){
		if(number==SET && hw.buttonState(number)) {
			longPressTime=millis();
			lookingForEdit=true;
			editPage++, editPage%=NUMBER_OF_EDIT_PAGES;
		}
		switch(editPage){
			case DIVIDER_PAGE:
				if(number==UP && hw.buttonState(UP)){
					if(divider==0) divider=NUMBER_OF_DIVIDERS-1;
					else divider--, divider%=NUMBER_OF_DIVIDERS;
				}
				if(number==DOWN && hw.buttonState(DOWN)) divider++, divider%=NUMBER_OF_DIVIDERS;
				break;
			case BANK_PAGE:
				if(number==UP && hw.buttonState(UP)) {
					save();
					if(currentPreset==0) currentPreset=NUMBER_OF_PRESETS-1;
					else currentPreset--, currentPreset%=NUMBER_OF_PRESETS, load();
				}
				if(number==DOWN && hw.buttonState(DOWN)) save(), currentPreset++, currentPreset%=NUMBER_OF_PRESETS, load();
				break;
			case PRIORITY_PAGE:
				if(number==UP && hw.buttonState(UP)) {
					if(priority==0)  priority=NUMBER_OF_PRIORITIES-1;
					else priority--, priority%=NUMBER_OF_PRIORITIES;
					buffer.setPriority(priority);
				}
				if(number==DOWN && hw.buttonState(DOWN)) priority++, priority%=NUMBER_OF_PRIORITIES;
				break;
			case TUNING_PAGE:
				if(number==UP && hw.buttonState(UP)) tune();
				if(number==DOWN && hw.buttonState(DOWN)) tune();
				break;
		}
	}
	else if(layoutMode){
		if(number==SET && hw.buttonState(SET)) save(),layoutMode=false;
		if(number==UP && hw.buttonState(UP)) {
			if(layout==0) layout=NUMBER_OF_LAYOUTS-1;
			else layout--, layout=layout%NUMBER_OF_LAYOUTS;
			reflectLayoutChange();
		}
		if(number==DOWN && hw.buttonState(DOWN)) layout++, layout=layout%NUMBER_OF_LAYOUTS, reflectLayoutChange();
		if(layout%2) renderClock=true;
		else renderClock=false;
	}
	else if(learn){
		if(number==SET && hw.buttonState(number)) save(), learn=false;
		if(number==UP && hw.buttonState(UP)) {
			if(channelLearn==0) channelLearn=NUMBER_OF_CHANNELS-1;
			else channelLearn--, channelLearn=channelLearn%NUMBER_OF_CHANNELS;
		}
		if(number==DOWN && hw.buttonState(DOWN)) channelLearn++, channelLearn=channelLearn%NUMBER_OF_CHANNELS;
	}
	else{
		if((number==UP && hw.buttonState(SET) && hw.buttonState(number)) || (number==SET && hw.buttonState(UP) && hw.buttonState(number))){
			layoutMode=true;
			hw.setDot(false);
		}
		if((number==DOWN && hw.buttonState(SET) && hw.buttonState(number)) || (number==SET && hw.buttonState(DOWN) && hw.buttonState(number))){
			learn=true;
			hw.setDot(false);
		}
		if(number==SET && hw.buttonState(number)){
			longPressTime=millis();
			lookingForEdit=true;
			hw.setDot(false);
		}
	}
	*/
	/*
if(hw.buttonState(number)){
	switch(number){
	case UP:
		count++;
		count=count%10;
		for(uint8_t i=0;i<4;i++) dac.setNote(i,count*6);
		//for(uint8_t i=0;i<4;i++) hw.setGate(i,false);
		//hw.setGate(count,true);
		break;
	case DOWN:
		count--;
		count=count%10;
		for(uint8_t i=0;i<4;i++) dac.setNote(i,count*6);
		//for(uint8_t i=0;i<4;i++) hw.setGate(i,false);
		//hw.setGate(count,true);
		break;
	case SET:
		break;


	}
	hw.displayNumber(count);
	*/

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

    hw.init(&buttonCall,&clockCall);
   // hw.displayChar('f');
    //saveTable();
  //  loadTable();


	//Serial.begin(38400);
	//Serial.println("start");
	for(int j=0;j<4;j++) hw.autoCalibrate(j);

	for(uint8_t i=0;i<4;i++){
		hw.displayChar('t');
		hw.setLed(i,false);
	//	Serial.print("channel ");
	//	Serial.println(i);
		if(hw.autoTune(i)) hw.setLed(i,true);
		hw.setNote(i,0);
	}
	//hw.tuning=false;


    buffer.init();
    buffer.setPolyphony(4);
    buffer.setPriority(2);
    /*
    for(uint8_t i=0;i<50;i++){
    	buffer.setBuffer(i,random(255));
    		Serial.print(buffer.getBuffer(i));
    		Serial.print(", ");
    	}
    buffer.orderMidiBuffer();
    Serial.println(", ");
    for(uint8_t i=0;i<50;i++){
    	Serial.print(buffer.getBuffer(i));
    	  Serial.print(", ");
    	}
    	Serial.println();
    	*/

    //	Serial.println();
  //  buffer.testOrdering();
   // buffer.init();
    /*

    dac.init(10,9);
    dac.autoTuneInit();
    tune();
    */

  // 	dac.autoCalibrate(3);
   //	Serial.println("calibration_done");
    //load();
}
#define MIDI_CV 0
#define MIDI_SEQ 1
#define MIDI_LOOP 2
#define ARP 3
uint16_t counter=0;
bool alter;
uint32_t alterTime;
//#define VOID 36
uint32_t voidTime;
bool showVoid=false;
char dividerSign[NUMBER_OF_DIVIDERS]={'0','1','2','4','8'};
char prioritySign[NUMBER_OF_PRIORITIES]={'l','h','o'};
void renderDisplay(){
	if(millis()-alterTime>250) alterTime=millis(), voidTime=millis(), alter=!alter;
		if(millis()-voidTime<50) showVoid=true;
		else showVoid=false;
		if(edit){
			switch(editPage){
				case DIVIDER_PAGE:
					if(showVoid){
						hw.displayChar(' ');
					}
					else{
						if(alter) hw.displayChar(dividerSign[divider]);
						else hw.displayChar('d');
					}

					break;
				case BANK_PAGE:
					if(showVoid){
						hw.displayChar(' ');
					}
					else{
						if(alter) hw.displayNumber(currentPreset);
						else hw.displayChar('b');
					}
					break;
				case PRIORITY_PAGE:
					if(showVoid){
						hw.displayChar(' ');
					}
					else{
						if(alter) hw.displayChar(prioritySign[priority]);
						else hw.displayChar('p');
					}
					break;
				case TUNING_PAGE:
					if(showVoid){
						hw.displayChar(' ');
					}
					else{
						if(alter)  hw.displayChar('a');
						else hw.displayChar('t');
					}
					break;
			}
			if(hw.buttonState(SET)){
				if(lookingForEdit){
					if(millis()-longPressTime>4000){
						save(),edit=false;
						lookingForEdit=false;
					}
				}
			}
		}
		else if(layoutMode){
			if(alter) hw.displayChar(layoutSign[layout]);
			else hw.displayChar(' ');
			hw.setDot(layout%2);
		}
		else if(learn){
			if(showVoid) hw.displayChar(' ');
			else hw.displayChar('a'+channelLearn);
		}
		else{
			hw.displayChar(layoutSign[layout]);
			hw.setDot(layout%2);

			if(hw.buttonState(SET)){
				if(lookingForEdit){
					if(millis()-longPressTime>4000){
							edit=true;
							lookingForEdit=false;
					}
				}
			}
		}

		if(!hw.buttonState(SET)) lookingForEdit=false;
}

char text[]="  tyyydaaatadaditaa jamtadamtydytata  jamtadamtaa                ";
uint16_t increment=0;
bool _flop;
void loop()
{
	//if(Serial.available()) increment++, Serial.read();//hw.displayChar('o');

	MIDI.read();
	if(hw.buttonState(1)) buffer.init(), updateVoices();
	//hw.setLed(0,hw.getDetectState());
//	hw.setLed(1,hw.getUpdateState());
	//hw.setMask(0);
	//delay(100);
	//Serial.print(hw.getPeriod());
	//Serial.print(" ,");

	//vypočítat error ve voltech resp v čísle do dacu
//	for(uint8_t i=0;i<1;i++){
/*
	uint8_t i=0;

	if(hw.buttonState(0)){
		hw.displayChar('t');
		for(uint8_t i=0;i<4;i++) hw.setLed(i,false);
		for(uint8_t i=0;i<4;i++){
		//	hw.setLed(i,false);
			if(hw.getScaling(i)){
				if(hw.getRoot(i)) hw.setLed(i,true);
			}


		}
		hw.tuning=false;
	}
	for(uint8_t i=0;i<4;i++) hw.setNote(i,random(6)*12);//random(3)*12);
	delay(800);
	*/

	//hw.displayChar('f');
	//delay(500);
/*
	for(uint8_t i=0;i<hw.highestNote[0];i++){ //hw.highestNote[0]
		hw.setNote(0,i);//i*12);
		hw.setNote(1,i);//i*12);
		hw.setNote(2,i);//i*12);
		hw.setNote(3,i);//i*12);
		delay(1000);
	}
*/
/*
	for(uint8_t i=0;i<1;i++){
	//	hw.setLed(i,false);
		if(hw.getScaling(i)){
			if(hw.getRoot(i)) hw.makeAutoTuneTable(i),hw.setLed(i,true);
		}
	}

	hw.setNote(0,random(6)*12);
	delay(1000);
	*/
//	Serial.println("note:");
//	Serial.println();
	/*
	hw.getToNote(0,12);
	delay(1000);
	hw.getToNote(0,24);
	delay(1000);
	hw.getToNote(0,36);
	delay(1000);
	hw.getToNote(0,48);
	delay(1000);
	*/


	//uint8_t _rnd=random(6);
//random(6)*12);//random(6)*12);//random(6)*12);

	//Serial.println();
	//Serial.println(i);
	//while(!hw.getScaling(i));
//	while(!hw.getRoot(i));


	//}
//	delay(1000);
	/*
	uint16_t _octave=hw.getScaling(0);
	//hw.writeDAC(0,100+(_flop*_octave<<1));
	//Serial.println(_octave);
	//_flop=!_flop;


	hw.writeDAC(0,hw.calibrationTable[0][0]);
	delay(1000);
	Serial.println(hw.getAveragePeriod(0));

	hw.writeDAC(0,hw.calibrationTable[0][0]+_octave);
	delay(1000);
	Serial.println(hw.getAveragePeriod(0));
	Serial.println();
	Serial.println();
	Serial.println();
	*/
	/*
	for(uint8_t i=0;i<4;i++){
		hw.analyseNote(i);
		Serial.print(hw.octave);
		Serial.print(" ,");
		Serial.print(hw.semitone);//hw.runningPCtime);
		Serial.print(" ,");
		Serial.print(hw.above);
		Serial.print("     ");
	}
	Serial.println();
	*/

	//hw.tuning=true;
	//for(uint8_t i=0;i<4;i++) hw.writeDAC(i,(i*1000)+1000);
//	increment++;
//	if(increment>48) increment=0;
//	for(uint8_t i=0;i<4;i++) hw.setNote(i,increment);

	if(hw.tuning){


		hw.isr_updateDisplay();
			hw.isr_updateButtons();
			hw.isr_updateDAC();
			hw.isr_updateShiftRegisters();
			hw.isr_updateAnalogInputs();

	}

	//if(increment>4095) increment=0;
	//for(uint8_t i=0;i<4;i++) hw.setVoltage(i,0);
	//hw.setVoltage(3,increment);
	//Serial.println();

	//dac.autoTuneCalibrateRelative();
//	for(uint8_t i=0;i<4;i++)  Serial.print(dac.measureOneVoltPeriod(i)),Serial.print(", ");
	//Serial.println();

	//if((millis()%100)==0)  Serial.print(dac.tunePeriod[0]),Serial.print(", "), Serial.print(dac.tunePeriod[1]),Serial.print(", "), Serial.print(dac.tunePeriod[2]),Serial.print(", "), Serial.println(dac.tunePeriod[3]);// Serial.print(micros()),Serial.print(", "),
	/*
	 MIDI.read();
	 renderDisplay();
	for(uint8_t i=0;i<4;i++) dac.activitySense(i);
	*/
}




