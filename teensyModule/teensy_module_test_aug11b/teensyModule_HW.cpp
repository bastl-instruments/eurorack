/*
teensyModule hardware library
this library is to be used with Bastl Instruments eurorack hardware and Teensy 3.1
MOZZI library compatible
documentation of the library you find here:

Created by Tomash Ghzegowsky 2015 http://tomashg.com/
for Bastl Instruments http://www.bastl-instruments.com/
*/

#include "teensyModule_HW.h"

uint8_t buttonPinsR[NUMBER_OF_BUTTONS]  = {
							BUTTON_A_PIN,
							SWITCH_PINA
};


uint8_t ledPinsR[NUMBER_OF_LEDS]  = {
							LED_A_PIN,
							LED_B_PIN,
							LED_C_PIN,
							LED_D_PIN,
							LED_E_PIN
};

uint8_t cvInPinsR[NUMBER_OF_CVIN]  = {
							CVIN_A_PIN,
							CVIN_B_PIN,
							CVIN_C_PIN,
							CVIN_D_PIN,
							CVIN_E_PIN
};

uint8_t knobPinsR[NUMBER_OF_KNOBS] = {  
							KNOB_A_PIN,
							KNOB_B_PIN,
							KNOB_C_PIN,
							KNOB_D_PIN
};

uint8_t cvOutPinsR[NUMBER_OF_CVOUT]  = {
							CVOUT_PIN
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
	for(int i=0;i<NUMBER_OF_BUTTONS;i++) pinMode(buttonPinsR[i], INPUT_PULLUP);
	for(int i=0;i<NUMBER_OF_LEDS;i++) pinMode(ledPinsR[i], OUTPUT);
	freezeAllKnobs();
	update();
}

void teensyHWR::initializeMozzi(){
}

void teensyHWR::update(){
	updateKnobs();
	updateButtons();
	updateCVs();
	writeToLeds();
}

void teensyHWR::mozziUpdate(){
	updateMozziKnobs();
	updateMozziButtons();
	writeToLeds();
}

//############# LED RELATED FUNCTIONS #############

// write the values from the hash to the pins
void teensyHWR::writeToLeds(){
	for(int i=ZERO;i<NUMBER_OF_LEDS;i++){
			digitalWrite(ledPinsR[i], bitRead(ledStateHash,i));  // why inverted?
	}
}

// set state of led
void teensyHWR::setLed(uint8_t _LED,boolean _STATE){

	bitWrite(ledStateHash,_LED,_STATE);

}

//############# KNOB RELATED FUNCTIONS #############

void teensyHWR::setFreezeType(uint8_t _TYPE){

	if(_TYPE==DEFAULT) unfreezeExternaly=false;
	else if(_TYPE==UNFREEZE_EXTERNALY) unfreezeExternaly=true;

}

