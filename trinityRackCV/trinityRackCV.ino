/*

 to do

 midi expansion support - asi ne
 reset expander pin mapping

 #to test

 retrigger ADSR
 lfo cv sync -deeper
 random rate mapping
 combo for reseting sequence
 sync indication
 tune random CV stuff - fill in table
	bit settings has to match the manual - test


 #tested
  default settings + clear memory
  handsake + selfrepair ?
odd automation
  lfo sync
  automation sync
  automation erase
   lfo range
 syncing with clock event
  CV input
  looping adsr doesnt trigger on its own - embed it to the library iteself
  buffer transfer
inits for all objects
 transfer automation data
saving changes sound
 selected automation recording
 to fine tune later
  potMapping


*/

#ifdef EXTERNAL_IDE

#include <Arduino.h>

//#include <EEPROM.h>

int main(void) {
  init();
  setup();
  while(true) {
    loop();
  }
}

#endif

#include <portManipulations.h>
#include <lfoExtended.h>
#include <simpleSerialDecoder.h>
#include "trinityRackCV_HW.h"
#include <expADSR.h>
#include <Line.h>
#include <interpolatingBuffer.h>

//#include <mapping.h>
//#include <quantizer8bit.h>
//quantizer8bit quantizer;



interpolatingBuffer buffer[6];
//Line <float> line[6];
ADSR envelope[6];
lfoExtended LFO[6];
simpleSerialDecoder com;

/* channel TYPES
 *lfo
 *envelope
 *cv follow
 *interpolate
 *trigger
 */

extern trinityRackCV_HW hardware;

/*
void stepperStep() {
	player->stepFourth();
	synchronizer.doStep();
}
*/
uint32_t gateTime;
bool gate[6];

uint32_t _time;
uint32_t lfoClockInDuration[6];
uint32_t lastLfoTime[6];
#define TRIGGER_MODE 1
#define GATE_MODE 2
#define CV_MODE 3
#define INTERPOLATE_MODE 4
#define LFO_MODE 5
#define ADSR_MODE 6

#define SETTINGS_BYTE 3

#define HOLD_BIT 0
#define LIN_LOG_BIT 1
#define LOOP_BIT 2
#define SYNC_BIT 3

#define CV_ASSING_0 4
#define CV_ASSING_1 5
#define CV_ASSING_2 6

#define LFO_SYNC_BIT 4
#define LFO_ODD_BIT 5
#define AUT_SYNC_BIT 6
#define AUT_ODD_BIT 7

uint8_t selectedChannel=0;
const uint8_t syncPhase[6]={0,64,127,0,64,127};
uint32_t clockInTime,lastClockInTime, channelTime, lastChannelTime;
uint16_t counter;
bool flop;
uint8_t currentPreset=0;
//uint8_t parameter[6][2]={{0,0},{0,0},{0,0},{0,0},{0,0},{0,20}};
bool channelSync[6];
uint16_t syncFactor[6];
bool syncFast[6];
bool gateOut[6];
//uint32_t triggerTime[6];
#define TRIGGER_LENGTH 20

#define NUMBER_OF_MODES 8

void buttonCall(uint8_t v) {

}

#define LFOMAP_POINTS 5
#define ADSRMAP_POINTS 5

uint16_t LFOMap[10]={0,63,127,191,255,   10,80,400,800,2000}; //novinka tune here
uint16_t LFOMap2[10]={0,63,127,191,255,   60,250,1000,15000,60000}; //afected by metaDivider
uint16_t ADSRMap[10]={0,63,127,191,255,  10,500,1000,3000,10000};
uint16_t bufferMap[10]={0,63,127,191,255,   10,100,500,1000,4000};

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


long myMap(long x,  long in_max,  long out_max)
{
  return (x) * (out_max) / (in_max);
}


uint8_t divider;
uint32_t offset[6];
// 2 ON OFF TIME STANDART
// 3 CHANNEL
// 3 TIME
uint8_t counterLeft;
uint8_t multiplication;

