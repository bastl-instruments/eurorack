
#define PAGE 0
#define FN 2

#define UP 1
#define DOWN 3

#define BUTTON_A 4
#define BUTTON_B 5

#define LOOP_LENGTH_SHIFT 4 //8 for possition based
#define SHIFT_SPEED_SYNC_SHIFT 5
#define SHIFT_SPEED_SHIFT 7

boolean whileShow;
long whileTime;

#define WHILE_DURATION 1600
#define NUMBER_OFFSET 48
#define BIG_LETTER_OFFSET 65
#define SMALL_LETTER_OFFSET 97
#define NUMBER_OF_LETTERS 3

#define PAGE_KNOB_LABEL_OFFSET 4
unsigned char lastMoved;
boolean combo;
boolean shift;

boolean tuned=true;

#define TUNED_BIT 0 //1
#define LEGATO_BIT 1 //0
#define REPEAT_BIT 2 //1
#define SYNC_BIT 3 //1
#define SHIFT_DIR_BIT 4 //0
#define EXTRA_BIT 5 //0

//1101
//TUNED, LEGATO, REPEAT, SYNC and RANDOM SHIFT
uint32_t startGranule;

unsigned char cvAssign=255;

#define DEFAULT_VELOCITY 127

uint32_t longTime;
boolean longPress;
#define LONG_PERIOD 2000
#define MOVE_PERIOD 400

//unsigned char updt;
const uint8_t pageOrder[8]={
  0,2,1,3,0,2,1,3};
uint8_t pageClick;
uint8_t lastAs=0;
void UI(){
  if(activeSound!=lastAs) hw.freezeAllKnobs();
  lastAs=activeSound;
  renderCombo();
  // renderHold();
  if(sync) renderSync();
  if(!combo){
    if(hw.justReleased(PAGE)){
      hw.freezeAllKnobs();
      if(++pageClick>=4) pageClick=0;
      //if(++page>=4) page=0;


      // hw.setLastKnobValue(i,_knobValue);
    }
  }
  if(pageClick>=4) pageClick=0; //security
  renderBigButtons();

  if(pageClick==3) hw.setColor(WHITE);
  else hw.setColor(pageClick+1); 
  page=pageOrder[pageClick];
  /*
    if(page==3) hw.setColor(WHITE);
   else hw.setColor(page+1);
   */
  renderSmallButtons();
  renderTweaking(page);
  renderKnobs();
  renderDisplay();



}
bool cvState;
#define CV_THRESHOLD 127
void renderSync(){
  bool newState=false;
  if(hw.getCvValue() > CV_THRESHOLD) newState=true;
  if(!cvState && newState) clockCounter++;//,Serial.println("c");
  cvState=newState;


}
void dimLeds(){
  // for(int i=0;i<NUMBER_OF_SOUNDS;i++) hw.setLed(bigButton[i],false);
  //hw.setLed(HOLD,false);
}
void playSound(unsigned char _sound){
  sound=_sound;
  /*
  if(_sound<=83 && _sound>=23){
   loadName(activeSound);
   startPlayback(activeSound);
   loadValuesFromMemmory(_sound);
   startEnvelope(midiVelocity,attackInterval);
   }
   else 
   */
  if(_sound<2){

    activeSound=_sound;
    loadName(activeSound);
    startPlayback(activeSound);
    loadValuesFromMemmory(activeSound);
    // Serial.println(cvAssign);
    startEnvelope(DEFAULT_VELOCITY,attackInterval);



  }

}

//long sizeOfFile;

//samples per second*1000 - millisecond - how many samples?
//x= sample Rate*number of samples / 1000

boolean revMidi=false;
void setEnd(unsigned char _sound){
  endPosition=sizeOfFile;
  if(revMidi) endIndex=startIndex+1, startIndex=0; //, startPosition=0;
  else endIndex=getVar(_sound,END);
  

    if(endIndex<1000){
      if(endIndex<=startIndex) endIndex=startIndex+10;     
      endPosition=endIndex*startGranule;

    }

    else endPosition=sizeOfFile-512;
    if(reverse) seekPosition=endPosition;//, lastPosition=endPosition;

    else seekPosition=startPosition;

  

}
int pitch;

