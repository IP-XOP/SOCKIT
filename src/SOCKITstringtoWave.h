// Runtime param structure for SOCKITstringtoWave operation.
#include "XOPStandardHeaders.h"
#include "XOPStructureAlignmentTwoByte.h"	// All structures passed to Igor are two-byte aligned.
struct SOCKITstringtoWaveRuntimeParams {
	// Flag parameters.

	// Parameters for /E flag group.
	int EFlagEncountered;
	// There are no fields for this group because it has no parameters.

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
};
typedef struct SOCKITstringtoWaveRuntimeParams SOCKITstringtoWaveRuntimeParams;
typedef struct SOCKITstringtoWaveRuntimeParams* SOCKITstringtoWaveRuntimeParamsPtr;
#include "XOPStructureAlignmentReset.h"		// Reset structure alignment to default.

int RegisterSOCKITstringtoWave(void);