/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

using namespace std;

int main (int argc, char *argv[])
{
  // command line parsing

//-----Create socket for connection to server on client's side (noted as server socket in rest of code)-----
		//create address structure for TCP connection socket
	struct sockaddr srvrSockAddr;
	   //create server socket file descriptor for TCP connection
	int srvrSocketFD = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		//check for error connecting server socket
	if( srvrSocketFD == -1  ) //failed to create socket for TCP connection
	{
		perror("Could not create socket");
		exit(EXIT_FAILURE);
	}

	   //clear sockAddrTCP srvrSockAddr structure
	memset(&srvrSockAddr, 0, sizeof(srvrSockAddr));
	
	srvrSockAddr.sa_family = AF_INET;
	srvrSockAddr.sin_port = htons(INADDR_ANY); //listening port
 	Res = inet_pton(AF_INET, "192.168.1.3", &srvrSockAddr.sin_addr); //set server's socket's address 
		//may need to change the IP address later; Perhaps add parameter to this function

		//Check for conversion of IP address error
	if( Res < 0 )
	{
		perror("Error: 1st parameter is not a valid address family");
		close(srvrSocketFD);
		exit(EXIT_FAILURE);
	}
	else if( Res == 0 )
	{
		perror("Error: A valid IP address was not passed in as the 2nd parameter");
		close(srvrSocketFD);
		exit(EXIT_FAILURE);
	}
	
//-----Create socket to listen for client(s) request on server side (noted as client socket in rest of the code)-----
	struct sockadd clientSockAddr;
	int Res;
	int clientSocketFD = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		//Check for error creating client socket
	if( clientSocketFD == -1 )
	{
		perror("Error: Cannot create socket");
		exit(EXIT_FAILURE);
	}

	memset(&clientSockAddr, 0, sizeof(clientSockAddr));

	clientSockAddr.sa_family = AF_INET; //sin_family instead of sa_family on wiki
	clientSockAddr.sin_port = htons(13728);
	clientSockAddr.sin_addr.s_addr = INADDR_ANY; //may need to switch addresses
		//above statement allows program to work w/o knowing IP address of the machine it was running on

		
//-----Bind server socket to listening port-----
		//&clientSockAddr is the address we bind it to
	if( bind(clientSocketFD, (struct sockaddr *)&clientSockAddr, sizeof(clientSockAddr)) == -1 )
	{
		perror("Error: Binding failed.");
		close(srvrSocketFD);
		exit(EXIT_FAILURE);
	}

		//prepare socket to listen for connections from client(s)
	if( listen(clientSocketFD, 10) == -1 ) //10, because we can queue up to 10 connections
	{
		perror("Error: Listen failed.");
		close(srvrSocketFD);
		exit(EXIT_FAILURE);
	}

	
//-----Accept and connect sockets-----
	for(;;)
	{
			//server tries to accept connection from client
		if( accept(srvrSocketFD, (struct sockaddr *)&clientSockAddr, sizeof(clientSockAddr)) < 0 )
		{
			perror("Error: Accept failed");
			close(srvrSocketFD);
			exit(EXIT_FAILURE);
		}

			//client tries connecting to server socket
		if( connect(clientSocketFD, (struct sockaddr *)&srvrSockAddr, sizeof(srvrSockAddr)) == -1 )
		{
			perror("Error: Failed to connect");
			close(clientSocketFD);
			exit(EXIT_FAILURE);
		}
		
			//Perform read write operations here
		if( shutdown(srvrConnectFD, HUT_RDWR) )
		{
			perror("Cannot shutdown socket");
			close(srvrConnectFD);
			exit(EXIT_FAILURE);
		}
		close(srvrConnectFD); //after all the reads have been done	
	}

	return EXIT_SUCCESS;
}
//*/
