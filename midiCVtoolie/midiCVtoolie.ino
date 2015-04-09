

#ifdef EXTERNAL_IDE
#include <Arduino.h>
int main(void) {

  init();

  setup();

  while(true) {
    loop();
  }
}

#endif

#include <EEPROM.h>
#include <MIDI.h>
MIDI_CREATE_INSTANCE(HardwareSerial, Serial, MIDI)  ;
#include <MIDInoteBuffer.h>
#include<spiShared.h>
#define DAC_CONFIG 0x30
#define SS_DAC B,2
MIDInoteBuffer buffer;

void writeDAC(uint16_t signal) {

  bit_clear(SS_DAC);

  spiWrite(DAC_CONFIG | ((signal >> 8) & 0xF)); // 4 config bits and bits 11-8
  spiWrite(signal); // bits 7-0

  bit_set(SS_DAC);
}
void dacInit(){
	bit_clear(SCK);
	bit_clear(MISO);
	bit_clear(MOSI);
	bit_set(SS);

	bit_dir_outp(SCK);
	bit_dir_inp(MISO);
	bit_dir_outp(MOSI);
	bit_dir_outp(SS);


	// chip select pins

	bit_set(SS_DAC);
	bit_dir_outp(SS_DAC);

	// Mode
	SPCR |= _BV(SPE);    // enable SPI
	setHigh(SPCR,MSTR);   // SPI master mode
	SPCR &= ~_BV(SPIE);  // SPI interrupts off
	SPCR &= ~_BV(DORD);  // MSB first
	SPCR &= ~_BV(CPOL);  // leading edge rising
	SPCR &= ~_BV(CPHA);  // sample on leading edge
	SPCR &= ~_BV(SPR1);  // speed = clock/4
	SPCR &= ~_BV(SPR0);
	SPSR |= _BV(SPI2X);  // 2X speed
}
bool noteOn[127];

bool anyNoteOn(){
	for(int i=0;i<128;i++){
		if(noteOn[i]) return true;
	}
	return false;
}
void setGate(bool state){
 digitalWrite(8,state);
 digitalWrite(7,state);
}
bool calibrating=false;
uint8_t selectedNote;
//void loa
uint16_t tuneInputTable[60]={0,17,34,51,68,85,102,119,136,153,170,188,
						205,222,239,256,273,290,307,324,341,358,375,392,
						409,426,443,460,477,494,512, 529,546,563,580,597,
						614,631,648,665,682,699,716,733,750,767,784,801,
						818,835,853,870,887,904,921,938,955,972,989,1006};

uint16_t tuneTable[22]={0,6, 12,18, 24,30, 36,42, 48,54, 60, 0,410, 819,1229, 1638,2048, 2458,2867, 3277,3686, 4095};
#define TUNE_POINTS 11
void loadTable(){
	for(int i=0;i<22;i++){
		tuneTable[i]=word(EEPROM.read(i*2),EEPROM.read(1+(i*2)));
	}
}
void saveTable(){
	for(int i=0;i<22;i++){
		EEPROM.write(i*2,highByte(tuneTable[i]));
		EEPROM.write(1+(i*2),lowByte(tuneTable[i]));
	}
}
uint8_t signalToNote(uint16_t signal){
	uint8_t resultNote=0;
	for(int i=0;i<5;i++){
		if(signal>tuneInputTable[i*12]){
			for(int j=0;j<12;j++){
				if(signal>tuneInputTable[i*12+j]){
					uint16_t halfDistance=(tuneInputTable[i*12+j+1]-tuneInputTable[i*12+j]) / 2;
					if(signal>tuneInputTable[i*12+j]+halfDistance){
						resultNote=i*12+j+1;
						return resultNote;
					}
					else{
						resultNote=i*12+j;
						return resultNote;
					}
				}
			}
		}
	}
	return resultNote;
}



