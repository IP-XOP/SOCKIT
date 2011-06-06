/*
 *  defines.h
 *  iPeek
 *
 *  Created by andrew on 17/05/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */
 
 #ifdef _MACINTOSH_
#include <netdb.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <sys/time.h>
#include <pthread.h>
//#include "recursiveMutex.h"

#define SOCKET long
#endif

#ifdef _WINDOWS_
#include <winsock2.h>
#include <ws2tcpip.h>
#include <Iphlpapi.h>
#include <conio.h>
#include <time.h>
#include "pthread.h"
#include "sched.h"
#include "semaphore.h"
#define snprintf _snprintf
#define close closesocket
#define read recv
#endif

#include <errno.h>
#include <map>
#include <algorithm>
#include <vector>
#include <string.h>
#include "memutils.h"
#include "NtoCR.h"
#include "StringTokenizer.h"

#include "XOPStandardHeaders.h"

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xPath.h>
#include <libxml/xmlstring.h>
#include <libxml/xpathInternals.h>

#define LIBXML_STATIC


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
#define CANT_START_READER_THREAD 13 + FIRST_XOP_ERR
#define STRING_INCORRECT_LEN_FOR_NUMTYPE 14 + FIRST_XOP_ERR
#define  COULDNT_GET_MUTEX 15 + FIRST_XOP_ERR

#define BUFLEN 4096
#define MAX_URL_LEN 256
#define MAX_MSG_LEN 256
#define BUFFER_WAVE_LEN 3000



