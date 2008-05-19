#include "XOPStandardHeaders.h"			// Include ANSI headers, Mac headers, IgorXOP.h, XOP.h and XOPSupport.h

/* Include the necessary networking gubbins */
#include "CurrentConnections.h"

using namespace std;

/*
	roundDouble returns a rounded value for val
 */
static double
roundDouble(double val){
	double retval;
	if(val>0){
		if(val-floor(val) < 0.5){
			retval = floor(val);
		} else {
			retval = ceil(val);
		}
	} else {
		if(val-floor(val) <= 0.5){
			retval = floor(val);
		} else {
			retval = ceil(val);
		}
	}
	return retval;
}
