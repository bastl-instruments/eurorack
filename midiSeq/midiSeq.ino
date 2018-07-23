/*
 done >> test

 -hw layer
 -auto calibration
 -frequency measurement method
 	 -check timer1 method
 	 -fix time scale
 	 -running average on period
 	 -tuning table for absolute tuning
-listening calibration
	-establish voltage to frequency response (scaling) - find how much voltage is one octave - than it is easy to move around
	-read only when stable - needs to be tested more
	-find offset

	-make the approach algorhythm bulletproof
 -polyphony handling
 -note buffer stucturing
  -UI-menu
-quick re-tuning
-implement layouts and their learn modes following the UI

test


 todo



 -implement midi clock and realtime handling

 -implement midi cc
-implement different tuning modes - 440hz 432hz,  microtuning (editor?), just intonation
	-make precision tests

 ____

-cv input calibration - implement ui & test

 -cv input implementations
 	 -portamento slew rendering -reads
 	 -transpose calibration + algo  - after calibration
 	 -quantizer calibration + algo - after calibration
 	 -window function variants - needs more thinking




 -handle sustain pedal


 */

#ifdef EXTERNAL_IDE
#include <Arduino.h>
//#include <avr/pgmspace.h>
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
//#include <SoftwareSerial.h>
//extern SoftwareSerial debug(0, 12); // RX, TX

#include "MIDInoteBuffer.h"
#include "midiSeqHW.h"
#define DAC_CONFIG 0x30
#define SS_DAC B,2

#define TUNE 0
#define SET 1
#define UP 2
#define DOWN 3

MIDInoteBuffer buffer[4];
extern midiSeqHW hw;
#define NUMBER_OF_DIVIDERS 6
#define NUMBER_OF_PRESETS 10
#define NUMBER_OF_PRIORITIES 3
#define NUMBER_OF_MIDI_CHANNELS 16
#define NUMBER_OF_MIDI_NOTES 128
#define NUMBER_OF_MIDI_CCS 128

#define NUMBER_OF_LAYOUTS 18

#define MIDI_CV 0
#define MIDI_SEQ 1
#define MIDI_LOOP 2
#define ARP 3
uint16_t counter=0;
bool alter;
uint32_t alterTime;
uint8_t divider;
//#define VOID 36
uint32_t voidTime;
bool showVoid=false;
const char dividerSign[NUMBER_OF_DIVIDERS]={'0','1','2','4','8','6'};
const char noteSign[12]={'c','c','d','d','e','f','f','g','g','a','a','b'};
const bool noteDot[12]={0,1,0,1, 0,0,1,0, 1,0,1,0};
const char prioritySign[NUMBER_OF_PRIORITIES]={'l','h','o'};
uint8_t pwmCounter;
uint8_t intensity;
uint32_t dimTime;
bool upDown;
uint8_t _increment;
uint32_t clockCounter;
bool sustain;
uint8_t setMicroNote=0;

bool microTonalMode=false;
#define POLY_4 0
#define POLY_3 1
#define MONO_1 2
#define MONO_2 3
#define MONO_4 4
#define TRIG_8 5
#define TRIG_4 6
#define CC_MODE 7
#define QUANT_MODE 8

bool calibrating=false;
uint8_t selectedNote;
//void loa

//uint16_t tuneTable[22]={0,6, 12,18, 24,30, 36,42, 48,54, 60, 0,410, 819,1229, 1638,2048, 2458,2867, 3277,3686, 4095};
#define TUNE_POINTS 11

uint8_t randomLight;
const uint8_t numberOfZones[NUMBER_OF_LAYOUTS/2]={1,2,1,2,4,8,4,8,1};
const uint8_t numberOfBuffers[NUMBER_OF_LAYOUTS/2]={1,2,1,2,4,1,1,1,1};
const uint8_t polyphonyPerZonePerLayout[NUMBER_OF_LAYOUTS/2][4]={{4,0,0,0},{1,3,0,0},{1,0,0,0},{1,1,0,0},{1,1,1,1},{8,0,0,0},{4,0,0,0},{0,0,0,0},{4,0,0,0}};

//variables to be stored in eeprom

uint8_t midiChannel[8];
uint8_t midiNote[8];
uint8_t midiCC[8];
uint8_t ccValue[8];
uint8_t layout;
const uint8_t midiClockDivider[NUMBER_OF_DIVIDERS]={3,6,12,24,48,96};
uint8_t priority;
uint8_t currentPreset;

