/*
 * Server.H
 *
 *  Created on: 12-Sep-2014
 *      Author: nishant
 */

#ifndef SERVER_H_
#define SERVER_H_

#include <map>
#include <pthread.h>
#include "MessageStructures.H"
void * tFunction(void * tPtr);
#define REQUIREDARGS 4
#define INDEXFORIP 1
#define INDEXFORPORT 2
#define ENDOFTIME 1
using namespace msgS;
class Server
{
public:
	//Get Functions
	int getServerPort(void);
	int getServerIp(void);
	//Set functions
	bool setServerPort(const int serverPort);
	bool setServerIp(const int serverIp);
	bool createClientHandlers (int clientSockFd );
	void debugClientMessage( char * msg,const int cSock,std::string &cName);
	void sendReply(char * msgPtr,int cSock);
	void sendReplyExc(char * msgPtr,int cSock);
	void createSbcpMessageHeader(char * msgPtr,const messageTypes mtype);
	void createSbcpAttribute(char * msgPtr,const messageTypes mtype,
			                    const std::string cName,const nackReasons nr);
    int createAckAttribute(char * msgptr);
    int createNakAttribute(char * msgptr,const nackReasons nr);
    int createOnlineAttribute(char * msgPtr,const std::string cName);
	static Server * getInstance(void);
	bool isClientExisting(const std::string cName);
	inline void mapMutexLock()
	{
		   pthread_mutex_lock(&mapMutex_);

	}
	inline void mapMutexUnlock()
    {
		   pthread_mutex_unlock(&mapMutex_);

    }
	void fetchClientName(int cSock,std::string &cName);
	struct sockaddr_in sockAddr;
	int sockFd_;
    int maxClients_;
    int currentClients_;
    std::map<std::string,int> clientMap_;

private:
	int serverPort_;
	int serverIp_;

	Server(const Server & s);
	Server()
	{
		mapMutex_=PTHREAD_MUTEX_INITIALIZER;
		currentClients_ =0 ;
	};
	static Server * sInstance_;
	pthread_mutex_t mapMutex_;
};



#endif /* SERVER_H_ */
