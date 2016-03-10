long loopLength;
int32_t  shiftSpeed;
uint32_t startPosition;
uint32_t endPosition;
int startIndex,endIndex;
boolean repeat;

boolean ending=true;
//unsigned char row;
boolean hold;
unsigned char page;

uint32_t lastPosition;

long instantStart,instantEnd;

unsigned char envelopePhase;
long envelopeTime;
int envelopeNow;


long startTime;
unsigned char lfoAmt;
boolean reverse;
long granularTime;
uint32_t sizeOfFile;
PROGMEM prog_uint16_t usefulLengths[17]={
  0,1,2,3,6,12,24/*doba*/,48/*2d*/,72/*3d*/,96/*4d*/,144/*6d*/,192/*8d*/,288/*12d*/,384/*16d*/,576/*24d*/,768/*32d*/,24000};


void updateSound(){

  if(shiftSpeed<0 && ll!=0) reverse=true; //&& ll!=0
  else reverse=false;
  // wave.getData();
  if(shiftDir && rand(2)==0) shiftSpeed=-shiftSpeed;
  if(notesInBuffer>0 || envelopePhase==2 || sustain){

    if(!wave.isPaused()){ //isPlaying
      renderEnvelope();
      renderLooping();
      renderGranular();
    }
    else {
      // setSetting(midiBuffer[ZERO]);
      if(sustain){
        renderEnvelope();
        renderLooping();
        renderGranular();
      }
      setSetting(activeSound);

      if(repeat){

        if(reverse) {
          wave.stop();
          wave.play(&file);
          // wave.setSampleRate(sampleRateNow);
          doGrainShift();

        }
        else loadValuesFromMemmory(sound);
      } 
      else stopSound();
    }
  }

  if(notesInBuffer>0){
    setSetting(activeSound);
    if(!wave.isPlaying() || wave.isPaused()){
      if(repeat) {
        if(reverse) {
          wave.stop();
          wave.play(&file);
          //  wave.setSampleRate(sampleRateNow);
          doGrainShift();

        }
        else playSound(midiBuffer[ZERO]); 
      }
    }
  }

}
#define POS_TOL 200

uint16_t comp;
int instantClockCounter;
long lastLL;

void renderLooping(){
  uint32_t _pos=wave.getCurPosition();
  if(instantLoop==2){
    /*
    if(sync){
     if((clockCounter % instantClockCounter)==0){
     wave.pause();
     wave.seek(instantStart);
     wave.resume();
     lastPosition=instantStart;
     }
     }
     */
    // else{
    //if(shiftSpeed<0 && ll!=0){ 
    if(reverse){
      if(_pos<=instantEnd){
        wave.pause();
        wave.seek(instantStart);
        wave.resume();
        lastPosition=instantStart;
      }
    }
    else{
      if(_pos>=instantEnd){
        wave.pause();
        wave.seek(instantStart);
        wave.resume();
        lastPosition=instantStart;
      }
    }
  }
  // }
  else{

    // if(shiftSpeed<0 && ll!=0){ 
    if(reverse){
      /*
      if(sync){
       if(endIndex!=1000){
       if((clockCounter % endIndex)==0){
       if(repeat) wave.pause(),loadValuesFromMemmory(sound);
       else stopSound();
       }
       }
       }*/

      //else{
      if(_pos<= startPosition ){
        if(repeat) lastPosition=endPosition,doGrainShift();//wave.pause(),loadValuesFromMemmory(sound),lastPosition=endPosition;// wave.pause(), lastPosition=endPosition,wave.seek(lastPosition),wave.resume();
        else stopSound();
      }
      if(_pos>= sizeOfFile-600) doGrainShift();

      // }

      /*
      if(_pos>= lastPosition ){
       lastPosition+=shiftSpeed;
       
       wave.pause();
       wave.seek(lastPosition);
       wave.resume();
       }
       */

    }
    else{
      // if(_pos<=startPosition) loadValuesFromMemmory(sound); // novinka
      /*
      if(sync){
       if(endIndex!=1000){
       if((clockCounter % endIndex)==0){
       if(repeat) wave.pause(),loadValuesFromMemmory(sound);
       else stopSound();
       }
       }
       }
       */
      // else{
      // if(abs(_pos-endPosition)>POS_TOL){ //novinka
      if(_pos>=endPosition){

        if(repeat) wave.pause(),loadValuesFromMemmory(sound),lastPosition=seekPosition;//,wave.seek(lastPosition),wave.resume();
        else stopSound();
      }
    }
    // }
  }

}


