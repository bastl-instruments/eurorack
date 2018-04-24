

#ifdef EXTERNAL_IDE
#include <Arduino.h>
#include <utilities.h>
#include <interface.h>
#include <dacRoutine.h>
#include<analogSeqHW.h>
extern analogSeqHW hw;
/*
 *
 * TODO
 *


 * *-oscilator tuner mode

 *


 *
 *  -dual
 * settings transfer master>slave dual
 * dual  - sync step
 * dual  - two cursors
 * cv inputs in dual
 *
 *
 * test
 *  * *-input CV calibration -
 * *	bootup menu -
 *  * -output CV calibration procedure
 * random step implementation
 *  *jump to step by pressing button with FN
 *  * tuned CV input implementation
 	 * negative range
 	 *
 	  invert minMAJ with expander
 *
 *
 * exit dual
 * - CV input implementation

 *-center position of  step knobs
 *-center -slide time and gate time curveMap



 *
 *	different dual setup visualisation - done
 *	tune input - done
 *
 *	trigger latency, 1 done
 *	reset latency, 1 done
 *	1 ready to be defined
 *	tuner, 1 done

for revision fix also
- active step v dual modu - cv update
- jump v dual modu
- trigger invert v dual modu

 *
 */
int main(void) {

  init();

  setup();

  while(true) {
    loop();
  }
}

#endif

#include <EEPROM.h>
//#include <MIDI.h>
//MIDI_CREATE_INSTANCE(HardwareSerial, Serial, MIDI)  ;
//#include <MIDInoteBuffer.h>
//#include<spiShared.h>
#include <simpleSerialDecoder.h>
simpleSerialDecoder com;
#include<spiShared.h>
//#include<analogSeqHW.h>
#include <Line.h>
Line <float> line;
extern analogSeqHW hw;
uint8_t bootByte;
bool booting=false;
bool interaction=false;
bool interactionS=false;

//MIDInoteBuffer buffer;


//MIDInoteBuffer buffer;
uint16_t gateMap[10]={0,63,127,191,255,   10,100,500,1000,4000};
uint16_t slideMap[10]={0,63,127,191,255,   10,100,500,1000,4000};
#define SLIDE_MAP_POINTS 5
#define GATE_MAP_POINTS 5

uint8_t triggerLatency=2;
bool calibrationAllowed=true;
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

//uint16_t cvOutCalibrate[15]={312, 645, 974, 1304, 1634, 1964, 2303, 2627, 2961, 3293, 3625, 0, 0, 0,0};
//uint16_t tuneIn[15]={480, 503, 525, 548, 571, 594, 617, 640, 662, 685, 708, 0, 0,0,0 }
uint16_t tuneInNeg[15]={480, 503, 525, 548, 571, 594, 617, 640, 662, 685, 708, 0, 0,0,0 };

uint16_t cvOutCalibrate[15]={85, 456, 837, 1214, 1593, 1974, 2346, 2717, 3093, 3484, 3848, 0, 0, 0,0}; //valentinos popcorn <3
uint16_t tuneIn[15]={482, 530, 578, 627, 675, 723, 771, 818, 866, 916, 962, 0, 0, 0,0}; //valentinos popcorn <3



uint32_t dualTableCurveMap(uint16_t value, uint8_t numberOfPoints, uint16_t * tableMap,uint16_t * tableMap2){
	uint16_t inMin=tuneIn[0], inMax=tuneIn[10], outMin=cvOutCalibrate[0], outMax=cvOutCalibrate[10];
	for(uint8_t i=0;i<numberOfPoints-1;i++){
		if(value >= tableMap[i] && value <= tableMap[i+1]) {
			inMax=tableMap[i+1];
			inMin=tableMap[i];
			outMax=tableMap2[i+1];
			outMin=tableMap2[i];
			i=numberOfPoints+10;
		}
	}
	return map(value,inMin,inMax,outMin,outMax);
}

void software_Reset() // Restarts program from beginning but does not reset the peripherals and registers
{
  asm volatile ("  jmp 0");
}

uint8_t numberOfSteps=8;
bool dual=false;
bool master=true;

#define PAGE 0
#define FN 2

#define UP 1
#define DOWN 3

#define BUTTON_A 4
#define BUTTON_B 5


uint16_t potValue[2][2];
uint16_t otherKnobs[8];
bool otherGates[8];
bool otherSlides[8];

#define NUMBER_OF_CV_IN_DESTINATIONS 11

#define TRANSPOSE 0 //PROPPER MAPPING -bipolar DONE
#define QUANTIZED_TRANSPOSE 1 //PROPPER MAPPING -bipolar DONE

#define OFFSET 2 //deep test - potřebuje hlubši definici  -bipolar
#define RESET_OFFSET 3   //deep test -bipolar
#define TRIG_A_SHIFT 4  //deep test -bipolar
#define TRIG_B_SHIFT 5 //deep test -bipolar
#define SLIDE_TIME 6  //deep test -bipolar
#define GATE_TIME 7  //deep test -bipolar

#define SHIFT_INVERT 8 //deep test -gate
#define MIN_MAJ 9 //deep test -gate
#define RANDOM_STEP 10 //TODO
#define RESET_TO_4 11 //deep test - has to come prior to trigger - trigger


/*
 * GATE_TIME
 * MIN_MAJ
 * OFFSET
 * RESET_TO_4
 *
 */
int currentCvValue,lastCvValue;
bool currentCvGate=false;
uint8_t cvInDestination=0;
uint8_t dualMode=0;

void setGate(bool state){
 digitalWrite(8,state);
 digitalWrite(7,state);
}
//bool calibrating=false;
uint8_t selectedNote;
//void loa


#define TUNE_POINTS 11
void loadTable(){
	for(int i=0;i<11;i++){
		cvOutCalibrate[i]=word(EEPROM.read(100+(i*2)),EEPROM.read(100+1+(i*2)));
		tuneIn[i]=word(EEPROM.read(200+(i*2)),EEPROM.read(200+1+(i*2)));
		//Serial.print(tuneIn[i]);
		//Serial.print(", ");
	}
	//Serial.println();
	for(int i=0;i<11;i++){
		//if(tuneIn[i]-tuneIn[0]>0)
		tuneInNeg[i]=constrain((int)tuneIn[0]-(int)(tuneIn[i]-tuneIn[0]),0,1024);
	//	Serial.print(tuneInNeg[i]);
		//Serial.print(", ");
	}
	//Serial.println();

}
void saveTable(){
	for(int i=0;i<11;i++){
		EEPROM.write(100+(i*2),highByte(cvOutCalibrate[i]));
		EEPROM.write(100+1+(i*2),lowByte(cvOutCalibrate[i]));

		EEPROM.write(200+(i*2),highByte(tuneIn[i]));
		EEPROM.write(200+1+(i*2),lowByte(tuneIn[i]));
	}
}





#define CHROMATIC 0
#define MAJOR 1
#define HARMONIC_MINOR 2
#define MELODIC_MINOR 3
#define PENTATONIC_MAJOR 4
#define PENTATONIC_MINOR 5
#define BLUES 6
#define WHOLE_TONE 7

bool scales[13][12]{
		{1,1,1,1,1,1,1,1,1,1,1,1}, //chroma

		{1,0,1,0,1,1,0,1,0,1,0,1}, //maj dia
		{1,0,1,0,1,0,0,1,0,1,0,0}, //maj penta
		{1,0,1,1,1,0,0,1,0,1,0,0}, //maj blues C, D, D♯/E♭, E, G, A
		//{1,0,1,0,1,0,1,0,1,0,1,0}, //whole tone
		{1,0,0,0,1,0,0,1,0,0,0,0}, //maj chord
		{1,0,0,0,0,0,0,1,0,0,0,0}, //maj fifth
		{1,0,0,0,1,0,0,1,0,0,1,0}, //maj +7

		{1,0,1,1,0,1,0,1,1,0,1,0}, //min dia
		{1,0,0,1,0,1,0,1,0,0,1,0}, //min penta
		{1,0,0,1,0,1,1,1,0,0,1,0}, //min blues
		{1,0,0,1,0,0,0,1,0,0,0,0}, //min chord
		{1,0,1,0,1,0,1,0,1,0,1,0}, //whole tone scale
		{1,0,0,1,0,0,0,1,0,0,1,0}, //min +7
};

