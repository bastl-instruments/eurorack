// op qr st   uv wx yz
unsigned char incomingByte,note;
boolean ignore, comandOff,slave=false,cc; //,midiNoteOn  noteOn,
unsigned char state=0;
int ll;

int clockCounter=1;
#define MIDI_CHANNEL 1023
#define POLYPHONY NUMBER_OF_VOICES
unsigned char notesInBuffer=ZERO;
boolean thereIsNoteToPlay;
unsigned char sound, activeSound;
unsigned char inputChannel;
#define BUFFER_SIZE 16
unsigned char midiBuffer[BUFFER_SIZE];
#define EMPTY 255
unsigned char midiVelocity;
unsigned char fromBuffer;
unsigned char instantLoop;
boolean sustain=false;
boolean legato;
unsigned char sideChannel=0;
unsigned char sideNote=1;
unsigned char sideDecay;
//int pitchBendNow;   
int sampleRateNow;
unsigned char setting;
int attackInterval, releaseInterval;
//long legatoPosition;
PROGMEM prog_uint16_t noteSampleRateTable[49]={/*0-C*/
  2772,2929,3103,3281,3500,3679,3910,4146,4392,4660,4924,5231,5528,5863,6221,6579,6960,7355,7784,8278,8786,9333,9847,10420,11023,/*11*/ 11662,12402,13119,13898,14706,15606,16491,17550,18555,19677,20857, /*0*/22050,23420,24807,26197,27815,29480,29480,29480,29480,29480,29480,29480,/*48-C*/29480};


void shiftBufferLeft(unsigned char from){
  for(int i=from;i<notesInBuffer;i++){
    midiBuffer[i]=midiBuffer[i+1]; 
  }
}

void shiftBufferRight(){
  for(int i=notesInBuffer;i>ZERO;i--){
    midiBuffer[i]=midiBuffer[i-1]; 
  }
}

boolean isThereNoteToPlay(){
  return thereIsNoteToPlay;

}
unsigned char noteToPlay(){
  thereIsNoteToPlay=false;
  return midiBuffer[fromBuffer];
}


void putNoteIn(unsigned char note){
  if(note<2) hw.freezeAllKnobs();
  if(notesInBuffer==BUFFER_SIZE-1) removeNote(midiBuffer[BUFFER_SIZE-1]);
  removeNote(note); // check if the note is already in the buffer if yes remove it
  if(notesInBuffer<BUFFER_SIZE){ //put the note to the first position
    if(notesInBuffer>ZERO){ 
      shiftBufferRight();
    }
    midiBuffer[ZERO]=note; // put the last note to the first place 
    notesInBuffer++;
    thereIsNoteToPlay=true;
    fromBuffer=ZERO;
  }


  if(thereIsNoteToPlay) {
   // if(legato && note>=23 && note<66 && notesInBuffer>1) sound=note,sampleRateNow=(pgm_read_word_near(noteSampleRateTable+sound-23)),wave.setSampleRate(sampleRateNow);
   // else 
   playSound(midiBuffer[ZERO]);
  }
  //  hw.freezAllKnobs();
}

void clearBuffer(){
  for(int i=ZERO;i<BUFFER_SIZE;i++) midiBuffer[i]=EMPTY;
  notesInBuffer=0;
}
boolean removeNote(unsigned char note){
  if(notesInBuffer>ZERO){ 
    unsigned char takenOut;
    boolean takeOut=ZERO;

    for(int i=ZERO;i<notesInBuffer;i++){
      if(midiBuffer[i]==note) takeOut=true, takenOut=i;
    } 

    if(takeOut){
      shiftBufferLeft(takenOut);
      notesInBuffer--;
      for(int i=notesInBuffer;i<BUFFER_SIZE;i++) midiBuffer[i]=EMPTY;
      return true;
    }
    else return false;

  }
}
unsigned char putNoteOut(unsigned char note){
  
  if(note<2) hw.freezeAllKnobs();

  if(removeNote(note)){

    if(notesInBuffer>0){
      if(midiBuffer[ZERO]!=sound) {
       playSound(midiBuffer[ZERO]),instantLoop=0;
      } //legatoPosition=wave.getCurPosition(),
    }
    else if(!sustain) stopEnvelope(),instantLoop=0;
    return midiBuffer[ZERO];

  }




}



