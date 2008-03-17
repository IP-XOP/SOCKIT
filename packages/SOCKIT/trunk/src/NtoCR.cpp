 #include "SOCKIT.h"
 
 
#include <string>
using namespace std;
 
char* NtoCR(char* source_str,char* search_str,char* replace_str)
{
char *ostr, *nstr = NULL, *pdest = "";
int length, nlen;
unsigned int nstr_allocated;
unsigned int ostr_allocated;

if(!source_str || !search_str || !replace_str){
printf("Not enough arguments\n");
return NULL;
}
ostr_allocated = sizeof(char) * (strlen(source_str)+1);
ostr = (char*) malloc( sizeof(char) * (strlen(source_str)+1));
if(!ostr){
printf("Insufficient memory available\n");
return NULL;
}
strcpy(ostr, source_str);

while(pdest)
{
pdest = strstr( ostr, search_str );
length = (int)(pdest - ostr);

if ( pdest != NULL )
{
ostr[length]='\0';
nlen = strlen(ostr)+strlen(replace_str)+strlen( strchr(ostr,0)+strlen(search_str) )+1;
if( !nstr || /* _msize( nstr ) */ nstr_allocated < sizeof(char) * nlen){
nstr_allocated = sizeof(char) * nlen;
nstr = (char*) malloc( sizeof(char) * nlen );
}
if(!nstr){
printf("Insufficient memory available\n");
return NULL;
}

strcpy(nstr, ostr);
strcat(nstr, replace_str);
strcat(nstr, strchr(ostr,0)+strlen(search_str));

if( /* _msize(ostr) */ ostr_allocated < sizeof(char)*strlen(nstr)+1 ){
ostr_allocated = sizeof(char)*strlen(nstr)+1;
ostr = (char*) malloc(sizeof(char)*strlen(nstr)+1 );
}
if(!ostr){
printf("Insufficient memory available\n");
return NULL;
}
strcpy(ostr, nstr);
}
}
if(nstr)
free(nstr);
return ostr;
}