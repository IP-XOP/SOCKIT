/*
 *  SOCKITisItOpen.h
 *  iPeek
 *
 *  Created by andrew on 28/08/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#include "XOPStructureAlignmentTwoByte.h"	// All structures passed to Igor are two-byte aligned.
typedef struct SOCKITisItOpenStruct {
	DOUBLE sockitQuery;
	DOUBLE retval;
}SOCKITisItOpenStruct, *SOCKITisItOpenStructPtr;
#include "XOPStructureAlignmentReset.h"

int SOCKITisItOpen(SOCKITisItOpenStructPtr);