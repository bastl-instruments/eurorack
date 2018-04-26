/*
 * MIDInoteBuffer.cpp

 *
 *  Created on: Jan 28, 2015
 *      Author: dasvaclav
 */
#include "MIDInoteBuffer.h"
#include <basic.h>
#include <Arduino.h>

#define LEFT_SHIFT 0
#define RIGHT_SHIFT 1
void MIDInoteBuffer::init(){
	for(uint8_t i=0;i<polyphony;i++) activeVoice[i]=255,whenReleased[i]=polyphony-i; //0,1,2,3
	clearBuffers();
}
#define LOW_P 0
#define HIGH_P 1
#define ORDER_P 2
void MIDInoteBuffer::handlePriority(){
	//uint8_t a;
	// unsigned char nextOne=255;
	 if(priority!=ORDER_P) orderMidiBuffer();

	switch(priority){
		case LOW_P:
			/*
		 for(int j=0;j<BUFFER_SIZE;j++)  orderedBuffer[j]=0;
		 nextOne=255;
		  for(uint8_t i=0;i<notesInBuffer;i++){
			if(midiBuffer[i] < nextOne) nextOne=midiBuffer[i];
		  }
		  orderedBuffer[0]=nextOne;

		  for(int j=1;j<notesInBuffer;j++){
			nextOne=255;
			for(int i=0;i<notesInBuffer;i++){
			  if(midiBuffer[i]<nextOne && midiBuffer[i] > orderedBuffer[j-1] ) nextOne=midiBuffer[i];
			}
			orderedBuffer[j]=nextOne;
		  }
*/
		  break;
		case HIGH_P:


			break;
		case ORDER_P:
			break;
	}
}
void MIDInoteBuffer::orderMidiBuffer(){
	for (uint8_t i = 0; i < notesInBuffer; ++i){
		for (uint8_t j = i + 1; j < notesInBuffer; ++j){
			if (midiBuffer[i] > midiBuffer[j]){
				uint8_t a =  midiBuffer[i];
				midiBuffer[i] = midiBuffer[j];
				midiBuffer[j] = a;
			}
		}
	}
}

void MIDInoteBuffer::orderVoiceBuffer(){
	for (uint8_t i = 0; i < notesInVoiceBuffer; ++i){
		for (uint8_t j = i + 1; j < notesInVoiceBuffer; ++j){
			if (voiceBuffer[i] > voiceBuffer[j]){
				uint8_t a =  voiceBuffer[i];
				voiceBuffer[i] = voiceBuffer[j];
				voiceBuffer[j] = a;
			}
		}
	}
}
void MIDInoteBuffer::testOrdering(){
	for(uint8_t i=0;i<BUFFER_SIZE;i++){
	//	midiBuffer[i]=random(255);
	//	Serial.print(midiBuffer[i]);
	//	Serial.print(", ");
	}
//	Serial.println();

	notesInBuffer=BUFFER_SIZE;
	orderMidiBuffer();
	for(uint8_t i=0;i<BUFFER_SIZE;i++){

	//		Serial.print(midiBuffer[i]);
		//	Serial.print(", ");
	}
//	Serial.println();
}
void MIDInoteBuffer::setBuffer(uint8_t _index,uint8_t _value){
	notesInBuffer=_index;
	midiBuffer[_index]=_value;
}
uint8_t MIDInoteBuffer::getBuffer(uint8_t _index){
	return midiBuffer[_index];
}
void MIDInoteBuffer::setPolyphony(uint8_t _polyphony){
	polyphony=_polyphony;
}
void MIDInoteBuffer::setPriority(uint8_t _priority){
	priority=_priority;
}
uint8_t MIDInoteBuffer::getNoteToBePlayedAgain(){
	return midiBuffer[polyphony];
}
uint8_t MIDInoteBuffer::getVelocityOfNoteToBePlayedAgain(){
	return velocityBuffer[polyphony];
}
uint8_t MIDInoteBuffer::getNumberOfNotesInBuffer(){
	return notesInBuffer;
}
uint8_t MIDInoteBuffer::getNoteFromBuffer(uint8_t _note){
	return midiBuffer[_note];
}
uint8_t MIDInoteBuffer::getVelocityFromBuffer(uint8_t _note){
	return velocityBuffer[_note];
}
uint8_t MIDInoteBuffer::getNoteFromVoiceBuffer(uint8_t _note){
	return voiceBuffer[_note];
}
void MIDInoteBuffer::clearBuffers(){
	for(int i=0;i<BUFFER_SIZE;i++){
		midiBuffer[i]=0;
		velocityBuffer[i]=0;
		voiceBuffer[i]=0;
	}
	notesInBuffer=0;
}
void  MIDInoteBuffer::shiftBuffer(uint8_t direction,uint8_t from){
	if(direction==LEFT_SHIFT){
		for(int i=from;i<notesInBuffer;i++){
			midiBuffer[i]=midiBuffer[i+1];
			velocityBuffer[i]=velocityBuffer[i+1];
		 }
	}
	else if(direction==RIGHT_SHIFT){
		  for(int i=notesInBuffer;i>from;i--){
			midiBuffer[i]=midiBuffer[i-1];
			velocityBuffer[i]=velocityBuffer[i-1];
		  }
	}
}
uint8_t  MIDInoteBuffer::findNoteInBuffer(uint8_t note){
	for(int i=0;i<notesInBuffer;i++){
		if(midiBuffer[i]==note) return i;
	}
	return 255;
}

