#include "pch.h"

#include "Injection.h"
#include <iostream>
#pragma comment (lib, "Psapi.lib")

DWORD InitErrorStruct(const wchar_t * szDllPath, INJECTIONDATAW * pData, bool bNative, DWORD RetVal);

DWORD InjectDLL(const wchar_t * szDllFile, HANDLE hTargetProc, INJECTION_MODE im, LAUNCH_METHOD Method, DWORD Flags, DWORD & LastError, HINSTANCE & hOut);

DWORD HijackHandle(INJECTIONDATAW * pData);

DWORD Cloaking(HANDLE hTargetProc, DWORD Flags, HINSTANCE hMod, DWORD & LastError);

DWORD InitErrorStruct(const wchar_t * szDllPath, INJECTIONDATAW * pData, bool bNative, DWORD RetVal)
{
	if (!RetVal)
		return INJ_ERR_SUCCESS;
	
	ERROR_INFO info{ 0 };
	info.szDllFileName				= szDllPath;
	info.szTargetProcessExeFileName = pData->szTargetProcessExeFileName;
	info.TargetProcessId			= pData->ProcessID;
	info.InjectionMode				= pData->Mode;
	info.LaunchMethod				= pData->Method;
	info.Flags						= pData->Flags;
	info.ErrorCode					= RetVal;
	info.LastWin32Error				= pData->LastErrorCode;
	info.HandleValue				= pData->hHandleValue;
	info.bNative					= bNative;

	ErrorLog(&info);

	return RetVal;
}

DWORD __stdcall InjectA(INJECTIONDATAA * pData)
{
#pragma EXPORT_FUNCTION(__FUNCTION__, __FUNCDNAME__)
	
	if (!pData->szDllPath)
		return InitErrorStruct(nullptr, ReCa<INJECTIONDATAW*>(pData), false, INJ_ERR_INVALID_FILEPATH);
	
	INJECTIONDATAW data{ 0 };
	size_t len_out = 0;
	size_t max_len = sizeof(data.szDllPath) / sizeof(wchar_t);
	StringCchLengthA(pData->szDllPath, max_len, &len_out);
	mbstowcs_s(&len_out, const_cast<wchar_t*>(data.szDllPath), max_len, pData->szDllPath, max_len);

	data.ProcessID		= pData->ProcessID;
	data.Mode			= pData->Mode;
	data.Method			= pData->Method;
	data.Flags			= pData->Flags;
	data.hHandleValue	= pData->hHandleValue;

	return InjectW(&data);	
}

DWORD __stdcall InjectW(INJECTIONDATAW * pData)
{
#pragma EXPORT_FUNCTION(__FUNCTION__, __FUNCDNAME__)
	
	DWORD ErrOut = 0;
	
	if (!pData->szDllPath)
		return InitErrorStruct(nullptr, pData, false, INJ_ERR_INVALID_FILEPATH);

	const wchar_t * szDllPath = pData->szDllPath;

	if (!pData->ProcessID)
		return InitErrorStruct(szDllPath, pData, false, INJ_ERR_INVALID_PID);

	if (!FileExists(szDllPath))
	{
		pData->LastErrorCode = GetLastError();

		return InitErrorStruct(szDllPath, pData, false, INJ_ERR_FILE_DOESNT_EXIST);
	}
	
	HANDLE hTargetProc = nullptr;
	if (pData->Flags & INJ_HIJACK_HANDLE)
	{
		if (pData->hHandleValue) 
		{
			hTargetProc = MPTR(pData->hHandleValue);
		}
		else
		{
			return HijackHandle(pData);
		}
	}
	else
	{
		DWORD access_mask = PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_QUERY_INFORMATION | PROCESS_QUERY_LIMITED_INFORMATION;
		if (pData->Method == LM_NtCreateThreadEx)
			access_mask |= PROCESS_CREATE_THREAD;

		hTargetProc = OpenProcess(access_mask, FALSE, pData->ProcessID);
		if (!hTargetProc)
		{
			pData->LastErrorCode = GetLastError();

			return InitErrorStruct(szDllPath, pData, false, INJ_ERR_CANT_OPEN_PROCESS);
		}
	}

	DWORD handle_info = 0;
	if (!hTargetProc || !GetHandleInformation(hTargetProc, &handle_info))
	{
		pData->LastErrorCode = GetLastError();

		return InitErrorStruct(szDllPath, pData, false, INJ_ERR_INVALID_PROC_HANDLE);
	}

	wchar_t target_exe_name[MAX_PATH]{ 0 };
	if (!K32GetModuleBaseNameW(hTargetProc, NULL, target_exe_name, sizeof(target_exe_name) / sizeof(target_exe_name[0])))
	{
		pData->szTargetProcessExeFileName = nullptr;
	}
	else
	{
		pData->szTargetProcessExeFileName = target_exe_name;
	}

	bool native_target = true;
#ifdef _WIN64
	native_target = IsNativeProcess(hTargetProc);
	if (native_target)
	{
		ErrOut = ValidateFile(szDllPath, IMAGE_FILE_MACHINE_AMD64);
	}
	else
	{
		ErrOut = ValidateFile(szDllPath, IMAGE_FILE_MACHINE_I386);
	}
#else
	ErrOut = ValidateFile(szDllPath, IMAGE_FILE_MACHINE_I386);
#endif

	if (ErrOut)
	{
		pData->LastErrorCode = ErrOut;

		return InitErrorStruct(szDllPath, pData, native_target, INJ_ERR_PLATFORM_MISMATCH);
	}
	
	HINSTANCE hOut	= NULL;
	DWORD RetVal	= INJ_ERR_SUCCESS;
#ifdef _WIN64
	if (native_target)
	{
		RetVal = InjectDLL(szDllPath, hTargetProc, pData->Mode, pData->Method, pData->Flags, ErrOut, hOut);
	}
	else
	{		
		RetVal = InjectDLL_WOW64(szDllPath, hTargetProc, pData->Mode, pData->Method, pData->Flags, ErrOut, hOut);
	}	
#else
	RetVal = InjectDLL(szDllPath, hTargetProc, pData->Mode, pData->Method, pData->Flags, ErrOut, hOut);
#endif

	if (!(pData->Flags & INJ_HIJACK_HANDLE))
		CloseHandle(hTargetProc);
	
	pData->LastErrorCode	= ErrOut;
	pData->hDllOut			= hOut;

	return InitErrorStruct(szDllPath, pData, native_target, RetVal);
}

