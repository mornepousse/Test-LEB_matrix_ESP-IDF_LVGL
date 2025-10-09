/*
===============================================================================
 Name        : main.c
 Author      : 
 Version     :
 Copyright   : Copyright (C) 
 Description : main definition
===============================================================================
*/

#ifdef __USE_CMSIS
#include "LPC11xx.h"
#endif

#include <cr_section_macros.h>
#include <NXP/crp.h>
#include "SpiDac.h"
#include "SPI_W25Qxx.h"
#include "uart.h"
#include "MemoryFileSystem.h"

#define PECHE_CANARDS


#define HARDWARE_VERSION "HARD: BDSG1 Rev C"
#define SOFTWARE_VERSION "SOFT: v1.0.1 01/03/2013"
// Variable to store CRP value in. Will be placed automatically
// by the linker when "Enable Code Read Protect" selected.
// See crp.h header for more information
//__CRP const unsigned int CRP_WORD = CRP_NO_CRP ;
__CRP const unsigned int CRP_WORD = CRP_CRP1 ;

// TODO: insert other include files here
#define GetBp1Level() (LPC_GPIO3->DATA&(1<<3))
#define GetBp2Level() (LPC_GPIO2->DATA&(1<<6))

#define OutASetHigh() (LPC_GPIO3->DATA |= (1<<1))
#define OutASetLow() (LPC_GPIO3->DATA &= ~(1<<1))

#define OutBSetHigh() (LPC_GPIO0->DATA |= (1<<5))
#define OutBSetLow() (LPC_GPIO0->DATA &= ~(1<<5))


///Led control
static unsigned int slowTimerPrescaler;
static unsigned char slowTimerFlag;

#define LED_SLOW_BLINK		0xFF
#define	LED_MEDIUM_BLINK	0xAA
#define LED_FAST_BLINK		0x44
#define LED_VFAST_BLINK		0x22
#define LED_SLOW_FLASH		0x2F
#define LED_MEDIUM_FLASH	0x2A
#define LED_FAST_FLASH		0x25
#define LED_VFAST_FLASH		0x13

typedef struct
{
	unsigned char Level 				:1;
	unsigned char RisingEdgeDetected	:1;
	unsigned char FallingEdgeDetected	:1;
	unsigned char Reserved				:4;
	unsigned char CurrentLevel			:1;
	int PushTime;
	int InactiveTime;
	unsigned int ShiftRegister;
}ButtonStruct;

static ButtonStruct BP1, BP2, BP_1, BP_2, BP_3, BP_4, BP_5, BP_6, BP_7, BP_8;
static int buttonPrescaler;

#define CMD_READ_HARDWARE_INFO 0x08
#define CMD_FORMAT_MEMORY		0xA0
#define CMD_READ_MEMORY			0x12
#define CMD_CREATE_FILE			0x20
#define CMD_WRITE_FILE			0x24
#define CMD_READ_FILE			0x21
#define CMD_CLOSE_FILE			0x2F
#define CMD_READ_ENTRY			0x2A
#define CMD_DELETE_FILE			0x28


// TODO: insert other definitions and declarations here

unsigned char ReadInputMode(void);

void Init1000HzTimer(void);
void InitButtonPinFunction(void);
void ADCInit( uint32_t ADC_Clk );
uint32_t ADCRead( uint8_t channelNum );

void WatchdogFeed(void);
void WatchdogInit(void);

