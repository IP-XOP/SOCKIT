#include "XOPStandardHeaders.h"			// Include ANSI headers, Mac headers, IgorXOP.h, XOP.h and XOPSupport.h

/* Include the necessary networking gubbins */
#include <stdio.h>


#ifdef _MACINTOSH_
#include <netdb.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <sys/time.h>
#define SOCKET int
#endif

#ifdef _WINDOWS_
#include <winsock2.h>
#include <ws2tcpip.h>
#include <Iphlpapi.h>
#include <conio.h>
#include <time.h>
#define snprintf _snprintf
#define close closesocket
#define read recv
#endif

#include <errno.h>
#include <map>
#include <algorithm>
#include <vector>
#include <string.h>


using namespace std;

#define REQUIRES_IGOR_500 1 + FIRST_XOP_ERR
#define NO_WINSOCK 2 + FIRST_XOP_ERR
#define NO_INPUT_STRING 3 + FIRST_XOP_ERR
#define BAD_HOST_RESOLV 4 + FIRST_XOP_ERR
#define SOCKET_ALLOC_FAILED 5 + FIRST_XOP_ERR
#define SERVER_CONNECT_FAILED 6 + FIRST_XOP_ERR
#define DATA_SEND_FAILED 7 + FIRST_XOP_ERR
#define WAVE_IN_USE 8 +FIRST_XOP_ERR
#define SOCKET_NOT_CONNECTED 9 + FIRST_XOP_ERR
#define PROCESSOR_NOT_AVAILABLE 10 + FIRST_XOP_ERR
#define PROBLEM_WRITING_TO_FILE 11 + FIRST_XOP_ERR
#define NO_SOCKET_DESCRIPTOR 12 + FIRST_XOP_ERR

#define BUFLEN 4096
#define MAX_URL_LEN 256
#define MAX_MSG_LEN 256
#define BUFFER_WAVE_LEN 1500


typedef struct waveBufferInfoStruct {
	waveHndl bufferWave;
	bool	toPrint;
	char processor[MAX_OBJ_NAME+1];
	char tokenizer[35];
}waveBufferInfoStruct, *waveBufferInfoStructPtr;

/* A structure to hold all the socket IO information */
typedef struct currentConnections {
	fd_set readSet;
	SOCKET maxSockNumber;
	std::map<SOCKET,waveBufferInfoStruct> bufferWaves;	//socket descriptor, wave buffer, containing the recv messages.
}currentConnections, *currentConnectionsPtr;


/* in SOCKITopenConnection.c */
#include "XOPStructureAlignmentTwoByte.h"	// All structures passed to Igor are two-byte aligned.
struct SOCKITopenconnectionRuntimeParams {
	// Flag parameters.

	// Parameters for /Q flag group.
	int QFlagEncountered;
	// There are no fields for this group because it has no parameters.

	// Parameters for /TOK flag group.
	int TOKFlagEncountered;
	Handle TOKFlagStrH;
	int TOKFlagParamsSet[1];

	// Parameters for /PROC flag group.
	int PROCFlagEncountered;
	char PROCFlagName[MAX_OBJ_NAME+1];
	int PROCFlagParamsSet[1];

	// Main parameters.

	// Parameters for ID keyword group.
	int IDEncountered;
	char IDVarName[MAX_OBJ_NAME+1];
	int IDParamsSet[1];

	// Parameters for IP keyword group.
	int IPEncountered;
	Handle IPStrH;
	int IPParamsSet[1];

	// Parameters for PORT keyword group.
	int PORTEncountered;
	double PORTNumber;
	int PORTParamsSet[1];

	// Parameters for BUF keyword group.
	int BUFEncountered;
	waveHndl BUFWaveH;
	int BUFParamsSet[1];

	// These are postamble fields that Igor sets.
	int calledFromFunction;					// 1 if called from a user function, 0 otherwise.
	int calledFromMacro;					// 1 if called from a macro, 0 otherwise.
};
typedef struct SOCKITopenconnectionRuntimeParams SOCKITopenconnectionRuntimeParams;
typedef struct SOCKITopenconnectionRuntimeParams* SOCKITopenconnectionRuntimeParamsPtr;
#include "XOPStructureAlignmentReset.h"		// Reset structure alignment to default.

int RegisterSOCKITopenconnection(void);
static int ExecuteSOCKITopenconnection(SOCKITopenconnectionRuntimeParamsPtr p);

/* in SOCKITcloseConnection.c */
#include "XOPStructureAlignmentTwoByte.h"	// All structures passed to Igor are two-byte aligned.
typedef struct SOCKITcloseConnectionStruct {
	DOUBLE socketToClose;
	DOUBLE retval;
}SOCKITcloseConnectionStruct, *SOCKITcloseConnectionStructPtr;
#include "XOPStructureAlignmentReset.h"

int SOCKITcloseConnection(SOCKITcloseConnectionStructPtr);

/* in checkRecvData.c */
int outputBufferDataToWave(SOCKET sockNum, waveHndl wavH, const char *writebuffer, const char *tokenizer);
int checkRecvData();

#include "XOPStructureAlignmentTwoByte.h"	// All structures passed to Igor are two-byte aligned.
typedef struct SOCKITcallProcessorStruct {
	Handle bufferWave;
	DOUBLE entryRow;
}SOCKITcallProcessorStruct, *SOCKITcallProcessorStructPtr;
#include "XOPStructureAlignmentReset.h"