void loadValuesFromMemmory(unsigned char _sound){
  unsigned char  notePitch=255;
  revMidi=false;
  sizeOfFile=file.fileSize();
  if(!tuned && _sound>=23 && _sound <=73){

    startIndex=_sound-23, _sound=activeSound, startGranule=sizeOfFile/60;
    if(reverse);// revMidi=true;//,startIndex=0 ;

  }
  else if(_sound>=23 && _sound<66) notePitch=_sound-23,_sound=activeSound,startGranule=sizeOfFile/1024, startIndex=getVar(_sound,START);
  else _sound=activeSound,startGranule=sizeOfFile/1024, startIndex=getVar(_sound,START);
  // endIndex=getVar(_sound,END);
  // if(startIndex>endIndex) startIndex=endIndex+3;

  startPosition=startIndex*startGranule;
  setSetting(_sound);
  attackInterval=getVar(_sound,ATTACK);
  releaseInterval=getVar(_sound,RELEASE);
  if(releaseInterval==127) sustain=true;
  else sustain=false;
  wave.pause();

  if(notePitch!=255){
    sampleRateNow=(pgm_read_word_near(noteSampleRateTable+notePitch)); //+rnd number to lower probability of interference novinka
  }
  else{
    sampleRateNow=valueToSampleRate(getVar(_sound,RATE));
  }


  wave.setSampleRate(sampleRateNow);//+pitchBendNow);
  //  bit_set(PIN);
  crush=getVar(_sound,CRUSH)<<1;
  wave.setCrush(crush);
  ll=getVar(_sound,LOOP_LENGTH);
  if(sync) loopLength=1,ll=1;//pgm_read_word_near(usefulLengths+(ll>>3));

  else loopLength=ll<<LOOP_LENGTH_SHIFT;
  //if(sync) shiftSpeed=((long)getVar(_sound,SHIFT_SPEED)-128)<<SHIFT_SPEED_SYNC_SHIFT;
  //else 
  shiftSpeed=((long)getVar(_sound,SHIFT_SPEED)-128)<<SHIFT_SPEED_SHIFT;
  if(shiftSpeed<0 && ll!=0) reverse=true;
  else reverse=false;
  setEnd(_sound);
  granularTime=millis(); //novinka
  // novinka last
  wave.seek(seekPosition);
  lastPosition=seekPosition;
  wave.resume();


}
uint16_t valueToSampleRate(int _value){ // longer zero
  pitch=myMap(_value,1023,420);
  
  if(tuned){
    pitch/=10;
    return (pgm_read_word_near(noteSampleRateTable+pitch));
  }
  else{
    int pitchStep=(pgm_read_word_near(noteSampleRateTable+pitch/10+1) - pgm_read_word_near(noteSampleRateTable+pitch/10))/10;
    return pgm_read_word_near(noteSampleRateTable+pitch/10)+(pitch%10)*pitchStep;
  } 
  
  //return _value<<4;
}
void setSetting(unsigned char _sound){
  setting=getVar(_sound,SETTING);
  repeat=bitRead(setting,REPEAT_BIT);
  tuned=false;//bitRead(setting,TUNED_BIT);
  shiftDir=false;//bitRead(setting,SHIFT_DIR_BIT);
  if(cvAssign!=255) sync=false;
  else sync=bitRead(setting,SYNC_BIT);
  legato=false;//bitRead(setting,LEGATO_BIT);
  if(sync) cvAssign=255;
  else{
    cvAssign=0;
    if(bitRead(setting,EXTRA_BIT)) cvAssign=255;
    else{
      bitWrite(cvAssign,0,bitRead(setting,TUNED_BIT));
      bitWrite(cvAssign,1,bitRead(setting,SHIFT_DIR_BIT));
      bitWrite(cvAssign,2,bitRead(setting,LEGATO_BIT));
    }
  }
  if(!tuned) legato=false;

  // if(slave && sync) sync=true;
  // else sync=false;
  // 
}


