#include <stdio.h>
#include <iostream>
#include <unistd.h> //read(),write(),pipe()...
#include <string.h> //strcat(), memset()...
#include <arpa/inet.h> //inet_ntoa()
#include <netdb.h> //hostent, gethostbyname()
#include "crc8.h"
//#include <sys/types.h> 
//#include <sys/socket.h>
//#include <netinet/in.h>
//#include <stdlib.h>

//Function for showing errors
void error(const char *msg){
	perror(msg);
	exit(1);
}


//Structure for holding adresses
struct Ips{
	std::string szClientIp;
	std::string szServerIp;
	int szClientPort;
	int szServerPort;	
};

//Message structure
struct Message{
	std::string szRecipientIp;
	std::string szSenderIp;
	bool bStatus;
	std::string szMessage;
};

//Parse recevied messages
void parseMessage(Message& msg, char buffer[256]){
	char wordStatus[] = {"status\n"};
	char msgBuffer[256];
	char recIp[32];
	char senIp[32];	

	memset(recIp, 0 , sizeof(recIp));
	memset(senIp, 0 , sizeof(senIp));
	memset(msgBuffer, 0 , sizeof(msgBuffer));
	
	//Put recipient's ip from message in variable
	int i = 0;
	int j = 0;
	while(buffer[i] != '\n'){
		recIp[i] = buffer[i];
		++i;
	}
	msg.szRecipientIp = recIp;
	recIp[i] = '\0';
	++i;
			
	//Put sender's ip from message in variable
	while(buffer[i] != '\n'){
		senIp[j] = buffer[i];
		++i;
		++j;
	}
	senIp[j] = '\0';
	msg.szSenderIp = senIp;

	//Whether is it a status message?
	msg.bStatus = true;
	j = 0;
	i = i+1; //Position after last '\n'
	int nTemp = i+7; //7 letters in "status\n"	
	for(; i < nTemp; ++j, ++i){
		if(wordStatus[j] != buffer[i]){
			msg.bStatus = false;
		}
	}
	//copy message in msgBuffer

	//return back our i if without "status"
	if(!msg.bStatus){
		i = nTemp - 7;		
	}
	j = 0;
	while(buffer[i] != '\0'){
		msgBuffer[j] = buffer[i];
		++i;
		++j;
	}
	msg.szMessage = msgBuffer;
}

	


class Client{
private:
	int sockfd, portno, bitsSent, pipeRead;
	char* host;
	char buffer[256];
	char message[256];
	struct hostent *server;
    struct sockaddr_in serv_addr;
	socklen_t servlen;
	Ips ips;
	Message msg;

public:

	~Client(){
		close(sockfd);
	}
	
	//Get address, port, pipe Read descriptor
	Client(char* addrToConnect, int port, int pipefd) : 
	portno(port), 
	host(addrToConnect),
	pipeRead(pipefd){

		//Creating a socket
	    sockfd = socket(AF_INET, SOCK_STREAM, 0);
	    if (sockfd < 0) 
		    error("ERROR opening socket");
		//Get hostent structure
	    server = gethostbyname(host);
	    if (server == NULL) {
	        fprintf(stderr,"ERROR, no such host\n");
	        exit(0);
		}
		
		//Filling our host structure
		bzero((char *) &serv_addr, sizeof(serv_addr));
	    serv_addr.sin_family = AF_INET;
	    bcopy((char *)server->h_addr,(char *)&serv_addr.sin_addr.s_addr,server->h_length);
	    serv_addr.sin_port = htons(portno);

	}
	
