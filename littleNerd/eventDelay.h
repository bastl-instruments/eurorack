#ifndef EVENTDELAY_H_
#define EVENTDELAY_H_

#include <Arduino.h>
#include <avr/pgmspace.h>

#define BUFFER_SIZE 64

class eventDelay {


public:



	// sets up all the pins, timers and SPI interface
	// call this before using any other method from this class
	void init(void(*eventNow)(uint8_t channel, uint8_t type));
	void update(uint32_t time);
	void createEvent(uint8_t channel, uint8_t type, uint32_t time);
	uint8_t getNumberOfEventsInBuffer(){ return numberOfEventsInBuffer;}
	void printBuffers();
	void shiftTimeBuffer(bool left,uint8_t from);
	void shiftTypeBuffer(bool left,uint8_t from);
	void fillBuffers();
	void clearBuffers();




private:
	void (*eventNow)(uint8_t channel, uint8_t type);
	uint32_t timeBuffer[BUFFER_SIZE];
	uint8_t typeBuffer[BUFFER_SIZE];
	uint8_t numberOfEventsInBuffer;
	uint32_t internalTime;







};



#endif