bool quantizeTable[12];

uint8_t quantizeNote(uint8_t _scale,uint8_t note){
	if(scales[_scale][note%12]){
		return note;
	}
	else{
		uint8_t higher=0;
		while(!scales[_scale][(note+higher)%12]){
			higher++;
		}
		uint8_t lower=0;
		while(!scales[_scale][(note-lower)%12]){
			lower++;
		}
		if(higher<lower) return note+higher;
		else return note-lower;
	}
	return note;
}


bool wait=false;
bool flop[8];
void channelModeCall(uint8_t channel, uint8_t value){
	wait=false;
}
bool gate[8];
bool slide[8];
int step=0;
int step2=0;

uint8_t trigAshift;
uint8_t trigBshift;
uint32_t gateTime;
uint32_t gateTimeStart;
uint32_t slideTimeStart;
uint16_t slideFrom;
bool calculateLine=false;
uint16_t out;
uint16_t realOut;
int virtualStep,offsetStep;
bool jump=false;
uint32_t scheduleTimeA,scheduleTimeB;

void showBiLed(uint8_t _AB, uint8_t _value){

	switch(_value){

	case 0:
			hw.setBiLed((_AB*3)+0,true,false);
			hw.setBiLed((_AB*3)+1,true,false);
			hw.setBiLed((_AB*3)+2,true,false);
			break;
	case 1:
			hw.setBiLed((_AB*3)+0,false,false);
			hw.setBiLed((_AB*3)+1,false,false);
			hw.setBiLed((_AB*3)+2,true,false);
			break;
	case 2:
			hw.setBiLed((_AB*3)+0,false,false);
			hw.setBiLed((_AB*3)+1,true,false);
			hw.setBiLed((_AB*3)+2,false,false);
			break;
	case 3:
			hw.setBiLed((_AB*3)+0,true,false);
			hw.setBiLed((_AB*3)+1,false,false);
			hw.setBiLed((_AB*3)+2,false,false);
			break;
	case 4:
			hw.setBiLed((_AB*3)+0,true,true);
			hw.setBiLed((_AB*3)+1,false,true);
			hw.setBiLed((_AB*3)+2,false,true);
			break;
	case 5:
			hw.setBiLed((_AB*3)+0,false,true);
			hw.setBiLed((_AB*3)+1,true,true);
			hw.setBiLed((_AB*3)+2,false,true);
			break;
	case 6:
			hw.setBiLed((_AB*3)+0,false,true);
			hw.setBiLed((_AB*3)+1,false,true);
			hw.setBiLed((_AB*3)+2,true,true);
			break;
	case 7:
			hw.setBiLed((_AB*3)+0,true,true);
			hw.setBiLed((_AB*3)+1,true,true);
			hw.setBiLed((_AB*3)+2,true,true);
			break;

	case 8:
			hw.setBiLed((_AB*3)+0,false,false);
			hw.setBiLed((_AB*3)+1,false,false);
			hw.setBiLed((_AB*3)+2,false,false);
			break;
	}

}
uint8_t resetLatency=0;
bool gatedPitch=false;
bool scheduleA=false,scheduleB=false, scheduleReset=false,scheduleReset4=false;
uint32_t scheduleResetTime;
uint32_t scheduleReset4Time;
uint16_t expanderOffset;
uint16_t bitOffset;
bool myTurn=false;
uint32_t myTurnTime;

void clockCall(uint8_t number){
	//Serial.println(number);
	uint8_t jumpTo=255;
	if(jump){
		for(int i=0;i<8;i++){
			if(hw.buttonState(i)) jumpTo=i;
		}
	}
	if(jumpTo!=255){
		virtualStep = jumpTo;
		step=virtualStep;
	}
	else{
		switch(number){
			case 0: //RST
				if(resetLatency>0){
					scheduleResetTime=hw.getElapsedBastlCycles();
					scheduleReset=true;
				}
				else{
					clockCall(10);
				}
				break;

			case 10:
				if(cvInDestination==RESET_OFFSET){
					virtualStep=constrain(map(currentCvValue,0,256,0,numberOfSteps),-7,7);
					while(virtualStep<0) virtualStep=numberOfSteps+virtualStep;
					//virtualStep=0;
					//clockCall(4);
				}
				else virtualStep=0;
				break;
			case 1:
				if(triggerLatency>0){
					scheduleTimeA=hw.getElapsedBastlCycles();
					scheduleA=true;
				}
				else clockCall(11);
				break;
			case 11:
				uint8_t shiftA;
				if(trigAshift>3) shiftA=trigAshift-3, virtualStep+=shiftA;
				else shiftA=4-trigAshift, virtualStep-=shiftA;
				if(virtualStep>=numberOfSteps) virtualStep%=numberOfSteps;
				if(virtualStep<0) virtualStep=numberOfSteps+virtualStep;
				break;
			case 2:
				if(triggerLatency>0){
					scheduleTimeB=hw.getElapsedBastlCycles();
					scheduleB=true;
				}
				else clockCall(12);
				break;
			case 12:
				//trigB
				uint8_t shiftB;
				if(trigBshift>3) shiftB=trigBshift-3, virtualStep+=shiftB;
				else shiftB=4-trigBshift, virtualStep-=shiftB;
				if(virtualStep>=numberOfSteps) virtualStep%=numberOfSteps;
				if(virtualStep<0) virtualStep=numberOfSteps+virtualStep;

				break;
			case 3: //resetTo4
				if(resetLatency>0){
					scheduleReset4Time=hw.getElapsedBastlCycles();
					scheduleReset4=true;
				}
				else{
					clockCall(13);
				}
				break;
			case 13:
				virtualStep=4;
				break;
			case 4: //offset
				offsetStep=map(constrain((int)currentCvValue+(int)(expanderOffset>>2)+(int)(bitOffset>>2),-255,255),0,256,0,numberOfSteps);
				break;
			case 5: //offset
				if(cvInDestination == OFFSET) offsetStep=map(constrain((int)(expanderOffset>>2)+(int)(bitOffset>>2)+(int)currentCvValue,-255,255),0,256,0,numberOfSteps);
				else offsetStep=map(expanderOffset+bitOffset,0,1024,0,numberOfSteps); //
				break;

			case 30:
				virtualStep=(0-offsetStep)%numberOfSteps;
				break;
			case 31:
				virtualStep=(1-offsetStep)%numberOfSteps;
				break;
			case 32:
				virtualStep=(2-offsetStep)%numberOfSteps;
				break;
			case 33:
				virtualStep=(3-offsetStep)%numberOfSteps;
				break;
			case 34:
				virtualStep=(4-offsetStep)%numberOfSteps;
				break;
			case 35:
				virtualStep=(5-offsetStep)%numberOfSteps;
				break;
			case 36:
				virtualStep=(6-offsetStep)%numberOfSteps;
				break;
			case 37:
				virtualStep=(7-offsetStep)%numberOfSteps;
				break;


		}
		step=virtualStep+offsetStep;
	}
	if(step>=numberOfSteps) step=step%numberOfSteps;
	while(step<0) step=numberOfSteps+step;
	if(number>3){


		slideFrom=realOut;
		gateTimeStart=hw.getElapsedBastlCycles();
		if(slide[step]) calculateLine=true,slideTimeStart=hw.getElapsedBastlCycles();
		if(dual){
			myTurn=false;
			myTurnTime=hw.getElapsedBastlCycles();
			com.sendPairMessage();
			com.sendStep(step+10);//send(step);
		}
	}
}
bool shift;
uint8_t range=5;


