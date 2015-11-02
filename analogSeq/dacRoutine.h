/*
 * dacRoutine.h
 *
 *  Created on: Oct 27, 2015
 *      Author: dasvaclav
 */

#ifndef DACROUTINE_H_
#define DACROUTINE_H_

#include<spiShared.h>
#define DAC_CONFIG 0x30
#define SS_DAC B,2
void writeDAC(uint16_t signal) {

  bit_clear(SS_DAC);

  spiWrite(DAC_CONFIG | ((signal >> 8) & 0xF)); // 4 config bits and bits 11-8
  spiWrite(signal); // bits 7-0

  bit_set(SS_DAC);
}
void dacInit(){
	bit_clear(SCK);
	bit_clear(MISO);
	bit_clear(MOSI);
	bit_set(SS);

	bit_dir_outp(SCK);
	bit_dir_inp(MISO);
	bit_dir_outp(MOSI);
	bit_dir_outp(SS);


	// chip select pins

	bit_set(SS_DAC);
	bit_dir_outp(SS_DAC);

	// Mode
	SPCR |= _BV(SPE);    // enable SPI
	setHigh(SPCR,MSTR);   // SPI master mode
	SPCR &= ~_BV(SPIE);  // SPI interrupts off
	SPCR &= ~_BV(DORD);  // MSB first
	SPCR &= ~_BV(CPOL);  // leading edge rising
	SPCR &= ~_BV(CPHA);  // sample on leading edge
	SPCR &= ~_BV(SPR1);  // speed = clock/4
	SPCR &= ~_BV(SPR0);
	SPSR |= _BV(SPI2X);  // 2X speed
}



#endif /* DACROUTINE_H_ */
