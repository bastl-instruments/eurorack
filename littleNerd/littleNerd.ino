/*
 * LITTLE NERD
 * eurorack module by Bastl Instruments
 * www.bastl-instruments.com
 * Vaclav Pelousek
 * licensed under cc-by-sa
 *
 *TODO
 *finish visual testing
 *do audioble testing
 *implement production code = testing + eeprom reset
 *implement midi ?
 *
 */

#ifdef EXTERNAL_IDE

#include <Arduino.h>
#include <EEPROM.h>

int main(void) {

  init();
  setup();

  while(true) {
    loop();
  }
}

#endif

#include <portManipulations.h>
#include "littleNerdHW.h"
#include "eventDelay.h"
#include "eventMemory.h"
#include "euclid.h"


extern littleNerdHW hardware;
eventDelay dly;
euclid euclidian[6];
//StepMultiplier mult[6];
eventMemory memory;

/*
void stepperStep() {
	player->stepFourth();
	synchronizer.doStep();
}
*/

//event types
#define CYCLE_TRIGGER 3
#define STANDART_TRIGGER 2
#define TRIGGER_OFF 0
#define TRIGGER_ON 1

void noteOn(unsigned char note, unsigned char velocity, unsigned char channel) {

}

void noteOff(unsigned char note, unsigned char velocity, unsigned char channel) {

}

void midiNoteOnIn(unsigned char channel, unsigned char note, unsigned char velocity) {


}

void midiNoteOffIn() {

}
long myMap(long x,  long in_max,  long out_max)
{
  return (x) * (out_max) / (in_max);
}

void test(uint8_t v) {
	//if(hardware.getButtonState(v) == IHWLayer::UP)

		//if(hardware.getButtonState(v)==IHWLayer::DOWN) hardware.setLED(v,IHWLayer::ON);
		//else hardware.setLED(v,IHWLayer::OFF);
}
#define FILTER 0
#define DIVIDER 1
#define FLOP_DIVIDER 2
#define MULTIPLIER 3

#define GROOVE 4
#define EUCLID 5
#define OSC 6
#define PROBABILITY 7

uint8_t bitShifter[8]={0,3,3,5,0,3,0,0};

#define DEFAULT_TRIGGER_LENGTH 20 // around 10ms

#define EDIT 1
#define MODE 0
boolean editMode;
uint8_t selectedChannel=255;
uint8_t channelMode[6]={0,0,FLOP_DIVIDER,0,DIVIDER,OSC};
uint32_t clockInTime[6],lastClockInTime[6], channelTime[6], lastChannelTime[6];
uint16_t counter[6];
uint16_t counterLeft[6];
bool flop[6];
uint8_t currentPreset=0;
uint8_t parameter[6][2]={{0,0},{0,0},{0,0},{0,0},{0,0},{0,20}};

void resetEverything(){
	//uint32_t _time=hardware.getElapsedBastlCycles();
	for(int i=0;i<6;i++){
		/*
		clockInTime[i]=_time;
		channelTime[i]=_time;
		*/
		if(channelMode[i]!=MULTIPLIER){
		counter[i]=0;
		counterLeft[i]=0;
		euclidian[i].resetSequence();
		}
		if(channelMode[i]!=PROBABILITY){
			if(parameter[i][1]==255) channelTime[i]=0;
		}
	}
	//dly.clearBuffers(); ?? yes or no?
	hardware.resetTriggers();
}
void resetChannel(uint8_t i){
	uint32_t _time=hardware.getElapsedBastlCycles();

		clockInTime[i]=_time;
		channelTime[i]=_time;
		counter[i]=0;
		counterLeft[i]=0;
		euclidian[i].resetSequence();

	//dly.clearBuffers();
	//hardware.resetTriggers();
}
void setAndRecordTrigger(uint8_t _ch, littleNerdHW::TriggerState type, uint16_t cycles){

	hardware.setTrigger(_ch,type,cycles);
	if(memory.recording()){
	if(cycles == DEFAULT_TRIGGER_LENGTH ){
		memory.createEvent(_ch,2,cycles,hardware.getElapsedBastlCycles());

	}
	else if(cycles ==0){
		if(type==littleNerdHW::ON) memory.createEvent(_ch,1,cycles, hardware.getElapsedBastlCycles());
		else memory.createEvent(_ch,0,cycles,hardware.getElapsedBastlCycles());

	}
	else{
		cycles=(cycles>>6)+1;
		if(cycles>7) cycles =7;
		memory.createEvent(_ch,3,cycles,hardware.getElapsedBastlCycles());
	}
	}

}