void renderMultiplier(){

	if (counterLeft != 0) {

			if ((hardware.getElapsedBastlCycles() - lastChannelTime) > (clockInTime / (multiplication))) {

				counter++;
				unsigned char subStepsFromLastStep = multiplication - counterLeft;

				//Next line is handling problems with rounding so instead of increasing the last triggered
				//To the current time we increase it by division of the step multiplied by number of current step
				//That should have little better precision
				lastChannelTime = channelTime + (( clockInTime * subStepsFromLastStep) / (multiplication));
				counterLeft--;

			}
		}

		//And here we trigger steps if there are some and the time from last step is lower then
		//Minimum distance in between steps that could be set by public setter
		if ((counter > 0) &&
			((hardware.getElapsedBastlCycles() - lastClockInTime) > 20)) {
			//-do step

			lastClockInTime = hardware.getElapsedBastlCycles();
			counter--;
		}


}
void doMultStep(){
		counter += counterLeft;
		counterLeft = ((multiplication) - 1);
		if (flop) {
		clockInTime = hardware.getElapsedBastlCycles() - channelTime;
		}
		channelTime = hardware.getElapsedBastlCycles();
		lastChannelTime = hardware.getElapsedBastlCycles();
		counter++;
		renderMultiplier();
		flop = true;
}


#define NUMBER_OF_CHANNELS 6
#define NUMBER_OF_VALUES 4
uint8_t channelMode[NUMBER_OF_CHANNELS];
uint8_t channelValue[NUMBER_OF_CHANNELS][NUMBER_OF_VALUES];


void reflectModeChange(uint8_t channel){
	switch(channelMode[channel]){
	case TRIGGER_MODE:
		break;
	case GATE_MODE:
		break;
	case CV_MODE:
		break;
	case INTERPOLATE_MODE:
		buffer[channel].interpolateMode(true);
		break;
	case LFO_MODE:
		buffer[channel].interpolateMode(false);

		break;
	case ADSR_MODE:
		buffer[channel].interpolateMode(false);
		break;
	}
}

