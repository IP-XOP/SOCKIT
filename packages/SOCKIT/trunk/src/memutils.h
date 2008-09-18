#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
*MemoryStruct - A dynamic memory re-writer.
*The MemoryStruct class is used as a storage mechanism for data.  It can be sent extra data
*as and when required. The contiguous memory can then be accessed at a later stage.
*/
class MemoryStruct{
public:
	/**
	*Writes data to a MemoryStruct object with a static method. Example:
	*MemoryStruct a; double *doublearray; long numdoubles; WriteMemoryCallback(doublearray, sizeof(double), numdoubles, &a);
	*@param ptr A pointer to the data that you wish to store in the MemoryStruct object
	*@param size The size of the type of data you wish to store
	*@param nmemb The number of data you are trying to store
	*@param Data A MemoryStruct object to which the data will be appended.
	*@return The amount of memory allocated for the data you are trying to write
	*/
	static size_t WriteMemoryCallback(void *ptr, size_t size, size_t nmemb,void* Data);

	/**
	*Writes data to a MemoryStruct object. 	
	Example:
	*MemoryStruct a; double *doublearray; long numdoubles; a.WriteMemoryCallback(doublearray, sizeof(double), numdoubles);
	*@param ptr A pointer to the data that you wish to store in the MemoryStruct object.
	*@param size The size of the type of data you wish to store.
	*@param nmemb The number of data you are trying to store.
	*@return The amount of memory allocated for the data you are trying to write.
	*/
	size_t WriteMemoryCallback(void *ptr, size_t size, size_t nmemb);
	/**
	*Obtain the number of bytes held in a MemoryStruct object.	Example:
	*MemoryStruct a; long bytesAllocated; bytesAllocated = a.getMemSize();
	*@return The total amount of memory allocated in the MemoryStruct object
	*/
	size_t getMemSize();
	/**
	*Access the data held in a MemoryStruct object.
	*@return A constant character pointer to the start of the memory that you have stored.
	*/
	const unsigned char* getData();
	
	/**
	*Default constructor.
	*/
	MemoryStruct();
	/**
	*Constructor with some initial data to store.
	*/
	MemoryStruct(void *ptr, size_t size, size_t nmemb);
	/**
	*Destructor frees the memory stored in the MemoryStruct object.
	*/
	~MemoryStruct();

	/**
	*reset allows the MemoryStruct object to be reused
	*/
	void reset();
		
	private:
	/**
	*A platform independent reallocator of memory.  Uses malloc and realloc to assign more memory.
	*/
	static void *myrealloc(void *ptr, size_t size);
	
	unsigned char *memory; /**< A pointer to the data stored. */ 
	size_t memsize; /**< The number of bytes stored. */ 
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
