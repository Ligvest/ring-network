#include <iostream>
#include <string.h>


int main(){


	uint8_t var = 127;
	char ch[8];
	memset(ch, '\0', sizeof(ch));
//	std::string str = std::to_string(var);
	strcat(ch, std::to_string(var).c_str());
	std::cout<<ch<<std::endl;
	return 0;
}