uint32_t clockInDuration, lastTime;
uint8_t currentStepNumber[6]={0,0,0,0,0,0};
bool wshape[6];
const uint16_t usefulLengths[16]={4,3,2,1,2,3,4,6,8,16,24,32,48,64,128,256};
const uint16_t usefulLengthsEven[11]={4,2,1,2,4,8,16,32,64,128,256};
bool randomLfo[6];
bool odd[6];
bool activateMetaDivider[6];
void reflectValueChange(uint8_t channel, uint8_t value){
	uint8_t rate=0;
	uint8_t form=0;
	uint8_t shape=0;
	uint8_t res=0;
	switch(channelMode[channel]){
	case TRIGGER_MODE:
		break;
	case GATE_MODE:
		break;
	case CV_MODE:
		break;
	case INTERPOLATE_MODE:
		//hardware.setDAC(channel,channelValue[channel][2]);
		buffer[channel].interpolateMode(true);
		buffer[channel].setInterpolationAmount(channelValue[channel][1]);
		channelSync[channel]=bitRead(channelValue[channel][SETTINGS_BYTE],AUT_SYNC_BIT);
		odd[channel]=bitRead(channelValue[channel][SETTINGS_BYTE],AUT_ODD_BIT);
		break;
	case LFO_MODE:
		rate=channelValue[channel][0];
		form=channelValue[channel][1];
		shape=channelValue[channel][2];
		activateMetaDivider[channel]=false;


		buffer[channel].interpolateMode(false);
		channelSync[channel]=bitRead(channelValue[channel][SETTINGS_BYTE],LFO_SYNC_BIT);
		if(bitRead(channelValue[channel][SETTINGS_BYTE],SYNC_BIT)){
			channelSync[channel]=false;
			//LFO[channel].setToStep(LFO[channel].getCurrentStep()+rate,0);
		}
		else{
			rate+=hardware.getCVValue(channel);
		}


		odd[channel]=bitRead(channelValue[channel][SETTINGS_BYTE],LFO_ODD_BIT);

		if(channelSync[channel]){

			if(odd[channel]){
				syncFactor[channel]=usefulLengths[15-(rate>>4)];
				if((15-(rate>>4))<4) syncFast[channel]=true;
				else syncFast[channel]=false;
			}
			else{

				syncFactor[channel]=usefulLengthsEven[map(rate,255,0,0,11)];
				if(map(rate,255,0,0,11)<3) syncFast[channel]=true;
				else syncFast[channel]=false;
			}



			if(syncFast[channel]) LFO[channel].setBastlCyclesPerPeriod(clockInDuration/syncFactor[channel]),buffer[channel].setBastlCyclesPerPeriod(clockInDuration/syncFactor[channel]);
			else LFO[channel].setBastlCyclesPerPeriod(clockInDuration*syncFactor[channel]),buffer[channel].setBastlCyclesPerPeriod(clockInDuration*syncFactor[channel]);

		}
		else{
		//	LFO[channel].setBastlCyclesPerPeriod((256-channelValue[channel][0])<<6);
			if(bitRead(channelValue[channel][SETTINGS_BYTE],SYNC_BIT)){
				if(odd[channel]){
					syncFactor[channel]=usefulLengths[15-(rate>>4)];
					if((15-(rate>>4))<4) syncFast[channel]=true;
					else syncFast[channel]=false;
				}
				else{
					syncFactor[channel]=usefulLengthsEven[map(rate,0,255,0,11)];
					if(map(rate,0,255,0,11)<3) syncFast[channel]=true;
					else syncFast[channel]=false;
				}

				LFO[channel].setBastlCyclesPerPeriod(lfoClockInDuration[channel]);
			//	if(syncFast[channel]) LFO[channel].setBastlCyclesPerPeriod(lfoClockInDuration[channel]/syncFactor[channel]),buffer[channel].setBastlCyclesPerPeriod(lfoClockInDuration[channel]/syncFactor[channel]);
			//	else LFO[channel].setBastlCyclesPerPeriod(lfoClockInDuration[channel]*syncFactor[channel]),buffer[channel].setBastlCyclesPerPeriod(lfoClockInDuration[channel]*syncFactor[channel]);
			}
			else{

				if(!odd[channel]){
					activateMetaDivider[channel]=true;
					 LFO[channel].setBastlCyclesPerPeriod(curveMap(255-rate,LFOMAP_POINTS,LFOMap2));
					 buffer[channel].setBastlCyclesPerPeriod(curveMap(255-rate,LFOMAP_POINTS,bufferMap));
				}
				else{
					LFO[channel].setBastlCyclesPerPeriod(curveMap(255-rate,LFOMAP_POINTS,LFOMap));
					buffer[channel].setBastlCyclesPerPeriod(curveMap(255-rate,LFOMAP_POINTS,bufferMap));
				}
			}
		}

		buffer[channel].setSmootingAmount(form);

		//make sofisticated map
		LFO[channel].setResolution(255);//channelValue[channel][1]>>3);
		if(value!=0){
		switch(map(shape,0,255,0,6)){ //
		case 1:
			LFO[channel].setWaveform(SAW);
			LFO[channel].setFlop(0);
			LFO[channel].setXOR(form);
			LFO[channel].setResolution(255);
			//,false,false,FOLDING);
			wshape[channel]=false;
			randomLfo[channel]=false;
			break;
		case 0:
			LFO[channel].setWaveform(TRIANGLE);
			LFO[channel].setFlop(0);
			LFO[channel].setXOR(form);
			LFO[channel].setResolution(255);
			//,false,false,FOLDING);
			wshape[channel]=false;
			randomLfo[channel]=false;
			break;
		case 2:
			LFO[channel].setWaveform(SAW);
			LFO[channel].setFlop(0);
			//res=9-(form>>5);
			if(form>127){
			//	res=(15-res)-7;
				//LFO[channel].setResolution((1<<res));
				LFO[channel].setXOR(0);
			}
			else{
				//LFO[channel].setResolution((1<<res));
				LFO[channel].setXOR(255);
			}

			//,true,false,FOLDING);

			wshape[channel]=true;
			randomLfo[channel]=false;
			break;

		case 3:
			LFO[channel].setWaveform(TRIANGLE);//
			LFO[channel].setFlop(1<<4);
			LFO[channel].setXOR(form);
			LFO[channel].setResolution(255);
			//,false,true,FOLDING);
			wshape[channel]=false;
			randomLfo[channel]=false;
			break;
		case 4:
			LFO[channel].setWaveform(SAW);
			LFO[channel].setFlop(1<<4);
			LFO[channel].setXOR(form);
			LFO[channel].setResolution(255);
			//,true,true,FOLDING);
			wshape[channel]=false;
			randomLfo[channel]=false;
			break;
		case 5:

			if(channelValue[channel][2]>=254) buffer[channel].loopRandom(true);
			else buffer[channel].loopRandom(false);
			LFO[channel].setWaveform(RANDOM);
			//,false,false,FOLDING);

			randomLfo[channel]=true;
			wshape[channel]=false;
			break;


		}
		}

		break;
	case ADSR_MODE:
		buffer[channel].interpolateMode(false);
		channelSync[channel]=false;//bitRead(channelValue[channel][SETTINGS_BYTE],SYNC_BIT);
		if( bitRead(channelValue[channel][SETTINGS_BYTE],LOOP_BIT) ){

		}
		if(value==3){
			if( bitRead(channelValue[channel][SETTINGS_BYTE],LIN_LOG_BIT) ){
				envelope[channel].setTargetRatioA(100);
				envelope[channel].setTargetRatioDR(100);
			}
			else{
				envelope[channel].setTargetRatioA(0.5);
				envelope[channel].setTargetRatioDR(0.01);
			}
			envelope[channel].setSustainLevel((float)channelValue[channel][1]/255.0);
			envelope[channel].setAttackRate(curveMap(channelValue[channel][0],ADSRMAP_POINTS,ADSRMap));
			envelope[channel].setDecayRate(curveMap(channelValue[channel][2],ADSRMAP_POINTS,ADSRMap));
			envelope[channel].setReleaseRate(curveMap(channelValue[channel][2],ADSRMAP_POINTS,ADSRMap));
			envelope[channel].setHoldRate(curveMap(channelValue[channel][1],ADSRMAP_POINTS,ADSRMap));
		}
		if(channelSync[channel]){
/*
			uint8_t syncFactor=usefulLengths[(channelValue[channel][0]>>4)];
			if(value==0){
			if(((channelValue[channel][0]>>4))<3) envelope[channel].setAttackRate(clockInDuration/syncFactor);
			else envelope[channel].setAttackRate(clockInDuration*syncFactor);
			}
			if(value==2){
			syncFactor=usefulLengths[15-(channelValue[channel][2]>>4)];
			if(((channelValue[channel][0]>>4))<3) envelope[channel].setDecayRate(clockInDuration/syncFactor),envelope[channel].setReleaseRate(clockInDuration/syncFactor);
			else envelope[channel].setDecayRate(clockInDuration*syncFactor),envelope[channel].setReleaseRate(clockInDuration*syncFactor);
			}
			if(value==1){
			syncFactor=usefulLengths[15-(channelValue[channel][1]>>4)];
			if(((channelValue[channel][1]>>4))<3) envelope[channel].setHoldRate(clockInDuration/syncFactor);
			else envelope[channel].setHoldRate(clockInDuration*syncFactor);
			}
*/
		}
		else{
			if(value==1) envelope[channel].setSustainLevel((float)channelValue[channel][1]/255.0);
			if(value==0) envelope[channel].setAttackRate(curveMap(channelValue[channel][0],ADSRMAP_POINTS,ADSRMap));
			if(value==2) envelope[channel].setDecayRate(curveMap(channelValue[channel][2],ADSRMAP_POINTS,ADSRMap));
			if(value==2) envelope[channel].setReleaseRate(curveMap(channelValue[channel][2],ADSRMAP_POINTS,ADSRMap));
			if(value==1) envelope[channel].setHoldRate(curveMap(channelValue[channel][1],ADSRMAP_POINTS,ADSRMap));
		}
		if( bitRead(channelValue[channel][SETTINGS_BYTE],HOLD_BIT) ){
			envelope[channel].setHold(true);
		}
		else envelope[channel].setHold(false);
		break;
	}
}


