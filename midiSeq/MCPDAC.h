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



#define NUMBER_OF_TUNING_POINTS 15
class MCPDACClass {



	public:

		void init(uint8_t _cspin, uint8_t _cspin2);
		void writeDAC(uint8_t channel, uint16_t voltage);
		void autoCalibrate(uint8_t channel);
		void setNote(uint8_t channel, uint8_t note);

		void measureFrequency(uint8_t _channel);

		uint16_t measureOneVoltPeriod(uint8_t _channel);
		uint16_t measureTwoVoltPeriod(uint8_t _channel);
		void activitySense(uint8_t _channel);
		void autoTuneInit();
		void resetPinActivity();
		void autoTuneCalibrateAbsolute();
		void autoTuneCalibrateRelative();
		uint32_t tunePeriod[4];
		uint32_t discardedPeriod[4];
		void autoTuneCalibrateHeuristic();
	private:
	void allDACLow();
	uint16_t calibrationTable[4][NUMBER_OF_TUNING_POINTS];
	uint16_t autoTuneTable[4][NUMBER_OF_TUNING_POINTS];
	uint8_t csPin;
	uint8_t csPin2;
	uint8_t periods[4];
	bool tunePinState[4];
	bool lastTunePinState[4];
	bool pinActive[4];
	bool autoTuneActive[4];


	uint32_t tuneTime[4];
	//uint32_t tunePeriod[4];

};


