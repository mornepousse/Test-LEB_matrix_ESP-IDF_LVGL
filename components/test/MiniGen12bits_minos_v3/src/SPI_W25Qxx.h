#ifndef _W25Q_H_

#define _W25Q_H_


/*
The W25Q16BV array is organized into 8,192 programmable pages of 256-bytes each.
Up to 256 bytes can be programmed at a time.
Pages can be erased in groups of 16 (sector erase),
groups of 128 (32KB block erase),
groups of 256 (64KB block erase) or
the entire chip (chip erase).
The W25Q16BV has 512 erasable sectors and 32 erasable blocks respectively.
The small 4KB sectors allow for greater flexibility in applications
that require data and parameter storage.
A Hold pin, Write Protect pin and programmable write protection,
with top or bottom array control,  provide further control flexibility.
Additionally, the device supports JEDEC standard manufacturer and
device identification with a 64-bit Unique Serial Number.
10.1.1 Standard SPI Instructions
The W25Q16BV is accessed through an SPI compatible bus consisting of four signals:
Serial Clock (CLK), Chip Select (/CS), Serial Data Input (DI) and Serial Data Output (DO).
Standard SPI instructions use the DI input pin to serially
write instructions, addresses or data to the device on the rising edge of CLK.
The DO output pin is used to read data or status from the device on the falling edge CLK.
SPI bus operation Modes 0 (0,0) and 3 (1,1) are supported.
The primary difference between Mode 0 and Mode 3 concerns the normal state of the CLK signal
when the SPI bus master is in standby and data is not being transferred to the Serial Flash.
For Mode 0 the CLK signal is normally low on the falling and rising edges of /CS.
For Mode 3 the CLK signal is normally high on the falling and rising edges of /CS.

  STATUS REGISTER  SUS R R R R R QE SRP1 -- SRP0 SEC TB BP2 BP1 BP0 WEL BUSY
                   Ro               ***********  *****************  *    *
  BUSY : VOLATILE
  WEL   : VOLATILE
  OTHER : NON-VOLATILE

11.1.1 BUSY
BUSY is a read only bit in the status register (S0) that is set to a 1 state
when the device is executing a Page Program,
Sector Erase, Block Erase, Chip Erase or
Write Status Register instruction.
During this time the device will ignore further instructions
except for the Read Status Register and Erase Suspend instruction
(see t W , t PP , t SE , t BE , and t CE in AC Characteristics).
When the program, erase or write status register instruction has completed,
the BUSY bit will be cleared to a 0 state
indicating the device is ready for further instructions.

11.1.2 Write Enable Latch (WEL)
Write Enable Latch (WEL) is a read only bit in the status register (S1) that is set to a 1
after executing a Write Enable Instruction.
The WEL status bit is cleared to a 0 when the device is write disabled.
A write disable state occurs upon power-up or after any of the following instructions:
Write Disable, Page Program, Sector Erase, Block Erase, Chip Erase and Write Status Register.

11.1.3 Block Protect Bits (BP2, BP1, BP0)
The Block Protect Bits (BP2, BP1, BP0) are non-volatile read/write bits in the status register (S4, S3, and S2)
that provide Write Protection control and status.
Block Protect bits can be set using the Write Status Register Instruction (see t W in AC characteristics).
All, none or a portion of the memory array can be protected from Program and Erase instructions
(see Status Register Memory Protection table).
The factory default setting for the Block Protection Bits is 0, none of the array protected.

11.1.4 Top/Bottom Block Protect (TB)
The non-volatile Top/Bottom bit (TB) controls if the Block Protect Bits (BP2, BP1, BP0) protect
from the Top (TB=0) or the Bottom (TB=1) of the array as shown in the Status Register Memory Protection table.
The factory default setting is TB=0.
The TB bit can be set with the Write Status Register Instruction depending on the state of the SRP0, SRP1 and WEL bits.

11.1.5 Sector/Block Protect (SEC)
The non-volatile Sector protect bit (SEC) controls if the Block Protect Bits (BP2, BP1, BP0)
protect 4KB Sectors (SEC=1) or 64KB Blocks (SEC=0) in the Top (TB=0) or the Bottom (TB=1) of the array
as shown in the Status Register Memory Protection table. The default setting is SEC=0.

11.1.6 Status Register Protect (SRP1, SRP0)
The Status Register Protect bits (SRP1 and SRP0) are non-volatile read/write bits in the status register  (S8 and S7).
The SRP bits control the method of write protection:
software protection, hardware protection, power supply lock-down or one time programmable (OTP) protection.

11.1.7 Erase Suspend Status (SUS)
The Suspend Status bit is a read only bit in the status register (S15) that is set to 1
after executing an Erase Suspend (75h) instruction.
The SUS status bit is cleared to 0 by Erase Resume (7Ah) instruction as well as a power-down, power-up cycle.

11.1.8 Quad Enable (QE)
The Quad Enable (QE) bit is a non-volatile read/write bit in the status register (S9)
that allows Quad SPI operation.
When the QE bit is set to a 0 state (factory default), the /WP pin and /HOLD are enabled.
When the QE bit is set to a 1, the Quad IO2 and IO3 pins are enabled, and /WP and /HOLD functions are disabled.
WARNING: If the /WP or /HOLD pins are tied directly to the power supply or ground
during standard SPI or Dual SPI operation, the QE bit should never be set to a 1.

SRP1 SRP0 /WP Status Register Description
0     0   X Software Protection           /WP pin has no control. The Status register can be written to after a Write Enable instruction, WEL=1. [Factory Default]
0     1   0   Hardware Protected            When /WP pin is low the Status Register locked and can not be written to.
0     1   1   Hardware Unprotected          When /WP pin is high the Status register is unlocked and can be written to after a Write Enable instruction, WEL=1.
1     0   X   Power Supply Lock-Down (1)    Status Register is protected and can not be written to again until the next power-down, power-up cycle. (2)
1     1   X   One Time Program (1)          Status Register is permanently protected and can not be written to.

Note:
1. These features are available upon special order. Please refer to Ordering Information.
2. When SRP1, SRP0 = (1, 0), a power-down, power-up cycle will change SRP1, SRP0 to (0, 0) state.

BP2 BP1 BP0 = 000 : protect none
sec = 0           : from 1/32 to 1/2 for Upper ( TB=0 )
sec = 0           : from 1/32 to 1/1 for Lower ( TB=1 )
sec = 1           : from 4K to 32K for Upper ( TB=0 )
sec = 0           : from 4K to 32K for Lower ( TB=1 )

Manufacturer Identification
Winbond Serial Flash : 0xEF
Device Identification : 0xAB/0x90 : 0x14
Device Identification : 0x9F      : 0x4015
*/
#define ID_W25Q16 0x14
#define ID_W25Q32 0x15

