#pragma once

#include <Windows.h>

#define INJ_ERASE_HEADER				0x0001
#define INJ_FAKE_HEADER					0x0002
#define INJ_UNLINK_FROM_PEB				0x0004
#define INJ_SHIFT_MODULE				0x0008
#define INJ_CLEAN_DATA_DIR				0x0010
#define INJ_THREAD_CREATE_CLOAKED		0x0020
#define INJ_SCRAMBLE_DLL_NAME			0x0040
#define INJ_LOAD_DLL_COPY				0x0080
#define INJ_HIJACK_HANDLE				0x0100
#define INJ_MAX_FLAGS					0x01FF

enum class INJECTION_MODE
{
	IM_LoadLibrary,
	IM_LdrLoadDll,
	IM_ManualMap
};

enum class LAUNCH_METHOD
{
	LM_NtCreateThreadEx,
	LM_HijackThread,
	LM_SetWindowsHookEx,
	LM_QueueUserAPC,
	LM_SetWindowLong
};

struct INJECTIONDATAA
{
	DWORD			LastErrorCode;								//used to store the error code of the injection 
	char			szDllPath[MAX_PATH * 2];					//fullpath to the dll to inject
	DWORD			ProcessID;									//process identifier of the target process
	INJECTION_MODE	Mode;										//injection mode
	LAUNCH_METHOD	Method;										//method to execute the remote shellcode
	DWORD			Flags;										//combination of the flags defined above
	DWORD			hHandleValue;								//optional value to identify a handle in a process
	HINSTANCE		hDllOut;									//returned image base of the injection
};

struct INJECTIONDATAW
{
	DWORD			LastErrorCode;								//used to store the error code of the injection 
	wchar_t			szDllPath[MAX_PATH * 2];					//fullpath to the dll to inject
	wchar_t*		szTargetProcessExeFileName;					//exe name of the target process, this value gets set automatically and should be ignored
	DWORD			ProcessID;									//process identifier of the target process
	INJECTION_MODE	Mode;										//injection mode
	LAUNCH_METHOD	Method;										//method to execute the remote shellcode
	DWORD			Flags;										//combination of the flags defined above
	DWORD			hHandleValue;								//optional value to identify a handle in a process
	HINSTANCE		hDllOut;									//returned image base of the injection
};

using f_InjectA = DWORD(__stdcall*)(INJECTIONDATAA* pData);
using f_InjectW = DWORD(__stdcall*)(INJECTIONDATAW* pData);