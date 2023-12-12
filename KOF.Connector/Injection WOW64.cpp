#include "pch.h"

#ifdef _WIN64

#include "Injection.h"
#pragma comment (lib, "Psapi.lib")

DWORD Cloaking_WOW64(HANDLE hTargetProc, DWORD Flags, HINSTANCE hMod, DWORD & LastError);

DWORD InjectDLL_WOW64(const wchar_t * szDllFile, HANDLE hTargetProc, INJECTION_MODE im, LAUNCH_METHOD Method, DWORD Flags, DWORD & LastError, HINSTANCE & hOut)
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
			Ret = _LoadLibrary_WOW64(szDllFile, hTargetProc, Method, Flags, hOut, LastError);
			break;

		case IM_LdrLoadDll:
			Ret = _LdrLoadDll_WOW64(szDllFile, hTargetProc, Method, Flags, hOut, LastError);
			break;

		case IM_ManualMap:
			Ret = _ManualMap_WOW64(szDllFile, hTargetProc, Method, Flags, hOut, LastError);
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
		Ret = Cloaking_WOW64(hTargetProc, Flags, hOut, LastError);
	}
	
	return Ret;
}

DWORD Cloaking_WOW64(HANDLE hTargetProc, DWORD Flags, HINSTANCE hMod, DWORD & LastError)
{
	if (!Flags) 
	{
		return INJ_ERR_SUCCESS;
	}

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
		void * pK32 = ReCa<void*>(GetModuleHandleEx_WOW64(hTargetProc, TEXT("kernel32.dll")));
		DWORD dwOld = 0;

		BYTE buffer[0x1000];
		BOOL bRet = ReadProcessMemory(hTargetProc, pK32, buffer, 0x1000, nullptr);
		if (!bRet)
		{
			LastError = GetLastError();

			return INJ_ERR_RPM_FAIL;
		}

		bRet = VirtualProtectEx(hTargetProc, hMod, 0x1000, PAGE_EXECUTE_READWRITE, &dwOld);
		if (!bRet)
		{
			LastError = GetLastError();

			return INJ_ERR_VPE_FAIL;
		}

		bRet = WriteProcessMemory(hTargetProc, hMod, buffer, 0x1000, nullptr);
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

		LDR_DATA_TABLE_ENTRY32 * pEntry = PI.GetLdrEntry_WOW64(hMod);
		if (!pEntry)
		{
			return INJ_ERR_CANT_FIND_MOD_PEB;
		}

		LDR_DATA_TABLE_ENTRY32 Entry{ 0 };
		if (!ReadProcessMemory(hTargetProc, pEntry, &Entry, sizeof(Entry), nullptr))
		{
			return INJ_ERR_CANT_ACCESS_PEB_LDR;
		}
		
		auto Unlink = [=](LIST_ENTRY32 entry)
		{
			LIST_ENTRY32 list;
			ReadProcessMemory(hTargetProc, MPTR(entry.Flink), &list, sizeof(LIST_ENTRY32), nullptr);
			list.Blink = entry.Blink;
			WriteProcessMemory(hTargetProc, MPTR(entry.Flink), &list, sizeof(LIST_ENTRY32), nullptr);

			ReadProcessMemory(hTargetProc, MPTR(entry.Blink), &list, sizeof(LIST_ENTRY32), nullptr);
			list.Flink = entry.Flink;
			WriteProcessMemory(hTargetProc, MPTR(entry.Blink), &list, sizeof(LIST_ENTRY32), nullptr);
		};
		
		Unlink(Entry.InLoadOrder);
		Unlink(Entry.InMemoryOrder);
		Unlink(Entry.InInitOrder);
	}
	
	return INJ_ERR_SUCCESS;
}

#endif