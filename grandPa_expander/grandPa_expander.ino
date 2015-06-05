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


const uint8_t channelMapSwitch[6]={4,5,2,3,1,0};
const uint8_t channelMapNoSwitch[6]={4,5,6,7,1,0};
//uint8_t channel, cv;
uint8_t lastSent[6];
bool lastSwitchState;
void loop() {
	hardware.update();

	if(lastSwitchState!=hardware.getSwitchState()){
		if(hardware.getSwitchState()){
			com.sendPairMessage();
			com.sendChannelCV(channelMapSwitch[3],hardware.getCVValue(3));
			com.sendPairMessage();
			com.sendChannelCV(channelMapSwitch[2],hardware.getCVValue(2));
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
			com.sendChannelCV(channelMapNoSwitch[3],hardware.getCVValue(3));
			com.sendPairMessage();
			com.sendChannelCV(channelMapNoSwitch[2],hardware.getCVValue(2));
		}
	}
	lastSwitchState=hardware.getSwitchState();

	if(hardware.getSwitchState()){
		for(uint8_t i=0;i<4;i++){
			if(hardware.getCVValue(i)!=hardware.getLastCVValue(i)){
				com.sendPairMessage();
				com.sendChannelCV(channelMapSwitch[i],hardware.getCVValue(i));
			}
		}
	}
	else{
		for(uint8_t i=0;i<4;i++){
			if(hardware.getCVValue(i)!=hardware.getLastCVValue(i)){
				//Serial.println(i);
				com.sendPairMessage();
				if(i==3) com.sendChannelCV(channelMapNoSwitch[i],255-hardware.getCVValue(i));
				else com.sendChannelCV(channelMapNoSwitch[i],hardware.getCVValue(i));
			}
		}
	}


	if(hardware.getCVValue(4)!=hardware.getLastCVValue(4)){
		com.sendPairMessage();
		com.sendChannelCV(channelMapNoSwitch[4],hardware.getCVValue(4));
	}

	if(hardware.getCVValue(5)>=100 && hardware.getLastCVValue(5)<100){
		com.sendPairMessage();
		com.sendChannelTrigger(1,1);
		//Serial.println("t");
	}
	delay(15);

}




