#include "XOPStandardHeaders.h"			// Include ANSI headers, Mac headers, IgorXOP.h, XOP.h and XOPSupport.h

/* Include the necessary networking gubbins */
#include <stdio.h>
#include "CurrentConnections.h"

using namespace std;

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



#include "XOPStructureAlignmentTwoByte.h"	// All structures passed to Igor are two-byte aligned.
typedef struct SOCKITprocessorStruct {
	Handle processor;
	DOUBLE sockNum;
	DOUBLE retval;		
}SOCKITprocessorStruct, *SOCKITprocessorStructPtr;
#include "XOPStructureAlignmentReset.h"

int SOCKITregisterProcessor(SOCKITprocessorStruct*);

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


#include "XOPStructureAlignmentTwoByte.h"	// All structures passed to Igor are two-byte aligned.
typedef struct SOCKITcloseConnectionStruct {
	DOUBLE socketToClose;
	DOUBLE retval;
}SOCKITcloseConnectionStruct, *SOCKITcloseConnectionStructPtr;
#include "XOPStructureAlignmentReset.h"

int SOCKITcloseConnection(SOCKITcloseConnectionStructPtr);

#include "XOPStructureAlignmentTwoByte.h"	// All structures passed to Igor are two-byte aligned.
struct SOCKITsendmsgRuntimeParams {

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
typedef struct SOCKITsendmsgRuntimeParams SOCKITsendmsgRuntimeParams;
typedef struct SOCKITsendmsgRuntimeParams* SOCKITsendmsgRuntimeParamsPtr;
#include "XOPStructureAlignmentReset.h"		// Reset structure alignment to default.

int RegisterSOCKITsendmsg(void);
int ExecuteSOCKITsendmsg(SOCKITsendmsgRuntimeParamsPtr p);



/*
	roundDouble returns a rounded value for val
 */
static double
roundDouble(double val){
	double retval;
	if(val>0){
		if(val-floor(val) < 0.5){
			retval = floor(val);
		} else {
			retval = ceil(val);
		}
	} else {
		if(val-floor(val) <= 0.5){
			retval = floor(val);
		} else {
			retval = ceil(val);
		}
	}
	return retval;
}
