#include <Audio.h>
#include <Wire.h>
#include <teensyModule_HW.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
//#include <ADC.h>
teensyHWR hw;

// GUItool: begin automatically generated code
AudioSynthWaveform       waveform1;      //xy=497,384
AudioOutputAnalog        dac1;           //xy=723,394
AudioConnection          patchCord1(waveform1, dac1);
// GUItool: end automatically generated code

// GUItool: end automatically generated code

// GUItool: end automatically generated code
bool flop;
void setup(){
  Serial.begin(11520);
  AudioMemory(12);
 dac1.analogReference(EXTERNAL);
 hw.initialize();
 hw.readCVE(true);
  //audioShield.enable();
//	audioShield.volume(1);

//	sine1.pulseWidth(1);
  waveform1.begin(1, 100, WAVEFORM_SAWTOOTH);
      //sine1.frequency(200);
}
void loop(){
  
  hw.update();
  
  for(int i=0;i<4;i++){
  Serial.print(hw.knobValue(i));
  Serial.print("  ");
  
  }
  for(int i=0;i<4;i++){
  Serial.print(hw.cvValue(i));
  Serial.print("  ");
  
  }
  Serial.println();
  
  for(int i=0;i<3;i++){
    if(hw.switchState()==i) hw.setLed(i,true);
    else hw.setLed(i,false);
  }
   hw.setLed(3, hw.buttonState(0));
  if(hw.buttonState(0)){
  }
 //if(hw.justPressed(0)) 
 flop=!flop;
 // hw.setLed(4,flop);
 hw.setSquareOut(flop);
 //Serial.println(hw.cvValue(4));
  
  
}
