/*
 * calibrationADC_HW.cpp
 *
 *  Created on: 23.03.2015
 *      Author: user
 */

#include "calibrationADC_HW.h"
#include <Arduino.h>
#include <SPI.h>
#include <portManipulations.h>
//#include <spiShared.h>
#include "globalSettings.h"




// constants
#define NOP asm volatile ("nop\n\t")
#define DAC_CONFIG 0x30 // DACa, unbuffered, gain=1, not shutdown


void calibrationADC_HW::init() {
	 SPI.begin();
	    SPI.setBitOrder(MSBFIRST);
	    SPI.setClockDivider(SPI_CLOCK_DIV4);
	    pinMode(10, OUTPUT);
	    digitalWrite(10, HIGH);


	// DEBUG
//	bit_set(TEST_CORE);
//	bit_dir_outp(TEST_CORE);

	// interrupt in
//	bit_dir_inp(MODE_CHANGE);

	// bitbang pins for ADC
//	bit_dir_outp(SCK_ADC);
	//bit_dir_inp(MISO_ADC);

	//bit_clear(SCK_ADC);
	//bit_clear(MISO_ADC);

	/**** HARDWARE SPI ****/
	/** make sure to set pin values before setting
	 ** them to output to not trigger interface
	 ** during setup  **/

	// Pins
	/*
	bit_clear(SCK);
	bit_clear(MISO);
	bit_clear(MOSI);
	bit_set(SS);

	bit_dir_outp(SCK);
	bit_dir_inp(MISO);
	bit_dir_outp(MOSI);
	bit_dir_outp(SS);
*/

	// chip select pins
	//bit_set(SS_RAM);
	//bit_set(SS_DAC);
	//bit_set(SS_ADC);
	//bit_set(SS_BUDDY);

	//bit_dir_outp(SS_RAM);
//	bit_dir_outp(SS_DAC);
	//bit_dir_outp(SS_ADC);
	//bit_dir_outp(SS_BUDDY);

/*
	// Mode
	SPCR |= _BV(SPE);    // enable SPI
	setHigh(SPCR,MSTR);  // SPI master mode
	SPCR &= ~_BV(SPIE);  // SPI interrupts off
	SPCR &= ~_BV(DORD);  // MSB first
	SPCR &= ~_BV(CPOL);  // leading edge rising
	SPCR &= ~_BV(CPHA);  // sample on leading edge
	SPCR &= ~_BV(SPR1);  // speed = clock/4
	SPCR &= ~_BV(SPR0);
	SPSR |= _BV(SPI2X);  // 2X speed
*/

	//TODO
	//parametersGeneral = NULL;
}




static inline __attribute__((always_inline)) void receiveBit(uint16_t &v, uint8_t bit) {
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;
  bit_set(SCK_ADC);
  NOP;
  NOP;
  NOP;
  if (bit_read_in(MISO_ADC)) v |= (1 << bit);
  bit_clear(SCK_ADC);
}





uint16_t calibrationADC_HW::readADC() {

	uint16_t result=0;
		uint8_t inByte=0;
		digitalWrite(10, LOW);
		result = SPI.transfer(0x00);
		result = result << 8;
		inByte = SPI.transfer(0x00);
		result = result | inByte;
		digitalWrite(10, HIGH);
		result = result >> 1;
		result = result & 0b0000111111111111;
		return result;

		/*
  uint16_t signal = 0;

  bit_clear(SS_ADC);

  // begin sample
  NOP;
  NOP;
  NOP;
  NOP;
  bit_set(SCK_ADC);   // <100ns to here | = 300ns
  NOP;
  NOP;
  NOP;
  NOP;
//  NOP;  // added for 20Mhz
  bit_clear(SCK_ADC);

  // second clock
  NOP;
  NOP;
  NOP;
  NOP;
 // NOP;  // added for 20Mhz
  bit_set(SCK_ADC);
  NOP;
  NOP;
  NOP;
  NOP;
//  NOP;  // added for 20Mhz
  bit_clear(SCK_ADC);

  // null bit
  NOP;
  NOP;
  NOP;
  NOP;
//  NOP;  // added for 20Mhz
  bit_set(SCK_ADC);
  NOP;
  NOP;
  NOP;
  NOP;
 // NOP;  // added for 20Mhz
  bit_clear(SCK_ADC);

  receiveBit(signal, 11);
  receiveBit(signal, 10);
  receiveBit(signal,  9);
  receiveBit(signal,  8);
  receiveBit(signal,  7);
  receiveBit(signal,  6);
  receiveBit(signal,  5);
  receiveBit(signal,  4);
  receiveBit(signal,  3);
  receiveBit(signal,  2);
  receiveBit(signal,  1);
  receiveBit(signal,  0);

  bit_set(SS_ADC);

  return signal;
  */
}