//update values and hashes of knobs
void teensyHWR::updateKnobs(){

 knobChangedHash = ZERO;
  for (int i = ZERO; i < NUMBER_OF_KNOBS; i++) {

    short newValue = analogRead(knobPinsR[i]); // why inverted?? 1023 - analogRead(knobPinsR[i])
    short distance = abs(newValue - knobValues[i]);


    if(!unfreezeExternaly){
    	if (bitRead(knobFreezedHash, i) == true) {
    	  if (distance > KNOB_FREEZE_DISTANCE) {
     	   bitWrite(knobFreezedHash, i, false);
     	   bitWrite(knobChangedHash, i, true);
    	  }
  	  }
    }

   	if (abs(newValue - knobValues[i]) > KNOB_TOLERANCE){
	bitWrite(knobChangedHash, i, true);
	activity=ZERO;
	}
    else {

    if(activity>ACTIVITY_LIMIT) bitWrite(knobChangedHash, i, false);
    else activity++;

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


//update values and hashes of knobs
void teensyHWR::updateMozziKnobs(){

 knobChangedHash = ZERO;
  for (int i = ZERO; i < NUMBER_OF_KNOBS; i++) {

    short newValue = analogRead(knobPinsR[i]); //mozziAnalogRead
    short distance = abs(newValue - knobValues[i]);


    if(!unfreezeExternaly){
    	if (bitRead(knobFreezedHash, i) == true) {
    	  if (distance > KNOB_FREEZE_DISTANCE) {
     	   bitWrite(knobFreezedHash, i, false);
     	   bitWrite(knobChangedHash, i, true);
    	  }
  	  }
    }

   	if (abs(newValue - knobValues[i]) > KNOB_TOLERANCE) bitWrite(knobChangedHash, i, true), activity=ZERO;
    else {

    if(activity>ACTIVITY_LIMIT) bitWrite(knobChangedHash, i, false);
    else activity++;

    }

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

boolean teensyHWR::mozziDigitalRead(uint8_t _pin){
   if(_pin>13) return analogRead(_pin)>>9; //mozziAnalogRead
   else
	   return digitalRead(_pin);
}

// updates all button related hashes
void teensyHWR::updateButtons(){

	boolean state;

	for(int i=ZERO;i<NUMBER_OF_BUTTONS;i++){ // first read the buttons and update button states

		state = !digitalRead(buttonPinsR[i]);
		
		if(state!=bitRead(lastButtonStateHash,i)) {
			if((millis()-buttonBounceTime[i])>50){
				buttonBounceTime[i]=millis();
				bitWrite(buttonStateHash,i,state);
				bitWrite(buttonChangedHash,i,true);
				bitWrite(lastButtonStateHash,i,state);
				
				// launch callback
				#if USE_CALLBACKS
					if(mButtonChangedCallback!=NULL)
						mButtonChangedCallback(i,state);
				#endif // USE_CALLBACKS
			}
			else {
				bitWrite(buttonChangedHash,i,false);
			}
		}
			
		
		// and now update all the other hashes
		bitWrite(justPressedHash,i,false);
		bitWrite(justReleasedHash,i,false);
		if(bitRead(buttonStateHash,i)==true && bitRead(lastButtonStateHash,i)==false)  bitWrite(justPressedHash,i,true);
		if(bitRead(buttonStateHash,i)==false && bitRead(lastButtonStateHash,i)==true)  bitWrite(justReleasedHash,i,true);
		//bitWrite(lastButtonStateHash,i,bitRead(buttonStateHash,i));
	}

}

void teensyHWR::updateMozziButtons(){

	for(int i=ZERO;i<NUMBER_OF_BUTTONS;i++){ // first read the buttons and update button states

		//	pinMode(pgm_read_word_near(buttonPinsR + i), INPUT_PULLUP);
		bitWrite(buttonStateHash,i,!mozziDigitalRead(buttonPinsR[i]));

		// and now update all the other hashes
	 	bitWrite(justPressedHash,i,false);
		bitWrite(justReleasedHash,i,false);
    	if(bitRead(buttonStateHash,i)==true && bitRead(lastButtonStateHash,i)==false)  bitWrite(justPressedHash,i,true);
    	if(bitRead(buttonStateHash,i)==false && bitRead(lastButtonStateHash,i)==true)  bitWrite(justReleasedHash,i,true);
		bitWrite(lastButtonStateHash,i,bitRead(buttonStateHash,i));
	}

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



//flips the software switch
void teensyHWR::flipSwitch(uint8_t _SWITCH){
	bitWrite(switchStateHash,_SWITCH,!bitRead(switchStateHash,_SWITCH));
}

// sets switch state
void teensyHWR::setSwitch(uint8_t _SWITCH, boolean _STATE){
	bitWrite(switchStateHash,_SWITCH,_STATE);
}

//returns switch state
boolean teensyHWR::switchState(uint8_t _SWITCH){
	return bitRead(switchStateHash,_SWITCH);
}

//resetsSwitches
void teensyHWR::resetSwitches(){
	for(int i=ZERO;i<NUMBER_OF_BUTTONS;i++){
		bitWrite(switchStateHash,i,false);
	}
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
  for (int i = ZERO; i < NUMBER_OF_CVIN; i++) {

    short newValue = analogRead(cvInPinsR[i]); // why inverted?? 1023 - analogRead(knobPinsR[i])
    short distance = abs(newValue - CVValues[i]);


   	if (abs(newValue - CVValues[i]) > KNOB_TOLERANCE){
	bitWrite(CVChangedHash, i, true);
	activity=ZERO;
	}
    else {

    if(activity>ACTIVITY_LIMIT) bitWrite(CVChangedHash, i, false);
    else activity++;
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

}

boolean teensyHWR::CVMoved(uint8_t _CVIN){
	return bitRead(CVChangedHash,_CVIN);
}

// get cv value
uint16_t teensyHWR::CVValue(uint8_t _CVIN){

	return CVValues[_CVIN];

}
