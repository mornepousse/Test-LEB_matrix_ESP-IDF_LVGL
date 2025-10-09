/*
 * MemoryFileSystem.c
 *
 *  Created on: 1 mars 2013
 *      Author: user
 */
#include "SPI_W25Qxx.h"
#include "MemoryFileSystem.h"
#include "LPC11xx.h"

unsigned int currentEntryAddress = 0;
unsigned int currentFileAddress = 0;
unsigned int currentFileSize = 0;
unsigned int currentFileOffset = 0;

FileEntryStruct fileEntryParams;

int MemoryFormat(DateStruct *firstDate)
{
	unsigned char buffer[32] = {0x55, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 'L', 'e', 'm', 'i', 'a', ' ', ' ', ' ', 'B', 'D', 'S', 'G', '1', ' ', '0', '9','5', '1','0', '5', '5', '0', '4', '6'};
	unsigned int i, startAddress;

	///Erase sector 0 which is the root directory
	W25Q_SectorErase(0);

	buffer[4] = firstDate->Day;
	buffer[5] = firstDate->Month;
	buffer[6] = firstDate->Year;
	buffer[7] = firstDate->Hour;
	W25Q_PageProgram(0, 32, buffer);

	startAddress = 0x1000;
	for(i = 1; i!= 2048; i++)
	{
		W25Q_Read(startAddress, 2, buffer);
		if(buffer[0] != 0xFF || buffer[1] != 0xFF)
		{
			LPC_WDT->FEED = 0xAA;
			LPC_WDT->FEED = 0x55;
			W25Q_SectorErase(startAddress);
		}
		startAddress+=0x1000;
	}
	return 0;
}

unsigned int MemoryGetNextFreeSector(int startAddress)
{
	unsigned char buffer[2];

	startAddress &= 0xFFFFF000;
	while(1)
	{
		W25Q_Read(startAddress, 2, buffer);
		if(buffer[0]==0xFF && buffer[1] == 0xFF)return startAddress;
		startAddress += 0x1000;
	}
}


int MemoryOpenFile(unsigned int entryNum)
{
	return 0;
}

int MemoryCreateFile(unsigned char type, unsigned char *name, DateStruct *creationDate)
{
	unsigned int newEntry = 0;
	unsigned int firstBlockAddress;
	unsigned char buffer[32];
	unsigned int i;

	// Search a new free entry for the file
	newEntry = MemoryGetNextEntry(MEM_FREE_ENTRY_SEARCH | MEM_ENTRY_FROM_ROOT, &fileEntryParams);
	// Search a new data free sector for the file
	firstBlockAddress = MemoryGetNextFreeSector(0);

	buffer[0] = 0x55;		//Signature
	buffer[1] = type;		//Signature
		buffer[2] = (unsigned char)(firstBlockAddress>>12);
	buffer[3] = (unsigned char)(firstBlockAddress>>20);
	buffer[4] = 0xFF;		//File size MSB will be updated when closing file
	buffer[5] = 0xFF;
	buffer[6] = 0xFF;
	buffer[7] = 0xFF;

	i=8;
	while(*name != 0)
	{
		buffer[i++] = (unsigned char)*name;
		name++;
	}
	while(i!= 24)buffer[i++] = 0x00;
	while(i != 28)buffer[i++] = 0xFF;
	buffer[28] = creationDate->Day;		//Date day
	buffer[29] = creationDate->Month;		//Date month
	buffer[30] = creationDate->Year;		//Date year
	buffer[31] = creationDate->Hour;		//Date hour
	W25Q_PageProgram(newEntry, 32, buffer);		//Program the new entry
	W25Q_PageProgram(firstBlockAddress, 2, buffer);	//Allocate the first data block

	currentEntryAddress = newEntry;
	currentFileAddress = firstBlockAddress + 4;
	currentFileSize = 0;

	return 0;
}

unsigned int MemoryReadFile(unsigned char *buffer, unsigned int length)
{
	unsigned int finalAddress, bufferIndex, readLength;
//	unsigned char tmpBuffer[2];

	if(currentFileOffset >= currentFileSize)return 0;
	finalAddress = currentFileOffset + length;
	if(finalAddress > currentFileSize)length = currentFileSize - currentFileOffset;

	bufferIndex = 0;
	while(length != 0)
	{
		finalAddress = currentFileAddress + length;
		if((finalAddress&0xFFFFF000) != (currentFileAddress&0xFFFFF000))
		{
			readLength = (currentFileAddress&0xFFFFF000)+0x1000-currentFileAddress;
			W25Q_Read(currentFileAddress, readLength, buffer);

			bufferIndex += readLength;
			buffer += readLength;
			length -= readLength;
			currentFileOffset += readLength;
			/// Get next sector
			currentFileAddress = GetNextSectorAddress(currentFileAddress);
			if(currentFileAddress == 0)
			{
				currentFileOffset = currentFileSize;
				return bufferIndex;
			}
			currentFileAddress += 4;
		}
		else
		{
			W25Q_Read(currentFileAddress, length, buffer);
			currentFileAddress += length;
			currentFileOffset += length;
			bufferIndex += length;
			length = 0;

		}
	}
	return bufferIndex;
}

