/*
*  NtoCr.cpp
*  SOCKIT
*
*  Created by andrew on 20/06/12.
*  Copyright 2012 __MyCompanyName__. All rights reserved.
*
*/
#include <string>
using namespace std;

void find_and_replace( string &source, const string find, string replace ){
	size_t j;
	for ( ; (j = source.find( find )) != string::npos ; ) {
		source.replace( j, find.length(), replace );
	}
}
