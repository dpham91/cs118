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

using namespace std;
const int MAX_THREADS = 10; //maximum number of processes (or threads) we are allowed
std::map<string,string> cache;

void* readAndWrite(void* fd);
bool CheckCache(string URL);
string getResponse(char* request,int socketFd, size_t length);


void* readAndWrite(void* fd)
{
	string clientBuffer;
	int* clientfd = (int*) fd;
	
	// Read in the request for parsing. Loop through until we get "\r\n\r\n"
	do
	{
		char buf[1024]; //initialize a new buffer to read in
		if (recv( *clientfd, buf, sizeof(buf), 0) < 0)
		{
			perror("Error: Cannot read request");
			break;
		}
		clientBuffer.append(buf);
	}
	while (memmem(clientBuffer.c_str(), clientBuffer.length(), "\r\n\r\n", 4) == NULL);
         //TODO: Check if this is being called correcly
		//http://www.thinkage.ca/english/gcos/expl/c/lib/memmem.html

        cout << "Testing print of clientBuffer..." << endl << clientBuffer << endl << "end";
        
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
	
	
	//get Content-length
	//int content length = (int)(clientReq.FindHeader("Content-Length"));
	
	// HTTP 1.0 needs Connection: close header if not already there
	string version = clientReq.GetVersion();

        cout << "version = " << version << endl;

	if( version == "1.0")
		clientReq.ModifyHeader("Connection", "close");
	
	//get url path
	string path = clientReq.GetPath();
	
	string response = ""; //response string to send back
	
	//call check cache function; call CheckCache(path, /*expiration string*/)
	if( false ) //TODO:Change
	{
		response = cache[path];
	}
	else //if not in cache, get from server
	{		
			//format request to send to server
		size_t requestLength = clientReq.GetTotalLength() + 1;// extra one for \0
                string fReqStr;
                cout << "requestLength = " << requestLength << endl;
		char *formattedReq = new char[requestLength];//(char *) malloc(requestLength);
		fReqStr = clientReq.FormatRequest(formattedReq);


                cout << "fReq Str " << fReqStr << endl;
               // cout << "formattedReq length = " ;
                /*int fReqSize = strlen(formattedReq);
                cout << fReqSize << " " << endl;

                for(int i = 0; i < fReqSize; i++)
                {
                        cout << formattedReq[i];
                }*/

			// If host not specificed then find in the headers
		string remoteHost;
		if (clientReq.GetHost().length() == 0)
			remoteHost = clientReq.FindHeader("Host");
		else
			remoteHost = clientReq.GetHost();
		
			//host and port need to be char* to be used in getaddrinfo
		const char* host = remoteHost.c_str();
		unsigned short remotePort = clientReq.GetPort();
		char port[10];
		sprintf(port, "%d", remotePort);
		//const char* path = (clientReq.GetPath()).c_str();
		
		struct addrinfo hints, *res;
		int toServerFD; //socket between proxy and server

		memset(&hints, 0, sizeof(hints));
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;

		if(getaddrinfo(host, port, &hints, &res)!=0)
			//return "400 - Bad Request\n\n";

                cout << "res->ai_family = " << res->ai_family << endl;
                cout << "res->ai_socktype = " << res->ai_socktype << endl;
                cout << "res->ai_protocol = " << res->ai_protocol << endl;
                cout << "Host = " << host << endl;
                cout << "Port = " << port << endl;
                

			//create socket and connect
		toServerFD = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if (toServerFD < 0){
			cerr << "ERROR: Cannot create socket."<< endl;
		}

		if (connect(toServerFD,res->ai_addr, res->ai_addrlen) < 0) {
			cerr << "ERROR: Cannot connect." << endl;
		}

                cout << "I have reached here!" << endl;
		
		//TODO: Add the call of the get response function passing in toServerFD file descriptor
                response = getResponse(formattedReq,toServerFD,requestLength);

                cout << "I have reached after getResponse." << endl;

                if(send(*clientfd, response.c_str(), requestLength, 0))
                {
                        
                };
                close(toServerFD);                

		return NULL;
        }
			//write and send request
		/*char *recBuf = new char [1024];
		char *sendBuf = new char [requestLength+1];
		if(write(toServerFD, sendBuf, requestLength)<0)
			cerr<<"Error: Cannot write to socket."<<endl;
		
                cout << "I have reached pass the write." << endl;

                int lengthOfSendBuf = strlen(sendBuf);
                cout << "length of sendBuf = " << lengthOfSendBuf << endl;
                
			//read in response
		int canRead;
		string message;
		do
		{
			bzero((char *)recBuf, sizeof(recBuf));
			canRead=read(toServerFD, recBuf, sizeof(recBuf)-1);
			if (canRead<0)
				cerr<<"Error: Cannot read"<<endl;

			message+=recBuf;

		}while(canRead>0);

                cout << "message = " << message << endl;

		//TODO: store "message" value in cache

			//Clean up
		freeaddrinfo(res);
		delete [] sendBuf;
		delete [] recBuf;
		//close(HTTPsockfd);
		
		response = message;
		
		//cleanup allocated memory
		free(formattedReq);
		
		//return NULL;
		
			
		
		
		if(version = "1.0" or server header has close connection){ //close socket if HTTP/1.0 and reconnect
			close(toServerFD);
		
			toServerFD = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
			if (toServerFD < 0){
				cerr<<"ERROR, cannot create socket"<<endl;
			}

			if (connect(toServerFD,res->ai_addr, res->ai_addrlen) < 0) {
				cerr<<"ERROR, cannot connect"<<endl;
			}
				
		}//*/
		
		
		
	/*}
	
	pthread_exit(NULL);
        return NULL;*/
        return NULL;

}