/*
void calibrationADC_HW::writeDAC(uint16_t signal) {

  bit_clear(SS_DAC);

  spiWrite(DAC_CONFIG | ((signal >> 8) & 0xF)); // 4 config bits and bits 11-8
  spiWrite(signal); // bits 7-0

  bit_set(SS_DAC);
}

void calibrationADC_HW::writeSRAM_3Bytes(int32_t address, int8_t *buf) {

  bit_clear(SS_RAM);

  spiWrite(0x02);           // mode = write
  spiWrite(address >> 16);  // address
  spiWrite(address >> 8);
  spiWrite(address);

  for(uint8_t i=0;i<3;i++) {
    spiWrite(*buf);
    buf++;
  }

  bit_set(SS_RAM);
}

void calibrationADC_HW::readSRAM_3Bytes(int32_t address, int8_t *buf) {

  bit_clear(SS_RAM);

  spiWrite(0x03);           // mode = read
  spiWrite(address >> 16);  // address
  spiWrite(address >> 8);
  spiWrite(address);
  for(int i=0;i<3;i++) {
    *buf = spiRead();
    buf++;
  }

  bit_set(SS_RAM);
}



void calibrationADC_HW::wrapAddress(int32_t& address) {
	static const int32_t maxAddress = 131067;

	if (address > maxAddress) address -= maxAddress+3 ;
	if (address < 0) address += maxAddress;
}


// expecting input as signed value
uint16_t calibrationADC_HW::combinedDACandADC(int16_t out) {


	uint16_t signal = 0;


	  bit_clear(SS_ADC);
	  out +=2048;   // DC OFFSET
	  NOP;
	  NOP;

	  bit_set(SCK_ADC);   // <100ns to here | ~ 300ns
	  NOP;
	  bit_clear(SS_DAC);
	  NOP;
	  NOP;

	  bit_clear(SCK_ADC);
	  NOP;
	  uint16_t tmp = (uint16_t)out>>8;
	  NOP;
	  NOP;

	  bit_set(SCK_ADC);
	  NOP;
	  tmp &= 0xF;
	  NOP;
	  NOP;

	  bit_clear(SCK_ADC);
	  NOP;
	  tmp |= DAC_CONFIG;
	  NOP;
	  NOP;
	  NOP;
	  NOP;

	  bit_set(SCK_ADC);
	  NOP;
	  SPDR = tmp;
	  NOP;
	  NOP;
	  bit_clear(SCK_ADC);

	  ///----------11
	  NOP;
	  NOP;
	  NOP;
	  NOP;
	  NOP;
	  bit_set(SCK_ADC);
	  if (bit_read_in(MISO_ADC)) signal |= (1 << 11);
	  bit_clear(SCK_ADC);

	  ///----------10
	  NOP;
	  NOP;
	  NOP;
	  NOP;
	  NOP;
	  bit_set(SCK_ADC);
	  NOP;
	  NOP;
	  NOP;
	  if (bit_read_in(MISO_ADC)) signal |= (1 << 10);
	  bit_clear(SCK_ADC);

	  ///----------9
	  (void)SPSR;
	  NOP;
	  NOP;
	  NOP;
	  NOP;
	  bit_set(SCK_ADC);
	  SPDR = (uint16_t)out;
	  NOP;
	  NOP;
	  if (bit_read_in(MISO_ADC)) signal |= (1 << 9);
	  bit_clear(SCK_ADC);

	  ///----------8
	  NOP;
	  NOP;
	  NOP;
	  NOP;
	  NOP;
	  bit_set(SCK_ADC);
	  NOP;
	  NOP;
	  NOP;
	  if (bit_read_in(MISO_ADC)) signal |= (1 << 8);
	  bit_clear(SCK_ADC);

	  ///----------7
	  NOP;
	  NOP;
	  NOP;
	  NOP;
	  NOP;
	  bit_set(SCK_ADC);
	  NOP;
	  NOP;
	  NOP;
	  if (bit_read_in(MISO_ADC)) signal |= (1 << 7);
	  bit_clear(SCK_ADC);

	  ///----------6
	  (void)SPSR;
	  NOP;
	  NOP;
	  NOP;
	  NOP;
	  bit_set(SCK_ADC);
	  bit_set(SS_DAC);
	  NOP;
	  if (bit_read_in(MISO_ADC)) signal |= (1 << 6);
	  bit_clear(SCK_ADC);


	  receiveBit(signal,  5);
	  receiveBit(signal,  4);
	  receiveBit(signal,  3);
	  receiveBit(signal,  2);
	  receiveBit(signal,  1);
	  receiveBit(signal,  0);

	  bit_set(SS_ADC);




	  return signal;

}










uint8_t calibrationADC_HW::receiveNextByte() {

		setLow(SPSR,SPI2X);
		bit_clear(SS_BUDDY);
		uint8_t nextByte = spiRead();
		bit_set(SS_BUDDY);
		setHigh(SPSR,SPI2X);

		return nextByte;
}

*/