bool renderClock=false;
uint8_t learnZone=0;
#define NUMBER_OF_CHANNELS 4
#define NUMBER_OF_EDIT_PAGES 4
uint8_t editPage;

const char layoutSign[NUMBER_OF_LAYOUTS/2]={'p','3','1','2','4','8','t','c','q'};

#define INDEX_CH 0
#define INDEX_NO 8
#define INDEX_CC 16

#define INDEX_L 24
#define INDEX_CD 25
#define INDEX_PR 26

#define INDEX_PT 999

#define PRESET_SIZE 50

#define DEFAULT_CH 0
#define DEFAULT_NO 64
#define DEFAULT_CC 64

#define DEFAULT_L 0
#define DEFAULT_CD 0
#define DEFAULT_PR 2

#define CONFIRM_BYTE_1 1000
#define CONFIRM_BYTE_2 1001
#define CONFIRM_BYTE_3 1002
#define CONFIRM_BYTE_1_VAL 105
#define CONFIRM_BYTE_2_VAL 87
#define CONFIRM_BYTE_3_VAL 214

uint8_t mode=0;
uint8_t parameter[4];
uint8_t selectedChannel=0;

uint8_t count;
bool edit,layoutMode, learn;
uint32_t longPressTime;
bool lookingForEdit=false;

#define MICRO_TABLE_OFFSET 800
#define NUMBER_OF_MICRO_TABLES 20
uint8_t microTable;
bool microEditing;

const uint8_t microTonalTables[5][12]={

		{127,127,127,127,127, 127,127,127,127,127,127,127},
		{127,127,127,127,127, 127,127,127,127,127,127,127},
		{127,127,127,127,127, 127,127,127,127,127,127,127},
		{127,127,127,127,127, 127,127,127,127,127,127,127},
		{127,127,127,127,127, 127,127,127,127,127,127,127}

};

void loadMicroTonalTable(uint8_t _microTonalTable){
	for(uint8_t i=0;i<12;i++){
		hw.setMicroTonalTable(i,EEPROM.read(_microTonalTable*12+MICRO_TABLE_OFFSET));
	}

	EEPROM.write(MICRO_TABLE_OFFSET-1,_microTonalTable);
}

void saveMicroTonalTable(uint8_t _microTonalTable){
	for(uint8_t i=0;i<12;i++){
		EEPROM.write(_microTonalTable*12+MICRO_TABLE_OFFSET,hw.getMicroTonalTable(i));
	}
	EEPROM.write(MICRO_TABLE_OFFSET-1,_microTonalTable);
}


void load(){

	for(uint8_t i=0;i<8;i++) {
		midiChannel[i]=EEPROM.read((currentPreset*PRESET_SIZE)+INDEX_CH+i);
		if(midiChannel[i]>=NUMBER_OF_MIDI_CHANNELS) midiChannel[i]=DEFAULT_CH+i;

		midiNote[i]=EEPROM.read((currentPreset*PRESET_SIZE)+INDEX_NO+i);
		if(midiNote[i]>=NUMBER_OF_MIDI_NOTES) midiNote[i]=DEFAULT_NO+i;

		midiCC[i]=EEPROM.read((currentPreset*PRESET_SIZE)+INDEX_CC+i);
		if(midiCC[i]>=NUMBER_OF_MIDI_CCS) midiCC[i]=DEFAULT_CC+i;

	}

	layout=EEPROM.read((currentPreset*PRESET_SIZE)+INDEX_L);
	if(layout>=NUMBER_OF_LAYOUTS) layout=DEFAULT_L;

	divider=EEPROM.read((currentPreset*PRESET_SIZE)+INDEX_CD);
	if(divider>=NUMBER_OF_DIVIDERS) divider=DEFAULT_CD;

	priority=EEPROM.read((currentPreset*PRESET_SIZE)+INDEX_PR);
	if(priority>=NUMBER_OF_PRIORITIES) priority=DEFAULT_PR;

	microTable=EEPROM.read(MICRO_TABLE_OFFSET-1);
	if(microTable>=NUMBER_OF_MICRO_TABLES) microTable=0;
	loadMicroTonalTable(microTable);


}