	void myConnect(){

		//Connect to a server
		if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
			error("ERROR connecting");

		//infinite loop for tracking and sending messages from
		//our server
		sendForwardRun();


		servlen = sizeof(serv_addr);
		//Get Ips of Server and Client
		    if (getpeername(sockfd, (struct sockaddr*)&serv_addr, &servlen) == -1)
			    perror("client getpeername");
		    else		    
				ips.szClientIp = inet_ntoa(((struct sockaddr_in *)&serv_addr)->sin_addr);
		    	ips.szClientPort = ntohs(((struct sockaddr_in *)&serv_addr)->sin_port);
		         
		    if (getsockname(sockfd, (struct sockaddr*)&serv_addr, &servlen) == -1)
				perror("client getsockname");
		    else
			    ips.szServerIp = inet_ntoa(((struct sockaddr_in *)&serv_addr)->sin_addr);
				ips.szServerPort = ntohs(((struct sockaddr_in *)&serv_addr)->sin_port); 
		//End of getting server's and client's ips 


	}

	//For sending messages
	void sendTo(){
		while(1){
			

			memset(buffer, 0, sizeof(buffer));
			memset(message, 0, sizeof(message));

			if (bitsSent < 0) 
				error("ERROR writing to socket");

			//Enter Ip to deliver
			usleep(10000);
			printf("*Enter IP of recipient* \n");
			fgets(message, sizeof(message) - 1,stdin);
			
			//Add our ip
			strncat(message, ips.szServerIp.c_str(), sizeof(ips.szServerIp));
			strcat(message, "\n");

			//Enter our message
			printf("*Enter your message* \n");
			fgets(buffer, sizeof(buffer) - 1,stdin);

			//Send message
			strcat(message, buffer);
			bitsSent = write(sockfd, message ,strlen(message));
			if (bitsSent < 0) 
				error("ERROR writing to socket");

		}
		close(sockfd);
	}

	//Infinite cycle for catching messages from
	//our server to send it forward to a next node
	void sendForwardRun(){
		char buffer[256];
		int bitsSent = 0;
		bool bChild = true;

		if(fork() != 0)
			bChild = false;

		//child will be here forever in interminable cycle
		while(bChild){			
			memset(buffer, '\0', sizeof(buffer));
			//wait for some message to send to a next node
			bitsSent = read(pipeRead, buffer, sizeof(buffer));
			if (bitsSent < 0)
				error("ERROR reading from pipe");

			//Send message/forward
			bitsSent = write(sockfd, buffer, strlen(buffer));
			if (bitsSent < 0)
				error("ERROR writing to socket");
		}
	}

};

class Server{
private:
	int sockfd, newsockfd, portno, pid, sockClient;
    socklen_t clilen, servlen;
	struct sockaddr cli_addr;
    struct sockaddr_in serv_addr;
	int pipeWrite;


public:

	Ips ips;
	Message msg;


	//Get port and pipe write descriptor
	Server(int port, int pipefd) : portno(port), pipeWrite(pipefd) {
	
		//Creating socket
		sockfd = socket(AF_INET, SOCK_STREAM, 0);
		servlen = sizeof(serv_addr);
		
		if (sockfd < 0)
			error("ERROR opening socket");
			
		//fill structure for the server
		memset(&serv_addr, 0 , servlen);
		serv_addr.sin_family = AF_INET;
		serv_addr.sin_port = htons(portno);
		serv_addr.sin_addr.s_addr = INADDR_ANY;

		//bind ip with socket
		if (bind(sockfd, (struct sockaddr *)&serv_addr, servlen) < 0)
			error("ERROR on binding");	
	}

	~Server(){
		close(sockfd);
	}

	void myListen(){
	
		//Ready for accepting new connections
		//Maxconnections = 5
		listen(sockfd,5);		
		

		
		while (1) {
		//accept connection from a client
		newsockfd = accept(sockfd, &cli_addr, &clilen);

		

		//Get Ips of Server and Client
		    if (getpeername(newsockfd, &cli_addr, &clilen) == -1)
			    perror("server getpeername");
		    else		    
				ips.szClientIp = inet_ntoa(((struct sockaddr_in *)&cli_addr)->sin_addr);
		    	ips.szClientPort = ntohs(((struct sockaddr_in *)&cli_addr)->sin_port);
		         
		    if (getsockname(newsockfd, &cli_addr, &clilen) == -1)
				perror("server getsockname");
		    else
			    ips.szServerIp = inet_ntoa(((struct sockaddr_in *)&cli_addr)->sin_addr);
				ips.szServerPort = ntohs(((struct sockaddr_in *)&cli_addr)->sin_port); 
		//End of getting server's and client's ips 

		std::cout<<"*Status: <"<<ips.szClientIp<<"> connected.*"<<std::endl;

			
			if (newsockfd < 0) 
				error("ERROR on accept");			
			pid = fork();
			if (pid < 0)
				error("ERROR on fork");
			if (pid == 0)  {
				while(1){
					close(sockfd);
					myReceive();					
				}
				exit(0);
			}
			else 
				close(newsockfd);
		} /* end of while */
	}