void startPlayback(unsigned char _sound){

  //if(wave.isPlaying()) stopSound();
  stopSound();
  playBegin(name,_sound);//,lastPosition=wave.getCurPosition(); //novinka
  lastMoved=5;
  showSampleName();

}
uint16_t lastCv;
bool cvChanged=false;
uint16_t cvToSampleRate(uint16_t _cv){
//  _cv/10
  return _cv<<4;
}
void renderTweaking(unsigned char _page){

  if(wave.isPlaying()){
    unsigned char _sound=activeSound;
    cvChanged=false;
    uint16_t _CV=0;
    if(cvAssign!=255){

      switch(cvAssign){
      case 0:
        _CV=cvToSampleRate(hw.getCvValue());
        if(lastCv!=_CV) cvChanged=true;
        lastCv=_CV;
        if(cvChanged) sampleRateNow=valueToSampleRate(getVar(_sound,RATE))+_CV;//((long)(valueToSampleRate(getVar(_sound,RATE))*_CV))/100; //novinka testthis
        wave.setSampleRate(sampleRateNow);
        break;
      case 1:
        _CV=hw.getCvValue();
        if(lastCv!=_CV) cvChanged=true;
        lastCv=_CV;
        if(cvChanged) wave.setCrush((getVar(_sound,CRUSH)<<1)+_CV);
        break;
      case 2:
        _CV=hw.getCvValue()>>1;
        if(lastCv!=_CV) cvChanged=true;
        lastCv=_CV;
        if(cvChanged) attackInterval=getVar(_sound,ATTACK)+_CV;
        break;
      case 3:
        _CV=hw.getCvValue()>>1;
        if(lastCv!=_CV) cvChanged=true;
        lastCv=_CV;
        if(cvChanged) releaseInterval=getVar(_sound,RELEASE)+_CV;
        break;
      case 4:
        _CV=hw.getCvValue(); //??
        if(lastCv!=_CV) cvChanged=true;
        lastCv=_CV;
        if(cvChanged){
          ll=getVar(_sound,LOOP_LENGTH)+_CV;
          if(sync) loopLength=1,ll=1;//pgm_read_word_near(usefulLengths+(ll>>3));
          else loopLength=ll<<LOOP_LENGTH_SHIFT;
        }
        break;
      case 5:
        _CV=hw.getCvValue(); //??
        if(lastCv!=_CV) cvChanged=true;
        lastCv=_CV;
        if(cvChanged) {
          shiftSpeed=(((long)getVar(_sound,SHIFT_SPEED)-128)+_CV)<<SHIFT_SPEED_SHIFT;
        }
        break;
      case 6:
        _CV=hw.getCvValue()<<2;
        if(lastCv!=_CV) cvChanged=true;
        lastCv=_CV;
        if(cvChanged) {
          startIndex=getVar(_sound,START)+_CV;
          startPosition=startIndex*startGranule;
        }
        break;
      case 7:
        _CV=hw.getCvValue();
        if(lastCv!=_CV) cvChanged=true;
        lastCv=_CV;
        break;
      }
    }

    switch(_page){
    case 0:


      if(!hw.knobFreezed(0) ){
        if(cvAssign==0) sampleRateNow=valueToSampleRate(getVar(_sound,RATE))+_CV;
        else sampleRateNow=valueToSampleRate(getVar(_sound,RATE));
        if(sound<7 || !tuned) wave.setSampleRate(sampleRateNow);//+pitchBendNow);
      }
      if(!hw.knobFreezed(1) ){
        if(cvAssign==1) wave.setCrush((getVar(_sound,CRUSH)<<1)+_CV);
        else wave.setCrush((getVar(_sound,CRUSH)<<1));
      }
      break;
    case 1:
      if(!hw.knobFreezed(0) ){ 
        if(cvAssign==2) attackInterval=getVar(_sound,ATTACK)+_CV;
        else attackInterval=getVar(_sound,ATTACK);
      }
      if(!hw.knobFreezed(1)){
        if(cvAssign==2) releaseInterval=getVar(_sound,RELEASE)+_CV;
        else releaseInterval=getVar(_sound,RELEASE);
      }

      break;
    case 2:
      // if(!hw.knobFreezed(0)){
      if(cvAssign==3) ll=getVar(_sound,LOOP_LENGTH)+_CV;
      else  ll=getVar(_sound,LOOP_LENGTH);
      if(sync) loopLength=1,ll=1;//pgm_read_word_near(usefulLengths+(ll>>3));
      else loopLength=ll<<LOOP_LENGTH_SHIFT;
      //}
      if(!hw.knobFreezed(1)) {
        //if(sync) shiftSpeed=((long)getVar(_sound,SHIFT_SPEED)-128)<<SHIFT_SPEED_SYNC_SHIFT;
        //else 
        if(cvAssign==4) shiftSpeed=(((long)getVar(_sound,SHIFT_SPEED)-128)+_CV)<<SHIFT_SPEED_SHIFT;
        else  shiftSpeed=(((long)getVar(_sound,SHIFT_SPEED)-128))<<SHIFT_SPEED_SHIFT;
      }
      break;
    case 3:
      if(!hw.knobFreezed(0)){ //splice here??
        if(!revMidi){
          if(cvAssign==5)  startIndex=getVar(_sound,START)+_CV;
          else startIndex=getVar(_sound,START);
          //if(startIndex>endIndex) startIndex=endIndex-3;//MINIMAL_LOOP; //novinka
          startPosition=startIndex*startGranule;
        }
        //setEnd(_sound); //novinka

      }
      //if(!hw.knobFreezed(3))  
      if(!revMidi)   setEnd(_sound);



      break;

    }
  }


}
void showValue(int _value, uint8_t _variable){

  switch(_variable){
  case RATE:
    hw.lightNumber(map(_value,-360,60,40,52));
    if(_value==0) hw.setDot(true);
    else hw.setDot(false);
    break; 
  case SHIFT_SPEED:
    hw.lightNumber(map(_value,-126,127,40,52));
    if(_value==0) hw.lightNumber(46),hw.setDot(true);
    else if(_value==-126) hw.setDot(true);
    else if(_value==127) hw.setDot(true);
    else hw.setDot(false);
    break; 
  case START:
    hw.lightNumber(map(_value,0,1023,40,52));
    if(_value==1023) hw.setDot(true);
    else if(_value==0) hw.setDot(true);
    else hw.setDot(false);
    break; 
  case END:
    hw.lightNumber(map(_value,0,1023,40,52));
    if(_value==1023) hw.setDot(true);
    else if(_value==0) hw.setDot(true);
    else hw.setDot(false);
    break; 
  default:
    hw.lightNumber(map(_value,0,127,40,52));
    if(_value==0) hw.setDot(true);
    else if(_value==127) hw.setDot(true);
    else hw.setDot(false);

    break;
  }



  // else  
  /*
  boolean minus=false;
   if(_value<0) minus=true;
   _value=abs(_value);
   
   hw.displayNumber(_value);
   if(!(_value/100)) hw.lightNumber(VOID,1);
   if(!((_value%100)/10)){
   if(_value<100) hw.lightNumber(VOID,2);
   */
  /*
    if(_value<0){ 
   hw.lightNumber(MINUS,2);
   }
   
   }
   */
  /*
  if(minus) {
   if(_value>=100) hw.lightNumber(MINUS,0);
   else if(_value>=10) hw.lightNumber(MINUS,1);
   else hw.lightNumber(MINUS,2);
   }
   
   if(_value>=1000) hw.lightNumber(1,0),hw.lightNumber(0,1);
   */
}
void renderCombo(){

  boolean _page=hw.buttonState(PAGE);

  if(_page && hw.justPressed(UP)) combo=true,loadPreset(currentBank,currentPreset+1),showForWhile(presetName[2]),clearIndexes(),hw.freezeAllKnobs();//,showForWhile("pr  "),hw.setDot(1,true),hw.displayChar(presetName[2],3),hw.displayChar(presetName[1],2),clearIndexes();
  if(hw.buttonState(PAGE) && hw.justPressed(DOWN)) combo=true,loadPreset(currentBank,currentPreset-1),showForWhile(presetName[2]),hw.freezeAllKnobs();//,showForWhile("pr  "),hw.setDot(1,true),hw.displayChar(presetName[2],3),hw.displayChar(presetName[1],2),clearIndexes();
  // if(hw.buttonState(DOWN) && hw.justPressed(UP)) combo=true, copy(activeSound) ; //copy
  //if(hw.buttonState(UP) && hw.justPressed(DOWN)) combo=true, paste(activeSound),hw.freezeAllKnobs(); // paste

  shift=hw.buttonState(FN);
  if(shift){

    if(hw.justPressed(DOWN)){
      combo=true;
      if(storePreset(currentBank,currentPreset)) showForWhile('s'); 
      else showForWhile('e');
    }
    if(hw.justPressed(PAGE)|| hw.justPressed(UP)){
      combo=true;
      setting=getVar(activeSound,SETTING);
      int i=0;
      if(hw.justPressed(PAGE)) i=REPEAT_BIT;
      if(hw.justPressed(UP)) i=SYNC_BIT;
      bitWrite(setting,i,!bitRead(setting,i));

      if(bitRead(setting,i) && i==SYNC_BIT) cvAssign=255;//, clockCounter=1;

      setVar(activeSound,SETTING,setting); 
      setSetting(activeSound);



    }


    //  if(hw.justPressed(bigButton[5])) combo=true, randomize(activeSound), loadValuesFromMemmory(activeSound);// demo();

  }


  if(combo){
    //turn off combo when all buttons are released 
    unsigned char _count=0; 
    for(int i=0;i<4;i++)  _count+=hw.buttonState(i);
    if(_count==0) combo=false;
  }  
}

