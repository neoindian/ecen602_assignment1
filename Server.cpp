//============================================================================
// Name        : Server.cpp
// Author      : Nishant Pattanaik
// Version     : 1
// Copyright   : This program is being created for the multiple client server
//               implementation for ECEN 602  HW1
// Description : Server implementation for the
//============================================================================



#include "HeaderInclusions.H"

#include "Server.H"

//#include "pthread.h"

using namespace std;



Server * Server::sInstance_ = NULL;
void handleError(string errStr)
{
	perror(errStr.c_str());
	exit(1);
}

Server * Server::getInstance(void)
{
	if(!sInstance_)
	{
		sInstance_ = new Server();

	}
	return sInstance_;
}

int main (int argc,char * argv[])
{
   if(argc != REQUIREDARGS )
   {
	 cout<<"Wrong usage of the program"<<endl;
	 cout<<"Proper usage is ./server <ip-address> <port> max_no_of_clients"<<endl;
   }
   Server * sObject = Server::getInstance();
   int clientConnFd;
   struct sockaddr_in clientAddr;
   socklen_t clientSize = sizeof(clientAddr);

   bzero(&(sObject->sockAddr),sizeof(sObject->sockAddr));
   bzero(&clientAddr,sizeof(clientAddr));
   //the 0 indicates the default protocol under AF_INET
   sObject->sockFd_ = socket(AF_INET,SOCK_STREAM,0);
   if(sObject->sockFd_ == -1)
   {
	   handleError("Socket creation failed ");
   }
   //Fetching and storing the server ip
   string ipS = argv[INDEXFORIP];
   if(inet_aton(ipS.c_str(),&(sObject->sockAddr.sin_addr))==0)
   {
	   handleError("inet_aton");
   }
   //cout<<inet_ntoa(sObject->sockAddr.sin_addr)<<endl;

   //Fetching and setting the server port number
   int portNo = atoi (argv[INDEXFORPORT]);
   sObject->sockAddr.sin_port = htons(portNo);
   //cout<<ntohs(sObject->sockAddr.sin_port)<<endl;

   //Fetching and storing number of queued connections
   sObject->maxClients_ = atoi(argv[3]);

   sObject->sockAddr.sin_family = AF_INET;
   if(bind(sObject->sockFd_,(struct sockaddr *)&(sObject->sockAddr)
		,sizeof(sObject->sockAddr)) == -1)
   {
	   handleError("Bind failed on server");
   }
   if(listen((sObject->sockFd_),sObject->maxClients_)==-1)
   {
	   handleError("listenError");
   }
   while (ENDOFTIME)
   {
	   clientConnFd = accept(sObject->sockFd_,(struct sockaddr *)&clientAddr,&clientSize);
       if(clientConnFd==-1)
       {
    	   perror("Client connection failed");
    	   continue;
       }
       else
       {
    	   sObject->createClientHandlers(clientConnFd);
       }
   }

   return 0;

}

