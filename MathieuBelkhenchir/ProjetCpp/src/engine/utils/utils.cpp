/** \file utils.cpp
	* Fonctions utilitaires 
	*
	* \author levieux guillaume
	* \date 2005
	*/

#include "utils.h"
#include "external/gl/glew.h"
#include "external/gl/freeglut.h"  

#define CHEMIN_LOG "log.txt"

/****************************************************************************/
/*GETFILESIZE/
/*****************************************************************************/
uint32 getFileSize(const char* name)
{
	#ifndef SEEK_END
	#define SEEK_END 2
	#endif
	
	FILE* File = NULL;
	fopen_s(&File,name, "rb");
	if(!File)
		return 0;

	fseek(File, 0, SEEK_END);
	uint32 eof_ftell = ftell(File);
	fclose(File);
	return eof_ftell;
}

uint16 fgetu16 (FILE * fe)
{
	if(!fe)
		return 0;

	uint16 res = 0;

	for(int i=0;i<2;i++)
	{
		if(feof(fe))
			return 0;
		
		res <<= 8;
		res |= fgetc(fe);
	}


	return res;
}

uint32 fgetu32 (FILE * fe)
{
	if(!fe)
		return 0;

	uint32 res = 0;
	
	for(int i=0;i<4;i++)
	{
		if(feof(fe))
			return 0;
		
		res <<= 8;
		res |= fgetc(fe);
	}


	return res;
}


bool isPowerOfTwo(uint32 value)
{
	uint16 count = 0;
	while(value)
	{
		if(value & 0x0001)
			count++;
		value>>=1;
	}

	if(count == 1)
		return true;

	return false;
}

unsigned long getMemoryUsage(void)
{
	PROCESS_MEMORY_COUNTERS pmc;
	if ( GetProcessMemoryInfo( GetCurrentProcess(), &pmc, sizeof(pmc)) )
	{
		return (unsigned long)  pmc.WorkingSetSize;
	}
	return 0;
}
