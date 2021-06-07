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
	double retval;
}SOCKITtotalOpenedStruct, *SOCKITtotalOpenedStructPtr;
#pragma pack()

#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
typedef struct SOCKITinfoStruct {
	double sockID;
	UserFunctionThreadInfoPtr tp;
	Handle retval;
}SOCKITinfoStruct,*SOCKITinfoStructPtr;
#pragma pack()


extern "C" int SOCKITtotalOpened(SOCKITtotalOpenedStructPtr);
extern "C" int SOCKITcurrentOpened(SOCKITtotalOpenedStructPtr);
extern "C" int SOCKITinfo(SOCKITinfoStructPtr);
