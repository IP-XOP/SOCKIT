#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//reallocation of memory utility
/**
initialisation:
	
	struct MemoryStruct chunk;
	chunk.memory=NULL; 
	chunk.size = 0;    
*/

/** usage:
	WriteMemoryCallback(src_ptr, sizeof(char), numchar, &chunk);
	if(chunk.memory == NULL){
		error("Mem allocation failed\n"); 
		continue; 
	}
*/

/**
	if(chunk.memory!=NULL){
		free(chunk.memory);
		chunk.size=0;
		chunk.memory = NULL;
	}
*/

class MemoryStruct{
public:
	static size_t WriteMemoryCallback(void *ptr, size_t size, size_t nmemb,void*);
	size_t WriteMemoryCallback(void *ptr, size_t size, size_t nmemb);
	size_t getMemSize();
	const char* getData();
	
	MemoryStruct();
	MemoryStruct(void *ptr, size_t size, size_t nmemb);
	~MemoryStruct();
	
	private:
	static void *myrealloc(void *ptr, size_t size);
	char *memory;
	size_t memsize;
};


/*
 * \brief Create a two-dimensional array in a single allocation
 *
 * The effect is the same as an array of "element p[ii][jj];
 * The equivalent declaration is "element** p;"
 * The array is created as an array of pointer to element, followed by an array of arrays of elements.
 * \param ii first array bound
 * \param jj second array bound
 * \param sz size in bytes of an element of the 2d array
 * \return NULL on error or pointer to array
 *
 * assign return value to (element**)
 */

/* to use this in practice one would write 

	double **pp = NULL;
	pp = (double**)malloc2d(5, 11, sizeof(double));
	if(pp==NULL)
		return NOMEM;
	
	<use pp as required>
	free(pp);

Note you can access elements by
	 *(*(p+i)+j) is equivalent to p[i][j]
 In addition *(p+i) points to a whole row.
	*/

void* malloc2d(int ii, int jj, int sz);