int snapToUseful(int _number){
  unsigned char smallerThan;
  for(int i=0;i<16;i++){
    if(_number < pgm_read_word_near(usefulLengths+i)){
      smallerThan=i; 
      break;
    }
  }

  int difference=pgm_read_word_near(usefulLengths+smallerThan)-pgm_read_word_near(usefulLengths+smallerThan-1);
  unsigned char _clock;
  if(_number>= ((difference/2)+pgm_read_word_near(usefulLengths+smallerThan-1))) _clock=smallerThan;
  else _clock=smallerThan-1;
  _number=pgm_read_word_near(usefulLengths+_clock);
  return _number;

}
void renderSmallButtons(){

  if(!combo){

    if(hw.justPressed(UP) && !hw.buttonState(DOWN)){
      stopSound();
      listNameUp();
      setVar(activeSound,SAMPLE_NAME_1,name[0]);
      setVar(activeSound,SAMPLE_NAME_2,name[1]);
      sound=activeSound;
      if(notesInBuffer>0) playSound(sound);
      else showSampleName(),whileShow=true,whileTime=millis();
      if(hw.buttonState(DOWN)) longTime=millis();
      longPress=false;
    } 

    if(hw.justPressed(DOWN) && !hw.buttonState(UP)){
      stopSound();
      listNameDown();
      setVar(activeSound,SAMPLE_NAME_1,name[0]);
      setVar(activeSound,SAMPLE_NAME_2,name[1]);
      sound=activeSound;
      if(notesInBuffer>0) playSound(sound);
      else showSampleName(),whileShow=true,whileTime=millis();
      if(hw.buttonState(UP)) longTime=millis();
      longPress=false;
    } 

    doIRestartGrandPa();

    if(hw.justReleased(UP) || hw.justReleased(DOWN)){
      if(longPress){  
        sound=activeSound;
        indexed(activeSound,false);
        if(notesInBuffer>0) playSound(sound);
        else showSampleName(),whileShow=true,whileTime=millis();
      }
    }
  }

}

