#include "pch.h"

#include "Injection.h"
#pragma comment (lib, "Psapi.lib")

HINSTANCE __stdcall LoadLibraryExW_Shell(LOAD_LIBRARY_EXW_DATA * pData);
DWORD LoadLibraryExW_Shell_End();

DWORD _LoadLibraryExW(const wchar_t * szDllFile, HANDLE hTargetProc, LAUNCH_METHOD Method, DWORD Flags, HINSTANCE& hOut, DWORD& LastError)
{
	LOAD_LIBRARY_EXW_DATA data{ 0 };
	StringCchCopyW(data.szDll, sizeof(data.szDll) / sizeof(wchar_t), szDllFile);

	void * pLoadLibraryExW	= nullptr;
	char sz_LoadLibName[]	= "LoadLibraryExW";

	if (!GetProcAddressEx(hTargetProc, TEXT("kernel32.dll"), sz_LoadLibName, pLoadLibraryExW))
	{
		LastError = GetLastError();
		return INJ_ERR_REMOTEFUNC_MISSING;
	}

	data.pLoadLibraryExW = ReCa<f_LoadLibraryExW*>(pLoadLibraryExW);

	ULONG_PTR ShellSize = (ULONG_PTR)LoadLibraryExW_Shell_End - (ULONG_PTR)LoadLibraryExW_Shell;
	
	BYTE * pArg = ReCa<BYTE*>(VirtualAllocEx(hTargetProc, nullptr, sizeof(LOAD_LIBRARY_EXW_DATA) + ShellSize + 0x10, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE));
	if (!pArg)
	{
		LastError = GetLastError();

		return INJ_ERR_OUT_OF_MEMORY_EXT;
	}

	if (!WriteProcessMemory(hTargetProc, pArg, &data, sizeof(LOAD_LIBRARY_EXW_DATA), nullptr))
	{
		LastError = GetLastError();

		VirtualFreeEx(hTargetProc, pArg, 0, MEM_RELEASE);

		return INJ_ERR_WPM_FAIL;
	}

	auto * pShell = ReCa<BYTE*>(ALIGN_UP(ReCa<ULONG_PTR>(pArg + sizeof(LOAD_LIBRARY_EXW_DATA)), 0x10));
	if (!WriteProcessMemory(hTargetProc, pShell, LoadLibraryExW_Shell, ShellSize, nullptr))
	{
		LastError = GetLastError();

		VirtualFreeEx(hTargetProc, pArg, 0, MEM_RELEASE);

		return INJ_ERR_WPM_FAIL;
	}

	ULONG_PTR remote_ret = 0;
	DWORD dwRet = StartRoutine(hTargetProc, ReCa<f_Routine*>(pShell), pArg, Method, (Flags & INJ_THREAD_CREATE_CLOAKED) != 0, LastError, remote_ret);

	hOut = ReCa<HINSTANCE>(remote_ret);

	if (Method != LM_QueueUserAPC)
	{
		VirtualFreeEx(hTargetProc, pArg, 0, MEM_RELEASE);
	}

	return dwRet;
}

HINSTANCE __stdcall LoadLibraryExW_Shell(LOAD_LIBRARY_EXW_DATA * pData)
{
	if (!pData || !pData->pLoadLibraryExW)
		return NULL;

	pData->hRet = pData->pLoadLibraryExW(pData->szDll, nullptr, NULL);

	pData->pLoadLibraryExW = nullptr;

	return pData->hRet;
}

DWORD LoadLibraryExW_Shell_End() { return 0; }