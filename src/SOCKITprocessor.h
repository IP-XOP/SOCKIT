/*
 *  SOCKITregisterprocessor.h
 *  iPeek
 *
 *  Created by andrew on 19/05/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
typedef struct SOCKITprocessorStruct {
	Handle processor;
	double sockNum;
	double retval;		
}SOCKITprocessorStruct, *SOCKITprocessorStructPtr;
#pragma pack()

int SOCKITregisterProcessor(SOCKITprocessorStruct*);
