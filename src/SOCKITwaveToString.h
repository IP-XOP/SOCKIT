#include "XOPStandardHeaders.h"
#pragma pack(2)
struct SOCKITwaveToStringRuntimeParams {
	// Flag parameters.

	// Parameters for /E flag group.
	int EFlagEncountered;
	// Parameters for /TXT flag group.
	int TXTFlagEncountered;
	Handle TXTFlagStrH;						// Optional parameter.
	int TXTFlagParamsSet[1];
	
	// Main parameters.

	int wavEncountered;
	waveHndl wavWaveH;
	int wavParamsSet[1];	// Parameters for simple main group #0.

	// Parameters for simple main group #1.
	int strEncountered;
	char str[MAX_OBJ_NAME+1];
	int strParamsSet[1];

	// These are postamble fields that Igor sets.
	int calledFromFunction;					// 1 if called from a user function, 0 otherwise.
	int calledFromMacro;					// 1 if called from a macro, 0 otherwise.
	UserFunctionThreadInfoPtr tp;
	
};
typedef struct SOCKITwaveToStringRuntimeParams SOCKITwaveToStringRuntimeParams;
typedef struct SOCKITwaveToStringRuntimeParams* SOCKITwaveToStringRuntimeParamsPtr;
#pragma pack()		// Reset structure alignment to default.

int RegisterSOCKITwaveToString(void);