bool cvInput=false;
bool quantize=false;
uint8_t quantizeType=0;
uint8_t scale=0;
bool major=true;
bool cvInMode=false;
uint8_t slideByte, gateByte;
#define PV_00 700
#define PV_01 701
#define PV_10 702
#define PV_11 703
void saveSettings(){
	for(int i=0;i<8;i++){
		bitWrite(slideByte,i,slide[i]);
		bitWrite(gateByte,i,gate[i]);
	}
	if(interaction){
		EEPROM.write(0,quantize);
		EEPROM.write(1,quantizeType);
		EEPROM.write(2,cvInDestination);
		EEPROM.write(3,scale);
		EEPROM.write(4,range);
		EEPROM.write(5,major);
		EEPROM.write(6,gateByte);
	}
	if(shift) EEPROM.write(7,slideByte);
	else EEPROM.write(8,bootByte);

	if(shift){
		EEPROM.write(PV_00,lowByte(potValue[0][1]));
		EEPROM.write(PV_01,highByte(potValue[0][1]));
		EEPROM.write(PV_10,lowByte(potValue[1][1]));
		EEPROM.write(PV_11,highByte(potValue[1][1]));
	}

}
#define CONFIRM_BYTE_1 303
#define CONFIRM_BYTE_2 304
#define CONFIRM_BYTE_3 305
#define CONFIRM_BYTE_1_VAL 105
#define CONFIRM_BYTE_2_VAL 87
#define CONFIRM_BYTE_3_VAL 214
#define BOOT_BYTE_DEFAULT 1
void loadSettings(){
	//just for
/*
	quantize=0;
		quantizeType=0;
		cvInDestination=0;
		scale=0;
		range=0;
		major=0;
		gateByte=0;
		slideByte=0;
		bootByte=BOOT_BYTE_DEFAULT;
		saveSettings();
		EEPROM.write(CONFIRM_BYTE_1,CONFIRM_BYTE_1_VAL);
		EEPROM.write(CONFIRM_BYTE_2,CONFIRM_BYTE_2_VAL);
		EEPROM.write(CONFIRM_BYTE_3,CONFIRM_BYTE_3_VAL);
*/

	//check for first time load
	if(EEPROM.read(CONFIRM_BYTE_1)==CONFIRM_BYTE_1_VAL && EEPROM.read(CONFIRM_BYTE_2)==CONFIRM_BYTE_2_VAL && EEPROM.read(CONFIRM_BYTE_3)==CONFIRM_BYTE_3_VAL ){
		quantize=constrain(EEPROM.read(0),0,1);
		quantizeType=constrain(EEPROM.read(1),0,2);
		cvInDestination=constrain(EEPROM.read(2),0,NUMBER_OF_CV_IN_DESTINATIONS);
		scale=constrain(EEPROM.read(3),0,2);
		range=constrain(EEPROM.read(4),0,5);
		if(range ==1 || range ==2 || range ==5) ;
		else range =2;
		major=constrain(EEPROM.read(5),0,1);
		gateByte=EEPROM.read(6);
		slideByte=EEPROM.read(7);
		bootByte=EEPROM.read(8);

		for(int i=0;i<8;i++){
			slide[i]=bitRead(slideByte,i);
			gate[i]=bitRead(gateByte,i);
		}
		potValue[0][1]=word(EEPROM.read(PV_01),EEPROM.read(PV_00));
		potValue[1][1]=word(EEPROM.read(PV_11),EEPROM.read(PV_10));
		if(potValue[0][1]>1023 || potValue[0][1]<0) potValue[0][1]=0, saveSettings();
		if(potValue[1][1]>1023 || potValue[1][1]<0) potValue[1][1]=0, saveSettings();
		loadTable();
	}
	else{
		quantize=0;
		quantizeType=0;
		cvInDestination=0;
		scale=0;
		range=1;
		major=0;
		gateByte=0;
		slideByte=0;
		bootByte=BOOT_BYTE_DEFAULT;
		saveSettings();
		saveTable();
		EEPROM.write(CONFIRM_BYTE_1,CONFIRM_BYTE_1_VAL);
		EEPROM.write(CONFIRM_BYTE_2,CONFIRM_BYTE_2_VAL);
		EEPROM.write(CONFIRM_BYTE_3,CONFIRM_BYTE_3_VAL);
		EEPROM.write(PV_00,0);
		EEPROM.write(PV_01,0);
		EEPROM.write(PV_10,0);
		EEPROM.write(PV_11,0);


	}

}

uint8_t selekt;
bool pair=false;


void tuneInput(){
	loadTable();
//	Serial.println("tune");
	for(uint8_t i=0;i<11;i++){
		writeDAC(cvOutCalibrate[i]);
		//uint32_t _time=hw.getElapsedBastlCycles();
		//while(hw.getElapsedBastlCycles()-_time<100);
		for(int j=0;j<40;j++) hw.isr_updateButtons();
		delayMicroseconds(2000);
		tuneIn[i]=hw.getCV(0);
		delayMicroseconds(2000);
//		Serial.println(tuneIn[i]);

	}
	saveTable();
	showBiLed(0,7);

}
bool vOctTuner=false;
bool fnJump=false;
void buttonCall(uint8_t number){

	if(number==8){
		if(booting){
			if(hw.buttonState(8)) booting=false,saveSettings();
		}
		if(pair){
			if(hw.buttonState(8)){
				if(dual){
					dual=false, numberOfSteps=8;
					com.sendPairMessage();
					com.sendTrigger(7);
				}
				else{
					dual=true;
					master=false;
					com.sendPairMessage();
					com.sendTrigger(2);
				//	numberOfSteps=16;
					interaction=true;
				}
			}
		}
		hw.freezeAllKnobs();
		if(hw.buttonState(number)){
			for(int i=0;i<8;i++){
				if(hw.buttonState(i)){
					clockCall(30+i);
					fnJump=true;
				}
			}
		}
		if(interactionS) saveSettings(), interactionS=false; // test !
		if(!fnJump){ // this can cause miss step also
			if(!hw.buttonState(number)){
				if(interaction) interaction=false;
				else shift=!shift;
				com.sendPairMessage();
				com.sendTrigger(hw.buttonState(8));
			}
			else{
				interaction=false;
				com.sendPairMessage();
				com.sendTrigger(hw.buttonState(8));
			}
		}
		cvInMode=false;

	}
	else if(cvInMode){
		if(number==3){
			if(hw.buttonState(3)){
				cvInDestination++;
				if(cvInDestination>=NUMBER_OF_CV_IN_DESTINATIONS) cvInDestination=0;
			}
		}
	}
	else if(number==9){
		//jump=hw.jumpState();
	}
	else{
		if(hw.buttonState(8)){
			if(hw.buttonState(number) && !fnJump){
				switch(number){
				case 0:
					range=5;
					break;
				case 1:
					range=2;
					break;
				case 2:
					range=1;
					break;
				case 3:
					cvInMode=true;
					break;
				case 4:
					quantize=!quantize;
					break;
				case 5:
					quantizeType++;
					if(quantizeType>2) quantizeType=0;
					break;
				case 6:
					major=!major;
					break;
				case 7:
					scale++;
					if(scale>2) scale=0;
					break;
				}
				//saveSettings();
				interaction=true;
				interactionS=true;
			}
		}
		else if(jump){
			if(hw.buttonState(number)){
				clockCall(5);
			}
		}
		else if(booting){
			if(hw.buttonState(7)){
				//tuneInput();
				showBiLed(0,7);
				delay(500);
				EEPROM.write(606,1),software_Reset(), vOctTuner=true;
			}
			else if(number<3){
				switch(number){
				case 0:
					bitWrite(bootByte,0,1);
					bitWrite(bootByte,1,0);
					bitWrite(bootByte,2,0);
					break;
				case 1:
					bitWrite(bootByte,0,0);
					bitWrite(bootByte,1,1);
					bitWrite(bootByte,2,0);
					break;
				case 2:
					bitWrite(bootByte,0,0);
					bitWrite(bootByte,1,0);
					bitWrite(bootByte,2,1);
					break;
				}
			}
			else if(hw.buttonState(number)) bitWrite(bootByte,number,!bitRead(bootByte,number));
		}
		else{
			if(!hw.buttonState(number) && !fnJump){
				if(shift==false){
					gate[number]=!gate[number];
				}
				else{
					slide[number]=!slide[number];
				}
				interactionS=true;
			}
		}
	}
}
#define _POINT 0
#define _MINUS 1
#define _PLUS 2
#define _REFUSE 3
uint8_t whichPoint=0;
bool flopy;
bool calibrating=false;
uint8_t lastNumber;

