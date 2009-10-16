#include "XOPStandardHeaders.h"
#include "XOPStructureAlignmentTwoByte.h"	// All structures passed to Igor are two-byte aligned.
struct SOCKITwaveToStringRuntimeParams {
	// Flag parameters.

	// Parameters for /E flag group.
	int EFlagEncountered;
	// Parameters for /TXT flag group.
	int TXTFlagEncountered;
	// There are no fields for this group because it has no parameters.

	// Main parameters.

	// Parameters for BUF keyword group.
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
};
typedef struct SOCKITwaveToStringRuntimeParams SOCKITwaveToStringRuntimeParams;
typedef struct SOCKITwaveToStringRuntimeParams* SOCKITwaveToStringRuntimeParamsPtr;
#include "XOPStructureAlignmentReset.h"		// Reset structure alignment to default.

int RegisterSOCKITwaveToString(void);