//Checks cache to see if URL is already stored, and whether it's expired
bool CheckCache(string URL)
{
	//iterate through the map container to find the URL
	std::map<string, string>::const_iterator found = cache.find(URL);

	//if  not found, return false
	if (found == cache.end())
	{
                return false;
        }
	/*else  //check to see if the cached URL has expired or not
	{
	        if ((cache[URL]).FindHeader("Expires")== "")
	        {
                        return true;
                }
	        else
	        {
                	struct tm tm;
		        time_t t;
		        time_t currenttime;
		        const char* date = cache[URL].FindHeader(expires);
	         
		        if (strptime(date, "%a, %d %b %Y %H:%M:%S %Z", &tm) != NULL)
		        {
			        t = mktime(&tm);
		                currenttime = time(NULL);
			        if (t < currenttime)
			        {
				        cache.erase(URL);
				        return true;
			        }
			        else
			        {
				        return false;
			        }
		        }
	        }
                return false;
	}*/
        return false;
	
}


string getResponse(char* request,int socketFd, size_t length)
{

        cout << "I have entered getResponse and socketFd = " << socketFd << endl;
        cout << "Request = " << request << endl;
	if (send(socketFd, request, length, 0) == -1)
            {
		        perror("Error: Send failed");
		        close(socketFd);
		        exit(EXIT_FAILURE);
            }

        cout << "get past if in getResponse" << endl;
	string response;
	for (;;)
	{
                cout << "get into for loop of getResponse" << endl;
		char resBuf[1024];
		
		// Get data from remote
		int numRecv = recv(socketFd, resBuf, sizeof(resBuf), 0);
                cout << "numRecv = " << numRecv << endl;
                //cout << "past recv in getResponse" << endl;
		if (numRecv < 0)
		{
			perror("Error: Could not get response");
			close(socketFd);
			exit(EXIT_FAILURE);
		}
		
		// If we didn't receive anything, we hit the end
		else if (numRecv == 0)
			break;
		
		// Append the buffer to the response if we got something
		response.append(resBuf, numRecv);
	}

        cout << "response = " << response << endl;
	return response;
}

int main (int argc, char *argv[])
{
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
	sSockAddr.sin_port = htons(13686); //port number used for listening; Change to 14805 later
	
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
		//Server tries to accept connection from client.
		//We can accept groups of 10 clients at a time.
		//KC: For your understanding of accept: http://stackoverflow.com/questions/489036/how-does-the-socket-api-accept-function-work
		clientFDs[threadNum] = accept(socketFD, (struct sockaddr *)&cSockAddr, &sizeCSAddr);
			//If no connection requests are queued and socket is in nonblocking mode, accept() returns -1
		
		//Check for accept failure
		if( clientFDs[threadNum] == -1 )
		{
			perror("Error: accept() failed");
			continue;
		}
		
		//create new thread
		//passing clientFDs[threadNum] to the function that will read in the request so it has the specific socket
		if( pthread_create(&threads[threadNum], NULL, readAndWrite, &clientFDs[threadNum]))
		{
			perror("Error: Not able to create thread.");
			exit(EXIT_FAILURE);          
		}
		
		threadNum++;
		if(threadNum == MAX_THREADS) //10 processes total, so processNum 0 to 9
		{
			//pthread_join(); //wait until all 10 threads are done and then accept more clients
			threadNum = 0;
		}
               
                //delete later 
               // close(socketFD);
               // break;
	}
	
	//pthreads_exit(NULL);
	close(socketFD);
	
	return EXIT_SUCCESS;
}
//*/
