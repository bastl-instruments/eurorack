#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <ADC.h>


ADC *adc = new ADC();  // should come before audio library

const int delayPin = A3; // ADC1
const int feedbackPin = A2; // ADC1

// GUItool: begin automatically generated code
AudioInputAnalog         adc1(A6);           //xy=55,336.2500057220459
AudioEffectDelay         delay1;         //xy=260.00000381469727,462.50000762939453
AudioMixer4              feedbackMixer;
AudioMixer4              mixer1;         //xy=508.75000762939453,247.50000381469727
AudioMixer4              mixer2;         //xy=508.75000762939453,247.50000381469727
AudioMixer4              mixer3;         //xy=508.75000762939453,247.50000381469727
AudioOutputAnalog        dac1;           //xy=800.0000076293945,292.5000057220459
AudioConnection          patchCord1(adc1, 0, mixer1, 0);
AudioConnection          patchCord2(adc1, 0, feedbackMixer, 0);
AudioConnection          patchCordf2(feedbackMixer, delay1);
AudioConnection          patchCord3(delay1, 0, mixer1, 1);
AudioConnection          patchCordf3(delay1, 0, feedbackMixer, 1);
AudioConnection          patchCord4(delay1, 1, mixer1, 2);
AudioConnection          patchCord5(delay1, 2, mixer1, 3);
AudioConnection          patchCords3(delay1, 3, mixer2, 0);
AudioConnection          patchCords4(delay1, 4, mixer2, 1);
AudioConnection          patchCords5(delay1, 5, mixer2, 2);
AudioConnection          patchCordc5(delay1, 6, mixer2, 3);
AudioConnection          patchCord63a(mixer1,0, mixer3,0);
AudioConnection          patchCord63(mixer2,0, mixer3,1);
AudioConnection          patchCord6(mixer3, dac1);
// GUItool: end automatically generated code

float delayR = 1.0;
float feedback = 1.0;



void setup() {
  // Audio connections require memory to work.  For more
  // detailed information, see the MemoryAndCpuUsage example
  AudioMemory(120);
  //chorus.begin(l_delayline,CHORUS_DELAY_LENGTH,n_chorus);
  //chorus.begin(l_delayline,FLANGE_DELAY_LENGTH,s_idx,s_depth,s_freq);
  mixer1.gain(0,0.6);
  mixer1.gain(1,0.6);
  mixer1.gain(2,0.6);
  mixer1.gain(3,0.6);
  
  mixer2.gain(0,0.6);
  mixer2.gain(1,0.6);
  mixer2.gain(2,0.6);
  mixer2.gain(3,0.6);
  
  //mixer3.gain(0,0.6);
  //mixer3.gain(1,0.6);
  
  //filter1.frequency(400);
  
  delay1.delay(0, 200);
  //delay1.delay(1, 100);
  //delay1.delay(2, 50);
  //delay1.delay(3, 25);
  //delay1.delay(4, 30);
  //delay1.delay(5, 35);
  //delay1.delay(6, 40);
  
  pinMode(delayPin, INPUT);
  pinMode(feedbackPin, INPUT); 
  
  adc->setAveraging(1, ADC_1); // set number of averages
  adc->setResolution(11, ADC_1); // set bits of resolution
  adc->setConversionSpeed(ADC_VERY_LOW_SPEED, ADC_1); // change the conversion speed
  adc->setSamplingSpeed(ADC_VERY_LOW_SPEED, ADC_1); // change the sampling speed
}

void loop() {
  // Do nothing here.  The Audio flows automatically
  delayR = adc->analogRead(delayPin, ADC_1)*1.0/adc->getMaxValue(ADC_1);
  feedback = adc->analogRead(feedbackPin, ADC_1)*1.0/adc->getMaxValue(ADC_1);
  
  //AudioNoInterrupts();
  delay1.delay(0, delayR*330);
  feedbackMixer.gain(1,feedback);
  //AudioInterrupts();
  // When AudioInputAnalog is running, analogRead() must NOT be used.
}


