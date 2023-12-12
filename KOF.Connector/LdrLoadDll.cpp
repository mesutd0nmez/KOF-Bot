#include "pch.h"

#include "Injection.h"
#pragma comment (lib, "Psapi.lib")


HINSTANCE __stdcall LdrLoadDll_Shell(LDR_LOAD_DLL_DATA * pData);
DWORD LdrLoadDll_Shell_End();

DWORD _LdrLoadDll(const wchar_t * szDllFile, HANDLE hTargetProc, LAUNCH_METHOD Method, DWORD Flags, HINSTANCE& hOut, DWORD& LastError)
{
	size_t size_out = 0;

	LDR_LOAD_DLL_DATA data{ 0 };
	data.pModuleFileName.MaxLength = sizeof(data.Data);
	StringCbLengthW(szDllFile, data.pModuleFileName.MaxLength, &size_out);
	StringCbCopyW(ReCa<wchar_t*>(data.Data), data.pModuleFileName.MaxLength, szDllFile);
	data.pModuleFileName.Length = (WORD)size_out;

	void * pLdrLoadDll = nullptr;
	if (!GetProcAddressEx(hTargetProc, TEXT("ntdll.dll"), "LdrLoadDll", pLdrLoadDll))
	{
		LastError = GetLastError();

		return INJ_ERR_LDRLOADDLL_MISSING;
	}
	data.pLdrLoadDll = ReCa<NTStuff::f_LdrLoadDll>(pLdrLoadDll);

	ULONG_PTR ShellSize	= (ULONG_PTR)LdrLoadDll_Shell_End - (ULONG_PTR)LdrLoadDll_Shell;
	BYTE * pAllocBase	= ReCa<BYTE*>(VirtualAllocEx(hTargetProc, nullptr, sizeof(LDR_LOAD_DLL_DATA) + ShellSize + 0x10, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE));
	BYTE * pArg			= pAllocBase;
	BYTE * pFunc		= ReCa<BYTE*>(ALIGN_UP(ReCa<ULONG_PTR>(pArg) + sizeof(LDR_LOAD_DLL_DATA), 0x10));

	if (!pArg)
	{
		LastError = GetLastError();

		return INJ_ERR_CANT_ALLOC_MEM;
	}

	if (!WriteProcessMemory(hTargetProc, pArg, &data, sizeof(LDR_LOAD_DLL_DATA), nullptr))
	{
		LastError = GetLastError();

		VirtualFreeEx(hTargetProc, pArg, 0, MEM_RELEASE);

		return INJ_ERR_WPM_FAIL;
	}

	if (!WriteProcessMemory(hTargetProc, pFunc, LdrLoadDll_Shell, ShellSize, nullptr))
	{
		LastError = GetLastError();

		VirtualFreeEx(hTargetProc, pArg, 0, MEM_RELEASE);

		return INJ_ERR_WPM_FAIL;
	}

	ULONG_PTR remote_ret = 0;
	DWORD dwRet = StartRoutine(hTargetProc, ReCa<f_Routine*>(pFunc), pArg, Method, (Flags & INJ_THREAD_CREATE_CLOAKED) != 0, LastError, remote_ret);
	ReadProcessMemory(hTargetProc, pArg, &data, sizeof(data), nullptr);

	hOut = ReCa<HINSTANCE>(remote_ret);

	if (Method != LM_QueueUserAPC)
	{
		VirtualFreeEx(hTargetProc, pArg, 0, MEM_RELEASE);
	}

	return dwRet;
}

HINSTANCE __stdcall LdrLoadDll_Shell(LDR_LOAD_DLL_DATA * pData)
{
	if (!pData || !pData->pLdrLoadDll)
		return NULL;

	pData->pModuleFileName.szBuffer = ReCa<wchar_t*>(pData->Data);
	pData->ntRet = pData->pLdrLoadDll(nullptr, 0, &pData->pModuleFileName, &pData->hRet);

	pData->pLdrLoadDll = nullptr;

	return ReCa<HINSTANCE>(pData->hRet);
}

DWORD LdrLoadDll_Shell_End() { return 1; }