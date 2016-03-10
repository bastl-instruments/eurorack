

/**
 * 
 * 
 * 
 * grandPa 1.1
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
char firstLetter='P';
//const uint8_t firstLetterAddress=245;
uint16_t MID_SET;
uint16_t LOW_SET;
uint16_t HI_SET;
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

bool ignoreCalibration=false;

long seekTo;
unsigned char crush;
unsigned char volume;
unsigned char currentPreset=0;
unsigned char currentBank=0;
#define E_BANK 1001
#define E_PRESET 1002
int clockCounter=1;
unsigned char cvAssign=255;
unsigned char sound, activeSound;
PROGMEM prog_uint16_t noteSampleRateTable[65]={
  1386,
  1465,
  1552,1641,1750,1840,1955,2073,2196,2330,2462,2616,
  /*0-C*/
  2772,2929,3103,3281,3500,3679,3910,4146,4392,4660,4924,5231, 5528,5863,6221,6579,6960,7355,7784,8278,8786,9333,9847,10420,11023,/*11*/ 11662,12402,13119,13898,14706,15606,16591,17550,18555,19677,20857, /*0*/22050,23420,24807,26197,27815,29480,32982,35100,37110,39354,41714,44100,/*48-C*/44100,44100,44100,44100,44100};
uint8_t expanderValue[8]={
  0,0,0,0,0,0,0,0};
uint16_t cvInputCalibrate[12]={
  0, 100, 202, 304, 406, 508, 610, 712, 815, 917, 1012,1023} 
;
// 101, 204, 306, 409, 511, 616, 719, 823, 924
//0, 100, 202, 304, 406, 508, 610, 712, 815, 917, 1012, 
/*
void channelModeCall(uint8_t channel, uint8_t value){
 	//
 	//if(channel==1)
 	wait=false;
 	hw.displayNumber(value);
 	hw.updateDisplay();
 //	com.sendPairMessage();
 //	com.sendChannelMode(channel, value);
 }
 */
void channelTriggerCall(uint8_t channel, uint8_t number) {
  ignoreCalibration=true;
  clockCounter++;
}
void channelModeCall(uint8_t channel, uint8_t number) {
  // if(channel==1) {
  if(!ignoreCalibration){
    if(number>=10) number=10;
    uint32_t avrg=0;
    hw.setColor(BLUE);
    for(uint8_t i=0;i<10;i++){
      //      hw.displayChar('c');
      hw.displayNumber(number/2);
      hw.updateKnobs();
      hw.updateButtons();   
      hw.updateDisplay();
      avrg+=hw.getCvValue();
      delay(10);
    }

    cvInputCalibrate[number]=(avrg/10);
    // if(number==10) cvInputCalibrate[number]=1022,hw.setColor(WHITE);
    // else hw.setColor(GREEN);
    hw.setColor(GREEN);
    //}
    EEPROM.write(100+(2*number), lowByte(cvInputCalibrate[number]));
    EEPROM.write(100+(2*number)+1, highByte(cvInputCalibrate[number]));

    hw.updateDisplay();
    delay(100);
    // pinMode(1,OUTPUT);
    //  com.sendChannelCV(1,cvInputCalibrate[number]>>2);
    //  com.init(38400);
    //com.sendPairMessage();
    //com.sendChannelMode(1, cvInputCalibrate[number]>>2);
  }
}
void fillEeprom(){
  //  int number;
  for(uint8_t i=0;i<11;i++){
    //  number=(1023/16)*i;
    EEPROM.write(100+(2*i), lowByte(cvInputCalibrate[i]));
    EEPROM.write(100+(2*i)+1, highByte(cvInputCalibrate[i]));
  }

}
uint8_t mapSample(uint16_t value){
  uint8_t numberOfPoints=11;
  int inMin=0, inMax=1024;
  uint8_t _note=0;
  for(uint8_t i=0;i<numberOfPoints-1;i++){
    if(value >= cvInputCalibrate[i] && value < cvInputCalibrate[i+1]) {
      inMax=cvInputCalibrate[i+1];
      inMin=cvInputCalibrate[i];
      inMax+=100;
      inMin+=100;
      value+=100;
      uint16_t sixth=(inMax-inMin) / 6;
      //value+=sixth/2;
      inMax-=sixth/2;
     //if(inMin>sixth/2) 
     inMin-=sixth/2;
     //else inMin=0;
      for(uint8_t j=0;j<6;j++){
        if(value >= (inMin+(sixth*j)) && value < (inMin+(sixth*(j+1)))) {
           _note=(i*6)+j;
           i=100;
           j=100;
        }
      }

     if(i!=100)  _note=(i*6)+6, i=100;
     // i=numberOfPoints+10;
    }
  }
  return _note;
}
uint32_t mapVOct(uint16_t value){
  // Serial.print(value);
  // Serial.print(", ");
  if(value>=cvInputCalibrate[9]) value=cvInputCalibrate[9];
  // if(value>=1023) value=1023;
  uint8_t numberOfPoints=11;
  uint32_t inMin=0, inMax=cvInputCalibrate[10], outMin=1465, outMax=30000;
  for(uint8_t i=0;i<numberOfPoints-1;i++){
    if(value > cvInputCalibrate[i] && value <= cvInputCalibrate[i+1]) {
      if(abs(value-cvInputCalibrate[i])<3) return pgm_read_word_near(noteSampleRateTable+(i*6));
      if(abs(value-cvInputCalibrate[i+1])<3) return pgm_read_word_near(noteSampleRateTable+((1+i)*6));

      inMax=cvInputCalibrate[i+1];
      inMin=cvInputCalibrate[i];
      uint16_t sixth=(inMax-inMin) / 6;
      uint8_t _note=0;
      for(uint8_t j=0;j<6;j++){
        if(value > (inMin+(sixth*j)) && value <= (inMin+(sixth*(j+1))))  _note=j, j=6;
      }
      inMax=inMin+(sixth*(_note+1));
      inMin=inMax-sixth;
      outMax=pgm_read_word_near(noteSampleRateTable+((i*6)+_note+1));//tableMap[numberOfPoints+i+1];
      outMin=pgm_read_word_near(noteSampleRateTable+(i*6)+_note+0);

      i=numberOfPoints+10;
    }
  }
  uint32_t out=map(value,inMin,inMax,outMin,outMax);
  //  Serial.println(out);
  if(out>=31000) out=31000;
  if(out<=1386) out=1386;
  return out;
}
#define GRAIN_MAP_POINTS 8
PROGMEM prog_uint16_t granSizeMap[16]={
  0,5,10,80,127,160,220,255,  0,25,25,100,400,1000,2000,4000};
