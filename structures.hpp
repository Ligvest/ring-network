#ifndef STRUCTURES_HPP
#define STRUCTURES_HPP

//Structure for holding adresses
struct Ips{
	std::string szClientIp;
	std::string szServerIp;
	int szClientPort;
	int szServerPort;	
};

//Message structure
struct Message{
	uint8_t crc8;
	std::string szRecipientIp;
	std::string szSenderIp;
	bool bStatus;
	std::string szMessage;
	std::string szBuffer;
};

#endif
