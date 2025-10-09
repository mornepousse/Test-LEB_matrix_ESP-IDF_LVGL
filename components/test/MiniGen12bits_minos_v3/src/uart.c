#include "LPC11xx.h"
#include "uart.h"

/*****************************************************************************
** Function name:		UARTInit
**
** Descriptions:		Initialize UART0 port, setup pin select,
**				clock, parity, stop bits, FIFO, etc.
**
** parameters:			UART baudrate
** Returned value:		None
**
*****************************************************************************/
void UARTInit(uint32_t baudrate)
{
  uint32_t Fdiv;
  uint32_t regVal;


  NVIC_DisableIRQ(UART_IRQn);

  LPC_IOCON->PIO1_6 &= ~0x07;    /*  UART I/O config */
  LPC_IOCON->PIO1_6 |= 0x01;     /* UART RXD */
  LPC_IOCON->PIO1_6 |= (2<<3);     /* pull up on  RXD */
  LPC_IOCON->PIO1_7 &= ~0x07;
  LPC_IOCON->PIO1_7 |= 0x01;     /* UART TXD */
  /* Enable UART clock */
  LPC_SYSCON->SYSAHBCLKCTRL |= (1<<12);
  LPC_SYSCON->UARTCLKDIV = 0x1;     /* divided by 1 */

  LPC_UART->LCR = 0x83;             /* 8 bits, no Parity, 1 Stop bit */
  regVal = LPC_SYSCON->UARTCLKDIV;

  Fdiv = (((SystemCoreClock*LPC_SYSCON->SYSAHBCLKDIV)/regVal)/16)/baudrate ;	/*baud rate */

  LPC_UART->DLM = Fdiv / 256;
  LPC_UART->DLL = Fdiv % 256;
  LPC_UART->LCR = 0x03;		/* DLAB = 0 */
  LPC_UART->FCR = 0x07;		/* Enable and reset TX and RX FIFO. */

  /* Read to clear the line status. */
  regVal = LPC_UART->LSR;

  /* Ensure a clean start, no data in either TX or RX FIFO. */
// CodeRed - added parentheses around comparison in operand of &
  while (( LPC_UART->LSR & (LSR_THRE|LSR_TEMT)) != (LSR_THRE|LSR_TEMT) );
  while ( LPC_UART->LSR & LSR_RDR )
  {
	regVal = LPC_UART->RBR;	/* Dump data from RX FIFO */
  }

 // LPC_UART->IER |= 0x01;

  /* Enable the UART Interrupt */
  //NVIC_EnableIRQ(UART_IRQn);

//#if CONFIG_UART_ENABLE_INTERRUPT==1
//#if CONFIG_UART_ENABLE_TX_INTERRUPT==1
//  LPC_UART->IER = IER_RBR | IER_THRE | IER_RLS;	/* Enable UART interrupt */
//#else
//  LPC_UART->IER = IER_RBR | IER_RLS;	/* Enable UART interrupt */
//#endif
//#endif
}

/*****************************************************************************
** Function name:		UARTSend
**
** Descriptions:		Send a block of data to the UART 0 port based
**				on the data length
**
** parameters:		buffer pointer, and data length
** Returned value:	None
**
*****************************************************************************/
void UARTSend(uint8_t *BufferPtr, uint32_t Length)
{

  while ( Length != 0 )
  {

	  while ( !(LPC_UART->LSR & LSR_THRE) );
	  LPC_UART->THR = *BufferPtr;
      BufferPtr++;
      Length--;
  }
  return;
}

unsigned char UARTIsDataReceived(void)
{
		if((LPC_UART->LSR & LSR_RDR) != 0)return 1;
	  //if((LPC_UART->IIR&0x0F) == 0x0004)return 1;
	  else return 0;
}

unsigned char UARTGetReceivedByte(void)
{
	//dummy = LPC_UART->LSR;
	return LPC_UART->RBR;
}

/*****************************************************************************
** Function name:		UARTSend
**
** Descriptions:		Send a block of data to the UART 0 port based
**				on the data length
**
** parameters:		buffer pointer, and data length
** Returned value:	None
**
*****************************************************************************/
void UARTPuts(const char *s)
{
  while ( *s != 0 )
  {
	  while ( !(LPC_UART->LSR & LSR_THRE) );
	  LPC_UART->THR = *s;
      s++;
  }
}

void UARTSendChar(unsigned char value)
{
	unsigned char txChar;

	 while ( !(LPC_UART->LSR & LSR_THRE) );
	 txChar = (value>>4);
	 if(txChar > 9)txChar += ('A' -10);
	 else txChar += '0';
	 LPC_UART->THR = txChar;

	 txChar = (value&0x0F);
	 if(txChar > 9)txChar += ('A' -10);
	 else txChar += '0';
	 LPC_UART->THR = txChar;
}
void UARTSendHex(int hexValue)
{
	unsigned char txChar;

	 while ( !(LPC_UART->LSR & LSR_THRE) );

	 txChar = (hexValue>>28);
	 if(txChar > 9)txChar += ('A' -10);
	 else txChar += '0';
	 LPC_UART->THR = txChar;

	 txChar = ((hexValue>>24)&0x0000000F);
	 if(txChar > 9)txChar += ('A' -10);
	 else txChar += '0';
	 LPC_UART->THR = txChar;

	 txChar = ((hexValue>>20)&0x0000000F);
	 if(txChar > 9)txChar += ('A' -10);
	 else txChar += '0';
	 LPC_UART->THR = txChar;

	 txChar = ((hexValue>>16)&0x0000000F);
	 if(txChar > 9)txChar += ('A' -10);
	 else txChar += '0';
	 LPC_UART->THR = txChar;

	 txChar = ((hexValue>>12)&0x0000000F);
	 if(txChar > 9)txChar += ('A' -10);
	 else txChar += '0';
	 LPC_UART->THR = txChar;

	 txChar = ((hexValue>>8)&0x0000000F);
	 if(txChar > 9)txChar += ('A' -10);
	 else txChar += '0';
	 LPC_UART->THR = txChar;

	 txChar = ((hexValue>>4)&0x0000000F);
	 if(txChar > 9)txChar += ('A' -10);
	 else txChar += '0';
	 LPC_UART->THR = txChar;

	 txChar = ((hexValue>>0)&0x0000000F);
	 if(txChar > 9)txChar += ('A' -10);
	 else txChar += '0';
	 LPC_UART->THR = txChar;

}

