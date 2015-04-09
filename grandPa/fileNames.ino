//#include <mozzi_rand.h>
unsigned char searchIndex=1;

void dt(){
  hw.displayText("SRCH");
}
void listNameUp(){
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
    dt();
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

}
void listNameDown(){

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





