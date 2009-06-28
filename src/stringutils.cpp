#include <string.h>

size_t strlcpy(char *d, const char *s, size_t bufsize)
{
	size_t len;
	size_t ret;
	
	if (!d || !s) return 0;
	len = strlen(s);
	ret = len;
	if (bufsize <= 0) return 0;
	if (len >= bufsize) len = bufsize-1;
	memcpy(d, s, len);
	d[len] = 0;
	
	return ret;
}

size_t strlcat(char *d, const char *s, size_t bufsize)
{
	size_t len1;
	size_t len2;
	size_t ret;
	
	if (!d || !s || bufsize <= 0) return 0;
	
	len1 = strlen(d);
	len2 = strlen(s);
	ret = len1 + len2;
	if (len1+len2 >= bufsize) 
	{
		len2 = bufsize - (len1+1);
	}
	if (len2 > 0) 
	{
		memcpy(d+len1, s, len2);
		d[len1+len2] = 0;
	}
	return ret;
};
