/*
 *  SOCKITisItOpen.h
 *  SOCKIT
 *
 *  Created by andrew on 28/08/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
typedef struct SOCKITisItOpenStruct {
	double sockitQuery;
	UserFunctionThreadInfoPtr tp;
	double retval;
}SOCKITisItOpenStruct, *SOCKITisItOpenStructPtr;
#pragma pack()

extern "C" int SOCKITisItOpen(SOCKITisItOpenStructPtr);

