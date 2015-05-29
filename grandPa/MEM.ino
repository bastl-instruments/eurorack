
PROGMEM prog_uint16_t clearTo[]={
  877, 0, 0,0,  0,128,0,1022, 4 , 82,48};
/*
// 4= 0010
 //13 = 1011 -  sync ON, shiftDir OFF,tuned ON,repeat ON, 
 
 red
 sampleRate, crush, start, end // loop
 
 green
 loopLen, shift, attack, release // normal-random
 
 blue
 rate, amt, shape, resolution // dest
 
 */

// variable names
//page 0


//osc mod-freq amt
//atck decay mod type // 2+2+3

#define PAGE_G 0
#define PAGE_B 1


#define VARIABLES_PER_PAGE 2
PROGMEM prog_char labels[]={
  "rcargmse"};

#define RATE 0 //1024 - 10 R
#define CRUSH 1 //127 - 7  C

#define ATTACK 2 // 127 - 7        A
#define RELEASE 3 // 127 - 7       R


// =41
#define LOOP_LENGTH 4  // 127 - 7  L
#define SHIFT_SPEED 5   // 127 - 7 S

#define START 6 //1024 - 10  S
#define END 7  //1024 - 10   E



//#define MODE 8 // 1 - 1            M

#define SETTING 8 // 7 - 4  L
// = 29
#define SAMPLE_NAME_1 9 // 127 - 7
#define SAMPLE_NAME_2 10 // 127 - 7
//#define SAMPLE_NAME_3 12 // 127 - 7
// = 14

/*
#define LFO_RATE 10 // 127 -7     R
 #define LFO_AMT 11 // 127 -7      A
 #define LFO_SHAPE 12 // 7 -3      S
 #define LFO_RES 13 // 15 - 4      R
 #define LFO_DEST 14 // 3 -2       D
 */
// =23





// 41+29+14 = 84 /8 = 10,5 =>11


#define KNOB_BITS 10


PROGMEM prog_uint16_t maxValue[]={
  1023,127,127,127, 127,255,1023,1023,63 ,127,127}; //const //127,127,7,15,3


#define NUMBER_OF_VARIABLES 11

#define NUMBER_OF_BYTES 12 //11
#define NUMBER_OF_PRESETS 36
#define NUMBER_OF_BANKS 9

#define PRESET_SIZE 84  //66 in real

#define NUMBER_OF_SOUNDS 6

#define CHANNEL_BYTE 1023


const unsigned char variableDepth[NUMBER_OF_VARIABLES]={
  10,7,7,7, 7,8,10,10,8, 7,7};



int maxVal(uint8_t _maxVal){
  return  pgm_read_word_near(maxValue+_maxVal);
}

const unsigned char byteCoordinate[NUMBER_OF_VARIABLES]={
  0, 1, 2, 3, 3, 4, 5, 7, 8, 9, 10} 
;

/*
unsigned char byteCoordinate[NUMBER_OF_VARIABLES]={
 0, 2, 3, 4, 5, 6, 7, 9, 11, 12, 13} 
 ;
 */

const unsigned char bitCoordinate[NUMBER_OF_VARIABLES]={
  0, 2, 1, 0, 7, 6, 6, 0, 2, 2, 1};


/*
int buffer[NUMBER_OF_SOUNDS][NUMBER_OF_VARIABLES];
 unsigned char bufferP[NUMBER_OF_STEPS];
 */

unsigned char variable[NUMBER_OF_SOUNDS][NUMBER_OF_BYTES];
/*
unsigned char variableCop[NUMBER_OF_SOUNDS][NUMBER_OF_BYTES];
 void copyPreset(){
 for(int i=0;i<NUMBER_OF_SOUNDS;i++){
 for(int j=0;j<NUMBER_OF_BYTES;j++){
 variableCop[i][j]=variable[i][j];
 } 
 }
 }
 void pastePreset(){
 for(int i=0;i<NUMBER_OF_SOUNDS;i++){
 for(int j=0;j<NUMBER_OF_BYTES;j++){
 variable[i][j]=variableCop[i][j];
 } 
 }
 }
 */
void initMem(){
  //calculateBitDepth();
  loadPreset(0,currentPreset);
}

void calculateBitDepth(){
  /*
  for(int i=0;i<NUMBER_OF_VARIABLES;i++){ // calculate bitDepth according to the maximum value
   int x=0;
   while(maxVal(i)-pow(2,x)>=0) x++;
   variableDepth[i]=x ;
   }
   */
  /*
  for(int i=0;i<NUMBER_OF_VARIABLES;i++){
   int sum=0;
   for(int j=0;j<i;j++){
   sum+=variableDepth[j];
   }
   byteCoordinate[i]=sum/8;
   
   bitCoordinate[i]=sum%8;
   Serial.print( bitCoordinate[i]);
   Serial.print(", ");
   
   } 
   */
}
// grainTrig, grain size 0-5v , grain speed -5 5V, crush, start, end,  attack, decay

