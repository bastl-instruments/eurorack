/***********************************************
 * Microchip DAC library
 * (c) 2012 Majenko Technologies
 *
 * This library is offered without any warranty as to
 * fitness for purpose, either explicitly or implicitly
 * implied.
 ***********************************************/

/*
 * Microchip DAC library.  This works with the MCP4822 chip and
 * similar chips that work with the same protocol.
 */

#include <Arduino.h>
#include <SPI.h>
#include "MCPDAC.h"
#include <portManipulations.h>
#include "midiSeqHW.h"
extern midiSeqHW hw;

const uint8_t channelUsedToCalibrate[4]={3,3,3,0};
const uint16_t tuningVoltage[9]={0,500,1000,1500,2000,2500,3000,3500,4000};
#define COMPARTOR_PIN  B,0

void MCPDACClass::init(uint8_t _cspin, uint8_t _cspin2)
{

	//bit_dir_inp(COMPARTOR_PIN);
	//bit_clear(COMPARTOR_PIN);
	pinMode(8,INPUT);
	pinMode(7,INPUT);
	csPin = _cspin;
	csPin2 = _cspin2;
	//this->ldacpin = ldacpin;
	pinMode(csPin,OUTPUT);
	digitalWrite(csPin,HIGH);
	pinMode(csPin2,OUTPUT);
	digitalWrite(csPin2,HIGH);
	SPI.begin();
}

void MCPDACClass::allDACLow(){
	for(uint8_t i=0;i<4;i++) writeDAC(i,0);
}
void MCPDACClass::writeDAC(uint8_t channel, uint16_t voltage)
{
	if(channel<4 && voltage<4096){
	uint8_t _csPin=0;
	if(channel/2){
		_csPin=csPin2;
	}
	else{
		_csPin=csPin;
	}
	uint16_t command=0;
	if((channel%2==0)) command = 0x0000;
	else command = 0x8000;
	command |= 0x1000;
	command |= 0x0000 ;
	command |= (voltage & 0x0FFF);

	SPI.setDataMode(SPI_MODE0);
	digitalWrite(_csPin,LOW);
	SPI.transfer(command>>8);
	SPI.transfer(command&0xFF);
	digitalWrite(_csPin,HIGH);
	}
}
uint8_t calPin[4]={3,3,3,4};
void MCPDACClass::autoCalibrate(uint8_t channel){
	allDACLow();
	digitalWrite(calPin[channel],LOW);
	for(uint8_t i=0;i<9;i++){
		calibrationTable[channel][i]=0;

		//allDACLow();
		writeDAC(channelUsedToCalibrate[channel],tuningVoltage[i]);
		writeDAC(channel,calibrationTable[channel][i]);
		//delay(500);

		while(!digitalRead(calPin[channel])){//bit_read_in(COMPARTOR_PIN)){
			calibrationTable[channel][i]++;
			writeDAC(channel,calibrationTable[channel][i]);
		//	Serial.print(".");
		}
	//	Serial.print(calibrationTable[channel][i]);
		//Serial.print(", ");

	}
	uint16_t differenceSum=0;
	for(uint8_t i=0;i<8;i++){
		uint16_t difference=calibrationTable[channel][i+1]-calibrationTable[channel][i];
		differenceSum+=difference;
	}
	uint16_t halfOctave=differenceSum/8;

	for(uint8_t i=9;i<15;i++){
		calibrationTable[channel][i]=calibrationTable[channel][i-1]+halfOctave;
		//Serial.print(calibrationTable[channel][i]);
		//Serial.print(", ");
	}
	//Serial.println(", ");
	digitalWrite(calPin[channel],HIGH);
}