DWORD InjectDLL(const wchar_t * szDllFile, HANDLE hTargetProc, INJECTION_MODE im, LAUNCH_METHOD Method, DWORD Flags, DWORD & LastError, HINSTANCE & hOut)
{
	if (Flags & INJ_LOAD_DLL_COPY)
	{
		size_t len_out = 0;
		StringCchLengthW(szDllFile, STRSAFE_MAX_CCH, &len_out);

		const wchar_t * pFileName = szDllFile;
		pFileName += len_out - 1;
		while (*(pFileName-- - 2) != '\\');
		
		wchar_t new_path[MAXPATH_IN_TCHAR]{ 0 };
		GetTempPathW(MAXPATH_IN_TCHAR, new_path);
		StringCchCatW(new_path, MAXPATH_IN_TCHAR, pFileName);

		CopyFileW(szDllFile, new_path, FALSE);

		szDllFile = new_path;
	}

	if (Flags & INJ_SCRAMBLE_DLL_NAME)
	{
		wchar_t new_name[15]{ 0 };
		UINT seed = rand() + Flags + LOWORD(hTargetProc);
		LARGE_INTEGER pfc{ 0 };
		QueryPerformanceCounter(&pfc);
		seed += pfc.LowPart;	
		srand(seed);

		for (UINT i = 0; i != 10; ++i)
		{
			auto val = rand() % 3;
			if (val == 0)
			{
				val = rand() % 10;
				new_name[i] = wchar_t('0' + val);
			}
			else if (val == 1)
			{
				val = rand() % 26;
				new_name[i] = wchar_t('A' + val);
			}
			else
			{
				val = rand() % 26;
				new_name[i] = wchar_t('a' + val);
			}
		}
		new_name[10] = '.';
		new_name[11] = 'd';
		new_name[12] = 'l';
		new_name[13] = 'l';
		new_name[14] = '\0';

		wchar_t OldFilePath[MAXPATH_IN_TCHAR]{ 0 };
		StringCchCopyW(OldFilePath, MAXPATH_IN_TCHAR, szDllFile);

		wchar_t * pFileName = const_cast<wchar_t*>(szDllFile);
		size_t len_out = 0;
		StringCchLengthW(szDllFile, STRSAFE_MAX_CCH, &len_out);
		pFileName += len_out;
		while (*(pFileName-- - 2) != '\\');

		memcpy(pFileName, new_name, 15 * sizeof(wchar_t));

		_wrename(OldFilePath, szDllFile);
	}

	DWORD Ret = 0;

	switch (im)
	{
		case IM_LoadLibrary:
			Ret = _LoadLibraryExW(szDllFile, hTargetProc, Method, Flags, hOut, LastError);
			break;

		case IM_LdrLoadDll:
			Ret = _LdrLoadDll(szDllFile, hTargetProc, Method, Flags, hOut, LastError);
			break;

		case IM_ManualMap:
			Ret = _ManualMap(szDllFile, hTargetProc, Method, Flags, hOut, LastError);
	}

	if (Ret != INJ_ERR_SUCCESS)
	{
		return Ret;
	}

	if (!hOut)
	{
		return INJ_ERR_REMOTE_CODE_FAILED;
	}

	if (im != IM_ManualMap)
	{
		Ret = Cloaking(hTargetProc, Flags, hOut, LastError);
	}
	
	return Ret;
}