int getVar(unsigned char _SOUND, unsigned char _VARIABLE){
  int _value=0;
  unsigned char byteShift=0;
  unsigned char _bitCoordinate=0;
  for(int i=0;i<variableDepth[_VARIABLE];i++){

    if((bitCoordinate[_VARIABLE]+i)>15){
      byteShift=2;
      _bitCoordinate = i-(16-bitCoordinate[_VARIABLE]);
    }
    else if((bitCoordinate[_VARIABLE]+i)>7){
      byteShift=1; 
      _bitCoordinate = i-(8-bitCoordinate[_VARIABLE]);//bitCount;
    }
    else {
      _bitCoordinate=bitCoordinate[_VARIABLE]+i;
    }

    boolean bitState=bitRead(variable[_SOUND][byteCoordinate[_VARIABLE]+byteShift],_bitCoordinate);
    bitWrite(_value,i,bitState);

  }
  //_value=_value+scale(expanderValue[_VARIABLE],8,variableDepth[_VARIABLE]);
  // if (_value >= maxVal(_VARIABLE)) _value=maxVal(_VARIABLE);
  return _value; 


}

void setVar(unsigned char _SOUND, unsigned char _VARIABLE, int _value){

  unsigned char byteShift=0;
  unsigned char _bitCoordinate;
  for(int i=0;i<variableDepth[_VARIABLE];i++){



    if((bitCoordinate[_VARIABLE]+i)>15){
      byteShift=2;
      _bitCoordinate = i-(16-bitCoordinate[_VARIABLE]);
    }
    else if((bitCoordinate[_VARIABLE]+i)>7){
      byteShift=1;
      _bitCoordinate = i-(8-bitCoordinate[_VARIABLE]);
    }
    else {
      _bitCoordinate=bitCoordinate[_VARIABLE]+i;
    }

    boolean bitState=bitRead(_value,i);
    bitWrite(variable[_SOUND][byteCoordinate[_VARIABLE]+byteShift],_bitCoordinate,bitState);
  }

}

char presetName[8]="p00.txt";
boolean storePreset(unsigned char _bank,unsigned char _preset) {
  // Serial.end();
  // Serial.begin(9600);
  noSound();
  //clearBuffer();
  _bank=0;
  if(_preset<10) presetName[2]=_preset+48; 
  else presetName[2]=_preset+65-10; 
  presetName[1]=_bank+48;

  if (!file.open(&root,presetName, O_WRITE | O_CREAT)) {
    //  errorLoop();
    return false;
  }
  for (int j = 0; j < NUMBER_OF_SOUNDS; j++) {
    for (int k = 0; k < NUMBER_OF_BYTES; k++) {
      file.write(variable[j][k]);
    }
  }

  file.close();

  //  clearIndexes();
  hw.freezeAllKnobs();
  return true;
}

void loadPreset(unsigned char _bank,unsigned char _preset) {
  _bank=0;
  if(_preset>=NUMBER_OF_PRESETS) _preset=NUMBER_OF_PRESETS-1;
  if(_preset>=200) _preset=0;
  if(_bank>=200) _bank=0;
  if(_bank>=NUMBER_OF_BANKS) _bank=NUMBER_OF_BANKS;
  //if(_bank<=0) _bank=0;
  if(_preset<10) presetName[2]=_preset+48; 
  else presetName[2]=_preset+65-10; 
  presetName[1]=_bank+48;

  currentPreset=_preset;
  currentBank=0;

  noSound();
  clearBuffer();
  if(!file.open(&root, presetName, O_READ)){
    clearPreset();
    storePreset(_bank,_preset);
    file.open(&root, presetName, O_READ);
  }

  for (int j = 0; j < NUMBER_OF_SOUNDS; j++) {
    for (int k = 0; k < NUMBER_OF_BYTES; k++) {
      variable[j][k]=file.read();
    }
  }
  file.close();
  hw.freezeAllKnobs();
  // showForWhile("pr  "),hw.displayChar(presetName[2],3),hw.displayChar(presetName[1],2), clearIndexes(); //,hw.setDot(1,true)
}



void clearMemmory(){
  for(int i=0;i<NUMBER_OF_BANKS;i++){
    for(int j=0;j<NUMBER_OF_PRESETS;j++){
      loadPreset(i,j);
      clearPreset();
      storePreset(i,j);
      //   hw.updateDisplay() ;
    }
  }
}

void clearPreset(){
  for(int i=0;i<NUMBER_OF_SOUNDS;i++){
    clearSound(i);

  }
}
void clearSound(unsigned char i){
  for(int j=0;j<NUMBER_OF_VARIABLES;j++){
    setVar(i,j, pgm_read_word_near(clearTo+j));
  } 
}
boolean inBetween(int val1,int val2,int inBet){

  boolean retVal;

  if(val1>=val2){
    if(inBet<=val1 && inBet>=val2) retVal=true;
    else retVal=false;
  }
  else if(val1<val2){
    if(inBet>=val1 && inBet<=val2) retVal=true;
    else retVal=false;
  }

  return retVal;

}

int scale(int _value, unsigned char _originalBits, unsigned char _endBits){
  if(_originalBits >= _endBits) return _value >> (_originalBits - _endBits);
  else return _value << (_endBits - _originalBits);
}



long myMap(long x,  long in_max,  long out_max)
{
  return (x) * (out_max) / (in_max);
}




