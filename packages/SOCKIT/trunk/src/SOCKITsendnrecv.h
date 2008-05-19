/*
 *  SOCKITsendnrecv.h
 *  iPeek
 *
 *  Created by andrew on 19/05/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#include "XOPStructureAlignmentTwoByte.h"	// All structures passed to Igor are two-byte aligned.
struct SOCKITsendnrecvRuntimeParams {

	// Parameters for /FILE flag group.
	int FILEFlagEncountered;
	Handle FILEFlagStrH;
	int FILEFlagParamsSet[1];

	// Parameters for /TIME flag group.
	int TIMEFlagEncountered;
	double TIMEFlagNumber;
	int TIMEFlagParamsSet[1];

	// Parameters for /SMALL flag group.
	int SMALFlagEncountered;

	// Main parameters.

	// Parameters for simple main group #0.
	int IDEncountered;
	double ID;
	int IDParamsSet[1];

	// Parameters for simple main group #1.
	int MSGEncountered;
	Handle MSG;
	int MSGParamsSet[1];

	// These are postamble fields that Igor sets.
	int calledFromFunction;					// 1 if called from a user function, 0 otherwise.
	int calledFromMacro;					// 1 if called from a macro, 0 otherwise.
};
typedef struct SOCKITsendnrecvRuntimeParams SOCKITsendnrecvRuntimeParams;
typedef struct SOCKITsendnrecvRuntimeParams* SOCKITsendnrecvRuntimeParamsPtr;
#include "XOPStructureAlignmentReset.h"		// Reset structure alignment to default.

int RegisterSOCKITsendnrecv(void);
static int ExecuteSOCKITsendnrecv(SOCKITsendnrecvRuntimeParamsPtr p);