void save(){
	for(uint8_t i=0;i<8;i++) {
		EEPROM.write((currentPreset*PRESET_SIZE)+INDEX_CH+i,midiChannel[i]);
		EEPROM.write((currentPreset*PRESET_SIZE)+INDEX_NO+i,midiNote[i]);
		EEPROM.write((currentPreset*PRESET_SIZE)+INDEX_CC+i,midiCC[i]);
	}

	EEPROM.write((currentPreset*PRESET_SIZE)+INDEX_L,layout);
	EEPROM.write((currentPreset*PRESET_SIZE)+INDEX_CD,divider);
	EEPROM.write((currentPreset*PRESET_SIZE)+INDEX_PR,priority);

	EEPROM.write(INDEX_PT,currentPreset);
}


void loadSetting(){

	//is memory formated already?
	if(EEPROM.read(CONFIRM_BYTE_1)==CONFIRM_BYTE_1_VAL && EEPROM.read(CONFIRM_BYTE_2)==CONFIRM_BYTE_2_VAL && EEPROM.read(CONFIRM_BYTE_3)==CONFIRM_BYTE_3_VAL ){
		//
		currentPreset=EEPROM.read(INDEX_PT);
		load();
	}
	//if not format the memory
	else{
		for (uint8_t j=0; j<NUMBER_OF_PRESETS;j++){
			currentPreset=j;
			for(uint8_t i=0;i<8;i++) {
				midiChannel[i]=DEFAULT_CH+i;
				midiNote[i]=DEFAULT_NO+i;
				midiCC[i]=DEFAULT_CC+i;
			}
			layout=DEFAULT_L;
			divider=DEFAULT_CD;
			priority=DEFAULT_PR;

			save();
		}

		currentPreset=0;
		load();

		EEPROM.write(CONFIRM_BYTE_1,CONFIRM_BYTE_1_VAL);
		EEPROM.write(CONFIRM_BYTE_2,CONFIRM_BYTE_2_VAL);
		EEPROM.write(CONFIRM_BYTE_3,CONFIRM_BYTE_3_VAL);

		for(uint8_t j=0;j<5;j++){
			for(uint8_t i=0;i<12;i++){
				hw.setMicroTonalTable(i,microTonalTables[j][i]);
			}
			saveMicroTonalTable(j);
		}
		for(uint8_t j=5;j<NUMBER_OF_MICRO_TABLES;j++){
			for(uint8_t i=0;i<12;i++){
				hw.setMicroTonalTable(i,microTonalTables[0][i]);
			}
			saveMicroTonalTable(j);
		}
	}

}



/*
bool anyNoteOn(){
	for(int i=0;i<128;i++){
		if(noteOn[i]) return true;
	}
	return false;
}
*/



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