void doIRestartGrandPa(){
if(hw.buttonState(DOWN) && hw.buttonState(UP)){
      if(!longPress){
        if((millis()-longTime) > LONG_PERIOD) longPress=true, longTime=millis();
      }
      else{
        if(hw.buttonState(DOWN) && hw.buttonState(UP)) chacha();
        /*
        if((millis()-longTime) > MOVE_PERIOD){ 
         longTime=millis();
         if(hw.buttonState(DOWN)){
         downWithFirstLetter();
         setVar(activeSound,SAMPLE_NAME_1,name[0]);
         //  hw.displayChar(name[0],2);
         whileShow=true,whileTime=millis();
         
         
         }
         else if(hw.buttonState(UP)){
         upWithFirstLetter();
         setVar(activeSound,SAMPLE_NAME_1,name[0]);
         // hw.displayChar(name[0],2);
         whileShow=true,whileTime=millis();
         
         
         } 
         
         }
         */
      }
    }
}


unsigned char interfaceSound;
uint8_t bigButs[2]={
  BUTTON_A, BUTTON_B};
uint8_t bigButLed[2]={
  6, 7};
void renderBigButtons(){
  for(int i=0;i<2;i++) {
    if(hw.justPressed(bigButs[i])){
      sound=i;
      //  if(sound!=activeSound){ 
      hw.freezeAllKnobs();
      activeSound=sound,interfaceSound=sound, putNoteIn(sound);
      // }
    }
    if(hw.justReleased(bigButs[i]))  sound=putNoteOut(i),hw.freezeAllKnobs();
    //if(activeSound==i && notesInBuffer>0) hw.setLed(bigButLed[i],true);
    //else hw.setLed(bigButLed[i],false);

  }
  /*
  for(int i=0;i<NUMBER_OF_BIG_BUTTONS;i++) {
   if(hold){
   if(hw.justPressed(bigButton[i])) putNoteOut(sound), sound=i, activeSound=sound,putNoteIn(sound), interfaceSound=sound;//,playSound(sound);
   }
   else{
   if(hw.justPressed(bigButton[i])) sound=i,activeSound=sound,putNoteIn(sound),interfaceSound=sound;//,playSound(sound);
   if(hw.justReleased(bigButton[i])) sound=putNoteOut(i);// stopEnvelope();
   }     
   
   if(activeSound==i && notesInBuffer>0) hw.setLed(bigButton[i],true);
   else hw.setLed(bigButton[i],false);
   }
   */
}

