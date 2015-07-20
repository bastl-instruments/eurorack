#ifdef EXTERNAL_IDE
#include <Arduino.h>
/*
 * trinity
 *
 */
int main(void) {

	init();
	setup();

	while (true) {
		loop();
	}
}

#endif

#include <Arduino.h>
#include <EEPROM.h>
#include <simpleSerialDecoder.h>
#include <trinityRack_HW.h>
#include <portManipulations.h>

trinityHWR hw;
simpleSerialDecoder com;

#define CLK_POINTS 5

uint16_t CLKMap[10]={0,63,127,191,255,  10,100,250,1000,2000};

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

uint8_t automationValue[6][32];
uint8_t currentStep = 0;

#define SHIFT 3
#define MODE 4

#define FN 6
#define CLEAR 5

#define SYNC 5
#define REC 5
bool shift;
uint8_t sound, lastSound;
uint8_t channelMode[6];
const uint8_t modeMap[3] = { 4, 5, 6 };
#define LFO_MODE 1
#define AUTOMATION_MODE 0
#define ADSR_MODE 2
uint8_t channelValue[6][4];
uint8_t channelSettings[6];

#define SETTINGS_BYTE 3

#define HOLD_BIT 0
#define LIN_LOG_BIT 1
#define LOOP_BIT 2
#define SYNC_BIT 3

#define CV_ASSING_0 4
#define CV_ASSING_1 5
#define CV_ASSING_2 6

#define LFO_SYNC_BIT 4
#define LFO_ODD_BIT 5
#define AUT_SYNC_BIT 6
#define AUT_ODD_BIT 7


uint16_t clockPeriod=100;

bool syncIndicate;
uint32_t syncIndicateTime;
#define SYNC_INDICATE_TIME 20
void indicateSync() {
	syncIndicateTime = millis();
	syncIndicate = true;
}

void processStepChange(uint8_t stepNumber) {
	currentStep = stepNumber;
	/*
	 if(currentStep<31) currentStep++;
	 else currentStep=0;
	 if(!hw.buttonState(REC)){
	 for(int i=0;i<6;i++){
	 if(channelMode[i]==AUTOMATION_MODE){


	 currentStep=currentStep%numberOfChannelSteps;
	 if(currentStep!=0) com.sendChannelInterpolateFrom(i,automationValue[i][currentStep-1]);
	 else com.sendChannelInterpolateFrom(i,automationValue[i][numberOfChannelSteps]);

	 com.sendChannelInterpolateTo(i,automationValue[i][currentStep]);
	 com.sendChannelInterpolate(i,channelValue[i][1]);
	 //com.sendChannelCV(i,automationValue[i][currentStep]);
	 }
	 }
	 }
	 */
	//if(channelSync)
	if (channelMode[sound] == AUTOMATION_MODE){
		if((currentStep%4) ==0) indicateSync();
	}
	else if(bitRead(channelValue[sound][SETTINGS_BYTE], LFO_SYNC_BIT)) indicateSync();
}

void clockCall() {
//	com.sendClock(1);

}

