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

#define CHANNEL_A false
#define CHANNEL_B true
#define GAIN_LOW false
#define GAIN_HIGH true


class MCPDACClass {

	public:
		boolean ldac;
		boolean gainA;
		boolean gainB;
		boolean shutdownA;
		boolean shutdownB;
		unsigned int cspin;
		unsigned int ldacpin;

	public:
		void begin();
		void begin(unsigned int cspin);
		void begin(unsigned int cspin, unsigned int ldacpin);
		void setGain(bool chan, bool gain);
		void shutdown(bool chan, bool sd);
		void setVoltage(bool channel, unsigned int mv);
		void update();
};

extern MCPDACClass MCPDAC;
