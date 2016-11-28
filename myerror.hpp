#ifndef MYERROR_HPP
#define MYERROR_HPP

//Function for showing errors
void error(const char *msg){
	perror(msg);
	exit(1);
}

#endif
