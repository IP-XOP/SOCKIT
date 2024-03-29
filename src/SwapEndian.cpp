/*
*  SwapEndian.cpp
*  SOCKIT
*
*  Created by andrew on 20/06/12.
*  Copyright 2012 __MyCompanyName__. All rights reserved.
*
*/
#include "XOPStandardHeaders.h"
#include "SwapEndian.h"
#ifdef WINIGOR
#include <winsock2.h>
#endif
#include <algorithm> //required for std::swap

void ByteSwap(unsigned char * b, int n)
{
	register int i = 0;
	register int j = n-1;
	while (i<j)
	{
		std::swap(b[i], b[j]);
        i++; j--;
	}
}
//static long _TestEndian=1;

int IsLittleEndian(void) {
	if ( htonl(47) == 47 ) {
		return 0;
	} else {
		return 1;
	}	
//	return *(char*)&_TestEndian;
}


