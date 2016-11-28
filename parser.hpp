//#include "structures.hpp"

#ifndef PARSER_HPP
#define PARSER_HPP



//Parse recevied messages
void parseMessage(Message& msg, char buffer[256]){
	std::string szCrc8;
	char wordStatus[] = {"status\n"};
	char msgBuffer[256];
	char recIp[32];
	char senIp[32];
	char msgCrc8[8];
	
//	std::cout<<"*** buffer from parseMessage() ***\n"<<buffer<<std::endl;
//	std::cout<<"\n***************"<<std::endl;

	memset(recIp, 0 , sizeof(recIp));
	memset(senIp, 0 , sizeof(senIp));
	memset(msgBuffer, 0 , sizeof(msgBuffer));
	
	
	//Put crc from message to variable
	int i = 0;
	int j = 0;
	while(buffer[i] != '\n'){
		msgCrc8[i] = buffer[i];
		++i;
	}
	msgCrc8[i] = '\0';
	msg.crc8 = atoi(msgCrc8);
	++i;
	
	
	//Put whole buffer except crc to variable
	while(buffer[i+j] != '\0'){
		msgBuffer[j] = buffer[i+j];
		++j;
	}
	msg.szBuffer = msgBuffer;
	j = 0;
	memset(msgBuffer, '\0', sizeof(msgBuffer));
	

	
		
	
	//Put recipient's ip from message to variable
	while(buffer[i] != '\n'){
		recIp[j] = buffer[i];
		++i;
		++j;
	}
	msg.szRecipientIp = recIp;
	recIp[i] = '\0';
	++i;

			
	//Put sender's ip from message to variable
	j = 0;
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
	//copy message to msgBuffer

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

#endif
