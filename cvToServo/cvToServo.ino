

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

#include <EEPROM.h>
#include <Servo.h>


Servo srv[2];  // create servo object to control a servo

int potpin = 0;  // analog pin used to connect the potentiometer
int val;    // variable to read the value from the analog pin
const uint8_t analogPin[6]={1,2,3,0,4,5};
uint16_t analogValue[6];
const uint8_t gatePin[2]={8,9};
//#define DEBUG


#define MEM_OFFSET 34
uint16_t channelCenter[2]={512,512};
#define CENTER_1_PIN 12
#define CENTER_2_PIN 13
#define SIGNAL_PIN 13
void calibrate(){
	 if(!digitalRead(CENTER_1_PIN)){// && !digitalRead(CENTER_2_PIN)) {
		  channelCenter[0]=analogRead(analogPin[0]);
		  channelCenter[1]=analogRead(analogPin[3]);

		  EEPROM.write(MEM_OFFSET,lowByte(channelCenter[0]));
		  EEPROM.write(MEM_OFFSET+1,highByte(channelCenter[0]));

		  EEPROM.write(MEM_OFFSET+2,lowByte(channelCenter[1]));
		  EEPROM.write(MEM_OFFSET+1+2,highByte(channelCenter[1]));

		  for(int i=0;i<8;i++){
			  digitalWrite(SIGNAL_PIN,HIGH);
			  delay(200);
			  digitalWrite(SIGNAL_PIN,LOW);
			  delay(200);
		  }
	  }
}

void initCalibrate(){
	if(EEPROM.read(MEM_OFFSET+10+1)!=13 || EEPROM.read(MEM_OFFSET+10+2)!=145 || EEPROM.read(MEM_OFFSET+10+3)!=109){
		EEPROM.write(MEM_OFFSET+10+1,13);
		EEPROM.write(MEM_OFFSET+10+2,145);
		EEPROM.write(MEM_OFFSET+10+3,109);

		EEPROM.write(MEM_OFFSET,lowByte(channelCenter[0]));
		EEPROM.write(MEM_OFFSET+1,highByte(channelCenter[0]));

		EEPROM.write(MEM_OFFSET+2,lowByte(channelCenter[1]));
		EEPROM.write(MEM_OFFSET+1+2,highByte(channelCenter[1]));
	}

	channelCenter[0]=word(EEPROM.read(MEM_OFFSET+1),EEPROM.read(MEM_OFFSET));
	channelCenter[1]=word(EEPROM.read(MEM_OFFSET+1+2),EEPROM.read(MEM_OFFSET+2));

}
//#define DEBUG

void setup()
{
	initCalibrate();
	pinMode(3,OUTPUT);
	pinMode(4,OUTPUT);
	pinMode(8,INPUT_PULLUP);
	pinMode(9,INPUT_PULLUP);
  srv[0].attach(3);  // attaches the servo on pin 9 to the servo object
  srv[1].attach(4);
#ifdef DEBUG
  Serial.begin(9600);
#endif
  /*
  Serial.begin(9600);
  Serial.println(channelCenter[0]);
  Serial.println(channelCenter[1]);
  */
}
#define CV 0
#define ATT 1
#define POT 2
void test(){
	Serial.print("SA: ");
	uint16_t angle=map(analogValue[1],0,1024,0,180);
	srv[0].write(angle);
	Serial.print(" 1: ");
	Serial.print(analogValue[1]);
	Serial.print(" 0: ");
	Serial.print(analogValue[0]);
	Serial.print(" 2: ");
	Serial.print(analogValue[2]);
	Serial.println();
	/*
	Serial.print("  SB: ");
	angle=map(analogRead(0),0,1024,0,180);
	srv[1].write(angle);
	Serial.print(angle);
	Serial.println();
	*/
}
bool gate[2];
void renderServo(uint8_t ch){
	int16_t angle;
	int16_t att;
	if(gate[ch]){
		angle=analogValue[ch*3+ATT];
		angle=map(angle,0,1024,0,180);
		angle=constrain(angle,1,179);
	}
	else{
		att=map(analogValue[ch*3+ATT] ,0,1024,0,360);

		angle=map(analogValue[ch*3+POT] ,0,1024,0,180)+ map(analogValue[ch*3+CV]-channelCenter[ch],-512,511,-att,att);
		angle=constrain(angle,1,179);

	   //map(analogValue[ch*3+CV]-512,-512,512,-(analogValue[ch*3+ATT])<<1,(analogValue[ch*3+ATT])<<1);
		//angle=map(angle,0,1024,0,180);
	}
	srv[ch].write(angle);
}
long time;

void loop()
{
	for(int i=0;i<2;i++){
		gate[i]=!digitalRead(gatePin[i]);
	}
	for(int i=0;i<6;i++){
		analogValue[i]=analogRead(analogPin[i]);
	}

	if(millis()-time>2){
		time=millis();

	#ifdef DEBUG
	//	test();
	#else
		for(int i=0;i<2;i++){
			renderServo(i);
		}
		calibrate();
	#endif
	}
	//delay(1);
}




