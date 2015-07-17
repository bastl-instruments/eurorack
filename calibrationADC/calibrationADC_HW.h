/*
 * saldaCoreHW.h
 *
 *  Created on: 23.03.2015
 *      Author: user
 */

#ifndef CAL_ADC_EHW_H_
#define CAL_ADC_EHW_H_

#include <inttypes.h>


class calibrationADC_HW {
public:
	void init();


	uint16_t readADC();
	/*
	void writeDAC(uint16_t signal);
	void writeSRAM_3Bytes(int32_t address, int8_t *buf);
	void readSRAM_3Bytes(int32_t address, int8_t *buf);

	// includes:
	// - moving input value to positive range
	// - send this value to DAC
	// - get current value from ADC
	uint16_t combinedDACandADC(int16_t signal);

	// reads the next byte from SPI bus connected to top board
	// includes necessary speed reduction
	uint8_t receiveNextByte();

	// map a given SRAM address to the valid range
	void wrapAddress(int32_t& address);

	static const uint32_t lastAddress = 131067;
	*/

};


#endif /* SALDACOREHW_H_ */
