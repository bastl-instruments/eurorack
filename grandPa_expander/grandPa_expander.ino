#ifdef EXTERNAL_IDE

#include <Arduino.h>
#define PIN B,4


int main(void) {

  init();
  setup();

  while(true) {
    loop();
  }
}

#endif


#include <portManipulations.h>
#include <mapping.h>

#include "GPE_HW.h"
#include <simpleSerialDecoder.h>


//#define DEBUG

extern GPE_HW hardware;


simpleSerialDecoder com;

void setup() {
	hardware.init();
	com.init(38400);
	for(int i=0;i<12;i++){
		hardware.update();
		delay(10);
	}
}

const uint8_t cvMap[6]={0,1,2,3,5,4};
const uint8_t channelMapSwitch[6]={4,5,2,3,1,0};
const uint8_t channelMapNoSwitch[6]={4,5,6,7,1,0};
//uint8_t channel, cv;
uint8_t lastSent[6];
bool lastSwitchState;
void loop() {
	// get new CV and knob values
	if(lastSwitchState!=hardware.getSwitchState()){
		if(hardware.getSwitchState()){
			com.sendPairMessage();
			com.sendChannelCV(channelMapSwitch[3],hardware.getCVValue(cvMap[3]));
			com.sendPairMessage();
			com.sendChannelCV(channelMapSwitch[2],hardware.getCVValue(cvMap[2]));
			com.sendPairMessage();
			com.sendChannelCV(channelMapNoSwitch[3],0);
			com.sendPairMessage();
			com.sendChannelCV(channelMapNoSwitch[2],0);
		}
		else{
			com.sendPairMessage();
			com.sendChannelCV(channelMapSwitch[3],0);
			com.sendPairMessage();
			com.sendChannelCV(channelMapSwitch[2],0);
			com.sendPairMessage();
			com.sendChannelCV(channelMapNoSwitch[3],hardware.getCVValue(cvMap[3]));
			com.sendPairMessage();
			com.sendChannelCV(channelMapNoSwitch[2],hardware.getCVValue(cvMap[2]));
		}
	}
	lastSwitchState=hardware.getSwitchState();
	hardware.update();
	if(hardware.getSwitchState()){
		for(uint8_t i=0;i<4;i++){
			if(hardware.getCVValue(cvMap[i])!=hardware.getLastCVValue(cvMap[i])){
				com.sendPairMessage();
				com.sendChannelCV(channelMapSwitch[i],hardware.getCVValue(cvMap[i]));
				/*
				if(lastSent[cvMap[i]]!=hardware.getCVValue(cvMap[i])){

				}
				lastSent[cvMap[i]]=hardware.getCVValue(cvMap[i]);

				Serial.println(i);
				*/
			}
		}
	}
	else{
		for(uint8_t i=0;i<4;i++){
			if(hardware.getCVValue(cvMap[i])!=hardware.getLastCVValue(cvMap[i])){
				//Serial.println(i);
				com.sendPairMessage();
				if(i==3) com.sendChannelCV(channelMapNoSwitch[i],255-hardware.getCVValue(cvMap[i]));
				else com.sendChannelCV(channelMapNoSwitch[i],hardware.getCVValue(cvMap[i]));
			}
		}
	}


	if(hardware.getCVValue(cvMap[4])!=hardware.getLastCVValue(cvMap[4])){
		com.sendPairMessage();
		com.sendChannelCV(channelMapNoSwitch[4],hardware.getCVValue(cvMap[4]));
	}

	if(hardware.getCVValue(cvMap[5])>=100 && hardware.getLastCVValue(cvMap[5])<100){
		com.sendPairMessage();
		com.sendChannelTrigger(1,1);
		//Serial.println("t");
	}
delay(15);

}




