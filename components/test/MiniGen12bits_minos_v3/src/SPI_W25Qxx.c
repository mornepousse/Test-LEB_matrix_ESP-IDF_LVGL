#include "LPC11xx.h"

/* SSP Status register */
#define SSPSR_TFE       (0x1<<0)
#define SSPSR_TNF       (0x1<<1)
#define SSPSR_RNE       (0x1<<2)
#define SSPSR_RFF       (0x1<<3)
#define SSPSR_BSY       (0x1<<4)

void SpiMemoryInit(void)
{
	unsigned char i, dummy;

	LPC_SYSCON->PRESETCTRL |= (0x1<<2);		// Disable reset module
	LPC_SYSCON->SYSAHBCLKCTRL |= (1<<18);	// Enable clock module
	LPC_SYSCON->SSP1CLKDIV = 0x02;			/* Divided by 2 */
	LPC_IOCON->PIO2_2 &= ~0x07;	/*  SSP I/O config */
	LPC_IOCON->PIO2_2 |= 0x02;		/* SSP MISO */
	LPC_IOCON->PIO2_3 &= ~0x07;
	LPC_IOCON->PIO2_3 |= 0x02;		/* SSP MOSI */
	LPC_IOCON->PIO2_1 &= ~0x07;
	LPC_IOCON->PIO2_1 |= 0x02;		/* SSP CLK */

	/* Enable AHB clock to the GPIO domain. */
	LPC_SYSCON->SYSAHBCLKCTRL |= (1<<6);

	LPC_IOCON->PIO2_0 &= ~0x07;		/* SSP SSEL is a GPIO pin */
	/* port2, bit 0 is set to GPIO output and high */

	LPC_GPIO2->DIR |= (1<<0);
	LPC_GPIO2->DATA |= (1<<0);


	//////////////////////////////////////////////////////////////

	/* Set DSS data to 8-bit, Frame format SPI, CPOL = 0, CPHA = 0, and SCR is 15 */
	LPC_SSP1->CR0 = 0x0107;

	/* SSPCPSR clock prescale register, master mode, minimum divisor is 0x02 */
	LPC_SSP1->CPSR = 0x2;

	for ( i = 0; i < 8; i++ )
	{
	  dummy = LPC_SSP1->DR;		/* clear the RxFIFO */
	}

	/* Enable the SSP Interrupt */
	//NVIC_EnableIRQ(SSP1_IRQn);

	/* Device select as master, SSP Enabled */


	/* Master mode */
	LPC_SSP1->CR1 = (1<<1);

	/* Set SSPINMS registers to enable interrupts */
	/* enable all error related interrupts */
	//LPC_SSP1->IMSC = SSPIMSC_RORIM | SSPIMSC_RTIM;
}


unsigned char SpiMemoryReadWriteByte(unsigned char value)
{
	  while ( (LPC_SSP1->SR & (SSPSR_TNF|SSPSR_BSY)) != SSPSR_TNF );
	  LPC_SSP1->DR = value;
	  while ( LPC_SSP1->SR & SSPSR_BSY );
	  return LPC_SSP1->DR;

}

void SpiMemoryCsHigh(void)
{
	LPC_GPIO2->DATA |= (1<<0);
}
void SpiMemoryCsLow(void)
{
	LPC_GPIO2->DATA &= ~(1<<0);
}
void SpiMemoryChipSelect(unsigned char enable)
{
	if(enable != 0)LPC_GPIO2->DATA |= (1<<0);
	else LPC_GPIO2->DATA &= ~(1<<0);
}

//#include "w25q.h"

/*
Write Enable (06h)
The Write Enable instruction (Figure 4) sets the Write Enable Latch (WEL) bit
in the Status Register to a 1.

The WEL bit must be set prior to every Page Program, Sector Erase,
Block Erase, Chip Erase and Write Status Register instruction.

The Write Enable instruction is entered by driving /CS low,
shifting the instruction code ?06h? into the Data Input (DI) pin
on the rising edge of CLK, and then driving /CS high.

*/
void W25Q_WriteEnable(void)
{
  SpiMemoryChipSelect(0);
  SpiMemoryReadWriteByte( 0x06 );
  SpiMemoryChipSelect(1);
}

