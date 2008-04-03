
//connect socket (specify timeout for connection?)
//send to socket
//receive from socket in idle time+ after send
//get list of connected sockets
//get info on specific socket
//close socket. (close all sockets

#include "SOCKIT.h"

//global variable to hold all the open connections
currentConnections openConnections;

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
//XOPNotice("Idling\r");
//check if any have closed, check if there are messages to receive.
	int err = 0;
	unsigned long ticks = 0;							// Current tick count.
	static unsigned long lastTicks = 0;				// Ticks last time XOP idled.
		
	#ifdef _MACINTOSH_
		ticks = TickCount();						// Find current ticks.
		if (ticks < lastTicks+30)					// Update every second.
			return err ;
	#endif
	
	#ifdef _WINDOWS_
		ticks = GetTickCount();						// Find current ticks.
		if (ticks < lastTicks+500)					// Update every second.
			return err;
	#endif
	
	err = checkRecvData();
	lastTicks = ticks;


	return err;
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
		case 0:						/* str1 = xstrcat0(str2, str3) */
            return((long)SOCKITsendMsg);	/* this uses the direct call method */
			break;
		case 1:
			return((long)SOCKITopenConnection);
			break;
		case 2:
			return((long)SOCKITcloseConnection);
			break;
		case 3:
			return((long)SOCKITregisterProcessor);
		case 4:
			return((long)SOCKITsendnrecv);
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
	extern currentConnections openConnections;
    
    int ii=0;
	waveHndl wav;

	switch (GetXOPMessage()) {
		case NEW:
			FD_ZERO(&(openConnections.readSet));
			openConnections.maxSockNumber = 0;
			
			break;
		case CLEANUP:
			for (ii=0; ii< openConnections.maxSockNumber+1 ; ii+=1){
				if (FD_ISSET(ii, &(openConnections.readSet))) { 
					FD_CLR(ii, &(openConnections.readSet)); 
					close(ii);
				} 
			}
			FD_ZERO((&openConnections.readSet));
			openConnections.maxSockNumber = 0;
#ifdef _WINDOWS_
			WSACleanup( );
#endif
			break;
		case OBJINUSE:
			//if we're going to tell it to write to buffer, then you can't get rid of the buffer.
			wav = (waveHndl) GetXOPItem(0);
			if(checkIfWaveInUseAsBuf(wav))
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

#ifdef _WINDOWS_
	WORD wVersionRequested;
	WSADATA wsaData;
	extern WSADATA globalWsaData;
#endif

	#ifdef _WINDOWS_
	if(WSAStartup(MAKEWORD(2, 2), &wsaData)){
		WSACleanup( );				
		SetXOPResult(NO_WINSOCK);   
	}
	#endif

	if (igorVersion < 200)
		SetXOPResult(REQUIRES_IGOR_200);
	else
		SetXOPResult(0L);
}
