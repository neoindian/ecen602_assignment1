//============================================================================
// Name        : Client.cpp
// Author      : Nishant Pattanaik
// Version     : 1
// Copyright   : This program is being created for the multiple client
//               implementation for ECEN 602  HW1
// Description : Client implementation for the project
//============================================================================

#include "HeaderInclusions.H"
#include "client.H"
#include <sstream>
using namespace std;

#define REQUIREDARGS 4
#define INDEXFORUNAME 1
#define INDEXFORIP 2
#define INDEXFORPORT 3
#define ENDOFTIME 1
#define CLIENTIDLETIME 100
#define STDIN 0
Client * Client::cInstance_ = NULL;



void handleError(string errStr)
{
	perror(errStr.c_str());
	exit(1);
}

Client * Client::getInstance(void)
{
	if(!cInstance_)
	{
		cInstance_ = new Client();

	}
	return cInstance_;
}

int main (int argc,char * argv[])
{
   if(argc != REQUIREDARGS )
   {
	 cout<<"Wrong usage of the program"<<endl;
	 cout<<"Proper usage is ./client <client user name> <ip-address> <port> "<<endl;
   }
   Client * cObject = Client::getInstance();
   struct sockaddr_in serverAddr;
   socklen_t serverSize = sizeof(serverAddr);

   //Get the clients user name
   cObject->clientName_ = argv[INDEXFORUNAME];
   bzero(&(cObject->sockAddr),sizeof(cObject->sockAddr));
   //the 0 indicates the default protocol under AF_INET
   cObject->sockFd_ = socket(AF_INET,SOCK_STREAM,0);
   if(cObject->sockFd_ == -1)
   {
	   handleError("Socket creation failed ");
   }
   //Fetching and storing the server ip
   string ipS = argv[INDEXFORIP];
   if(inet_aton(ipS.c_str(),&(cObject->sockAddr.sin_addr))==0)
   {
	   handleError("inet_aton");
   }
   //cout<<inet_ntoa(cObject->sockAddr.sin_addr)<<endl;

   //Fetching and setting the Client port number
   int portNo = atoi (argv[INDEXFORPORT]);
   cObject->sockAddr.sin_port = htons(portNo);
   cout<<ntohs(cObject->sockAddr.sin_port)<<endl;

   //Fetching and storing number of queued connections
   cObject->sockAddr.sin_family = AF_INET;
   fd_set fdSet;
   struct timeval tStruct;

   //Initialize the timeout data structure.
   tStruct.tv_sec = 10;
   tStruct.tv_usec = 0;
   //char serverMessage[2048];
   int rMsgSize=0;
   if (connect(cObject->sockFd_, (const struct sockaddr *)&(cObject->sockAddr),serverSize) == -1)
   {
      close(cObject->sockFd_);
      handleError("Connect to server failed");
   }
   else
   {
	   cout<<"Connected to server at IP " <<inet_ntoa(cObject->sockAddr.sin_addr)\
			<<" and Port "<< ntohs(cObject->sockAddr.sin_port) <<endl;

   }
   cObject->createMessageNSend(JOIN,cObject->clientName_);
   char * serverMessage = (char *)malloc(MAXMSGSIZE);
   FD_ZERO (&fdSet);
   FD_SET (cObject->sockFd_, &fdSet);
   FD_SET (fileno(stdin),&fdSet);
   int maxFd = (fileno(stdin)<cObject->sockFd_)?cObject->sockFd_+1:fileno(stdin)+1;
   int clientIdleTime=0;
   string cName;
   int bytesRead=0;
   int retVal;
   cout<<"Enter a message on console and press enter to send: "<<endl;
   while (ENDOFTIME)
   {
	     bzero(serverMessage,MAXMSGSIZE);
	     FD_ZERO (&fdSet);
	     FD_SET (cObject->sockFd_, &fdSet);
	     FD_SET (fileno(stdin),&fdSet);
	     maxFd = (fileno(stdin)<cObject->sockFd_)?cObject->sockFd_+1:fileno(stdin)+1;
         bytesRead = 0;
         tStruct.tv_sec = 10;
         tStruct.tv_usec = 0;

         cName="";

         retVal = select(maxFd+1,&fdSet, NULL, NULL,&tStruct);
         if	(retVal==-1)
	     {
            perror("Error in select ") ;
            continue;
	     }
         else if (retVal==0)
         {

        	 cObject->createMessageNSend(IDLE,cObject->clientName_);
        	 continue;
         }
         else
         {
			 if(FD_ISSET(cObject->sockFd_,&fdSet))
			 {


				if((bytesRead=recv(cObject->sockFd_,serverMessage,MAXMSGSIZE,0)) ==-1)
				{
					perror("Read error");
					continue;
				}
				else
				{


					if(!cObject->debugServerMessage( serverMessage,cObject->sockFd_,cName))
					{
						cout <<"Time to stop the connection "<<endl;
						break;
					}

				}
			  }
			 else if(FD_ISSET(STDIN,&fdSet))
			 {

				bzero(serverMessage,MAXMSGSIZE);
				if((bytesRead=read(fileno(stdin),serverMessage,MAXMSGSIZE)) ==-1)
				{
					perror("Read error");
					continue;
				}
				else
				{

					string data = serverMessage;
					cObject->createMessageNSend(SEND,data);
					if(clientIdleTime>0)
					{
						clientIdleTime--;
					}
				}

			 }
			 else
			 {

			 }
         }//end of if(rv)
     }//end of while(ENDOFTIME)
   //cout<<"Closing client because it has been idle for 10 seconds";
   close(cObject->sockFd_);
   return 0;

}
void Client::createMessageNSend(const messageTypes mtype,const std::string data)
{
    char * msg = createSbcpMessage(mtype,data);
    sbcpMessageHeader * msgHdr = (sbcpMessageHeader * )msg;
    msgHdr->length_ += ((sbcpMessageAttribute *)(msg+sizeof(sbcpMessageHeader)))->length_;
    send(sockFd_,msg,msgHdr->length_,0);
    free(msg);
}