void clockCall(uint8_t number){

}
void stepCall(uint8_t number){

}
void gateCall(uint8_t number){

}
void triggerCall(uint8_t number){

	switch(channelMode[number]){
		case INTERPOLATE_MODE:
			currentStepNumber[number]=0;
			buffer[number].setStep(currentStepNumber[number]);
			break;
		case LFO_MODE:
			currentStepNumber[number]=0;
			buffer[number].setStep(currentStepNumber[number]);
			LFO[number].setToStep(syncPhase[number],hardware.getElapsedBastlCyclesLong());
			break;
		case ADSR_MODE:
			envelope[number].reTrigger();
			break;
	}
	/*
	gateOut[number]=true;
	hardware.setDAC(number,255);
	triggerTime[number]=hardware.getElapsedBastlCyclesLong();
	*/
}
void startCall(uint8_t number){

}
void stopCall(uint8_t number){

}
void restartCall(uint8_t number){

}

void selectCall(uint8_t number){
	selectedChannel=number;
	hardware.setSelect(number);
	com.sendPairMessage();
}
void channelTriggerCall(uint8_t channel,uint8_t number){

}
void channelGateCall(uint8_t channel,uint8_t number){
	if(number==0) hardware.setDAC(channel,number);
	else hardware.setDAC(channel,255);
}
void channelCVCall(uint8_t channel,uint8_t number){
	 hardware.setDAC(channel,number);
}
uint8_t writeToStep[6];
bool channelRecord[6];
void channelInterpolateFromCall(uint8_t channel,uint8_t number){
	writeToStep[channel]=number;
	channelRecord[channel]=false;
	//line[channel].set(number); //number
}
void channelInterpolateToCall(uint8_t channel,uint8_t _number){
	if((writeToStep[channel]>=0) && (writeToStep[channel]<=31)) buffer[channel].setStepValue(writeToStep[channel],_number);
	buffer[channel].interpolateMode(true);
	channelRecord[channel]=false;
}