void updateClock(){
	if(layout%2) renderClock=true;
	else renderClock=false;

	if(renderClock){
		bool resetState=clockCounter<midiClockDivider[divider];
		bool clockState=(clockCounter%midiClockDivider[divider])<(midiClockDivider[divider]/2);
		hw.setDacGate(4,resetState); //
		hw.setGate(4,clockState);
		hw.setLed(4,clockState);
	}
}
void updateVoices(){
	if(microTonalMode){
		for(uint8_t i=0;i<4;i++){
			hw.setNote(i,setMicroNote);
			hw.setGate(i,buffer[0].getVoiceGate(i));
			hw.setLed(i,buffer[0].getVoiceGate(i));
		}
	}
	else{
		if(layout%2) renderClock=true;
		else renderClock=false;

		switch(layout/2){
			case POLY_4:
				if(buffer[0].getWindowPosition()==0){
					for(uint8_t i=0;i<4-renderClock;i++){
						hw.setNote(i,buffer[0].getVoiceNote(i));
						hw.setGate(i,buffer[0].getVoiceGate(i));
						hw.setLed(i,buffer[0].getVoiceGate(i));
					}
				}
				else {
					for(uint8_t i=0;i<4-renderClock;i++){
						//Serial.println(buffer.getVoiceNote(i));
						hw.setNote(i,buffer[0].getWindowNote(i,buffer[0].getWindowPosition()));
						hw.setGate(i,buffer[0].getVoiceGate(i));
						hw.setLed(i,buffer[0].getVoiceGate(i));
					}
				}
				break;

			case POLY_3:

				hw.setNote(0,buffer[0].getVoiceNote(0)); //
				hw.setGate(0,buffer[0].getVoiceGate(0));
				hw.setLed(0,buffer[0].getVoiceGate(0));

				for(uint8_t i=0;i<3-renderClock;i++){
					hw.setNote(i+1,buffer[1].getVoiceNote(i)); //
					hw.setGate(i+1,buffer[1].getVoiceGate(i));
					hw.setLed(i+1,buffer[1].getVoiceGate(i));
				}
				break;

			case MONO_1:

				hw.setNote(0,buffer[0].getVoiceNote(0)); //
				hw.setGate(0,buffer[0].getVoiceGate(0));
				hw.setLed(0,buffer[0].getVoiceGate(0));
				//setVelocity
				//aftertouch
				//cc
				//triggers
				break;
			case MONO_2:

				hw.setNote(0,buffer[0].getVoiceNote(0)); //
				hw.setGate(0,buffer[0].getVoiceGate(0));
				hw.setLed(0,buffer[0].getVoiceGate(0));
				//setVelocity
				//triggers
				hw.setNote(2,buffer[1].getVoiceNote(0)); //
				hw.setGate(2,buffer[1].getVoiceGate(0));
				hw.setLed(2,buffer[1].getVoiceGate(0));
				//if renderClock
					//setVelocity
					//triggers
				break;
			case MONO_4:

				hw.setNote(0,buffer[0].getVoiceNote(0)); //
				hw.setGate(0,buffer[0].getVoiceGate(0));
				hw.setLed(0,buffer[0].getVoiceGate(0));

				hw.setNote(1,buffer[1].getVoiceNote(0)); //
				hw.setGate(1,buffer[1].getVoiceGate(0));
				hw.setLed(1,buffer[1].getVoiceGate(0));

				hw.setNote(2,buffer[2].getVoiceNote(0)); //
				hw.setGate(2,buffer[2].getVoiceGate(0));
				hw.setLed(2,buffer[2].getVoiceGate(0));
				if(!renderClock){
					hw.setNote(3,buffer[3].getVoiceNote(0)); //
					hw.setGate(3,buffer[3].getVoiceGate(0));
					hw.setLed(3,buffer[3].getVoiceGate(0));
				}

				break;
			case TRIG_8:


				hw.setGate(0,buffer[0].isNoteInBuffer(midiNote[0]));
				hw.setGate(1,buffer[0].isNoteInBuffer(midiNote[1]));
				hw.setGate(2,buffer[0].isNoteInBuffer(midiNote[2]));

				hw.setDacGate(0,buffer[0].isNoteInBuffer(midiNote[4])); //
				hw.setDacGate(1,buffer[0].isNoteInBuffer(midiNote[5])); //
				hw.setDacGate(2,buffer[0].isNoteInBuffer(midiNote[6])); //

				hw.setLed(0,buffer[0].isNoteInBuffer(midiNote[0])); //do some PWM shit here
				hw.setLed(1,buffer[0].isNoteInBuffer(midiNote[1]));
				hw.setLed(2,buffer[0].isNoteInBuffer(midiNote[2]));


				if(!renderClock){

					hw.setGate(3,buffer[0].isNoteInBuffer(midiNote[3]));
					hw.setLed(3,buffer[0].isNoteInBuffer(midiNote[3]));

					hw.setDacGate(3,buffer[0].isNoteInBuffer(midiNote[7])); //
				}

				break;
			case TRIG_4:


				hw.setGate(0,buffer[0].isNoteInBuffer(midiNote[0]));
				hw.setLed(0,buffer[0].isNoteInBuffer(midiNote[0]));


				hw.setGate(1,buffer[0].isNoteInBuffer(midiNote[1]));
				hw.setLed(1,buffer[0].isNoteInBuffer(midiNote[1]));


				hw.setGate(2,buffer[0].isNoteInBuffer(midiNote[2]));
				hw.setLed(2,buffer[0].isNoteInBuffer(midiNote[2]));


				// this should happen at noteOn i guess

			//	hw.setVoltage(0,buffer[0].getVoiceVelocity(4));
				//hw.setVoltage(1,buffer[0].getVoiceGate(5)); //
				//hw.setVoltage(2,buffer[0].getVoiceGate(6)); //



				if(!renderClock){

					hw.setGate(3,buffer[0].isNoteInBuffer(midiNote[3]));
					hw.setLed(3,buffer[0].isNoteInBuffer(midiNote[3]));

				//	hw.setVoltage(3,buffer[0].getVoiceVelocity(7)); //
					//hw.setDacGate(3,buffer[0].getVoiceGate(7)); //
				}

				break;

			case CC_MODE:
				for(uint8_t i=0;i<4-renderClock;i++){
					hw.setGate(i,ccValue[i]>63);
					hw.setLed(i,ccValue[i]>63);
					hw.setVoltage(i,ccValue[i+4]<<5);
				}
				break;

			case QUANT_MODE: //bigQuestionMark here

				break;


		}
	}

	updateClock();
}