/*
Write Disable (04h)
The Write Disable instruction (Figure 5) resets the Write Enable Latch (WEL) bit
in the Status Register to a 0.

The Write Disable instruction is entered by driving /CS low,
shifting the instruction code ?04h? into the DI pin and then driving /CS high.

Note that the WEL bit is automatically reset after Power-up and
upon completion of the Write Status Register, Page Program, Sector Erase,
Block Erase and Chip Erase instructions.

*/

void W25Q_WriteDisable(void)
{
  SpiMemoryChipSelect(0);
  SpiMemoryReadWriteByte( 0x04 );
  SpiMemoryChipSelect(1);
}

/*

Read Status Register-1 (05h) and Read Status Register-2 (35h)

The Read Status Register instructions allow the 8-bit Status Registers to be read.
The instruction is entered by driving /CS low and
shifting the instruction code ?05h?  for Status Register-1
and ?35h? for Status Register-2 into the DI pin on the rising edge of CLK.

The status register bits are then shifted out on the DO pin
at the falling edge of CLK with most significant bit (MSB) first as shown in figure 6.

The Status Register bits are shown in figure 3a and 3b and
include the BUSY, WEL, BP2-BP0, TB, SEC, SRP0, SRP1, QE and SUS bits
(see description of the Status Register earlier in this datasheet).

The Read Status Register instruction may be used at any time,
even while a Program, Erase or Write Status Register cycle is in progress.

This allows the BUSY status bit to be checked to determine when the cycle is complete
and if the device can accept another instruction.

The Status Register can be read continuously, as shown in Figure 6.
The instruction is completed by driving /CS high.

*/


unsigned char W25Q_ReadStatusRegisterLow(void)
{
  SpiMemoryChipSelect(0);
  SpiMemoryReadWriteByte( 0x05 );
  unsigned char StatusRegisterLow = SpiMemoryReadWriteByte( 0xFF );
  SpiMemoryChipSelect(1);
  return StatusRegisterLow;
}

unsigned char W25Q_ReadStatusRegisterHigh(void)
{
  SpiMemoryChipSelect(0);
  SpiMemoryReadWriteByte( 0x35 );
  unsigned char StatusRegisterHigh = SpiMemoryReadWriteByte( 0xFF );
  SpiMemoryChipSelect(1);
  return StatusRegisterHigh;
}

void W25Q_WaitMemoryReady(void)
{
	while((W25Q_ReadStatusRegisterLow() & 0x01) != 0);
}

/*
Write Status Register (01h) :: 01 : S7..S0 [ : S15..S8 ]

The Write Status Register instruction allows the Status Register to be written.

A Write Enable instruction must previously have been executed for the device
to accept the Write Status Register Instruction (Status Register bit WEL must equal 1).

Once write enabled, the instruction is entered by driving /CS low,
sending the instruction code ?01h?, and then writing the status register data byte as illustrated in figure 7.

The Status Register bits are shown in figure 3 and described earlier in this datasheet.

Only non-volatile Status Register bits SRP0, SEC, TB, BP2, BP1, BP0 (bits 7, 5, 4, 3, 2 of Status Register-1)
and QE, SRP1(bits 9 and 8 of Status Register-2) can be written to.

All other Status Register bit locations are read-only and will not be affected by the Write Status Register instruction.

The /CS pin must be driven high after the eighth or sixteenth bit of data that is clocked in.

If this is not done the Write Status Register instruction will not be executed.

If /CS is driven high after the eighth clock (compatible with the 25X series)
the QE and SRP1 bits will be cleared to 0.
    ***********

After /CS is driven high, the self-timed Write Status Register cycle will commence
for a time duration of t W (See AC Characteristics).

While the Write Status Register cycle is in progress,
the Read Status Register instruction may still be accessed to check the status of the BUSY bit.

The BUSY bit is a 1 during the Write Status Register cycle and
a 0 when the cycle is finished and ready to accept other instructions again.

After the Write Register cycle has finished the Write Enable Latch (WEL) bit
in the Status Register will be cleared to 0.

The Write Status Register instruction allows the Block Protect bits
(SEC, TB, BP2, BP1 and BP0) to be set for protecting all, a portion, or none of the memory
from erase and program instructions.

Protected areas become read-only (see Status Register Memory Protection table and description).

The Write Status Register instruction also allows the Status Register Protect bits (SRP0, SRP1) to be set.
Those bits are used in conjunction with the Write Protect (/WP) pin, Lock out or OTP features
to disable writes to the status register.

Please refer to 11.1.6 for detailed descriptions regarding Status Register protection methods.
Factory default for all status Register bits are 0.

*/

