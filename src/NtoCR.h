/**
*replaces substrings with another string.  Memory is allocated for the returned string and must be freed by the caller.
*@param source The sourcestring on which the replacement is to be done
*@param find The substring to be replaced
*@param replace The replacement for search_str
*/
#include <string>
using namespace std;

void find_and_replace(string &source, const string find, string replace);
