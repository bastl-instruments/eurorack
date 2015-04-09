

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


//#define DEBUG

#include <EEPROM.h>


#define FORWARD_A 6
#define BACKWARD_A 7
#define PWM_A 10
#define LED_F_A 3
#define LED_B_A 2

#define CV_A 0
#define POT_1_A 2
#define POT_2_A 3


#define FORWARD_B 8
#define BACKWARD_B 9
#define PWM_B 11
#define LED_F_B 5
#define LED_B_B 4

#define CV_B 1
#define POT_1_B 4
#define POT_2_B 5

uint8_t motorPin[4]={
  FORWARD_A,BACKWARD_A,FORWARD_B,BACKWARD_B};
uint8_t ledPin[4]={
  LED_F_A,LED_B_A,LED_F_B,LED_B_B};
uint8_t pwmPin[2]={
  PWM_A,PWM_B};

uint8_t analogPin[6]={
  CV_A,POT_1_A,POT_2_A,CV_B,POT_1_B,POT_2_B};
uint16_t analogValue[6];

void setup(){

  for(int i=0;i<4;i++){
    pinMode(motorPin[i],OUTPUT);
    pinMode(ledPin[i],OUTPUT);
  }
  pinMode(pwmPin[0],OUTPUT);
  pinMode(pwmPin[1],OUTPUT);
  for(int i=0;i<6;i++){
    pinMode(analogPin[i],INPUT);
  }
#ifdef DEBUG
  Serial.begin(9600);
#endif

}
#define THRESHOLD 0
#define CV 1
#define FORWARD 1
#define BACKWARD 0
uint8_t speed[2];

void test(){
  //for(int i=0;i<2;i++){
  bool dir;//=random(2);
  if(analogRead(POT_2_A)>512) dir=true;
  else dir=false;

  uint8_t sp=random(255);
  motor(0,dir,sp);
  Serial.print("m: ");
  Serial.print(0);
  Serial.print(" d: ");
  Serial.print(dir);
  Serial.print(" s: ");
  Serial.print(sp);

  if(analogRead(POT_2_B)>512) dir=true;
  else dir=false;

  sp=random(255);
  motor(1,dir,sp);
  Serial.print(" m: ");
  Serial.print(1);
  Serial.print(" d: ");
  Serial.print(dir);
  Serial.print(" s: ");
  Serial.print(sp);
  //}
  Serial.println();
  delay(random(400));
}
void motor(uint8_t ch,bool direction, uint8_t pwm){
  if(direction){
    digitalWrite(motorPin[ch*2+0],HIGH);
    digitalWrite(motorPin[ch*2+1],LOW);
    digitalWrite(ledPin[ch*2+0],HIGH);
    digitalWrite(ledPin[ch*2+1],LOW);
  }
  else{
    digitalWrite(motorPin[ch*2+0],LOW);
    digitalWrite(motorPin[ch*2+1],HIGH);
    digitalWrite(ledPin[ch*2+0],LOW);
    digitalWrite(ledPin[ch*2+1],HIGH);
  }
  analogWrite(pwmPin[ch],pwm);
}
#define SPEED_OFFSET 2
void renderChannel(uint8_t ch){
  uint16_t value=analogValue[3*ch + CV]+analogValue[3*ch + THRESHOLD];
  bool dir;
  if(value>512){
    dir=true;
    speed[ch]=constrain((( value-512)>>1) + ((analogValue[3*ch + SPEED_OFFSET])>>2),0,255);
  }
  else{
    dir=false;
    speed[ch]=constrain(((512-value)>>1) + ((analogValue[3*ch + SPEED_OFFSET])>>2),0,255);
  }


  if(dir){
    motor(ch,FORWARD,speed[ch]);
  }
  else {
    motor(ch,BACKWARD,speed[ch]);
  }
}
void loop()
{
#ifdef DEBUG
  test();
#else
  for(int i=0;i<6;i++){
    analogValue[i]=analogRead(analogPin[i]);
  }
  for(int i=0;i<2;i++){
    renderChannel(i);

  }

#endif


}


