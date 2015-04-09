

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

#define INPUT_1 B,0
#define INPUT_2 B,1
#define INPUT_3 B,2
#define INPUT_4 B,3

#define OUTPUT_1 D,4
#define OUTPUT_2 D,7
#define OUTPUT_3 D,6
#define OUTPUT_4 D,5
#define POT_PIN 0

const uint8_t input[4]={
  8,9,10,11};
const uint8_t output[4]={
  4,7,6,5};
bool triggerState[4];
#define NUMBER_OF_CHANNELS 4
void setup(){
  for(int i=0;i<NUMBER_OF_CHANNELS;i++){
    pinMode(input[i],INPUT_PULLUP);
    pinMode(output[i],OUTPUT);
    digitalWrite(output[i],LOW);
    triggerState[i]=false;
  }

}
uint32_t triggerTime[4];
bool outputState[4];
uint32_t triggerLength;

void loop()
{
  uint16_t newTriggerLength=analogRead(POT_PIN)>>1;
  if(newTriggerLength!=0 && triggerLength==0){
    for(int i=0;i<NUMBER_OF_CHANNELS;i++){
      triggerState[i]=false;
      outputState[i]=true;
      //digitalWrite(output[i],LOW);
    }
  }
  triggerLength=newTriggerLength;

  if(triggerLength==0){
    for(int i=0;i<NUMBER_OF_CHANNELS;i++){
      digitalWrite(output[i],!digitalRead(input[i]));
    }
  }
  else{
    for(int i=0;i<NUMBER_OF_CHANNELS;i++){
      bool newState=!digitalRead(input[i]);
      if(newState && !triggerState[i]){
        triggerTime[i]=millis();
        digitalWrite(output[i],HIGH);
        outputState[i]=true;
      }
      triggerState[i]=newState;
    }

    for(int i=0;i<NUMBER_OF_CHANNELS;i++){
      if(outputState[i]){
        if(millis()-triggerTime[i]>triggerLength){
          digitalWrite(output[i],LOW);
          outputState[i]=false;
        }
      }
    }
  }

}






