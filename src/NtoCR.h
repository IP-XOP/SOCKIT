/**
*replaces substrings with another string.  Memory is allocated for the returned string and must be freed by the caller.
*@param source_str The sourcestring on which the replacement is to be done
*@param search_str The substring to be replaced
*@param replace_str The replacement for search_str
@return A new string.  The caller must use free() when the string is to be disposed of, as memory is allocated to create it.
*/
char* NtoCR(const char* source_str,char* search_str,char* replace_str);