int main(void) {
	volatile static int i = 0 ;

	DateStruct firstDate = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
	DateStruct currentDate;
	unsigned int j, tmpInt0, tmpInt1;
	unsigned char buffer[128];
	unsigned char unsignedBuffer[1024];
	unsigned char *uint8Ptr;
	char tmpBuffer[20];
	unsigned char ledPrescaler;
	unsigned char ledMode;
	unsigned char ledState = 0;
	unsigned short memoryType;
	int bufferIndex;
	unsigned char rxCommand;
	unsigned char rxCommandBuffer[256];
	int rxCommandLength;
	signed short workBuffer[1000];
	int readByteCount, sampleCount;
	FileEntryStruct entryParams;
	FileEntryStruct audioFileEntry;
	int autoStartingTimmer = 100;
	int toggleArretDepart = 0;
	unsigned char minimumTime = 44;

	unsigned char	stopSoundToDo  = 0;
	//unsigned char optionModeValue = 0;

	//unsigned char canard1Count, canard2Count;

	unsigned char audioSelectIndex = 0;

	unsigned int adcValue = 0;

	WatchdogInit();
	SpiDacInit();
	InitAudioTimer();
	for(j=0; j!=1000000; j++){WatchdogFeed();}
	LPC_GPIO1->DATA &= ~(1<<5);
	WatchdogInit();


	AudioContextStruct audioContext;

	audioContext.StateIndex = 0;
	audioContext.SampleBitsCount = 8;


	autoStartingTimmer = 0;

	slowTimerPrescaler = 10;
	ledPrescaler = 10;
	ledMode = LED_SLOW_BLINK;
	buttonPrescaler = 25;

	LPC_GPIO1->DIR |= (1<<5);
	LPC_GPIO1->DATA &= ~(1<<5);



	InitButtonPinFunction();

	LPC_GPIO0->DIR |= (1<<5);
	LPC_GPIO0->DATA |= (1<<5);

	// TODO: insert code here

	ADCInit(2400000);
	SpiDacInit();
	InitAudioTimer();

	SpiMemoryInit();
	UARTInit(1500000);
	//UARTInit(115200);
	Init1000HzTimer();

	//LPC_GPIO1->DATA |= (1<<5);

	//while(1);

	memoryType = W25Q_ReadManfDeviceId();
	MemoryReadFirstDate(&firstDate);


/*	for(i = 0; i != MAXIMUM_SOUND_COUNT, i++)
	{
		//Read entry
		if(entryType == AUDIO_FILE)
		{

		}
	}*/

	audioContext.StateIndex = 0;
	audioContext.SampleBitsCount = 8;





	while(1)
	{
		i++ ;

		WatchdogFeed();
//		LPC_GPIO1->DATA |= (1<<5);
		//BP1.CurrentLevel = 1;

		/*if(ADCRead(0) < 416)
		{*/

		/*audioContext.StateIndex = 0;
		 */


		if(autoStartingTimmer != 0 || stopSoundToDo != 0)
		{

			BP1.RisingEdgeDetected = 0;
			BP2.RisingEdgeDetected = 0;

			audioContext.StateIndex = 0;
			audioContext.SampleBitsCount = 12;

			MemoryGetNextEntry(0x01, &audioFileEntry);
			MemoryGetNextEntry(0x00, &audioFileEntry);

			if(stopSoundToDo != 0)MemoryGetNextEntry(0x00, &audioFileEntry);
			//stopSoundToDo = 0;

			currentFileSize = audioFileEntry.Size;

			currentFileAddress = audioFileEntry.FileAddress;

			currentFileOffset = 0;

			readByteCount = MemoryReadFile(unsignedBuffer, 256);

			LPC_GPIO1->DATA |= (1<<5);

			while(readByteCount != 0)
			{
				readByteCount = MemoryReadFile(unsignedBuffer, 500);

				sampleCount = MemoryDecodeAudioFile(unsignedBuffer, readByteCount, workBuffer, &audioContext);
				//sampleCount = readByteCount;
				WatchdogFeed();
				while(AudioDualBufferReady() != 1)
				{
					if(slowTimerFlag == 1)
					{
						if(autoStartingTimmer != 0)autoStartingTimmer--;
						slowTimerFlag = 0;
					}
				}
				AudioWriteDualBuffer(workBuffer, sampleCount);
				if(autoStartingTimmer == 0 && stopSoundToDo == 0)stopSoundToDo = 1;
				else stopSoundToDo = 0;

				//if(autoStartingTimmer == 0 || BP1.RisingEdgeDetected == 1)break;
			}

		}
		//}
		//if(autoStartingTimmer == 0)
		if(BP1.RisingEdgeDetected == 1)	// || autoStartingTimmer == 0)
		{



			audioContext.StateIndex = 0;
			audioContext.SampleBitsCount = 12;

			MemoryGetNextEntry(0x01, &audioFileEntry);
			MemoryGetNextEntry(0x00, &audioFileEntry);	//son continu
			MemoryGetNextEntry(0x00, &audioFileEntry); // son arret
			MemoryGetNextEntry(0x00, &audioFileEntry); // son demarrage

			if(autoStartingTimmer != 0)MemoryGetNextEntry(0x00, &audioFileEntry);//acceleration

			/*if(autoStartingTimmer != 0)
			{
					audioSelectIndex++;
					if(audioSelectIndex > 3)audioSelectIndex = 0;
					for(j= audioSelectIndex; j != 0; j--)MemoryGetNextEntry(0x00, &audioFileEntry);
				}*/
			autoStartingTimmer = 400;





			//if(BP2.RisingEdgeDetected == 1 || BP2.CurrentLevel == 1)MemoryGetNextEntry(0x00, &audioFileEntry);

			BP1.RisingEdgeDetected = 0;
			BP2.RisingEdgeDetected = 0;

			currentFileSize = audioFileEntry.Size;

			currentFileAddress = audioFileEntry.FileAddress;

			currentFileOffset = 0;

			readByteCount = MemoryReadFile(unsignedBuffer, 256);

			LPC_GPIO1->DATA |= (1<<5);

			minimumTime = 40;

			while(readByteCount != 0)
			{
				readByteCount = MemoryReadFile(unsignedBuffer, 500);

				sampleCount = MemoryDecodeAudioFile(unsignedBuffer, readByteCount, workBuffer, &audioContext);
				//sampleCount = readByteCount;
				WatchdogFeed();
				while(AudioDualBufferReady() != 1)
				{
					//if(BP_1.RisingEdgeDetected == 1 || BP_2.RisingEdgeDetected == 1 || BP_3.RisingEdgeDetected == 1 || BP_4.RisingEdgeDetected == 1 ||
					//BP_5.RisingEdgeDetected == 1 || BP_6.RisingEdgeDetected == 1 || BP_7.RisingEdgeDetected == 1 || BP_8.RisingEdgeDetected == 1)readByteCount = 0;


				}
				AudioWriteDualBuffer(workBuffer, sampleCount);
			}


			BP1.RisingEdgeDetected = 0;
			BP2.RisingEdgeDetected = 0;
			OutBSetHigh();

		}





		/// Uart command manager
		rxCommand = UARTParseCommand(rxCommandBuffer, &rxCommandLength);

		if(rxCommand != 0)autoStartingTimmer = 0;
		if(rxCommand == CMD_WRITE_FILE)
		{

			MemoryWriteFile(rxCommandBuffer, rxCommandLength);
			unsignedBuffer[0] = CMD_WRITE_FILE;
			UARTSendCommand(CMD_WRITE_FILE, unsignedBuffer, 1);
		}
		else if(rxCommand == CMD_READ_HARDWARE_INFO)
		{
			bufferIndex = strCopy(HARDWARE_VERSION, buffer,0);
			bufferIndex = strCopy(SOFTWARE_VERSION, buffer,bufferIndex);

			strInt8ToString(firstDate.Day, tmpBuffer);
			tmpBuffer[2] = '/';
			strInt8ToString(firstDate.Month, tmpBuffer+3);
			tmpBuffer[5] = '/';
			tmpBuffer[6] = '2';
			tmpBuffer[7] = '0';
			strInt8ToString(firstDate.Year, tmpBuffer+8);
			tmpBuffer[10] = ' ';
			strInt8ToString(firstDate.Hour, tmpBuffer+11);
			tmpBuffer[13] = 'h';
			tmpBuffer[14] = 0;
			bufferIndex = strCopy(tmpBuffer, buffer,bufferIndex);

			if(memoryType == 0xEF14)bufferIndex = strCopy("W25Q16 Windbond 16Mb SPI Flash", buffer,bufferIndex);
			else if(memoryType == 0xEF15)bufferIndex = strCopy("W25Q32 Windbond 32Mb SPI Flash", buffer,bufferIndex);
			else if(memoryType == 0xEF16)bufferIndex = strCopy("W25Q64 Windbond 64Mb SPI Flash", buffer,bufferIndex);
			else bufferIndex = strCopy("SPI Flash not detected", buffer,bufferIndex);

			W25Q_ReadUniqueIdString(tmpBuffer);
			bufferIndex = strCopy("ID ", buffer,bufferIndex);

			bufferIndex = strCopy(tmpBuffer, buffer,bufferIndex-1);

			UARTSendCommand(CMD_READ_HARDWARE_INFO, buffer, bufferIndex);
		}
		else if(rxCommand == CMD_FORMAT_MEMORY)
		{
			if(rxCommandLength == 6 && rxCommandBuffer[0] == CMD_FORMAT_MEMORY && rxCommandBuffer[1] == 0xAA)
			{
				if(firstDate.Day > 31 || firstDate.Month > 12 || firstDate.Month == 0)
				{
					firstDate.Day = rxCommandBuffer[2];
					firstDate.Month = rxCommandBuffer[3];
					firstDate.Year = rxCommandBuffer[4];
					firstDate.Hour = rxCommandBuffer[5];
				}
				MemoryFormat(&firstDate);
				unsignedBuffer[0] = CMD_FORMAT_MEMORY;
				UARTSendCommand(CMD_FORMAT_MEMORY, unsignedBuffer, 1);
			}
		}
		else if(rxCommand == CMD_READ_MEMORY)
		{
			if(rxCommandLength == 8 && rxCommandBuffer[0] == CMD_READ_MEMORY)
			{
				tmpInt0 = (int)rxCommandBuffer[3]<<16 | (int)rxCommandBuffer[4]<<8 | (int)rxCommandBuffer[5];
				tmpInt1 = (int)rxCommandBuffer[6]<<8 | (int)rxCommandBuffer[7];
				if(tmpInt1 > 256)tmpInt1 = 256;
				W25Q_Read(tmpInt0, tmpInt1, unsignedBuffer);
				UARTSendCommand(CMD_READ_MEMORY, unsignedBuffer, tmpInt1);

			}
		}
		else if(rxCommand == CMD_CREATE_FILE)
		{
			currentDate.Day = rxCommandBuffer[2];
			currentDate.Month = rxCommandBuffer[3];
			currentDate.Year = rxCommandBuffer[4];
			currentDate.Hour = rxCommandBuffer[5];

			MemoryCreateFile(rxCommandBuffer[0], rxCommandBuffer+6, &currentDate);
			unsignedBuffer[0] = CMD_CREATE_FILE;
			UARTSendCommand(CMD_CREATE_FILE, unsignedBuffer, 1);

		}
		else if(rxCommand == CMD_CLOSE_FILE)
		{

			MemoryCloseFile();
			unsignedBuffer[0] = CMD_CLOSE_FILE;
			UARTSendCommand(CMD_CLOSE_FILE, unsignedBuffer, 1);
		}
		else if(rxCommand == CMD_READ_ENTRY)
		{
			MemoryGetNextEntry(rxCommandBuffer[0], &entryParams);
			uint8Ptr = (unsigned char*)&entryParams;
			UARTSendCommand(CMD_READ_ENTRY, uint8Ptr, 25);
		}

		/*switch(globalStatemachine)
		{
		case IDLE:
			if(rxCommand == CMD_READ_HARDWARE_INFO)
			{
				bufferIndex = strCopy(HARDWARE_VERSION, buffer,0);
				bufferIndex = strCopy(SOFTWARE_VERSION, buffer,bufferIndex);
				bufferIndex = strCopy("W25Q16 Windbond 16Mb SPI Flash", buffer,bufferIndex);

			}
		}*/


		//SpiDacCsLow();

		//if(GetBp1Level()!=0)LPC_GPIO1->DATA &= ~(1<<5);
		//else LPC_GPIO1->DATA |= (1<<5);

//		for(j=0; j!=1000000; j++)LPC_GPIO0->DATA &= ~(1<<7);


		//LED management
		if(slowTimerFlag == 1)
		{
			if(autoStartingTimmer != 0)autoStartingTimmer--;

			slowTimerFlag = 0;

			ledPrescaler--;
			if(ledPrescaler == 0)
			{
				if(ledMode == 0)
				{
					LPC_GPIO1->DATA &= ~(1<<5);
					ledPrescaler = 15;
				}
				else if(ledState == 1)
				{
					ledPrescaler = ledMode&0x0F;
					ledState = 0;
					LPC_GPIO1->DATA &= ~(1<<5);
				}
				else
				{
					ledPrescaler = ledMode>>4;
					ledState = 1;
					LPC_GPIO1->DATA |= (1<<5);
				}


			}
		}
	}
}

