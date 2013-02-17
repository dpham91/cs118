/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
//http://www.cs.rutgers.edu/~pxk/rutgers/notes/sockets/index.html

#include <iostream>
#include <sstream>
#include <string>
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
#include "http-request.h"
#include <map>
#include <time.h>

using namespace std;

const int MAX_THREADS = 10; //maximum number of processes (or threads) we are allowed

std::map<string,string> cache;
bool CheckCache(string URL);
string getResponse(char* request,string remoteHost, unsigned short remotePort, string version, size_t length);

void* readAndParseRequest(void* fd)
{
	string clientBuffer;
	int* clientfd = (int*) fd;
	
	// Read in the request for parsing. Loop through until we get "\r\n\r\n"
	while (memmem(clientBuffer.c_str(), clientBuffer.length(), "\r\n\r\n", 4) == NULL)
	{
		char buf[1024];
		if (recv( *clientfd, buf, sizeof(buf), 0) < 0)
		{
			perror("Error: Cannot read request");
			break;
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
		if (send(*clientfd, clientRes.c_str(), clientRes.length(), 0) == -1)
			perror("Error: Cannot send request");
		
		//TODO: Might need to add some sort of return
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
	
	//TODO: connect to remote host, get data from remote host, 
	//cache response, and send response to client
	
	string URL = clientReq.GetPath();
	string response;
	//Expires Header is not empty so we need to put it in the cache.
	if (clientReq.FindHeader("Expires") != "")
	{
		//We first check to see ift he response is already in the cache.
		if (!checkcache(URL))
		{response = cache[URL];}
		else
			response = getResponse(formattedReq,remoteHost,clientReq.GetPort(),clientReq.GetVersion(), requestLength);
	}
	else
	{
		response = getResponse(formattedReq,host,port);
	}
	
	if (send(clientfd, response.c_str(), response.length(), 0) == -1)
	{
		perror("send");
		free(formattedReq);
		exit(EXIT_FAILURE);
	}
	
	//cleanup allocated memory
	free(formattedReq);
	
	return NULL;
}

string getResponse(char* request,string remoteHost, unsigned short remotePort, string version, size_t length)
{
	//get address of url
	struct addrinfo addr, *res;
	memset(&addr, 0, sizeof(addr));
	addr.ai_family = AF_NET;
	addr.ai_socktype = SOCK_STREAM;
	
	//host and port need to be char* to be used in getaddrinfo
	const char* host = remoteHost.c_str();
	char port[10];
	sprintf(port, "%d", remotePort);
	
	if (getaddrinfo(host, port, &addr, &res) != 0) {
        return "HTTP/" + version + " 400 Bad Requestr\n\r\n";
	}
	
	int socketFd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if( socketFd == -1 )
	{
		perror("Error: Cannot create socket");
		exit(EXIT_FAILURE);
	}
	
	if (connect(socketFd,res->ai_addr, res->ai_addrlen) < 0) {
		perror("Error: Connect failed");
		close(socketFd);
		exit(EXIT_FAILURE);
	}
	
	if (send(socketFd, request, length, 0) == -1)
    {
		perror("Error: Send failed");
		close(socketFd);
		exit(EXIT_FAILURE);
    }
	
	string response;
	for (;;)
	{
		char resBuf[1024];
		
		// Get data from remote
		int numRecv = recv(socketFd, resBuf, sizeof(resBuf), 0);
		if (numRecv < 0)
		{
			perror("Error: Could not get response");
			close(socketFd);
			exit(EXIT_FAILURE);
		}
		
		// If we didn't recieve anything, we hit the end
		else if (numRecv == 0)
			break;
		
		// Append the buffer to the response if we got something
		response.append(res_buf, num_recv);
	}
	close(socketFd);
	return response;
}

bool CheckCache(string URL)
{
	std::map<string,string>:const_iterator found = cache.find(URL);
	if (found == cache.end())
	{return false;}
	else
	{
		return true;
	}
	
}


int main (int argc, char *argv[])
{
	// command line parsing
	
	//-----Create socket on server side-----
	struct sockaddr_in sSockAddr;
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
	sSockAddr.sin_port = htons(13572); //port number used for listening; Change to 14805 later
	
	//-----Create socket for connection to server on client's side (cSockAddr)-----
	struct sockaddr_in cSockAddr;
	memset(&cSockAddr, 0, sizeof(cSockAddr));
	
	//-----Bind server's socket to listening port-----
	if( bind(socketFD, (struct sockaddr *)&sSockAddr, sizeof(sSockAddr)) == -1 )
	{
		perror("Error: Binding failed.");
		close(socketFD);
		exit(EXIT_FAILURE);
	}
	
	//----prepare server's socket to listen for incoming connections from client(s)
	if( listen(socketFD, 10) == -1 ) //10, because we can queue up to 10 connections
	{
		perror("Error: Listen failed.");
		close(socketFD);
		exit(EXIT_FAILURE);
	}
	
	int clientFDs[10]; //array for file descriptors of client's we accept;
	//Note: we can only have up to 10 processes, so we can hold at most 10 FDs
	
	pthread_t threads[MAX_THREADS];
	int threadNum = 0; //indicates thread number
	socklen_t sizeCSAddr = sizeof(cSockAddr);
	
	//-----Accept and connect sockets-----
	while(true)
	{
		//server tries to accept connection from client
		// accept groups of 10 requests at a time
		//KC: For your understanding of accept: http://stackoverflow.com/questions/489036/how-does-the-socket-api-accept-function-work
		clientFDs[threadNum] = accept(socketFD, (struct sockaddr *)&cSockAddr, &sizeCSAddr);
		
		//Check for accept failure
		if( clientFDs[threadNum] < 0 )
		{
			perror("Error: accept() failed");
			continue;
		}
		
		//create new thread
		//passing clientFDs[threadNum] to the function that will read in the request so it has the specific socket
		if( pthread_create(&threads[threadNum], NULL, readAndParseRequest, &clientFDs[threadNum])) //correct usage of &?
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