DWORD HijackHandle(INJECTIONDATAW * pData)
{
	wchar_t * szDllPath = pData->szDllPath;

	DWORD access_mask = PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_QUERY_INFORMATION | PROCESS_QUERY_LIMITED_INFORMATION;
	if (pData->Method == LM_NtCreateThreadEx)
		access_mask |= PROCESS_CREATE_THREAD;

	auto handles = FindProcessHandles(pData->ProcessID, access_mask);
	if (handles.empty())
	{
		return InitErrorStruct(szDllPath, pData, true, INJ_ERR_NO_HANDLES);
	}

	INJECTIONDATAW hijack_data{ 0 };
	hijack_data.Mode	= IM_LdrLoadDll;
	hijack_data.Method	= LM_NtCreateThreadEx;
	GetOwnModulePath(hijack_data.szDllPath, sizeof(hijack_data.szDllPath) / sizeof(hijack_data.szDllPath[0]));
	StringCbCatW(hijack_data.szDllPath, sizeof(hijack_data.szDllPath), GH_INJ_MOD_NAMEW);
	
	DWORD LastErrCode	= INJ_ERR_SUCCESS;
	HANDLE hHijackProc	= nullptr;

	for (auto i : handles)
	{
		hHijackProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, i.OwnerPID);
		if (!hHijackProc)
		{
			pData->LastErrorCode = GetLastError();
			LastErrCode = INJ_ERR_CANT_OPEN_PROCESS;

			continue;
		}
					
		if (!IsElevatedProcess(hHijackProc) || !IsNativeProcess(hHijackProc))
		{
			pData->LastErrorCode = GetLastError();
			LastErrCode = INJ_ERR_HIJACK_NO_NATIVE_HANDLE;

			CloseHandle(hHijackProc);
			
			continue;
		}

		HINSTANCE hInjectionModuleEx = NULL;
		hInjectionModuleEx = GetModuleHandleEx(hHijackProc, GH_INJ_MOD_NAME);
		if (!hInjectionModuleEx)
		{
			hijack_data.ProcessID = i.OwnerPID;
			DWORD inj_ret = InjectW(&hijack_data);

			if (inj_ret || !hijack_data.hDllOut)
			{
				pData->LastErrorCode = inj_ret;
				LastErrCode = INJ_ERR_HIJACK_INJ_FAILED;

				CloseHandle(hHijackProc);
			
				continue;
			}

			hInjectionModuleEx = hijack_data.hDllOut;
		}

		void * pRemoteInjectW = nullptr;
		bool bLoaded = GetProcAddressEx(hHijackProc, hInjectionModuleEx, "InjectW", pRemoteInjectW);
		if (!bLoaded)
		{
			LastErrCode = INJ_ERR_HIJACK_INJECTW_MISSING;

			EjectDll(hHijackProc, hInjectionModuleEx);
			CloseHandle(hHijackProc);
			
			continue;
		}


		pData->hHandleValue = (DWORD)i.hValue;
		
		void * pArg = VirtualAllocEx(hHijackProc, nullptr, sizeof(INJECTIONDATAW), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
		if (!pArg)
		{
			pData->LastErrorCode = GetLastError();
			LastErrCode = INJ_ERR_HIJACK_CANT_ALLOC;

			EjectDll(hHijackProc, hInjectionModuleEx);
			CloseHandle(hHijackProc);
			
			continue;
		}

		if (!WriteProcessMemory(hHijackProc, pArg, pData, sizeof(INJECTIONDATAW), nullptr))
		{
			pData->LastErrorCode = GetLastError();
			LastErrCode = INJ_ERR_HIJACK_CANT_WPM;

			VirtualFreeEx(hHijackProc, pArg, 0, MEM_RELEASE);
			EjectDll(hHijackProc, hInjectionModuleEx);
			CloseHandle(hHijackProc);
			
			continue;
		}

		pData->hHandleValue = 0;

		DWORD win32err			= ERROR_SUCCESS;
		ULONG_PTR hijack_ret	= INJ_ERR_SUCCESS;
		DWORD remote_ret = StartRoutine(hHijackProc, static_cast<f_Routine*>(pRemoteInjectW), pArg, LM_NtCreateThreadEx, false, win32err, hijack_ret);
		
		INJECTIONDATAW data_out{ 0 };
		ReadProcessMemory(hHijackProc, pArg, &data_out, sizeof(INJECTIONDATAW), nullptr);
		
		VirtualFreeEx(hHijackProc, pArg, 0, MEM_RELEASE);
		EjectDll(hHijackProc, hInjectionModuleEx);
		CloseHandle(hHijackProc);

		if (remote_ret != SR_ERR_SUCCESS)
		{
			pData->LastErrorCode = win32err;
			LastErrCode = remote_ret;
			
			continue;
		}
		else if(remote_ret == SR_ERR_SUCCESS)
		{
			if (hijack_ret != INJ_ERR_SUCCESS)
			{
				pData->LastErrorCode = hijack_ret;
				LastErrCode = INJ_ERR_REMOTE_CODE_FAILED;
				
				continue;
			}
		}

		pData->LastErrorCode = INJ_ERR_SUCCESS;
		LastErrCode = INJ_ERR_SUCCESS;

		break;
	}
		
	return InitErrorStruct(szDllPath, pData, true, LastErrCode);
}

