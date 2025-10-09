#define IER_RBR		0x01
#define IER_THRE	0x02
#define IER_RLS		0x04

#define IIR_PEND	0x01
#define IIR_RLS		0x03
#define IIR_RDA		0x02
#define IIR_CTI		0x06
#define IIR_THRE	0x01

#define LSR_RDR		0x01
#define LSR_OE		0x02
#define LSR_PE		0x04
#define LSR_FE		0x08
#define LSR_BI		0x10
#define LSR_THRE	0x20
#define LSR_TEMT	0x40
#define LSR_RXFE	0x80

#define BUFSIZE		0x40


void UARTInit(uint32_t Baudrate);
//void UART_IRQHandler(void);
void UARTSend(uint8_t *BufferPtr, uint32_t Length);
void UARTSendHex(int hexValue);
void UARTPuts(const char *s);
void UARTSendChar(unsigned char value);
unsigned char UARTIsDataReceived(void);
unsigned char UARTGetReceivedByte(void);
int UARTParseCommand(unsigned char *rxCommandBuffer, int *length);
int strCopy(char *input, unsigned char *output, int offset);
void UARTSendCommand(unsigned char commandType, unsigned char *commandBuffer, int length);
int strInt8ToString(unsigned char value, char *buffer);

