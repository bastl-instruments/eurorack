

#ifdef EXTERNAL_IDE
#include <Arduino.h>
#include <mcpDac.h>
#include <pgHW.h>
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
#define DAC_CONFIG 0x30
#define SS_DAC B,2
//MIDInoteBuffer buffer;
pgHW hw;



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
uint16_t tuneInputTable[60]={0,17,34,51,68,85,102,119,136,153,170,188,
						205,222,239,256,273,290,307,324,341,358,375,392,
						409,426,443,460,477,494,512, 529,546,563,580,597,
						614,631,648,665,682,699,716,733,750,767,784,801,
						818,835,853,870,887,904,921,938,955,972,989,1006};

uint16_t tuneTable[12]={ 0,410, 819,1229, 1638,2048, 2458,2867, 3277,3686, 4095,4095};
#define TUNE_POINTS 11
void loadTable(){
	for(int i=0;i<11;i++){
		tuneTable[i]=word(EEPROM.read(i*2),EEPROM.read(1+(i*2)));
	}
}
void saveTable(){
	for(int i=0;i<11;i++){
		EEPROM.write(i*2,highByte(tuneTable[i]));
		EEPROM.write(1+(i*2),lowByte(tuneTable[i]));
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
#define CHROMATIC 0
#define MAJOR 1
#define HARMONIC_MINOR 2
#define MELODIC_MINOR 3
#define PENTATONIC_MAJOR 4
#define PENTATONIC_MINOR 5
#define BLUES 6
#define WHOLE_TONE 7

bool scale[8][12]{
		{1,1,1,1,1,1,1,1,1,1,1,1},
		{1,0,1,0,1,1,0,1,0,1,0,1},
		{1,0,1,1,0,1,0,1,0,1,1,0},
		{1,0,1,0,1,1,0,1,0,1,0,1},
		{1,0,1,0,1,1,0,1,0,1,0,1},
		{1,0,1,0,1,1,0,1,0,1,0,1},
		{1,0,1,0,1,1,0,1,0,1,0,1},
		{1,0,1,0,1,1,0,1,0,1,0,1},
};
bool quantizeTable[12];

uint8_t quantizeNote(uint8_t note){
	if(quantizeTable[note%12]){
		return note;
	}
	else{
		uint8_t higher=0;
		while(!quantizeTable[(note+higher)%12]){
			higher++;
		}
		uint8_t lower=0;
		while(!quantizeTable[(note-lower)%12]){
			lower++;
		}
		if(higher<lower) return note+higher;
		else return note-lower;
	}

	return note;
}


bool wait=false;
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

void clockCall(uint8_t number){

}
void setup(){

	hw.initialize();

	com.init(38400);
	//com.attachChannelCVCallback(&channelCVCall);
	com.attachClockCallback(&clockCall);
	com.attachChannelModeCallback(&channelModeCall);

    loadTable();
    //com.sendPairMessage();


	mcpDacInit();
	//dacInit();

}
bool mode=true;

uint32_t waitTime;
void loop()
{
/*
	hw.updateKnobs();
	hw.updateButtons();
	hw.updateDisplay();



	if(mode){

		hw.setColor(RED);
		uint8_t voltage=map(hw.knobValue(0),0,1024,0,11);
		hw.displayNumber(voltage/2);
		if((voltage%2)==1) hw.setDot(true);
		else hw.setDot(false);
		if(hw.justPressed(BUTTON_A)){
			if(hw.buttonState(FN)) tuneTable[voltage]-=30;
			else tuneTable[voltage]--;
			saveTable();
		}
		if(hw.justPressed(BUTTON_B)){
			if(hw.buttonState(FN)) tuneTable[voltage]+=30;
			tuneTable[voltage]++;
			saveTable();
		}


		mcpDacSend(tuneTable[voltage]);

	}
	else{

	//	hw.displayChar('c');
		if(hw.buttonState(FN)){
			hw.setColor(BLUE);
		//	pinMode(1,OUTPUT);
			//com.init(38400);
			mcpDacSend(0);
			com.sendPairMessage();
			com.sendChannelMode(1, 0);
			//com.sendPairMessage();
			for (int j = 0; j < 12; j++) {
				mcpDacSend(tuneTable[j]);
				com.sendPairMessage();
				//com.sendChannelCV(1, j);
				com.sendChannelMode(1, j);
				waitTime=millis();
				wait=true;
				while(wait){
					wait=false;
					if(millis()-waitTime>3000) wait=false, j=13;
					com.update();
					hw.displayNumber(j/2);
					if((j%2)==1) hw.setDot(true);
					else hw.setDot(false);
					hw.updateKnobs();
					//hw.updateButtons();
					hw.updateDisplay();
				}
				delay(500);
			}

			//delay(1000);
		}
		else hw.setColor(GREEN);
		com.update();

	}
	if(hw.justPressed(PAGE)){
		mode=!mode;
	}
*/
	com.sendChannelTrigger(1,1);
	delay(80);

/*

	for (int j = 0; j < 4096; j++) {
		mcpDacSend(j);

					//com.sendChannelCV(1, j);
					//com.sendChannelMode(1, j);
					delay(100);
	}
*/
	//hw.updateKnobs();
	//mcpDacSend(hw.knobValue(0)<<2);
}