bool quantizerNote[12];

void handleNoteOn(byte channel, byte pitch, byte velocity)
{
	setMicroNote=pitch;

	if(learn){
		if(layout/2<=MONO_4) midiChannel[learnZone]=channel;
		else if(layout/2<=TRIG_4) midiChannel[learnZone]=channel, midiNote[learnZone]=pitch;
		else if(layout/2==QUANT_MODE){
			quantizerNote[pitch%12]=true;
		}
	}
	else{
		for(uint8_t i=0;i<numberOfBuffers[layout/2];i++){
			if(channel==midiChannel[i] || midiChannel[i]==0) buffer[i].addNoteToBuffer(pitch,velocity);
		}
		hw.setDot(true);

		for(uint8_t i=0;i<4;i++){
		//	buffer[i].addNoteToBuffer(pitch,velocity);
		}

		if(!hw.getDetectState()) updateVoices();
	}
}
void handlePitchBend(byte channel, int bend){
	for(uint8_t i=0;i<4;i++) hw.setPitchBend(i,bend);
	if(!hw.getDetectState()) updateVoices();
}
void handleNoteOff(byte channel, byte pitch, byte velocity)
{
	hw.setDot(false);
	for(uint8_t i=0;i<numberOfBuffers[layout/2];i++){
		if(channel==midiChannel[i] || midiChannel[i]==0) buffer[i].removeNoteFromBuffer(pitch);
	}
	//for(uint8_t i=0;i<4;i++) buffer[i].removeNoteFromBuffer(pitch);
	if(!hw.getDetectState()) updateVoices();

}





void reflectLayoutChange(){
	for(uint8_t i=0;i<numberOfBuffers[layout/2];i++){
		buffer[i].init();
		buffer[i].setPolyphony(polyphonyPerZonePerLayout[layout/2][i]);
		buffer[i].setPriority(priority);
	}
	updateVoices();
}

#define DIVIDER_PAGE 0
#define BANK_PAGE 1
#define PRIORITY_PAGE 2
#define TUNING_PAGE 3


void tune(){
	for(uint8_t i=0;i<4;i++){
	//	dac.autoCalibrate(i);
	}
}
void buttonCall(uint8_t number){


}

bool flop=false;
void clockCall(uint8_t number){
	flop=!flop;
}

void handleCC(byte channel, byte number, byte value){

	if(number==64) sustain=value>63;
	if((layout/2)==CC_MODE){

		 if(learn){
			 midiCC[learnZone]=number;
			 midiChannel[learnZone]=number;

		 }

	 for(uint8_t i=0;i<8;i++){
		 if(midiChannel[i]==channel && midiCC[i]==number) ccValue[i]=value, updateVoices();
	 }

 }


}

void handleClock(){
	clockCounter++;
	updateClock();
}

void handleStart(){
	clockCounter=0;
	updateClock();
}

void handleStop(){
	clockCounter=0;
	updateClock();
}

void handleContinue(){
	updateClock();
}

 void handleAfterTouchPoly( byte channel, byte note, byte pressure){


 }

 void handleAfterTouchChannel( byte channel, byte pressure){


 }

 void handleProgramChange( byte channel, byte number){


 }

void handleTuneRequest(){

}

void handleSysEx(byte * array, byte size){

}

