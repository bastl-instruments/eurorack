#include "teensyModule_HW.h"
// demonstrate pulse with slow changes in pulse width

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>

// GUItool: begin automatically generated code
AudioSynthWaveform       waveform1;      //xy=188,240
AudioOutputAnalog  dac;     // play to both I2S audio board and on-chip DAC
AudioConnection          patchCord1(waveform1, dac);
AudioControlSGTL5000     audioShield;     //xy=586,175

teensyHWR teensy;

float f=0;
float fcv=0;

float w=1;
float wcv=0;

int wave=1;

short wave_type[5] = {
  WAVEFORM_SINE,
  WAVEFORM_SQUARE,
  WAVEFORM_SAWTOOTH,
  WAVEFORM_TRIANGLE,
  WAVEFORM_PULSE
};

void setup(){
	Serial.begin(115200);
	teensy.initialize();
	teensy.setLed(0,true);
	teensy.setLed(3,true);
	//teensy.setHandleButtonChange(buttons);
	teensy.setHandleKnobChange(knobs);
	//teensy.setHandleCVChange(cvs);
	
	AudioMemory(8);
	audioShield.enable();
	audioShield.volume(0.7);

	waveform1.pulseWidth(1);
	waveform1.begin(0.7, 80, WAVEFORM_PULSE);
}

void loop(){
	teensy.update();
	
	f=map(teensy.knobValue(0),0,1024,20,400);
	fcv=teensy.CVValue(0);
	fcv=fcv-555;
	fcv= (fcv/555.0)*400.0;
	w=teensy.knobValue(1)/1024.0;
	wcv=(teensy.CVValue(1)-555)/555.0;
	//waveform1.begin(wave_type[wave]);
	waveform1.pulseWidth(w+wcv);
	waveform1.frequency(f+fcv);
	
	
	
	//delay(1);
}

void buttons(int b, boolean s){
	Serial.print("Callback button ");
	if(s)
		Serial.print("pressed ");
	else
		Serial.print("unpressed ");
	Serial.println(b);
}

void knobs(uint8_t k, uint16_t v){
	Serial.print("Callback knob ");
	Serial.print(k);
	Serial.print(" = ");
	Serial.println(v);
	
	if(k==0){
		f=map(v,0,1024,20,400);
		waveform1.frequency(f+fcv);
	}
}

void cvs(int c, int v){
	//Serial.print("Callback CV ");
	//Serial.print(c);
	//Serial.print(" ");
	//printGraph(v);
	
	if(c==0){
		if(v>650)
			teensy.setLed(4,true);
		else
			teensy.setLed(4,false);
		
	}
	
	if(c==1){
		v=v-555;
		fcv= (v/555.0)*400.0;
		waveform1.frequency(f+fcv);
	}
}

void printGraph(float val){
    Serial.print(" Value: ");
    Serial.print(val);
    Serial.print("\t");
    for(int i=0;i<map(val,0,1024,0,100);i++)
      Serial.print("|");
    Serial.println("");
  delay(10);
}