void MCPDACClass::setNote(uint8_t channel, uint8_t note){

	uint8_t tableSegment=note/6;
	uint8_t numberOfSemitones=note%6;
	if(autoTuneActive[channel]){
		uint16_t semiToneSize=(autoTuneTable[channel][tableSegment+1]-autoTuneTable[channel][tableSegment]) /6;
		uint16_t DACoutput=autoTuneTable[channel][tableSegment]+ (semiToneSize*numberOfSemitones);
		writeDAC(channel,DACoutput);
	}
	else{
		uint16_t semiToneSize=(calibrationTable[channel][tableSegment+1]-calibrationTable[channel][tableSegment]) /6;
		uint16_t DACoutput=calibrationTable[channel][tableSegment]+ (semiToneSize*numberOfSemitones);
		writeDAC(channel,DACoutput);
	}

}

#define TUNE_PIN_A D,6
#define TUNE_PIN_B D,7
#define TUNE_PIN_C B,0
#define TUNE_PIN_D B,4
#define STATISTICAL_THRESHOLD 100

void MCPDACClass::autoTuneInit(){

	bit_dir_inp(TUNE_PIN_A);
	bit_set(TUNE_PIN_A);

	bit_dir_inp(TUNE_PIN_B);
	bit_set(TUNE_PIN_B);

	bit_dir_inp(TUNE_PIN_C);
	bit_set(TUNE_PIN_C);

	bit_dir_inp(TUNE_PIN_D);
	bit_set(TUNE_PIN_D);
}


void MCPDACClass::resetPinActivity(){
	for(uint8_t i=0;i<4;i++) pinActive[i]=false;
}

void MCPDACClass::activitySense(uint8_t _channel){
	bool newState=0;
	switch(_channel){
		case 0:
		newState=!bit_read_in(TUNE_PIN_A);
		if(tunePinState[0] !=newState) pinActive[0]=true;
		lastTunePinState[0]=tunePinState[0];
		tunePinState[0]=newState;
		break;
		case 1:
		newState=!bit_read_in(TUNE_PIN_B);
		if(tunePinState[1] !=newState) pinActive[1]=true;
		lastTunePinState[1]=tunePinState[1];
		tunePinState[1]=newState;
		break;
		case 2:
		newState=!bit_read_in(TUNE_PIN_C);
		if(tunePinState[2] !=newState) pinActive[2]=true;
		lastTunePinState[2]=tunePinState[2];
		tunePinState[2]=newState;
		break;
		case 3:
		newState=!bit_read_in(TUNE_PIN_D);
		if(tunePinState[3] !=newState) pinActive[3]=true;
		lastTunePinState[3]=tunePinState[3];
		tunePinState[3]=newState;
		break;

	}
}


void MCPDACClass::measureFrequency(uint8_t _channel){
	//frequency measurement running with hysteresis


	activitySense(_channel);
	if(!lastTunePinState[_channel] && tunePinState[_channel]){
		periods[_channel]++;
		uint32_t newPeriod=micros()-tuneTime[_channel];
		tuneTime[_channel]=micros();
		tunePeriod[_channel]+=newPeriod,tunePeriod[_channel]=tunePeriod[_channel]>>1;
/*  removal of statistically unlikely values - didnt make it to work :(
		if(discardedPeriod!=0){
			if(abs(newPeriod-tunePeriod[_channel])>abs(newPeriod-discardedPeriod[_channel])){
				tunePeriod[_channel]=discardedPeriod[_channel];
			}
			discardedPeriod[_channel]=0;
		}

		if(abs(newPeriod-tunePeriod[_channel])>STATISTICAL_THRESHOLD) discardedPeriod[_channel]=newPeriod;
		else tunePeriod[_channel]+=newPeriod,tunePeriod[_channel]=tunePeriod[_channel]>>1, discardedPeriod[_channel]=0; //hysteresis

*/
	}

}
uint16_t MCPDACClass::measureTwoVoltPeriod(uint8_t _channel){

	uint16_t period;
		autoTuneActive[_channel]=false;
		writeDAC(_channel,calibrationTable[_channel][4]); //twoVolt
		uint32_t _time=millis();
		pinActive[_channel]=false;

		while(millis()-_time<100){
			measureFrequency(_channel);
			//Serial.println("x");
		}

		if(pinActive[_channel]){

			//_time=millis();
			periods[_channel]=0;
			while(periods[_channel]<4){
				measureFrequency(_channel);
				//Serial.println("y");
			}
			period=tunePeriod[_channel];
			autoTuneActive[_channel]=true;
			return period;

		}
		else{
			autoTuneActive[_channel]=false;
			return 0;
		}
}
uint16_t MCPDACClass::measureOneVoltPeriod(uint8_t _channel){

	uint16_t period;
	autoTuneActive[_channel]=false;
	writeDAC(_channel,calibrationTable[_channel][2]); //oneVolt
	uint32_t _time=millis();
	pinActive[_channel]=false;
	while(millis()-_time<100){
		measureFrequency(_channel);
		//Serial.println("x");
	}

	if(pinActive[_channel]){

		periods[_channel]=0;
		while(periods[_channel]<4){
			measureFrequency(_channel);
			//Serial.println("y");
		}
		period=tunePeriod[_channel];
		autoTuneActive[_channel]=true;
		return period;

	}
	else{
		autoTuneActive[_channel]=false;
		return 0;
	}
}

