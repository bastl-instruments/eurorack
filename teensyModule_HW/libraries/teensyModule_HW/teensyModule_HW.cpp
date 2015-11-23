/*
teensyModule hardware library
this library is to be used with Bastl Instruments eurorack hardware and Teensy 3.1
MOZZI library compatible
documentation of the library you find here:

Created by Tomash Ghzegowsky 2015 http://tomashg.com/
for Bastl Instruments http://www.bastl-instruments.com/
*/

#include "teensyModule_HW.h"
//#include <ADC.h>

//ADC *adc = new ADC();
uint8_t buttonPin[3]  = {
							BUTTON_A_PIN,
							SWITCH_PIN_A,
							SWITCH_PIN_B
};

uint8_t muxPin[3]{
	MUX_PIN_A,MUX_PIN_B,MUX_PIN_C
};

uint8_t ledPin[NUMBER_OF_LEDS]  = {
							LED_A_PIN,
							LED_B_PIN,
							LED_C_PIN,
							LED_D_PIN,
							LED_E_PIN
};

uint8_t cvPin[4]  = {
							CV_PIN_A,
							CV_PIN_B,
							CV_PIN_C,
							CV_PIN_D,
							
};

uint8_t knobPin[NUMBER_OF_KNOBS] = {  
							KNOB_PIN_A,
							KNOB_PIN_B,
							KNOB_PIN_C,
							KNOB_PIN_D
};


// class definition //

// constructor
teensyHWR::teensyHWR(){
	#if USE_CALLBACKS
	mButtonChangedCallback=NULL;
	mKnobChangedCallback=NULL;
	mCVChangedCallback=NULL;
	#endif // USE_CALLBACKS
	
	KNOB_TOLERANCE=2;
	
	analogReference(EXTERNAL);
	//analogReadResolution(14);
	analogReadAveraging(4);
}

// init fuctions

void teensyHWR::initialize(){
	for(int i=0;i<3;i++) pinMode(muxPin[i], OUTPUT);
	for(int i=0;i<3;i++) pinMode(buttonPin[i], INPUT_PULLUP);
	for(int i=0;i<NUMBER_OF_LEDS;i++) pinMode(ledPin[i], OUTPUT);
	pinMode(MUX_PIN,INPUT);
	pinMode(CV_PIN_E,INPUT);
	pinMode(SQUARE_OUT_PIN,OUTPUT);
	
	freezeAllKnobs();
	update();
}


void teensyHWR::update(){
	
	updateButtons();
	updateKnobs();
	updateCVs();
	writeToLeds();
}



//############# LED RELATED FUNCTIONS #############

// write the values from the hash to the pins
void teensyHWR::writeToLeds(){
	for(int i=ZERO;i<NUMBER_OF_LEDS;i++){
			digitalWrite(ledPin[i], bitRead(ledStateHash,i));  // why inverted?
	}
}

// set state of led
void teensyHWR::setLed(uint8_t _LED,boolean _STATE){

	bitWrite(ledStateHash,_LED,_STATE);

}

//############# KNOB RELATED FUNCTIONS #############



//update values and hashes of knobs
void teensyHWR::setMux(uint8_t bits){
	for(int i=0;i<3;i++){
		digitalWrite(muxPin[i],bitRead(bits,i));
	}
}
void teensyHWR::updateKnobs(){

 knobChangedHash = ZERO;
  for (int i = ZERO; i < NUMBER_OF_KNOBS; i++) {
	setMux(knobPin[i]);
	//delay(1);
    short newValue = analogRead(MUX_PIN); // why inverted?? 1023 - analogRead(knobPinsR[i])
    short distance = abs(newValue - knobValues[i]);


   	if (abs(newValue - knobValues[i]) > KNOB_TOLERANCE){
	bitWrite(knobChangedHash, i, true);
	}
   
	
	// launch callback
	#if USE_CALLBACKS
		if(mKnobChangedCallback!=NULL)
			if(bitRead(knobChangedHash, i))
				mKnobChangedCallback(i,newValue);
	#endif // USE_CALLBACKS

    lastKnobValues[i]=knobValues[i];
    knobValues[i] = newValue;
  }

}




void teensyHWR::setKnobTolerance(uint16_t _tolerance){
	KNOB_TOLERANCE=_tolerance;
}

//returns the freezing state of knob
boolean teensyHWR::knobFreezed(uint8_t _KNOB){
	return bitRead(knobFreezedHash,_KNOB);
}

//returns the freezing state of knob
boolean teensyHWR::knobMoved(uint8_t _KNOB){
	return bitRead(knobChangedHash,_KNOB);
}

// freeze all knobs
void teensyHWR::freezeAllKnobs(){
	for(int i=ZERO;i<NUMBER_OF_KNOBS;i++){
		bitWrite(knobFreezedHash,i,true);
	}
}

// unfreeze all knobs
void teensyHWR::unfreezeAllKnobs(){
	for(int i=ZERO;i<NUMBER_OF_KNOBS;i++){
		bitWrite(knobFreezedHash,i,false);
	}
}