void save(uint8_t _preset){
	uint16_t presetOffset=_preset*32;
	for(int i=0;i<6;i++){
		for(int j=0;j<2;j++){
			EEPROM.write(presetOffset+j+i*2,parameter[i][j]);
		}
		EEPROM.write(presetOffset+12+i,channelMode[i]);

	}

}
void load(uint8_t _preset){
	uint16_t presetOffset=_preset*32;
		for(int i=0;i<6;i++){
			for(int j=0;j<2;j++){
				parameter[i][j]=EEPROM.read(presetOffset+j+i*2);
			}
			channelMode[i]=EEPROM.read(presetOffset+12+i);
		}
}

#define NUMBER_OF_MODES 8

uint32_t showTime;
bool show;

void renderNumberDisplay(uint8_t i, uint8_t index, uint8_t _ch){
	if(hardware.getElapsedBastlCycles()-showTime>100){
		//if(index==0) hardware.setColor(BLACK);
	show=false;
	if(channelMode[i]==DIVIDER || channelMode[i]==FLOP_DIVIDER || channelMode[i]==MULTIPLIER ||channelMode[i]==EUCLID){
		if(index==0){ // only for EUCLID - needs to map the fills because maximum is the number of steps
			if(myMap(parameter[_ch][0]>>3,31,parameter[_ch][1]>>3)!=myMap(hardware.getKnobValue(i)>>3,31,parameter[_ch][1]>>3)){
						if(((myMap(hardware.getKnobValue(i)>>3,31,parameter[_ch][1]>>3)+1) % 4) == 0) hardware.setColor(WHITE);
						else hardware.setColor(channelMode[_ch]);
			}
			else hardware.setColor(BLACK);
		}
		else{
		if((parameter[i][index]>>bitShifter[channelMode[_ch]])!=(hardware.getKnobValue(i)>>bitShifter[channelMode[_ch]])){
			if((((hardware.getKnobValue(i)>>bitShifter[channelMode[_ch]])+1) % 4) == 0) hardware.setColor(WHITE);

			else hardware.setColor(BLACK);


			//Serial.println(i);
			showTime=hardware.getElapsedBastlCycles();
			show=true;
		}
		else hardware.setColor(channelMode[_ch]);
		}



	}


	}
	else {


		if(channelMode[i]==DIVIDER || channelMode[i]==FLOP_DIVIDER || channelMode[i]==MULTIPLIER || channelMode[i]==EUCLID){
		if((((hardware.getKnobValue(i)>>bitShifter[channelMode[_ch]])+1) % 4) == 0) hardware.setColor(WHITE);
		else if(index==0) hardware.setColor(channelMode[_ch]);
		else hardware.setColor(BLACK);
		}


	}
}
uint32_t bothTime;
uint8_t col;
bool rainbow;
void buttonCall(uint8_t v) {
	//Serial.println(v);d
	if(v==EDIT){
		if(hardware.getButtonState(EDIT) == IHWLayer::DOWN) {
			if(hardware.getButtonState(MODE) == IHWLayer::DOWN){
				//bothTime=hardware.getElapsedBastlCycles();
				// looping

					if(!memory.recording() && !memory.looping()){
						memory.startRecording(hardware.getElapsedBastlCycles());
						for(int i=0;i<6;i++){
							memory.setInitialState(i,hardware.getTriggerState(i));
						}
					}
					else if(memory.recording()) memory.loopRecorded(hardware.getElapsedBastlCycles());
					else if(memory.looping()) memory.stopLoop();
					hardware.setColor(BLACK);

			}
			editMode=true;
			for(int i=0;i<6;i++) hardware.freezeKnob(i,parameter[i][1]);

			if(rainbow) save(currentPreset), rainbow=false,bothTime=hardware.getElapsedBastlCycles(), selectedChannel=255,hardware.setColor(BLACK);
		}
		if(hardware.getButtonState(EDIT) == IHWLayer::UP) {
			editMode=false;
			for(int i=0;i<6;i++) hardware.freezeKnob(i,parameter[i][0]);
			//showNumber(selectedChannel);
			hardware.setColor(BLACK);
			selectedChannel=255;

		}
	}
	if(v==MODE){
		//if(hardware.getButtonState(MODE) == IHWLayer::DOWN && hardware.getButtonState(EDIT) == IHWLayer::DOWN) bothTime=hardware.getElapsedBastlCycles();
		if(hardware.getButtonState(MODE) == IHWLayer::UP){
			if(editMode){
				if(selectedChannel!=255){
					//resetChannel(selectedChannel); // něco takovýho tu musí být ale
					if(channelMode[selectedChannel]<NUMBER_OF_MODES-1) channelMode[selectedChannel]++;
					else channelMode[selectedChannel]=0;

					hardware.setColor(channelMode[selectedChannel]);
					//Serial.println(channelMode[selectedChannel]);
				}
			}
			else{
				if(!memory.recording() && !memory.looping()) resetEverything(),hardware.setColor(BLACK); // RESET

			}
		}

		if(hardware.getButtonState(MODE) == IHWLayer::DOWN){
			if(rainbow) load(currentPreset), rainbow=false,bothTime=hardware.getElapsedBastlCycles(), selectedChannel=255,hardware.setColor(BLACK);
		}


	}
}






