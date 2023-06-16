#pragma once

#include <winternl.h>

#define STATUS_SUCCESS 0x00000000

class Remap
{
public:
	typedef NTSTATUS(WINAPI* _ZwCreateSection) (
		PHANDLE            SectionHandle,
		ACCESS_MASK        DesiredAccess,
		POBJECT_ATTRIBUTES ObjectAttributes,
		PLARGE_INTEGER     MaximumSize,
		ULONG              SectionPageProtection,
		ULONG              AllocationAttributes,
		HANDLE             FileHandle
		);

	typedef NTSTATUS(WINAPI* _ZwUnmapViewOfSection) (
		HANDLE ProcessHandle,
		PVOID  BaseAddress
		);

	typedef enum _SECTION_INHERIT
	{
		ViewShare = 1,
		ViewUnmap = 2

	} SECTION_INHERIT;

	typedef NTSTATUS(WINAPI* _ZwMapViewOfSection) (
		HANDLE          SectionHandle,
		HANDLE          ProcessHandle,
		PVOID* BaseAddress,
		ULONG_PTR       ZeroBits,
		SIZE_T          CommitSize,
		PLARGE_INTEGER  SectionOffset,
		PSIZE_T         ViewSize,
		SECTION_INHERIT InheritDisposition,
		ULONG           AllocationType,
		ULONG           Win32Protect
		);

	inline static bool RemapViewOfSection(HANDLE ProcessHandle, PVOID BaseAddress, SIZE_T RegionSize, DWORD NewProtection, PVOID CopyBuffer) 
	{
		SIZE_T numberOfBytesRead = 0;

		HMODULE hNtDllModule = GetModuleHandle(skCryptDec("ntdll.dll"));

		if (hNtDllModule == NULL)
			return false;

		_ZwCreateSection _NtCreateSection = (_ZwCreateSection)(void*)GetProcAddress(hNtDllModule, skCryptDec("ZwCreateSection"));

		_ZwUnmapViewOfSection _NtUnmapViewOfSection = (_ZwUnmapViewOfSection)(void*)GetProcAddress(hNtDllModule, skCryptDec("ZwUnmapViewOfSection"));

		_ZwMapViewOfSection _NtMapViewOfSection = (_ZwMapViewOfSection)(void*)GetProcAddress(hNtDllModule, skCryptDec("ZwMapViewOfSection"));

		if (ReadProcessMemory(ProcessHandle, BaseAddress, CopyBuffer, RegionSize, &numberOfBytesRead) == FALSE) 
		{
			return false;
		}

		HANDLE hSection = NULL;
		LARGE_INTEGER sectionMaxSize = {};
		sectionMaxSize.QuadPart = RegionSize;

		NTSTATUS R = _NtCreateSection(&hSection, SECTION_ALL_ACCESS, NULL, &sectionMaxSize, PAGE_EXECUTE_READWRITE, SEC_COMMIT, NULL);

		if (R != STATUS_SUCCESS) 
		{
			return false;
		}

		NTSTATUS R2 = _NtUnmapViewOfSection(ProcessHandle, BaseAddress);

		if (R2 != STATUS_SUCCESS) 
		{
			return false;
		}

		PVOID viewBase = BaseAddress;
		LARGE_INTEGER sectionOffset = {};
		SIZE_T viewSize = 0;

		NTSTATUS R3 = _NtMapViewOfSection(hSection, ProcessHandle, &viewBase, 0, RegionSize, &sectionOffset, &viewSize, ViewUnmap, 0, NewProtection);

		if (R3 != STATUS_SUCCESS) 
		{
			return false;
		}

		SIZE_T numberOfBytesWritten2 = 0;

		if (WriteProcessMemory(ProcessHandle, viewBase, CopyBuffer, viewSize, &numberOfBytesWritten2) == FALSE) 
		{
			return false;
		}

		if (CloseHandle(hSection) == FALSE) 
		{
			return false;
		}

		return true;
	}

	inline static bool PatchSection(HANDLE hProcess, PVOID regionBase, SIZE_T regionSize, DWORD newProtection)
	{
		PVOID EmptyAlloc = VirtualAlloc(NULL, regionSize, MEM_COMMIT | MEM_RESERVE, newProtection);

		if (EmptyAlloc == NULL)
		{
			return false;
		}

		if (RemapViewOfSection(hProcess, regionBase, regionSize, newProtection, EmptyAlloc) == false)
		{
			return false;
		}

		if (VirtualFree(EmptyAlloc, NULL, MEM_RELEASE) == FALSE)
		{
			return false;
		}

		return true;
	}

	inline static PIMAGE_SECTION_HEADER GetSectionByName(const char* name)
	{
		uint32_t modulebase = (uint32_t)GetModuleHandleA(0);
		PIMAGE_NT_HEADERS nt = (PIMAGE_NT_HEADERS)(modulebase + ((PIMAGE_DOS_HEADER)modulebase)->e_lfanew);
		PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(nt);

		for (int i = 0; i < nt->FileHeader.NumberOfSections; ++i, ++section) 
		{
			if (!_stricmp((char*)section->Name, name))
				return section;
		}

		return nullptr;
	}
};

