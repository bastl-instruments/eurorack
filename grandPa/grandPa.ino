

/**
 * 
 * 
 * 
 * picoGranny 2.2
 * eurorack modular
 * by Vaclav Pelousek      http://www.pelousek.net/
 * for Bastl Instruments   http://www.bastl-instruments.com/
 * 
 * based on WaveRP library Adafruit Wave Shield - Copyright (C) 2009 by William Greiman
 * -library heavily hacked - BIG THX https://code.google.com/p/waverp/
 * 
 * needs SD Fat library too https://code.google.com/p/sdfatlib/
 *
 *
 *
 * -thanks for understanding basics thru Mozzi library http://sensorium.github.io/Mozzi/
 * -written in Arduino + using SDFat library
 *
 * 
 * TODO
 * 
 * - test CV assign and grain sync
 * - card removal pimpUp ?
 * - when synced and no clock available - end position doesnot repeat is good, but...
 * 
 * - longer zero on RATE knob ?
 * BUGS
 * when synced doesnt loop properly
 * looping sometimes doesnt work
 * 
 * 
 * 
 */

#define VERSION 1
#include <simpleSerialDecoder.h>
#include <simpleSerialProtocol.h>
#include <SdFat.h>
#include <WaveRP.h>
#include <SdFatUtil.h>
#include <ctype.h>
//#include <mozzi_analog.cpp>
#include <pgHW.h>
//#include <MIDI.h>
#include <EEPROM.h>
#include <avr/pgmspace.h>
char firstLetter='R';


///#include <envelope.h>


#define RECORD_RATE 22050 // try pot !!

#define MAX_FILE_SIZE 104857600UL  // 100 MB
//#define MAX_FILE_SIZE 1048576000UL // 1 GB
#define MIC_ANALOG_PIN 0 // Analog pin connected to mic preamp
// Voltage Reference Selections for ADC
//#define ADC_REFERENCE ADC_REF_AREF  // use voltage on AREF pin
#define ADC_REFERENCE ADC_REF_AVCC  // use 5V VCC

//#define DISPLAY_RECORD_LEVEL 1

#define MIDI_BAUD 31250
//------------------------------------------------------------------------------
// global variables
Sd2Card card;           // SD/SDHC card with support for version 2.00 features
SdVolume vol;           // FAT16 or FAT32 volume
SdFile root;            // volume's root directory
SdFile file;            // current file
WaveRP wave;            // wave file recorder/player
pgHW hw;
simpleSerialDecoder com;
//------------------------------------------------------------------------------



long seekTo;
unsigned char crush;
unsigned char volume;
unsigned char currentPreset=0;
unsigned char currentBank=0;
#define E_BANK 1001
#define E_PRESET 1002
PROGMEM prog_uint16_t noteSampleRateTable[49]={/*0-C*/
  2772,2929,3103,3281,3500,3679,3910,4146,4392,4660,4924,5231,5528,5863,6221,6579,6960,7355,7784,8278,8786,9333,9847,10420,11023,/*11*/ 11662,12402,13119,13898,14706,15606,16491,17550,18555,19677,20857, /*0*/22050,23420,24807,26197,27815,29480,29480,29480,29480,29480,29480,29480,/*48-C*/29480};

uint16_t cvInputCalibrate[15];
void channelCVCall(uint8_t channel, uint8_t number) {
  if(channel==1) {
    uint16_t avrg=0;
    for(int i=0;i<10;i++){
      hw.updateKnobs();
      hw.updateButtons();   
      hw.updateDisplay();
      avrg+=hw.getCvValue();
      delay(1);
    }
    cvInputCalibrate[number]=avrg/10;
  }
  EEPROM.write(100+(2*number), lowByte(cvInputCalibrate[number]));
  EEPROM.write(100+(2*number)+1, highByte(cvInputCalibrate[number]));
  com.sendChannelCV(1,cvInputCalibrate[number]>>2);
}
void fillEeprom(){
  int number;
  for(int i=0;i<15;i++){
    number=(1024/15)*i;
    EEPROM.write(100+(2*i), lowByte(number));
    EEPROM.write(100+(2*i)+1, highByte(number));
  }
}
uint32_t mapVOct(uint16_t value){
  uint8_t numberOfPoints=15;
  uint32_t inMin=0, inMax=255, outMin=0, outMax=255;
  for(int i=0;i<numberOfPoints-1;i++){
    if(value >= cvInputCalibrate[i] && value <= cvInputCalibrate[i+1]) {
      inMax=cvInputCalibrate[i+1];
      inMin=cvInputCalibrate[i];
      outMax=pgm_read_word_near(noteSampleRateTable+((i+1)*4));//tableMap[numberOfPoints+i+1];
      outMin=pgm_read_word_near(noteSampleRateTable+(i*4));
      i=numberOfPoints+10;
    }
  }
  uint32_t out=map(value,inMin,inMax,outMin,outMax);
  if(out>=44100) out=44099;
  return out;
}




