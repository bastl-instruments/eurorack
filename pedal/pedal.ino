

#ifdef EXTERNAL_IDE
#include <Arduino.h>
#include <shiftRegisterFast.h>
#include <avr/pgmspace.h>

int main(void) {

  init();

  setup();

  while(true) {
    loop();
  }
}

#endif

#include <EEPROM.h>



#define LED_1_PIN C,5
#define LED_2_PIN C,4

#define MUX_1_PIN D,4
#define MUX_2_PIN D,5

#define OUT_LED_1_PIN D,7
#define OUT_LED_2_PIN D,3

#define SWITCH_1_PIN C,1
#define SWITCH_2_PIN C,0

#define BUTTON_1_PIN C,3
#define BUTTON_2_PIN C,2

#define GATE_1_PIN B,5
#define GATE_2_PIN B,4

#define OUT_BUT_1_PIN D,6
#define OUT_BUT_2_PIN D,2

const uint8_t input[4]={
  8,9,10,11};
const uint8_t output[4]={
  4,7,6,5};
bool triggerState[4];
#define NUMBER_OF_CHANNELS 4

bool switchState[2];
bool butState[2];
bool ledState[2];
bool outLedState[2];
bool outButState[2];
bool gateState[2];
bool muxState[2];
void fastRoutine(){
	gateState[0]=!bit_read_in(GATE_1_PIN);
			gateState[1]=!bit_read_in(GATE_2_PIN);

			if(gateState[0]){
				if(muxState[0]){
					bit_set(LED_1_PIN);
					bit_set(OUT_LED_1_PIN);
					bit_set(MUX_1_PIN);
				}
				else{
					bit_clear(LED_1_PIN);
					bit_clear(OUT_LED_1_PIN);
					bit_clear(MUX_1_PIN);
				}
			}
			else{
				if(!muxState[0]){
					bit_set(LED_1_PIN);
					bit_set(OUT_LED_1_PIN);
					bit_set(MUX_1_PIN);
				}
				else{
					bit_clear(LED_1_PIN);
					bit_clear(OUT_LED_1_PIN);
					bit_clear(MUX_1_PIN);
				}
			}


			if(gateState[1]){
					if(muxState[1]){
						bit_set(LED_2_PIN);
						bit_set(OUT_LED_2_PIN);
						bit_set(MUX_2_PIN);
					}
					else{
						bit_clear(LED_2_PIN);
						bit_clear(OUT_LED_2_PIN);
						bit_clear(MUX_2_PIN);
					}
				}
				else{
					if(!muxState[1]){
						bit_set(LED_2_PIN);
						bit_set(OUT_LED_2_PIN);
						bit_set(MUX_2_PIN);
					}
					else{
						bit_clear(LED_2_PIN);
						bit_clear(OUT_LED_2_PIN);
						bit_clear(MUX_2_PIN);
					}
				}
}

void routine(){
	switchState[0]=!bit_read_in(SWITCH_1_PIN);
		switchState[1]=!bit_read_in(SWITCH_2_PIN);
		bool newState;

		if(switchState[0]){
			newState=!bit_read_in(BUTTON_1_PIN);
			if(!butState[0] && newState) muxState[0]=!muxState[0];
			butState[0]=newState;

			newState=!bit_read_in(OUT_BUT_1_PIN);
			if(!outButState[0] && newState) muxState[0]=!muxState[0];
			outButState[0]=newState;
		}
		else{
			butState[0]=!bit_read_in(BUTTON_1_PIN);
			outButState[0]=!bit_read_in(OUT_BUT_1_PIN);
			if(butState[0]==true || outButState[0]==true) muxState[0]=false;
			else muxState[0]=true;
		}

		if(switchState[1]){
			newState=!bit_read_in(BUTTON_2_PIN);
			if(!butState[1] && newState) muxState[1]=!muxState[1];
			butState[1]=newState;

			newState=!bit_read_in(OUT_BUT_2_PIN);
			if(!outButState[1] && newState) muxState[1]=!muxState[1];
			outButState[1]=newState;
		}
		else{
			butState[1]=!bit_read_in(BUTTON_2_PIN);
			outButState[1]=!bit_read_in(OUT_BUT_2_PIN);
			if(butState[1]==true || outButState[1]==true) muxState[1]=false;
			else muxState[1]=true;
		}
		fastRoutine();
}


void setup(){

	bit_dir_inp(SWITCH_1_PIN);
	bit_dir_inp(SWITCH_2_PIN);

	bit_dir_inp(BUTTON_1_PIN);
	bit_dir_inp(BUTTON_2_PIN);

	bit_dir_inp(GATE_1_PIN);
	bit_dir_inp(GATE_2_PIN);

	bit_dir_inp(OUT_BUT_1_PIN);
	bit_dir_inp(OUT_BUT_2_PIN);

	bit_set(SWITCH_1_PIN);
	bit_set(SWITCH_2_PIN);

	bit_set(BUTTON_1_PIN);
	bit_set(BUTTON_2_PIN);

	bit_set(GATE_1_PIN);
	bit_set(GATE_2_PIN);

	bit_set(OUT_BUT_1_PIN);
	bit_set(OUT_BUT_2_PIN);

	bit_dir_outp(LED_1_PIN);
	bit_dir_outp(LED_2_PIN);

	bit_dir_outp(MUX_1_PIN);
	bit_dir_outp(MUX_2_PIN);

	bit_dir_outp(OUT_LED_1_PIN);
	bit_dir_outp(OUT_LED_2_PIN);

	bit_clear(LED_1_PIN);
	bit_clear(LED_2_PIN);

	bit_clear(MUX_1_PIN);
	bit_clear(MUX_2_PIN);

	bit_clear(OUT_LED_1_PIN);
	bit_clear(OUT_LED_2_PIN);
	routine();
	switchState[0]=false;
	switchState[1]=false;


}
uint32_t _time;
void loop()
{

	routine();
	while(millis()-_time==0){
		fastRoutine();
	}
	_time=millis();

}





