//crc8.h
typedef uint8_t crc;

#define POLYNOMIAL 0xD8 //11011000
#define BIT7 (1 << 7)

//Generate crc8 table
void crcGenTable(uint8_t crcTable[]); //crcTable[256]

//Get crc from message
crc crcGet(uint8_t crcTable[], //Generated Table
			uint8_t const message[], //Data for calculating crc
			int nBytes); //number of bytes of data
			
//Check crc's on matching
bool crcCheck(uint8_t crc1, uint8_t crc2);


//Generate table for CRC8
void crcGenTable(uint8_t crcTable[])
{
    crc  remainder = 0;	
    //Perform modulo-2 division, a byte at a time.
    for (int dividend = 0; dividend < 256; ++dividend)
    {
        //Bring the next byte into the remainder.
        remainder = dividend;
		
		//Perform modulo-2 division, a bit at a time.
        for (uint8_t bit = 8; bit > 0;){
            //Try to divide the current data bit.
            if (remainder & BIT7){
                remainder = remainder ^ POLYNOMIAL;
            } else {
                remainder = (remainder << 1);
				--bit;
            }
        }
		//The final remainder is the CRC result.
		crcTable[dividend] = remainder;
    }
}   /* crcGenTable() */


//Get CRC from data
crc crcGet(uint8_t crcTable[], //Generated Table
			uint8_t const message[], //Data for calculating crc
			int nBytes){ //number of bytes of data
    uint8_t data;
    crc remainder = 0;

    //Divide the message by the polynomial, a byte at a time.
    for (int byte = 0; byte < nBytes; ++byte){
        data = message[byte] ^ remainder;
        remainder = crcTable[data];
    }
    
	//The final remainder is the CRC.
    return (remainder);
}   /* crcGet() */


//True if CRC's matched
bool crcCheck(uint8_t crc1, uint8_t crc2){
    
	bool bMatch;
	if(crc1 ^ crc2)
		bMatch = false;
	else 
		bMatch = true;
    
    return bMatch;
}   /* crcCheck() */
