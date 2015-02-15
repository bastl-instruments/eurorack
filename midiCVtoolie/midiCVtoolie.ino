

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

#include<spiShared.h>
#define DAC_CONFIG 0x30
#define SS_DAC B,2

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

}
bool calibrating=false;
uint8_t selectedNote;
//void loa
uint16_t tuneTable[10];

void calibrate(uint8_t note){
	if(note>=36 && note<=96){
		note-=36;
		if((note % 6) == 0){
			selectedNote=note/6;
			//dacWrite( curveMap selectedNote
			//note-36
		}
		if((note % 5) == 0){
			if(((note+1) /6) == selectedNote) tuneTable[selectedNote]--;
		}
		if((note % 7) == 0){
			if(((note-1) / 6) == selectedNote) tuneTable[selectedNote]++;
		}
	}
}

void handleNoteOn(byte channel, byte pitch, byte velocity)
{
	writeDAC(pitch);
	noteOn[pitch]=true;
	setGate(true);
	if(calibrating) calibrate(pitch);
}

void handleNoteOff(byte channel, byte pitch, byte velocity)
{
	noteOn[pitch]=false;
	if(!anyNoteOn()) setGate(false);
}

void setup(){

	dacInit();
    MIDI.setHandleNoteOn(handleNoteOn);
    MIDI.setHandleNoteOff(handleNoteOff);
    MIDI.begin(MIDI_CHANNEL_OMNI);
}

void loop()
{
    MIDI.read();
}




