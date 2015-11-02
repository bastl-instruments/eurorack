/*
 * utilities.h
 *
 *  Created on: Oct 27, 2015
 *      Author: dasvaclav
 */

#ifndef UTILITIES_H_
#define UTILITIES_H_
#include <Arduino.h>
bool inBetween(int val1, int val2, int inBet) {
	bool retVal;
	if (val1 >= val2) {
		if (inBet <= val1 && inBet >= val2)
			retVal = true;
		else
			retVal = false;
	} else if (val1 < val2) {
		if (inBet >= val1 && inBet <= val2)
			retVal = true;
		else
			retVal = false;
	}
	return retVal;

}

uint32_t curveMap(uint8_t value, uint8_t numberOfPoints, uint16_t * tableMap){
	uint32_t inMin=0, inMax=255, outMin=0, outMax=255;
	for(int i=0;i<numberOfPoints-1;i++){
		if(value >= tableMap[i] && value <= tableMap[i+1]) {
			inMax=tableMap[i+1];
			inMin=tableMap[i];
			outMax=tableMap[numberOfPoints+i+1];
			outMin=tableMap[numberOfPoints+i];
			i=numberOfPoints+10;
		}
	}
	return map(value,inMin,inMax,outMin,outMax);
}


#endif /* UTILITIES_H_ */
