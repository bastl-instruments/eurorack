

/*** ACTUAL CODE ***/

#include <Arduino.h>
#include "eventMemory.h"


void eventMemory::init(void(*recEventNow)(uint8_t channel,uint8_t type, uint8_t cycles)) {
	this->recEventNow = recEventNow;
	numberOfEventsInBuffer=0;
}

void eventMemory::update(uint32_t time){
	if(state==RECORDING_STATE){


	}
	else if(state==LOOPING_STATE){

		if(recEventNow!=0){
				internalTime=time;
				if(numberOfEventsInBuffer!=0){

					if(time>=stopTime){

							stopTime=time+(time-startTime);
							startTime=time;
							numberOfEventsPassed=0;
							refreshInitialState();
							//return;
						}

						while(time>=(timeBuffer[numberOfEventsPassed]+startTime)){
							if(numberOfEventsPassed<=numberOfEventsInBuffer){
							uint8_t buffOut=typeBuffer[numberOfEventsPassed];
							uint8_t channel=buffOut>>5;
							uint8_t type=0;
							uint8_t cycles=0;
							bitWrite(cycles,0,bitRead(buffOut,2));
							bitWrite(cycles,1,bitRead(buffOut,3));
							bitWrite(cycles,2,bitRead(buffOut,4));

							bitWrite(type,0,bitRead(buffOut,0));
							bitWrite(type,1,bitRead(buffOut,1));
							/*
							Serial.print(" b: ");
							Serial.print(numberOfEventsInBuffer);
							Serial.print(" n: ");
							Serial.print(numberOfEventsPassed);
							*/
							recEventNow(channel,type, cycles);

							numberOfEventsPassed++;

						}
						else return;
					}

			}
			}

	}

}
void eventMemory::createEvent(uint8_t channel, uint8_t type, uint8_t cycles, uint32_t time){
	if(numberOfEventsInBuffer<M_BUFFER_SIZE-1){
		timeBuffer[numberOfEventsInBuffer]=time-startTime;
		uint8_t buffOut=0;
		typeBuffer[numberOfEventsInBuffer]=0;
		buffOut=(channel<<5);

		bitWrite(buffOut,2,bitRead(cycles,0));
		bitWrite(buffOut,3,bitRead(cycles,1));
		bitWrite(buffOut,4,bitRead(cycles,2));

		bitWrite(buffOut,0,bitRead(type,0));
		bitWrite(buffOut,1,bitRead(type,1));
		typeBuffer[numberOfEventsInBuffer]=buffOut;

		numberOfEventsInBuffer++;
	//	Serial.print(" b: ");
		//		Serial.print(buffOut);
	}
/*
	Serial.print(" CH: ");
		Serial.print(channel);
		Serial.print(" t: ");
		Serial.print(type);
		Serial.print(" c: ");
		Serial.print(cycles);
		Serial.println();
*/

}

void eventMemory::fillBuffers(){
	for(uint8_t i=0;i<M_BUFFER_SIZE;i++){
		timeBuffer[i]=random(3000);
		typeBuffer[i]=random(255);
	}

}
void eventMemory::printBuffers(){
	numberOfEventsInBuffer=M_BUFFER_SIZE+1;
	Serial.println();
		Serial.println();
		Serial.println();
	for(uint8_t i=0;i<M_BUFFER_SIZE;i++){
		Serial.print(i);
		Serial.print(": ");
		Serial.print(timeBuffer[i]);
		Serial.print("  ");
		Serial.println(typeBuffer[i]);
	}
	Serial.println();
	Serial.println();
	Serial.println();
}

bool eventMemory::recording(){
	if(state==RECORDING_STATE) return true;
	else return false;
}
bool eventMemory::looping(){
	if(state==LOOPING_STATE) return true;
	else return false;
}

void eventMemory::startRecording(uint32_t time){
	//Serial.println("rec: ");
	state=RECORDING_STATE;
	startTime=time;

}
void eventMemory::loopRecorded(uint32_t time){
	//Serial.println("loop: ");
	state=LOOPING_STATE;
	stopTime=time+(time-startTime);
	startTime=time;
	numberOfEventsPassed=0;
	refreshInitialState();

}
void eventMemory::stopLoop(){
	state=STOP_STATE;
	clearBuffers();

}
void eventMemory::clearBuffers(){
	for (uint8_t i=0;i<M_BUFFER_SIZE;i++){
	 timeBuffer[i]=0;
	 typeBuffer[i]=0;
	}
	numberOfEventsInBuffer=0;
	numberOfEventsPassed=0;
}

void eventMemory::setInitialState(uint8_t _ch,bool state){
	initState[_ch]=state;
}
void eventMemory::refreshInitialState(){
	if(recEventNow!=0){
	for(int i=0;i<6;i++){
		if(initState[i]) recEventNow(i, 1, 0);
		else  recEventNow(i, 0, 0);
	}
	}

}
