/*
 *  SOCKITopenconnection.h
 *  iPeek
 *
 *  Created by andrew on 19/05/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */
#include "XOPStandardHeaders.h"

#include "XOPStructureAlignmentTwoByte.h"	// All structures passed to Igor are two-byte aligned.
struct SOCKITopenconnectionRuntimeParams {
	// Flag parameters.

	// Parameters for /Q flag group.
	int QFlagEncountered;
	// There are no fields for this group because it has no parameters.

	// Parameters for /TOK flag group.
	int TOKFlagEncountered;
	Handle TOKFlagStrH;
	int TOKFlagParamsSet[1];

	// Parameters for /PROC flag group.
	int PROCFlagEncountered;
	char PROCFlagName[MAX_OBJ_NAME+1];
	int PROCFlagParamsSet[1];

	// Main parameters.

	// Parameters for ID keyword group.
	int IDEncountered;
	char IDVarName[MAX_OBJ_NAME+1];
	int IDParamsSet[1];

	// Parameters for IP keyword group.
	int IPEncountered;
	Handle IPStrH;
	int IPParamsSet[1];

	// Parameters for PORT keyword group.
	int PORTEncountered;
	double PORTNumber;
	int PORTParamsSet[1];

	// Parameters for BUF keyword group.
	int BUFEncountered;
	waveHndl BUFWaveH;
	int BUFParamsSet[1];

	// These are postamble fields that Igor sets.
	int calledFromFunction;					// 1 if called from a user function, 0 otherwise.
	int calledFromMacro;					// 1 if called from a macro, 0 otherwise.
};
typedef struct SOCKITopenconnectionRuntimeParams SOCKITopenconnectionRuntimeParams;
typedef struct SOCKITopenconnectionRuntimeParams* SOCKITopenconnectionRuntimeParamsPtr;
#include "XOPStructureAlignmentReset.h"		// Reset structure alignment to default.

int RegisterSOCKITopenconnection(void);
static int ExecuteSOCKITopenconnection(SOCKITopenconnectionRuntimeParamsPtr p);