void Init1000HzTimer(void)
{
	LPC_SYSCON->SYSAHBCLKCTRL |= (1<<8);
	LPC_TMR16B1->TCR |= (1<<1);		//reset timer module
	LPC_TMR16B1->TCR = 0;			//release reset

	LPC_TMR16B1->PR = 47;		//No prescaler every MCLK
	LPC_TMR16B1->MR0 = 999;
	LPC_TMR16B1->MCR = 3; //Interup and reset on match 0
	LPC_TMR16B1->CTCR = 0; //acts as a timer
	LPC_TMR16B1->TCR = 1;
	   /* Enable the TIMER1 Interrupt */
	    NVIC_EnableIRQ(TIMER_16_1_IRQn);
}

void BOD_IRQHandler(void)
{
	  NVIC_SystemReset();
}

void WDT_IRQHandler(void)
{
	  NVIC_SystemReset();
}

void TIMER16_1_IRQHandler(void)
{
	unsigned short adcValue;

  if ( LPC_TMR16B1->IR & 0x1 )
  {
	  LPC_TMR16B1->IR = 1;			/* clear interrupt flag */


	  buttonPrescaler--;
	  if(buttonPrescaler == 0)
	  {
		  //LPC_GPIO1->DATA |= (1<<5);
		  buttonPrescaler = 120;

		  /// BP1 scan
		  BP1.ShiftRegister = BP1.ShiftRegister<<1;
		  if(BP1.PushTime > 50)
		  {
			  BP1.ShiftRegister |= 1;
			 // LPC_GPIO1->DATA |= (1<<5);
		  }
		  BP1.PushTime = 0;

		  if((BP1.ShiftRegister&7) == 1)BP1.RisingEdgeDetected = 1;
		  if((BP1.ShiftRegister&7) == 6)BP1.FallingEdgeDetected = 1;
		  if((BP1.ShiftRegister&7) == 7)BP1.CurrentLevel = 1;
		  if((BP1.ShiftRegister&7) == 0)BP1.CurrentLevel = 0;

		  /// BP2 scan
		  BP2.ShiftRegister = BP2.ShiftRegister<<1;
		  if(BP2.PushTime > 50)
		  {
			  BP2.ShiftRegister |= 1;
		  }
		  BP2.PushTime = 0;

		  if((BP2.ShiftRegister&7) == 1)BP2.RisingEdgeDetected = 1;
		  if((BP2.ShiftRegister&7) == 6)BP2.FallingEdgeDetected = 1;
		  if((BP2.ShiftRegister&7) == 7)BP2.CurrentLevel = 1;
		  if((BP2.ShiftRegister&7) == 0)BP2.CurrentLevel = 0;



		  /// BP_1 scan
		  BP_1.ShiftRegister = BP_1.ShiftRegister<<1;
		  if(BP_1.PushTime > 15)
		  {
			  BP_1.ShiftRegister |= 1;
		  }
		  BP_1.PushTime = 0;

		  if((BP_1.ShiftRegister&7) == 1)BP_1.RisingEdgeDetected = 1;
		  if((BP_1.ShiftRegister&7) == 7)BP_1.CurrentLevel = 1;
		  if((BP_1.ShiftRegister&7) == 0)BP_1.CurrentLevel = 0;

		  /// BP_2 scan
		  BP_2.ShiftRegister = BP_2.ShiftRegister<<1;
		  if(BP_2.PushTime > 15)
		  {
			  BP_2.ShiftRegister |= 1;
		  }
		  BP_2.PushTime = 0;

		  if((BP_2.ShiftRegister&7) == 1)BP_2.RisingEdgeDetected = 1;
		  if((BP_2.ShiftRegister&7) == 7)BP_2.CurrentLevel = 1;
		  if((BP_2.ShiftRegister&7) == 0)BP_2.CurrentLevel = 0;

		  /// BP_3 scan
		  BP_3.ShiftRegister = BP_3.ShiftRegister<<1;
		  if(BP_3.PushTime > 15)
		  {
			  BP_3.ShiftRegister |= 1;
		  }
		  BP_3.PushTime = 0;

		  if((BP_3.ShiftRegister&7) == 1)BP_3.RisingEdgeDetected = 1;
		  if((BP_3.ShiftRegister&7) == 7)BP_3.CurrentLevel = 1;
		  if((BP_3.ShiftRegister&7) == 0)BP_3.CurrentLevel = 0;

		  /// BP_4 scan
		  BP_4.ShiftRegister = BP_4.ShiftRegister<<1;
		  if(BP_4.PushTime > 15)
		  {
			  BP_4.ShiftRegister |= 1;
		  }
		  BP_4.PushTime = 0;

		  if((BP_4.ShiftRegister&7) == 1)BP_4.RisingEdgeDetected = 1;
		  if((BP_4.ShiftRegister&7) == 7)BP_4.CurrentLevel = 1;
		  if((BP_4.ShiftRegister&7) == 0)BP_4.CurrentLevel = 0;

		  /// BP_5 scan
		  BP_5.ShiftRegister = BP_5.ShiftRegister<<1;
		  if(BP_5.PushTime > 15)
		  {
			  BP_5.ShiftRegister |= 1;
		  }
		  BP_5.PushTime = 0;

		  if((BP_5.ShiftRegister&7) == 1)BP_5.RisingEdgeDetected = 1;

		  /// BP_6 scan
		  BP_6.ShiftRegister = BP_6.ShiftRegister<<1;
		  if(BP_6.PushTime > 15)
		  {
			  BP_6.ShiftRegister |= 1;
		  }
		  BP_6.PushTime = 0;

		  if((BP_6.ShiftRegister&7) == 1)BP_6.RisingEdgeDetected = 1;

		  /// BP_7 scan
		  BP_7.ShiftRegister = BP_7.ShiftRegister<<1;
		  if(BP_7.PushTime > 15)
		  {
			  BP_7.ShiftRegister |= 1;
		  }
		  BP_7.PushTime = 0;

		  if((BP_7.ShiftRegister&7) == 1)BP_7.RisingEdgeDetected = 1;

		  /// BP_8 scan
		  BP_8.ShiftRegister = BP_8.ShiftRegister<<1;
		  if(BP_8.PushTime > 15)
		  {
			  BP_8.ShiftRegister |= 1;
		  }
		  BP_8.PushTime = 0;

		  if((BP_8.ShiftRegister&7) == 1)BP_8.RisingEdgeDetected = 1;

	  }
	  else
	  {
		  if(GetBp1Level()!=0)
		  {
			  BP1.PushTime++;
		  }
		  if(GetBp2Level()!=0)
		  {
			  BP2.PushTime++;
		  }

		  		  adcValue = ADCRead(3);
		  		  if(adcValue < 768)
		  		  {
		  			  if(adcValue < 282)
		  			  {
		  				  BP_1.PushTime++;
		  				  BP_5.PushTime++;
		  			  }
		  			  else if(adcValue < 416)
		  			  {
		  				  BP_1.PushTime++;
		  			  }
		  			  else  BP_5.PushTime++;
		  		 }
		  		  adcValue = ADCRead(2);
		  		  if(adcValue < 768)
		  		  {
		  			  if(adcValue < 282)
		  			  {
		  				  BP_2.PushTime++;
		  				  BP_6.PushTime++;
		  			  }
		  			  else if(adcValue < 416)
		  			  {
		  				  BP_2.PushTime++;
		  			  }
		  			  else  BP_6.PushTime++;
		  		 }
		  		  adcValue = ADCRead(1);
		  		  if(adcValue < 768)
		  		  {
		  			  if(adcValue < 282)
		  			  {
		  				  BP_3.PushTime++;
		  				  BP_7.PushTime++;
		  			  }
		  			  else if(adcValue < 416)
		  			  {
		  				  BP_3.PushTime++;
		  			  }
		  			  else  BP_7.PushTime++;
		  		 }
		  		  adcValue = ADCRead(0);
		  		  if(adcValue < 768)
		  		  {
		  			  if(adcValue < 282)
		  			  {
		  				  BP_4.PushTime++;
		  				  BP_8.PushTime++;
		  			  }
		  			  else if(adcValue < 416)
		  			  {
		  				  BP_4.PushTime++;
		  			  }
		  			  else  BP_8.PushTime++;
		  		 }
	  }

	  slowTimerPrescaler--;
	  if(slowTimerPrescaler == 0)
	  {
		  slowTimerPrescaler = 50;
		  slowTimerFlag = 1;
		  //LPC_GPIO1->DATA |= (1<<5);
	  }

  }
}

