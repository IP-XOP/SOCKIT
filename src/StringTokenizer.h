/*
 *  StringTokenizer.h
 *  iPeek
 *
 *  Created by andrew on 18/05/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */
#include <map>
#include <algorithm>
#include <vector>
#include <string>
using namespace std;

/* in StringTokenizer.cpp */
void Tokenize(const unsigned char* STR, unsigned long szStr, vector<string> &tokens, unsigned long *szTotalTokens, const char* DELIMITERS, int szDELIMITERS);

