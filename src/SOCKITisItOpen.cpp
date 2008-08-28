/*
 *  SOCKITisItOpen.cpp
 *  iPeek
 *
 *  Created by andrew on 28/08/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */
#include "SOCKIT.h"
#include "SOCKITisItOpen.h"

int SOCKITisItOpen(SOCKITisItOpenStruct *p){
	int	err = 0;
	extern CurrentConnections *pinstance;
	SOCKET sockNum;
	int retVal;
	
	retVal = pinstance->isSockitOpen(p->sockitQuery,&sockNum);
	p->retval = retVal;
	
	done:
	return err;
}