bool MIDInoteBuffer::removeNoteFromBuffer(uint8_t _note){

	if(notesInBuffer>0){ //midi noteOnBuffer
		uint8_t position=findNoteInBuffer(_note);
		if(position!=255) shiftBuffer(LEFT_SHIFT,position),notesInBuffer--;
	}
	deallocateVoice(_note);
	//voice alocation

	if(notesInBuffer>=polyphony) return 1; // signal realocate voice externaly
	else return 0;

}
void MIDInoteBuffer::addNoteToBuffer(uint8_t _note, uint8_t _velocity){

	removeNoteFromBuffer(_note);
	shiftBuffer(RIGHT_SHIFT,0);

	if(notesInBuffer<BUFFER_SIZE){
		notesInBuffer++;
	}

	midiBuffer[0]=_note;
	velocityBuffer[0]=_velocity;
//	handlePriority();
	allocateVoice(_note);
}

void MIDInoteBuffer::allocateVoice(uint8_t _note){
	uint8_t voiceToUse=255;

	switch(priority){
		case HIGH_P:
			voiceBuffer[notesInVoiceBuffer]=_note;
			if(notesInBuffer<BUFFER_SIZE){
				notesInVoiceBuffer++;
			}
			orderVoiceBuffer();
			for(uint8_t i=0;i<polyphony;i++) activeVoice[i]=voiceBuffer[notesInVoiceBuffer-i];
			break;
		case LOW_P:
			orderVoiceBuffer();
			for(uint8_t i=0;i<polyphony;i++) activeVoice[i]=voiceBuffer[i];
			break;
		case ORDER_P:
			//voice steal - steal the voice from the last played note >the the one that just came out
			if(notesInBuffer>polyphony){
				for(uint8_t i=0;i<polyphony;i++) if(activeVoice[i]==midiBuffer[polyphony]) voiceToUse=i;
			}

			for(uint8_t i=0;i<polyphony;i++) if(activeVoice[i]==_note) voiceToUse=i;
			if(voiceToUse==255){ //if not find a voice with lowest number (the last used), voice with 255 is voice that is occupied
				//
				for(uint8_t i=0;i<polyphony;i++) if(whenReleased[i]<voiceToUse) voiceToUse=whenReleased[i]; //find the lowest value
				for(uint8_t i=0;i<polyphony;i++) if(whenReleased[i]==voiceToUse) voiceToUse=i; //find the index of the lowest value

			}
			if(voiceToUse<polyphony) whenReleased[voiceToUse]=255,activeVoice[voiceToUse]=_note; //voice occupied

			break;

	}




	//activeVoiceBuffer[];
}
void MIDInoteBuffer::decreaseReleaseOrder(){
	for(uint8_t i=0;i<polyphony;i++) if(whenReleased[i]<=polyphony && whenReleased[i]>0) whenReleased[i]--;
}
void MIDInoteBuffer::deallocateVoice(uint8_t _note){
	uint8_t _whichVoice=255;
	switch(priority){
		case HIGH_P:

			break;
		case LOW_P:

			break;
		case ORDER_P:
			//is the note assigned to a voice?
			for(uint8_t i=0;i<polyphony;i++){
				if(activeVoice[i]==_note) _whichVoice=i; // if yes replace it with something from midi buffer, or put it to release stage
			}
			if(_whichVoice<polyphony){
				//should it be replaced?
				if(notesInBuffer>=polyphony){ //needs to be replaced
					activeVoice[_whichVoice]=midiBuffer[polyphony-1];
				}
				else{	//is actually released
					whenReleased[_whichVoice]=polyphony;
					decreaseReleaseOrder();
					 //lower all positions in the buffer

				}
			}
			break;
	}


	if(notesInBuffer==0){
		uint8_t _hanging;
		for(uint8_t i=0;i<polyphony;i++){
			if(whenReleased[i]==255) _hanging=i;
			whenReleased[_hanging]=polyphony;
			decreaseReleaseOrder();
		}


		legato=false;
		//lastNote released
	}

}

bool MIDInoteBuffer::getVoiceGate(uint8_t _voice){
	bool _gate=false;
	//if(whenReleased[_voice]<=polyphony) _
	if(whenReleased[_voice]==255) _gate=true;
	else _gate=false;
	return _gate;
}

uint8_t MIDInoteBuffer::getVoiceNote(uint8_t _voice){
	 return activeVoice[_voice];
	//if(_voice<polyphony) return activeVoice[_voice];
	//else return 255;

	/*
	uint8_t voiceNote=255;
	switch(priority){
		case LOW_P:
			if(_voice<notesInBuffer){
				voiceNote=midiBuffer[_voice];
			}
		  break;
		case HIGH_P:
			if(_voice<notesInBuffer){
				voiceNote=midiBuffer[notesInBuffer-_voice-1];
			}
			break;
		case ORDER_P:
			if(_voice<notesInBuffer){
				voiceNote=midiBuffer[_voice];
			}
			/*
		case NATURAL:

			naturalCounter++;
			if(naturalCounter>polyphony) naturalCounter=0;
			if(_voice<notesInBuffer){
				voiceNote=midiBuffer[naturalCounter];
			}

			break;
			*/
	//}
	//return voiceNote;

}
