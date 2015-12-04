// op qr st   uv wx yz
unsigned char incomingByte,note;
boolean ignore, comandOff,slave=false,cc; //,midiNoteOn  noteOn,
unsigned char state=0;
int ll;
boolean shiftDir,sync=true;

#define MIDI_CHANNEL 1023
#define POLYPHONY NUMBER_OF_VOICES
unsigned char notesInBuffer=ZERO;
boolean thereIsNoteToPlay;

unsigned char inputChannel;
#define BUFFER_SIZE 3
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
uint16_t sampleRateNow;
unsigned char setting;
int attackInterval, releaseInterval;
//long legatoPosition;


void shiftBufferLeft(unsigned char from){
  for(uint8_t i=from;i<notesInBuffer;i++){
    midiBuffer[i]=midiBuffer[i+1]; 
  }
}

void shiftBufferRight(){
  for(uint8_t i=notesInBuffer;i>ZERO;i--){
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
  for(uint8_t i=ZERO;i<BUFFER_SIZE;i++) midiBuffer[i]=EMPTY;
  notesInBuffer=0;
}
boolean removeNote(unsigned char note){
  if(notesInBuffer>ZERO){ 
    unsigned char takenOut;
    boolean takeOut=ZERO;

    for(uint8_t i=ZERO;i<notesInBuffer;i++){
      if(midiBuffer[i]==note) takeOut=true, takenOut=i;
    } 

    if(takeOut){
      shiftBufferLeft(takenOut);
      notesInBuffer--;
      for(uint8_t i=notesInBuffer;i<BUFFER_SIZE;i++) midiBuffer[i]=EMPTY;
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