void initMidi(){
  clearBuffer();
  //Serial.begin(9600);
  readMidiChannel();
  //Serial.end();
  Serial.begin(MIDI_BAUD);

}
#define SIDE_CHANNEL 1022
#define SIDE_NOTE 1021
#define SIDE_DECAY 1020
unsigned char TOLERANCE=3;
//#define TOLERANCE 3
unsigned char controler, CCvalue;
#define TOL_EE 1019
void readMidiChannel(){
/*
  sideChannel=EEPROM.read(SIDE_CHANNEL);

  sideNote=EEPROM.read(SIDE_NOTE);
  //sideDecay=EEPROM.read(SIDE_DECAY);
  inputChannel=EEPROM.read(MIDI_CHANNEL);
  if(inputChannel>16) EEPROM.write(MIDI_CHANNEL,0), inputChannel=0;//,EEPROM.write(TOL_EE,0);
  

  //TOLERANCE=EEPROM.read(TOL_EE);
  //  if(TOLERANCE>10) EEPROM.write(TOL_EE,0);
 
   hw.update();
  if(hw.buttonState(PAGE)) EEPROM.write(TOL_EE,0);
  if(hw.buttonState(REC)) EEPROM.write(TOL_EE,4);
  if(hw.buttonState(HOLD)) EEPROM.write(TOL_EE,9);
  TOLERANCE = 3 +  EEPROM.read(TOL_EE);
  if(TOLERANCE!=12 && TOLERANCE!=7 && TOLERANCE!=3) TOLERANCE=3;
  //Serial.print(TOLERANCE);
 
  
  
  
  for(int i=0;i<6;i++){
    if(hw.buttonState(bigButton[i])){
      
      if(hw.buttonState(UP)) sideChannel=i+6*hw.buttonState(FN), EEPROM.write(SIDE_CHANNEL,sideChannel);//, showValue(sideChannel+1);//, hw.displayChar('C',1); 
      else if(hw.buttonState(DOWN)) sideNote=i+60*hw.buttonState(FN), EEPROM.write(SIDE_NOTE,sideNote);//, showValue(sideNote), hw.displayChar('N',1); 
      else inputChannel=i+6*hw.buttonState(FN),EEPROM.write(MIDI_CHANNEL,inputChannel);
    }
  }
  showValue(inputChannel+1);
  hw.displayChar('C',0);
  hw.displayChar('H',1); 
  //hw.setDot(3,true);
  hw.setDot(VERSION,true);


  if(wave.isPlaying()){
    while(!wave.isPaused()) hw.update();
  }
  stopSound();
  //noDots();
  */
}

long lastClockPosition, clockLength;
//unsigned char pByte1,pByte2;
//boolean pb;
boolean side;
int bytesAvailable;
void readMidi(){


  //channel=map(analogRead(4),0,1024,0,16);
  while(Serial.available() > 0){
    bytesAvailable=Serial.available();
    if (bytesAvailable <= 0) return;
    if(bytesAvailable>=64) Serial.flush(); // If the buffer is full -> Don't Panic! Call the Vogons to destroy it.
    else handleByte(Serial.read());
  }
}
boolean noteOnStatus=false, noteOffStatus=false, ccStatus=false, firstByte=true;

unsigned char number, value;
unsigned char channel=0;