bool Server::createClientHandlers (int clientSockFd )
{
	int *cSock = (int*)malloc(sizeof(int));
	*cSock = clientSockFd;
	pthread_t cPid;
	if(pthread_create(&cPid,NULL,tFunction,(void*)cSock))
	{
		handleError("Client thread creation failed");
	}
	//Create a detached thread for each client .
	pthread_detach(cPid);
	return true;
}
void * tFunction(void * tPtr)
{

	  int cSock = *((int*)tPtr);
	  int rSize;
	  char  *rMsg = (char *)malloc(MAXMSGSIZE);
      bzero(rMsg,MAXMSGSIZE);
      string cName="";
      nackReasons nr;
	  while((rSize = recv(cSock,(void *)rMsg,MAXMSGSIZE,0))>0)
	  {

		   Server::getInstance()->debugClientMessage(rMsg,cSock,cName);
		   bzero(rMsg,MAXMSGSIZE);
	  }
      if(rSize == 0 || rSize == -1)
	  {

	     //Inform other members that the client is offline
	     fflush(stdout);
    	 bzero(rMsg,MAXMSGSIZE);
		 Server::getInstance()->createSbcpMessageHeader(rMsg,OFFLINE);

		 //Server::getInstance()->createSbcpAttribute( rMsg+ sizeof(sbcpMessageHeader),ONLINE,cName,nr);
		 sbcpMessageHeader * msgHdr = (sbcpMessageHeader *)rMsg;
		 msgHdr->length_ += ((sbcpMessageAttribute *)(rMsg+sizeof(sbcpMessageHeader)))->length_;
         Server::getInstance()->sendReplyExc(rMsg,cSock);

         Server::getInstance()->mapMutexLock();
         std::map<string,int>::iterator it=Server::getInstance()->clientMap_.begin();
         //Erase the client from the map as it no longer exists
         while ( it!=Server::getInstance()->clientMap_.end())
      	 {
      		 if(it->second ==cSock)
      		 {
      			Server::getInstance()->clientMap_.erase(it++);
      		 }
      		 else
      		 {
      			 ++it;
      		 }
      	 }
         Server::getInstance()->currentClients_--;
         Server::getInstance()->mapMutexUnlock();

	   }
      close(cSock);
      free(tPtr);


	return NULL;
}
void Server::debugClientMessage( char * msg,const int cSock,string &cName)
{
	char * msgPtr = msg;
	char clientName[20]={0};
	int clientNameSize =0;
	sbcpMessageHeader * msgHdr = (sbcpMessageHeader *)msgPtr;
    bool doesClientExists=false;
    nackReasons nr;
	switch(msgHdr->type_)
	{
	   case JOIN:
	   {

		   sbcpMessageAttribute *msgAttr =(sbcpMessageAttribute *)((char *)msgPtr+sizeof(sbcpMessageHeader));
		   if(msgAttr->type_ ==  UNAME)
		   {
               //Get the client name aka user name
			   clientNameSize = (msgAttr->length_) - sizeof(sbcpMessageAttribute);
			   fflush(stdout);

			   char * ptr = (char *) (msgPtr+sizeof(sbcpMessageHeader)+sizeof(sbcpMessageAttribute));;
			   memcpy(clientName,ptr,clientNameSize);

               cName=clientName;
               doesClientExists = isClientExisting(clientName);
               currentClients_++;


			   //Send an ACK to client as the connection was accepted
			   if((currentClients_<=maxClients_) && !doesClientExists)
			   {
				   //Insert the username and associated socket in our map

				   mapMutexLock();
				   cout<<"JOIN :: "<<clientName<<endl;
				   clientMap_.insert(std::pair<string,int>(clientName,cSock));
				   mapMutexUnlock();
				   bzero(msgPtr,MAXMSGSIZE);
				   createSbcpMessageHeader(msgPtr,ACK);
				   createSbcpAttribute(msgPtr+ sizeof(sbcpMessageHeader),ACK,clientName,nr);
				   msgHdr->length_ += ((sbcpMessageAttribute *)(msgPtr+sizeof(sbcpMessageHeader)))->length_;
				   sendReply(msgPtr,cSock);
				   //currentClients_++;
				   //Send ONLINE MSG to rest of the connected clients
				   bzero(msgPtr,MAXMSGSIZE);
				   createSbcpMessageHeader(msgPtr,ONLINE);
				   createSbcpAttribute(msgPtr+ sizeof(sbcpMessageHeader),ONLINE,clientName,nr);
				   msgHdr->length_ += ((sbcpMessageAttribute *)(msgPtr+sizeof(sbcpMessageHeader)))->length_;
				   sendReplyExc(msgPtr,cSock);

			   }
			   else
			   {
				   //Send NACK MSG to  client
				   if(currentClients_ >= maxClients_ )
				   {
                      nr = MAXCLIENTSREACHED;
				   }
				   else
				   {
					  nr = EXISTS;
				   }
				   bzero(msgPtr,MAXMSGSIZE);
				   createSbcpMessageHeader(msgPtr,NACK);
				   createSbcpAttribute(msgPtr+ sizeof(sbcpMessageHeader),NACK,clientName,nr);

				   msgHdr->length_ += ((sbcpMessageAttribute *)(msgPtr+sizeof(sbcpMessageHeader)))->length_;
	    		   sendReply(msgPtr,cSock);

			   }

		   }
	   }
	       break;
	   case SEND:
	   {   //Forward the same message to other clients except from which
		   //received

		   msgHdr->type_=FWD;
		   int totalMsgLength = ((sbcpMessageHeader*)msgPtr)->length_;

		   //append a UNAME attribute for the client who sent this message
		   fetchClientName(cSock,cName);
		   createSbcpAttribute(msgPtr+ totalMsgLength,FWD,cName,nr);
		   //update the 1st msgAttr length and the message header length
		   //with the second attribute length
		   sbcpMessageHeader * msgHdr = (sbcpMessageHeader*)msgPtr;
		   sbcpMessageAttribute * secondAttribute = ((sbcpMessageAttribute *)(msgPtr+ totalMsgLength));
           msgHdr->length_ += secondAttribute->length_;

		   sendReplyExc(msg,cSock);
	   }
		   break;
	   case IDLE:
		   //Send an IDLE message to all except who sent

		   sendReplyExc(msg,cSock);
		   break;
	   default:
		   break;
	}
}
void Server::sendReply(char * msgPtr,int cSock)
{
	int length = ((sbcpMessageHeader*)msgPtr)->length_;

	send(cSock , msgPtr, length,0);
}
void Server::sendReplyExc(char * msgPtr,int cSock)
{
	mapMutexLock();
    //std::map<std::string,int> cMap=clientMap_;
	for (std::map<string,int>::iterator it=clientMap_.begin(); it!=clientMap_.end(); ++it)
	{
		 if(it->second !=cSock)
		 {

		   sendReply(msgPtr,it->second);
		 }
	}
    mapMutexUnlock();
}
void Server::fetchClientName(int cSock,std::string &cName)
{
	mapMutexLock();

	for (std::map<string,int>::iterator it=clientMap_.begin(); it!=clientMap_.end(); ++it)
	{
		 if(it->second ==cSock)
		 {

		   cName = it->first;
		 }
	}
    mapMutexUnlock();
}

