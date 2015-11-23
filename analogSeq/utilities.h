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

#endif /* UTILITIES_H_ */
