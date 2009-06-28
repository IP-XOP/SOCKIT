/*
 *  SOCKITpeek.h
 *  iPeek
 *
 *  Created by andrew on 25/04/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */
#include "XOPStandardHeaders.h"			// Include ANSI headers, Mac headers, IgorXOP.h, XOP.h and XOPSupport.h

#include "XOPStructureAlignmentTwoByte.h"	// All structures passed to Igor are two-byte aligned.
typedef struct SOCKITpeekStruct {
	DOUBLE  sockID;
	void* tp;
	Handle dest;			//the string containing the content
}SOCKITpeekStruct, *SOCKITpeekStructPtr;
#include "XOPStructureAlignmentReset.h"

int SOCKITpeek(SOCKITpeekStructPtr p);