void handleByte(unsigned char incomingByte){

  if(handleRealTime(incomingByte));
  else{
    if(incomingByte>127) recognizeStatus(incomingByte);

    else if(firstByte){
      firstByte=false;
      if(ccStatus) number=incomingByte;
      else if(noteOnStatus || noteOffStatus) number=incomingByte;
      else firstByte=true;


    }
    else{
      value=incomingByte;
      firstByte=true; 
      if(ccStatus) handleCC(number,value, channel);
      else if(noteOnStatus) handleNote(number,value, channel);
      else if(noteOffStatus) handleNote(number,0, channel);

      //firstByte=true; 
    }

  } 

}
#define NOTE_ON_BYTE 0x09
#define NOTE_OFF_BYTE 0x08
#define CC_BYTE 0x0B
void recognizeStatus(unsigned char incomingByte){
  channel=0;
  for(int i=0;i<4;i++) bitWrite(channel,i,bitRead(incomingByte,i));

  incomingByte=incomingByte>>4;
  if(incomingByte==NOTE_ON_BYTE) noteOnStatus=true, noteOffStatus=false, ccStatus=false;
  else if(incomingByte==NOTE_OFF_BYTE) noteOnStatus=false, noteOffStatus=true, ccStatus=false;
  else if(incomingByte==CC_BYTE) noteOnStatus=false, noteOffStatus=false, ccStatus=true;
  else noteOnStatus=false, noteOffStatus=false, ccStatus=false;
  firstByte=true;
  //channel=


}
void handleNote(unsigned char _number,unsigned char _value,unsigned char _channel){
  _number--;
  if(_channel==inputChannel){
    if(_value==0) putNoteOut(_number);
    else putNoteIn(_number), midiVelocity=_value;
  } 

  if(_channel==sideChannel && _number==sideNote && _value!=0) startEnvelope(midiVelocity,attackInterval);
}

void handleCC(unsigned char _number,unsigned char _value,unsigned char _channel){
  if(_channel==inputChannel){
    proceedCC(_number,_value);
  }
}


boolean handleRealTime(unsigned char _incomingByte){
  if((_incomingByte>=0xF8) && (_incomingByte<=0xFF)){
    if(_incomingByte==0xF8){ //clock
      clockCounter++;
      slave=true;
    }
    else if(_incomingByte==0xFA ){ //start
      clockCounter=1;
      slave=true;
      // setSetting(activeSound);
    }
    else if(_incomingByte==0xFC){ //stop
      clockCounter=1;
      slave=true;
    } 
    return true;
  }
  else return false;
}


#define SUSTAIN_PEDAL_BYTE 64
#define PRESET_BY_CC_BYTE 0
#define BANK_BY_CC_BYTE 1
#define RANDOMIZE_BYTE 127

#define CONTROL_CHANGE_BITS 7
#define CONTROL_CHANGE_OFFSET 102
#define CONTROL_CHANGE_OFFSET_2 110

void proceedCC(unsigned char _number,unsigned char _value){

  if(_number==1) setVar(activeSound,CRUSH,_value), hw.unfreezeAllKnobs(),renderTweaking(0),hw.freezeAllKnobs(); //modwheel
  //if(_number==123) clearBuffer(),sound=0, stopEnvelope(),instantLoop=0; // all notes off
  else if(_number==SUSTAIN_PEDAL_BYTE){ 
   // sustain=_value>>6;
    //if(!sustain && notesInBuffer==0) stopEnvelope(),instantLoop=0;  
  }

  else if(_number==PRESET_BY_CC_BYTE) loadPreset(currentBank,myMap(_value,128,NUMBER_OF_PRESETS));
  //if(_number==BANK_BY_CC_BYTE) loadPreset(myMap(_value,128,NUMBER_OF_BANKS),currentPreset);

  // else if(_number==RANDOMIZE_BYTE) randomize(activeSound);

  else if(_number>=CONTROL_CHANGE_OFFSET && _number<CONTROL_CHANGE_OFFSET_2){
    _number=_number-CONTROL_CHANGE_OFFSET;
    setVar(activeSound,_number,scale(_value,CONTROL_CHANGE_BITS,variableDepth[_number]));  
    hw.unfreezeAllKnobs();
    renderTweaking(_number/VARIABLES_PER_PAGE);
    hw.freezeAllKnobs();
  }
  //if(_number==123) clearBuffer();

}


/*
void proceedPB(unsigned char _byte1,unsigned char _byte2){
 int pitchBendNow=word(_byte1,_byte2)-8192; 
 wave.setSampleRate(sampleRateNow+pitchBendNow);
 }
 */





