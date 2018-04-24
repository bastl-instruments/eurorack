

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
	//saveTable();
    loadTable();
    //com.sendPairMessage();


	mcpDacInit();
	//dacInit();

}
uint8_t mode=1;

uint32_t waitTime;
uint32_t measureTime;
uint32_t measureInterval=100;
uint32_t measureResult[3];
uint32_t measuredTime[3][10];

uint32_t measureUP[3];
#include <fastAnalogRead.h>
const uint8_t tune[]={0,2,4,5, 7,9,11,7, 12,12,12,12, 11,11,11,11, 9,9,9,9, 7,7,7,7, 5,9,5,2, 4,7,4,0, 2,2,2,2, 7,7,7,7, 5,9,5,2, 4,7,4,0, 2,2,2,2, 0,0,0,0};
void loop()
{

	if(mode!=2){
		hw.updateKnobs();
		hw.updateButtons();
		hw.updateDisplay();
	}


	switch(mode){
	case 0:{
		hw.setColor(RED);
		uint8_t voltage=map(hw.knobValue(0),0,1024,0,11);
		hw.displayNumber(voltage/2);
		if((voltage%2)==1) hw.setDot(true);
		else hw.setDot(false);
		if(hw.justPressed(BUTTON_A)){
			if(hw.buttonState(FN)) tuneTable[voltage]-=30;
			else tuneTable[voltage]--;
			//saveTable();
		}
		if(hw.justPressed(BUTTON_B)){
			if(hw.buttonState(FN)) tuneTable[voltage]+=30;
			tuneTable[voltage]++;
			//saveTable();
		}


		mcpDacSend(tuneTable[voltage]);
	}
	break;
	case 1:
	{
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
		break;
		case 2:{
			if(hw.buttonState(UP)){
				for(int i=0;i<56;i++){
					mcpDacSend(map(tune[i],0,12,tuneTable[2],tuneTable[4]));
					delay(250);
				}
				hw.updateButtons();
			}
			else{
			hw.setColor(BLUE);
			hw.updateDisplay();
			bool inState;

			hw.updateButtons();
			bit_dir_inp(C,5);
			for(int i=0;i<3;i++){
				mcpDacSend(tuneTable[(i*2)+2]);
				delay(10);
				for(int j=0;j<8;j++){
					measureTime=millis();
					inState=bit_read_in(C,5);
					uint8_t period=0;
					while(((millis()-measureTime)<measureInterval) && period<4){
						bool newState=bit_read_in(C,5);
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

							//measureUP[i]=millis();

						}
						inState=newState;
					}
					for(int k=0;k<4;k++){
						measureResult[i]+=measuredTime[i][k];
					}
					measureResult[i]/=4;//(period-1);

				}
			}
			if(measureResult[0]==0 || measureResult[1]==0 || measureResult[2]==0){
				hw.displayChar('e');
			}
				else{

			long difference1=measureResult[0]-(2*measureResult[1]);
			long difference2=measureResult[1]-(2*measureResult[2]);

			long finalError=(difference1+difference2)/2;
			if(abs(finalError)<20) hw.displayNumber(5),hw.setDot(true);
			else hw.displayNumber(constrain(map(finalError,-500,500,0,9),0,9)), hw.setDot(false);
		//	Serial.print(measureResult[0]), Serial.print(", "), Serial.print(measureResult[1]),Serial.print(", "), Serial.println(measureResult[2]);


			}
			for(int i=0;i<3;i++) measureResult[i]=0;

		}
		}
		break;

	}




	if(hw.justPressed(PAGE)){
		if(mode<2) mode++;
		else mode=0;
	}

//	com.sendChannelTrigger(1,1);
//	delay(80);

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




