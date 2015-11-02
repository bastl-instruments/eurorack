

#ifdef EXTERNAL_IDE
#include <Arduino.h>
#include <utilities.h>
#include <dacRoutine.h>
/*
 *
 * TODO
 * - CV input implementation
 * -input CV calibration -
 * -output CV calibration procedure
 * -dual setup
 * -slide time and gate time curveMap
 * -scale tuning
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
#include<analogSeqHW.h>
#include <Line.h>
Line <float> line;
extern analogSeqHW hw;

//MIDInoteBuffer buffer;


//MIDInoteBuffer buffer;



#define PAGE 0
#define FN 2

#define UP 1
#define DOWN 3

#define BUTTON_A 4
#define BUTTON_B 5

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
uint16_t cvOutCalibrate[12]={ 0,410, 819,1229, 1638,2048, 2458,2867, 3277,3686, 4095,4095};
uint16_t tuneInputTable[60]={0,17,34,51,68,85,102,119,136,153,170,188,
						205,222,239,256,273,290,307,324,341,358,375,392,
						409,426,443,460,477,494,512, 529,546,563,580,597,
						614,631,648,665,682,699,716,733,750,767,784,801,
						818,835,853,870,887,904,921,938,955,972,989,1006};

uint16_t tuneTable[12]={ 0,410, 819,1229, 1638,2048, 2458,2867, 3277,3686, 4095,4095};
#define TUNE_POINTS 11
void loadTable(){
	for(int i=0;i<12;i++){
		cvOutCalibrate[i]=word(EEPROM.read(100+(i*2)),EEPROM.read(100+1+(i*2)));
	}
}
void saveTable(){
	for(int i=0;i<12;i++){
		EEPROM.write(100+(i*2),highByte(cvOutCalibrate[i]));
		EEPROM.write(100+1+(i*2),lowByte(cvOutCalibrate[i]));
	}
}
uint8_t signalToNote(uint16_t signal){
	uint8_t resultNote=0;
	for(int i=0;i<5;i++){
		if(signal>tuneInputTable[i*12]){
			for(int j=0;j<12;j++){
				if(signal>tuneInputTable[i*12+j]){
					uint16_t halfDistance=(tuneInputTable[i*12+j+1]-tuneInputTable[i*12+j]) / 2;
					if(signal>tuneInputTable[i*12+j]+halfDistance){
						resultNote=i*12+j+1;
						return resultNote;
					}
					else{
						resultNote=i*12+j;
						return resultNote;
					}
				}
			}
		}
	}
	return resultNote;
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
		{1,0,0,1,1,0,0,1,0,1,1,0}, //maj blues
		{1,0,0,0,1,0,0,1,0,0,0,0}, //maj chord
		{1,0,0,0,0,0,0,1,0,0,0,0}, //maj fifth
		{1,0,0,0,1,0,0,1,0,0,1,0}, //maj +7

		{1,0,1,1,0,1,0,1,0,1,1,0}, //min dia
		{1,0,0,1,0,1,0,1,0,0,1,0}, //min penta
		{1,0,0,1,0,1,1,1,0,0,1,0}, //min blues
		{1,0,0,1,0,0,0,1,0,0,0,0}, //min chord
		{1,0,0,0,0,0,0,1,0,0,0,0}, //min fifth
		{1,0,0,1,0,0,0,1,0,0,1,0}, //main +7
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
//void channelCVCall(uint8_t channel, uint8_t value){
void channelModeCall(uint8_t channel, uint8_t value){
	//
	//if(channel==1)
	wait=false;
//	hw.displayNumber(value);
//	hw.updateDisplay();
//	com.sendPairMessage();
//	com.sendChannelMode(channel, value);
}
bool gate[8];
bool slide[8];
int step=0;
uint8_t trigAshift;
uint8_t trigBshift;
uint32_t gateTime;
uint32_t gateTimeStart;
uint32_t slideTimeStart;
uint16_t slideFrom;
bool calculateLine=false;
uint16_t out;
uint16_t realOut;
void clockCall(uint8_t number){
	//Serial.println(number);
	switch(number){
	case 0:
		step=0;
		slideFrom=realOut;
		//RST
		gateTimeStart=hw.getElapsedBastlCycles();
		if(slide[step]) calculateLine=true,slideTimeStart=hw.getElapsedBastlCycles();
		break;
	case 1:
		uint8_t shiftA;
		slideFrom=realOut;
		if(trigAshift>3) shiftA=trigAshift-3, step+=shiftA;
		else shiftA=4-trigAshift, step-=shiftA;
		if(step>7) step%=8;
		if(step<0) step=8+step;
		gateTimeStart=hw.getElapsedBastlCycles();
		if(slide[step]) calculateLine=true,slideTimeStart=hw.getElapsedBastlCycles();
		//trigA
		break;
	case 2:
		//trigB
		uint8_t shiftB;
		slideFrom=realOut;
		if(trigBshift>3) shiftB=trigBshift-3, step+=shiftB;
		else shiftB=4-trigBshift, step-=shiftB;
		if(step>7) step%=8;
		if(step<0) step=8+step;
		gateTimeStart=hw.getElapsedBastlCycles();
		if(slide[step]) calculateLine=true,slideTimeStart=hw.getElapsedBastlCycles();

		break;

	}
}
bool shift;
uint8_t range=5;


bool cvInput=false;
bool quantize=false;
uint8_t quantizeType=0;
uint8_t scale=0;
bool major=true;
void saveSettings(){
	EEPROM.write(0,quantize);
	EEPROM.write(1,quantizeType);
	EEPROM.write(2,cvInput);
	EEPROM.write(3,scale);
	EEPROM.write(4,range);
	EEPROM.write(5,major);
}
void loadSettings(){
	quantize=EEPROM.read(0);
	quantizeType=EEPROM.read(1);
	cvInput=EEPROM.read(2);
	scale=EEPROM.read(3);
	range=EEPROM.read(4);
	major=EEPROM.read(5);
	loadTable();
}
bool interaction;
uint8_t selekt;
void buttonCall(uint8_t number){
/*
	if(number==0 && hw.buttonState(0)) cvOutCalibrate[selekt]--, saveTable();
	if(number==1 && hw.buttonState(1)) cvOutCalibrate[selekt]++, saveTable();
	if(number==2 && hw.buttonState(2)) cvOutCalibrate[selekt]-=30, saveTable();
	if(number==3 && hw.buttonState(3)) cvOutCalibrate[selekt]+=30, saveTable();
	if(number==4 && hw.buttonState(4)) selekt++, Serial.println(selekt),Serial.println(cvOutCalibrate[selekt]);
	if(number==5 && hw.buttonState(5)) selekt--, Serial.println(selekt),Serial.println(cvOutCalibrate[selekt]);
*/
	if(number==8){
		hw.freezeAllKnobs();
		if(!hw.buttonState(number)){
			if(interaction) interaction=false;
			else shift=!shift;
		}
		else{
			interaction=false;
		}

	}
	else{
		if(hw.buttonState(8)){
			if(hw.buttonState(number)){
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
					cvInput=!cvInput;
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
				saveSettings();
				interaction=true;
			}
		}
		else if(hw.jumpState()){
			if(hw.buttonState(number)){
				step=number;
				gateTimeStart=hw.getElapsedBastlCycles();
				if(slide[step]) calculateLine=true,slideTimeStart=hw.getElapsedBastlCycles();
			}
		}
		else{
			if(hw.buttonState(number)){
				if(shift==false){
					gate[number]=!gate[number];
				}
				else{
					slide[number]=!slide[number];
				}
			}
		}
	}
}
#define _POINT 0
#define _MINUS 1
#define _PLUS 2
uint8_t whichPoint=0;
void interpolateCall(uint8_t channel, uint8_t number){
	switch(channel){
		case _POINT:
			saveTable();
			whichPoint=number;
			writeDAC(cvOutCalibrate[number]);
			break;
		case _MINUS:
			tuneTable[whichPoint]-=number;
			writeDAC(cvOutCalibrate[whichPoint]);
			break;
		case _PLUS:
			tuneTable[whichPoint]+=number;
			writeDAC(cvOutCalibrate[whichPoint]);
			break;
	}
}
void setup(){
/*
	hw.initialize();

	com.init(38400);
	//com.attachChannelCVCallback(&channelCVCall);
	com.attachClockCallback(&clockCall);
	com.attachChannelModeCallback(&channelModeCall);


	*/

	hw.init(&buttonCall,&clockCall);
	dacInit();
	//Serial.begin(38400);

	com.init(38400);
		//com.attachChannelCVCallback(&channelCVCall);
	com.attachChannelInterpolateCallback(&interpolateCall);
	//saveTable();
	loadSettings();

}
bool mode=true;

