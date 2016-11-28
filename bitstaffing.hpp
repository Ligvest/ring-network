#ifndef BITSTUFFING_HPP
#define BITSTUFFING_HPP

typedef unsigned char UCHAR;

bool getBit(UCHAR ucBuffer, int nBitShift);
void setBit(UCHAR &ucBuffer, int nBitShift, bool bSetTo);
void showBits(const UCHAR ucBuffer);
UCHAR* bitStuff(const UCHAR* pucBuffer, const size_t nSize);
UCHAR* unBitStuff(const UCHAR* pucBuffer, const size_t nSize);



UCHAR* bitStuff(const UCHAR* pucBuffer, const size_t nSize)
{
	size_t nLenght = nSize/sizeof(UCHAR); // Lenght of an array
	UCHAR* pucResult = (UCHAR*) malloc(nSize); //Allocate memory for new Bitstaffed array
	memset(pucResult,'\0',nSize);
	
	
	int nUnits = 0; //Counter of units	
	int nBitCounterBuf = 0; // Bit counter for buffer
	int nBitCounterRes = 0; // Bit counter for result
	int nElementBuf = 0; // Element counter for buffer array
	int nElementRes = 0; // Element counter for result array
	
	while(nElementBuf < nLenght)
	{
		

		// Set bit in result to bit from buffer
		setBit(pucResult[nElementRes],(nBitCounterRes), getBit(pucBuffer[nElementBuf], nBitCounterBuf));
					
		// If bit is '1', then increment counter of units
		if( getBit(pucResult[nElementRes], nBitCounterRes) )
		{nUnits++;	}
		else
		{nUnits = 0;}

		// If counter of units counts 5 units, then let's add '0' after them
		if(nUnits == 5)
		{
			// If here's not last bit in this element of array, then add '0' to a next bit
			if(nBitCounterRes < 7)
			{nBitCounterRes++; setBit(pucResult[nElementRes],nBitCounterRes,0);}
			else if(nBitCounterRes == 7) // If here's last bit, then add '0' to a next element of array
			{
				nBitCounterRes = 0; 
				nElementRes++; 

				setBit(pucResult[nElementRes],nBitCounterRes,0);
			}

			// After adding '0' set units counter to 0
			nUnits = 0;
		}

		if(nBitCounterBuf == 7) // Counter for bits in buffer
		{nElementBuf++; nBitCounterBuf = 0;} // Element in buffer array
		else
		nBitCounterBuf++; // Increment our bit counter for buffer

		if(nBitCounterRes == 7) // Counter for bits in result array
		{nElementRes++; nBitCounterRes = 0;} // Element in result array
		else		
		nBitCounterRes++; // Increment our bit counter for result
	}

	return pucResult;
}

UCHAR* unBitStuff(const UCHAR* pucBuffer, const size_t nSize)
{
	size_t nLenght = nSize/sizeof(*pucBuffer); // Lenght of an array
	UCHAR* pucResult = (UCHAR*) malloc(nSize); //Allocate memory for new unBitstaffed array
	memset(pucResult,'\0',nSize);

	int nUnits = 0; //Counter of units	
	int nBitCounterBuf = 0; // Bit counter for buffer
	int nBitCounterRes = 0; // Bit counter for result
	int nElementBuf = 0; // Element counter for buffer array
	int nElementRes = 0; // Element counter for result array
	
	while(nElementBuf < nLenght)
	{
		

		// Set bit in result to bit from buffer
		setBit(pucResult[nElementRes],(nBitCounterRes), getBit(pucBuffer[nElementBuf], nBitCounterBuf));
					
		// If bit is '1', then increment counter of units
		if( getBit(pucResult[nElementRes], nBitCounterRes) )
		{nUnits++;	}
		else
		{nUnits = 0;}

		// If counter of units counts 5 units, then let's add '0' after them
		if(nUnits == 5)
		{
			// If here's not last bit in this element of buffer array, then skip to a next bit
			if(nBitCounterBuf < 7)
			{nBitCounterBuf++; }
			else if(nBitCounterBuf == 7) // If here's last bit, then skip to a next element of array
			{
				nBitCounterBuf = 0; 
				nElementBuf++; 
			}

			// After skippng '0' set units counter to 0
			nUnits = 0;
		}

		if(nBitCounterBuf == 7) // Counter for bits in buffer
		{nElementBuf++; nBitCounterBuf = 0;} // Element in buffer array
		else
		nBitCounterBuf++; // Increment our bit counter for buffer

		if(nBitCounterRes == 7) // Counter for bits in result array
		{nElementRes++; nBitCounterRes = 0;} // Element in result array
		else		
		nBitCounterRes++; // Increment our bit counter for result
	}

	return pucResult;
}

bool getBit(UCHAR cBuffer, int nBitShift)
{
	return cBuffer & (1U << nBitShift);
}

void setBit(UCHAR &cBuffer, int nBitShift, bool bSetTo)
{
	if(bSetTo)
	{ cBuffer = cBuffer | (1U << nBitShift); }
	else
	{ cBuffer = cBuffer & ~(1U << nBitShift); }
}

void showBits(const UCHAR ucBuffer)
{
	for(int i = 0; i < 8; i++)
	std::cout<<getBit(ucBuffer,(7-i));

	std::cout<<std::endl;
}

#endif