void W25Q_WriteStatusRegisterLow(unsigned char StatusRegisterLow)
{
  W25Q_WriteEnable();
  SpiMemoryChipSelect(0);
  SpiMemoryReadWriteByte( 0x01 );
  SpiMemoryReadWriteByte( StatusRegisterLow );
  SpiMemoryChipSelect(1);
}

void W25Q_WriteStatusRegister(unsigned char StatusRegisterLow, unsigned char StatusRegisterHigh)
{
  W25Q_WriteEnable();
  SpiMemoryChipSelect(0);
  SpiMemoryReadWriteByte( 0x01 );
  SpiMemoryReadWriteByte( StatusRegisterLow );
  SpiMemoryReadWriteByte( StatusRegisterHigh );
  SpiMemoryChipSelect(1);
}

/*
Read Data (03h)
The Read Data instruction allows one more data bytes to be sequentially read from the memory.

The instruction is initiated by driving the /CS pin low and
then shifting the instruction code ?03h? followed by a 24-bit address (A23-A0) into the DI pin.

The code and address bits are latched on the rising edge of the CLK pin.
After the address is received, the data byte of the addressed memory location will be
shifted out on the DO pin at the falling edge of CLK with most significant bit (MSB) first.

The address is automatically incremented to the next higher address
after each byte of data is shifted out allowing for a continuous stream of data.

This means that the entire memory can be accessed
with a single instruction as long as the clock continues.

The instruction is completed by driving /CS high.

The Read Data instruction sequence is shown in figure 8.
If a Read Data instruction is issued while an Erase, Program or Write cycle is in process (BUSY=1)
the instruction is ignored and will not have any effects on the current cycle.

The Read Data instruction allows clock rates from D.C. to a maximum of f R (see AC Electrical Characteristics).

*/

void W25Q_Read(unsigned int address, unsigned int count, unsigned char * buffer)
{
  unsigned int i = 0;
  W25Q_WaitMemoryReady();
  SpiMemoryChipSelect(0);
  SpiMemoryReadWriteByte( 0x03 );
  SpiMemoryReadWriteByte( address >> 16 );
  SpiMemoryReadWriteByte( address >> 8 );
  SpiMemoryReadWriteByte( address >> 0 );

  for (i=0; i<count; i++)
    buffer[i] = SpiMemoryReadWriteByte( 0xFF );

  SpiMemoryChipSelect(1);
}

/*
Fast Read (0Bh)

The Fast Read instruction is similar to the Read Data instruction
except that it can operate at the highest possible frequency of F R (see AC Electrical Characteristics).

This is accomplished by adding eight ?dummy? clocks after the 24-bit address as shown in figure 9.

The dummy clocks allow the devices internal circuits additional time for setting up the initial address.

During the dummy clocks the data value on the DO pin is a ?don?t care?.

*/

