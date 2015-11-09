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
		hardware.update(i%5);
		delay(5);
	}
}


const uint8_t channelMapSwitch[6]={4,5,2,3,1,0};
const uint8_t channelMapNoSwitch[6]={4,5,6,7,1,0};
//uint8_t channel, cv;
uint8_t lastSent[6];
bool lastSwitchState;
uint8_t finalSetting[8]={0,0,0,0,0,0,0,0};
void renderSwitchChange(){
	if(lastSwitchState!=hardware.getSwitchState()){
		if(hardware.getSwitchState()){
			com.sendPairMessage();
			com.sendChannelCV(channelMapSwitch[3],hardware.getCVValue(3));
			com.sendPairMessage();
			com.sendChannelCV(channelMapSwitch[2],hardware.getCVValue(2));
			com.sendPairMessage();
			com.sendChannelCV(channelMapNoSwitch[3],255);
			com.sendPairMessage();
			com.sendChannelCV(channelMapNoSwitch[2],0);
		}
		else{
			com.sendPairMessage();
			com.sendChannelCV(channelMapSwitch[3],0);
			com.sendPairMessage();
			com.sendChannelCV(channelMapSwitch[2],0);
			com.sendPairMessage();
			com.sendChannelCV(channelMapNoSwitch[3],255-hardware.getCVValue(3));
			com.sendPairMessage();
			com.sendChannelCV(channelMapNoSwitch[2],hardware.getCVValue(2));
		}
	}
	lastSwitchState=hardware.getSwitchState();
}
uint8_t i=0;
uint8_t preventUpdate=0;
uint32_t time2;
void loop() {
	if(i<4) i++;
	else i=0;

	hardware.update(i);
	renderSwitchChange();

	if(i<4){
		if(hardware.getSwitchState()){

				if(hardware.getCVValue(i)!=hardware.getLastCVValue(i) || finalSetting[channelMapSwitch[i]]!=hardware.getCVValue(i)){
					com.sendPairMessage();
					com.sendChannelCV(channelMapSwitch[i],hardware.getCVValue(i));
					finalSetting[channelMapSwitch[i]]=hardware.getCVValue(i);
				}

		}
		else{

				if(hardware.getCVValue(i)!=hardware.getLastCVValue(i) || finalSetting[channelMapNoSwitch[i]]!=hardware.getCVValue(i)){
					//Serial.println(i);
					com.sendPairMessage();
					if(i==3){
						com.sendChannelCV(channelMapNoSwitch[i],255-hardware.getCVValue(i));
					}
					else {
						com.sendChannelCV(channelMapNoSwitch[i],hardware.getCVValue(i));
					}
					finalSetting[channelMapNoSwitch[i]]=hardware.getCVValue(i);
				}

		}

	}
	else{
		if(hardware.getCVValue(4)!=hardware.getLastCVValue(4) || finalSetting[channelMapNoSwitch[4]]!=hardware.getCVValue(4)){
			com.sendPairMessage();
			com.sendChannelCV(channelMapNoSwitch[4],hardware.getCVValue(4));
			finalSetting[channelMapNoSwitch[4]]=hardware.getCVValue(4);
		}
	}


	uint32_t time=millis();
	while((millis()-time)<10){
		hardware.update(5);
		if(hardware.getCVValue(5)>=100 && hardware.getLastCVValue(5)<100){

			com.sendPairMessage();
			com.sendChannelTrigger(2,4);
			com.sendPairMessage();
			com.sendChannelTrigger(4,5);
			//Serial.println("t");
		}
	}


	if((millis()-time2)>150){
		time2=millis();
		if(preventUpdate<4) preventUpdate++;
		else preventUpdate=0;
		if(preventUpdate<4){
			if(hardware.getSwitchState()){
				com.sendPairMessage();
				com.sendChannelCV(channelMapSwitch[preventUpdate],hardware.getCVValue(preventUpdate));
				com.sendPairMessage();
				if(preventUpdate==2) com.sendPairMessage(),com.sendChannelCV(channelMapNoSwitch[preventUpdate],0);
				else if (preventUpdate==3) com.sendPairMessage(), com.sendChannelCV(channelMapNoSwitch[preventUpdate],255);
				finalSetting[channelMapSwitch[preventUpdate]]=hardware.getCVValue(preventUpdate);
			}
			else{
				com.sendPairMessage();
				if(preventUpdate==3){
					com.sendChannelCV(channelMapNoSwitch[preventUpdate],255-hardware.getCVValue(preventUpdate));
				}
				else {
					com.sendChannelCV(channelMapNoSwitch[preventUpdate],hardware.getCVValue(preventUpdate));
				}
				if(preventUpdate==2 || preventUpdate==3) com.sendPairMessage(),com.sendChannelCV(channelMapSwitch[preventUpdate],0);
				finalSetting[channelMapNoSwitch[preventUpdate]]=hardware.getCVValue(preventUpdate);
			}
		}
		else{
				com.sendPairMessage();
				com.sendChannelCV(channelMapNoSwitch[4],hardware.getCVValue(4));
				finalSetting[channelMapNoSwitch[4]]=hardware.getCVValue(4);
		}

	}
	//delay(10);

}




