/*
 *  SOCKITinfo.h
 *  SOCKIT
 *
 *  Created by andrew on 4/01/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "XOPStructureAlignmentTwoByte.h"	// All structures passed to Igor are two-byte aligned.
typedef struct SOCKITtotalOpened {
	DOUBLE retval;
}SOCKITtotalOpenedStruct, *SOCKITtotalOpenedStructPtr;
#include "XOPStructureAlignmentReset.h"

int SOCKITtotalOpened(SOCKITtotalOpenedStructPtr);
int SOCKITcurrentOpened(SOCKITtotalOpenedStructPtr);