uint32_t curveMap(uint8_t value, uint8_t numberOfPoints, uint16_t * tableMap){
	uint32_t inMin=0, inMax=255, outMin=0, outMax=255;
	for(int i=0;i<numberOfPoints-1;i++){
		if(value >= tableMap[i] && value <= tableMap[i+1]) {
			inMax=tableMap[i+1];
			inMin=tableMap[i];
			outMax=tableMap[numberOfPoints+i+1];
			outMin=tableMap[numberOfPoints+i];
			i=numberOfPoints+10;
		}
	}
	return map(value,inMin,inMax,outMin,outMax);
}
#define CHROMATIC 0
#define MAJOR 1
#define HARMONIC_MINOR 2
#define MELODIC_MINOR 3
#define PENTATONIC_MAJOR 4
#define PENTATONIC_MINOR 5
#define BLUES 6
#define WHOLE_TONE 7

bool scale[8][12]{
		{1,1,1,1,1,1,1,1,1,1,1,1},
		{1,0,1,0,1,1,0,1,0,1,0,1},
		{1,0,1,1,0,1,0,1,0,1,1,0},
		{1,0,1,0,1,1,0,1,0,1,0,1},
		{1,0,1,0,1,1,0,1,0,1,0,1},
		{1,0,1,0,1,1,0,1,0,1,0,1},
		{1,0,1,0,1,1,0,1,0,1,0,1},
		{1,0,1,0,1,1,0,1,0,1,0,1},
};
bool quantizeTable[12];

uint8_t quantizeNote(uint8_t note){
	if(quantizeTable[note%12]){
		return note;
	}
	else{
		uint8_t higher=0;
		while(!quantizeTable[(note+higher)%12]){
			higher++;
		}
		uint8_t lower=0;
		while(!quantizeTable[(note-lower)%12]){
			lower++;
		}
		if(higher<lower) return note+higher;
		else return note-lower;
	}

	return note;
}

void calibrate(uint8_t note){
	if(note>=36 && note<=96){
		note-=36;
		if((note % 6) == 0){
			selectedNote=note;
			writeDAC(curveMap(selectedNote,TUNE_POINTS, tuneTable));
			//note-36
		}
		if((note % 6) == 5){
			if(((note+1)) == selectedNote) tuneTable[TUNE_POINTS+(selectedNote/6)]--;
			writeDAC(curveMap(selectedNote,TUNE_POINTS, tuneTable));
			saveTable();
		}
		if((note % 6) == 1){
			if(((note-1)) == selectedNote) tuneTable[TUNE_POINTS+(selectedNote/6)]++;
			writeDAC(curveMap(selectedNote,TUNE_POINTS, tuneTable));
			saveTable();
		}
	}
}

void handleNoteOn(byte channel, byte pitch, byte velocity)
{
	noteOn[pitch]=true;
	setGate(true);
	if(channel==1) calibrating=true;
	//else
	calibrating=false;

	if(calibrating){
		calibrate(pitch);
	//	digitalWrite(8,HIGH);
	}
	else{
		buffer.addNoteToBuffer(pitch,velocity);
		if(pitch>=36 && pitch<=96) writeDAC(curveMap(pitch-36,TUNE_POINTS, tuneTable));
		//digitalWrite(8,LOW);
	}

	//digitalWrite(8,HIGH);
}

void handleNoteOff(byte channel, byte pitch, byte velocity)
{
	buffer.removeNoteFromBuffer(pitch);
	if(buffer.getNumberOfNotesInBuffer()>0) writeDAC(curveMap(buffer.getNoteFromBuffer(0)-36,TUNE_POINTS, tuneTable));
	else setGate(false);
	noteOn[pitch]=false;
	//if(!anyNoteOn())


}

void setup(){

	dacInit();

	MIDI.begin(MIDI_CHANNEL_OMNI);
    MIDI.setHandleNoteOn(handleNoteOn);
    MIDI.setHandleNoteOff(handleNoteOff);

//saveTable();
    loadTable();

    pinMode(8,OUTPUT);
    pinMode(7,OUTPUT);
    buffer.init();
    buffer.setPolyphony(1);
}

void loop()
{/*
	for(int i=0;i<4096;i++){
	writeDAC(i);
	delay(1);
	}*/
    MIDI.read();

}




