#include "headers.hpp"

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
//		std::cout<<"Before sendTo()"<<std::endl;
		client.sendTo(); //infinite loop for sending messages
	}

	return 0; //We never get here 
}