void InitButtonPinFunction(void)
{
	//Init BP1 function GPIO3.3
	LPC_IOCON->PIO3_3 = (1<<5);		//IO function and Hysteresis enabled
	LPC_GPIO3->DIR &= ~(1<<3);

	BP1.CurrentLevel = 0;
	BP1.FallingEdgeDetected = 0;
	BP1.InactiveTime = 0;
	BP1.Level = 0;
	BP1.PushTime = 0;
	BP1.RisingEdgeDetected = 0;
	BP1.ShiftRegister = 0;

	//Init BP2 function GPIO2.6
	LPC_IOCON->PIO2_6 = (1<<5);		//IO function and Hysteresis enabled
	LPC_GPIO2->DIR &= ~(1<<6);

	BP2.CurrentLevel = 0;
	BP2.FallingEdgeDetected = 0;
	BP2.InactiveTime = 0;
	BP2.Level = 0;
	BP2.PushTime = 0;
	BP2.RisingEdgeDetected = 0;
	BP2.ShiftRegister = 0;
}

#define ADC_OFFSET		0x10
#define ADC_INDEX		4

#define ADC_DONE		0x80000000
#define ADC_OVERRUN		0x40000000
#define ADC_ADINT		0x00010000

#define ADC_NUM			8			/* for LPC11xx */
#define ADC_CLK			2400000		/* set to 2.4Mhz */


