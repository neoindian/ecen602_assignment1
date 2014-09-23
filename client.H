/*
 * client.H
 *
 *  Created on: 13-Sep-2014
 *      Author: nishant
 */

#ifndef CLIENT_H_
#define CLIENT_H_
#include <pthread.h>
#include "MessageStructures.H"
using namespace msgS;


class Client
{
public:
	//Get Functions
	int getClientPort(void);
	int getClientIp(void);
	//Set functions
	bool setClientPort(const int ClientPort);
	bool setClientIp(const int ClientIp);
	static Client * getInstance(void);
	char * createSbcpMessage(const messageTypes mtype,const std::string data);
	void createSbcpMessageHeader(char * msgPtr,const messageTypes mtype);
	void createSbcpAttribute(char * msgPtr,const messageTypes mtype,const std::string data);
	bool debugServerMessage( char * msg,const int cSock,std::string &cName);
	int createAttribute(char * msgPtr,const std::string cName);
    void createMessageNSend(const messageTypes mtype,const std::string data);
    struct sockaddr_in sockAddr;
	int sockFd_;
    int maxClients_;
    std::string clientName_;
private:
	int ClientPort_;
	int ClientIp_;

	Client(const Client & s);
	Client(){};
	static Client * cInstance_;

};



#endif /* CLIENT_H_ */