#define COM_RATE 5
uint8_t whatToSend;
void renderCommunication(){
	uint8_t _hash=0;
	if(myTurn){
		if(hw.getElapsedBastlCycles()-myTurnTime>COM_RATE){
			if(master && dualMode==0){
				if(whatToSend<16) whatToSend++;
				else whatToSend=0;
			}
			else{
				if(whatToSend<10) whatToSend++;
				else whatToSend=0;
			}

			switch(whatToSend){
			case 8: //gates
				for(int i=0;i<8;i++){
					bitWrite(_hash,i,gate[i]);
				}
				com.sendPairMessage();
				com.sendGate(_hash);
				break;
			case 9: //slides
				for(int i=0;i<8;i++){
					bitWrite(_hash,i,slide[i]);
				}
				com.sendPairMessage();
				com.sendSelect(_hash);
				break;


			case 10: // slide time
				com.sendPairMessage();
				com.sendStart(potValue[0][1]);
				break;
			case 11: // gate time
				com.sendPairMessage();
				com.sendStop(potValue[1][1]);
				break;
			case 12: //settings
				com.sendPairMessage();
					com.sendChannelTrigger(0,quantize);
					break;
			case 13:
				com.sendPairMessage();
					com.sendChannelTrigger(1,quantizeType);
					break;
			case 14:
				com.sendPairMessage();
					com.sendChannelTrigger(2,cvInDestination);
					break;
			case 15:
				com.sendPairMessage();
					com.sendChannelTrigger(3,scale);
					break;
			case 16:
				com.sendPairMessage();
					com.sendChannelTrigger(4,range);
					break;
			case 17:
				com.sendPairMessage();
				com.sendChannelTrigger(5,major);
				//com.sendSelect(_hash);
				break;
			default:
				if(whatToSend<8){
					com.sendPairMessage();
					com.sendChannelValue(whatToSend,highByte(hw.getKnobValue(whatToSend)),lowByte(hw.getKnobValue(whatToSend)));
				}
				break;
			}
			myTurn=false;
		}
	}
	else{
		if(hw.getElapsedBastlCycles()-myTurnTime>100){
			myTurn=true;
			myTurnTime=hw.getElapsedBastlCycles();
			renderCommunication();
		}
	}
}
void channelTriggerCall(uint8_t channel, uint8_t number){
	calibrationAllowed=false;
	if(dualMode==0){
		switch(channel){
			case 0:
			quantize=constrain(number,0,1);
				break;
			case 1:
			quantizeType=constrain(number,0,2);
				break;
			case 2:
			//cvInDestination=number;
				break;
			case 3:
			scale=constrain(number,0,2);
				break;
			case 4:
			range=constrain(number,0,2);
				break;
			case 5:
			major=constrain(number,0,1);
				break;
		}
	}
	myTurn=true;
	myTurnTime=hw.getElapsedBastlCycles();
}
void stepCall(uint8_t number){
	calibrationAllowed=false;
	if(dualMode==1){
		step2=constrain(number-10,0,15);
	}
	else{
		step=number-10;
		virtualStep=step-offsetStep; //virtualStep //potreba udelat transfer offsetStep z mastru do slavu....
		if(virtualStep>=numberOfSteps) virtualStep%=numberOfSteps;
		if(virtualStep<0) virtualStep=numberOfSteps+virtualStep;
		slideFrom=realOut;
		gateTimeStart=hw.getElapsedBastlCycles();
		if(slide[step]) calculateLine=true,slideTimeStart=hw.getElapsedBastlCycles();
	}
	myTurn=true;
	myTurnTime=hw.getElapsedBastlCycles();

}
void gateCall(uint8_t number){
	calibrationAllowed=false;
	for(int i=0;i<8;i++){
		otherGates[i]=bitRead(number,i);
	}
	myTurn=true;
	myTurnTime=hw.getElapsedBastlCycles();
}
void selectCall(uint8_t number){
	calibrationAllowed=false;
	for(int i=0;i<8;i++){
		otherSlides[i]=bitRead(number,i);
	}
	myTurn=true;
	myTurnTime=hw.getElapsedBastlCycles();
}
void channelValueCall(uint8_t channel, uint8_t value, uint8_t number){
	calibrationAllowed=false;
	otherKnobs[channel]=constrain(word(value,number),0,1023);
	myTurn=true;
	myTurnTime=hw.getElapsedBastlCycles();
}
void triggerCall(uint8_t number){
	calibrationAllowed=false;
	if(number==0) pair=false;
	if(number==1) pair=true;
	if(number==2){ //dual mode
		interaction=true;
		if(dual==true) dual=false,numberOfSteps=8;
		else{
			dual=true,master=true, numberOfSteps=16;
			myTurn=true;
			myTurnTime=hw.getElapsedBastlCycles();
			interaction=true;
			for(int i=0;i<3;i++){
				if(bitRead(bootByte,i)) dualMode=i;
			}
			if(dualMode==2) numberOfSteps=8;
			com.sendPairMessage();
			com.sendTrigger(3+dualMode);
		}
	}
	if(number==3){
		master=false;
		dual=true;
		dualMode=0;
		numberOfSteps=16;
	}
	if(number==4){
		master=false;
		dual=true;
		dualMode=1;
		numberOfSteps=16;
	}
	if(number==5){
		master=false;
		dual=true;
		dualMode=2;
		numberOfSteps=8;
	}
	if(number==7){
		if(dual){
			com.sendPairMessage();
			com.sendTrigger(7);
		}
		dual=false;
		master=false;
		numberOfSteps=8;
	}
}
void startCall(uint8_t number){ //slide time
	calibrationAllowed=false;
	potValue[0][1]=number;
	myTurn=true;
	myTurnTime=hw.getElapsedBastlCycles();

}
void stopCall(uint8_t number){ //gate time
	calibrationAllowed=false;
	potValue[1][1]=number;
	myTurn=true;
	myTurnTime=hw.getElapsedBastlCycles();
}