void channeInterpolateCall(uint8_t channel,uint8_t number){
	channelRecord[channel]=true;
	buffer[channel].setStepValue(currentStepNumber[channel],number);
	buffer[channel].interpolateMode(true);
	channelCVCall(channel,number);
	if(selectedChannel==channel) channelCVCall(6,number);
}
void channelModeCall(uint8_t channel,uint8_t number){
	channelMode[channel]=number;
	reflectModeChange(channel);
}

void channelValueCall(uint8_t channel, uint8_t value, uint8_t number){
	if(value==0 && channelMode[channel]==LFO_MODE){
		//if(abs(number-channelValue[channel][value])>1){
			if(bitRead(channelValue[channel][SETTINGS_BYTE],SYNC_BIT)){
				channelSync[channel]=false;
				LFO[channel].setToStep(LFO[channel].getCurrentStep()+number-channelValue[channel][value],0);
			}
		//}
	}
	channelValue[channel][value]=number;
	reflectValueChange(channel,value);
}
uint16_t syncStepNumber;

void clockInCall(){

	clockInDuration=hardware.getElapsedBastlCyclesLong()-lastTime;
	lastTime=hardware.getElapsedBastlCyclesLong();
	//if(currentStepNumber<255) currentStepNumber++;
	//else
	for(int i=0;i<6;i++){
		if((channelMode[i]==INTERPOLATE_MODE && bitRead(channelValue[i][SETTINGS_BYTE],AUT_SYNC_BIT)) || (channelMode[i]==LFO_MODE && channelSync[i])){
			uint8_t _numberOfSteps=0;
			if(odd[i]) _numberOfSteps=(channelValue[i][2]>>3)+1;
			else _numberOfSteps=1<<map(channelValue[i][2],0,255,0,5);
			if(currentStepNumber[i]<_numberOfSteps) currentStepNumber[i]++;
			else currentStepNumber[i]=0;
		}
	}

	if(syncStepNumber<256) syncStepNumber++;
	else syncStepNumber=0;

	if(channelMode[selectedChannel]==INTERPOLATE_MODE && bitRead(channelValue[selectedChannel][SETTINGS_BYTE],AUT_SYNC_BIT)) com.sendStep(currentStepNumber[selectedChannel]);
	for(int i=0;i<6;i++){

		if(selectedChannel==i && channelMode[i]!=INTERPOLATE_MODE){
			if(channelSync[i]){
				if( syncStepNumber%4 == 0){ //syncFactor
					com.sendStep(syncStepNumber);
				}
			}
		}

		if(channelMode[i]==INTERPOLATE_MODE && bitRead(channelValue[i][SETTINGS_BYTE],AUT_SYNC_BIT)){
			buffer[i].setBastlCyclesPerPeriod(clockInDuration);
			buffer[i].setStep(currentStepNumber[i]);
		}
		else if(channelSync[i]){
			if(syncFast[i]){
				LFO[i].setToStep(syncPhase[i],hardware.getElapsedBastlCyclesLong());
				//LFO[i].setBastlCyclesPerPeriod(clockInDuration);
				buffer[i].sync();
				envelope[i].sync();
				reflectValueChange(i,0);
			}
			else{
				if(syncStepNumber%syncFactor[i]==0){
					buffer[i].sync();
					LFO[i].setToStep(syncPhase[i],hardware.getElapsedBastlCyclesLong());
					//LFO[i].setBastlCyclesPerPeriod(clockInDuration);
					envelope[i].sync();
					reflectValueChange(i,0);
				}
			}
		}
	}

	//com.sendClock(1);
}
void resetInCall(uint8_t _number){
	triggerCall(_number);
}