void renderRecordRoutine(){
  /*
  if(recSound==0){
   if(hw.justPressed(REC) || hw.justPressed(PAGE)) rec=false,restoreAnalogRead(),combo=true;
   if(hw.justPressed(HOLD)) thru=!thru,wave.setAudioThru(thru);
   hw.setLed(HOLD,thru);
   for(int i=0;i<NUMBER_OF_BIG_BUTTONS;i++) {
   if(hw.justPressed(bigButton[i])) recSound=i,trackRecord(recSound,currentPreset);
   }
   //   displayVolume();
   }
   */
}

void displayVolume(){

  unsigned char _range= wave.adcGetRange();
  for(int i=0;i<4;i++){
    // if(_range>=((i+1)*31)) hw.setDot(i,true);
    //else hw.setDot(i,false);
  }
  wave.adcClearRange();
}
void renderHold(){
  /*
  if(!shift){
   if(hw.justPressed(HOLD)){
   if(hold){
   hold=false;
   unsigned char keepPlaying=255;
   for(int i=0;i<NUMBER_OF_BIG_BUTTONS;i++){
   if(hw.buttonState(bigButton[i])){
   if(i==midiBuffer[ZERO]) keepPlaying=i;
   }
   }
   if(keepPlaying!=255) sound=keepPlaying;//, clearBuffer(1);
   else clearBuffer(),sound=0, stopEnvelope(),instantLoop=0;
   }
   else hold=true;
   }
   }
   hw.setLed(HOLD,hold); 
   */
}

// bylo 3 novinka

#define TOLERANCE_2 1


unsigned int mapComp(unsigned int _val,unsigned int lowT,unsigned int hiT,unsigned int midSet){ 

  if(_val>hiT){
    //_val=myMap(_val-hiT,1023-hiT,1023); //-hiT

      //_val= (_val ) * (1023 - lowT) / (1023 ) + lowT;

    //_val=map(_val,hiT,1023,midSet,1023);
    //     return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    //  _val= (((_val - hiT) * ((2*(1023 - midSet)) / (1023 - hiT)))/2)  + midSet; //    
    // _val=myMap(_val,lowT,midSet)-85;

    _val-=hiT;
    _val=myMap(_val,1023-hiT,1023-midSet);
    _val+=midSet;
  }
  else if(_val>lowT){
    _val=midSet;
  } 

  else{
    //_val=map(_val,0,lowT,0,midSet);
    // _val= (_val ) * (out_max ) / (1023 ) ;
    _val=myMap(_val,lowT,midSet);
  }

  return _val;
}

#define LOW_T_L 1
#define HI_T_L 200
#define MID_V_L 8

