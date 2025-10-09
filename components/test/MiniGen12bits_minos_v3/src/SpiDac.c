
#include "LPC11xx.h"
#include "SpiDac.h"

#define __JTAG_DISABLED

unsigned short posX, posY = 0;

#define ACCURACY_COUNT 6


#define RD_BUFA_WR_BUFB 			1
#define RD_BUFB_WR_BUFA				0
#define I2S_MAX_DUAL_BUFFER_SIZE	1024

/// Dual Buffer
signed short audioBufferA[I2S_MAX_DUAL_BUFFER_SIZE];
signed short audioBufferB[I2S_MAX_DUAL_BUFFER_SIZE];

/// Read pointer
signed short *readBuffer;
unsigned int readLength = 0;
unsigned int readIndex = 0;

/// Buffer mode
unsigned int currentBufferId = RD_BUFA_WR_BUFB;

/// Available Data ready in the buffers
unsigned int countBufferA = 0;
unsigned int countBufferB = 0;
unsigned int audioIndex = 0;


/******************************************************************************
** Function name:		TIMER16_0_IRQHandler
**
** Descriptions:		Timer/Counter 0 interrupt handler
**						executes each 10ms @ 60 MHz CPU Clock
**
** parameters:			None
** Returned value:		None
**
******************************************************************************/
void TIMER16_0_IRQHandler(void)
{
  if ( LPC_TMR16B0->IR & 0x1 )
  {
  LPC_TMR16B0->IR = 1;			/* clear interrupt flag */
  //timer16_0_counter++;


	if(readLength > 0)
	{
		SpiDacWriteValue(readBuffer[readIndex++]);
		readLength--;
	}
	else
	{
		if(currentBufferId == RD_BUFA_WR_BUFB)
		{
			if(countBufferB != 0)
			{
				readIndex = 0;
				currentBufferId = RD_BUFB_WR_BUFA;
				readLength = countBufferB;
				countBufferB = 0;
				readBuffer = audioBufferB;
				SpiDacWriteValue(readBuffer[readIndex]);
				readIndex++;
				readLength--;

			}
			else
			{
				SpiDacWriteValue(0x0080);
			}
		}
		else
		{
			if(countBufferA != 0)
			{
				readIndex = 0;
				currentBufferId = RD_BUFA_WR_BUFB;
				readLength = countBufferA;
				countBufferA = 0;
				readBuffer = audioBufferA;
				SpiDacWriteValue(readBuffer[readIndex]);
				readIndex++;
				readLength--;
			}
			else
			{
				SpiDacWriteValue(0x0080);
			}
		}
	}

  }

  return;
}


void InitAudioTimer(void)
{
	LPC_SYSCON->SYSAHBCLKCTRL |= (1<<7);
	LPC_TMR16B0->TCR |= (1<<1);		//reset timer module
	LPC_TMR16B0->TCR = 0;			//release reset

	LPC_TMR16B0->PR = 0;		//No prescaler every MCLK
	LPC_TMR16B0->MR0 = 1087;
	LPC_TMR16B0->MCR = 3; //Interup and reset on match 0
	LPC_TMR16B0->CTCR = 0; //acts as a timer
	LPC_TMR16B0->TCR |= 1;
	   /* Enable the TIMER0 Interrupt */
	    NVIC_EnableIRQ(TIMER_16_0_IRQn);


}


int AudioWriteDualBuffer(signed short *buf, int length)
{
	int index = 0;

	if(currentBufferId == RD_BUFA_WR_BUFB)
	{
		if(countBufferB != 0)return 0;
		for(index = 0; index < length; index++)
		{
			audioBufferB[index] = buf[index];
		}
		countBufferB = length;
	}
	else
	{
		if(countBufferA != 0)return 0;
		for(index = 0; index < length; index++)
		{
			audioBufferA[index] = buf[index];
		}
		countBufferA = length;
	}
	return length;
}

int AudioDualBufferReady(void)
{
	if(currentBufferId == RD_BUFA_WR_BUFB)
	{
		if(countBufferB == 0)return 1;
		else return 0;
	}
	else
	{
		if(countBufferA == 0)return 1;
		else return 0;
	}
}










void SpiDacInit(void)
{
	int i;
	unsigned char dummy;



    /* Set SSEL0 as GPIO, output high */
	LPC_IOCON->PIO0_7 = 0;
	LPC_GPIO0->DIR |= (1<<7);

    /* Set SSEL to high */
    SpiDacCsHigh();


    LPC_SYSCON->PRESETCTRL |= (0x1<<0);		//Disable reset
  	LPC_SYSCON->SYSAHBCLKCTRL |= (0x1<<11);	//Enable clock module
  	LPC_SYSCON->SSP0CLKDIV = 0x02;			/* Divided by 2 */
    //LPC_IOCON->PIO0_8           &= ~0x07;	/*  SSP I/O config */
   // LPC_IOCON->PIO0_8           |= 0x01;		/* SSP MISO */
    LPC_IOCON->PIO0_9           &= ~0x07;
    LPC_IOCON->PIO0_9           |= 0x01;		/* SSP MOSI */

  	LPC_IOCON->SCK_LOC = 0x02;
  	LPC_IOCON->PIO0_6 = 0x02;	/* P0.6 function 2 is SSP clock, need to
  								combined with IOCONSCKLOC register setting */

    /* Set DSS data to 8-bit, Frame format SPI, CPOL = 0, CPHA = 0, and SCR is 15 */
   	LPC_SSP0->CR0 = 0x0107;

     /* SSPCPSR clock prescale register, master mode, minimum divisor is 0x02 */
   	LPC_SSP0->CPSR = 0x2;

     for ( i = 0; i < 8; i++ )
     {
   	  dummy = LPC_SSP0->DR;		/* clear the RxFIFO */
     }

     /* Enable the SSP Interrupt */
   	//NVIC_EnableIRQ(SSP0_IRQn);


     /* Master mode */
   	LPC_SSP0->CR1 = (0x1<<1);

     /* Set SSPINMS registers to enable interrupts */
     /* enable all error related interrupts */
   	//LPC_SSP0->IMSC = (0x1<<0) | (0x1<<1);

}

void SpiDacWriteByte(unsigned char value)
{
    /* Put the data on the FIFO */
    LPC_SSP0->DR = value;
    /* Wait for sending to complete */
    //while (((LPC_SSP0->SR&(1<<4)) != 0) || ((LPC_SSP0->SR&(1<<2))==0)); //While busy or while rx empty
    /* Return the received value */
    //return (LPC_SSP0->DR);
}

void SpiDacWriteValue(signed short value)
{
	signed int dacValue;
	SpiDacCsLow();
	dacValue = value + 0x8000;

	SpiDacWriteByte((unsigned char)((dacValue>>10)&0x003F));
	SpiDacWriteByte((unsigned char)((dacValue>>2)&0x00FF));

	while (((LPC_SSP0->SR&(1<<4)) != 0) || ((LPC_SSP0->SR&(1<<2))==0)); //While busy or while rx empty
	SpiDacCsHigh();

}
