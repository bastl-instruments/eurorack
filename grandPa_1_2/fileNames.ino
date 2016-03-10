//#include <mozzi_rand.h>
unsigned char searchIndex=1;
void indexAll(){
 // name[0]=EEPROM.read(firstLetterAddress);
//  EEPROM.write(firstLetterAddress,firstLetter);
   name[0]=firstLetter;
  name[1]=48;
  
  for(uint8_t i=0;i<36;i++){
      uint8_t _index=i;
      if(_index<10) name[1]=_index+48;
      else name[1]=_index+55;
       hw.displayChar(name[1]);
    hw.updateDisplay();
     // if(name[1]>58) _index=name[1]-48;
     // else _index=name[1]-65;
      
    if (!file.open(&root,name, O_READ)) { 
      // errorLoop();
     // return false;
     indexed(_index,false); 
  //   Serial.print("0");
    hw.setColor(RED);
    }
    else{
       hw.setColor(GREEN);
      index[_index]=root.curPosition()/32-1;
      indexed(_index,true); 
      //indexed(_sound,false); 
//       Serial.print("1");
    }
   // Serial.println(_index);
    file.close();
    hw.updateDisplay();
    delay(50);
  }

  

}
void dt(){
  // hw.displayText("SRCH");
}
void listNameUp(){
   name[1]++;
  if(name[1]>=58 && name[1]<65) name[1]=65;
  else if(name[1]>=91){
    name[1]=48; 
    upWithFirstLetter();

  }
  
    uint8_t _index;
  if(name[1]>58) _index=name[1]-55;
   else _index=name[1]-48;
   if(!indexed(_index)) listNameUp();
  
  /*
  uint8_t _index;
  if(name[1]>58) _index=name[1]-65;
   else _index=name[1]-48;
   if(!indexed(_index)){
      listNameUp();
   }
   */
   
   /*
  name[1]++;
  if(name[1]>=58 && name[1]<65) name[1]=65;
  else if(name[1]>=91){
    name[1]=48; 
    upWithFirstLetter();

  }
  while(!file.open(&root, name, O_READ)){
    //    hw.updateM
    hw.updateDisplay();
    //    hw.displayText("SRCH");
    // dt();
    name[1]+=searchIndex;
    searchIndex++;
    if(name[1]>=58 && name[1]<65) name[1]=65,searchIndex=1;
    else if(name[1]>=91){
      name[1]=48; 
      upWithFirstLetter();
      searchIndex=1;
    }
    hw.updateButtons();
    doIRestartGrandPa();
  }
  file.close();
  indexed(activeSound,false);
  if(!playBegin(name,activeSound)) listNameUp();
  stopSound();
  searchIndex=1;
  */

}
void listNameDown(){
  
  name[1]--;
  if(name[1]<48){
    name[1]=90;
    downWithFirstLetter();
  }
  else if(name[1]<65 && name[1]>58) name[1]=58;
  
  uint8_t _index;
  if(name[1]>58) _index=name[1]-55;
   else _index=name[1]-48;
   if(!indexed(_index)) listNameDown();
   
   
  /*
  uint8_t _index;
  if(name[1]>58) _index=name[1]-65;
   else _index=name[1]-48;
   if(!indexed(_index)){
      listNameDown();
   }
   */
 
   
/*
  name[1]--;
  if(name[1]<48){
    name[1]=90;
    downWithFirstLetter();

  }
  else if(name[1]<65 && name[1]>58) name[1]=58;

  while(!file.open(&root, name, O_READ)){
    hw.updateDisplay();
    dt();
    // hw.displayText("SRCH");
    name[1]-=searchIndex;
    searchIndex++;
    if(name[1]<48){
      name[1]=90;
      downWithFirstLetter();
      searchIndex=1;
    }
    else if(name[1]<65 && name[1]>58) name[1]=58,searchIndex=1;
    hw.updateButtons();
    doIRestartGrandPa();
  }
  file.close();
  indexed(activeSound,false);

  if(!playBegin(name,activeSound)) listNameDown();

  stopSound();
  searchIndex=1;
  */
}

void upWithFirstLetter(){
  name[0]=firstLetter;
  /*
  name[0]++;
   if(name[0]>=58 && name[0]<65) name[0]=65;
   else if(name[0]>=91) name[0]=48,name[1]=48;
   */
}
void downWithFirstLetter(){
  name[0]=firstLetter;
  /*
  name[0]--;
   if(name[0]<65 && name[0]>58) name[0]=58;
   if(name[0]<48) name[0]=90,name[1]=90;
   */

}