void W25Q_FastRead(unsigned int address, unsigned int count, unsigned char * buffer)
{
  unsigned int i = 0;
  SpiMemoryChipSelect(0);
  SpiMemoryReadWriteByte( 0x03 );
  SpiMemoryReadWriteByte( address >> 16 );
  SpiMemoryReadWriteByte( address >> 8 );
  SpiMemoryReadWriteByte( address >> 0 );
  SpiMemoryReadWriteByte( 0xFF );

  for (i=0; i<count; i++)
    buffer[i] = SpiMemoryReadWriteByte( 0xFF );

  SpiMemoryChipSelect(1);
}

/*

Page Program (02h)

The Page Program instruction allows from one byte to 256 bytes (a page) of data
to be programmed at previously erased (FFh) memory locations.

A Write Enable instruction must be executed
before the device will accept the Page Program Instruction (Status Register bit WEL= 1).

The instruction is initiated by driving the /CS pin low
then shifting the instruction code ?02h? followed by
a 24-bit address (A23-A0) and
at least one data byte, into the DI pin.

The /CS pin must be held low for the entire length of the instruction
while data is being sent to the device.

The Page Program instruction sequence is shown in figure 16.

If an entire 256 byte page is to be programmed,
the last address byte (the 8 least significant address bits) should be set to 0.

If the last address byte is not zero, and the number of clocks exceed the remaining page length,
the addressing will wrap to the beginning of the page.
                    *********************************

In some cases, less than 256 bytes (a partial page) can be programmed
without having any effect on other bytes within the same page.

One condition to perform a partial page program is that
the number of clocks can not exceed the remaining page length.

If more than 256 bytes are sent to the device
the addressing will wrap to the beginning of the page and
                    *********************************
overwrite previously sent data.

As with the write and erase instructions, the /CS pin must be driven high
after the eighth bit of the last byte has been latched.

If this is not done the Page Program instruction will not be executed.
After /CS is driven high, the self-timed Page Program instruction
will commence for a time duration of tpp (See AC Characteristics).

While the Page Program cycle is in progress,
the Read Status Register instruction may still be accessed
for checking the status of the BUSY bit.

The BUSY bit is a 1 during the Page Program cycle and
becomes a 0 when the cycle is finished and the device is ready to accept other instructions again.

After the Page Program cycle has finished the Write Enable Latch (WEL) bit
in the Status Register is cleared to 0.

The Page Program instruction will not be executed
if the addressed page is protected by the Block Protect (BP2, BP1, and BP0) bits.

*/

void W25Q_PageProgram(unsigned int address, unsigned int count, unsigned char * buffer)
{
	unsigned int i;
	W25Q_WaitMemoryReady();
  W25Q_WriteEnable();
  SpiMemoryChipSelect(0);

  SpiMemoryReadWriteByte( 0x02 );
  SpiMemoryReadWriteByte( address >> 16 );
  SpiMemoryReadWriteByte( address >> 8 );
  SpiMemoryReadWriteByte( address >> 0 );

  for (i=0; i<count; i++)
     SpiMemoryReadWriteByte( buffer[i] );

  SpiMemoryChipSelect(1);
}
/*
Sector Erase (20h)

The Sector Erase instruction sets all memory within a specified sector (4K-bytes)
to the erased state of all 1s (FFh).

A Write Enable instruction must be executed before the device will
accept the Sector Erase Instruction (Status Register bit WEL must equal 1).

The instruction is initiated by driving the /CS pin low and
shifting the instruction code ?20h? followed a 24-bit sector address (A23-A0) (see Figure 2).

The Sector Erase instruction sequence is shown in figure 18.

The /CS pin must be driven high after the eighth bit of the last byte has been latched.
If this is not done the Sector Erase instruction will not be executed.

After /CS is driven high, the self-timed Sector Erase instruction will
commence for a time duration of t SE (See AC Characteristics).

While the Sector Erase cycle is in progress,
the Read Status Register instruction may still be accessed for
checking the status of the BUSY bit.

The BUSY bit is a 1 during the Sector Erase cycle and becomes
a 0 when the cycle is finished and the device is ready to accept other instructions again.

After the Sector Erase cycle has finished the Write Enable Latch (WEL) bit
in the Status Register is cleared to 0.

The Sector Erase instruction will not be executed
if the addressed page is protected by the Block Protect (SEC, TB, BP2, BP1, and BP0) bits
(see Status Register Memory Protection table).

*/

