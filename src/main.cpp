
//connect socket (specify timeout for connection?)
//send to socket
//receive from socket in idle time+ after send
//get list of connected sockets
//get info on specific socket
//close socket. (close all sockets
#include "SOCKIT.h"
#include "SOCKITsendmsg.h"
#include "SOCKITopenConnection.h"
#include "SOCKITprocessor.h"
#include "SOCKITcloseConnection.h"
#include "SOCKITsendnrecv.h"
#include "SOCKITisItOpen.h"

/*
variable sock
make/t buf
sockitopenconnection/TOK="\r\n" sock,"www.wavemetrics.com",80,buf
sockitsendnrecv sock, "GET / \r\n"
*/

#ifdef _MACINTOSH_
HOST_IMPORT int main(IORecHandle ioRecHandle);
#endif	
#ifdef _WINDOWS_
WSADATA globalWsaData;
HOST_IMPORT void main(IORecHandle ioRecHandle);
#endif

static int XOPIdle(){
//this function should go through all the sockets and see if there are any messages.
//close off any that don't reply

//check if any have closed, check if there are messages to receive.
	int err = 0;
	unsigned long ticks = 0;							// Current tick count.
	static unsigned long lastTicks = 0;				// Ticks last time XOP idled.
		
/*	#ifdef _MACINTOSH_
		ticks = TickCount();						// Find current ticks.
		if (ticks < lastTicks+60)					// Update every second.
			return err ;
			XOPNotice("Idling\r");
	#endif
	
	#ifdef _WINDOWS_
		ticks = GetTickCount();						// Find current ticks.
		if (ticks < lastTicks+500)					// Update every second.
			return err;
	#endif
*/	
	extern CurrentConnections* pinstance;
	err = pinstance->checkRecvData();
	lastTicks = ticks;


	return err;
}

static int
RegisterOperations(void)		// Register any operations with Igor.
{
	int result;
	
	// Register XOP1 operation.
	if (result = RegisterSOCKITopenconnection())
		return result;
	if (result = RegisterSOCKITsendnrecv())
		return result;
	if (result = RegisterSOCKITsendmsg())
		return result;
		
	// There are no more operations added by this XOP.
	
	return 0;
}

static long
RegisterFunction()
{
	int funcIndex;

	/*	NOTE:
		Most XOPs should return a result of NIL in response to the FUNCADDRS message.
		See XOP manual "Restrictions on Direct XFUNCs" section.
	*/
    
	funcIndex = GetXOPItem(0);		/* which function invoked ? */
	switch (funcIndex) {
		case 0:
			return((long)SOCKITcloseConnection);
			break;
		case 1:
			return((long)SOCKITisItOpen);
			break;
		case 2:
			return((long)SOCKITregisterProcessor);
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
	long result = 0;
	
	extern CurrentConnections* pinstance;
	
	waveHndl wav;

	switch (GetXOPMessage()) {
		case NEW:
			pinstance->resetCurrentConnections();            
			break;
		case CLEANUP:
			pinstance->resetCurrentConnections();
			delete pinstance;

#ifdef _WINDOWS_
			WSACleanup( );
#endif
			break;
		case OBJINUSE:
			//if we're going to tell it to write to buffer, then you can't get rid of the buffer.
			wav = (waveHndl) GetXOPItem(0);
			if(pinstance->checkIfWaveInUseAsBuf(wav))
				result = WAVE_IN_USE;
			break;
		case FUNCADDRS:
                result = RegisterFunction();
			break;
		case IDLE:
			result = XOPIdle();
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

#ifdef _WINDOWS_
HOST_IMPORT void main(IORecHandle ioRecHandle)
#endif
#ifdef _MACINTOSH_
HOST_IMPORT int main(IORecHandle ioRecHandle)
#endif
{	
	XOPInit(ioRecHandle);							/* do standard XOP initialization */
	SetXOPEntry(XOPEntry);							/* set entry point for future calls */
	SetXOPType((long)(RESIDENT | IDLES));			// Specify XOP to stick around and to receive IDLE messages.

	extern CurrentConnections *pinstance;
	CurrentConnections::Instance();
	
	pinstance->resetCurrentConnections();
	
	long result = 0;
	
#ifdef _WINDOWS_
	WORD wVersionRequested;
	WSADATA wsaData;
	//extern WSADATA globalWsaData;

	if(WSAStartup(MAKEWORD(2, 2), &wsaData)){
		WSACleanup( );				
		SetXOPResult(NO_WINSOCK);
		goto done;
	}
#endif

	if (igorVersion < 504){
		SetXOPResult(REQUIRES_IGOR_500);
		goto done;
	} else
		SetXOPResult(0L);
	
	if (result = RegisterOperations())
		SetXOPResult(result);

done:

#ifdef _MACINTOSH_
		return 0;
#else
	return;
#endif
}
