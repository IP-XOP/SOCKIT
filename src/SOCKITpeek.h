/*
 *  SOCKITpeek.h
 *  SOCKIT
 *
 *  Created by andrew on 25/04/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */
#include "XOPStandardHeaders.h"			// Include ANSI headers, Mac headers, IgorXOP.h, XOP.h and XOPSupport.h

#pragma pack(2)
typedef struct SOCKITpeekStruct {
	double  sockID;
	UserFunctionThreadInfoPtr tp;
	Handle dest;			//the string containing the content
}SOCKITpeekStruct, *SOCKITpeekStructPtr;
#pragma pack()        // Reset structure alignment to default.

extern "C" int SOCKITpeek(SOCKITpeekStructPtr p);
