/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
//http://www.cs.rutgers.edu/~pxk/rutgers/notes/sockets/index.html

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

//-----Create socket for connection to server on client's side (cSockAddr)-----
	struct sockaddr cSockAddr;
	int cSocketFD = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); //file descriptor

		//check for error connecting server socket
	if( cSocketFD == -1  ) //failed to create socket for TCP connection
	{
		perror("Could not create socket");
		exit(EXIT_FAILURE);
	}

	   //clear sockAddrTCP cSockAddr structure
	memset(&cSockAddr, 0, sizeof(cSockAddr));
	
	//---initialize socket attributes---
	cSockAddr.sa_family = AF_INET;
	
		//Allowing our OS to assign a port #;
		// We're a client and we won't be receiving any incoming connections,
		// so there is no need to specify a port #
	cSockAddr.sin_port = htonl(INADDR_ANY); 
		
 	Res = inet_pton(AF_INET, "192.168.1.3", &crvrSockAddr.sin_addr); //set server's socket's address 
		//may need to change the IP address later; Perhaps add parameter to this function

		//Check for conversion of IP address error
	if( Res < 0 )
	{
		perror("Error: 1st parameter is not a valid address family");
		close(cSocketFD);
		exit(EXIT_FAILURE);
	}
	else if( Res == 0 )
	{
		perror("Error: A valid IP address was not passed in as the 2nd parameter");
		close(cSocketFD);
		exit(EXIT_FAILURE);
	}
	
//-----Create socket to listen for client(s) request on server side (noted as client socket in rest of the code)-----
	struct sockadd sSockAddr;
	int Res;
	int sSocketFD = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		//Check for error creating client socket
	if( sSocketFD == -1 )
	{
		perror("Error: Cannot create socket");
		exit(EXIT_FAILURE);
	}

	memset(&sSockAddr, 0, sizeof(sSockAddr));

	sSockAddr.sa_family = AF_INET; //sin_family instead of sa_family on wiki
	sSockAddr.sin_addr.s_addr = INADDR_ANY; //may need to switch addresses
	sSockAddr.sin_port = htons(13728);
		//above statement allows program to work w/o knowing IP address of the machine it was running on

		
//-----Bind server socket to listening port-----
		//&clientSockAddr is the address we bind it to
	if( bind(sSocketFD, (struct sockaddr *)&sSockAddr, sizeof(sSockAddr)) == -1 )
	{
		perror("Error: Binding failed.");
		close(sSocketFD);
		exit(EXIT_FAILURE);
	}

		//prepare socket to listen for connections from client(s)
	if( listen(sSocketFD, 10) == -1 ) //10, because we can queue up to 10 connections
	{
		perror("Error: Listen failed.");
		close(sSocketFD);
		exit(EXIT_FAILURE);
	}

	
//-----Accept and connect sockets-----
	for(;;)
	{
			//server tries to accept connection from client
		if( accept(cSocketFD, (struct sockaddr *)&sSockAddr, sizeof(sSockAddr)) < 0 )
		{
			perror("Error: Accept failed");
			close(cSocketFD);
			exit(EXIT_FAILURE);
		}

			//client tries connecting to server socket
		if( connect(sSocketFD, (struct sockaddr *)&cSockAddr, sizeof(cSockAddr)) == -1 )
		{
			perror("Error: Failed to connect");
			close(sSocketFD);
			exit(EXIT_FAILURE);
		}
		
		//KC to do: fix below 
			//Perform read write operations here
		if( shutdown(cConnectFD, HUT_RDWR) )
		{
			perror("Cannot shutdown socket");
			close(cConnectFD);
			exit(EXIT_FAILURE);
		}
		close(cConnectFD); //after all the reads have been done	
	}

	return EXIT_SUCCESS;
}
//*/