	//Receive messages from client
	void myReceive (){
		int bitsReceived;
		char buffer[256];
		//Variable for knowing whether it's a
		//status message or not
		bool isStatus = true;

		memset(buffer, 0 , sizeof(buffer));

		//Read from the socket
		bitsReceived = read(newsockfd, buffer, 255);
		if (bitsReceived < 0) 
			error("ERROR reading from socket");

		//Parse message to us
		parseMessage(msg, buffer);
			
		//Whether is it a message to us?
		if( strcmp(msg.szRecipientIp.c_str(), ips.szServerIp.c_str()) == 0 ){
			//Whether is it a status message?
			if(!msg.bStatus){
				//Show sender's ip and message
				std::cout<<"\033[1;32m<"<<msg.szSenderIp<<"> : "<<msg.szMessage<<"\033[0m"<<std::endl;
				sendStatus(msg.szSenderIp.c_str(), "I received your message");
			} else {
				//Show sender's ip and message
				std::cout<<"*Status from <"<<msg.szSenderIp<<"> : "<<msg.szMessage<<"*"<<std::endl;
			}
		} else {
			//if our IP match sender's ip, then message did a circle
			//and didn't find his recipient
			if( strcmp(msg.szSenderIp.c_str(), ips.szServerIp.c_str()) != 0 ){
				std::cout<<"IT'S Not ME! "<<std::endl;
				sendForward(buffer);
			} else std::cout<<"*Status: here is no such IP*"<<std::endl;
		}	
	}

	//For status messages
	void sendStatus(const char ip[32], const char message[256]){
			int bitsSent = 0;
			char buffer[256];
			char wordStatus[] = "status";
			memset(buffer, 0, sizeof(buffer));
			//Add ip to deliver
			strcat(buffer, ip);
			strcat(buffer, "\n");
			//Add our ip
			strcat(buffer, ips.szServerIp.c_str());
			strcat(buffer, "\n");
			//Add status word
			strcat(buffer, wordStatus);
			strcat(buffer, "\n");
			//Unite buffer with message
			strcat(buffer, message);

			sendForward(buffer);
	}


	//Transfer message to our Client
	//for sending it to a next node
	void sendForward(char buffer[256]){
		int bitsSent = 0;

		bitsSent = write(pipeWrite,	buffer,	strlen(buffer));
		if (bitsSent < 0) 
				error("ERROR writing to pipe");
	}


};


int main(int argc, char *argv[]){

	//Create pipe for sending messages from
	//server through our client
	int pipefd[2];
	pipe(pipefd);

	char addrToConnect[128];
	std::string szAddrToConnect;
	int portToConnect, portToListen, pid;

	std::cout<<"Enter port to listen"<<std::endl;
	std::cin>>portToListen;

	//Set port and pipe write identificator
	Server server(portToListen, pipefd[1]);	
	pid = fork();
	
	if(pid == 0){
		close(pipefd[0]); //close read side of pipe
		server.myListen();
	} else {
		close(pipefd[1]); //close write side of pipe
		std::cout<<"Enter address to connect"<<std::endl;
		std::cin>>addrToConnect;

		std::cout<<"Enter port to connect"<<std::endl;
		std::cin>>portToConnect;
		std::cin.ignore();

		//Set address, port and pipe read identificator
		Client client(addrToConnect, portToConnect, pipefd[0]);
		client.myConnect(); //connect to a server
		client.sendTo(); //infinite loop for sending messages
	}

	return 0; //We never get here 
}