void setup(){

	//debug.begin(38400);
	//debug.println("test");
	MIDI.begin(MIDI_CHANNEL_OMNI);

    MIDI.setHandleNoteOn(handleNoteOn);
    MIDI.setHandleNoteOff(handleNoteOff);

    MIDI.setHandleControlChange(handleCC);
    MIDI.setHandlePitchBend(handlePitchBend);

    MIDI.setHandleAfterTouchPoly(handleAfterTouchPoly);
    MIDI.setHandleAfterTouchChannel(handleAfterTouchChannel);
    MIDI.setHandleProgramChange(handleProgramChange);

    MIDI.setHandleClock(handleClock);
    MIDI.setHandleStart(handleStart);
    MIDI.setHandleStop(handleStop);
    MIDI.setHandleContinue(handleContinue);

    MIDI.setHandleTuneRequest(handleTuneRequest);
    MIDI.setHandleSystemExclusive(handleSysEx);





    hw.init(&buttonCall,&clockCall);
    hw.setPitchBendRange(7);

    //saveTable();
  //  loadTable();


	//Serial.begin(38400);
	//Serial.println("start");

	for(int j=0;j<4;j++) hw.autoCalibrate(j);
	loadSetting();
	reflectLayoutChange();



    //load();
}

bool shortBlink;
void renderDisplay(){

	if(millis()-dimTime>2){
		dimTime=millis();
		intensity++;
		if(intensity==0) upDown=!upDown;
	}

	if(millis()-alterTime>300) alterTime=millis(), voidTime=millis(), alter=!alter, _increment++,randomLight=random(255);
		if(millis()-voidTime<50) showVoid=true;
		else showVoid=false;

		if(millis()-voidTime<100) shortBlink=true;
		else shortBlink=false;
		if(microTonalMode){
			if(microEditing){
				hw.displayChar(noteSign[setMicroNote%12]);
				hw.setDot(noteDot[setMicroNote%12]);
			}
			else{
				if(showVoid){
					hw.displayChar(' ');
				}
				else{
					if(alter) hw.displayNumber(microTable%10);
					else hw.displayChar('y');
				}
			//	if(shortBlink) hw.displayChar('y');
				//else hw.displayNumber(microTable%10);
				hw.setDot(microTable/10);
			}

		}
		else if(edit){
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
		 //uses fake PWM so make it faster alright?

		}
		else if(learn){

		}
		else{
			hw.displayChar(layoutSign[layout/2]);
			hw.setDot(layout%2);

			if(hw.buttonState(SET) && !hw.buttonState(UP) && !hw.buttonState(DOWN)){ //avoid interaction
				if(lookingForEdit){
					if(millis()-longPressTime>4000){
							edit=true;
							lookingForEdit=false;
					}
				}
			}
		}

		if(!hw.buttonState(SET) || hw.buttonState(UP) || hw.buttonState(DOWN)) lookingForEdit=false; //avoid interaction

		pwmCounter+=4;

		if(layoutMode){

			if(alter) hw.displayChar(layoutSign[layout/2]);
			else hw.displayChar(' ');

			hw.setDot(layout%2);
			for(uint8_t i=0;i<4;i++) hw.setLed(i,false);


			switch(layout/2){
				case POLY_4:
					for(uint8_t i=0;i<4;i++) hw.setLed(i,true);
					break;
				case POLY_3:
					for(uint8_t i=1;i<4;i++) hw.setLed(i,true);
					hw.setLed(0,!alter);
					break;
				case MONO_1:
					hw.setLed(0,alter);
					//hw.setLed(3,!alter);
					break;
				case MONO_2:
					hw.setLed(0,alter);
					hw.setLed(2,!alter);
					break;

				case MONO_4:
					hw.setLed(_increment%4,true);
					break;

				case TRIG_8:
					hw.setLed(_increment%4,showVoid);
					break;

				case TRIG_4:
					for(uint8_t i=0;i<4;i++) hw.setLed(i,showVoid & (pwmCounter>randomLight));
					//for(uint8_t i=0;i<4;i++) hw.setLed(i,showVoid & (pwmCounter>((_increment%8)<<2)));
					break;

				case CC_MODE:
					for(uint8_t i=0;i<4;i++) hw.setLed(i,upDown ^ (pwmCounter>intensity));

					break;
				case QUANT_MODE:
					for(uint8_t i=0;i<4;i++) hw.setLed(i,(pwmCounter>randomLight));
					break;

			}
			if(layout%2)  hw.setLed(3,!(intensity%32));
		}
		if(learn){

			if(showVoid) hw.displayChar(' ');
			else if((layout/2)<5){
				if(_increment%3==1) hw.displayNumber(midiChannel[learnZone]);
				else if (_increment%3==0)  hw.displayNumber(midiChannel[learnZone]/10);
				else hw.displayChar(' ');
			}
			else if((layout/2)==CC_MODE){
				if(_increment%3==1) hw.displayNumber(midiCC[learnZone]);
				else if (_increment%3==0) hw.displayNumber(midiCC[learnZone]/10);
				else if (midiCC[learnZone]>=100) hw.displayChar('1');
				else hw.displayChar(' ');
			}
			//else if((layout/2)==QUANT_MODE) hw.displayNumber(midiCC[learnZone]);
			else {
				if(_increment%3==1) hw.displayNumber(midiNote[learnZone]);
				else if (_increment%3==0) hw.displayNumber(midiNote[learnZone]/10);
				else if (midiNote[learnZone]>=100) hw.displayChar('1');
				else hw.displayChar(' ');
			}

			if(learnZone>=4){
				for(uint8_t i=0;i<4;i++) hw.setLed(i,!(i+4==learnZone) && pwmCounter>230);
			}
			else{
				for(uint8_t i=0;i<4;i++) hw.setLed(i,i==learnZone);

			}
		}
}



