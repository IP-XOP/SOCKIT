#include "XOPStandardHeaders.h"			// Include ANSI headers, Mac headers, IgorXOP.h, XOP.h and XOPSupport.h

/* Include the necessary networking gubbins */
#include <stdio.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>
#include <map>
#include <algorithm>
#include <vector>
#include <string.h>
using namespace std;

#define REQUIRES_IGOR_200 1 + FIRST_XOP_ERR
#define UNKNOWN_XFUNC 2 + FIRST_XOP_ERR
#define NO_INPUT_STRING 3 + FIRST_XOP_ERR
#define BAD_HOST_RESOLV 4 + FIRST_XOP_ERR
#define SOCKET_ALLOC_FAILED 5 + FIRST_XOP_ERR
#define SERVER_CONNECT_FAILED 6 + FIRST_XOP_ERR
#define DATA_SEND_FAILED 7 + FIRST_XOP_ERR
#define WAVE_IN_USE 8 +FIRST_XOP_ERR
#define SOCKET_NOT_CONNECTED 9 + FIRST_XOP_ERR
#define PROCESSOR_NOT_AVAILABLE 10 + FIRST_XOP_ERR

#define BUFLEN 4096
#define MAX_URL_LEN 256
#define MAX_MSG_LEN 256
#define BUFFER_WAVE_LEN 1500


typedef struct waveBufferInfoStruct {
	waveHndl bufferWave;
	bool	toPrint;
	FunctionInfo *processorfip;
	char processor[MAX_OBJ_NAME+1];
	char tokenizer[35];
}waveBufferInfoStruct, *waveBufferInfoStructPtr;

/* A structure to hold all the socket IO information */
typedef struct currentConnections {
	fd_set readSet;
	int maxSockNumber;
	std::map<int,waveBufferInfoStruct> bufferWaves;	//socket descriptor, wave buffer, containing the recv messages.
	
}currentConnections, *currentConnectionsPtr;


/* in SOCKITopenConnection.c */
#include "XOPStructureAlignmentTwoByte.h"	// All structures passed to Igor are two-byte aligned.
typedef struct SOCKITopenConnectionStruct {
	DOUBLE toPrint;
	Handle tokenizer;
	Handle processor;
	Handle bufferWave;
	DOUBLE port;
	Handle IPaddress;
//	Handle initialMessage;
	DOUBLE retval;		//what socket descriptor is opened.
}SOCKITopenConnectionStruct, *SOCKITopenConnectionStructPtr;
#include "XOPStructureAlignmentReset.h"
int SOCKITopenConnection(SOCKITopenConnectionStructPtr );

/* in SOCKITopenConnection.c */
#include "XOPStructureAlignmentTwoByte.h"	// All structures passed to Igor are two-byte aligned.
typedef struct SOCKITcloseConnectionStruct {
	DOUBLE socketToClose;
	DOUBLE retval;
}SOCKITcloseConnectionStruct, *SOCKITcloseConnectionStructPtr;

#include "XOPStructureAlignmentReset.h"
int SOCKITcloseConnection(SOCKITcloseConnectionStructPtr);

/* in checkRecvData.c */
int checkRecvData();

#include "XOPStructureAlignmentTwoByte.h"	// All structures passed to Igor are two-byte aligned.
typedef struct SOCKITcallProcessorStruct {
	Handle bufferWave;
	DOUBLE entryRow;
}SOCKITcallProcessorStruct, *SOCKITcallProcessorStructPtr;


/* in SOCKITcloseConnection */
void resetMaxSocketNumber();
int SOCKITcloseWorker(int socketToClose);
int checkIfWaveInUseAsBuf(waveHndl wav);

/*in NtoCR.cpp */
char* NtoCR(char*,  char*,char*);

/*in SOCKITsendMsg.cpp*/
#include "XOPStructureAlignmentTwoByte.h"	// All structures passed to Igor are two-byte aligned.
typedef struct SOCKITsendMsgStruct {
	Handle message;
	DOUBLE socketToWrite;
	DOUBLE retval;		
}SOCKITsendMsgStruct, *SOCKITsendMsgStructPtr;

#include "XOPStructureAlignmentReset.h"
int SOCKITsendMsg(SOCKITsendMsgStruct *p);

/* in StringTokenizer.cpp */
void Tokenize(const char *str, vector<string>& tokens,  const char* delimiters);

/* in SOCKITprocessor.cpp */
int checkProcessor(const char *processor, FunctionInfo *fip);
int registerProcessor(long sockNum, const char *processor);
int deRegisterProcessor(long sockNum);

#include "XOPStructureAlignmentTwoByte.h"	// All structures passed to Igor are two-byte aligned.
typedef struct SOCKITprocessorStruct {
	Handle processor;
	DOUBLE sockNum;
	DOUBLE retval;		
}SOCKITprocessorStruct, *SOCKITprocessorStructPtr;
#include "XOPStructureAlignmentReset.h"

int SOCKITregisterProcessor(SOCKITprocessorStruct*);

