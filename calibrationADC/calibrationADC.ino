

#ifdef EXTERNAL_IDE
#include <Arduino.h>
#include <pgHW.h>
#include <SPI.h>
#include <mcpDac.h>
#include <calibrationADC_HW.h>
int main(void) {

  init();

  setup();

  while(true) {
    loop();
  }
}

#endif

#include <EEPROM.h>
#include <simpleSerialDecoder.h>
simpleSerialDecoder com;
 uint8_t thePoint=255;

#define PAGE 0
#define FN 2

#define UP 1
#define DOWN 3

#define BUTTON_A 4
#define BUTTON_B 5

#define _WAIT 2
#define _POINT 0
#define _REFUSE_POINT 3
#define _MINUS 1
#define _PLUS 2


pgHW hw;
calibrationADC_HW adc;
//MIDInoteBuffer buffer;

uint16_t tuneInputTable[60]={0,17,34,51,68,85,102,119,136,153,170,188,
						205,222,239,256,273,290,307,324,341,358,375,392,
						409,426,443,460,477,494,512, 529,546,563,580,597,
						614,631,648,665,682,699,716,733,750,767,784,801,
						818,835,853,870,887,904,921,938,955,972,989,1006};


#define TUNE_POINTS 11


uint16_t tuneTable[12]={ 0,410, 819,1229, 1638,2048, 2458,2867, 3277,3686, 4095,4095};
uint16_t tuneOutTable[12]={0,410, 819,1229, 1638,2048, 2458,2867, 3277,3686, 4095,4095};

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
#define OUT_OFFSET 111
void loadOutTable(){
	for(int i=0;i<11;i++){
		tuneOutTable[i]=word(EEPROM.read(OUT_OFFSET+(i*2)),EEPROM.read(OUT_OFFSET+1+(i*2)));
	}
}
void saveOutTable(){
	for(int i=0;i<11;i++){
		EEPROM.write(OUT_OFFSET+(i*2),highByte(tuneOutTable[i]));
		EEPROM.write(OUT_OFFSET+1+(i*2),lowByte(tuneOutTable[i]));
	}
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
bool acceptCalibration=false;
void channelInterpolateCall(uint8_t channel, uint8_t number){
	if(channel==_POINT) thePoint=number, acceptCalibration=true;
	if(channel==_REFUSE_POINT) thePoint=number,acceptCalibration=false;
}
void setup(){

//	hw.initialize();

	com.init(38400);
	//com.attachChannelCVCallback(&channelCVCall);
//	com.attachClockCallback(&clockCall);
//	com.attachChannelModeCallback(&channelModeCall);
	com.attachChannelInterpolateCallback(&channelInterpolateCall);

    loadTable();
   // saveOutTable();
    loadOutTable();
    //com.sendPairMessage();

    adc.init();


    hw.initialize();
	mcpDacInit();
	//dacInit();

}
uint8_t mode=1;

#include <fastAnalogRead.h>


void calibrateDevice(){

	for(int i=0;i<11;i++){
		hw.setColor(RED);
		uint8_t volts=i;
		hw.displayNumber(volts/2);
		if((volts%2)==1) hw.setDot(true);
		else hw.setDot(false);
		com.sendPairMessage();
		com.sendChannelInterpolate(_POINT,i);
		mcpDacSend(tuneOutTable[i]);
		thePoint=255;
		acceptCalibration=false;
		while(thePoint!=i){
			com.update();
		}
	//	if(acceptCalibration){
		//while(com.shoudIcalibrateThisVoltate())

		delay(_WAIT);
		uint16_t input=adc.readADC();
		if(input==tuneTable[volts]) hw.setColor(GREEN); //done
		else {
			while((input>tuneTable[volts])){
				com.sendPairMessage();
				com.sendChannelInterpolate(_MINUS,5);

				tuneOutTable[i]-=5;
				mcpDacSend(tuneOutTable[i]);

				delay(_WAIT);
				input=adc.readADC();
				hw.update();
				if(hw.buttonState(UP)) loop();
			}
			while(input<tuneTable[volts]){
				com.sendPairMessage();
				com.sendChannelInterpolate(_PLUS,5);

				tuneOutTable[i]+=5;
				mcpDacSend(tuneOutTable[i]);

				delay(_WAIT);
				input=adc.readADC();
				hw.update();
				if(hw.buttonState(UP)) loop();
			}
			while((input>tuneTable[volts])){
				com.sendPairMessage();
				com.sendChannelInterpolate(_MINUS,1);
				tuneOutTable[i]-=1;
				mcpDacSend(tuneOutTable[i]);

				delay(_WAIT);
				input=adc.readADC();
				hw.update();
				if(hw.buttonState(UP)) loop();
			}
			while(input<tuneTable[volts]){
				com.sendPairMessage();
				com.sendChannelInterpolate(_PLUS,1);

				tuneOutTable[i]+=1;
				mcpDacSend(tuneOutTable[i]);

				delay(_WAIT);
				input=adc.readADC();
				hw.update();
				if(hw.buttonState(UP)) loop();
			}
			uint16_t higher=input;
			com.sendPairMessage();
			com.sendChannelInterpolate(_MINUS,1);
			tuneOutTable[i]-=1;
			mcpDacSend(tuneOutTable[i]);

			delay(_WAIT);
			input=adc.readADC();
			if(abs(higher-tuneTable[volts]) < abs(tuneTable[volts]-input)){
				com.sendPairMessage(),com.sendChannelInterpolate(_PLUS,1);
				tuneOutTable[i]+=1;
				mcpDacSend(tuneOutTable[i]);
			}
			saveOutTable();
			//loadOutTable();
			hw.setColor(GREEN);
			hw.update();
			delay(10);
		}
	}
}
//#define DEBUG

void loop()
{
#ifndef DEBUG
	hw.update();
	uint8_t volts=map(hw.knobValue(0),0,1024,0,11);
	hw.displayNumber(volts/2);
	if((volts%2)==1) hw.setDot(true);
	else hw.setDot(false);
	mcpDacSend(tuneOutTable[volts]);
	if(hw.buttonState(BUTTON_A)&&hw.justPressed(BUTTON_B)){ //average?
		hw.setColor(RED);
		hw.update();
		tuneTable[volts]=adc.readADC();
		saveTable();
	}
	else{

		if(hw.buttonState(UP)){
			uint16_t input=adc.readADC();
			for(int i=0;i<11;i++){
				if(input>=tuneTable[i]){
					if(input==tuneTable[i]){
						hw.displayNumber((i)/2);
						if(((i)%2)==1) hw.setDot(true);
						else hw.setDot(false);
						hw.setColor(GREEN);
					}
					else if ( input > (tuneTable[i] + ( (tuneTable[i+1]-tuneTable[i])/2) ) ){
					//(abs(tuneTable[i]-input)>abs(tuneTable[i+1]-input)){

						hw.displayNumber((i+1)/2);
						if(((i+1)%2)==1) hw.setDot(true);
						else hw.setDot(false);
						hw.setColor(BLUE);
					}
					else{
						hw.displayNumber((i)/2);
						if(((i)%2)==1) hw.setDot(true);
						else hw.setDot(false);
						hw.setColor(RED);
					}
					i=100;
				}

			}
		}
		else{
			hw.setColor(BLACK);
			if(hw.justPressed(PAGE)) calibrateDevice();
		}
	}

#endif

#ifdef DEBUG

	hw.update();
	Serial.println(adc.readADC());
	delay(100);

	/*
	uint16_t result=0;
	uint8_t inByte=0;
	digitalWrite(10, LOW);
	result = SPI.transfer(0x00);
	result = result << 8;
	inByte = SPI.transfer(0x00);
	result = result | inByte;
	digitalWrite(10, HIGH);
	result = result >> 1;
	result = result & 0b0000111111111111;
	Serial.println(result);
	delay(100);
	*/


#endif



}
//uint16_t tuneTable[10];



