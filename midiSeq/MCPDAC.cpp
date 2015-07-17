/***********************************************
 * Microchip DAC library
 * (c) 2012 Majenko Technologies
 *
 * This library is offered without any warranty as to
 * fitness for purpose, either explicitly or implicitly
 * implied.
 ***********************************************/

/*
 * Microchip DAC library.  This works with the MCP4822 chip and
 * similar chips that work with the same protocol.
 */

#include <Arduino.h>
#include <SPI.h>
#include "MCPDAC.h"


void MCPDACClass::init(uint8_t _cspin, uint8_t _cspin2)
{

	csPin = _cspin;
	csPin2 = _cspin2;
	//this->ldacpin = ldacpin;
	pinMode(csPin,OUTPUT);
	digitalWrite(csPin,HIGH);
	pinMode(csPin2,OUTPUT);
	digitalWrite(csPin2,HIGH);
	SPI.begin();
}


void MCPDACClass::writeDAC(uint8_t channel, uint16_t voltage)
{
	uint8_t _csPin=0;
	if(channel/2){
		_csPin=csPin2;
	}
	else{
		_csPin=csPin;
	}
	unsigned int command;
	if(channel%2==0) command = 0x0000;
	else command = 0x8000;
	command |= 0x1000;
	command |= 0x0000 ;
	command |= (voltage & 0x0FFF);

	SPI.setDataMode(SPI_MODE0);
	digitalWrite(csPin,LOW);
	SPI.transfer(command>>8);
	SPI.transfer(command&0xFF);
	digitalWrite(csPin,HIGH);

}

