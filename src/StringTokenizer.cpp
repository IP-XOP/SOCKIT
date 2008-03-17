#include <string>
#include <algorithm>
#include <vector>

using namespace std;

void Tokenize(const char* STR, vector<string> &tokens, const char *DELIMITERS)
{
    // Skip delimiters at beginning.
	string str(STR);
	string delimiters(DELIMITERS);
	
    string::size_type lastPos = str.find_first_not_of(delimiters, 0);
    // Find first "non-delimiter".
    string::size_type pos     = str.find_first_of(delimiters, lastPos);

    while (string::npos != pos || string::npos != lastPos)
    {
        // Found a token, add it to the vector.
        tokens.push_back(str.substr(lastPos, pos - lastPos));
        // Skip delimiters.  Note the "not_of"
        lastPos = str.find_first_not_of(delimiters, pos);
        // Find next "non-delimiter"
        pos = str.find_first_of(delimiters, lastPos);
    }
}