char * Client::createSbcpMessage(const messageTypes mtype,const std::string data)
{
	char * msgPtr;
           msgPtr=(char *)malloc(MAXMSGSIZE);

	switch (mtype)
	{
	   case JOIN:
	   case IDLE:
	   {
		     createSbcpMessageHeader(msgPtr,mtype);
		     createSbcpAttribute(msgPtr+ sizeof(sbcpMessageHeader),mtype,data);
		     char * payload = msgPtr + sizeof(sbcpMessageHeader) +sizeof(sbcpMessageAttribute);
	         memcpy(payload,clientName_.c_str(),clientName_.length());

	   }
		     break;
	   case SEND:
	   {
		     createSbcpMessageHeader(msgPtr,mtype);
		     createSbcpAttribute(msgPtr+ sizeof(sbcpMessageHeader),mtype,data);
		     char * payload = msgPtr + sizeof(sbcpMessageHeader) +sizeof(sbcpMessageAttribute);
	         memcpy(payload,data.c_str(),data.length());


	   }
		   break;
	   default :
		     break;
	}
	return msgPtr;
}

void Client::createSbcpMessageHeader(char * msgPtr,const messageTypes mtype)
{
    sbcpMessageHeader  *msgHdr= (sbcpMessageHeader*)msgPtr;


    msgHdr->version_ = 3;
    msgHdr->type_ = mtype;
    msgHdr->length_=sizeof(sbcpMessageHeader);


}

void Client::createSbcpAttribute(char * msgPtr,const messageTypes mtype,const string data)
{
	sbcpMessageAttribute * msgAttr= (sbcpMessageAttribute *)msgPtr;
    int attrLength =0;

    switch(mtype)
    {
    case JOIN:
    case IDLE:
    	msgAttr->type_ = UNAME;
    	attrLength= createAttribute(msgPtr+sizeof(sbcpMessageAttribute),clientName_);
    	msgAttr->length_ = sizeof(sbcpMessageAttribute) + attrLength;
    	break;
    case SEND:
    	msgAttr->type_ = MSG;
    	attrLength= createAttribute(msgPtr+sizeof(sbcpMessageAttribute),data);
    	msgAttr->length_ = sizeof(sbcpMessageAttribute) + attrLength;
    	break;

    default :
    	break;
    }

}
int Client::createAttribute(char * msgPtr,const string data)
{
	memcpy(msgPtr,data.c_str(),data.length());
	return data.length();
}