void interpolateCall(uint8_t channel, uint8_t number){
	//flopy=!flopy;
//	hw.setShiftLed(flopy);
if(calibrationAllowed){ //1){//
 calibrating=true;
 //if(number>10) calibrating=false;
	switch(channel){
		case _POINT:
			number-=10;
			if(number==0){
			//	for(int i=0;i<15;i++) cvOutCalibrate[i]=0;
			}
			flopy=!flopy;
			hw.setShiftLed(flopy);
			//saveTable();
			if(lastNumber==number){

				saveTable();
				if(number==10){
					calibrating=false;
					com.sendPairMessage();
					com.sendChannelInterpolate(_REFUSE,number);
					//loop();
					saveTable();
					while(!hw.buttonState(3)){
						showBiLed(0,0);
						delay(200);
						hw.allLedsOff();
						delay(200);
					}

					tuneInput();
					showBiLed(0,7);
					delay(500);
				 }

			}
			else{

				if(number>10){
					 calibrating=false;
					 com.sendPairMessage();
					com.sendChannelInterpolate(_REFUSE,number);
					//loop();
				 }

			}
			whichPoint=number;
			//if(whichPoint<11)
		//	writeDAC(cvOutCalibrate[whichPoint]);

			for(int i=0;i<6;i++){

				if(number%6==i){
					if(number>=6) hw.setBiLed(i,true,false);
					else hw.setBiLed(i,true,true);
				}
				//else if ((i-6)==number) hw.setBiLed(i-6,true,true);
				else hw.setBiLed(i,false,false);

			}
			com.sendPairMessage();
			com.sendChannelInterpolate(_POINT,number);
			lastNumber=number;
			break;
		case _MINUS:
			if(calibrating){
				if(cvOutCalibrate[whichPoint]-number>0){
					if(whichPoint<11) cvOutCalibrate[whichPoint]-=number;
					writeDAC(cvOutCalibrate[whichPoint]);
				}
			}
			break;
		case _PLUS:
			if(calibrating){
				if(cvOutCalibrate[whichPoint]+number<=4095){
					if(whichPoint<11) cvOutCalibrate[whichPoint]+=number;
					writeDAC(cvOutCalibrate[whichPoint]);
				}
				else{

					cvOutCalibrate[whichPoint]=0;
					writeDAC(cvOutCalibrate[whichPoint]);
				}
			}
			break;
	}
}

}
const uint8_t tune[]={0,2,4,5, 7,9,11,7, 12,12,12,12, 11,11,11,11, 9,9,9,9, 7,7,7,7, 5,9,5,2, 4,7,4,0, 2,2,2,2, 7,7,7,7, 5,9,5,2, 4,7,4,0, 2,2,2,2, 0,0,0,0};
uint32_t measureTime;
uint32_t measureInterval=1000000;
uint32_t measureUP[3]={0,0,0};
uint32_t measuredTime[3][10];
uint32_t measureResult[3]={0,0,0};


#define NUMBER_OF_MEAUREMENTS 8
void vOctTune(){



	hw.pinInit();
	hw.isr_updateTriggerStates();
	hw.readyForCvIn();
	//hw.isr_updateButtons();
	//cli();
	bit_dir_inp(C,5);
	bit_set(C,5);
	bit_dir_inp(B,1);
	bit_set(B,1);
	bit_dir_inp(C,1);
	bit_set(C,1);
	delay(20);
	if(!bit_read_in(C,1) && measureUP[0]!=0){
		for(int i=0;i<56;i++){
			writeDAC(map(tune[i],0,12,cvOutCalibrate[2],cvOutCalibrate[4]));
			showBiLed(0,i%8);
			showBiLed(1,7-(i%8));
			delay(250);
		}
	}
	else{
	bool inState;


	for(int i=0;i<3;i++){
		writeDAC(cvOutCalibrate[(i*2)+2]);
		delay(20);

		//for(int j=0;j<1;j++){

	//		hw.isr_updateTriggerStates();
//			hw.isr_updateButtons();

			//hw.getElapsedBastlCycles();
			inState=bit_read_in(B,1);
			measureTime=micros();
			uint8_t period=0;
			while(((micros()-measureTime)<measureInterval) && period<NUMBER_OF_MEAUREMENTS){


				bool newState=bit_read_in(B,1);
				if(newState && !inState){
					if(period==0){
						measureUP[i]=micros();
						period++;
					}
					else{
						measuredTime[i][period]=micros()-measureUP[i];
						measureUP[i]=micros();
						period++;
					}
				}
				inState=newState;
			}
			for(int k=0;k<NUMBER_OF_MEAUREMENTS;k++){
				measureResult[i]+=measuredTime[i][k];
			}
			measureResult[i]/=NUMBER_OF_MEAUREMENTS;
		//}
	}
	//hw.init(&buttonCall,&clockCall);
	hw.allLedsOff();
	if(measureResult[0]==0 || measureResult[1]==0 || measureResult[2]==0){
		showBiLed(1,0);
	}
		else{

	long difference1=measureResult[0]-(2*measureResult[1]);
	long difference2=measureResult[1]-(2*measureResult[2]);

	long finalError=(difference1+difference2)/2;
	if(abs(finalError)<30){
		hw.allLedsOff();
		hw.setLed(4,true);
		showBiLed(0,7);
		showBiLed(1,7);

	}
	else{
		hw.allLedsOff();
		hw.setLed(constrain(map(finalError,-600,600,0,9),0,7),true);
		showBiLed(0,0);
		showBiLed(1,0);
	}
	//	Serial.print(measureResult[0]), Serial.print(", "), Serial.print(measureResult[1]),Serial.print(", "), Serial.println(measureResult[2]);
	}
//	hw.isr_updateTriggerStates();
	for(int i=0;i<3;i++) measureResult[i]=0;
	}


}
void bootMenu(){
	while(booting){
		if(vOctTuner){
			vOctTune();
		}
		else{
			for(int i=0;i<8;i++){
				hw.setLed(i,bitRead(bootByte,i));
			}
		}
	}
}
bool cvInExpMode=false;
void setup(){

	if(EEPROM.read(606)==1){
		booting=true;
		vOctTuner=true;
		EEPROM.write(606,0);
		dacInit();
		loadSettings();
		bootMenu();
	}




	calibrationAllowed=true;
	hw.init(&buttonCall,&clockCall);
	dacInit();
	com.init(38400);
		//com.attachChannelCVCallback(&channelCVCall);
	com.attachStepCallback(&stepCall);
	com.attachTriggerCallback(&triggerCall);
	com.attachStartCallback(&startCall);
	com.attachStopCallback(&stopCall);
	com.attachGateCallback(&gateCall);
	com.attachSelectCallback(&selectCall);
	com.attachChannelInterpolateCallback(&interpolateCall);
	com.attachChannelValueCallback(&channelValueCall);
	com.attachChannelTriggerCallback(&channelTriggerCall);
	//saveTable();
	loadSettings();

	delay(10);
	if(hw.buttonState(8)){
		booting=true,bootMenu();
		hw.unfreezeKnob(0),hw.unfreezeKnob(1);
	}
	if(!hw.buttonState(8)) hw.unfreezeKnob(0),hw.unfreezeKnob(1);

	if(bitRead(bootByte,4)) triggerLatency=2;
	else triggerLatency=0;
	if(bitRead(bootByte,5)) resetLatency=2;
	else resetLatency=0;
	if(bitRead(bootByte,6)) cvInExpMode=true;
	else cvInExpMode=false;
	if(bitRead(bootByte,3)) gatedPitch=true;
	else gatedPitch=false;
	/*
	Serial.print("cvOutCalibrate[13]={");
	for(int i=0;i<13;i++) Serial.print(cvOutCalibrate[i]),Serial.print(", ");
	Serial.println("};");
	Serial.print("tuneIn[13]={");
		for(int i=0;i<13;i++) Serial.print(tuneIn[i]),Serial.print(", ");
		Serial.println("};");
	for(int i=0;i<11;i++){
		//Serial.println(tuneIn[i]);
	}
	*/
	numberOfSteps=8;
}
bool mode=true;

uint32_t waitTime;
uint32_t time;
int j=0;
bool clr;

uint8_t cvInAnimationStep=0;
uint32_t cvInTime;
#define CV_IN_DURATION 180

uint8_t randomNumber;

