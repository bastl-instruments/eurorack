	/* Arduino WaveRP Library
 * Copyright (C) 2009 by William Greiman
 *
 * This file is part of the Arduino WaveRP Library
 *
 * This Library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the Arduino WaveRP Library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */
/**
 *  inline functions for MCP4921 DAC
 */
#ifndef mcpDac_h
#define mcpDac_h

#include <avr/io.h>
#include <pin_to_avr.h>

// LDAC may be connected to ground to save a pin
// Set USE_MCP_DAC_LDAC to 0 if LDAC is grounded
#define USE_MCP_DAC_LDAC 1

// To change a pin assignment edit the #define for
// DDR, PORT, and BIT.  See the file pin_to_avr.h

// use arduino pins 2,3,4,5 for DAC

// pin 2 is DAC chip select
#define MCP_DAC_CS_DDR  PIN2_DDRREG
#define MCP_DAC_CS_PORT PIN2_PORTREG
#define MCP_DAC_CS_BIT  PIN2_BITNUM

// pin 3 is DAC serial clock
#define MCP_DAC_SCK_DDR  PIN3_DDRREG
#define MCP_DAC_SCK_PORT PIN3_PORTREG
#define MCP_DAC_SCK_BIT  PIN3_BITNUM

// pin 4 is DAC serial data in
#define MCP_DAC_SDI_DDR  PIN4_DDRREG
#define MCP_DAC_SDI_PORT PIN4_PORTREG
#define MCP_DAC_SDI_BIT  PIN4_BITNUM

// pin 5 is LDAC if used
#if USE_MCP_DAC_LDAC
#define MCP_DAC_LDAC_DDR  PIN5_DDRREG
#define MCP_DAC_LDAC_PORT PIN5_PORTREG
#define MCP_DAC_LDAC_BIT  PIN5_BITNUM
#endif  // USE_MCP_DAC_LDAC

//------------------------------------------------------------------------------
// set DAC CS low
static inline void mcpDacCsLow(void ) {
  MCP_DAC_CS_PORT &= ~_BV(MCP_DAC_CS_BIT);
}
// set DAC CS high
static inline void mcpDacCsHigh(void) {
  MCP_DAC_CS_PORT |= _BV(MCP_DAC_CS_BIT);
}
// set DAC SCK low
static inline void mcpDacSckLow(void) {
  MCP_DAC_SCK_PORT &= ~_BV(MCP_DAC_SCK_BIT);
}
// set DAC SCK high
static inline void mcpDacSckHigh(void) {
  MCP_DAC_SCK_PORT |= _BV(MCP_DAC_SCK_BIT);
}
// set DAC SDI low
static inline void mcpDacSdiLow(void) {
  MCP_DAC_SDI_PORT &= ~_BV(MCP_DAC_SDI_BIT);
}
// set DAC SDI high
static inline void mcpDacSdiHigh(void) {
  MCP_DAC_SDI_PORT |= _BV(MCP_DAC_SDI_BIT);
}
//------------------------------------------------------------------------------
// pulse DAC SCK pin
static inline void mcpDacSckPulse(void) {
  mcpDacSckHigh();
  mcpDacSckLow();
}
//------------------------------------------------------------------------------
// send bit b of d
static inline void mcpDacSendBit(uint16_t d, uint8_t b) {
  if (d & _BV(b)) {
    mcpDacSdiHigh();
  } else {
    mcpDacSdiLow();
  }
  mcpDacSckPulse();
}
//------------------------------------------------------------------------------
// init dac I/O ports
inline void mcpDacInit(void) {
  // set all to output mode
  MCP_DAC_CS_DDR |= _BV(MCP_DAC_CS_BIT);
  MCP_DAC_SCK_DDR |= _BV(MCP_DAC_SCK_BIT);
  MCP_DAC_SDI_DDR |= _BV(MCP_DAC_SDI_BIT);
  // chip select high
  mcpDacCsHigh();

#if USE_MCP_DAC_LDAC
  // LDAC low always - use unbuffered mode
  MCP_DAC_LDAC_DDR |= _BV(MCP_DAC_LDAC_BIT);
  MCP_DAC_LDAC_PORT &= ~_BV(MCP_DAC_LDAC_BIT);
#endif  // USE_MCP_DAC_LDAC
}
//------------------------------------------------------------------------------
// send 12 bits to dac
// trusted compiler to optimize and it does
// csLow to csHigh takes 8 - 9 usec on a 16 MHz Arduino //4096
static inline void mcpDacSend(uint16_t data) { // B0001 0000
  mcpDacCsLow();
  // send DAC config bits
  mcpDacSdiLow();
  mcpDacSckPulse();  // DAC A
//   mcpDacSdiHigh(); // hack
  mcpDacSckPulse();  // unbuffered
 //  mcpDacSdiLow(); // hack
 mcpDacSckPulse();  // 2X gain - hack
  mcpDacSdiHigh();
  //mcpDacSckPulse();  // 1X gain - comment out to hack
 
  mcpDacSckPulse();  // no SHDN

  // send 12 data bits
  mcpDacSendBit(data, 11);
  mcpDacSendBit(data, 10);
  mcpDacSendBit(data,  9);
  mcpDacSendBit(data,  8);
  mcpDacSendBit(data,  7);
  mcpDacSendBit(data,  6);
  mcpDacSendBit(data,  5);
  mcpDacSendBit(data,  4);
  mcpDacSendBit(data,  3);
  mcpDacSendBit(data,  2);
  mcpDacSendBit(data,  1);
  mcpDacSendBit(data,  0);
  mcpDacCsHigh();
}
#endif  // mcpDac_h