uint32_t waitTime;
uint32_t time;
int j=0;
bool clr;
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
	}

}
uint16_t mapRange(uint16_t _in){
	return _in;
}

bool gateTimeActive;
uint32_t lastTime;
uint32_t slideTimeEnd;
uint32_t slideTime;
uint16_t potValue[2][2];
uint16_t lineOut;
uint16_t renderPitch(){
	uint16_t _in=hw.getKnobValue(step);
		if(quantize){
			if(range==5) _in=map(_in,0,1024,0,61);
			if(range==2) _in=map(_in,0,1024,0,25);
			if(range==1) _in=map(_in,0,1024,0,13);

			uint8_t _scale=0;
			if(quantizeType==0) _scale=0;
			else{
				if(major){
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
		}
		else{
			if(range==5) _in=map(_in,0,1024,0,cvOutCalibrate[10]);
			if(range==2) _in=map(_in,0,1024,0,cvOutCalibrate[4]);
			if(range==1) _in=map(_in,0,1024,0,cvOutCalibrate[2]);
		}
		return _in;
}
bool gateState;
void renderGate(){
	if(!gate[step]) hw.setGateOut(false);
		else {
			if(gateTimeActive){
				if(hw.getElapsedBastlCycles()-gateTimeStart>gateTime){
					gateState=false;
					hw.setGateOut(false);
				}
				else{
					gateState=true;
					hw.setGateOut(true);
				}
			}
			else{
				gateState=gate[step];
				hw.setGateOut(gate[step]);
			}
		}

	for(int i=0;i<8;i++){
		if(i==step && gate[step]) hw.setGate(i,gateState);
		else hw.setGate(i,false);
	}
}
uint8_t lastPV[2];
void renderCTRLknobs(){
	if(hw.knobFreezed(0)){
			if (hw.knobMoved(0) || inBetween(hw.getPotA() >> 2,hw.getLastPotA() >> 2,potValue[0][hw.buttonState(8)])) hw.unfreezeKnob(0);//, interaction=true;
		}
		else{
			potValue[0][hw.buttonState(8)]=hw.getPotA()>>2;
		}
		if(hw.knobFreezed(1)){
			if (hw.knobMoved(1) || inBetween(hw.getPotB() >> 2,hw.getLastPotB() >> 2,potValue[1][hw.buttonState(8)])) hw.unfreezeKnob(1);//,interaction=true;
		}
		else{
			potValue[1][hw.buttonState(8)]=hw.getPotB()>>2;
		}
		trigAshift=map(potValue[0][0],0,256,0,8);
		trigBshift=map(potValue[1][0],0,256,0,8);

		slideTime=map(potValue[0][1],0,256,15,2000);
		if(potValue[1][1]>250) gateTimeActive=false;
		else gateTimeActive=true;
		gateTime=map(potValue[1][1],0,256,15,1500);

		if(lastPV[0]!=potValue[0][1]) interaction=true;
		lastPV[0]=potValue[0][1];
		if(lastPV[1]!=potValue[1][1]) interaction=true;
		lastPV[1]=potValue[1][1];
		//potValue[1][1]
}

void renderLeds(){
	hw.setShiftLed(shift);
		if(hw.buttonState(8)) hw.setShiftLed(true);
		hw.dimShiftLed(!hw.buttonState(8));

		for(int i=0;i<8;i++){
			if(hw.buttonState(8)){
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
						hw.setLed(i,quantize);
						hw.dimLed(i,false);
						break;
					case 5:
						if(quantize){
							if(quantizeType==0) hw.setLed(i,false);
							if(quantizeType==1) hw.setLed(i,true), hw.dimLed(i,true);
							if(quantizeType==2) hw.setLed(i,true), hw.dimLed(i,false);
						}
						else hw.setLed(i,false);

						break;
					case 6:
						if(quantize && (quantizeType!=0)){
							hw.setLed(i,major);
							hw.dimLed(i,false);
						}
						else hw.setLed(i,false);
						break;
					case 7:
						if(quantize && (quantizeType!=0)){
							if(scale==0) hw.setLed(i,false);
							if(scale==1) hw.setLed(i,true), hw.dimLed(i,true);
							if(scale==2) hw.setLed(i,true), hw.dimLed(i,false);
						}
						else hw.setLed(i,false);
						break;
					}
			}
			else{
				if(!shift){
					if( i==step){
						hw.setLed(i,true);
						if(gate[i]) hw.dimLed(i,false);
						else hw.dimLed(i,false);
					}
					else hw.setLed(i,gate[i]),hw.dimLed(i,true);
				}
				else{
					if( i==step){
						hw.setLed(i,true);
						if(slide[i]) hw.dimLed(i,false);
						else hw.dimLed(i,false);
					}
					else hw.setLed(i,slide[i]),hw.dimLed(i,true);
				}
			}
		}

		for(int i=0;i<3;i++){
			hw.dimBiLed(i,!hw.trigAState());
			hw.dimBiLed(i+3,!hw.trigBState());
		}
		showBiLed(0,trigAshift);
		showBiLed(1,trigBshift);

}
void loop()
{

	com.update();

	renderLeds();
	renderCTRLknobs();
	renderGate();
	out=renderPitch();

	if(slide[step]){
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
			//if()
		//	lastTime=hw.getElapsedBastlCycles();
			out=lineOut;
		}
	}
	lastTime=hw.getElapsedBastlCycles();
	//out=line(out);
	realOut=out;

	//selekt=map(hw.getKnobValue(0),0,1024,0,12);

	//out=cvOutCalibrate[selekt];
	writeDAC(out);

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
	/*
	if(hw.trigAState()){

	}
	*/

	//hw.getPotA()

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