void setup() {
	//create callbacks
	com.init(38400);
	com.attachClockCallback(&clockCall);
	com.attachStepCallback(&stepCall);
	com.attachGateCallback(&gateCall);
	com.attachTriggerCallback(&triggerCall);
	com.attachStartCallback(&startCall);
	com.attachStopCallback(&stopCall);
	com.attachRestartCallback(&restartCall);
	com.attachSelectCallback(&selectCall);
	com.attachChannelTriggerCallback(&channelTriggerCall);
	com.attachChannelGateCallback(&channelGateCall);
	com.attachChannelCVCallback(&channelCVCall);
	com.attachChannelInterpolateFromCallback(&channelInterpolateFromCall);
	com.attachChannelInterpolateToCallback(&channelInterpolateToCall);
	com.attachChannelInterpolateCallback(&channeInterpolateCall);
	com.attachChannelModeCallback(&channelModeCall);
	com.attachChannelValueCallback(&channelValueCall);

	//Serial.begin(9600);
	hardware.init(&clockInCall,&resetInCall);
	for(int i=0;i<6;i++){
		envelope[i].setAttackRate(1000);
		envelope[i].setDecayRate(2000);
		envelope[i].setReleaseRate(3000);
		envelope[i].setSustainLevel(0.5);
		envelope[i].setTargetRatioA(0.5);
		envelope[i].setTargetRatioDR(0.01);
		LFO[i].init(1000);
		LFO[i].setBastlCyclesPerPeriod(500);
	//	LFO[i].setFlop(1<<5);
		//LFO[i].setFreq(100);
		buffer[i].init(i);
	}
	_time=hardware.getElapsedBastlCyclesLong();
	 //LFO.setBastlCyclesPerPeriod(800);
}