// freeze one knob
void teensyHWR::freezeKnob(uint8_t _KNOB){
	bitWrite(knobFreezedHash,_KNOB,true);
}

// unfreeze one knob
void teensyHWR::unfreezeKnob(uint8_t _KNOB){
		bitWrite(knobFreezedHash,_KNOB,false);
}


// get knob value
uint16_t teensyHWR::knobValue(uint8_t _KNOB){

	return knobValues[_KNOB];

}

// get last knob value
uint16_t teensyHWR::lastKnobValue(uint8_t _KNOB){

	return lastKnobValues[_KNOB];

}

//############# BUTTON RELATED FUNCTIONS #############



// updates all button related hashes
void teensyHWR::updateButtons(){

	boolean state;

	for(int i=ZERO;i<3;i++){ // first read the buttons and update button states
	
		// and now update all the other hashes
		bitWrite(lastButtonStateHash,i,bitRead(buttonStateHash,i));
		state = !digitalRead(buttonPin[i]);
		bitWrite(buttonStateHash,i,state);
		bitWrite(justPressedHash,i,false);
		bitWrite(justReleasedHash,i,false);
		if(bitRead(buttonStateHash,i)==true && bitRead(lastButtonStateHash,i)==false)  bitWrite(justPressedHash,i,true);
		if(bitRead(buttonStateHash,i)==false && bitRead(lastButtonStateHash,i)==true)  bitWrite(justReleasedHash,i,true);
		//bitWrite(lastButtonStateHash,i,bitRead(buttonStateHash,i));
	}
	lastSwitchHash=switchHash;
	if(bitRead(buttonStateHash,1) && !bitRead(buttonStateHash,2)) switchHash=0;
	if(!bitRead(buttonStateHash,1) && !bitRead(buttonStateHash,2)) switchHash=1;
	if(!bitRead(buttonStateHash,1) && bitRead(buttonStateHash,2)) switchHash=2;
}



//returns if button state changed
boolean teensyHWR::buttonChanged(uint8_t _BUTTON){
	return bitRead(buttonChangedHash,_BUTTON);
}

//returns current state of a button
boolean teensyHWR::buttonState(uint8_t _BUTTON){
	bitWrite(buttonChangedHash,_BUTTON,false);
	return bitRead(buttonStateHash,_BUTTON);
}

//returns true if the button was just pressed
boolean teensyHWR::justPressed(uint8_t _BUTTON){
	bitWrite(buttonChangedHash,_BUTTON,false);
	return bitRead(justPressedHash,_BUTTON);

}

//returns true if the button was just released
boolean teensyHWR::justReleased(uint8_t _BUTTON){
	bitWrite(buttonChangedHash,_BUTTON,false);
	return bitRead(justReleasedHash,_BUTTON);
}



//returns switch state
uint8_t teensyHWR::switchState(){
	return switchHash;
}
uint8_t teensyHWR::lastSwitchState(){
	return lastSwitchHash;
}
bool teensyHWR::switchChanged(){
	if(switchHash!=lastSwitchHash) return true;
	else return false;
	
}


#if USE_CALLBACKS
void teensyHWR::setHandleButtonChange(void (*fptr)(uint8_t button, boolean state)){
	mButtonChangedCallback=fptr;
}
void teensyHWR::setHandleKnobChange(void (*fptr)(uint8_t knob, uint16_t value)){
	mKnobChangedCallback=fptr;
}

void teensyHWR::setHandleCVChange(void (*fptr)(uint8_t cv, uint16_t value)){
	mCVChangedCallback=fptr;
}
#endif // USE_CALLBACKS

// CV Ins

//update values and hashes of knobs
void teensyHWR::updateCVs(){

 CVChangedHash = ZERO;
  for (int i = ZERO; i < 4; i++) {
	setMux(cvPin[i]);
	//delay(1);
    short newValue = analogRead(MUX_PIN); // why inverted?? 1023 - analogRead(knobPinsR[i])
    short distance = abs(newValue - CVValues[i]);


   	if (abs(newValue - CVValues[i]) > KNOB_TOLERANCE){
	bitWrite(CVChangedHash, i, true);
	}
	
	// launch callback
	#if USE_CALLBACKS
		if(mCVChangedCallback!=NULL)
			if(bitRead(CVChangedHash, i))
				mCVChangedCallback(i,newValue);
	#endif // USE_CALLBACKS

    lastCVValues[i]=CVValues[i];
    CVValues[i] = newValue;
  }
	if(readCV_E){
	 lastCVValues[4]=CVValues[4];
    CVValues[4] = analogRead(CV_PIN_E);
	}
}

bool teensyHWR::isConnected(uint8_t _CV){
	
}
void teensyHWR::readCVE(bool yesNo){
	readCV_E=yesNo;
}

void teensyHWR::setSquareOut(bool highLow){
	digitalWrite(SQUARE_OUT_PIN,highLow);
}
boolean teensyHWR::cvMoved(uint8_t _CVIN){
	return bitRead(CVChangedHash,_CVIN);
}

// get cv value
uint16_t teensyHWR::cvValue(uint8_t _CVIN){

	return CVValues[_CVIN];

}


