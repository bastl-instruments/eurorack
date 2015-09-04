/*
 * A simple hardware test which receives audio on the A2 analog pin
 * and sends it to the PWM (pin 3) output and DAC (A14 pin) output.
 *
 * This example code is in the public domain.
 */

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <ADC.h>


ADC *adc = new ADC();  // should come before audio library

const int readPin = A2; // ADC0

// GUItool: begin automatically generated code
AudioInputAnalog         adc1(A6);           //xy=161,80
AudioOutputAnalog        dac1;          //xy=331,125
AudioConnection          patchCord1(adc1, dac1);
// GUItool: end automatically generated code



void setup() {
  // Audio connections require memory to work.  For more
  // detailed information, see the MemoryAndCpuUsage example
  Serial.begin(9600);
  AudioMemory(12);
  pinMode(readPin, INPUT); 
  
  adc->setAveraging(1, ADC_1); // set number of averages
  adc->setResolution(11, ADC_1); // set bits of resolution
  adc->setConversionSpeed(ADC_VERY_LOW_SPEED, ADC_1); // change the conversion speed
  adc->setSamplingSpeed(ADC_VERY_LOW_SPEED, ADC_1); // change the sampling speed
}

void loop() {
  // Do nothing here.  The Audio flows automatically
  int a = adc->analogRead(readPin, ADC_1);
  Serial.println(a*1.0/adc->getMaxValue(ADC_1), DEC);
  
  delay(200);
  // When AudioInputAnalog is running, analogRead() must NOT be used.
}