/*****************************************************************************
** Function name:		ADCInit
**
** Descriptions:		initialize ADC channel
**
** parameters:			ADC clock rate
** Returned value:		None
**
*****************************************************************************/
void ADCInit( uint32_t ADC_Clk )
{
  /* Disable Power down bit to the ADC block. */
  LPC_SYSCON->PDRUNCFG &= ~(0x1<<4);

  /* Enable AHB clock to the ADC. */
  LPC_SYSCON->SYSAHBCLKCTRL |= (1<<13);

  /* Unlike some other pings, for ADC test, all the pins need
  to set to analog mode. Bit 7 needs to be cleared according
  to design team. */
  LPC_IOCON->PIO1_11   = 0x01;	// Select AD7 pin function
  LPC_IOCON->PIO1_4   = 0x01;	// Select AD5 pin function
  LPC_IOCON->R_PIO1_2   = 0x01;	// Select AD3 pin function
  LPC_IOCON->R_PIO1_1   = 0x01;	// Select AD2 pin function
  LPC_IOCON->R_PIO1_0   = 0x01;	// Select AD1 pin function
  LPC_IOCON->R_PIO0_11   = 0x01;	// Select AD0 pin function

  LPC_ADC->CR = ((SystemCoreClock/LPC_SYSCON->SYSAHBCLKDIV)/ADC_Clk-1)<<8;

  return;
}

