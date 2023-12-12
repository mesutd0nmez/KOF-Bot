#include "pch.h"

#ifdef _WIN64

#include "Injection.h"
#pragma comment (lib, "Psapi.lib")

BYTE LoadLibrary_Shell_WOW64[] = 
{ 
	0x55, 0x8B, 0xEC, 0x56, 0x8B, 0x75, 0x08, 0x85, 0xF6, 0x74, 0x1F, 0x8B, 0x4E, 0x04, 0x85, 0xC9, 0x74, 0x18, 0x6A, 0x00, 0x6A, 0x00, 0x8D, 0x46, 0x08, 0x50, 0xFF, 0xD1, 0x89, 0x06, 0xC7, 0x46, 0x04, 
	0x00, 0x00, 0x00, 0x00, 0x5E, 0x5D, 0xC2, 0x04, 0x00, 0x33, 0xC0, 0x5E, 0x5D, 0xC2, 0x04, 0x00
};


DWORD _LoadLibrary_WOW64(const wchar_t * szDllFile, HANDLE hTargetProc, LAUNCH_METHOD Method, DWORD Flags, HINSTANCE & hOut, DWORD & LastError)
{
	LOAD_LIBRARY_EXW_DATA_WOW64 data{ 0 };
	StringCchCopyW(data.szDll, sizeof(data.szDll) / sizeof(wchar_t), szDllFile);

	void * pLoadLibraryExW = nullptr;
	char sz_LoadLibName[] = "LoadLibraryExW";

	if (!GetProcAddressEx_WOW64(hTargetProc, TEXT("kernel32.dll"), sz_LoadLibName, pLoadLibraryExW))
	{
		LastError = GetLastError();
		return INJ_ERR_REMOTEFUNC_MISSING;
	}

	data.pLoadLibraryExW = (DWORD)(ULONG_PTR)pLoadLibraryExW;

	BYTE * pArg = ReCa<BYTE*>(VirtualAllocEx(hTargetProc, nullptr, sizeof(LOAD_LIBRARY_EXW_DATA_WOW64) + sizeof(LoadLibrary_Shell_WOW64) + 0x10, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE));
	if (!pArg)
	{
		LastError = GetLastError();

		return INJ_ERR_OUT_OF_MEMORY_EXT;
	}

	if (!WriteProcessMemory(hTargetProc, pArg, &data, sizeof(LOAD_LIBRARY_EXW_DATA_WOW64), nullptr))
	{
		LastError = GetLastError();

		VirtualFreeEx(hTargetProc, pArg, 0, MEM_RELEASE);

		return INJ_ERR_WPM_FAIL;
	}

	auto * pShell = ReCa<BYTE*>(ALIGN_UP(ReCa<ULONG_PTR>(pArg + sizeof(LOAD_LIBRARY_EXW_DATA_WOW64)), 0x10));
	if (!WriteProcessMemory(hTargetProc, pShell, LoadLibrary_Shell_WOW64, sizeof(LoadLibrary_Shell_WOW64), nullptr))
	{
		LastError = GetLastError();

		VirtualFreeEx(hTargetProc, pArg, 0, MEM_RELEASE);

		return INJ_ERR_WPM_FAIL;
	}

	DWORD remote_ret = 0;
	DWORD dwRet = StartRoutine_WOW64(hTargetProc, MDWD(pShell), MDWD(pArg), Method, (Flags & INJ_THREAD_CREATE_CLOAKED) != 0, LastError, remote_ret);
	hOut = (HINSTANCE)(ULONG_PTR)remote_ret;

	if(Method != LM_QueueUserAPC)
	{
		VirtualFreeEx(hTargetProc, pArg, 0, MEM_RELEASE);
	}

	return dwRet;
}

#endif