uint8_t divider;
uint32_t offset[6];

void renderMultiplier(uint8_t _ch){
	uint8_t multiplication=(parameter[_ch][1]>>5)+1;

	if (counterLeft[_ch] != 0) {

			if ((hardware.getElapsedBastlCycles() - lastChannelTime[_ch]) > (clockInTime[_ch] / multiplication)) {

				counter[_ch]++;
				unsigned char subStepsFromLastStep = (multiplication - counterLeft[_ch]);

				//Next line is handling problems with rounding so instead of increasing the last triggered
				//To the current time we increase it by division of the step multiplied by number of current step
				//That should have little better precision
				lastChannelTime[_ch] = channelTime[_ch] + (( clockInTime[_ch] * subStepsFromLastStep) / multiplication);
				counterLeft[_ch]--;

			}
		}

		//And here we trigger steps if there are some and the time from last step is lower then
		//Minimum distance in between steps that could be set by public setter
		if ((counter[_ch] > 0) &&
			((hardware.getElapsedBastlCycles() - lastClockInTime[_ch]) >  (clockInTime[_ch] / multiplication))) { // tady byl default trigger length
			setAndRecordTrigger(_ch,littleNerdHW::ON, (clockInTime[_ch] / multiplication)/2);
			/*
			if (stepCallback_ != 0) {
				stepCallback_(id);
			}
			*/
			lastClockInTime[_ch] = hardware.getElapsedBastlCycles();
			counter[_ch]--;
		}


	//mult[_ch].update(hardware.getElapsedBastlCycles());

}


