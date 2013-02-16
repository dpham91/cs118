/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
//http://www.cs.rutgers.edu/~pxk/rutgers/notes/sockets/index.html

#include <iostream>
#include <sstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>

using namespace std;

const MAX_THREADS = 10; //maximum number of processes (or threads) we are allowed

void* readAndParseRequest(void* clientfd)
{
	string clientBuffer;
	
	// Read in the request for parsing. Loop through until we get "\r\n\r\n"
	while (memmem(clientBuffer.c_str(), clientBuffer.length(), "\r\n\r\n", 4) == NULL)
	{
		char buf[1024];
		if (recv(clientfd, buf, sizeof(buf), 0) < 0)
		{
			perror("Error: Cannot read request");
			return -1;
		}
		clientBuffer.append(buf);
	}
	
	//Parse the request using the given parsing library found in http-request.cc
	HttpRequest clientReq;
	try {
		clientReq.ParseRequest(clientBuffer.c_str(), clientBuffer.length());
	}
	catch (ParseException ex) {		
		// Find HTTP version of request
		string clientRes = "HTTP/" + clientReq.GetVersion() + " ";
		
		// Deal with not implemented methods and bad requests
		string cmp = "Request is not GET";
		if (strcmp(ex.what(), cmp.c_str()))
			clientRes += "501 Not Implemented\r\n\r\n";
		else
			clientRes += "400 Bad Request\r\n\r\n";
		
		// Send response for the bad request
		if (send(client_fd, clientRes.c_str(), clientRes.length(), 0) == -1)
			perror("Error: Cannot send request");
		return;
	}
	
	// HTTP 1.0 needs Connection: close header if not already there
	if(clientReq.GetVersion() == "1.0")
		clientReq.ModifyHeader("Connection", "close");
	
	// extra one for \0
	size_t requestLength = clientReq.GetTotalLength() + 1;
	char *formattedReq = (char *) malloc(requestLength);
	formattedReq = clientReq.FormatRequest(formattedReq);
	
	// If host not specificed then find in the headers
	string remoteHost;
	if (clientReq.GetHost().length() == 0)
		remoteHost = clientReq.FindHeader("Host");
	else
		remoteHost = clientReq.GetHost();
	
	//host and port need to be char* to be used in getaddrinfo
	char* host = remoteHost.c_str();
	unsigned short remotePort = clientReq.GetPort();
	char port[10];
	sprintf(port, "%d", remotePort);
	char* path = (clientReq.GetPath()).c_str();
	
	//TODO: connect to remote host, get data from remote host, 
	//cache response, and send response to client
	
	//cleanup allocated memory
	free(formattedReq);
}

int main (int argc, char *argv[])
{
	// command line parsing
	
	//-----Create socket on server side-----
	struct sockadd_in sSockAddr;
	int socketFD = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	
	//Check for error creating socket
	if( socketFD == -1 )
	{
		perror("Error: Cannot create socket");
		exit(EXIT_FAILURE);
	}
	
	memset(&sSockAddr, 0, sizeof(sSockAddr)); //clear sSockAddr
	
	sSockAddr.sin_family = AF_INET; //sin_family instead of sa_family on wiki
	sSockAddr.sin_addr.s_addr = INADDR_ANY; //may need to switch addresses
	sSockAddr.sin_port = htons(13728); //port number used for listening; Change to 14805 later
	
	//-----Create socket for connection to server on client's side (cSockAddr)-----
	struct sockaddr_in cSockAddr
	memset(&cSockAddr, 0, sizeof(cSockAddr));
	
	//-----Bind server's socket to listening port-----
	if( bind(socketFD, (struct sockaddr *)&sSockAddr, sizeof(sSockAddr)) == -1 )
	{
		perror("Error: Binding failed.");
		close(sSocketFD);
		exit(EXIT_FAILURE);
	}
	
	//----prepare server's socket to listen for incoming connections from client(s)
	if( listen(sSocketFD, 10) == -1 ) //10, because we can queue up to 10 connections
	{
		perror("Error: Listen failed.");
		close(sSocketFD);
		exit(EXIT_FAILURE);
	}
	
	int clientFDs[10]; //array for file descriptors of client's we accept;
	//Note: we can only have up to 10 processes, so we can hold at most 10 FDs
	
	//pthread_t threads[MAX_THREADS]; //Delete later...
	int threadNum; //indicates thread number
	
	//-----Accept and connect sockets-----
	while(true)
	{
		//server tries to accept connection from client
		// accept groups of 10 requests at a time
		//KC: For your understanding of accept: http://stackoverflow.com/questions/489036/how-does-the-socket-api-accept-function-work
		clientFDs[threadNum] = accept(socketFD, (struct sockaddr *)&cSockAddr, sizeof(cSockAddr));
		
		//Check for accept failure
		if( clientFDs[threadNum] < 0 )
		{
			perror("Error: accept() failed");
			continue;
		}
		
		//create new thread
		//passing clientFDs[threadNum] to the function that will read in the request so it has the specific socket
		if( pthread_create(&thread, NULL, /*add read in request fnc*/, &clientFDs[threadNum])) //correct usage of &?
		{
			perror("Error: Not able to create thread.");
			exit(EXIT_FAILURE);          
		}
		
		threadNum++;
		if(threadNum == MAX_THREADS) //10 processes total, so processNum 0 to 9
		{
			threadNum = 0;
		}
	}
	
	close(socketFD);
	
	return EXIT_SUCCESS;
}
//*/
