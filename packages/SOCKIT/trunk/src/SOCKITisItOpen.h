/*
 *  SOCKITisItOpen.h
 *  iPeek
 *
 *  Created by andrew on 28/08/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
typedef struct SOCKITisItOpenStruct {
	DOUBLE sockitQuery;
	void* tp;
	DOUBLE retval;
}SOCKITisItOpenStruct, *SOCKITisItOpenStructPtr;
#pragma pack()

int SOCKITisItOpen(SOCKITisItOpenStructPtr);