void clockInChannel(uint8_t _ch){

	switch(channelMode[_ch]){
	case FILTER:
		if(hardware.getTriggerState(_ch)){ // NO_RE_TRIGGER // NEED TO TEST
			if(!(parameter[_ch][1]>>7)) setAndRecordTrigger(_ch,littleNerdHW::ON,(parameter[_ch][0]<<4)+1); // naladit
		}
		else setAndRecordTrigger(_ch,littleNerdHW::ON,(parameter[_ch][0]<<4)+1); // naladit
		break;

	case DIVIDER: // TESTED

		counter[_ch]++;
		divider=(parameter[_ch][1]>>3)+1;
		if(counter[_ch]%divider==0){
			counter[_ch]=0; // ?
			lastClockInTime[_ch]=clockInTime[_ch];
			clockInTime[_ch]=hardware.getElapsedBastlCycles();
			offset[_ch]=myMap(parameter[_ch][0],255,(clockInTime[_ch]-lastClockInTime[_ch]));
			dly.createEvent(_ch,STANDART_TRIGGER,hardware.getElapsedBastlCycles()+offset[_ch]);

			//setAndRecordTrigger(_ch,littleNerdHW::ON,DEFAULT_TRIGGER_LENGTH);
		}


		break;


	case FLOP_DIVIDER: // TESTED

		counter[_ch]++;
		divider=(parameter[_ch][1]>>3)+1;
		if(counter[_ch]%divider==0){
			counter[_ch]=0; // ?
			lastClockInTime[_ch]=clockInTime[_ch];
			clockInTime[_ch]=hardware.getElapsedBastlCycles();

			flop[_ch]=!flop[_ch];
			if(flop[_ch]) {
				offset[_ch]=myMap(parameter[_ch][0],255,(clockInTime[_ch]-lastClockInTime[_ch]));
				dly.createEvent(_ch,TRIGGER_ON,hardware.getElapsedBastlCycles()+offset[_ch]);
			}
			else dly.createEvent(_ch,TRIGGER_OFF,hardware.getElapsedBastlCycles()+offset[_ch]);

		}

		break;


	case MULTIPLIER:{ //
		uint8_t multiplication=(parameter[_ch][1]>>5)+1; //

		if (flop[_ch]) {
			clockInTime[_ch] = hardware.getElapsedBastlCycles() - channelTime[_ch];
		}
		channelTime[_ch] = hardware.getElapsedBastlCycles();
		lastChannelTime[_ch] = hardware.getElapsedBastlCycles();


		if(parameter[_ch][0]==0){ // clock multiplier // need to do full scale testing
			counter[_ch] += counterLeft[_ch];
			counterLeft[_ch] = ((multiplication) - 1);

			counter[_ch]++;
			renderMultiplier(_ch);
			flop[_ch] = true;
		}

		else { // trigger repeater TESTED !
			for(int i=0;i<multiplication;i++){
				dly.createEvent(_ch,STANDART_TRIGGER,hardware.getElapsedBastlCycles()+i*((parameter[_ch][0]<<2)+DEFAULT_TRIGGER_LENGTH*2)); // naladit
			}
		}
	}
		break;
	case OSC: //TESTED !
		if(parameter[_ch][1]==255) channelTime[_ch]=0; // reset
		break;
	case PROBABILITY: //  TESTED !
		if(parameter[_ch][1]==0){ // PROBABILITY
			if(random(255)<=parameter[_ch][0]) setAndRecordTrigger(_ch,littleNerdHW::ON,DEFAULT_TRIGGER_LENGTH);
		}
		else if(parameter[_ch][1]==255){ // PHASER
			lastClockInTime[_ch]=clockInTime[_ch];
			clockInTime[_ch]=hardware.getElapsedBastlCycles();
		}
		else{ // RANDOM PATTERN
			if(euclidian[_ch].getCurrentStep()) setAndRecordTrigger(_ch,littleNerdHW::ON,DEFAULT_TRIGGER_LENGTH);

/*
					Serial.print(" fills: ");
					Serial.print(euclidian[_ch].getNumberOfFills());
					Serial.print(" channel: ");
					Serial.print(_ch);
					Serial.print(" step no.: ");
					Serial.print(euclidian[_ch].getStepNumber());
					Serial.print(" step value: ");
					Serial.print(euclidian[_ch].getCurrentStep());
					Serial.println();
*/
			euclidian[_ch].doStep();
		}

		break;
	case GROOVE: // TESTED !

		if(parameter[_ch][1]==0){ // SIMPLE DELAY
			dly.createEvent(_ch,STANDART_TRIGGER,hardware.getElapsedBastlCycles()+(parameter[_ch][0]<<2)); //naladit
		}
		else if(parameter[_ch][1]==255){ //HUMANIZER
			dly.createEvent(_ch,STANDART_TRIGGER,hardware.getElapsedBastlCycles()+random(parameter[_ch][0]<<1)); // naladit
		}
		else{ //GROOVE
		lastClockInTime[_ch]=clockInTime[_ch];
		clockInTime[_ch]=hardware.getElapsedBastlCycles();
		offset[_ch]=myMap(parameter[_ch][0],255,(clockInTime[_ch]-lastClockInTime[_ch]));
		counter[_ch]++;
		if(counter[_ch]%2==1) dly.createEvent(_ch,STANDART_TRIGGER,hardware.getElapsedBastlCycles()+offset[_ch]);
		else setAndRecordTrigger(_ch,littleNerdHW::ON,DEFAULT_TRIGGER_LENGTH);
		}
		break;
	case EUCLID:{ // TESTED ! - DODELAT ZOBRAZOVANI MIMO EDIT MOD
		uint8_t fills=myMap(parameter[_ch][0]>>3,31,parameter[_ch][1]>>3);
		euclidian[_ch].generateSequence(fills+1,(parameter[_ch][1]>>3)+1);
		if(euclidian[_ch].getCurrentStep()) setAndRecordTrigger(_ch,littleNerdHW::ON,DEFAULT_TRIGGER_LENGTH);
		/*
		Serial.print(" steps: ");
		Serial.print((parameter[_ch][1]>>3)+1);
		Serial.print(" fills: ");
		Serial.print(fills+1);
		Serial.print(" channel: ");
		Serial.print(_ch);
		Serial.print(" step no.: ");
		Serial.print(euclidian[_ch].getStepNumber());
		Serial.print(" step value: ");
		Serial.print(euclidian[_ch].getCurrentStep());
		Serial.println();
		*/
		euclidian[_ch].doStep();
		}
		break;

	}

}