void MCPDACClass::autoTuneCalibrateAbsolute(){


}
#define ACCEPTABLE_COARSE_DIFFERENCE 2000
#define ACCEPTABLE_DIFFERENCE 500
uint8_t animation=0;


	/*
	 * alternative method
	 *
	 * measure bass frequency
	 *
	 *
	 * measure low frequency
	 * move one volt
	 *
	 * while{
	 * measure new frequency
	 * compare new and low frequency and deduce the DAC difference
	 * }
	 */
void MCPDACClass::autoTuneCalibrateHeuristic(){
	uint16_t tuneAllTo=0;
	int firstResponse[4];
	int secondResponse[4];

	for(uint8_t i=0;i<4;i++) if(autoTuneActive[i]) writeDAC(i,calibrationTable[i][0]); //set zero volts everywhere
	uint32_t _time=millis();

	if(autoTuneActive[0]){

		while((millis()-_time)<200){  //measure all frequencies
			measureFrequency(0);
			measureFrequency(1);
			measureFrequency(2);
			measureFrequency(3);
		}
		tuneAllTo=tunePeriod[0];
		for(uint8_t i=1;i<4;i++){
			firstResponse[i]=tunePeriod[i]; //remember that frequency for other oscillators
			writeDAC(i,calibrationTable[i][2]); //add one volt for start
		}
		while((millis()-_time)<200){ //measure frequency again
			measureFrequency(1);
			measureFrequency(2);
			measureFrequency(3);
		}

		Serial.println(tuneAllTo);
		for(uint8_t i=1;i<4;i++){ //evaluate and try to get to the right offset
			uint16_t oneVoltDistance=calibrationTable[i][2]-calibrationTable[i][0];
			secondResponse[i]=tunePeriod[i];
			 int absoluteTuningError=tuneAllTo-tunePeriod[i];
			 int relativeTuningError=map(absoluteTuningError,0,firstResponse[i]-secondResponse[i],0,oneVoltDistance);//255);//PWM_STEPS_PER_SEMITONE);
			int DACdifference =relativeTuningError;
			writeDAC(i,calibrationTable[i][0]+DACdifference);
			firstResponse[i]=secondResponse[i];
		}

		//while(holdingbutton)

		while((millis()-_time)<200){ //use for out of bounds animation
			measureFrequency(1);
			measureFrequency(2);
			measureFrequency(3);
		}
		for(uint8_t i=1;i<4;i++){
			if(abs(tunePeriod[i]-tuneAllTo)>ACCEPTABLE_DIFFERENCE){
				uint16_t oneVoltDistance=calibrationTable[i][2]-calibrationTable[i][0];
				secondResponse[i]=tunePeriod[i];
				 int absoluteTuningError=tuneAllTo-tunePeriod[i];
				 int relativeTuningError=map(absoluteTuningError,0,firstResponse[i]-secondResponse[i],0,oneVoltDistance);//255);//PWM_STEPS_PER_SEMITONE);
				int DACdifference =relativeTuningError;
				writeDAC(i,calibrationTable[i][0]+DACdifference);
				firstResponse[i]=secondResponse[i];
			}
			else{
				hw.setGate(i,true);

			}

		}


	}
	else{ //calibration failed
		Serial.println("failed");
	}
}

