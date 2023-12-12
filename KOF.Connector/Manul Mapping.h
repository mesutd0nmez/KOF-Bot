#pragma once

#include "Injection.h"

struct MANUAL_MAPPER
{
	BYTE * pRawData;

	BYTE * pLocalImageBase;
	IMAGE_DOS_HEADER		* pLocalDosHeader;
	IMAGE_NT_HEADERS		* pLocalNtHeaders;
	IMAGE_OPTIONAL_HEADER	* pLocalOptionalHeader;
	IMAGE_FILE_HEADER		* pLocalFileHeader;
	
	DWORD ImageSize;
	DWORD AllocationSize;
	DWORD ShiftOffset;

	BYTE * pAllocationBase;
	BYTE * pTargetImageBase;
	BYTE * pShellcode;
	BYTE * pManualMappingData;

	DWORD ShellSize;

	HANDLE hTargetProcess;
	DWORD Flags;

	DWORD AllocateMemory(DWORD & LastWin32Error);
	DWORD CopyData		(DWORD & LastWin32Error);
	DWORD RelocateImage	(DWORD & LastWin32Error);
	DWORD CopyImage		(DWORD & LastWin32Error);
};

#ifdef _WIN64

struct MANUAL_MAPPER_WOW64
{
	BYTE * pRawData;

	BYTE * pLocalImageBase;
	IMAGE_DOS_HEADER		* pLocalDosHeader;
	IMAGE_NT_HEADERS32		* pLocalNtHeaders;
	IMAGE_OPTIONAL_HEADER32	* pLocalOptionalHeader;
	IMAGE_FILE_HEADER		* pLocalFileHeader;

	DWORD ImageSize;
	DWORD AllocationSize;
	DWORD ShiftOffset;

	BYTE * pAllocationBase;
	BYTE * pTargetImageBase;
	BYTE * pShellcode;
	BYTE * pManualMappingData;

	DWORD ShellSize;

	HANDLE hTargetProcess;
	DWORD Flags;

	DWORD AllocateMemory(DWORD & LastWin32Error);
	DWORD CopyData		(DWORD & LastWin32Error);
	DWORD RelocateImage	(DWORD & LastWin32Error);
	DWORD CopyImage		(DWORD & LastWin32Error);
};

struct MANUAL_MAPPING_DATA_WOW64
{
	DWORD hRet;
	DWORD pLoadLibraryA;
	DWORD pGetProcAddress;
	DWORD pModuleBase;
	DWORD Flags;
};

#endif