/*****************************************************************************
** Function name:		ADCRead
**
** Descriptions:		Read ADC channel
**
** parameters:			Channel number
** Returned value:		Value read, if interrupt driven, return channel #
**
*****************************************************************************/
uint32_t ADCRead( uint8_t channelNum )
{

  uint32_t regVal, ADC_Data;


  /* channel number is 0 through 7 */
  if ( channelNum >= ADC_NUM )
  {
	channelNum = 0;		/* reset channel number to 0 */
  }
  LPC_ADC->CR &= 0xFFFFFF00;
  LPC_ADC->CR |= (1 << 24) | (1 << channelNum);
				/* switch channel,start A/D convert */

  while ( 1 )			/* wait until end of A/D convert */
  {
	regVal = *(volatile unsigned long *)(LPC_ADC_BASE
			+ ADC_OFFSET + ADC_INDEX * channelNum);
	/* read result of A/D conversion */
	if ( regVal & ADC_DONE )
	{
	  break;
	}
  }

  LPC_ADC->CR &= 0xF8FFFFFF;	/* stop ADC now */
  if ( regVal & ADC_OVERRUN )	/* save data when it's not overrun, otherwise, return zero */
  {
	return ( 0 );
  }
  ADC_Data = ( regVal >> 6 ) & 0x3FF;
  return ( ADC_Data );	/* return A/D conversion value */


}