DWORD Cloaking(HANDLE hTargetProc, DWORD Flags, HINSTANCE hMod, DWORD & LastError)
{
	if (!Flags)
		return INJ_ERR_SUCCESS;

	if (Flags & INJ_ERASE_HEADER)
	{
		BYTE Buffer[0x1000]{ 0 };
		DWORD dwOld = 0; 
		BOOL bRet = VirtualProtectEx(hTargetProc, hMod, 0x1000, PAGE_EXECUTE_READWRITE, &dwOld);
		if (!bRet)
		{
			LastError = GetLastError();

			return INJ_ERR_VPE_FAIL;
		}

		bRet = WriteProcessMemory(hTargetProc, hMod, Buffer, 0x1000, nullptr);
		if (!bRet)
		{
			LastError = GetLastError();

			return INJ_ERR_WPM_FAIL;
		}
	}
	else if (Flags & INJ_FAKE_HEADER)
	{
		void * pK32 = ReCa<void*>(GetModuleHandleA("kernel32.dll"));
		if (!pK32)
		{
			LastError = GetLastError();

			return INJ_ERR_GET_MODULE_HANDLE_FAIL;
		}


		DWORD dwOld = 0;

		BOOL bRet = VirtualProtectEx(hTargetProc, hMod, 0x1000, PAGE_EXECUTE_READWRITE, &dwOld);
		if (!bRet)
		{
			LastError = GetLastError();

			return INJ_ERR_VPE_FAIL;
		}
		
		bRet = WriteProcessMemory(hTargetProc, hMod, pK32, 0x1000, nullptr);
		if (!bRet)
		{
			LastError = GetLastError();

			return INJ_ERR_WPM_FAIL;
		}

		bRet = VirtualProtectEx(hTargetProc, hMod, 0x1000, dwOld, &dwOld);
		if (!bRet)
		{
			LastError = GetLastError();

			return INJ_ERR_VPE_FAIL;
		}
	}

	if (Flags & INJ_UNLINK_FROM_PEB)
	{
		ProcessInfo PI;
		PI.SetProcess(hTargetProc);

		NTStuff::LDR_DATA_TABLE_ENTRY * pEntry = PI.GetLdrEntry(hMod);
		if (!pEntry)
		{
			return INJ_ERR_CANT_FIND_MOD_PEB;
		}

		NTStuff::LDR_DATA_TABLE_ENTRY Entry{ 0 };
		if (!ReadProcessMemory(hTargetProc, pEntry, &Entry, sizeof(Entry), nullptr))
		{
			return INJ_ERR_CANT_ACCESS_PEB_LDR;
		}

		auto Unlink = [=](LIST_ENTRY entry)
		{
			LIST_ENTRY list;
			ReadProcessMemory(hTargetProc, entry.Flink, &list, sizeof(LIST_ENTRY), nullptr);
			list.Blink = entry.Blink;
			WriteProcessMemory(hTargetProc, entry.Flink, &list, sizeof(LIST_ENTRY), nullptr);
			
			ReadProcessMemory(hTargetProc, entry.Blink, &list, sizeof(LIST_ENTRY), nullptr);
			list.Flink = entry.Flink;
			WriteProcessMemory(hTargetProc, entry.Blink, &list, sizeof(LIST_ENTRY), nullptr);
		};

		Unlink(Entry.InInitOrder);
		Unlink(Entry.InLoadOrder);
		Unlink(Entry.InMemoryOrder);
	}
	
	return INJ_ERR_SUCCESS;
}