//uint32_t clockInTime[6],lastClockInTime[6], channelTime[6], lastChannelTime[6];


// hardware.getBastlCycles();
void renderOsc(uint8_t _ch){
	uint16_t period=0;
	uint16_t duty=0;
	if(channelMode[_ch]==PROBABILITY){ // PHASER
		period=clockInTime[_ch]-lastClockInTime[_ch];
		period+=parameter[_ch][0];  // naladit phaser
		duty=period/2;
	}
	else if(parameter[_ch][1]==255){ // RESETING OSC
		period=(257-parameter[_ch][0])<<3; //naladit
		duty=period/2;
	}
	else{
		period=(257-parameter[_ch][0])<<3; //naladit
		duty=int(((float)period/2048)*float(parameter[_ch][1]<<3));
		if(duty>=period) duty=period-1;
		if(duty==0) duty=1;
	}
	if((hardware.getElapsedBastlCycles()-channelTime[_ch]) >= period){
		channelTime[_ch]=hardware.getElapsedBastlCycles();
		setAndRecordTrigger(_ch,littleNerdHW::ON,duty);
	}

}




void renderChannel(uint8_t _ch){
	switch(channelMode[_ch]){
	case MULTIPLIER:
		if(parameter[_ch][0]==0) renderMultiplier(_ch);
		break;
	case OSC:
		renderOsc(_ch);
		break;
	case PROBABILITY:
		if(parameter[_ch][1]==255){
			renderOsc(_ch);
		}
		/*
		else if(parameter[_ch][1]!=0){
			if(hardware.knobMoved(_ch)) euclidian[_ch].generateRandomSequence((parameter[_ch][0]>>4)+1,16);//,Serial.println("R");
		}*/
		break;
	default:
		break;
	}

}



uint8_t probability;
void renderClockIn(uint8_t in){
	//setAndRecordTrigger(in*4,littleNerdHW::ON,5); //wtf?
	if(in==0){
		clockInChannel(0);
		clockInChannel(1);
		clockInChannel(2);
	}
	else if(in==1){
		clockInChannel(4);
		clockInChannel(5);
	}
}