void Server::createSbcpMessageHeader(char * msgPtr,const messageTypes mtype)
{
    sbcpMessageHeader  *msgHdr= (sbcpMessageHeader*)msgPtr;

    msgHdr->version_ = 3;
    msgHdr->type_ = mtype;
    msgHdr->length_=sizeof(sbcpMessageHeader);

}

void Server::createSbcpAttribute(char * msgPtr,const messageTypes mtype,const string cName,const nackReasons nr)
{
	sbcpMessageAttribute * msgAttr= (sbcpMessageAttribute *)msgPtr;

    int attrLength=0;
    switch(mtype)
    {
    case JOIN:
    	msgAttr->type_ = UNAME;
    	msgAttr->length_ = sizeof(sbcpMessageAttribute) + cName.length();
    	break;
    case ACK:
    	msgAttr->type_ = CLICOUNT;
    	attrLength = createAckAttribute(msgPtr+sizeof(sbcpMessageAttribute));
    	msgAttr->length_ = sizeof(sbcpMessageAttribute) + attrLength;
    	break;
    case NACK:
    	msgAttr->type_ = REASON;
    	attrLength = createNakAttribute(msgPtr+sizeof(sbcpMessageAttribute),nr);
    	msgAttr->length_ = sizeof(sbcpMessageAttribute) + attrLength;
    	break;
    case ONLINE:
    case OFFLINE:
    	msgAttr->type_ = UNAME;
    	attrLength = createOnlineAttribute(msgPtr+sizeof(sbcpMessageAttribute),cName);
    	msgAttr->length_ = sizeof(sbcpMessageAttribute) + attrLength;
        break;
    case FWD:
    	msgAttr->type_ = UNAME;
    	attrLength = createOnlineAttribute(msgPtr+sizeof(sbcpMessageAttribute),cName);
    	msgAttr->length_ = sizeof(sbcpMessageAttribute) + attrLength;
    	break;
    default :
    	break;
    }


}
int Server::createAckAttribute(char * msgPtr)
{

	std::string uNames = "";
	mapMutexLock();
	for (std::map<string,int>::iterator it=clientMap_.begin(); it!=clientMap_.end(); ++it)
	{
	    uNames+=it->first;
	    uNames+=" ";
	}
	mapMutexUnlock();
	memcpy(msgPtr,uNames.c_str(),uNames.length());
	return uNames.length();
}

int Server::createNakAttribute(char * msgPtr,const nackReasons nr)
{
	std::string uNames = "";
	if(nr == EXISTS)
	{
		uNames="Client exists";
	}
	else if (nr == MAXCLIENTSREACHED)
	{
		uNames = "Max clients reached";
	}

	memcpy(msgPtr,uNames.c_str(),uNames.length());

	return uNames.length();
}
int Server::createOnlineAttribute(char * msgPtr,const string cName)
{
	memcpy(msgPtr,cName.c_str(),cName.length());

	return cName.length();
}
bool Server::isClientExisting(const std::string cName)
{
	bool retVal = false;
	mapMutexLock();
    std::map<std::string,int> cMap=clientMap_;
	for (std::map<string,int>::iterator it=cMap.begin(); it!=cMap.end(); ++it)
	{
		 if(it->first ==cName)
		 {

    	   retVal=true;
    	   break;
		 }
	}
	mapMutexUnlock();

	return retVal;
}