void gateCall(uint8_t channel, uint8_t number) {
	if (number == 1)
		hw.setLed(channel, true);
	else
		hw.setLed(channel, false);
}
void clockCall(uint8_t number) {

}
void stepCall(uint8_t number) {
	processStepChange(number);
}
void gateCall(uint8_t number) {

}
void triggerCall(uint8_t number) {

}
void startCall(uint8_t number) {

}
void stopCall(uint8_t number) {

}
void restartCall(uint8_t number) {

}
void channelTriggerCall(uint8_t channel, uint8_t number) {

}
void channelGateCall(uint8_t channel, uint8_t number) {

}
void channelCVCall(uint8_t channel, uint8_t number) {

}
void channelInterpolateFromCall(uint8_t channel, uint8_t number) {

}
void channelInterpolateToCall(uint8_t channel, uint8_t number) {

}
void channeInterpolateCall(uint8_t channel, uint8_t number) {

}
void channelModeCall(uint8_t channel, uint8_t number) {

}
void channelValueCall(uint8_t channel, uint8_t value, uint8_t number) {

}
void transferAll() {
	for (int i = 0; i < 6; i++) {
		com.sendChannelMode(i, modeMap[channelMode[i]]);
		com.sendPairMessage();
		for (int j = 0; j < 4; j++) {
			com.sendChannelValue(i, j, channelValue[i][j]);
		}
		com.sendPairMessage();
		for (int j = 0; j < 32; j++) {
			com.sendChannelInterpolateFrom(i, j);
			com.sendChannelInterpolateTo(i, automationValue[i][j]);
		}
		com.sendPairMessage();
		//com.sendChannelMode(i,modeMap[channelMode[i]]);
	}

	com.sendChannelMode(sound, modeMap[channelMode[sound]]);
}

#define PRESET_SIZE 255
#define MODE_OFFSET 24
#define AUTOMATION_OFFSET 30
uint8_t currentPreset;
const uint8_t clearTo[4] = { 200, 0, 0, 96 }; //defaultně zapnout AUT_SYNC_BIT=6
#define CLEAR_BYTE_0 1020
#define CLEAR_BYTE_1 1021
#define CLEAR_BYTE_2 1022
#define CLEAR_VALUE_0 7
#define CLEAR_VALUE_1 48
#define CLEAR_VALUE_2 99

void save(uint8_t preset) {
	uint16_t offset = preset * PRESET_SIZE;
	for (int i = 0; i < 6; i++) {
		for (int j = 0; j < 4; j++) {
			EEPROM.write(offset + (i * 4) + j, channelValue[i][j]);
		}
		EEPROM.write(offset + MODE_OFFSET + i, channelMode[i]);
		for (int j = 0; j < 32; j++) {
			EEPROM.write(offset + AUTOMATION_OFFSET + (i * 32) + j,
					automationValue[i][j]);
		}
	}
}

void load(uint8_t preset) {
	currentPreset = preset;
	uint16_t offset = preset * PRESET_SIZE;
	for (int i = 0; i < 6; i++) {
		for (int j = 0; j < 4; j++) {
			channelValue[i][j] = EEPROM.read(offset + (i * 4) + j);
		}
		channelMode[i] = EEPROM.read(offset + MODE_OFFSET + i);
		for (int j = 0; j < 32; j++) {
			automationValue[i][j] = EEPROM.read(
					offset + AUTOMATION_OFFSET + (i * 32) + j);
		}
	}
	hw.freezeAllKnobs();
}

void clearMemory() {

	EEPROM.write(CLEAR_BYTE_0, CLEAR_VALUE_0);
	EEPROM.write(CLEAR_BYTE_1, CLEAR_VALUE_1);
	EEPROM.write(CLEAR_BYTE_2, CLEAR_VALUE_2);

	for (int _preset = 0; _preset < 3; _preset++) {
		load(_preset);

		for (int i = 0; i < 6; i++) {
			for (int j = 0; j < 4; j++) {
				channelValue[i][j] = clearTo[j];
			}
			channelMode[i] = 1;
			for (int j = 0; j < 32; j++) {
				automationValue[i][j] = 0;
			}
		}
		save(_preset);
	}

}
void shouldIClearMemory() {
	if (hw.buttonState(0) && hw.buttonState(1) && hw.buttonState(2))
		clearMemory();
	if ((EEPROM.read(CLEAR_BYTE_0) != CLEAR_VALUE_0)
			|| (EEPROM.read(CLEAR_BYTE_1) != CLEAR_VALUE_1)
			|| (EEPROM.read(CLEAR_BYTE_2) != CLEAR_VALUE_2))
		clearMemory();
}

