#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioSynthWaveform       waveform1;      //xy=497,384
AudioOutputAnalog        dac1;           //xy=723,394
AudioConnection          patchCord1(waveform1, dac1);
// GUItool: end automatically generated code

// GUItool: end automatically generated code

// GUItool: end automatically generated code

void setup(){
  AudioMemory(12);
 dac1.analogReference(EXTERNAL);
  //audioShield.enable();
//	audioShield.volume(1);

//	sine1.pulseWidth(1);
	waveform1.begin(1, 100, WAVEFORM_SAWTOOTH);
      //sine1.frequency(200);
}
void loop(){
  
}
