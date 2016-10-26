/*
 *  Connections.h
 *
 *  Created by andrew on 17/05/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#include "XOPStandardHeaders.h"
#include "defines.h"
#include <iostream>
#include <fstream>

#ifdef WINIGOR
#include "stringutils.h"
#endif

double roundDouble(double val);
void *readerThread(void*);
int GetTheTime(long *year, long *month, long *day, long *hour, long *minute, long *second);
void GetTimeStamp(char timestamp[101]);


/**
*Stores relevant information about what to do with the messages received from the socket.
*/

class waveBufferInfo {
	public:
	waveHndl bufferWaveRef;				/**<stores output from the socket. */ 
	bool toPrint;						/**<if set to true then incoming messages from the socket are printed in the history area.*/
	char processor[MAX_OBJ_NAME+1];		/**<the name of an IGOR function that is notified when messages are received*/
	char tokenizer[31];					/**<the output from the socket is tokenized using the characters in this array*/
	char hostIP[MAX_URL_LEN+1];
	char port[PORTLEN + 1];
	SOCKET sockNum;
	int sztokenizer;
	bool toClose;
	string readBuffer;
	
	bool NOIDLES;						//NOIDLES will mean that the user picks up messages with SOCKITpeek, the 

	char logFilePath[MAX_PATH_LEN + 1];
	ofstream *logFile;
	
	waveBufferInfo(){
		sockNum = 0;
		memset(port, 0, sizeof(char) * (PORTLEN + 1));
		bufferWaveRef = NULL;
		toPrint = true;
		memset(processor, 0 , sizeof(char) * (MAX_OBJ_NAME + 1));
		memset(tokenizer,0, sizeof(char) * 31);
		memset(hostIP, 0, sizeof(char) * (MAX_URL_LEN + 1));
		sztokenizer = 0;
		toClose= false;
		NOIDLES = false;
		
		memset(logFilePath, 0, sizeof(char) * (MAX_PATH_LEN + 1));
		ofstream *logFile = NULL;
	};
	
	~waveBufferInfo(){
		if(bufferWaveRef){
			ReleaseWave(&bufferWaveRef);
			bufferWaveRef = NULL;
		}
		
		readBuffer.clear();
		if(logFile){
			if(logFile->is_open())
				logFile->close();
			delete logFile;
		}
		
	}
	
	//isSend = 1 -> msg sent
	//isSend = 0 -> msg recv
	//isSend = -1 -> close socket
	//isSend = -2 -> open socket
	int log_msg(const char *msg, int isSend);

};

#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
/**
*The structure definition for calling the IGOR processor function.
*/
typedef struct SOCKITcallProcessorStruct {
	Handle bufferWave;	/**<The bufferwave into which the message was placed*/
	double entryRow;	/**<The row in the bufferwave into which the message is placed*/
}SOCKITcallProcessorStruct, *SOCKITcallProcessorStructPtr;
#pragma pack()


/**
*A singleton class that holds all information about currently open sockets.
*/
class CurrentConnections{
	public:
	/**
	*A static initialiser for this singleton class.
	*/
	static void Instance();
	/**
	*A destructor. The destructor closes all open connections.
	*/
	~CurrentConnections();
	
	void resetCurrentConnections();	/**<Closes all open connections.  Useful for when the XOP receives the NEW or CLEANUP message.*/
	
	/**
	*Get the number of the highest socket descriptor open.
	*@return the highest socket descriptor open.
	*/
	SOCKET getMaxSockNumber();
	
	/**
	*Updates the number of the highest socket descriptor open.
	*/
	void resetMaxSocketNumber();
	
	/**
	*finds out whether a given socket is open
	*@param query The sockit being queried
	*@param sockNum a pointer to an integer containing the valid sockit number.
	*@return 1 if open, 0 if closed
	*/
	int isSockitOpen(double query, SOCKET *sockNum);
	
	/**
	*Closes an open socket.
	*@param sockNum The socket descriptor to close.
	*@return 0 if the shutdown was successful. -1 if sockNum was not open in the first place.
	*/
	int closeWorker(SOCKET sockNum);
	/**
	*Adds an open socket to the CurrentConnections object.
	*@param sockNum The socket descriptor that has just been opened.
	*@param bufferInfo Information on what to do with incoming messages.
	*@param bufWaveH The text wave designated to act as a buffer for the incoming messages.
	*@see waveBufferInfo
	*/	
	int addWorker(SOCKET sockNum, waveBufferInfo &bufferInfo, waveHndl bufWaveH);