void setup() {
	com.init(38400);
	com.attachClockCallback(&clockCall);
	com.attachStepCallback(&stepCall);
	com.attachGateCallback(&gateCall);
	com.attachTriggerCallback(&triggerCall);
	com.attachStartCallback(&startCall);
	com.attachStopCallback(&stopCall);
	com.attachRestartCallback(&restartCall);
	com.attachChannelTriggerCallback(&channelTriggerCall);
	com.attachChannelGateCallback(&channelGateCall);
	com.attachChannelCVCallback(&channelCVCall);
	com.attachChannelInterpolateFromCallback(&channelInterpolateFromCall);
	com.attachChannelInterpolateToCallback(&channelInterpolateToCall);
	com.attachChannelInterpolateCallback(&channeInterpolateCall);
	com.attachChannelModeCallback(&channelModeCall);
	com.attachChannelValueCallback(&channelValueCall);
	hw.initialize(0);
	digitalWrite(9, OUTPUT);
	digitalWrite(10, OUTPUT);
	hw.update();
	hw.update();
	hw.update();
	hw.update();
	hw.freezeAllKnobs();
	shouldIClearMemory();
	load(0);
	delay(200);
	com.sendPairMessage();
	com.sendSelect(sound);
	while (!Serial.available()) {
		com.update();
		delay(10);
		com.sendPairMessage();
		com.sendSelect(sound);
	}
	transferAll();
	hw.freezeAllKnobs();
}
void renderBigButtons() {
	lastSound = sound;
	for (int i = 0; i < 3; i++) {
		if (hw.justPressed(i)) {
			sound = shift * 3 + i;
		}
	}

	if (hw.justPressed(SHIFT)){
		hw.flipSwitch(SHIFT);
		shift = hw.switchState(SHIFT);
		if(!shift) sound = sound % 3;
		else if(sound<3) sound = sound + 3;
	}
	shift = hw.switchState(SHIFT);

	if (sound != lastSound) hw.freezeAllKnobs(), com.sendSelect(sound), com.sendPairMessage();
}

void renderLeds() {
	if (hw.buttonState(FN)) {
		switch (channelMode[sound]) {
		case ADSR_MODE:
			hw.setLed(0, bitRead(channelValue[sound][SETTINGS_BYTE], HOLD_BIT));
			hw.setLed(1, bitRead(channelValue[sound][SETTINGS_BYTE], LOOP_BIT));
			hw.setLed(2, bitRead(channelValue[sound][SETTINGS_BYTE], LIN_LOG_BIT));
			break;
		case AUTOMATION_MODE:
			hw.setLed(0, bitRead(channelValue[sound][SETTINGS_BYTE], AUT_SYNC_BIT));
			hw.setLed(1,hw.buttonState(1));
			hw.setLed(2, bitRead(channelValue[sound][SETTINGS_BYTE], AUT_ODD_BIT));
			break;
		case LFO_MODE:
			hw.setLed(0, bitRead(channelValue[sound][SETTINGS_BYTE], LFO_SYNC_BIT));
			if(bitRead(channelValue[sound][SETTINGS_BYTE], SYNC_BIT)) hw.setLed(1,true), hw.setLed(0,false);
			else hw.setLed(1,false);
			hw.setLed(2, bitRead(channelValue[sound][SETTINGS_BYTE], LFO_ODD_BIT));
			break;
		}

	} else {
		for (int i = 0; i < 3; i++) {
			if (sound % 3 == i ) hw.setLed(i, true);
			else hw.setLed(i, false);
		}
		//if (sound >= 3)
			///hw.setLed(sound - 3, false);
		//else
		//	hw.setLed(sound, true);
	}
}

bool combo = false;
void renderCombo() {

	if(hw.buttonState(FN)){
		if (hw.justPressed(REC)) save(currentPreset), combo=true;
		if (hw.justPressed(MODE)){
			com.sendTrigger(sound);
			combo=true;
		}
	}

	if (hw.buttonState(MODE)) {
		for (int i = 0; i < 3; i++) {
			if (hw.justPressed(i)) {
				combo = true;
				save(currentPreset);
				load(i);
				transferAll();
				hw.freezeAllKnobs();
			}
		}

	}

	if (combo) {
		//turn off combo when all buttons are released
		unsigned char _count = 0;
		for (int i = 0; i < 7; i++) _count += hw.buttonState(i);
		if (_count == 0) combo = false;
	}
}

