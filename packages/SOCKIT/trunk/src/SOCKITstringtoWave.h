// Runtime param structure for SOCKITstringtoWave operation.
#include "XOPStandardHeaders.h"
#pragma pack(2)
struct SOCKITstringtoWaveRuntimeParams {
	// Flag parameters.
	
	// Parameters for /E flag group.
	int EFlagEncountered;
	// There are no fields for this group because it has no parameters.
	
	// Parameters for /DEST flag group.
	int DESTFlagEncountered;
	DataFolderAndName dest;
	int DESTFlagParamsSet[1];
	
	// Parameters for /FREE flag group.
	int FREEFlagEncountered;
	// There are no fields for this group because it has no parameters.
	
	// Parameters for /TOK flag group.
	int TOKFlagEncountered;
	Handle TOKFlagStrH;
	int TOKFlagParamsSet[1];
	
	// Main parameters.
	
	// Parameters for simple main group #0.
	int numEncountered;
	double num;
	int numParamsSet[1];
	
	// Parameters for simple main group #1.
	int convEncountered;
	Handle conv;
	int convParamsSet[1];
	
	// These are postamble fields that Igor sets.
	int calledFromFunction;					// 1 if called from a user function, 0 otherwise.
	int calledFromMacro;					// 1 if called from a macro, 0 otherwise.
	UserFunctionThreadInfoPtr tp;			// If not null, we are running from a ThreadSafe function.
};
typedef struct SOCKITstringtoWaveRuntimeParams SOCKITstringtoWaveRuntimeParams;
typedef struct SOCKITstringtoWaveRuntimeParams* SOCKITstringtoWaveRuntimeParamsPtr;

#pragma pack()

int RegisterSOCKITstringtoWave(void);