void renderLeds(){
	if(!cvInMode) {
		hw.setShiftLed(shift);
		if(hw.buttonState(8)) hw.setShiftLed(true);
		hw.dimShiftLed(!hw.buttonState(8));
	}
		for(int i=0;i<8;i++){
			if(hw.buttonState(8)){
				if(cvInMode){
					hw.allLedsOff();

					if(hw.getElapsedBastlCycles()-cvInTime>CV_IN_DURATION){
						cvInTime=hw.getElapsedBastlCycles();
						cvInAnimationStep++;
						randomNumber=random(0,8);

					}
					//hw.setLed(cvInDestination,true);
					switch(cvInDestination){

					case TRANSPOSE:
						//transpose
						for(int i=0;i<4;i++){
							if(cvInAnimationStep%4>=i) hw.setLed(3-i,true);
						}
						break;

					case QUANTIZED_TRANSPOSE:
						// quantized transpose
						hw.setShiftLed(true);
						hw.setLed(4,!(cvInAnimationStep%2));
						for(int i=0;i<4;i++){
							if(cvInAnimationStep%4>=i) hw.setLed(3-i,true);
						}
						break;

					case OFFSET:
						// offset
						hw.setLed(cvInAnimationStep%8,true);
						break;

					case RESET_OFFSET:
						// resetOffset
						hw.setLed(cvInAnimationStep%8,true);
						if(cvInAnimationStep%2)  showBiLed(0,0), showBiLed(1,0);
						break;
					case TRIG_A_SHIFT:
						// trigA
						showBiLed(0,cvInAnimationStep%8);
						break;
					case TRIG_B_SHIFT:
						//trigB
						showBiLed(1,cvInAnimationStep%8);
						break;
					case SLIDE_TIME:
						//slideTime
						hw.setShiftLed(true);
						showBiLed(0,(cvInAnimationStep%3)+4);
						break;
					case GATE_TIME:
						//gateTime
						hw.setShiftLed(true);
						showBiLed(1,(cvInAnimationStep%3)+4);
						break;
					case SHIFT_INVERT:
						// invertTrigNumber
						if(cvInAnimationStep%2){
							showBiLed(0,trigAshift);
							showBiLed(1,trigBshift);
						}
						else{
							showBiLed(0,map(trigAshift,0,7,7,0));
							showBiLed(1,map(trigBshift,0,7,7,0));
						}
						break;
					case MIN_MAJ:
						//minMaj
						hw.setShiftLed(true);
						hw.setLed(4,true);
						hw.setLed(6,cvInAnimationStep%2);
						break;
					case RESET_TO_4:
						// resetTo4
						hw.setLed(4,!(cvInAnimationStep%2));
						if(cvInAnimationStep%2)  showBiLed(0,0), showBiLed(1,0);
						break;

					case RANDOM_STEP:
						hw.setLed(randomNumber,true);

						break;

					}
				}
				else{
				switch(i){
					case 0:
						hw.setLed(i,range==5);
						hw.dimLed(i,false);
						break;
					case 1:
						hw.setLed(i,range==2);
						hw.dimLed(i,false);
						break;
					case 2:
						hw.setLed(i,range==1);
						hw.dimLed(i,false);
						break;
					case 3:
						hw.setLed(i,cvInput);
						hw.dimLed(i,false);
						break;
					case 4:
						hw.setLed(i, quantize || (cvInDestination==QUANTIZED_TRANSPOSE));
						hw.dimLed(i,false);
						break;
					case 5:
						if( quantize || (cvInDestination==QUANTIZED_TRANSPOSE) ){
							if(quantizeType==0) hw.setLed(i,false);
							if(quantizeType==1) hw.setLed(i,true), hw.dimLed(i,true);
							if(quantizeType==2) hw.setLed(i,true), hw.dimLed(i,false);
						}
						else hw.setLed(i,false);

						break;
					case 6:
						if(( quantize || (cvInDestination==QUANTIZED_TRANSPOSE)) && (quantizeType!=0)){
							hw.setLed(i,major);
							hw.dimLed(i,false);
						}
						else hw.setLed(i,false);
						break;
					case 7:
						if((  quantize || (cvInDestination==QUANTIZED_TRANSPOSE))&& (quantizeType!=0)){
							if(scale==0) hw.setLed(i,false);
							if(scale==1) hw.setLed(i,true), hw.dimLed(i,true);
							if(scale==2) hw.setLed(i,true), hw.dimLed(i,false);
						}
						else hw.setLed(i,false);
						break;
					}
				}
			}
			else{
				if(dual){
					if(!shift) hw.setLed(i,gate[i]),hw.dimLed(i,true);
					else hw.setLed(i,slide[i]),hw.dimLed(i,true);
					if(dualMode==0){
						uint8_t _equal;
						if(master) _equal=i;
						else _equal=i+8;
						if(_equal==step){
							hw.setLed(i,true);
							hw.dimLed(i,false);
						}
					}
					if(dualMode==1){
						if(step==i){
							hw.setLed(i,true);
							hw.dimLed(i,false);
						}
						if(step2==i+8){
							hw.setLed(i,true);
							hw.dimLed(i,false);
						}
					}
					if(dualMode==2){
						if(i==step){
							hw.setLed(i,true);
							hw.dimLed(i,false);
						}
					}
				}
				else{
					if(i==step){
						hw.setLed(i,true);
						hw.dimLed(i,false);
					}
					else if(!shift) hw.setLed(i,gate[i]),hw.dimLed(i,true);
					else hw.setLed(i,slide[i]),hw.dimLed(i,true);
				}

			}
		}
		if(!cvInMode){
			/*
			for(int i=0;i<3;i++){
				hw.dimBiLed(i,false);//!hw.trigAState());
				hw.dimBiLed(i+3,false);//!hw.trigBState());
			}
			if(hw.trigAState()) showBiLed(0,8);
			else showBiLed(0,trigAshift);
			if(hw.trigBState()) showBiLed(1,8);
			else showBiLed(1,trigBshift);
			*/

			for(int i=0;i<3;i++){
				hw.dimBiLed(i,!hw.trigAState());
				hw.dimBiLed(i+3,!hw.trigBState());
			}
			showBiLed(0,trigAshift);
			showBiLed(1,trigBshift);
		}
}


uint16_t mapRange(uint16_t _in){
	return _in;
}

