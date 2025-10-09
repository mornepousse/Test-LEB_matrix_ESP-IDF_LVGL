/*
 * MemoryFileSystem.h
 *
 *  Created on: 2 mars 2013
 *      Author: user
 */

#ifndef MEMORYFILESYSTEM_H_
#define MEMORYFILESYSTEM_H_

typedef struct
{
	unsigned char Name[16];
	unsigned int Size;
	unsigned int FileAddress;
	unsigned char Type;
	unsigned char Day;
	unsigned char Month;
	unsigned char Year;
	unsigned char Hour;
} FileEntryStruct;

typedef struct
{
	unsigned char SampleBitsCount;
	unsigned char LsbBitsBackup;
	unsigned char StateIndex;
} AudioContextStruct;

typedef struct
{
	unsigned char Day;
	unsigned char Month;
	unsigned char Year;
	unsigned char Hour;
	unsigned char Min;
	unsigned char Sec;
}DateStruct;

extern unsigned int currentFileAddress;
extern unsigned int currentFileSize;
extern unsigned int currentFileOffset;

int MemoryFormat(DateStruct *firstDate);
unsigned int MemoryGetNextFreeSector(int startAddress);
unsigned int GetNextSectorAddress(unsigned int address);
unsigned int MemoryGetNextEntry(unsigned char params, FileEntryStruct *entryStruct);
unsigned int MemoryReadFile(unsigned char *buffer, unsigned int length);
unsigned int MemoryWriteFile(unsigned char *buffer, unsigned int length);
int MemoryCreateFile(unsigned char type, unsigned char *name, DateStruct *creationDate);
unsigned int MemoryCloseFile(void);
unsigned int MemoryDecodeAudioFile(unsigned char *input, unsigned int length, signed short *output, AudioContextStruct *audioContext);
void MemoryReadFirstDate(DateStruct *firstDate);

#define MEM_ENTRY_FROM_ROOT		0x01
#define MEM_FREE_ENTRY_SEARCH	0x02

#endif /* MEMORYFILESYSTEM_H_ */