void WatchdogInit(void)
{

	/*
	** Make sure WDT config clock is turned on
	*/
	LPC_SYSCON->SYSAHBCLKCTRL |= 0x08000;

	/*
	** Enable internal RC clock
	*/
	LPC_SYSCON->PDRUNCFG &= ~(1);

	/*
	** Select internal RC for watchdog
	*/
	LPC_SYSCON->WDTCLKSEL = 0;

	/*
	** Toggle WDTCLKUEN to latch clock selection
	*/
	LPC_SYSCON->WDTCLKUEN = 0;
	LPC_SYSCON->WDTCLKUEN = 1;

	/*
	** We are using the internal RC clock to drive the watchdog because it is
	** independent of PLL settings. The "System Clock" option routes the clock
	** after the PLL to the watchdog. The "System Clock" could vary depending
	** on what CPU speed is configured with the PLL.
	*/

	/*
	** Set watchdog counter value to timeout 250 ms
	*/
	LPC_WDT->TC = (unsigned int) (((unsigned int) 12000000/4)/1000) * (unsigned int) 250;

	/*
	** Set up watchdog clock "divide" by one (default state is gated off)
	*/
	LPC_SYSCON->WDTCLKDIV = 1;

	/*
	** Enable watchdog, and arm it to generate a reset upon timeout
	*/
	LPC_WDT->MOD = 0x3;

	/*
	** Send watchdog "feed" sequence- final step to begin the timeout
	*/
	LPC_WDT->FEED = 0xAA;
	LPC_WDT->FEED = 0x55;
}

void WatchdogFeed(void)
{
	LPC_WDT->FEED = 0xAA;
	LPC_WDT->FEED = 0x55;
}

unsigned char ReadInputMode(void)
{
	unsigned short analogValue;
	analogValue = ADCRead(5);

	if(analogValue >= 572)
	{
		if(analogValue >= 715)
		{
			if(analogValue >= 795)
			{
				if(analogValue >= 922)return 0;
				else return 1;
			}
			else
			{
				if(analogValue > 757)return 2;
				else return 3;
			}
		}
		else
		{
			if(analogValue >= 646)
			{
				if(analogValue >= 668)return 4;
				else return 5;
			}
			else
			{
				if(analogValue >=638)return 6;
				else return 7;
			}
		}
	}
	else
	{
		if(analogValue >= 417)
		{
			if(analogValue >= 443)
			{
				if(analogValue >= 480)return 8;
				else return 9;
			}
			else
			{
				if(analogValue >= 431)return 10;
				else return 11;
			}
		}
		else
		{
			if(analogValue >= 393)
			{
				if(analogValue >= 401)return 12;
				else return 13;
			}
			else
			{
				if(analogValue >= 390)return 14;
				else return 15;
			}

		}
	}
}