	/**
	*Reports to IGOR whether the wave is in use.  IGOR sends the OBJINUSE message to the XOP.  Here we check
	*if its currently being used.
	*@param wav The wave that IGOR wants to kill.
	*@return 0 if the wave is not in use, 1 if the wave is in use.
	*/
	int checkIfWaveInUseAsBuf(waveHndl wav);

	/**
	*Registers an IGOR processor function with an open socket.
	*@param sockNum The socket on which the processor will work.
	*@param processor The name of the IGOR processor function.  Must have the signature f(textWave,variable).
	*@return 0 if the registration is successful, otherwise non-zero.
	*/
	int registerProcessor(SOCKET sockNum, const char* processor);

	/**
	*Checks whether the IGOR processor registered for a given socket is callable.  This method checks whether the 
	*IGOR function registered on a given socket is of the right signature and is currently compiled.
	*@param sockNum The socket whose processor you are intending to call.
	*@param fip The FunctionInfo structure to send to the XOP toolkit CallFunction function.
	*@return 0 if the processor is compiled and has the right signature, otherwise non-zero.
	*/	
	int checkProcessor(SOCKET sockNum, FunctionInfo* fip);

	/**
	*Returns a pointer to the file descriptor set (fd_set) of sockets that are currently open.
	*@return a pointer to the file descriptor set (fd_set) of sockets that are currently open.
	*/		
	fd_set* getReadSet();
	
	/**
	*Sends a signal that you want the read thread to stop.
	*/		
	void quitReadThread();
	
	/**
	 says if you are currently using the processor
	 */
	bool usingProcessor;
	
	/**
	*Get the flag status to see if you want to quit the reader thread.
	*/		
	bool quitReadThreadStatus();
	
	/**
	*Get the waveBufferInfo for a given socket.
	*@param sockNum the socket that you are interested in.
	@return a pointer to the waveBufferInfo for a given socket.
	*@see waveBufferInfo 
	*/
	waveBufferInfo* getWaveBufferInfo(SOCKET sockNum);
	/**
	*checks whether any of the sockets have any data waiting to be recv().  When IGOR sends the IDLE
	*message to the XOP (~1/20 second) this method checks for new messages waiting on the socket.  If there are any it reads them
	*and then distributes the output.
	*@return 0 if there are no errors.
	*@see outputBufferDataToWave
	*/
	int checkRecvData();

	/**
	*Distributes the bufferData from a given socket in the required way. It accesses the waveBufferInfo object for a given class to see where the
	*data should go.  These places are (1) the history window (2) The bufferWave and (3) the processor function
	*@param sockNum The socket on which the data has been received.
	*@param bufferData A pointer to the data to be output to IGOR.
	*return 0 if there are no errors.
	*@see waveBufferInfo
	*@see registerProcessor()
	*@see checkProcessor()
	*/
	int outputBufferDataToWave(SOCKET sockNum, const unsigned char *bufferData, size_t szbufferdata, bool useProcessor);

	long getTotalSocketsOpened();
	
	//return the number of sockets opened at the present time.
	long getCurrentSocketsOpened();
	
	void getListOfOpenSockets(vector<SOCKET>&);
	
	private:
	/**
	*Constructor for the singleton class.  Only callable from Instance().  It checks to see if there are currently any other CurrentConnections objects created first.
	*@see Instance()
	*/
	CurrentConnections();
	CurrentConnections(const CurrentConnections&){};
	CurrentConnections& operator=(const CurrentConnections&);
	
	/**
	*constains references to the open sockets.  Used with select() to see if the socket is readable or writable
	@see select()
	@see FD_ISSET()
	@see FD_CLR()
	@see FD_SET()
	*/
	fd_set readSet;
	/**
	*THe highest socket descriptor that is currently open.
	*/
	SOCKET maxSockNumber;
	
	//the total number of sockets opened
	long totalSocketsOpened;
	
	/**
	*Quit the thread that reads all the messages
	*/
	bool quitReadThreadFlag;
	
	/**
	*links the waveBufferInfo to a given socket number.
	*/
	std::map<SOCKET, waveBufferInfo> bufferWaves;	//socket descriptor, wave buffer, containing the recv messages.

	};

extern CurrentConnections *pinstance;
extern pthread_t *readThread;
extern pthread_mutex_t readThreadMutex;
extern bool SHOULD_IDLE_SKIP;