bool Client::debugServerMessage( char * msg,const int cSock,string &cName)
{
	bool retVal = true;
	char * msgPtr = msg;
	char messageFwd[MAXMSGSIZE]={0};
	int messageSize =0;
	sbcpMessageHeader * msgHdr = (sbcpMessageHeader *)msgPtr;
	switch(msgHdr->type_)
	{
	   case FWD:
	   {

		   int totalMessageLength = msgHdr->length_ - sizeof(sbcpMessageHeader);


		   string messageServ="";
		   sbcpMessageAttribute *msgAttr = (sbcpMessageAttribute *)((char *)msgPtr+sizeof(sbcpMessageHeader));

		   while(totalMessageLength >0)
		   {
               bzero(&messageFwd,MAXMSGSIZE);

			   if((msgAttr->type_ ==  MSG) || (msgAttr->type_==UNAME))
			   {


				   messageSize = (msgAttr->length_) - sizeof(sbcpMessageAttribute);
				   char * ptr = (char *) ((char *)msgAttr+sizeof(sbcpMessageAttribute));
				   totalMessageLength -= sizeof(sbcpMessageAttribute);

				   memcpy(messageFwd,ptr,messageSize);

				   std::string msgStr = messageFwd;
				   int pos;
				   if((pos=msgStr.find('\n')) != string::npos)
				      msgStr.erase(pos);

				   messageServ.append(msgStr);
				   messageServ.append(" ");

				   totalMessageLength -= messageSize;
				   if(totalMessageLength !=0)
				   {
					   messageServ.append("from ");
				   }

			   }

               msgAttr = (sbcpMessageAttribute *)((char *)msgAttr+msgAttr->length_);
		   }
		   cout<<messageServ<<endl;
	   }
		   break;
	   case ONLINE:

	   {
		   sbcpMessageAttribute *msgAttr = (sbcpMessageAttribute *)((char *)msgPtr+sizeof(sbcpMessageHeader));
  		   if(msgAttr->type_ ==  UNAME)
   		   {
              //Get the client name aka user name
			   messageSize = (msgAttr->length_) - sizeof(sbcpMessageAttribute);
  			   fflush(stdout);

  			   char * ptr = (char *) (msgPtr+sizeof(sbcpMessageHeader)+sizeof(sbcpMessageAttribute));
  			   memcpy(messageFwd,ptr,messageSize);
   			   cout<< "Client "<<messageFwd <<" is ONLINE "<< endl;
   		   }

	   }
		   break;
	   case OFFLINE:
	   {
		   sbcpMessageAttribute *msgAttr = (sbcpMessageAttribute *)((char *)msgPtr+sizeof(sbcpMessageHeader));
  		   if(msgAttr->type_ ==  UNAME)
   		   {
              //Get the client name aka user name
			   messageSize = (msgAttr->length_) - sizeof(sbcpMessageAttribute);
  			   fflush(stdout);

  			   char * ptr = (char *) (msgPtr+sizeof(sbcpMessageHeader)+sizeof(sbcpMessageAttribute));
  			   memcpy(messageFwd,ptr,messageSize);
  			   messageFwd[messageSize+1]='\0';
  			 cout<< "Client "<<messageFwd <<" is OFFLINE "<< endl;
   		   }

	   }
		   break;
	   case IDLE:
	   {
		   sbcpMessageAttribute *msgAttr = (sbcpMessageAttribute *)((char *)msgPtr+sizeof(sbcpMessageHeader));
  		   if(msgAttr->type_ ==  UNAME)
   		   {
              //Get the client name aka user name
			   messageSize = (msgAttr->length_) - sizeof(sbcpMessageAttribute);
  			   fflush(stdout);

  			   char * ptr = (char *) (msgPtr+sizeof(sbcpMessageHeader)+sizeof(sbcpMessageAttribute));
  			   memcpy(messageFwd,ptr,messageSize);
  			 cout<< "Client "<<messageFwd <<" is IDLE "<< endl;
   		   }
	   }
	   break;
	   case NACK:
	   {
		   sbcpMessageAttribute *msgAttr = (sbcpMessageAttribute *)((char *)msgPtr+sizeof(sbcpMessageHeader));
  		   if(msgAttr->type_ ==  REASON)
   		   {
              //Get the server NACK reason
			   messageSize = (msgAttr->length_) - sizeof(sbcpMessageAttribute);
  			   fflush(stdout);

  			   char * ptr = (char *) (msgPtr+sizeof(sbcpMessageHeader)+sizeof(sbcpMessageAttribute));
  			   memcpy(messageFwd,ptr,messageSize);
   			   cout<<" NACK received with reason "<< messageFwd<<endl;
   			   retVal = false;
   		   }

	   }
		   break;
	   case ACK:
	   {
		   sbcpMessageAttribute *msgAttr = (sbcpMessageAttribute *)((char *)msgPtr+sizeof(sbcpMessageHeader));
  		   if(msgAttr->type_ ==  CLICOUNT)
   		   {
              //Get the user names
			   messageSize = (msgAttr->length_) - sizeof(sbcpMessageAttribute);
  			   fflush(stdout);

  			   char * ptr = (char *) (msgPtr+sizeof(sbcpMessageHeader)+sizeof(sbcpMessageAttribute));
  			   memcpy(messageFwd,ptr,messageSize);

   		   }

	   }

		   break;
	   default:
		   break;
	}
	return retVal;
}