bool inBetween(int val1, int val2, int inBet) {
	bool retVal;
	if (val1 >= val2) {
		if (inBet <= val1 && inBet >= val2)
			retVal = true;
		else
			retVal = false;
	} else if (val1 < val2) {
		if (inBet >= val1 && inBet <= val2)
			retVal = true;
		else
			retVal = false;
	}
	return retVal;

}

void renderKnobs() {
	if (hw.buttonState(FN)) {
		if (hw.knobMoved(0))
			hw.unfreezeKnob(0);
		if (!hw.knobFreezed(0))
			//clockPeriod = (1024 - hw.knobValue(0)) << 1;
			clockPeriod=curveMap((255-(hw.knobValue(0)>>2)),CLK_POINTS,CLKMap);

	} else {
		for (int i = 0; i < 3; i++) {
			if (hw.knobFreezed(i)) {
				if (hw.knobMoved(i)
						|| inBetween(hw.knobValue(i) >> 2,
								hw.lastKnobValue(i) >> 2,
								channelValue[sound][i]))
					hw.unfreezeKnob(i);
			} else {
				channelValue[sound][i] = hw.knobValue(i) >> 2;
				if (hw.knobValue(i) >> 2 != hw.lastKnobValue(i) >> 2) {
					com.sendChannelValue(sound, i, hw.knobValue(i) >> 2);
				}
			}
		}
	}
}