//#define COMPENSATION 70 //novinka
int lastCC;
void renderGranular(){

  if(ll!=0){
    uint32_t pos=wave.getCurPosition();
    if(lastLL==0) lastPosition=pos;
    if(millis()-granularTime>=(loopLength)){
      //+COMPENSATION//  novinka - kompenzace
      granularTime=millis(); 
      doGrainShift();
    }

    if(sync){
    }
    else{
    } 
  }
  if(clockCounter!=lastCC){
    doGrainShift();
  }
  lastLL=ll;
  lastCC=clockCounter;

}

void doGrainShift(){
  lastPosition+=shiftSpeed;
  if(shiftSpeed>=0){
    //lastPosition+=shiftSpeed;
    if(lastPosition>=endPosition) lastPosition=startPosition; 
    if(lastPosition+ll>=endPosition) lastPosition=startPosition; 
  }
  else{
    if(lastPosition>=endPosition || lastPosition<=startPosition) lastPosition=endPosition;
    //if(lastPosition<=startPosition) lastPosition=endPosition;
    // else lastPosition+=shiftSpeed;
  }
  wave.pause();
  wave.seek(lastPosition);
  wave.resume();
}



unsigned char velocity;
int attackInt;
void startEnvelope(unsigned char _velocity,int _attack){
  envelopeTime=millis();
  envelopeNow=37;
  envelopePhase=0;
  wave.setVolume(envelopeNow);
  velocity=31-(_velocity>>2);
  attackInt=_attack;
  if(attackInt==0) envelopePhase=1, envelopeNow=0,wave.setVolume(envelopeNow);
}

void stopEnvelope(){
  envelopeTime=millis();
  //  envelopeNow=0;

  if(envelopePhase!=2) wave.setVolume(envelopeNow);
  envelopePhase=2;
}

void renderEnvelope(){
  switch(envelopePhase){
  case 0:
/*
    if(attackInt==0){
      if(reverse) attackInt=1;
      else envelopePhase=1, envelopeNow=0,wave.setVolume(envelopeNow);
    }
    else 
    */
    if(millis()-envelopeTime>=attackInt){
      envelopeTime=millis();
      if(attackInt<30) envelopeNow-=3;
      else envelopeNow--;
      if(envelopeNow<=velocity) envelopeNow=0, envelopePhase=1;
      wave.setVolume(envelopeNow);
    }

    break;
  case 1:
    envelopeNow=velocity;
    wave.setVolume(velocity);
    break;
  case 2:
    if(wave.isPlaying()){
      if(releaseInterval==0) envelopePhase=3, envelopeNow=36,wave.setVolume(envelopeNow),stopSound();
      if(millis()-envelopeTime>=releaseInterval){
        envelopeTime=millis();
        if(releaseInterval==127) ;//sustain=true;
        else if(releaseInterval<30) envelopeNow+=3;//,sustain=false;
        else envelopeNow++;//,sustain=false;

        if(envelopeNow>=36) envelopePhase=3, stopSound();
        wave.setVolume(envelopeNow);
      }
    }
    else{
      envelopeNow=36;
      envelopePhase=3, stopSound();
      wave.setVolume(envelopeNow);
    }

    break;
  case 3:
    stopSound();
    break;
  }
}


static unsigned long x=132456789, y=362436069, z=521288629;

unsigned long xorshift96()
{ //period 2^96-1
  // static unsigned long x=123456789, y=362436069, z=521288629;
  unsigned long t;

  x ^= x << 16;
  x ^= x >> 5;
  x ^= x << 1;

  t = x;
  x = y;
  y = z;
  z = t ^ x ^ y;

  return z;
}

int rand(int maxval)
{
  return (int) (((xorshift96() & 0xFFFF) * maxval)>>16);
}
/*
int rand( int minval,  int maxval)
 {
 return (int) ((((xorshift96() & 0xFFFF) * (maxval-minval))>>16) + minval);
 }
 */















