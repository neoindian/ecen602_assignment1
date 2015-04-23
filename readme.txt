ECEN602 HW1 Programming Assignment
----------------------------------

Description/Comments:
--------------------
1. The client and server are implemented using C++ .
2. The client and server functionalities are encapsulated in a singleton
C++ class . Singleton design pattern was decided so as to encapsulate all
server and client data per binary run instance.
3. Each server contains a C++ STL map with key as the client username and 
the value as the client socket associated with that client name.
4. Whenever a client requests a join with the server, the server first 
verifies if maximum client limit is reached and whether the requesting client's name
already exists in the map . If it does exist , then server sends a NACK to the client
with the appropriate reason ("Max clients reached" ) or ("user name already exists")
to the client. 
5. But if the client JOIN request goes through then the server sends an ACK with the client count and the list of clients to the requesting client.
6. Whenever a client closes the connection, the client is removed from the map 
and any new user can connect with the server by using the previous username again. 
7. Once a client is successfully connected with the server,the client then does a select call on the stdin and the server socket with a timeout of 10 seconds.
8. If no data is received after the select times out, then the client sends an IDLE
message to the SERVER. The SERVER receives this client message and then forwards it to the other users/clients except the one from which the original IDLE message was received.
9. When a client closes the connection to the server, the server cleans up the resources associated with the client and sends an OFFLINE message with the
clients name to the other connected clients to let them know that the client has exited. 
10. The client and server has been tested on the same machine on the wireless network id and on loopback id.
11. The client and server connectivity has also been tested between a virtual ubuntu running in virtual box and the host machine .


Unix command for starting server:
------------------------------------------
./server SERVER_IP SERVER_PORT MAX_CLIENTS

Unix command for starting client:
------------------------------------------
./client USERNAME SERVER_IP SERVER_PORT