void renderModeSpecificInteraction() {


	switch (channelMode[sound]) {
	case AUTOMATION_MODE:

		if (hw.buttonState(REC)) {
			automationValue[sound][currentStep] = hw.knobValue(0) >> 2;
			com.sendChannelInterpolate(sound,automationValue[sound][currentStep]);
			//com.sendChannelCV(sound,hw.knobValue(0)>>2);
			//	com.sendChannelInterpolateFrom(sound,currentStep);
			//	com.sendChannelInterpolateTo(sound,automationValue[sound][currentStep]);
			//Serial.println(automationValue[sound][currentStep]);
		}
		if (hw.justReleased(REC)) com.sendChannelInterpolateFrom(sound, currentStep);


		if (hw.buttonState(FN)) {
			if (hw.justPressed(0)) {
				bitWrite(channelValue[sound][SETTINGS_BYTE], AUT_SYNC_BIT,!bitRead(channelValue[sound][SETTINGS_BYTE],AUT_SYNC_BIT));
				com.sendChannelValue(sound, SETTINGS_BYTE,channelValue[sound][SETTINGS_BYTE]);
			}
			if (hw.justPressed(1)) {
				for(int i=0;i<32;i++){
					automationValue[sound][i] = 0;
					com.sendChannelInterpolateFrom(sound, i);
					com.sendChannelInterpolateTo(sound, automationValue[sound][i]);
				}
			}
			if (hw.justPressed(2)) {
				bitWrite(channelValue[sound][SETTINGS_BYTE], AUT_ODD_BIT,!bitRead(channelValue[sound][SETTINGS_BYTE],AUT_ODD_BIT));
				com.sendChannelValue(sound, SETTINGS_BYTE,channelValue[sound][SETTINGS_BYTE]);
			}
		}



		break;

	case LFO_MODE:

		if (hw.buttonState(FN)) {
			if (hw.justPressed(0)) {
				bitWrite(channelValue[sound][SETTINGS_BYTE], LFO_SYNC_BIT,!bitRead(channelValue[sound][SETTINGS_BYTE],LFO_SYNC_BIT));
				com.sendChannelValue(sound, SETTINGS_BYTE,channelValue[sound][SETTINGS_BYTE]);
			}
			if (hw.justPressed(1)) {
				//tap here
				bitWrite(channelValue[sound][SETTINGS_BYTE], SYNC_BIT,!bitRead(channelValue[sound][SETTINGS_BYTE],SYNC_BIT));
				com.sendChannelValue(sound, SETTINGS_BYTE,channelValue[sound][SETTINGS_BYTE]);
			}
			if (hw.justPressed(2)) {
				bitWrite(channelValue[sound][SETTINGS_BYTE], LFO_ODD_BIT,!bitRead(channelValue[sound][SETTINGS_BYTE],LFO_ODD_BIT));
				com.sendChannelValue(sound, SETTINGS_BYTE,channelValue[sound][SETTINGS_BYTE]);
			}
		}
		break;
	case ADSR_MODE:

		/*
		if (hw.justPressed(SYNC)) {
			bitWrite(channelValue[sound][SETTINGS_BYTE], SYNC_BIT,!bitRead(channelValue[sound][SETTINGS_BYTE],SYNC_BIT));
			com.sendChannelValue(sound, SETTINGS_BYTE,channelValue[sound][SETTINGS_BYTE]);
		}
		*/
		if (hw.buttonState(FN)) {
			if (hw.justPressed(0)) {
				bitWrite(channelValue[sound][SETTINGS_BYTE], HOLD_BIT,!bitRead(channelValue[sound][SETTINGS_BYTE],HOLD_BIT));
				com.sendChannelValue(sound, SETTINGS_BYTE,channelValue[sound][SETTINGS_BYTE]);
			}
			if (hw.justPressed(1)) {
				bitWrite(channelValue[sound][SETTINGS_BYTE], LOOP_BIT,!bitRead(channelValue[sound][SETTINGS_BYTE],LOOP_BIT));
				com.sendChannelValue(sound, SETTINGS_BYTE,channelValue[sound][SETTINGS_BYTE]);
			}
			if (hw.justPressed(2)) {
				bitWrite(channelValue[sound][SETTINGS_BYTE], LIN_LOG_BIT,!bitRead(channelValue[sound][SETTINGS_BYTE],LIN_LOG_BIT));
				com.sendChannelValue(sound, SETTINGS_BYTE,channelValue[sound][SETTINGS_BYTE]);
			}

		}

		break;
	}
}
bool clockState;
long lastClockTime;

#define CLOCK_LENGTH 10

void renderClockGenerator() {
	if (clockState) {
		if (millis() - lastClockTime >= CLOCK_LENGTH) {
			clockState = false;
			lastClockTime = millis();
			digitalWrite(9, LOW);
			digitalWrite(10, LOW);
		}
	} else {
		if (millis() - lastClockTime >= clockPeriod) {
			clockState = true;
			lastClockTime = millis();
			digitalWrite(9, HIGH);
			digitalWrite(10, HIGH);
		}
	}
}

void renderRGB() {
	if (syncIndicate) {
		if (millis() - syncIndicateTime >= SYNC_INDICATE_TIME) {
			syncIndicate = false;
		}
		hw.setColor(WHITE);
	}
	else {
		hw.setColor(channelMode[sound] + 1);
	}
}
void loop() {

	com.update();
	hw.update();
	delay(1);
	if (hw.justPressed(FN) || hw.justReleased(FN)) hw.freezeAllKnobs();
	renderKnobs();

	if (hw.justReleased(MODE) && !combo) {
		if (channelMode[sound] < 2) channelMode[sound]++;
		else channelMode[sound] = 0;
		com.sendChannelMode(sound, modeMap[channelMode[sound]]);
		// set to Default??

	}

	renderRGB();
	renderCombo();
	if (!combo && !hw.buttonState(FN) && !hw.buttonState(MODE)) {
		renderBigButtons();
	}

	renderLeds();
	renderModeSpecificInteraction();
	renderClockGenerator();

}

