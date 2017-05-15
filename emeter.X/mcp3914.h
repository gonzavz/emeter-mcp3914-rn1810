/* 
 * File:   mcp3914.h
 * Author: gonza
 *
 * Created on April 29, 2017, 6:48 PM
 */

#ifndef MCP3914_H
#define	MCP3914_H

#ifdef	__cplusplus
extern "C" {
#endif
typedef union
{ 
    struct
    {
        unsigned char b0:1;					//Not Implemented
        unsigned char b1:1;					//Not Implemented
        unsigned char b2:1;					//Not Implemented
        unsigned char b3:1;					//Not Implemented
        unsigned char TM0:1;				//Test modes
        unsigned char TM1:1;				//Test modes
        unsigned char ENINT:1;				//Enable interrupt on Register Map CRC
        unsigned char ENCRCN:1;				//Enable CRC for Channel Reads
        unsigned char WIDTHdata0:1;			//Global width setting for all channels
        unsigned char WIDTHdata1:1;			//Global width setting for all channels
        unsigned char WIDTH_CRC:1;			//Width setting for CRC word
        unsigned char DR_LINK:1;			//Data Ready Link Control
        unsigned char DR_HIZ:1;				//Data Ready Pin Inactive State Control
        unsigned char WRITE:1;				//Loop Single Register Write
        unsigned char READ0:1;				//Address Loop Setting
        unsigned char READ1:1;				//Address Loop Setting
    } bits;
    unsigned int     int_val;       		//INT access

} STATUSCOM_FLAGS;

unsigned long Read3914(char address);
void Read_Internal_Registers(void);

#ifdef	__cplusplus
}
#endif

#endif	/* MCP3914_H */