uint16_t increment=0;
bool _flop;
bool updateState=false;
bool windowState=false;
bool tuneState=false;




bool combo=false;

void renderMenu(){



	if(microTonalMode){


		if(microEditing){
			if(!combo && hw.justReleased(SET)) microEditing=false,saveMicroTonalTable(microTable);

			if(hw.justPressed(UP)) {
				hw.setMicroTonalTable(setMicroNote%12,hw.getMicroTonalTable(setMicroNote%12)+2);
				updateVoices();
			}
			if(hw.justPressed(DOWN)) {
				hw.setMicroTonalTable(setMicroNote%12,hw.getMicroTonalTable(setMicroNote%12)-2);
				updateVoices();
			}
		}
		else{
			if(hw.buttonState(SET) && hw.justPressed(TUNE)) microEditing=true;
			if(!combo && hw.justReleased(SET)) microTonalMode=false, saveMicroTonalTable(microTable), combo=true;

			if(hw.justPressed(UP)) {
				saveMicroTonalTable(microTable);
				microTable++;
				if(microTable>=NUMBER_OF_MICRO_TABLES) microTable=0;
				loadMicroTonalTable(microTable);
			}
			if(hw.justPressed(DOWN)) {
				saveMicroTonalTable(microTable);
				if(microTable==0) microTable=NUMBER_OF_MICRO_TABLES-1;
				else microTable--;
				loadMicroTonalTable(microTable);
			}
		}
	}
	else if(edit){
		if(hw.justPressed(SET)) {
			longPressTime=millis();
			lookingForEdit=true;
			editPage++, editPage%=NUMBER_OF_EDIT_PAGES;
		}
		switch(editPage){
			case DIVIDER_PAGE:
				if(hw.justPressed(UP)){
					if(divider==0) divider=NUMBER_OF_DIVIDERS-1;
					else divider--, divider%=NUMBER_OF_DIVIDERS;
					 save();
				}
				if(hw.justPressed(DOWN)) divider++, divider%=NUMBER_OF_DIVIDERS, save();
				break;
			case BANK_PAGE:
				if(hw.justPressed(UP)) {
					save();
					if(currentPreset==0) currentPreset=NUMBER_OF_PRESETS-1;
					else currentPreset--, currentPreset%=NUMBER_OF_PRESETS, load();
				}
				if(hw.justPressed(DOWN)) save(), currentPreset++, currentPreset%=NUMBER_OF_PRESETS, load();
				break;
			case PRIORITY_PAGE:
				if(hw.justPressed(UP)) {
					if(priority==0)  priority=NUMBER_OF_PRIORITIES-1;
					else priority--, priority%=NUMBER_OF_PRIORITIES;
					for(uint8_t i=0;i<4;i++) buffer[i].setPriority(priority);
					 save();
				}
				if(hw.justPressed(DOWN)) priority++, priority%=NUMBER_OF_PRIORITIES, save();
				break;
			case TUNING_PAGE: //tuning system

					//440hz
					//432hz
					//just
					//micro
					//

					//
				//if(hw.justPressed(UP)) tune();
				//if(hw.justPressed((DOWN)) tune();
				break;
		}
	}
	else if(layoutMode){
		if(hw.justPressed(SET)) save(),layoutMode=false;
		if(hw.justPressed(UP)) {
			if(layout==0) layout=NUMBER_OF_LAYOUTS-1;
			else layout--, layout=layout%NUMBER_OF_LAYOUTS;
			reflectLayoutChange();
		}
		if(hw.justPressed(DOWN)) layout++, layout=layout%NUMBER_OF_LAYOUTS, reflectLayoutChange();
		if(layout%2) renderClock=true;
		else renderClock=false;
	}
	else if(learn){
		if(hw.justPressed(SET)) save(), learn=false, learnZone=0;
		if(hw.justPressed(UP)) {
			if(learnZone==0) learnZone=numberOfZones[layout/2]-1;
			else learnZone--, learnZone=learnZone%numberOfZones[layout/2];
		}
		if(hw.justPressed(DOWN)) learnZone++, learnZone=learnZone%numberOfZones[layout/2];
	}
	else{
		if(hw.buttonState(SET) && hw.justPressed(TUNE)) microTonalMode=true, combo=true;;

		if( (hw.justPressed(UP) && hw.buttonState(SET))  || (hw.justPressed(SET) && hw.buttonState(UP) )){
			learn=true;
			learnZone=0;
			hw.setDot(false);
			combo=true;

		}
		if( (hw.justPressed(DOWN) && hw.buttonState(SET))  || (hw.justPressed(SET) && hw.buttonState(DOWN) )){
			layoutMode=true;
			hw.setDot(false);
			combo=true;
		}
		if(hw.justPressed(SET)){ //avoid interaction?
			longPressTime=millis();
			lookingForEdit=true;
			hw.setDot(false);
		}

	}

	bool allUp=true;
	for(uint8_t i=0;i<4;i++){
		if(hw.buttonState(i)) allUp=false;
	}
	if(allUp) combo=false;

}

