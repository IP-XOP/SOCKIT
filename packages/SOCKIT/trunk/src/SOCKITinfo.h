/*
 *  SOCKITinfo.h
 *  SOCKIT
 *
 *  Created by andrew on 4/01/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#pragma pack(2)
typedef struct SOCKITtotalOpened {
	DOUBLE retval;
}SOCKITtotalOpenedStruct, *SOCKITtotalOpenedStructPtr;
#pragma pack()

#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
typedef struct SOCKITinfoStruct {
	DOUBLE sockID;
	void* tp;
	Handle retval;
}SOCKITinfoStruct, *SOCKITinfoStructPtr;
#pragma pack()


int SOCKITtotalOpened(SOCKITtotalOpenedStructPtr);
int SOCKITcurrentOpened(SOCKITtotalOpenedStructPtr);
int SOCKITinfo(SOCKITinfoStructPtr);