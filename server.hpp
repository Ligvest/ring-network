#ifndef SERVER_HPP
#define SERVER_HPP

class Server{
private:
	int sockfd, newsockfd, portno, pid, sockClient;
    socklen_t clilen, servlen;
	struct sockaddr cli_addr;
    struct sockaddr_in serv_addr;
	int pipeWrite;
	uint8_t crc8Table[256];


public:

	Ips ips;
	Message msg;

	//Constructor
	//Get port and pipe write descriptor
	Server(int port, int pipefd);
	//Destructor
	~Server();
	
	//Receive messages from client
	void myReceive();
	//For listening for new connection
	void myListen();

	//For status messages
	void sendStatus(const char ip[32], const char message[256]);

	//Transfer message to our Client
	//for sending it to a next node
	void sendForward(const char buffer[256]);

};



/*
*
* server.cpp
*
*/

	//Constructor
	//Get port and pipe write descriptor
	Server::Server(int port, int pipefd) : portno(port), pipeWrite(pipefd) {

		//Generate table of crc values
		crcGenTable(crc8Table);

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
	
	//Destructor
	Server::~Server(){
		close(sockfd);
	}
	
	
	//For listening for new connection
	void Server::myListen(){
	
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
	void Server::myReceive (){
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
		

		//unbitstuffing of a received message
		UCHAR* pucUnbitstuffed = unBitStuff((UCHAR*)buffer, sizeof(buffer));

		//Parse message to us
		parseMessage(msg, (char*)pucUnbitstuffed);
		
		//Calculate crc of data
		uint8_t crcHere = 
		crcGet(	crc8Table, 
				(const uint8_t*)(msg.szBuffer.c_str()),
				strlen(msg.szBuffer.c_str()) );

		//True if CRC's matched
		if(!crcCheck(msg.crc8, crcHere))
			sendStatus(msg.szSenderIp.c_str(), "bad crc\0");
		else
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
				if(strcmp(msg.szMessage.c_str(), "bad crc") == 0){
					sendForward("resend\0");				
				}
			}
		} else {
			//if our IP match sender's ip, then message did a circle
			//and didn't find his recipient
			if( strcmp(msg.szSenderIp.c_str(), ips.szServerIp.c_str()) != 0 ){
				std::cout<<"IT'S Not ME! "<<std::endl;
				sendForward(buffer);
			} else std::cout<<"*Status: here is no such IP*"<<std::endl;
		}

		//free allocated memory
		usleep(10000);
		free(pucUnbitstuffed);
	}
	
	
	//For status messages
	void Server::sendStatus(const char ip[32], const char message[256]){
			int bitsSent = 0;
			char buffer[256];
			char bufferWithCrc[256];
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
			
			//Paste crc8 code before whole message
			memset(bufferWithCrc, '\0', sizeof(bufferWithCrc));
			uint8_t crc;
			crc = crcGet(crc8Table,
						(const uint8_t*)(buffer),
						strlen(buffer));
						
			strcat(bufferWithCrc, std::to_string(crc).c_str());
			strcat(bufferWithCrc, "\n");
			strcat(bufferWithCrc, buffer);


			

			sendForward(bufferWithCrc);
	}

	//Transfer message to our Client
	//for sending it to a next node
	void Server::sendForward(const char buffer[256]){
		//Bitstuffing of our message
		UCHAR* pucBitstaffed = bitStuff((UCHAR*)buffer, strlen(buffer));

		int bitsSent = 0;

		bitsSent = write(pipeWrite,	pucBitstaffed,	strlen(buffer));
		if (bitsSent < 0) 
				error("ERROR writing to pipe");
				
		//Freeing allocated memory
		free(pucBitstaffed);

	}

#endif
