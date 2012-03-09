//
//connect socket (specify timeout for connection?)
//send to socket
//receive from socket in idle time+ after send
//get list of connected sockets
//get info on specific socket
//close socket. (close all sockets
#include "CurrentConnections.h"
#include "SOCKITsendmsg.h"
#include "SOCKITopenConnection.h"
#include "SOCKITprocessor.h"
#include "SOCKITcloseConnection.h"
#include "SOCKITsendnrecv.h"
#include "SOCKITisItOpen.h"
#include "SOCKITstringtoWave.h"
#include "SOCKITwaveToString.h"
#include "SOCKITpeek.h"
#include "SOCKITinfo.h"
#include "SOCKITlist.h"

/*
variable sock
make/t buf
sockitopenconnection/TOK="\r\n" sock,"www.wavemetrics.com",80,buf
sockitsendnrecv sock, "GET / \r\n"
*/


static int XOPIdle(){
//this function should go through all the sockets and see if there are any messages.
//close off any that don't reply

//check if any have closed, check if there are messages to receive.
	int err = 0;
	
	extern CurrentConnections* pinstance;
	extern pthread_mutex_t readThreadMutex;
	extern bool SHOULD_IDLE_SKIP;
	
	if(SHOULD_IDLE_SKIP)
		return 0;
	
	pthread_mutex_lock( &readThreadMutex );

	err = pinstance->checkRecvData();
	
done:
	pthread_mutex_unlock( &readThreadMutex);
	return err;
}

static int
RegisterOperations(void)		// Register any operations with Igor.
{
	int result;
	
	if (result = RegisterSOCKITopenconnection())
		return result;
	if (result = RegisterSOCKITsendnrecv())
		return result;
	if (result = RegisterSOCKITsendmsg())
		return result;
	if (result = RegisterSOCKITstringtoWave())
		return result;
	if (result = RegisterSOCKITwaveToString())
		return result;	
	if (result = RegisterSOCKITlist())
		return result;	
	return 0;
}

static long
RegisterFunction()
{
	int funcIndex;
    
	funcIndex = (int) GetXOPItem(0);		/* which function invoked ? */
	switch (funcIndex) {
		case 0:
			return((XOPIORecResult)SOCKITcloseConnection);
			break;
		case 1:
			return((XOPIORecResult)SOCKITisItOpen);
			break;
		case 2:
			return((XOPIORecResult)SOCKITregisterProcessor);
            break;
		case 3:
			return((XOPIORecResult)SOCKITpeek);
			break;
		case 4:
			return((XOPIORecResult)SOCKITsendmsgF);
			break;
		case 5:
			return((XOPIORecResult)SOCKITsendnrecvF);
			break;
		case 6:
			return((XOPIORecResult)SOCKITopenconnectionF);
			break;
		case 7:
			return((XOPIORecResult)SOCKITtotalOpened);
			break;
		case 8:
			return((XOPIORecResult)SOCKITcurrentOpened);
			break;			
		case 9:
			return((XOPIORecResult)SOCKITinfo);
			break;
			
	}
	return(NIL);
}

/*	XOPEntry()

	This is the entry point from the host application to the XOP for all messages after the
	INIT message.
*/

static void
XOPEntry(void)
{	
	XOPIORecResult result = 0;
	
	extern CurrentConnections* pinstance;
	extern pthread_t *readThread;
	extern pthread_mutex_t readThreadMutex;
	
	waveHndl wav;
	int _message = GetXOPMessage();

	switch (_message) {
		case NEW:
			pthread_mutex_lock( &readThreadMutex );
			pinstance->resetCurrentConnections();            
			pthread_mutex_unlock( &readThreadMutex );
			break;
		case CLEANUP:
			if(readThread){
				pthread_mutex_lock( &readThreadMutex );
//				pthread_cancel(*readThread);
				pinstance->quitReadThread();
				pthread_mutex_unlock( &readThreadMutex );

				int res=0;
				pthread_join(*readThread, (void**)res);
			}
			if(readThread)
				free(readThread);
			
			//don't unlock the mutex again or it is possible threadsafe functions
			//can start working.
			pthread_mutex_lock( &readThreadMutex );
			pinstance->resetCurrentConnections();
			delete pinstance;
			pinstance = NULL;

#ifdef WINIGOR
//			pthread_win32_process_detach_np();
			WSACleanup( );
#endif
			break;
		case OBJINUSE:
			//if we're going to tell it to write to buffer, then you can't get rid of the buffer.
			wav = (waveHndl) GetXOPItem(0);
			
			if(!pthread_mutex_trylock( &readThreadMutex )){
				if(pinstance->checkIfWaveInUseAsBuf(wav))
					result = WAVE_IN_USE;
				pthread_mutex_unlock( &readThreadMutex );
			} else {
				result = WAVE_IN_USE;
			}
	
			break;
		case FUNCADDRS:
			if(pinstance)
                result = RegisterFunction();
			break;
		case IDLE:
			if(pinstance)					
				result = XOPIdle();
			else {
				result = 0;
			}

			break;		
		default:
			break;
	}
	SetXOPResult(result);
}


/*	main(ioRecHandle)

	This is the initial entry point at which the host application calls XOP.
	The message sent by the host must be INIT.
	main() does any necessary initialization and then sets the XOPEntry field of the
	ioRecHandle to the address to be called for future messages.
*/


HOST_IMPORT int main(IORecHandle ioRecHandle)
{	
	XOPInit(ioRecHandle);							/* do standard XOP initialization */
	SetXOPEntry(XOPEntry);							/* set entry point for future calls */
	SetXOPType((long)(RESIDENT | IDLES));			// Specify XOP to stick around and to receive IDLE messages.

	long result = 0;

//	#ifdef _WINDOWS_
//		pthread_win32_process_attach_np();
//	#endif

	extern pthread_t *readThread;
	extern CurrentConnections *pinstance;
	extern pthread_mutex_t readThreadMutex;

	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	pthread_mutexattr_settype( &attr, PTHREAD_MUTEX_RECURSIVE );
	pthread_mutex_init( &readThreadMutex, &attr );

	pthread_mutex_lock( &readThreadMutex );
	CurrentConnections::Instance();
	pinstance->resetCurrentConnections();
	pthread_mutex_unlock( &readThreadMutex );
	
	readThread = (pthread_t*)malloc(sizeof(pthread_t));
	if(readThread == NULL){
		SetXOPResult(NOMEM);
		return EXIT_FAILURE;
	}
	if(pthread_create( readThread, NULL, &readerThread, NULL)){
		SetXOPResult(CANT_START_READER_THREAD);
		return EXIT_FAILURE;
	}
	
#ifdef WINIGOR
	WORD wVersionRequested;
	WSADATA wsaData;
	//extern WSADATA globalWsaData;

	if(WSAStartup(MAKEWORD(2, 2), &wsaData)){
		WSACleanup( );				
		SetXOPResult(NO_WINSOCK);
		return EXIT_FAILURE;
	}
#endif

	if (igorVersion < 600){
		SetXOPResult(IGOR_OBSOLETE);
		return EXIT_FAILURE;
	}
	
	if (result = RegisterOperations()){
		SetXOPResult(result);
		return EXIT_FAILURE;
	}

	SetXOPResult(0);
	return EXIT_SUCCESS;
}
