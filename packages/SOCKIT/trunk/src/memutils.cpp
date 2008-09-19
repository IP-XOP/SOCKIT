#include "memutils.h"
#include <new>

//base constructor
MemoryStruct::MemoryStruct(){
	memory=NULL;
	memsize=0;
}

//constructor with initial data
MemoryStruct::MemoryStruct(void *ptr, size_t size, size_t nmemb){
	memory=NULL;
	memsize=0;
	WriteMemoryCallback(ptr, size, nmemb);
}

//destructor free's the memory
MemoryStruct::~MemoryStruct(){
	if(memory)
		free(memory);
	memsize=0;
}

//resets the memory
void MemoryStruct::reset(){
	if(memory)
		free(memory);
	memsize = 0;
	memory = NULL;
}

//return the size of the memory used
size_t MemoryStruct::getMemSize(){
	return memsize;
};

//return a pointer to the filled memory.
const unsigned char* MemoryStruct::getData(){
	return (const unsigned char*)memory;
};
	

//create a platform independent routine for continuous reallocation of memory, appending data to it
void *MemoryStruct::myrealloc(void *src_ptr, size_t size)
{
    /* There might be a realloc() out there that doesn't like reallocing
	NULL pointers, so we take care of it here */
    if(src_ptr)
		return realloc(src_ptr, size);
    else
		return malloc(size);
}

//to use the static version you may have to define a function pointer
//size_t (*f)(void*,size_t,size_t,void*)=(MemoryStruct::WriteMemoryCallback);
//data has to be a point of a MemoryStruct object

size_t
MemoryStruct::WriteMemoryCallback(void *ptr, size_t size, size_t nmemb, void *Data)
{
    size_t realsize = size * nmemb;
    MemoryStruct *mem = (MemoryStruct *)Data;
	
    mem->memory = (unsigned char *)myrealloc(mem->memory, mem->memsize + realsize);
    if (mem->memory) {
		memcpy(&(mem->memory[mem->memsize]), ptr, realsize);
		mem->memsize += realsize;
//		mem->memory[mem->memsize] = 0;
    } else
		throw (std::bad_alloc());
    return realsize;
}

size_t
MemoryStruct::WriteMemoryCallback(void *ptr, size_t size, size_t nmemb)
{
    size_t realsize = size * nmemb;
	
    memory = (unsigned char *)myrealloc(memory, memsize + realsize);
    if (memory) {
		memcpy(&(memory[memsize]), ptr, realsize);
		memsize += realsize;
    } else
		throw (std::bad_alloc());

    return realsize;
}

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

void* malloc2d(int ii, int jj, int sz)
{
  void** p;
  int sz_ptr_array;
  int sz_elt_array;
  int sz_allocation;
  int i;

  sz_ptr_array = ii * sizeof(void*);
  sz_elt_array = jj * sz;
  sz_allocation = sz_ptr_array + ii * sz_elt_array;
 
  p = (void**) malloc(sz_allocation);
  if (p == NULL)
    return p;
  memset(p, 0, sz_allocation);
  for (i = 0; i < ii; ++i)
  {
    *(p+i) = (void*) ((int)p + sz_ptr_array + i * sz_elt_array);
  }
  return p;
}