#define LOW_T_S 400
#define HI_T_S 600
#define MID_V_S 512
uint8_t lastPage;
void renderKnobs(){
  unsigned char _sound;
  //if(notesInBuffer>0) _sound=activeSound;
  //else 
  _sound=activeSound; //interfaceSound

    if(shift){
    for(int i=0;i<2;i++){
      if(hw.knobMoved(i)){
        hw.setColor(WHITE);
        unsigned char _variable=i+(VARIABLES_PER_PAGE*page);
        cvAssign=_variable;


        uint8_t setting=getVar(activeSound,SETTING); //remove grainSync

        bitWrite(setting,TUNED_BIT, bitRead(cvAssign,0));
        bitWrite(setting,SHIFT_DIR_BIT, bitRead(cvAssign,1));
        bitWrite(setting,LEGATO_BIT, bitRead(cvAssign,2));
        bitWrite(setting,EXTRA_BIT, 0);

        bitWrite(setting,SYNC_BIT,false);
        setVar(activeSound,SETTING,setting); 
        setSetting(activeSound);
        //  Serial.println(cvAssign);


      }

    }
  }
  else{
 uint8_t _page=constrain(page,0,3);//%4;
    for(int i=0;i<2;i++){
    
      unsigned char _variable=i+(VARIABLES_PER_PAGE*_page);
      //32132

      int _knobValue, _lastKnobValue;
      //int _varNow=getVar(_sound,_variable);
      int was=getVar(_sound,_variable);


      _knobValue=hw.knobValue(i);
      //   _lastKnobValue=hw.lastKnobValue(i);

      if(_variable==LOOP_LENGTH)  _knobValue=mapComp(_knobValue,LOW_T_L,HI_T_L,MID_V_L);
      else if(_variable==SHIFT_SPEED) {
        _knobValue=mapComp(_knobValue,LOW_T_S,HI_T_S,MID_V_S);
        // if(_knobValue==511) _knobValue=512; 
      }
      //inBetween( scale(_knobValue,KNOB_BITS,variableDepth[_variable]), scale(_lastKnobValue,KNOB_BITS,variableDepth[_variable]),was ) 

      if(hw.knobFreezed(i)) {

        // if(page == lastPage){
        //if(inBetween( scale(_knobValue,KNOB_BITS,variableDepth[_variable]), scale(hw.lastKnobValue(i),KNOB_BITS,variableDepth[_variable]),was ) ) Serial.println(_knobValue),Serial.println(hw.lastKnobValue(i)),Serial.println(was),hw.unfreezeKnob(i);
        if(inBetween( scale(_knobValue,KNOB_BITS,variableDepth[_variable]), scale(hw.lastKnobValue(i),KNOB_BITS,variableDepth[_variable]),was ) || hw.knobMoved(i)) hw.unfreezeKnob(i),hw.setColor(WHITE);//,showForWhile(knobLabel(page,i)),lastMoved==i; //external unfreez
        // }
        //  hw.setLastKnobValue(i,_knobValue);
        // lastPage=page;
      }

      else{    
        // hw.setLed(knobLed[i],true);   
        int _value=scale(_knobValue,KNOB_BITS,variableDepth[_variable]);
        //_varNow;

        setVar(_sound,_variable,_value); 
        // long _timeNow=millis();




        if(variableDepth[_variable]>8){ //novink
          //if(((was>>2)!=(_value>>2))) { //minus větší než - ripple compensate // novinka
          if(abs(was-_value)>TOLERANCE) {
            lastMoved=i;
            whileShow=true;
            whileTime=millis();

            //

            //if(_variable==START) loadValuesFromMemmory(sound); //snap to start //novinka
          }

        }
        else if((was>>3)!=(_value>>3)) { ////minus větší než - ripple compensate // novinka

          lastMoved=i;
          whileShow=true;
          whileTime=millis();
        }

        /*
        if(hw.knobMoved(i) || was!=_value) {
         lastMoved=i;
         whileShow=true;
         whileTime=millis();
         }
         */
        //  if(was!=_value)

        boolean showSlash=false;
        if(lastMoved==i){
          if(_variable==RATE){
            if(tuned){
              _value=pitch-36;
              //  _value=myMap(_value,1024,43);
              // _value-=36;
            }
            else _value=pitch-360;//_value-=860;

          }
          else if(_variable==LOOP_LENGTH && sync){
            _value=pgm_read_word_near(usefulLengths+(_value>>3));
            if(_value<24 && _value>0) _value= 24/_value,showSlash=true;
            else _value/=24; 
          }
          else if(_variable==SHIFT_SPEED){
            _value-=128;
          }
          else if(_variable==END){
            if(sync){
              _value=pgm_read_word_near(usefulLengths+(_value>>6)+1);
              if(_value<24 && _value>0) _value= 24/_value,showSlash=true;
              else _value/=24; 
            }
            else _value=endIndex; 
          }
          //hw.displayChar(pgm_read_word_near(labels + i + page*PAGE_KNOB_LABEL_OFFSET),0);
          showValue(_value, _variable);
          //  if(showSlash) hw.lightNumber(SLASH,1);

        }
      }

    }
  }

}