unsigned int MemoryWriteFile(unsigned char *buffer, unsigned int length)
{
	unsigned int finalAddress = 0;
	unsigned int writeLength = 0;
	unsigned char tmpBuffer[2];

	while(length > 0)
	{
		finalAddress = currentFileAddress + length;

		if((finalAddress&0xFFFFFF00) == (currentFileAddress&0xFFFFF00))		/// Same page
		{
			W25Q_PageProgram(currentFileAddress, length, buffer);
			currentFileAddress +=length;
			currentFileSize += length;
			length = 0;
		}
		else
		{
			writeLength = (currentFileAddress&0xFFFFF00)+256-currentFileAddress;
			W25Q_PageProgram(currentFileAddress, writeLength, buffer);
			currentFileSize += writeLength;
			buffer+= writeLength;
			length -= writeLength;
			if(((currentFileAddress+writeLength)&0XFFFFF000) == (currentFileAddress&0xFFFFF000))///next page is in the same sector
			{
				currentFileAddress += writeLength;
			}
			else
			{
				finalAddress = MemoryGetNextFreeSector(currentFileAddress);
				tmpBuffer[0] = (unsigned char)(finalAddress>>12);
				tmpBuffer[1] = (unsigned char)(finalAddress>>20);
				W25Q_PageProgram((currentFileAddress&0xFFFFF000)+2, 2, tmpBuffer);	//Update the next sector address
				tmpBuffer[0] = 0x55;
				tmpBuffer[1] = 0x21;
				W25Q_PageProgram(finalAddress, 2, tmpBuffer);	//Allocate new sector
				currentFileAddress = finalAddress + 4;
			}
		}

	}
	return 1;
}

unsigned int MemoryCloseFile(void)
{
	unsigned char tmpBuffer[4];
	unsigned int *intPtr;

	intPtr = (unsigned int*)tmpBuffer;
	*intPtr = currentFileSize;
	//tmpBuffer[0] = (unsigned char)(currentFileSize>>24);
	//tmpBuffer[1] = (unsigned char)(currentFileSize>>16);
	//tmpBuffer[2] = (unsigned char)(currentFileSize>>8);
	//tmpBuffer[3] = (unsigned char)(currentFileSize);

	W25Q_PageProgram(currentEntryAddress+4, 4, tmpBuffer);	//Update file size

	return 1;
}


unsigned int MemoryGetNextEntry(unsigned char params, FileEntryStruct *entryStruct)
{
	unsigned char buffer[32];
	unsigned int newSectorAddress;
	unsigned int i;

	if((params&0x01) != 0)
	{
		currentEntryAddress = 0;
	}
	while(1)
	{
		if((currentEntryAddress & 0x00000FFF) == 0xFE0)
		{
			//Get next sector
			currentEntryAddress = GetNextSectorAddress(currentEntryAddress);
			if(currentEntryAddress == 0)
			{
				if((params&0x02)!=0)
				{
					//allocate new bloc
					newSectorAddress = MemoryGetNextFreeSector(0);

					buffer[0] = (unsigned char)(newSectorAddress>>12);
					buffer[1] = (unsigned char)(newSectorAddress>>20);
					W25Q_PageProgram((currentEntryAddress&0xFFFFF000)+2, 2, buffer);
					currentEntryAddress = newSectorAddress;

					buffer[0] = 0x55;
					buffer[1] = 0x01;
					W25Q_PageProgram(currentEntryAddress, 2, buffer);

				}
				else
				{
					currentEntryAddress = 32;
					entryStruct->Type = 0;
					return 0; //no entry found
				}
			}
		}
		currentEntryAddress += 32;

		W25Q_Read(currentEntryAddress, 32, buffer);
		if((params&0x02)!=0)
		{
			if(buffer[0] == 0xFF) //free entry found
			{
				entryStruct->Type = 0xFF;
				return currentEntryAddress;
			}
		}
		else
		{
			if(buffer[0] == 0xFF) //end of directory
			{
				entryStruct->Type = 0xFF;
				return 0;
			}
			if(buffer[0] == 0x55)// a valid entry is found
			{
				entryStruct->Type = buffer[1];
				for(i=0; i!=16; i++)
				{
					entryStruct->Name[i] = buffer[i+8];
				}
				entryStruct->Size = *((unsigned int*)(buffer+4));
				entryStruct->FileAddress = ((int)(*((unsigned short*)(buffer+2)))<<12)+4;
				entryStruct->Day = buffer[28];
				entryStruct->Month = buffer[29];
				entryStruct->Year = buffer[30];
				entryStruct->Hour = buffer[31];

				//entryStruct->Size = buffer[4]

				return currentEntryAddress;
			}
		}

	}


}

