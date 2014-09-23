/*
 * MessageStructures.H
 *
 *  Created on: 12-Sep-2014
 *      Author: nishant
 */

#ifndef MESSAGESTRUCTURES_H_
#define MESSAGESTRUCTURES_H_
// 4 bytes sbcp attribute header, 4 bytes sbcp attribute type and length
// 512 bytes max payload to be supported

#define MAXMSGSIZE 520
//SBCP Message Header Structure
namespace msgS
{
typedef struct
{
	unsigned short int version_:9;
	unsigned short int type_:7;
	unsigned short int length_;
}sbcpMessageHeader;

typedef struct
{
    unsigned short int type_;
    unsigned short int length_;
}sbcpMessageAttribute;


enum sbcpAttributeTypes
{
	REASON=1,
	UNAME,
	CLICOUNT,
	MSG
};
enum nackReasons
{
	EXISTS = 0,
	MAXCLIENTSREACHED
};


enum messageTypes
{
	ACK=0,
	NACK,
	JOIN,
	FWD,
	SEND,
	ONLINE,
	OFFLINE,
	IDLE
};

}


#endif /* MESSAGESTRUCTURES_H_ */