uint16_t testTime=0;
uint8_t metaDivider;
void renderOutput(){

	uint8_t selectedOutput=0;
	for(int i=0;i<6;i++){
		uint8_t out=0;
		float fout=0;
		uint16_t form;
		switch(channelMode[i]){
			case TRIGGER_MODE:
				/*
				if((hardware.getCVValue(i)>64) && gate[i]==false) gate[i]=true, com.sendTrigger(i);
				else if((hardware.getCVValue(i)<=64) && gate[i]==true) gate[i]=false;
				if(gateOut[i]){
					if(hardware.getElapsedBastlCyclesLong()-triggerTime[i]>TRIGGER_LENGTH){
						gateOut[i]=false;
						hardware.setDAC(i,0);
					}
				}
				*/
				break;
			case GATE_MODE:
				break;
			case CV_MODE:
				if(hardware.getCVValue(i)!=hardware.getLastCVValue(i)){
					com.sendChannelCV(i,hardware.getCVValue(i));
				}
				break;
			case INTERPOLATE_MODE:
				buffer[i].update();
				if(!channelRecord[i]){
					out=buffer[i].getCurrentValue();
					hardware.setDAC(i,out);
				}
				if(!bitRead(channelValue[i][SETTINGS_BYTE],AUT_SYNC_BIT)){
					if((hardware.getCVValue(i)>64) && gate[i]==false){
						gate[i]=true; //int clock
						uint8_t _numberOfSteps=0;
						if(odd[i]) _numberOfSteps=(channelValue[i][2]>>3);
						else _numberOfSteps=1<<map(channelValue[i][2],0,255,0,5);
						if(currentStepNumber[i]<_numberOfSteps) currentStepNumber[i]++;
						else currentStepNumber[i]=0;
						lfoClockInDuration[i]=hardware.getElapsedBastlCyclesLong()-lastLfoTime[i];
						lastLfoTime[i]=hardware.getElapsedBastlCyclesLong();
						if(i==selectedChannel) com.sendStep(currentStepNumber[i]);
						buffer[i].setBastlCyclesPerPeriod(lfoClockInDuration[i]);
						buffer[i].setStep(currentStepNumber[i]);
					}
					else if((hardware.getCVValue(i)<=64) && gate[i]==true) gate[i]=false;
				}

				//,envelope[i].gate(gate[i]);
				break;
			case LFO_MODE:
				if(bitRead(channelValue[i][SETTINGS_BYTE],SYNC_BIT)){
					if((hardware.getCVValue(i)>64) && gate[i]==false){
						gate[i]=true; //int clock
						if(currentStepNumber[i]<255) currentStepNumber[i]++;
						else currentStepNumber[i]=0;
						if(i==selectedChannel) com.sendStep(currentStepNumber[i]);
						lfoClockInDuration[i]=hardware.getElapsedBastlCyclesLong()-lastLfoTime[i];
						lastLfoTime[i]=hardware.getElapsedBastlCyclesLong();
						LFO[i].setBastlCyclesPerPeriod(lfoClockInDuration[i]);
						if(syncFast[i]){
							//LFO[i].setToStep(syncPhase[i],hardware.getElapsedBastlCyclesLong());
							//LFO[i].setBastlCyclesPerPeriod(clockInDuration);
							buffer[i].sync();
							reflectValueChange(i,0);
						}
						else{
							if(syncStepNumber%syncFactor[i]==0){
								buffer[i].sync();
							//	LFO[i].setToStep(syncPhase[i],hardware.getElapsedBastlCyclesLong());
								//LFO[i].setBastlCyclesPerPeriod(clockInDuration);
								reflectValueChange(i,0);
							}
						}


					}
					else if((hardware.getCVValue(i)<=64) && gate[i]==true) gate[i]=false;
				}

				else if(hardware.getCVValue(i)!=hardware.getLastCVValue(i)){
					reflectValueChange(i,0);
				}

				if(randomLfo[i]){
					buffer[i].update();
					out=buffer[i].getCurrentValue();
				}
				else{
					if(activateMetaDivider[i]) {
						if(metaDivider==0) LFO[i].step();//testTime
					}
					else{
						LFO[i].step();
					}
					out=LFO[i].getValue();//hardware.getElapsedBastlCyclesLong()); //reflectValueChange

					if(wshape[i]){
						form=channelValue[i][1];
						if(form>127){
							form=(255-form)>>4;
						}
						else{
							form=form>>4;
						}
						if(form==7){
							if(out>127) out=255;
							else out=0;
						}
						else{
							out=out>>(form);
							out=out<<(form);
						}

					}
					else{
						//out=out^(form>>1);
					}
				}

				hardware.setDAC(i,out);
				break;
			case ADSR_MODE:
				if( bitRead(channelValue[i][SETTINGS_BYTE],LOOP_BIT) ){
					envelope[i].setLoop(true);
					if((hardware.getCVValue(i)>64) && gate[i]==false) gate[i]=true,envelope[i].reset();
					else if((hardware.getCVValue(i)<=64) && gate[i]==true) gate[i]=false;
				}
				else{
					envelope[i].setLoop(false);
					if((hardware.getCVValue(i)>64) && gate[i]==false) gate[i]=true,envelope[i].gate(gate[i]) ;
					else if((hardware.getCVValue(i)<=64) && gate[i]==true) gate[i]=false,envelope[i].gate(gate[i]);


				}
				fout=(float)255*envelope[i].process();
				out=(int)(fout);
				hardware.setDAC(i,out);

				break;
		}
		if(i==selectedChannel) selectedOutput=out;
	}
	if(!channelRecord[selectedChannel]) hardware.setDAC(6,selectedOutput);
	if(metaDivider<2) metaDivider++;
	else metaDivider=0;

}
uint32_t time;
void updateHW(){
	hardware.isr_updateClockIn();
	hardware.isr_updateDAC();
	hardware.isr_updateSelect();
	hardware.isr_updateReset();
}

void loop() {


	if(hardware.getElapsedBastlCyclesLong()!=_time){
		_time=hardware.getElapsedBastlCyclesLong();
		testTime++;
		renderOutput();
		updateHW();
	}
	com.update();
	hardware.isr_updateADC();



	/*
	for(int i=0;i<6;i++){
		Serial.print(i);
		Serial.print(": ");
		Serial.print(hardware.getCVValue(i));
		Serial.print("    ");
	}
	Serial.println();
delay(100);
*/

/*
if(hardware.getElapsedBastlCyclesLong()-time>100){
		time=hardware.getElapsedBastlCyclesLong();
		clockInCall();
	}
*/
/*
	for(uint8_t i=0;i<6;i++){
		if(resetTrig[i]) triggerCall(i);
	}
	*/
}


