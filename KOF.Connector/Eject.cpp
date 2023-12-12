#include "pch.h"

#include "Eject.h"

void EjectDll(HANDLE hTargetProc, HINSTANCE hModBase)
{
	void * pFreeLibrary = nullptr;
	GetProcAddressEx(hTargetProc, TEXT("kernel32.dll"), "FreeLibrary", pFreeLibrary);

	if (!pFreeLibrary)
		return;

	DWORD win32		= 0;
	ULONG_PTR Out	= 0;
	StartRoutine(hTargetProc, ReCa<f_Routine*>(pFreeLibrary), ReCa<void*>(hModBase), LM_NtCreateThreadEx, true, win32, Out);
}