bool gateTimeActive;
uint32_t lastTime;
uint32_t slideTimeEnd;
uint32_t slideTime;
int updatedIn;
uint16_t lineOut;
#define NEG_POINTS 11
uint16_t lastQuantizedCV[5];
uint16_t renderPitch(){
	int _in;
	uint8_t _step=byte(step)%numberOfSteps;
	if(dual){
		if(dualMode==2){
			if(gatedPitch){
				if(gate[_step]) updatedIn=hw.getKnobValue(_step);
				_in=updatedIn;
			}
			else{
				_in=hw.getKnobValue(_step);
			}
		}
		else{
			if(master){
				if(_step<8) _in=hw.getKnobValue(_step);
				else _in=otherKnobs[_step%8];
			}
			else{
				if(_step>=8) _in=hw.getKnobValue(_step%8);
				else _in=otherKnobs[_step];
			}
		}
	}
	else{
		if(gatedPitch){
			if(gate[_step]) updatedIn=hw.getKnobValue(_step);
			_in=updatedIn;
		}
		else{
			_in=hw.getKnobValue(_step);
		}
	}
	//if cv in destination
		if(quantize || (cvInDestination==QUANTIZED_TRANSPOSE)){
			if(range==5) _in=map(_in,0,1024,0,61);
			if(range==2) _in=map(_in,0,1024,0,25);
			if(range==1) _in=map(_in,0,1024,0,13);

			if(cvInDestination==QUANTIZED_TRANSPOSE){

				uint16_t _cv=hw.getCV(0);
				lastQuantizedCV[0]=_cv;
				_cv=0;
				for(uint8_t i=0;i<5;i++){
					_cv+=lastQuantizedCV[i];
				}
				_cv=_cv/5;

				for(int i=0;i<4;i++){
					 lastQuantizedCV[4-i]=lastQuantizedCV[3-i];
				}
				//lastQuantizedCV[0]=_cv;
				//for(uint8_t i=0;i<5;i++) Serial.print(lastQuantizedCV[i]), Serial.print(", ");
				//Serial.println(_cv);
				if(_cv<tuneIn[0] ){
					if(_cv<=1) _in=0;
					else{
						_cv=constrain(_cv,1,tuneInNeg[0]);
						for(int i=0;i<10;i++){
							if(_cv>=tuneInNeg[NEG_POINTS-i] && _cv <tuneInNeg[NEG_POINTS-(i+1)] ){
								uint16_t sixth= (tuneInNeg[NEG_POINTS-(i+1)]-tuneInNeg[NEG_POINTS-i])/6;
								uint16_t _min=tuneInNeg[NEG_POINTS-i]-sixth/2;
								//uint16_t _max=tuneIn[i+1]-sixth/2;

								//_cv+=sixth/2;
								for(int j=0;j<6;j++){
									if(_cv>=(_min+(j*sixth)) && _cv<(_min+((j+1)*sixth))){
										_cv=(i*6)+j;
										j=100, i=100;
									}
								}
								if(i!=100) _cv=(i*6)+6;
							}
						}
						//_cv=map(_cv,tuneIn[0],tuneIn[10],0,61);
						_cv=constrain(_cv,0,60);
						_in-=60-_cv;
					}
					//Serial.print("neg");
					//Serial.println(_in);
				}
				else{
					_cv=constrain(_cv,tuneIn[0],tuneIn[10]);
					for(int i=0;i<10;i++){
						if(_cv>=tuneIn[i] && _cv < tuneIn[i+1]){
							uint16_t sixth= (tuneIn[i+1]-tuneIn[i])/6;
							uint16_t _min=tuneIn[i]-sixth/2;
							//uint16_t _max=tuneIn[i+1]-sixth/2;

							//_cv+=sixth/2;
							for(int j=0;j<6;j++){
								if(_cv>=(_min+(j*sixth)) && _cv<(_min+((j+1)*sixth))){
									_cv=(i*6)+j;
									j=100, i=100;
								}
							}
							if(i!=100) _cv=(i*6)+6;
						}
					}
					//_cv=map(_cv,tuneIn[0],tuneIn[10],0,61);
					_in+=_cv;
					//Serial.print("pos");
					//Serial.println(_in);
				}
				_in=constrain(_in,0,60);
			}




			uint8_t _scale=0;
			bool majorNow=major;
			if(cvInDestination==MIN_MAJ && currentCvGate) {
				 majorNow=!major;
			}
			if(hw.expanderState()){
				if(!hw.getMinMajState()){ //expander
					majorNow=!major;
				}
			}

			if(quantizeType==0) _scale=0;
			else{
				if(majorNow){
					if(quantizeType==1) _scale=1+scale; //scale
					else _scale=4+scale; //chord
				}
				else{
					if(quantizeType==1) _scale=7+scale; //scale
					else _scale=10+scale; //chord
				}
			}
			_in=quantizeNote(_scale,_in);
			_in=cvOutCalibrate[_in/6]+((_in%6)*((cvOutCalibrate[(_in/6)+1]- cvOutCalibrate[_in/6])/6));
			if(cvInDestination==TRANSPOSE){
				uint16_t _cv=hw.getCV(0);
				if(_cv<tuneIn[0] ){
					_cv=tuneIn[0]+(tuneIn[0]-_cv);
					_in-=dualTableCurveMap(_cv,11,tuneIn,cvOutCalibrate)-cvOutCalibrate[0];
					_in=constrain(_in,0,4095);
				}
				else{
					_cv=constrain(_cv,tuneIn[0],tuneIn[10]);
					_in+=dualTableCurveMap(_cv,11,tuneIn,cvOutCalibrate)-cvOutCalibrate[0];
					_in=constrain(_in,0,4095);
				}
				//_in+=map(currentCvValue,0,255,0,cvOutCalibrate[11]); //LAME - do propper mapping
				//_in=constrain(_in,0,cvOutCalibrate[11]);
			}
		}
		else{

			if(range==5) _in=map(_in,0,1024,cvOutCalibrate[0],cvOutCalibrate[10]);
			if(range==2) _in=map(_in,0,1024,cvOutCalibrate[0],cvOutCalibrate[4]);
			if(range==1) _in=map(_in,0,1024,cvOutCalibrate[0],cvOutCalibrate[2]);
			if(cvInDestination==TRANSPOSE){
				uint16_t _cv=hw.getCV(0);
				if(_cv<tuneIn[0] ){
					_cv=tuneIn[0]+(tuneIn[0]-_cv);
					_in-=dualTableCurveMap(_cv,11,tuneIn,cvOutCalibrate)-cvOutCalibrate[0];
					_in=constrain(_in,0,4095);
				}
				else{
					_cv=constrain(_cv,tuneIn[0],tuneIn[10]);
					_in+=dualTableCurveMap(_cv,11,tuneIn,cvOutCalibrate)-cvOutCalibrate[0];
					_in=constrain(_in,0,4095);
				}
				//_in+=currentCvValue<<2; //LAME - do propper mapping
				//_in=constrain(_in,0,1023);
			}
		}
		return _in;
}
bool gateState;
void renderGate(){
	bool _gate;
	if(dual){
		if(dualMode==2){
			_gate=gate[step];
		}
		else{
			if(master){
				if(step>=8) _gate=otherGates[step%8];
				else _gate=gate[step];
			}
			else{
				if(step>=8) _gate=gate[step%8];
				else _gate=otherGates[step];

			}
		}
	}
	else _gate=gate[step];


	if(gateTimeActive){
		if(hw.getElapsedBastlCycles()-gateTimeStart>gateTime){
			gateState=false;
			hw.setGateOut(false);
		}
		else{
			gateState=true;
			if(_gate) hw.setGateOut(true);
			else hw.setGateOut(false);
		}
	}
	else{
		gateState=true;
		if(gate[step]) hw.setGateOut(true);
		else hw.setGateOut(false);
	}


	for(int i=0;i<8;i++){
		if(i==step) hw.setGate(i,gateState);
		else hw.setGate(i,false);
	}
}
uint8_t lastPV[2];