void W25Q_SectorErase(unsigned int address)
{
	  W25Q_WaitMemoryReady();
  W25Q_WriteEnable();
  SpiMemoryChipSelect(0);

  SpiMemoryReadWriteByte( 0x20 );
  SpiMemoryReadWriteByte( address >> 16 );
  SpiMemoryReadWriteByte( address >> 8 );
  SpiMemoryReadWriteByte( address >> 0 );

  SpiMemoryChipSelect(1);
}

/*
32KB Block Erase (52h)

The Block Erase instruction sets all memory within a specified block (32K-bytes)
to the erased state of all 1s (FFh).

A Write Enable instruction must be executed before the device will
accept the Block Erase Instruction (Status Register bit WEL must equal 1).

The instruction is initiated by driving the /CS pin low and
shifting the instruction code ?52h? followed a 24-bit block address (A23-A0) (see Figure 2).

The Block Erase instruction sequence is shown in figure 19.

The /CS pin must be driven high after the eighth bit of the last byte has been latched.
If this is not done the Block Erase instruction will not be executed.

After /CS is driven high, the self-timed Block Erase instruction will
commence for a time duration of t BE 1 (See AC Characteristics).

While the Block Erase cycle is in progress,
the Read Status Register instruction may still be accessed for
checking the status of the BUSY bit.

The BUSY bit is a 1 during the Block Erase cycle and becomes
a 0 when the cycle is finished and the device is ready to accept other instructions again.

After the Block Erase cycle has finished the Write Enable Latch (WEL) bit
in the Status Register is cleared to 0.

The Block Erase instruction will not be executed
if the addressed page is protected by the Block Protect (SEC, TB, BP2, BP1, and BP0) bits
(see Status Register Memory Protection table).

*/

void W25Q_BlockErase32K(unsigned int address)
{
	  W25Q_WaitMemoryReady();

	W25Q_WriteEnable();
  SpiMemoryChipSelect(0);

  SpiMemoryReadWriteByte( 0x52 );
  SpiMemoryReadWriteByte( address >> 16 );
  SpiMemoryReadWriteByte( address >> 8 );
  SpiMemoryReadWriteByte( address >> 0 );

  SpiMemoryChipSelect(1);
}
/*
64KB Block Erase (D8h)

The Block Erase instruction sets all memory within a specified block (64K-bytes)
to the erased state of all 1s (FFh).

A Write Enable instruction must be executed before the device will
accept the Block Erase Instruction (Status Register bit WEL must equal 1).

The instruction is initiated by driving the /CS pin low and
shifting the instruction code ?D8h? followed a 24-bit block address (A23-A0) (see Figure 2).

The Block Erase instruction sequence is shown in figure 20.

The /CS pin must be driven high after the eighth bit of the last byte has been latched.
If this is not done the Block Erase instruction will not be executed.

After /CS is driven high, the self-timed Block Erase instruction will
commence for a time duration of t BE 1 (See AC Characteristics).

While the Block Erase cycle is in progress,
the Read Status Register instruction may still be accessed for
checking the status of the BUSY bit.

The BUSY bit is a 1 during the Block Erase cycle and becomes
a 0 when the cycle is finished and the device is ready to accept other instructions again.

After the Block Erase cycle has finished the Write Enable Latch (WEL) bit
in the Status Register is cleared to 0.

The Block Erase instruction will not be executed
if the addressed page is protected by the Block Protect (SEC, TB, BP2, BP1, and BP0) bits
(see Status Register Memory Protection table).
*/

