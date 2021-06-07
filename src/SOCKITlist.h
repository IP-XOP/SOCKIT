/*
 *  SOCKITlist.h
 *  SOCKIT
 *
 *  Created by andrew on 13/06/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

// Runtime param structure for SOCKITstringtoWave operation.
#include "XOPStandardHeaders.h"
#pragma pack(2)
struct SOCKITlistRuntimeParams {

	
	// These are postamble fields that Igor sets.
	int calledFromFunction;					// 1 if called from a user function, 0 otherwise.
	int calledFromMacro;					// 1 if called from a macro, 0 otherwise.
};
typedef struct SOCKITlistRuntimeParams SOCKITlistRuntimeParams;
typedef struct SOCKITlistRuntimeParams* SOCKITlistRuntimeParamsPtr;
#pragma pack()

extern "C" int RegisterSOCKITlist(void);