void MCPDACClass::autoTuneCalibrateRelative(){
	uint16_t tuneAllTo=0;
	uint16_t oneVoltResponse[4];
	uint16_t twoVoltResponse[4];
	for(uint8_t i=0;i<4;i++){
		Serial.print(i);
		oneVoltResponse[i]=measureOneVoltPeriod(i);
		twoVoltResponse[i]=measureTwoVoltPeriod(i);
	}
	for(uint8_t i=0;i<4;i++) if(autoTuneActive[i]) writeDAC(i,calibrationTable[i][0]);
	uint32_t _time=millis();

	if(autoTuneActive[0]){
		while((millis()-_time)<200){
			measureFrequency(0);
		}
		tuneAllTo=tunePeriod[0];
		Serial.println(tuneAllTo);

		for(uint8_t i=1;i<4;i++){
			Serial.print(i);
			Serial.print(": ");
			if(autoTuneActive[i]){
				while(abs(tunePeriod[1]-tuneAllTo)>ACCEPTABLE_DIFFERENCE){ // || timeout implementation
					_time=millis();
					while(millis()-_time<100){
						measureFrequency(i);
					}
					Serial.print(" goal: ");
					Serial.print(tuneAllTo);
					Serial.print(" current: ");
					Serial.print(tunePeriod[i]);
					uint16_t oneVoltDistance=calibrationTable[i][4]-calibrationTable[i][2];
					//how much the period changes by changing one volt to how much it has to change
					 int absoluteTuningError=tuneAllTo-tunePeriod[i];
					 int relativeTuningError=map(absoluteTuningError,0,oneVoltResponse[i]-twoVoltResponse[i],0,oneVoltDistance);//255);//PWM_STEPS_PER_SEMITONE);
					int DACdifference =relativeTuningError;
					//int DACdifference=map(oneVoltDistance,tunePeriod[i],tuneAllTo,oneVoltResponse[i],twoVoltResponse[i]); //is bipolar
					Serial.print(", dacChange: ");
					Serial.print(DACdifference);
					if(((calibrationTable[i][0]+DACdifference)<0) || ((calibrationTable[i][0]+DACdifference)>4096)){ //problem
						Serial.print(", OUT_OF_RANGE");
						//SIGNALIZE YOU HAVE TO RE-TUNE THE OSC in some way...
						while(abs(tunePeriod[i]-tuneAllTo)>ACCEPTABLE_COARSE_DIFFERENCE){

							measureFrequency(i);
							if(tunePeriod[i]>tuneAllTo){
								if(!millis()%250) animation++;
								Serial.println("a+");
							}
							else{
								if(!millis()%250) animation--;
								Serial.println("a-");
							}
							hw.setGate(i,true);
							hw.lightNumber(53+animation);

						}
					}
					else{ //we are good to continue and measure again
						Serial.print(", IN_RANGE");
						writeDAC(i,calibrationTable[i][0]+DACdifference);
					}
					while(millis()-_time<100){
						measureFrequency(i);
					}
					Serial.print(", result:");
					Serial.println(tunePeriod[i]);
				}

				//introduce offset table calculation here
			}
		}

	}
	else{ //calibration failed
		Serial.println("failed");
	}
}