void W25Q_BlockErase64K(unsigned int address)
{
	  W25Q_WaitMemoryReady();

	W25Q_WriteEnable();
  SpiMemoryChipSelect(0);

  SpiMemoryReadWriteByte( 0xD8 );
  SpiMemoryReadWriteByte( address >> 16 );
  SpiMemoryReadWriteByte( address >> 8 );
  SpiMemoryReadWriteByte( address >> 0 );

  SpiMemoryChipSelect(1);
}


/*
Chip Erase (C7h / 60h)

The Chip Erase instruction sets all memory within the device
to the erased state of all 1s (FFh).

A Write Enable instruction must be executed before the device will
accept the Block Erase Instruction (Status Register bit WEL must equal 1).

The instruction is initiated by driving the /CS pin low and
shifting the instruction code ?C7h? or ?60h?.

The Chip Erase instruction sequence is shown in figure 21.

The /CS pin must be driven high after the eighth bit of the last byte has been latched.
If this is not done the Chip Erase instruction will not be executed.

After /CS is driven high, the self-timed Chip Erase instruction will
commence for a time duration of t BE 1 (See AC Characteristics).

While the Chip Erase cycle is in progress,
the Read Status Register instruction may still be accessed for
checking the status of the BUSY bit.

The BUSY bit is a 1 during the Chip Erase cycle and becomes
a 0 when the cycle is finished and the device is ready to accept other instructions again.

After the Block Erase cycle has finished the Write Enable Latch (WEL) bit
in the Status Register is cleared to 0.

The Block Erase instruction will not be executed
if the addressed page is protected by the Block Protect (SEC, TB, BP2, BP1, and BP0) bits
(see Status Register Memory Protection table).

*/
void W25Q_ChipErase(void)
{
	  W25Q_WaitMemoryReady();

  W25Q_WriteEnable();
  SpiMemoryChipSelect(0);
  SpiMemoryReadWriteByte( 0xC7 );
  SpiMemoryChipSelect(1);
}

/*
Device ID (ABh)

It can be used to obtain the devices electronic identification (ID) number.

the instruction is initiated by driving the /CS pin low and
shifting the instruction code ?ABh? followed by 3-dummy bytes.

The Device ID bits are then shifted out on the falling edge of CLK
with most significant bit (MSB) first as shown in figure 25b.

The Device ID values for the W25Q16BV is listed in Manufacturer and Device Identification table.
The Device ID can be read continuously.

The instruction is completed by driving /CS high.
*/

unsigned char W25Q_ReadDeviceId(void)
{
  SpiMemoryChipSelect(0);
  SpiMemoryReadWriteByte( 0xAB );
  SpiMemoryReadWriteByte( 0xFF );
  SpiMemoryReadWriteByte( 0xFF );
  SpiMemoryReadWriteByte( 0xFF );
  unsigned char DeviceId = SpiMemoryReadWriteByte( 0xFF );
  SpiMemoryChipSelect(1);
  return DeviceId;
}

/*
Read Manufacturer / Device ID (90h)

The Read Manufacturer/Device ID instruction is an alternative
to the Release from Power-down / Device ID instruction that
provides both the JEDEC assigned manufacturer ID and the specific device ID.

The Read Manufacturer/Device ID instruction is very similar
to the Release from Power-down / Device ID instruction.

The instruction is initiated by driving the /CS pin low and
shifting the instruction code ?90h? followed by a 24-bit address (A23-A0) of 000000h.

After which, the Manufacturer ID for Winbond (EFh) and the Device ID are
shifted out on the falling edge of CLK with most significant bit (MSB) first as shown in figure 26.

The Device ID values for the W25Q16BV is listed in Manufacturer and Device Identification table.

If the 24-bit address is initially set to 000001h the Device ID will be read first
and then followed by the Manufacturer ID.

The Manufacturer and Device IDs can be read continuously,
alternating from one to the other.

The instruction is completed by driving /CS high.
*/
unsigned short W25Q_ReadManfDeviceId(void) // Manf -- Device
{
  SpiMemoryChipSelect(0);
  SpiMemoryReadWriteByte( 0x90 );
  SpiMemoryReadWriteByte( 0x00 );  // Address = 0x000000 : ManfId
  SpiMemoryReadWriteByte( 0x00 );  // Address = 0x000001 : DeviceId
  SpiMemoryReadWriteByte( 0x00 );
  unsigned char ManfId = SpiMemoryReadWriteByte( 0xFF );
  unsigned char DeviceId = SpiMemoryReadWriteByte( 0xFF );
  SpiMemoryChipSelect(1);
  return ( (ManfId<<8) + DeviceId );
}
/*

Read Unique ID Number (4Bh)

The Read Unique ID Number instruction accesses a factory-set
read-only 64-bit number that is unique to each W25Q16BV device.

The ID number can be used in conjunction with user software methods
to help prevent copying or cloning of a system.

The Read Unique ID instruction is initiated by driving the /CS pin low and
shifting the instruction code ?4Bh? followed by a four bytes of dummy clocks.

After which, the 64-bit ID is shifted out on the falling edge of CLK as shown in figure 29.

*/