void SpiMemoryInit(void);
unsigned char SpiMemoryReadWriteByte(unsigned char value);

void SpiMemoryCsHigh(void);
void SpiMemoryCsLow(void);
void W25Q_WaitMemoryReady(void);

#define W25Q16_FIRST_BYTE_PROG_TIME 50L       /* unit : micro second */
#define W25Q16_NEXT_BYTE_PROG_TIME  12L       /* unit : micro second */
#define W25Q16_PAGE_PROG_TIME       3000L     /* unit : micro second */
#define W25Q16_SECTOR_ERASE_TIME    200000L   /* unit : micro second */
#define W25Q16_32K_BLOCK_ERASE_TIME 800000L   /* unit : micro second */
#define W25Q16_64K_BLOCK_ERASE_TIME 1000000L  /* unit : micro second */
#define W25Q16_CHIP_ERASE_TIME      10000000L /* unit : micro second */

void W25Q_WriteDisable(void);
void W25Q_WriteEnable(void);

unsigned char W25Q_ReadDeviceId(void);
unsigned int W25Q_ReadJEDEDId(void); // 00--Manf--Type--Capacity
unsigned short W25Q_ReadManfDeviceId(void); // Manf -- Device
void W25Q_ReadUniqueId(unsigned char * UniqueId); // DO :: 63..0 :: [0..7]
void W25Q_ReadUniqueIdString(char *UniqueIdString);

unsigned char W25Q_ReadStatusRegisterHigh(void);
unsigned char W25Q_ReadStatusRegisterLow(void);
void W25Q_WriteStatusRegisterLow(unsigned char StatusRegisterLow);
void W25Q_WriteStatusRegister(unsigned char StatusRegisterLow, unsigned char StatusRegisterHigh);

void W25Q_ChipErase(void);
void W25Q_SectorErase(unsigned int address);
void W25Q_BlockErase32K(unsigned int address);
void W25Q_BlockErase64K(unsigned int address);

void W25Q_PageProgram(unsigned int address, unsigned int count, unsigned char * buffer);

void W25Q_Read(unsigned int address, unsigned int count, unsigned char * buffer);
void W25Q_FastRead(unsigned int address, unsigned int count, unsigned char * buffer);

#endif /* _W25Q_H_ */