void renderDisplay(){


  if(shift){
    //  hw.displayText("set "); 
    // hw.displayChar(activeSound+49,3);
    //  for(int i=0;i<5;i++)  hw.setLed(bigButton[i],bitRead(setting,i));

    hw.setLed(bigButLed[0],repeat);
    hw.setLed(bigButLed[1],bitRead(setting,SYNC_BIT));


    //  hw.setLed(bigButton[5],false);

  }
  else{
    if(wave.isPlaying() && activeSound==0) hw.setLed(bigButLed[0],true);
    else hw.setLed(bigButLed[0],false);
    if(wave.isPlaying() && activeSound==1) hw.setLed(bigButLed[1],true);
    else hw.setLed(bigButLed[1],false);
  }

  if(instantLoop==2){
    // hw.displayText("loop"); 
  }//else if(

  else if(whileShow){
    if(millis()-whileTime>WHILE_DURATION) whileShow=false,showSampleName(),noDots(),lastMoved=5;
  }
  else if(!wave.isPlaying() && !rec) {
    hw.lightNumber(LINES);
    // for(int i=0;i<4;i++) hw.lightNumber(LINES,i);
    noDots();
  } 
  else{
    showSampleName(); 
    noDots();
  }
  //showValue(bytesAvailable);
}



void showForWhile(char show){
  whileShow=true;
  whileTime=millis();
  hw.displayChar(show);
  // hw.displayText(show); 
}


void blinkLed(unsigned char _LED,int interval){
  if(++blinkCounter >= interval) blinkCounter=0, blinkState=!blinkState;
  // hw.setLed(_LED,blinkState);
  //
}


void randomize(unsigned char _sound){
  for(int i=0;i<8;i++) setVar(_sound,i,rand(maxVal(i))); 
  setVar(_sound,CRUSH,rand(20));
  setVar(_sound,ATTACK,rand(20));
}

unsigned char copyMemory[NUMBER_OF_BYTES];
void copy(unsigned char _sound){
  showForWhile('c');
  for(int i=0;i<NUMBER_OF_BYTES;i++) copyMemory[i]=variable[_sound][i];
}

void paste(unsigned char _sound){
  showForWhile('p');
  for(int i=0;i<NUMBER_OF_BYTES;i++) variable[_sound][i]=copyMemory[i];
  indexed(_sound,false);
  hw.freezeAllKnobs();
}

void noSound(){
  envelopePhase=3;
  stopSound();
}
void stopSound(){
  wave.stop(),file.close();
}

void loadName(unsigned char _sound){
  //for(int i=0;i<2;i++)  name[i]=getVar(_sound,SAMPLE_NAME_1+i);
  name[0]=firstLetter;//getVar(_sound,SAMPLE_NAME_1);
  name[1]=getVar(_sound,SAMPLE_NAME_2);
}

void showSampleName(){

  // hw.displayChar('S',0);
  //  hw.displayChar(' ',1);
  hw.displayChar(name[1]);
  // hw.displayChar(name[1],3);

}


void noDots(){
  //for(int i=0;i<4;i++) hw.setDot(i,false);
}

/*
char* knobLabel(unsigned char _page,unsigned char _knob){
 
 char label[NUMBER_OF_DIGITS];
 for(int i=0;i<NUMBER_OF_DIGITS;i++) label[i]=pgm_read_word_near(labels + i + _knob + _page*PAGE_KNOB_LABEL_OFFSET);
 return label;
 
 }
 */
/*
void demo(){
 
 currentBank=9;
 currentPreset=5;
 loadPreset(currentBank,currentPreset);
 dimLeds();
 
 for(int i=0;i<30;i++){
 hw.setLed(bigButton[5],true);
 unsigned char _note=rand(6);
 putNoteIn(_note);
 while(wave.isPlaying()){
 hw.displayText("demo");
 hw.update();
 updateSound();
 if(hw.buttonState(bigButton[5])) break;  
 }
 putNoteOut(_note);
 } 
 // clearBuffer();
 chacha();
 
 }
 
 
 
 */

































