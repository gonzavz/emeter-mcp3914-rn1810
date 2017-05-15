#include "mcp3914.h"
#include "mcc_generated_files/mcc.h"

static unsigned long internal_registers[24];
STATUSCOM_FLAGS Configs;

unsigned long Read3914(char address)
{
	char i;
	long data;
    unsigned char read_address;
    

	MCP3914_CS_SetLow();
	data=0;
	i=SPI1BUF;
	read_address=(address<<1);
	read_address|=0b01000001;
	SPI1BUF=read_address;
	while (!SPI1STATbits.SPIRBF);
	i=SPI1BUF;
	if (address<8)
	{
		if (Configs.bits.WIDTHdata1)							//32bit
		{
			if (Configs.bits.WIDTHdata0)
			{
				SPI1BUF=0;	
				while (!SPI1STATbits.SPIRBF);
				i=SPI1BUF;
				SPI1BUF=0;	
				while (!SPI1STATbits.SPIRBF);
				data+=SPI1BUF;
				data=(data<<8);
				SPI1BUF=0;
				while (!SPI1STATbits.SPIRBF);
				data+=SPI1BUF;
				data=(data<<8);
				SPI1BUF=0;
				while (!SPI1STATbits.SPIRBF);
				data+=SPI1BUF;
			}
			else
			{
				SPI1BUF=0;	
				while (!SPI1STATbits.SPIRBF);
				data+=SPI1BUF;
				data=(data<<8);
				SPI1BUF=0;
				while (!SPI1STATbits.SPIRBF);
				data+=SPI1BUF;
				data=(data<<8);
				SPI1BUF=0;
				while (!SPI1STATbits.SPIRBF);
				data+=SPI1BUF;
			}
		}
		else
		{
			SPI1BUF=0;	
			while (!SPI1STATbits.SPIRBF);
			data=SPI1BUF;
			data=(data<<8);
			SPI1BUF=0;	
			while (!SPI1STATbits.SPIRBF);
			data+=SPI1BUF;
			data=(data<<8);
			SPI1BUF=0;
			while (!SPI1STATbits.SPIRBF);
			if (Configs.bits.WIDTHdata0)
			{
				data+=SPI1BUF;
			}
			else i=SPI1BUF;
		}
	}
	else
	{
		SPI1BUF=0;	
		while (!SPI1STATbits.SPIRBF);
		data=SPI1BUF;
		data=(data<<8);
		SPI1BUF=0;
		while (!SPI1STATbits.SPIRBF);
		data+=SPI1BUF;
		data=(data<<8);
		SPI1BUF=0;
		while (!SPI1STATbits.SPIRBF);
		data+=SPI1BUF;
	}
	MCP3914_CS_SetHigh();

	return(data);
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//  Reads the all the registers from MCP3914
void Read_Internal_Registers(void)
{
	unsigned char i, address;
	for(i=0; i<23; i++)
	{
		address=i+9;
		internal_registers[i]=Read3914(address);
	}
	Configs.int_val=(unsigned int)(internal_registers[3]>>8);
}