
#ifndef MIDI_NOTE_BUFFER_H_
#define MIDI_NOTE_BUFFER_H_

#include <basic.h>
#include <Arduino.h>

#define BUFFER_SIZE 64
class MIDInoteBuffer {

public:
	void init();
	void addNoteToBuffer(uint8_t _note, uint8_t _velocity);
	bool removeNoteFromBuffer(uint8_t note);
	void handlePriority();
	void setPolyphony(uint8_t _polyphony);
	void setPriority(uint8_t _priority);
	void testOrdering();
	void orderMidiBuffer();
	void orderVoiceBuffer();
	void setBuffer(uint8_t _index,uint8_t _value);
	uint8_t getBuffer(uint8_t _index);

	uint8_t getVoiceNote(uint8_t _voice);
	uint8_t getNoteForVoice(uint8_t _voice);

	uint8_t getNoteToBePlayedAgain();
	uint8_t getVelocityOfNoteToBePlayedAgain();
	uint8_t getNumberOfNotesInBuffer();
	uint8_t getNoteFromBuffer(uint8_t _note);
	uint8_t getVelocityFromBuffer(uint8_t _note);
	uint8_t getNoteFromVoiceBuffer(uint8_t _note);

	bool getVoiceGate(uint8_t _voice);
	void allocateVoice(uint8_t _note);
	void deallocateVoice(uint8_t _note);

	bool legato;
	//
	void clearBuffers();



private:
	void decreaseReleaseOrder();
	uint8_t activeVoice[4];
	uint16_t whenReleased[4];
	uint8_t priority;
	uint8_t polyphony;
	uint8_t notesInBuffer;
	uint8_t notesInVoiceBuffer;
	uint8_t midiBuffer[BUFFER_SIZE];
	uint8_t velocityBuffer[BUFFER_SIZE];
	uint8_t voiceBuffer[BUFFER_SIZE];

	bool voiceOn[4];
	//uint8_t voiceRotation;
	void shiftBuffer(uint8_t direction,uint8_t from);
	uint8_t findNoteInBuffer(uint8_t note);

};





#endif