//const uint16_t granSizeMap[2]={0,30};//,80,127,160,190,255,  0,1,1,100,500,1000,2000};
#define SHIFT_SPEED_POINTS 10
PROGMEM prog_uint16_t shiftSpeedMap[20]={
  0,30,60,90,115,135,160,190,220,255,  0,5000,12000,15000,16000,16000,17000,20000,27000,32000};

uint32_t curveMap(uint16_t value, uint8_t numberOfPoints, prog_uint16_t * tableMap){
  if(value>255) value=255;
  uint8_t inMin=0, inMax=255;
  uint32_t outMin=0, outMax=255;
  for(uint8_t i=0;i<numberOfPoints-1;i++){
    if(value >= pgm_read_word_near(tableMap+i) && value <= pgm_read_word_near(tableMap+i+1)) {
      inMax=pgm_read_word_near(tableMap+i+1);
      inMin=pgm_read_word_near(tableMap+i);
      outMax=pgm_read_word_near(tableMap+numberOfPoints+i+1);
      outMin=pgm_read_word_near(tableMap+numberOfPoints+i);
      i=numberOfPoints+10;
    }
  }
  return map(value,inMin,inMax,outMin,outMax);
}

void clockCall(uint8_t _number){
}
void setup(void) {
  /*
   for(int i=0;i<11;i++) cvInputCalibrate[i]=word(EEPROM.read(100+(2*i)+1),EEPROM.read(100+(2*i)));
   Serial.begin(38400);
   for(int i=0;i<11;i++) Serial.print(cvInputCalibrate[i]), Serial.print(", "); // comment out here
   */
  hw.initialize();

  initSdCardAndReport();
  // fillEeprom();
  //
  // if(!EEPROM.read(1000)) ;//wave.setSampleRate(22050), wave.resume();
  // else EEPROM.write(1000,0),currentPreset=EEPROM.read(E_PRESET);//,currentBank=EEPROM.read(E_BANK);

  if(EEPROM.read(43)!=43 || EEPROM.read(42)!=42 || EEPROM.read(41)!=109){
    EEPROM.write(43,43);
    EEPROM.write(42,42);
    EEPROM.write(41,109);
    fillEeprom();
  }

  currentPreset=EEPROM.read(E_PRESET);

  for(uint8_t i=0;i<11;i++) cvInputCalibrate[i]=word(EEPROM.read(100+(2*i)+1),EEPROM.read(100+(2*i)));

  MID_SET=cvInputCalibrate[8];
  LOW_SET=MID_SET-30;
  HI_SET=MID_SET+30;
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
  com.attachClockCallback(&clockCall);
  com.attachChannelModeCallback(&channelModeCall);
  com.attachChannelCVCallback(&channelCVCall);
  com.attachChannelTriggerCallback(&channelTriggerCall);

  hw.initialize();
  //  Serial.begin(38400);
  indexAll();
//expanderValue[7]=0;
  /*
  for(int i=0;i<11;i++){
   Serial.print(cvInputCalibrate[i]);
   Serial.print(", ");
   }
   */
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
  //  com.sendPairMessage();
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
 com.update(); 
  hw.updateDisplay();
  //com.update(); 
  hw.updateButtons(); 
  // com.update(); 
  UI();
   //com.update(); 
  updateSound();
 //com.update(); 
  //for(int i=0;i<2;i++) Serial.print(hw.knobValue(i));
  //Serial.println();

  // pinMode(1,INPUT_PULLUP);

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
char _error='e';
uint32_t erTime;
void errorLoop(){
  while(1){
    if(millis()-erTime>500){
      erTime=millis();
      //   if(er<3) er++;
      // else er=0;
      hw.displayChar(_error); 
    }
    hw.updateButtons();   
    hw.updateDisplay();
    for(uint8_t i=0;i<6;i++){
      if(hw.buttonState(i)) chacha();
      return;
    }

  }
}










