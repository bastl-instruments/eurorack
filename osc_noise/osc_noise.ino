/*
 * LITTLE NERD
 * eurorack module by Bastl Instruments
 * www.bastl-instruments.com
 * Vaclav Pelousek
 * licensed under cc-by-sa
 *
 *TODO
 *finish visual testing
 *do audioble testing
 *implement production code = testing + eeprom reset
 *implement midi ?
 *
 */

#ifdef EXTERNAL_IDE

#include <Arduino.h>
#include <EEPROM.h>

int main(void) {

  init();
  setup();

  while(true) {
    loop();
  }
}

#endif

#include <portManipulations.h>
#include "osc_noiseHW.h"



extern osc_noiseHW hardware;
// hardware.getBastlCycles();
uint16_t parameter[8][2];
uint32_t channelTime[8];
/*
 * freq 100 hz
 * perioda je 100 tiků za 1000 ms
 * tzn perioda je 1000 / 100
 *
 * freq 100
 *
 * perioda je 100 tiků za 1000 ms
 *
 *
 * 1ms = 5 tiků
 *
 */
void renderOsc(uint8_t _ch){
	uint16_t period=0;
	uint16_t freq=parameter[_ch][0];

	period=(float)(51230/freq); // 5123 is measured interupt cycle

	uint16_t duty=period/2;

		//period=(257-parameter[_ch][0])<<3; //naladit
	//	duty=int(((float)period/2048)*float(parameter[_ch][1]<<3)); // zmenit
		if(duty>=period) duty=period-1;
		if(duty==0) duty=1;

	if((hardware.getElapsedBastlCycles()-channelTime[_ch]) >= period){
		channelTime[_ch]=hardware.getElapsedBastlCycles();
		hardware.setTrigger(_ch,osc_noiseHW::ON,duty);
		//setAndRecordTrigger(_ch,littleNerdHW::ON,duty);
	}

}
void buttonCall(uint8_t v){

}
void clockCall(uint8_t v){

}



void setup() {
	Serial.begin(38400);
	//	Serial.println("start");

	hardware.init(&buttonCall,&clockCall);
	for(int i=0;i<8;i++) parameter[i][0]=200*(i+1);



}



void loop() {
	for(int i=0;i<8;i++){
		renderOsc(i);
	}
	parameter[0][0]=hardware.getKnobValue(0)<<6;

}




