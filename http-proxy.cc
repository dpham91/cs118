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


//For forking and reading in request
//http://www.programmersheaven.com/mb/Linux/348344/348344/parallel-processing-using-fork/
//http://www.yolinux.com/TUTORIALS/ForkExecProcesses.html
int forkAndRead(void* sockfd)
{
        int status;
        pid_t pid;
        pid = fork();
        
        if(pid == 0)
        {
                //read request for each child process
        }


        if(pid < 0) //Error with forking
        {
                perror("Error: failed to fork");
                exit(EXIT_FAILURE);
        }

        //insert what both parent and child processes do: Read request from client
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
        
        int processNum = 0; //indicates processNum
	
//-----Accept and connect sockets-----
	for(;;)
	{
		        //server tries to accept connection from client
                clientFDs[processNum] = accept(socketFD, (struct sockaddr *)&cSockAddr, sizeof(cSockAddr));

                        //Check for accept failure
                if( clientFDs[processNum] < 0 )
		{
			perror("Error: accept() failed");
			continue;
		}
                
                forkAndFetch(clientFDs[processNum]);
                processNum++;
                if(processNum == 10) //10 processes total, so processNum 0 to 9
                {
                        processNum = 0;
                }
	}

        close(socketFD);

	return EXIT_SUCCESS;
}
//*/
