#ifndef EVENTMEMORY_H_
#define EVENTMEMORY_H_

#include <Arduino.h>
#include <avr/pgmspace.h>

#define M_BUFFER_SIZE 5
#define STOP_STATE 0
#define RECORDING_STATE 1
#define LOOPING_STATE 2


class eventMemory {


public:



	// sets up all the pins, timers and SPI interface
	// call this before using any other method from this class
	void init(void(*recEventNow)(uint8_t channel, uint8_t type, uint8_t cycles));
	void update(uint32_t time);

	void startRecording(uint32_t time);
	void loopRecorded(uint32_t time);
	void stopLoop();
	bool recording();
	bool looping();
	uint32_t getloopLength(){return stopTime-startTime;};
	void setInitialState(uint8_t _ch,bool state);
	void refreshInitialState();




	void createEvent(uint8_t channel, uint8_t type, uint8_t cycles, uint32_t time);
	uint8_t getNumberOfEventsInBuffer(){ return numberOfEventsInBuffer;}



	void clearBuffers();

	void printBuffers();
	void fillBuffers();




private:
	uint8_t state;
	void (*recEventNow)(uint8_t channel, uint8_t type, uint8_t cycles);
	uint32_t timeBuffer[M_BUFFER_SIZE];
	uint8_t typeBuffer[M_BUFFER_SIZE];
	uint8_t numberOfEventsInBuffer;
	uint8_t numberOfEventsPassed;
	uint32_t internalTime;
	uint32_t startTime;
	uint32_t stopTime;
	bool initState[6];



};



#endif