void clockCall(uint8_t w) {

	if(w==0) renderClockIn(0);

 // PROBABILITY MIXER for second channel
	static uint8_t nextClockFrom;
	if(nextClockFrom==w) renderClockIn(1);
	if(random(254)>=probability){
		nextClockFrom=1;
	}
	else {
		nextClockFrom=0;
	}

}
void handleEvent(uint8_t channel,uint8_t type,uint8_t cycles=0){
	switch(type){
		case STANDART_TRIGGER:
			setAndRecordTrigger(channel,littleNerdHW::ON,DEFAULT_TRIGGER_LENGTH);
			break;
		case TRIGGER_OFF:
			setAndRecordTrigger(channel,littleNerdHW::OFF,0);
			break;
		case TRIGGER_ON:
			setAndRecordTrigger(channel,littleNerdHW::ON,0);
			break;
		case CYCLE_TRIGGER:
		setAndRecordTrigger(channel,littleNerdHW::ON,cycles<<6);
			break;
		}
}
void eventNow(uint8_t channel, uint8_t type){
	handleEvent(channel,type);
}
void recEventNow(uint8_t channel, uint8_t type, uint8_t cycles){
	/*
	Serial.print(" CH: ");
	Serial.print(channel);
	Serial.print(" t: ");
	Serial.print(type);
	Serial.print(" c: ");
	Serial.print(cycles);
	Serial.println();
	*/
	handleEvent(channel,type,cycles);
}


/*
void multCall(unsigned char _id){
	handleEvent(_id,STANDART_TRIGGER);
	//Serial.println(_id);
}
*/
void setup() {
	Serial.begin(38400);
	//	Serial.println("start");

	hardware.init(&buttonCall,&clockCall);
	dly.init(&eventNow);
	memory.init(&recEventNow);
	load(currentPreset);
/*
	for(int i=0;i<6;i++){
		mult[i].init(hardware.getBastlCyclesPerSecond(),i);
		mult[i].setStepCallback(&multCall);
		mult[i].setMinTriggerTime(10);
	}
	*/


}


uint32_t timeNow;
#define LONG_TIME 5000
//bool rainbow;
uint8_t color;
uint8_t selectedChannel2;
void loop() {
	uint32_t _time=hardware.getElapsedBastlCycles();
	if(hardware.getButtonState(EDIT)==IHWLayer::DOWN && hardware.getButtonState(MODE)==IHWLayer::DOWN) {
		if(_time-bothTime>LONG_TIME) rainbow=true, memory.stopLoop();
	}
	else bothTime=_time;

	if(rainbow){
		if(_time-timeNow>500){
			timeNow = _time;
			if(color<7) color++;
			else color=0;
			hardware.setColor(color);
		}
		for(int i=0;i<6;i++){
			if(hardware.knobMoved(i)) currentPreset=i,load(currentPreset), rainbow=false, bothTime=_time, selectedChannel=255,hardware.setColor(BLACK);
		}
	}
	else{

	if(memory.looping()){
		memory.update(_time);
		if(_time-timeNow>(memory.getloopLength()/32)){
					timeNow = _time;
					if(color==RED) color=BLACK;
					else color=RED;
					hardware.setColor(color);
		}
	}
	else{

	dly.update(_time);
	for(int i=0;i<6;i++){
		renderChannel(i);
		if(editMode) {
			if(!hardware.knobFreezed(i)) {
				if(i==selectedChannel) renderNumberDisplay(i,1, selectedChannel);
				parameter[i][1]=hardware.getKnobValue(i);
			}
			if(hardware.knobMoved(i)){
				selectedChannel=i;
				if(!show) hardware.setColor(channelMode[selectedChannel]);//, Serial.println(i);
			}

		}
		else{


			if(!hardware.knobFreezed(i)){
				if(hardware.knobMoved(i)) selectedChannel2=i;
				if(channelMode[i]==EUCLID) if(selectedChannel2==i) renderNumberDisplay(i,0,i);

				if(channelMode[i]==PROBABILITY && parameter[i][1]!=0 && parameter[i][1]!=255){
					if((parameter[i][0]>>4)!=(hardware.getKnobValue(i)>>4)){
							euclidian[i].generateRandomSequence((hardware.getKnobValue(i)>>4)+1,16);
						}
					}

				parameter[i][0]=hardware.getKnobValue(i);
			}
			//if(!show) hardware.setColor(BLACK);
		}

	}
	probability=hardware.getKnobValue(3);
	if(memory.recording()){
		selectedChannel2=255;
	if(_time-timeNow>500){
						timeNow = _time;
						if(color==RED) color=BLACK;
						else color=RED;
						hardware.setColor(color);
			}
	}

	}
	}

}




