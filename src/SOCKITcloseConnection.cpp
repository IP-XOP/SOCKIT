#include "CurrentConnections.h"
#include "SOCKITcloseConnection.h"

int SOCKITcloseConnection(SOCKITcloseConnectionStruct *p){
	int err = 0;
	
	SOCKET socketToClose = 0;
	
	extern CurrentConnections *pinstance;
	extern pthread_mutex_t readThreadMutex;			
	
	SOCKET ii;
    p->retval = 0;

	if(!pinstance)
		return 0;
	pthread_mutex_lock( &readThreadMutex );

	
	if(!p->socketToClose){
		err = OH_EXPECTED_NUMBER;
		goto done;
	}
	
	socketToClose = (SOCKET)doubleToLong(roundDouble(p->socketToClose));

	if(socketToClose == -1){
		for (ii=0; ii< pinstance->getMaxSockNumber()+1 ; ii+=1){
//			if (FD_ISSET(ii, pinstance->getReadSet()))
//				pinstance->closeWorker(ii);

			pinstance->resetCurrentConnections();
			
		}
	} else {
		if(pinstance->isSockitOpen(p->socketToClose,&socketToClose)){
			p->retval = pinstance->closeWorker(socketToClose);
		} else {
			err = -1;
		}
	}
	
done:
	if(err)
		p->retval = -1;
	pthread_mutex_unlock( &readThreadMutex );
		
		return err;
}