void renderCTRLknobs(){
	if(hw.knobFreezed(0)){
			if (hw.knobMoved(0) || inBetween(hw.getPotA() >> 2,hw.getLastPotA() >> 2,potValue[0][shift/*hw.buttonState(8)*/])) hw.unfreezeKnob(0);//, interactionS=true;//, interaction=true;
		}
		else{
			potValue[0][shift/*hw.buttonState(8)*/]=hw.getPotA()>>2;
		}
		if(hw.knobFreezed(1)){
			if (hw.knobMoved(1) || inBetween(hw.getPotB() >> 2,hw.getLastPotB() >> 2,potValue[1][shift/*hw.buttonState(8)*/])) hw.unfreezeKnob(1);//, interactionS=true;//,interaction=true;
		}
		else{
			potValue[1][shift/*hw.buttonState(8)*/]=hw.getPotB()>>2;
		}

		uint16_t cv[4];
		if(cvInExpMode){
			cv[1]=0;//hw.getCV(1);
			cv[2]=0;//hw.getCV(2);
			cv[3]=0;
		}
		else{
			cv[1]=hw.getCV(1);
			cv[2]=hw.getCV(2);
			cv[3]=hw.getCV(3);
		}


		if(cvInDestination==TRIG_A_SHIFT) trigAshift=constrain(map((int)potValue[0][0]+(int)currentCvValue+(int)(cv[1]>>2),0,240,0,8),0,7);
		else if(cvInDestination==SHIFT_INVERT && currentCvGate) trigAshift=map(constrain(map(potValue[0][0]+(cv[1]>>2),0,240,0,8),0,7),0,7,7,0);//,trigAshift=map(trigAshift,0,7,7,0);
		else trigAshift=constrain(map(potValue[0][0]+(cv[1]>>2),0,240,0,8),0,7);

		if(cvInDestination==TRIG_B_SHIFT) trigBshift=constrain(map((int)potValue[1][0]+currentCvValue,0,240,0,8),0,7);
		else trigBshift=constrain(map(potValue[1][0],0,240,0,8),0,7);

		if(trigAshift>7) trigAshift=7;
		if(trigBshift>7) trigBshift=7;


		if(cvInDestination==SLIDE_TIME) slideTime=curveMap(constrain((int)potValue[0][1]+(int)(cv[3]>>2)+currentCvValue,0,255),SLIDE_MAP_POINTS,slideMap);//map(potValue[0][1]+currentCvValue+(hw.getCV(3)>>2),0,256,15,2000);
		else slideTime= curveMap(constrain(potValue[0][1]+(cv[3]>>2),0,255),SLIDE_MAP_POINTS,slideMap); // map(potValue[0][1]+(hw.getCV(3)>>2),0,256,15,2000); // curveMap

		if(potValue[1][1]+(cv[2]>>2)>250) gateTimeActive=false;
		else gateTimeActive=true;
		if(cvInDestination==GATE_TIME) gateTime=curveMap(constrain((int)potValue[1][1]+(int)(cv[2]>>2)+currentCvValue,0,255),GATE_MAP_POINTS,gateMap);//map(potValue[1][1]+currentCvValue+(hw.getCV(2)>>2),0,256,15,1500);
		else gateTime=curveMap(constrain(potValue[1][1]+(cv[2]>>2),0,255),GATE_MAP_POINTS,gateMap);  //map(potValue[1][1]+(hw.getCV(2)>>2),0,256,15,1500); // curveMap

		if(lastPV[0]!=potValue[0][1]) interactionS=true;
		lastPV[0]=potValue[0][1];
		if(lastPV[1]!=potValue[1][1]) interactionS=true;
		lastPV[1]=potValue[1][1];
		//potValue[1][1]
}

void renderCvIn(){
	//implement cvInExpMode!!!

	lastCvValue=currentCvValue;
	int _cv=map(hw.getCV(0),tuneIn[0],tuneIn[10],0,256);

	//_cv=constrain(_cv,0,255);
	currentCvValue=_cv;

	switch(cvInDestination){
		case OFFSET:
			currentCvValue=constrain(currentCvValue,-250,255);
			lastCvValue=constrain(lastCvValue,-250,255);
			if(map(currentCvValue,0,256,0,numberOfSteps)!=map(lastCvValue,0,256,0,numberOfSteps)) expanderOffset=hw.getCV(4),clockCall(4);
			break;
		case RESET_TO_4:
			if(lastCvValue<120 && currentCvValue>=120) clockCall(3); //has to happen prior trigger //2.5V reference
			break;

		case RANDOM_STEP:
			if(lastCvValue<120 && currentCvValue>=120) clockCall(random(30,38)); //has to happen prior trigger //2.5V reference
			break;
	}

	if(currentCvValue>=120) currentCvGate=true;
	else currentCvGate=false;

	//offset
	if(map(hw.getCV(4),0,1024,0,8)!=map(hw.getLastCV(4),0,1024,0,8)) expanderOffset=hw.getCV(4),clockCall(5);
	if(!cvInExpMode){
		bitOffset=0;
	}
	else{
		bool lastBit[3],bit[3];
		for(int i=0;i<3;i++){
			lastBit[i]=hw.getLastCV(i+1)>128;
			bit[i]=hw.getCV(i+1)>128;
		}
		for(int i=0;i<3;i++){
			if(lastBit[i]!=bit[i]){
				if(i<3){
					bitOffset=0;
					bitWrite(bitOffset,0,bit[0]);
					bitWrite(bitOffset,1,bit[1]);
					bitWrite(bitOffset,2,bit[2]);
					bitOffset=map(bitOffset,0,7,0,1023);
					expanderOffset=hw.getCV(4);
					clockCall(5);
				}
			}
		}

	}
}
void renderSlide(){
	bool _slide;
	if(dual){
		if(dualMode==2){
			_slide=slide[step];
		}
		else{
			if(master){
				if(step>=8) _slide=otherSlides[step%8];
				else _slide=slide[step];
			}
			else{
				if(step>=8) _slide=slide[step%8];
				else _slide=otherSlides[step];
			}
		}
	}
	else _slide=slide[step];
	if(_slide){
		if(calculateLine){
			calculateLine=false;
			slideTimeEnd=slideTimeStart+slideTime;
			line.set(realOut , renderPitch(), slideTime); //slideFrom
			//lineOut=slideFrom
		}
		if(hw.getElapsedBastlCycles()<slideTimeEnd){
			uint16_t misses=hw.getElapsedBastlCycles()-lastTime;
			while(misses>0){
				lineOut=line.next();
				misses--;
			}
			out=lineOut;
		}
	}
}
void renderLatencies(){
	if(scheduleA){
		if(hw.getElapsedBastlCycles()-scheduleTimeA>triggerLatency) clockCall(11), scheduleA=false;
	}
	if(scheduleB){
		if(hw.getElapsedBastlCycles()-scheduleTimeB>triggerLatency) clockCall(12), scheduleB=false;
	}
	if(scheduleReset){
		if(hw.getElapsedBastlCycles()-scheduleResetTime>resetLatency) clockCall(10), scheduleReset=false;
	}
	if(scheduleReset4){
		if(hw.getElapsedBastlCycles()-scheduleReset4Time>resetLatency) clockCall(13), scheduleReset4=false;
	}
}
void loop()
{
	if(step>=numberOfSteps) step=step%numberOfSteps;
	while(step<0) step=numberOfSteps+step;

	bool anyButton=false;
	for(int i=0;i<9;i++){
		if(hw.buttonState(i)){
			anyButton=true;
		}
	}
	if(!anyButton) fnJump=false;
	jump=hw.jumpState();
	com.update();
	renderLatencies();

	if(!calibrating){
		renderCvIn();
		renderLeds();
		renderCTRLknobs();
		renderGate();
		out=renderPitch();
		renderSlide();
		lastTime=hw.getElapsedBastlCycles();
		realOut=out;
		writeDAC(out);
	}
	if(dual){
		renderCommunication();
	}
}


void test(){
	for(int i=0;i<8;i++){
		if(hw.getKnobValue(i)>300 && hw.getKnobValue(i)<600) hw.setLed(i,true), hw.dimLed(i,true);
		else if(hw.getKnobValue(i)>=600) hw.setLed(i,true), hw.dimLed(i,false);
		else hw.setLed(i,false);
	}
	for(int i=0;i<3;i++){
		hw.dimBiLed(i,hw.trigAState());
		hw.dimBiLed(i+3,hw.trigBState());
	}


	if(hw.getPotA()>250) hw.setBiLed(0,true,hw.resetState());
	else hw.setBiLed(0,false,hw.resetState());
	if(hw.getPotA()>500) hw.setBiLed(1,true,hw.resetState());
	else hw.setBiLed(1,false,hw.resetState());
	if(hw.getPotA()>750) hw.setBiLed(2,true,hw.resetState());
	else hw.setBiLed(2,false,hw.resetState());

	hw.setBiLed(3,true,!hw.resetState());
	hw.setBiLed(4,true,!hw.resetState());
	hw.setBiLed(5,true,!hw.resetState());


	hw.setShiftLed(shift);
}

