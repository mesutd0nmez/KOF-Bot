// pch.cpp: source file corresponding to the pre-compiled header

#include "pch.h"
#include "Injection.h"

void SuspendProcess(HANDLE hProcess)
{
	HMODULE hModuleNtdll = GetModuleHandle(skCryptDec("ntdll"));

	if (hModuleNtdll != 0)
	{
		NtSuspendProcess pNtSuspendProcess = (NtSuspendProcess)GetProcAddress(
			hModuleNtdll, skCryptDec("NtSuspendProcess"));

		if (!pNtSuspendProcess)
			return;

		pNtSuspendProcess(hProcess);
	}
}

void SuspendProcess(DWORD dwProcessId)
{
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessId);

	if (hProcess != nullptr)
	{
		SuspendProcess(hProcess);
		CloseHandle(hProcess);
	}
}

void ResumeProcess(HANDLE hProcess)
{
	HMODULE hModuleNtdll = GetModuleHandle(skCryptDec("ntdll"));

	if (hModuleNtdll != 0)
	{
		NtResumeProcess pResumeProcess = (NtResumeProcess)GetProcAddress(
			hModuleNtdll, skCryptDec("NtResumeProcess"));

		if (!pResumeProcess)
			return;

		pResumeProcess(hProcess);
	}
}

void ResumeProcess(DWORD dwProcessId)
{
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessId);

	if (hProcess != nullptr)
	{
		ResumeProcess(hProcess);
		CloseHandle(hProcess);
	}
}

BOOL StartProcess(std::string szFilePath, std::string szFile, std::string szCommandLine, PROCESS_INFORMATION& processInfo)
{
	STARTUPINFO info = { sizeof(info) };

	std::string szWorkingDirectory(szFilePath.begin(), szFilePath.end());
	std::string strFileName(szFile.begin(), szFile.end());

	std::ostringstream fileArgs;
	fileArgs << szWorkingDirectory.c_str() << skCryptDec("\\") << strFileName.c_str();

	std::string file = fileArgs.str();

	std::ostringstream cmdArgs;
	cmdArgs << skCryptDec("\"") << szWorkingDirectory.c_str() << skCryptDec("\\") << strFileName.c_str() << skCryptDec("\"");
	cmdArgs << " ";
	cmdArgs << szCommandLine.c_str();

	std::string cmd = cmdArgs.str();

	SECURITY_ATTRIBUTES securityInfo = { sizeof(securityInfo) };

	securityInfo.bInheritHandle = FALSE;

	BOOL result = CreateProcess(&file[0], &cmd[0], &securityInfo, NULL, FALSE, 0, NULL, &szWorkingDirectory[0], &info, &processInfo);

	if (!result)
		return FALSE;

	return TRUE;
}

std::string to_string(wchar_t const* wcstr)
{
	auto s = std::mbstate_t();
	auto const target_char_count = std::wcsrtombs(nullptr, &wcstr, 0, &s);
	if (target_char_count == static_cast<std::size_t>(-1))
	{
		throw std::logic_error(skCryptDec("Illegal byte sequence"));
	}

	// +1 because std::string adds a null terminator which isn't part of size
	auto str = std::string(target_char_count, '\0');
	std::wcsrtombs(const_cast<char*>(str.data()), &wcstr, str.size() + 1, &s);
	return str;
}

std::string to_string(std::wstring const& wstr)
{
	return to_string(wstr.c_str());
}

BOOL TerminateMyProcess(DWORD dwProcessId, UINT uExitCode)
{
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessId);

	if (hProcess == nullptr)
		return false;

	BOOL bStatus = TerminateProcess(hProcess, uExitCode);

	CloseHandle(hProcess);

	return bStatus;
}

size_t CurlWriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
	((std::string*)userp)->append((char*)contents, size * nmemb);
	return size * nmemb;
}

std::string CurlPost(std::string szUrl, JSON jData)
{
	CURL* curl;
	CURLcode res;
	std::string szReadBuffer;

	curl_global_init(CURL_GLOBAL_DEFAULT);

	curl = curl_easy_init();

	if (curl)
	{
		std::string szJson = jData.dump();

		curl_easy_setopt(curl, CURLOPT_URL, szUrl.c_str());
		curl_easy_setopt(curl, CURLOPT_POST, 1L);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, szJson.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &szReadBuffer);
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, FALSE);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, FALSE);

		struct curl_slist* headers = NULL;
		headers = curl_slist_append(headers, "Content-Type: application/json");
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

		res = curl_easy_perform(curl);

		if (res != CURLE_OK)
		{
			printf("curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
		}

		curl_easy_cleanup(curl);
		curl_slist_free_all(headers);
	}

	curl_global_cleanup();

	return szReadBuffer;
}

void Injection(DWORD iTargetProcess, std::string szPath)
{
	std::ifstream instream(szPath.c_str(), std::ios::in | std::ios::binary);

	if (instream)
	{
		std::vector<uint8_t> dllBuff((std::istreambuf_iterator<char>(instream)), std::istreambuf_iterator<char>());
		Injection(iTargetProcess, dllBuff);
	}
}

void Injection(DWORD iTargetProcess, std::vector<uint8_t> vecBuff)
{
	HINSTANCE hInjectionMod = LoadLibrary(GH_INJ_MOD_NAME);

	if (hInjectionMod == nullptr)
		return;

	auto MemoryInject = (f_Memory_Inject)GetProcAddress(hInjectionMod, "Memory_Inject");
	auto GetSymbolState = (f_GetSymbolState)GetProcAddress(hInjectionMod, "GetSymbolState");
	auto GetImportState = (f_GetSymbolState)GetProcAddress(hInjectionMod, "GetImportState");
	auto StartDownload = (f_StartDownload)GetProcAddress(hInjectionMod, "StartDownload");
	auto GetDownloadProgressEx = (f_GetDownloadProgressEx)GetProcAddress(hInjectionMod, "GetDownloadProgressEx");

	std::this_thread::sleep_for(std::chrono::milliseconds(500));

	StartDownload();

	while (GetDownloadProgressEx(PDB_DOWNLOAD_INDEX_NTDLL, false) != 1.0f)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

#ifdef _WIN64
	while (GetDownloadProgressEx(PDB_DOWNLOAD_INDEX_NTDLL, true) != 1.0f)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
#endif

	while (GetSymbolState() != 0)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	while (GetImportState() != 0)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	bool bGenerateErrorLog = false;

//#ifdef DEBUG
//	bGenerateErrorLog = true;
//#endif

	MEMORY_INJECTIONDATA pData =
	{
		vecBuff.data(),
		vecBuff.size(),
		iTargetProcess,
		INJECTION_MODE::IM_ManualMap,
		LAUNCH_METHOD::LM_QueueUserAPC,
		MM_DEFAULT,
		0,
		NULL,
		NULL,
		bGenerateErrorLog
	};

	MemoryInject(&pData);
}


bool ConsoleCommand(const std::string& input, std::string& out)
{
	auto* shell_cmd = _popen(input.c_str(), "r");

	if (!shell_cmd)
	{
		return false;
	}

	static char buffer[1024] = {};
	while (fgets(buffer, 1024, shell_cmd))
	{
		out.append(buffer);
	}
	_pclose(shell_cmd);

	while (out.back() == '\n' ||
		out.back() == '\0' ||
		out.back() == ' ' ||
		out.back() == '\r' ||
		out.back() == '\t') {
		out.pop_back();
	}

	out.erase(std::remove(out.begin(), out.end(), '\n'), out.cend());

	return !out.empty();
}