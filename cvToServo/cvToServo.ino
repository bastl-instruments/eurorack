

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
#include <Servo.h>

Servo srv[2];  // create servo object to control a servo

int potpin = 0;  // analog pin used to connect the potentiometer
int val;    // variable to read the value from the analog pin
uint8_t analogPin[4]={1,3,0,2};
uint16_t analogValue[4];
#define DEBUG
void setup()
{
  srv[0].attach(3);  // attaches the servo on pin 9 to the servo object
  srv[1].attach(4);
#ifdef DEBUG
  Serial.begin(9600);
#endif
}
#define CV 0
#define POT 1
void test(){
	Serial.print("SA: ");
	uint16_t angle=map(analogRead(1),0,1024,0,180);
	srv[0].write(angle);
	Serial.print(angle);
	Serial.print("  SB: ");
	angle=map(analogRead(0),0,1024,0,180);
	srv[1].write(angle);
	Serial.print(angle);
	Serial.println();
}
void renderServo(uint8_t ch){
	uint16_t angle;
	angle=analogValue[ch*2+CV]-512+analogValue[ch*2+POT];
	angle=map(angle,0,1024,0,180);
	srv[ch].write(angle);
}
void loop()
{
#ifdef DEBUG
	test();
#else
	for(int i=0;i<4;i++){
		analogValue[i]=analogRead(analogPin[i]);
	}
	for(int i=0;i<2;i++){
		renderServo(i);
	}
#endif
	delay(1);
}