uint8_t _counter=0;
void renderUpdate(){
	bool newUpdateState= hw.getUpdateState();
	if(!updateState && newUpdateState) updateVoices();
	updateState=newUpdateState;
}
uint16_t portamento=0;
void renderPortamento(){
	uint16_t newPortamento=constrain( map(hw.getAnalogValue(1),512,0,0,1000) ,0,1000);
	if(portamento!=newPortamento) portamento=newPortamento,hw.setSlew(portamento);
}
void renderWindow(){
	bool newWindowState=(hw.getAnalogValue(0)<300);
	if(!windowState && newWindowState){
		for(uint8_t i=0;i<4;i++) buffer[i].increaseWindowPosition();
		if(!hw.getDetectState()) updateVoices();
	}
	windowState=newWindowState;
}
void renderHW(){
	if(hw.tuning){


		hw.isr_updateDisplay();
		hw.isr_updateButtons();
		hw.isr_updateDAC();
		hw.isr_updateShiftRegisters();
		hw.isr_updateAnalogInputs();

	}
}
bool alreadyTuned=false;

void tuneAllVoices(){
	for(uint8_t i=0;i<4;i++){
		hw.displayChar('t');
		hw.setDot(true);
		hw.isr_updateDisplay();
		hw.setLed(i,false);
	//	Serial.print("channel ");
	//	Serial.println(i);
		if(hw.autoTune(i)) hw.setLed(i,true);
		hw.setNote(i,0);
	}
	if(!hw.getDetectState()) updateVoices();
}
uint32_t longTunePressTime;

void renderTuneButton(){
	if(!hw.buttonState(SET)){
		if(hw.justPressed(TUNE)){//if(!tuneState && newTuneState){
			longTunePressTime=millis();
		}

		if(hw.justReleased(TUNE)){
			if(alreadyTuned && (millis()-longTunePressTime<2000)){
				hw.displayChar('t');
				hw.setDot(false);

				hw.isr_updateDisplay();
				for(uint8_t i=0;i<4;i++){
					hw.setLed(i,false);
					if(hw.isTuned(i)) {
						if(hw.reAutoTune(i)) hw.setLed(i,true);
						else hw.autoTune(i);
					}
					else hw.autoTune(i);
				}
				if(!hw.getDetectState()) updateVoices();
			}
			else{
				tuneAllVoices();
				alreadyTuned=true;
			}
		}
	}

}

void loop()
{
	//if(hw.buttonState(1)) buffer.init(), updateVoices(); = panic

	MIDI.read();
	renderDisplay();
	//hw.setDot(hw.getUpdateState());
	renderMenu();
	renderUpdate();
	renderWindow();
	renderTuneButton();
	renderHW();
	renderPortamento();


}




