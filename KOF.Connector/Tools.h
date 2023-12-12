#pragma once

#include "NT Stuff.h"
#include "Error.h"

//Filenames and errorlog setup

#define GH_INJ_VERSION L"3.3"

#define GH_INJ_MOD_NAME64W L"GH Injector - x64.dll"
#define GH_INJ_MOD_NAME86W L"GH Injector - x86.dll"

#define GH_INJ_MOD_NAME64A "GH Injector - x64.dll"
#define GH_INJ_MOD_NAME86A "GH Injector - x86.dll"

#ifdef _WIN64
#define GH_INJ_MOD_NAMEW GH_INJ_MOD_NAME64W
#define GH_INJ_MOD_NAMEA GH_INJ_MOD_NAME64A
#else
#define GH_INJ_MOD_NAMEW GH_INJ_MOD_NAME86W
#define GH_INJ_MOD_NAMEA GH_INJ_MOD_NAME86A
#endif

#ifdef UNICODE
#define GH_INJ_MOD_NAME GH_INJ_MOD_NAMEW
#else
#define GH_INJ_MOD_NAME GH_INJ_MOD_NAMEA
#endif

//reinterpret_cast = too long to type
#define ReCa reinterpret_cast

#define MPTR(d) (void*)(ULONG_PTR)d
//Macro to convert 32-bit DWORD into void*.

#define MDWD(p) (DWORD)((ULONG_PTR)p & 0xFFFFFFFF)
//Macro to convert void* into 32-bit DWORD.

//Global variable to store the base address of the current image of the injector. Initialized in DllMain.
extern HINSTANCE g_hInjMod;

UINT __forceinline _strlenA(const char * szString)
{
	UINT Ret = 0;
	if (!szString) return Ret;
	for (; *szString++; Ret++);
	return Ret;
}
//Simple function to determine the length of a string. It gets mapped into memory which is why __forceinline is required.
//
//Arguments:
//		szString (const char*)
///			A pointer to an ansi string.
//
//Returnvalue (UINT):
///		The amount of characters in the string

void __forceinline _ZeroMemory(BYTE * pData, UINT Len)
{
	while (Len--)
		*pData++ = 0;
}

void __forceinline _ZeroMemory2(BYTE * pData, UINT Len)
{
	for (UINT i = 0; i != Len; ++i)
	{
		pData[i] = 0;
	}
}
//Simple function to zero memoryg. It gets mapped into memory which is why __forceinline is required.
//
//Arguments:
//		pData (BYTE*):
///			Pointer to the data to zero.
//		Len (UINT):
///			Amount of bytes to overwrite.
//
//Returnvalue (void)

struct ERROR_INFO
	//A structure used to pass information to the error log function.
{
	const wchar_t *	szDllFileName;
	const wchar_t * szTargetProcessExeFileName;
	DWORD			TargetProcessId;
	DWORD			InjectionMode;
	DWORD			LaunchMethod;
	DWORD			Flags;
	DWORD			ErrorCode;
	DWORD			LastWin32Error;
	DWORD			HandleValue;
	bool			bNative;
};

bool FileExists(const wchar_t * szFile);
//A function to quickly check whether a file exists or not.
//
//Arguments:
//		szFile (const wchar_t*):
///			Pointer to a string containing the full path to the file.
//
//Returnvalue (bool):
///		true:	the file exists.
///		false:	the file doesn't exist.

DWORD ValidateFile(const wchar_t * szFile, DWORD desired_machine);
//A function used to verify whether the file fits the requirements of current injection settings.
//
//Arguments:
//		szFile (const wchar_t*):
///			Pointer to a string containing the full path to the file.
//		desired_machine (DWORD):
///			A value to be compared to the Machine member of the files IMAGE_FILE_HEADER	.
//
//Returnvalue (DWORD):
///		On success: 0.
///		On failure: an errocode from Error.h.

bool GetOwnModulePath(wchar_t * pOut, size_t BufferCchSize);
//A function to get the filepath to the file of this image of the injector.
//
//Arguments:
//		pOut (wchar_t*):
///			Pointer to a widechar buffer to contain the full path.
//		BufferCchSize (size_t):
///			Size of the buffer in characters.
//
//Returnvalue (bool):
///		true:	pOut now contains the path.
///		false:	error enumerating the modules.

bool IsNativeProcess(HANDLE hTargetProc);
//A function to determine whether a process runs natively or under WOW64.
//
//Arguments:
//		hTargetProc (HANDLE):
///			A handle to the desired process. This handle must have the PROCESS_QUERY_LIMITED_INFORMATION or PROCESS_QUERY_INFORMATION access right.
//
//Returnvalue (bool):
///		true: the specified process runs natively.
///		false: the specified process doesn't run natively.

ULONG GetSessionId(HANDLE hTargetProc, NTSTATUS & ntRetOut);
//A function to retrieve the session identifier of a process.
//
//Arguments:
//		hTargetproc (HANDLE):
///			A handle to the desired process. This handle must have the PROCESS_QUERY_LIMITED_INFORMATION or PROCESS_QUERY_INFORMATION access right.
//		ntRetOut (NTSTATUS &):
///			A reference to an NTSTATUS variable which will receive the returnvalue of NtQueryInformationProcess.
//
//Returnvalue (ULONG):
//		On success: The session identifier of the specified process.
//		On failuer: -1, check ntRetOut for more information.

bool IsElevatedProcess(HANDLE hTargetProc);
//A function used to determine whether a process is running elevated or not (administrator vs. user).
//
//Arguments:
//		hTargetproc (HANDLE):
///			A handle to the desired process. This handle must have the PROCESS_QUERY_INFORMATION access right.
//
//Returnvalue (bool):
///		true:	process is elevated.
///		false:	process is not elevated.

void ErrorLog(ERROR_INFO * info);
//A function used to generate an error log file in case shit hit the fan for some reason.
//
//Arguments:
//		info (ERROR_INFO*):
///			A pointer to an ERROR_INFO structure which contains information about what went wrong.
//
//Returnvalue (void)