unsigned int GetNextSectorAddress(unsigned int address)
{
	unsigned char buffer[4];
	unsigned int nextSectorAddress;

	address &= 0xFFFFF000;

	W25Q_Read(address, 4, buffer);
	if(buffer[0] == 0x55)
	{
		nextSectorAddress = (((unsigned int)buffer[2])<<12) | (((unsigned int)buffer[3])<<20);

		if(nextSectorAddress >= 0xFFFF000)return 0;
		else return nextSectorAddress;
	}
	else return 0;


}

unsigned int MemoryDecodeAudioFile(unsigned char *input, unsigned int length, signed short *output, AudioContextStruct *audioContext)
{
	unsigned int inputIndex, outputIndex;
	unsigned short tmpSignedShort;
	signed int tmpSignedInt;
	unsigned int tmpUnsignedInt;

	//unsigned short

	outputIndex = 0;
	inputIndex = 0;

	if(audioContext->SampleBitsCount == 8)
	{
		while(length > 0)
		{
			tmpSignedInt = 0;
			tmpSignedInt += ((unsigned short)(input[outputIndex])<<8);
			tmpSignedInt -= 0x8000;
			output[outputIndex] = (signed short)tmpSignedInt;	//tmpSignedShort&0x00FF;
			outputIndex++;
			length--;
		}
		return outputIndex;
	}
	else if(audioContext->SampleBitsCount == 12)		//12 bits
	{
		while(length > 0)
		{
			switch(audioContext->StateIndex)
			{
			case 0:
				audioContext->LsbBitsBackup = input[inputIndex];
				break;
			case 1:
				tmpSignedShort = (unsigned short)(input[inputIndex])<<8;
				tmpSignedShort |= ((audioContext->LsbBitsBackup&0x0F)<<4);
				output[outputIndex++] = (signed short)(tmpSignedShort);
				break;
			case 2:
				tmpSignedShort = (unsigned short)(input[inputIndex])<<8;
				tmpSignedShort |= (audioContext->LsbBitsBackup&0xF0);
				output[outputIndex++] = (signed short)(tmpSignedShort);
				break;
			}
				/*	switch(audioContext->StateIndex)
					{
					case 0:
						audioContext->LsbBitsBackup = input[inputIndex];
						break;
					case 1:
						tmpUnsignedInt = (unsigned int)(input[inputIndex])<<8;
						tmpUnsignedInt |= ((audioContext->LsbBitsBackup&0x0F)<<4);
						output[outputIndex++] = (signed short)(tmpUnsignedInt);

						//output[outputIndex++] = (signed short)(tmpSignedShort);
						break;
					case 2:
						tmpUnsignedInt = (unsigned int)(input[inputIndex])<<8;
						tmpUnsignedInt |= ((audioContext->LsbBitsBackup&0xF0));
						output[outputIndex++] = (signed short)(tmpUnsignedInt);



						//tmpSignedShort = (unsigned short)(input[inputIndex])<<8;
						//tmpSignedShort |= (audioContext->LsbBitsBackup&0xF0);
						//output[outputIndex++] = (signed short)(tmpSignedShort);
						break;
					}*/

			inputIndex++;
			audioContext->StateIndex++;
			length--;
			if(audioContext->StateIndex >2)audioContext->StateIndex=0;
		}
		return outputIndex;
	}
	return 0;
}

void MemoryReadFirstDate(DateStruct *firstDate)
{
	unsigned char buffer[4];

	W25Q_Read(4, 4, buffer);
	firstDate->Day = buffer[0];
	firstDate->Month = buffer[1];
	firstDate->Year = buffer[2];
	firstDate->Hour = buffer[3];
	firstDate->Min = 0;
	firstDate->Sec = 0;
}