enum RxStateEnum
{
	WAIT_CMD_START,
	WAIT_CMD_TYPE,
	WAIT_CMD_LENGTH_MSB,
	WAIT_CMD_LENGTH_LSB,
	WAIT_CMD_PAYLOAD,
	WAIT_CMD_CHECKSUM_MSB,
	WAIT_CMD_CHECKSUM_LSB,
};

static enum RxStateEnum rxState;
static unsigned int rxTimeout;
static unsigned short rxCommandChecksum, rxCommandRcvChecksum, rxCommandLength,rxCommandIndex;
static unsigned char previousRxByte = 0;
static unsigned char rxCommandType;

int UARTParseCommand(unsigned char *rxCommandBuffer, int *length)
{
	unsigned char rxByte;
	*length = 0;

	if(UARTIsDataReceived())
	{
		rxTimeout = 1000000;
		while(UARTIsDataReceived())
		{
			rxByte = UARTGetReceivedByte();
			switch(rxState)
			{
			case WAIT_CMD_START:
				if(previousRxByte == 'C' && rxByte == '>')
				{
					rxState = WAIT_CMD_TYPE;
				}
				previousRxByte = rxByte;
				break;
			case WAIT_CMD_TYPE:
				rxCommandType = rxByte;
				if(rxCommandType == 0)rxState = WAIT_CMD_START;
				else rxState = WAIT_CMD_LENGTH_MSB;
				break;
			case WAIT_CMD_LENGTH_MSB:
				rxCommandLength = (unsigned short)rxByte<<8;
				rxState = WAIT_CMD_LENGTH_LSB;
				break;
			case WAIT_CMD_LENGTH_LSB:
				rxCommandLength |= (unsigned short)rxByte;
				if(rxCommandLength != 0)rxState = WAIT_CMD_PAYLOAD;
				else rxState = WAIT_CMD_CHECKSUM_MSB;
				rxCommandChecksum = 0;
				rxCommandIndex = 0;
				break;
			case WAIT_CMD_PAYLOAD:
				rxCommandBuffer[rxCommandIndex] = rxByte;
				rxCommandIndex++;
				rxCommandChecksum += rxByte;
				if(rxCommandIndex == rxCommandLength)
				{
					rxState = WAIT_CMD_CHECKSUM_MSB;
				}
				break;
			case WAIT_CMD_CHECKSUM_MSB:
				rxCommandRcvChecksum = ((unsigned short)rxByte)<<8;
				rxState = WAIT_CMD_CHECKSUM_LSB;
				break;

			case WAIT_CMD_CHECKSUM_LSB:
				rxCommandRcvChecksum |= rxByte;
				rxCommandRcvChecksum ^= 0xAA55;
				rxState = WAIT_CMD_START;
				if(rxCommandRcvChecksum == rxCommandChecksum)
				{
					*length = rxCommandLength;
					return rxCommandType;
				}
				else
				{
					return -1;
				}
				break;

			}
		}
	}
	else
	{
		if(rxTimeout != 0)
		{
			rxTimeout--;
		}
		else
		{
			//rxState = WAIT_CMD_START;
		}
	}
	return 0;
}

void UARTSendCommand(unsigned char commandType, unsigned char *commandBuffer, int length)
{
	unsigned short checksum;
	int index;
	LPC_UART->THR = 'C';
	LPC_UART->THR = '>';
	LPC_UART->THR = commandType;
	LPC_UART->THR = (unsigned char)(length>>8);
	LPC_UART->THR = (unsigned char)length;
	checksum = 0;
	index = 0;
	while ( !(LPC_UART->LSR & LSR_THRE) );

	if(length != 0)
	{
		while(index != length)
		{
			LPC_UART->THR = commandBuffer[index];
			checksum += commandBuffer[index];
			index++;
			if((index&0x07)== 0)
			{
				while ( !(LPC_UART->LSR & LSR_THRE) );
			}

		}
	}
	checksum ^= 0xAA55;
	LPC_UART->THR = (unsigned char)(checksum>>8);
	LPC_UART->THR = (unsigned char)checksum;


}

int strCopy(char *input, unsigned char *output, int offset)
{
	int finalOffset;
	finalOffset = offset;
	output += offset;
	while(*input != 0)
	{
		*output = (unsigned char)*input;
		output++;
		input++;
		finalOffset++;
	}
	*output = (unsigned char)*input;
	finalOffset++;
	return finalOffset;
}

int strInt8ToString(unsigned char value, char *buffer)
{
	char digit;
	int index;

	index = 0;
	digit = 0;
	while(value > 99)
	{
		digit++;
		value-=100;
	}
	if(digit!=0)
	{
		buffer[index++] = digit + '0';
		digit = 0;
	}
	while(value > 9)
	{
		digit++;
		value-=10;
	}
	buffer[index++] = digit + '0';
	buffer[index++] = value + '0';
	return index;

}
