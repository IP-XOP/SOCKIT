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

int SOCKITtotalOpened(SOCKITtotalOpenedStructPtr);
int SOCKITcurrentOpened(SOCKITtotalOpenedStructPtr);