#include "SOCKIT.h"
#include "SOCKITcloseConnection.h"

int SOCKITcloseConnection(SOCKITcloseConnectionStruct *p){
	int err = 0;
	
	SOCKET socketToClose = 0;
	
	extern CurrentConnections *pinstance;
	SOCKET ii;
    p->retval = 0;
    char  report[MAX_MSG_LEN+1];
	
	if(!p->socketToClose){
		err = OH_EXPECTED_NUMBER;
		goto done;
	}
	
	socketToClose = (SOCKET)p->socketToClose;

	if (socketToClose = -1){
		for (ii=0; ii< pinstance->getMaxSockNumber()+1 ; ii+=1){
			if (FD_ISSET(ii, pinstance->getReadSet())) { 
				pinstance->closeWorker(socketToClose);
				snprintf(report,sizeof(report),"SOCKITmsg:  Closed connection to socket descriptor %d\r", ii);
				XOPNotice(report);
			} 
		}
	} else {
		if (FD_ISSET(ii, pinstance->getReadSet())) { 
			p->retval = pinstance->closeWorker(socketToClose);
			snprintf(report,sizeof(report),"SOCKITmsg:  Closed connection to socket descriptor %d\r", socketToClose);
			XOPNotice(report);
		} else {
			snprintf(report,sizeof(report),"SOCKIT err: there is no open socket with descriptor number %d\r", socketToClose);
			XOPNotice(report);
			err = -1;
		}
	}
	
	
done:
		if(err)
            p->retval = -1;
            
		return err;
}

