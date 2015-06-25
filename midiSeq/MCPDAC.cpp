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

MCPDACClass MCPDAC;

void MCPDACClass::begin()
{
	this->begin(10);
}

void MCPDACClass::begin(unsigned int cspin)
{
	this->ldac = false;
	this->begin(cspin,0);
}

void MCPDACClass::begin(unsigned int cspin, unsigned int ldacpin)
{
	this->ldac = true;
	this->cspin = cspin;
	this->ldacpin = ldacpin;
	this->shutdownA = false;
	this->shutdownB = false;
	this->gainA = false;
	this->gainB = false;
	pinMode(this->ldacpin,OUTPUT);
	digitalWrite(this->ldacpin,HIGH);
	pinMode(this->cspin,OUTPUT);
	digitalWrite(this->cspin,HIGH);
	SPI.begin();
}

void MCPDACClass::setGain(bool chan, bool gain)
{
	if(chan == CHANNEL_A)
	{
		this->gainA = gain;
	} else {
		this->gainB = gain;
	}
}

void MCPDACClass::shutdown(bool chan, bool sd)
{
	if(chan == CHANNEL_A)
	{
		this->shutdownA = sd;
	} else {
		this->shutdownB = sd;
	}
}

void MCPDACClass::setVoltage(bool channel, unsigned int mv)
{
	unsigned int command;
	if(channel == CHANNEL_A)
	{
		command = 0x0000;
		command |= this->shutdownA ? 0x0000 : 0x1000;
		command |= this->gainA ? 0x0000 : 0x2000;
		command |= (mv & 0x0FFF);
		SPI.setDataMode(SPI_MODE0);
		digitalWrite(this->cspin,LOW);
		SPI.transfer(command>>8);
		SPI.transfer(command&0xFF);
		digitalWrite(this->cspin,HIGH);
	} else {
		command = 0x8000;
		command |= this->shutdownB ? 0x0000 : 0x1000;
		command |= this->gainB ? 0x0000 : 0x2000;
		command |= (mv & 0x0FFF);
		SPI.setDataMode(SPI_MODE0);
		digitalWrite(this->cspin,LOW);
		SPI.transfer(command>>8);
		SPI.transfer(command&0xFF);
		digitalWrite(this->cspin,HIGH);
	}
}

void MCPDACClass::update()
{
	if(this->ldac==false)
		return;
	digitalWrite(this->ldacpin,LOW);
	digitalWrite(this->ldacpin,HIGH);
}
