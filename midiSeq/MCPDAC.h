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




class MCPDACClass {



	public:

		void init(uint8_t _cspin, uint8_t _cspin2);

		void writeDAC(uint8_t channel, uint16_t voltage);


	private:

	uint8_t csPin;
	uint8_t csPin2;

};