void setup(void) {

  hw.initialize();

  initSdCardAndReport();
  fillEeprom();
  //
  if(!EEPROM.read(1000)) ;//wave.setSampleRate(22050), wave.resume();
  else EEPROM.write(1000,0),currentPreset=EEPROM.read(E_PRESET),currentBank=EEPROM.read(E_BANK);
  for(int i=0;i<15;i++) cvInputCalibrate[i]=word(EEPROM.read(100+(2*i)+1),EEPROM.read(100+(2*i)));
  // initMidi();


  //  initMem(); 
  //  clearMemmory();
  initMem(); 
  restoreAnalogRead();
  hw.updateKnobs();
  hw.updateKnobs();
  hw.updateKnobs();

  hw.freezeAllKnobs();
  // timer2setup();

  com.init(38400);

  com.attachChannelCVCallback(&channelCVCall);
  /*
  com.attachClockCallback(&clockCall);
   com.attachStepCallback(&stepCall);
   com.attachGateCallback(&gateCall);
   com.attachTriggerCallback(&triggerCall);
   com.attachStartCallback(&startCall);
   com.attachStopCallback(&stopCall);
   com.attachRestartCallback(&restartCall);
   com.attachChannelTriggerCallback(&channelTriggerCall);
   com.attachChannelGateCallback(&channelGateCall);
   
   com.attachChannelInterpolateFromCallback(&channelInterpolateFromCall);
   com.attachChannelInterpolateToCallback(&channelInterpolateToCall);
   com.attachChannelInterpolateCallback(&channeInterpolateCall);
   com.attachChannelModeCallback(&channelModeCall);
   com.attachChannelValueCallback(&channelValueCall);
   */
  com.sendPairMessage();
  //Serial.begin(9600);
  // Serial.println("start");

}
/*
void timer2setup(){
 TCCR2A =  (1<<WGM21);
 TIMSK2 |= (1 << OCIE2A);
 TCCR2B = B00000111;
 OCR2A = (F_CPU / 1024) / 300;
 TCNT2 = 0;
 }
 
 ISR(TIMER2_COMPA_vect){
 
 //if(wave.matrixAvailable) 
 hw.updateButtons();
 hw.updateDisplay();
 }
 */
void restoreAnalogRead()
{
  ADCSRA=135; // default ARDUINO B10000111
}


void software_Reset() // Restarts program from beginning but does not reset the peripherals and registers
{
  asm volatile ("  jmp 0");  
} 
uint8_t clr=0;
void loop() {
  // Serial.print(".");
  // if(!wave.isPlaying() || wave.isPaused()) clearIndexes(),stopSound(),playBegin("A0.WAV",1),wave.setSampleRate(22050), wave.pause(), wave.seek(0), wave.resume(), Serial.println("ply");
  // else if(wave.isPlaying());// Serial.print(".");
  //delay(100);
  hw.updateKnobs();
  hw.updateButtons();   
  hw.updateDisplay();
  UI();
  updateSound();
  com.update();

  /*
    if(clr<7) clr++;
   else clr=0;
   hw.setColor(RED);
   hw.setLed(6,hw.buttonState(3));
   hw.setLed(7,hw.buttonState(1));
   hw.lightNumber(hw.knobValue(1)>>5);
   for(int i=0;i<6;i++) if(hw.justPressed(i)) Serial.println(i,DEC);
   */
  /*
  readMidi();//,hw.displayText("midi");
   readMidi();
   UI();
   readMidi();
   readMidi();
   updateSound();
   readMidi();
   readMidi();
   */
}
unsigned char er;
char _error[5]="eror";
uint32_t erTime;
void errorLoop(){
  while(1){
    if(millis()-erTime>500){
      erTime=millis();
      if(er<3) er++;
      else er=0;
      hw.displayChar(_error[er]); 
    }
    hw.updateButtons();   
    hw.updateDisplay();
    for(int i=0;i<6;i++){
      if(hw.buttonState(i)) chacha();
      return;
    }

  }
}






