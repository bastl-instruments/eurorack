

/*** ACTUAL CODE ***/

#include <Arduino.h>
#include "eventDelay.h"


void eventDelay::init(void(*eventNow)(uint8_t channel,uint8_t type)) {
	this->eventNow = eventNow;
	numberOfEventsInBuffer=0;

}

void eventDelay::update(uint32_t time){
	if(eventNow!=0){
		internalTime=time;
		if(numberOfEventsInBuffer!=0){
		for (uint8_t i=0;i<numberOfEventsInBuffer;i++){

				if(time>=timeBuffer[i]){
					uint8_t channel=typeBuffer[i]>>4;
					uint8_t type=0;
					bitWrite(type,0,bitRead(typeBuffer[i],0));
					bitWrite(type,1,bitRead(typeBuffer[i],1));
					bitWrite(type,2,bitRead(typeBuffer[i],2));
					bitWrite(type,3,bitRead(typeBuffer[i],3));
					eventNow(channel,type);

					shiftTimeBuffer(true,i);
					shiftTypeBuffer(true,i);
					numberOfEventsInBuffer--;

				}
		}
	}
	}
}
void eventDelay::createEvent(uint8_t channel, uint8_t type, uint32_t time){
	if(numberOfEventsInBuffer<(BUFFER_SIZE-1)){
		shiftTimeBuffer(false,0);
		shiftTypeBuffer(false,0);
		timeBuffer[0]=time;
		typeBuffer[0]=(channel<<4) | type;
		numberOfEventsInBuffer++;
	}
/*
	Serial.print("  ");
	Serial.print(channel);
	Serial.print("  ");
	Serial.print(type);
	Serial.print("  ");
	Serial.print(time);
	Serial.println("  ");
*/

}

void eventDelay::fillBuffers(){
	for(uint8_t i=0;i<BUFFER_SIZE;i++){
		timeBuffer[i]=random(3000);
		typeBuffer[i]=random(255);
	}

}
void eventDelay::clearBuffers(){

	for(uint8_t i=0;i<BUFFER_SIZE;i++){
		numberOfEventsInBuffer=0;
		timeBuffer[i]=0;
		typeBuffer[i]=255;
	}

}
void eventDelay::printBuffers(){
	numberOfEventsInBuffer=BUFFER_SIZE+1;
	Serial.println();
		Serial.println();
		Serial.println();
	for(uint8_t i=0;i<BUFFER_SIZE;i++){
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

void eventDelay::shiftTimeBuffer(bool left, uint8_t from){

	if(left){
		for(uint8_t i=from;i<numberOfEventsInBuffer+1;i++){
						timeBuffer[i]=timeBuffer[i+1];
					}

	}
	else{
		for(uint8_t i=numberOfEventsInBuffer+2;i>=from+1;i--){
					timeBuffer[i]=timeBuffer[i-1];
				}
	}

}

void eventDelay::shiftTypeBuffer(bool left, uint8_t from){
	if(left){
			for(uint8_t i=from;i<numberOfEventsInBuffer+1;i++){
							typeBuffer[i]=typeBuffer[i+1];
						}

		}
		else{
			for(uint8_t i=numberOfEventsInBuffer+2;i>=from+1;i--){
						typeBuffer[i]=typeBuffer[i-1];
					}
		}

}
