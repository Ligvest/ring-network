//#include "headers.hpp"

#ifndef CLIENT_HPP
#define CLIENT_HPP

#define BREAK_CRC false


//Infinite cycle for catching messages from
//our server to send it forward to a next node	
void* sendForwardThread(void* arg);

class Client{
private:
	int sockfd, portno, bitsSent, pipeRead;
	char* host;
	char buffer[256];
	char prevBuf[256];
	char message[256];
	uint8_t crc8Table[256];
	struct hostent *server;
    struct sockaddr_in serv_addr;
	socklen_t servlen;
	Ips ips;
	Message msg;

public:

	//Constructor
	Client(char* addrToConnect, int port, int pipefd);
	//Destructor
	~Client();
	
	//For connecting to a server
	void myConnect();
	//For sending messages
	void sendTo();
	//Infinite cycle for catching messages from
	//our server to send it forward to a next node	
	friend void* sendForwardThread(void* arg);
	//Run sendForwardThread
	void sendForwardRun();
};

/*
*
* client.cpp
*
*/

	//Constructor
	//Get address, port, pipe Read descriptor
	Client::Client(char* addrToConnect, int port, int pipefd) : 
	portno(port), 
	host(addrToConnect),
	pipeRead(pipefd),
	bitsSent(0){

		memset(prevBuf, '\0', sizeof(prevBuf));

		//Generate table of crc8 values
		crcGenTable(crc8Table);

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

	//Destructor	
	Client::~Client(){
		close(sockfd);
	}

	
	//Function for connecting to a server
	void Client::myConnect(){

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
	void Client::sendTo(){
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

			//Paste crc8 code before whole message
			strcat(message, buffer);
			memset(buffer, '\0', sizeof(buffer));
			uint8_t crc;
			crc = crcGet(crc8Table,
						(const uint8_t*)(message),
						strlen(message));
//breaking crc
			if(BREAK_CRC)
			crc++;

			strcat(buffer, std::to_string(crc).c_str());
			strcat(buffer, "\n");
			strcat(buffer, message);
		
			char somebuf[] = {(char)255,(char)255};
			//Bitstuffing of our message
			UCHAR* pucBitstaffed = bitStuff((UCHAR*)buffer, sizeof(buffer));

			//Send message
			bitsSent = write(sockfd, pucBitstaffed, strlen(buffer));
			if (bitsSent < 0) 
				error("ERROR writing to socket");

			//Freeing allocated memory
			free(pucBitstaffed);


			memset(prevBuf, '\0', sizeof(prevBuf));
			strcpy(prevBuf, buffer);
		}
		close(sockfd);
	}
	
	void Client::sendForwardRun(){
		static pthread_t threadId;
		int retVal = pthread_create(&threadId, NULL, sendForwardThread, (void*)this);
		if(retVal != 0){
			error("ERROR creating thread");
		}
	}

	//Infinite cycle for catching messages from
	//our server to send it forward to a next node
	//make a thread here 
	void* sendForwardThread(void* arg){
		Client* client = (Client*)arg;
		char buffer[256];
		int bitsSent = 0;

		//child will be here forever in interminable cycle
		while(true){			
			memset(buffer, '\0', sizeof(buffer));
			//wait for some message to send to a next node
			bitsSent = read(client->pipeRead, buffer, sizeof(buffer));
			if (bitsSent < 0)
				error("ERROR reading from pipe");

		
			//Just resend our global buffer with previous message
			if(strcmp(buffer, "resend") == 0){
				std::cout<<"prevBuf:\n"<<client->prevBuf<<std::endl;
				bitsSent = write(client->sockfd, client->prevBuf, strlen(client->prevBuf));
				if (bitsSent < 0)
					error("ERROR writing to socket");
			} else if(strcmp(buffer, "16tries") == 0){
				//If was 16 unsuccessful tries to resend prev buffer
				std::cout<<"Tries limit has been exceeded"<<std::endl;			
			} else {
				//Send message/forward
				bitsSent = write(client->sockfd, buffer, strlen(buffer));
				if (bitsSent < 0)
					error("ERROR writing to socket");
			}
		}
	}




#endif