/* in SOCKITcloseConnection */
void resetMaxSocketNumber();
int SOCKITcloseWorker(SOCKET socketToClose);
int checkIfWaveInUseAsBuf(waveHndl wav);

/*in NtoCR.cpp */
char* NtoCR(const char*,  char*, char*);

/*in SOCKITsendMsg.cpp*/
#include "XOPStructureAlignmentTwoByte.h"	// All structures passed to Igor are two-byte aligned.
struct SOCKITsendmsgRuntimeParams {

	// Main parameters.
	// Parameters for simple main group #0.
	int IDEncountered;
	double ID;
	int IDParamsSet[1];

	// Parameters for simple main group #1.
	int MSGEncountered;
	Handle MSG;
	int MSGParamsSet[1];

	// These are postamble fields that Igor sets.
	int calledFromFunction;					// 1 if called from a user function, 0 otherwise.
	int calledFromMacro;					// 1 if called from a macro, 0 otherwise.
};
typedef struct SOCKITsendmsgRuntimeParams SOCKITsendmsgRuntimeParams;
typedef struct SOCKITsendmsgRuntimeParams* SOCKITsendmsgRuntimeParamsPtr;
#include "XOPStructureAlignmentReset.h"		// Reset structure alignment to default.

int RegisterSOCKITsendmsg(void);
int ExecuteSOCKITsendmsg(SOCKITsendmsgRuntimeParamsPtr p);


/* in StringTokenizer.cpp */
void Tokenize(const char *str, vector<string>& tokens,  const char* delimiters);

/* in SOCKITprocessor.cpp */
int checkProcessor(const char *processor, FunctionInfo *fip);
int registerProcessor(SOCKET sockNum, const char *processor);
int deRegisterProcessor(SOCKET sockNum);

#include "XOPStructureAlignmentTwoByte.h"	// All structures passed to Igor are two-byte aligned.
typedef struct SOCKITprocessorStruct {
	Handle processor;
	DOUBLE sockNum;
	DOUBLE retval;		
}SOCKITprocessorStruct, *SOCKITprocessorStructPtr;
#include "XOPStructureAlignmentReset.h"

int SOCKITregisterProcessor(SOCKITprocessorStruct*);

/* in SOCKITsendnrecv.cpp */
#include "XOPStructureAlignmentTwoByte.h"	// All structures passed to Igor are two-byte aligned.
struct SOCKITsendnrecvRuntimeParams {

	// Parameters for /FILE flag group.
	int FILEFlagEncountered;
	Handle FILEFlagStrH;
	int FILEFlagParamsSet[1];

	// Parameters for /TIME flag group.
	int TIMEFlagEncountered;
	double TIMEFlagNumber;
	int TIMEFlagParamsSet[1];

	// Parameters for /SMALL flag group.
	int SMALFlagEncountered;

	// Main parameters.

	// Parameters for simple main group #0.
	int IDEncountered;
	double ID;
	int IDParamsSet[1];

	// Parameters for simple main group #1.
	int MSGEncountered;
	Handle MSG;
	int MSGParamsSet[1];

	// These are postamble fields that Igor sets.
	int calledFromFunction;					// 1 if called from a user function, 0 otherwise.
	int calledFromMacro;					// 1 if called from a macro, 0 otherwise.
};
typedef struct SOCKITsendnrecvRuntimeParams SOCKITsendnrecvRuntimeParams;
typedef struct SOCKITsendnrecvRuntimeParams* SOCKITsendnrecvRuntimeParamsPtr;
#include "XOPStructureAlignmentReset.h"		// Reset structure alignment to default.

int RegisterSOCKITsendnrecv(void);
static int ExecuteSOCKITsendnrecv(SOCKITsendnrecvRuntimeParamsPtr p);


#ifdef _WINDOWS_
size_t strlcpy(char *d, const char *s, size_t bufsize);
size_t strlcat(char *d, const char *s, size_t bufsize);
#endif

struct MemoryStruct {
	char *memory;
	size_t size;
};
typedef struct MemoryStruct MemoryStruct;


static void *myrealloc(void *ptr, size_t size)
{
    /* There might be a realloc() out there that doesn't like reallocing
	NULL pointers, so we take care of it here */
    if(ptr)
		return realloc(ptr, size);
    else
		return malloc(size);
}

static size_t
WriteMemoryCallback(void *ptr, size_t size, size_t nmemb, void *data)
{
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)data;
	
    mem->memory = (char *)myrealloc(mem->memory, mem->size + realsize + 1);
    if (mem->memory) {
		memcpy(&(mem->memory[mem->size]), ptr, realsize);
		mem->size += realsize;
		mem->memory[mem->size] = 0;
    }
    return realsize;
}

/*
	roundDouble returns a rounded value for val
 */
static double
roundDouble(double val){
	double retval;
	if(val>0){
		if(val-floor(val) < 0.5){
			retval = floor(val);
		} else {
			retval = ceil(val);
		}
	} else {
		if(val-floor(val) <= 0.5){
			retval = floor(val);
		} else {
			retval = ceil(val);
		}
	}
	return retval;
}