void W25Q_ReadUniqueId(unsigned char * UniqueId) // DO :: 63..0 :: [0..7]
{
  unsigned int i;
  SpiMemoryChipSelect(0);
  SpiMemoryReadWriteByte( 0x4B );
  SpiMemoryReadWriteByte( 0xFF );
  SpiMemoryReadWriteByte( 0xFF );
  SpiMemoryReadWriteByte( 0xFF );
  SpiMemoryReadWriteByte( 0xFF );

  for (i=0; i<8; i++)
    UniqueId[i] = SpiMemoryReadWriteByte( 0xFF );

  SpiMemoryChipSelect(1);
}
void W25Q_ReadUniqueIdString(char *UniqueIdString) // DO :: 63..0 :: [0..7]
{
  unsigned int i;
  unsigned char readByte, readByteMsb;
  SpiMemoryChipSelect(0);
  SpiMemoryReadWriteByte( 0x4B );
  SpiMemoryReadWriteByte( 0xFF );
  SpiMemoryReadWriteByte( 0xFF );
  SpiMemoryReadWriteByte( 0xFF );
  SpiMemoryReadWriteByte( 0xFF );

  for (i=0; i<16; i++)
  {
	  readByte = SpiMemoryReadWriteByte( 0xFF );
	  readByteMsb = readByte>>4;
	  if(readByteMsb>9)UniqueIdString[i++] = readByteMsb + 'A'-10;
	  else UniqueIdString[i++] = readByteMsb + '0';
	  readByte &= 0x0F;
	  if(readByte>9)UniqueIdString[i] = readByte + 'A'-10;
	  else UniqueIdString[i] = readByte + '0';

  }
  UniqueIdString[i] = 0;

  SpiMemoryChipSelect(1);
}

/*
Read JEDEC ID (9Fh)

For compatibility reasons, the W25Q16BV provides several instructions
to electronically determine the identity of the device.

The Read JEDEC ID instruction is compatible with the JEDEC standard
for SPI compatible serial memories that was adopted in 2003.

The instruction is initiated by driving the /CS pin low and
shifting the instruction code ?9Fh?.

The JEDEC assigned Manufacturer ID byte for Winbond (EFh)
and two Device ID bytes, Memory Type (ID15-ID8) and Capacity (ID7-ID0) are
then shifted out on the falling edge of CLK with most significant bit (MSB) first as shown in figure 30.

For memory type and capacity values refer to Manufacturer and Device Identification table.

*/

unsigned int W25Q_ReadJEDEDId(void) // 00--Manf--Type--Capacity
{
  SpiMemoryChipSelect(0);
  SpiMemoryReadWriteByte( 0x9F );
  unsigned char ManfId = SpiMemoryReadWriteByte( 0xFF );
  unsigned char Type = SpiMemoryReadWriteByte( 0xFF );
  unsigned char Capacity = SpiMemoryReadWriteByte( 0xFF );
  SpiMemoryChipSelect(1);
  return ( (ManfId<<16) + (Type<<8) + Capacity );
}

