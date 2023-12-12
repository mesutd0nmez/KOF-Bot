#include "pch.h"

#include "Tools.h"

bool FileExists(const wchar_t * szFile)
{
	return (GetFileAttributesW(szFile) != INVALID_FILE_ATTRIBUTES);
}

DWORD ValidateFile(const wchar_t * szFile, DWORD desired_machine)
{
	std::ifstream File(szFile, std::ios::binary | std::ios::ate);
	if (!File.good())
	{
		return FILE_ERR_CANT_OPEN_FILE;
	}

	auto FileSize = File.tellg();
	if (FileSize < 0x1000)
	{
		return FILE_ERR_INVALID_FILE_SIZE;
	}

	BYTE * headers = new BYTE[0x1000];
	File.seekg(0, std::ios::beg);
	File.read(ReCa<char*>(headers), 0x1000);
	File.close();

	auto * pDos = ReCa<IMAGE_DOS_HEADER*>(headers);
	auto * pNT	= ReCa<IMAGE_NT_HEADERS*>(headers + pDos->e_lfanew); //no need for correct nt headers type

	WORD magic		= pDos->e_magic;
	DWORD signature = pNT->Signature;
	WORD machine	= pNT->FileHeader.Machine;

	delete[] headers;

	if (magic != 0x5A4D || signature != 0x4550 || machine != desired_machine) //"MZ" & "PE"
	{
		return FILE_ERR_INVALID_FILE;
	}

	return 0;
}

bool GetOwnModulePath(wchar_t * pOut, size_t BufferCchSize)
{
	MODULEENTRY32W ME32{ 0 };
	ME32.dwSize = sizeof(ME32);

	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, GetCurrentProcessId());

	BOOL bRet = Module32FirstW(hSnap, &ME32);
	while (bRet)
	{
		if (ME32.hModule == g_hInjMod)
			break;
		bRet = Module32NextW(hSnap, &ME32);
	}

	CloseHandle(hSnap);

	if (!bRet)
		return false;

	size_t size_out = 0;
	StringCchLengthW(ME32.szExePath, sizeof(ME32.szExePath), &size_out);

	wchar_t * temp_path = ME32.szExePath;
	temp_path += size_out;
	while (*temp_path-- != '\\');
	*(temp_path + 2) = '\0';

	StringCchCopyW(pOut, BufferCchSize, ME32.szExePath);

	return true;	
}

bool IsNativeProcess(HANDLE hTargetProc)
{
	BOOL bWOW64 = FALSE;
	IsWow64Process(hTargetProc, &bWOW64);

	return (bWOW64 == FALSE);
}

ULONG GetSessionId(HANDLE hTargetProc, NTSTATUS & ntRetOut)
{	
	auto h_nt_dll = GetModuleHandleA("ntdll.dll");
	if (!h_nt_dll)
		return false;

	auto p_NtQueryInformationProcess = ReCa<NTStuff::f_NtQueryInformationProcess>(GetProcAddress(h_nt_dll, "NtQueryInformationProcess"));
	if (!p_NtQueryInformationProcess)
		return false;

	NTStuff::PROCESS_SESSION_INFORMATION psi{ 0 };
	ntRetOut = p_NtQueryInformationProcess(hTargetProc, NTStuff::ProcessSessionInformation, &psi, sizeof(psi), nullptr);
	if (NT_FAIL(ntRetOut))
		return (ULONG)-1;

	return psi.SessionId;
}

bool IsElevatedProcess(HANDLE hTargetProc)
{
	HANDLE hToken = nullptr;
	if (!OpenProcessToken(hTargetProc, TOKEN_QUERY, &hToken))
		return false;

	TOKEN_ELEVATION te{ 0 };
	DWORD SizeOut = 0;
	GetTokenInformation(hToken, TokenElevation, &te, sizeof(te), &SizeOut);

	CloseHandle(hToken);
	
	return (te.TokenIsElevated != 0);
}

void ErrorLog(ERROR_INFO * info)
{
	return;
	wchar_t pPath[MAX_PATH * 2]{ 0 };
	if (!GetOwnModulePath(pPath, sizeof(pPath) / sizeof(pPath[0])))
		return;

	wchar_t ErrorLogName[] = L"Log.txt";

	wchar_t FullPath[MAX_PATH];
	StringCbCopyW(FullPath, sizeof(FullPath), pPath);
	StringCbCatW(FullPath, sizeof(FullPath), ErrorLogName);
		
	time_t time_raw	= time(nullptr);
	tm time_info;
	localtime_s(&time_info, &time_raw);
	wchar_t szTime[30]{ 0 };
	wcsftime(szTime, 30, L"%d-%m-%Y %H:%M:%S", &time_info);

	wchar_t szFlags			[9]{ 0 };
	wchar_t szErrorCode		[9]{ 0 };
	wchar_t szWin32Error	[9]{ 0 };
	wchar_t szHandleValue	[9]{ 0 };
	StringCchPrintfW(szFlags,		9, L"%08X", info->Flags);
	StringCchPrintfW(szErrorCode,	9, L"%08X", info->ErrorCode);
	StringCchPrintfW(szWin32Error,	9, L"%08X", info->LastWin32Error);
	StringCchPrintfW(szHandleValue,	9, L"%08X", info->HandleValue);
	
	std::wofstream error_log(FullPath, std::ios_base::out | std::ios_base::app);
	if (!error_log.good())
	{
		return;
	}

	error_log << szTime																			<< std::endl;
	error_log << L"Version          : " << GH_INJ_VERSION										<< std::endl;
	error_log << L"File             : ";
	if (info->szDllFileName)
		error_log << info->szDllFileName														<< std::endl;
	else
		error_log << "(nullptr)"																<< std::endl;
	error_log << L"Target           : ";
	if (info->szTargetProcessExeFileName)
		error_log << info->szTargetProcessExeFileName											<< std::endl;
	else
		error_log << "(nullptr)"																<< std::endl;
	error_log << L"Target PID       : "		<< info->TargetProcessId							<< std::endl;
	error_log << L"Injectionmode    : "		<< info->InjectionMode								<< std::endl;
	error_log << L"Launchmethod     : "		<< info->LaunchMethod								<< std::endl;
	error_log << L"Platform         : "		<< (info->bNative ? L"x64/x86 (native)" : L"wow64")	<< std::endl;
	error_log << L"Flags            : 0x"	<< szFlags											<< std::endl;
	error_log << L"Errorcode        : 0x"	<< szErrorCode										<< std::endl;
	error_log << L"Win32Error       : 0x"	<< szWin32Error										<< std::endl;
	if (info->HandleValue && (info->Flags & 0x100))
	{
		error_log << L"Hijack PID       : "	<< GetCurrentProcessId()							<< std::endl;
		error_log << L"HandleValue      : "	<< szHandleValue									<< std::endl;
	}
	
	error_log << std::endl;

	error_log.close();
}