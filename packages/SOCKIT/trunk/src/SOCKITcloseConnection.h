/*
 *  SOCKITcloseConnection.h
 *  iPeek
 *
 *  Created by andrew on 19/05/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
typedef struct SOCKITcloseConnectionStruct {
	double socketToClose;
	UserFunctionThreadInfoPtr tp;
	double retval;
}SOCKITcloseConnectionStruct, *SOCKITcloseConnectionStructPtr;
#pragma pack()

int SOCKITcloseConnection(SOCKITcloseConnectionStructPtr);
