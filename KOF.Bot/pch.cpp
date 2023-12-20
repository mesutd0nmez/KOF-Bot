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

BOOL TerminateMyProcess(DWORD dwProcessId, UINT uExitCode)
{
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessId);

	if (hProcess == nullptr)
		return false;

	BOOL bStatus = TerminateProcess(hProcess, uExitCode);

	CloseHandle(hProcess);

	return bStatus;
}

bool Injection(DWORD iTargetProcess, std::string szPath)
{
	HINSTANCE hInjectionModule = LoadLibrary(skCryptDec("Connector.dll"));

	if (!hInjectionModule)
	{
		return false;
	}

	auto InjectA = (f_InjectA)GetProcAddress(hInjectionModule, skCryptDec("InjectA"));

	INJECTIONDATAA data =
	{
		0,
		"",
		iTargetProcess,
		INJECTION_MODE::IM_ManualMap,
		LAUNCH_METHOD::LM_HijackThread,
		INJ_ERASE_HEADER | INJ_UNLINK_FROM_PEB | INJ_CLEAN_DATA_DIR | INJ_SHIFT_MODULE | INJ_THREAD_CREATE_CLOAKED,
		0,
		NULL
	};

	strcpy(data.szDllPath, szPath.c_str());

	(f_InjectA)InjectA(&data);

	return true;
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

bool KillProcessesByFileName(const char* fileName) 
{
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if (hSnapshot == INVALID_HANDLE_VALUE) 
	{
#ifdef DEBUG
		std::cerr << "Bot: Error creating process snapshot" << std::endl;
#endif
		return false;
	}

	PROCESSENTRY32 pe;
	pe.dwSize = sizeof(PROCESSENTRY32);

	if (Process32First(hSnapshot, &pe)) 
	{
		do 
		{
			if (_stricmp(pe.szExeFile, fileName) == 0) 
			{
				HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pe.th32ProcessID);

				if (hProcess) 
				{
					if (TerminateProcess(hProcess, 0)) 
					{
#ifdef DEBUG
						std::cout << "Bot: Terminated process ID: " << pe.th32ProcessID << std::endl;
#endif
					}
					else 
					{
#ifdef DEBUG
						std::cerr << "Bot: Failed to terminate process ID: " << pe.th32ProcessID << std::endl;
#endif
					}

					CloseHandle(hProcess);
				}
				else 
				{
#ifdef DEBUG
					std::cerr << "Bot: Failed to open process for termination" << std::endl;
#endif
				}
			}
		} 
		while (Process32Next(hSnapshot, &pe));
	}

	CloseHandle(hSnapshot);

	return true;
}

bool KillProcessesByFileNames(const std::vector<const char*>& fileNames)
{
	DWORD currentProcessId = GetCurrentProcessId();

	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if (hSnapshot == INVALID_HANDLE_VALUE)
	{
#ifdef DEBUG
		std::cerr << "KillProcessesByFileNames: Error creating process snapshot" << std::endl;
#endif
		return false;
	}

	PROCESSENTRY32 pe;
	pe.dwSize = sizeof(PROCESSENTRY32);

	if (Process32First(hSnapshot, &pe))
	{
		do
		{
			if (pe.th32ProcessID == currentProcessId)
			{
				continue;
			}

			for (const char* fileName : fileNames)
			{
				if (_stricmp(pe.szExeFile, fileName) == 0)
				{
					HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pe.th32ProcessID);

					if (hProcess)
					{
						if (TerminateProcess(hProcess, 0))
						{
#ifdef DEBUG
							printf("KillProcessesByFileNames: Terminated process ID: %d\n", pe.th32ProcessID);
#endif
						}
						else
						{
#ifdef DEBUG
							printf("KillProcessesByFileNames: Failed to terminate process ID: %d\n", pe.th32ProcessID);
#endif
						}

						CloseHandle(hProcess);
					}
					else
					{
#ifdef DEBUG
						printf("KillProcessesByFileNames: Failed to open process for termination\n\n");
#endif
					}
				}
			}
		} while (Process32Next(hSnapshot, &pe));
	}

	CloseHandle(hSnapshot);

	return true;
}

uint8_t hexCharToUint8(char c) 
{
	if (c >= '0' && c <= '9') return static_cast<uint8_t>(c - '0');
	if (c >= 'A' && c <= 'F') return static_cast<uint8_t>(c - 'A' + 10);
	if (c >= 'a' && c <= 'f') return static_cast<uint8_t>(c - 'a' + 10);
	throw std::invalid_argument(skCryptDec("Invalid hex character"));
}

std::vector<uint8_t> fromHexString(const std::string& hexString) 
{
	if (hexString.empty())
		return {};

	if (hexString.size() % 2 != 0)
		throw std::invalid_argument(skCryptDec("Bad hex length"));

	std::vector<uint8_t> result(hexString.size() / 2);

	for (size_t i = 0; i < hexString.size(); i += 2) {
		result[i / 2] = (hexCharToUint8(hexString[i]) << 4) | hexCharToUint8(hexString[i + 1]);
	}

	return result;
}

std::string toHexString(const std::vector<uint8_t>& bytes) 
{
	if (bytes.size() == 0)
		return "";

	std::stringstream ss;
	for (auto c : bytes)
		ss << std::setw(2) << std::setfill('0') << std::hex << (int)c;

	return ss.str();
}

const char* stristr(const char* haystack, const char* needle)
{
	size_t needleLength = std::strlen(needle);
	while (*haystack)
	{
		if (_strnicmp(haystack, needle, needleLength) == 0)
		{
			return haystack;
		}
		haystack++;
	}
	return nullptr;
}

std::string calculateElapsedTime(const std::chrono::time_point<std::chrono::system_clock>& start_time) 
{
	auto current_time = std::chrono::system_clock::now();

	auto elapsed_time = std::chrono::duration_cast<std::chrono::minutes>(current_time - start_time);

	auto minutes = elapsed_time.count() % 60;
	auto hours = elapsed_time.count() / 60;

	if (minutes == 0) 
	{
		minutes = 1;
	}

	std::stringstream result_stream;

	if (hours > 0) 
	{
		result_stream << hours << skCryptDec(" saat ");
	}

	if (minutes > 0) 
	{
		result_stream << minutes << skCryptDec(" dakika");
	}

	return result_stream.str();
}

DWORD CalculateCRC32(const std::string& filePath)
{
	std::ifstream fileStream(filePath, std::ios::binary);

	if (!fileStream.is_open())
	{
#ifdef DEBUG
		printf("File not opened: %s\n", filePath.c_str());
#endif
		return 0xFFFFFFFF;
	}

	fileStream.seekg(0, std::ios::end);
	uint32_t iFileSize = (uint32_t)fileStream.tellg();
	fileStream.seekg(0, std::ios::beg);

	std::vector<char> buffer(iFileSize);

	fileStream.read(buffer.data(), iFileSize);
	fileStream.close();

	return crc32((uint8_t*)buffer.data(), iFileSize);
}

bool IsProcessRunning(const char* fileName)
{
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	PROCESSENTRY32 pe32;
	pe32.dwSize = sizeof(PROCESSENTRY32);

	if (Process32First(hSnap, &pe32))
	{
		do
		{
			if (_stricmp(pe32.szExeFile, fileName) == 0)
			{
				CloseHandle(hSnap);
				return true;
			}
		} while (Process32Next(hSnap, &pe32));
	}

	CloseHandle(hSnap);
	return false;
}

std::string GenerateAlphanumericString(int length) 
{
	const std::string characters = skCryptDec("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");

	std::string alphanumericString = "";
	int charactersLength = characters.length();

	std::srand(static_cast<unsigned int>(std::time(nullptr)));

	for (int i = 0; i < length; ++i) 
	{
		int randomIndex = std::rand() % charactersLength;
		alphanumericString += characters[randomIndex];
	}

	return alphanumericString;
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

bool IsFolderExists(const std::string& folderPath) 
{
	DWORD dwAttrib = GetFileAttributesA(folderPath.c_str());

	return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
		(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

bool IsFolderExistsOrCreate(const std::string& folderPath)
{
	if (IsFolderExists(folderPath))
		return true;

	return CreateFolder(folderPath);
}

bool CreateFolder(const std::string& folderPath) 
{
	if (CreateDirectoryA(folderPath.c_str(), NULL) ||
		ERROR_ALREADY_EXISTS == GetLastError()) 
	{
		return true;
	}
	return false;
}

std::string RemainingTime(long long int seconds) 
{
	const int secondsPerMinute = 60;
	const int minutesPerHour = 60;
	const int hoursPerDay = 24;

	long long int remainingDays = seconds / (secondsPerMinute * minutesPerHour * hoursPerDay);
	seconds %= secondsPerMinute * minutesPerHour * hoursPerDay;

	long long int remainingHours = seconds / (secondsPerMinute * minutesPerHour);
	seconds %= secondsPerMinute * minutesPerHour;

	long long int remainingMinutes = seconds / secondsPerMinute;
	seconds %= secondsPerMinute;

	std::string result;

	if (remainingDays > 0) 
	{
		result += std::to_string(remainingDays) + skCryptDec(" gun ");
	}

	if (remainingHours > 0) 
	{
		result += std::to_string(remainingHours) + skCryptDec(" saat ");
	}

	if (remainingMinutes > 0) 
	{
		result += std::to_string(remainingMinutes) + skCryptDec(" dakika ");
	}

	if (seconds > 0 || result.empty()) 
	{
		result += std::to_string(seconds) + skCryptDec(" saniye ");
	}

	if (remainingDays == 0 && remainingHours == 0 && remainingMinutes == 0 && seconds == 0) {
		result += skCryptDec("0 saniye ");
	}

	return result;
}

bool CheckFileExistence(const std::string& path, const std::vector<std::string>& fileArray) 
{
	std::filesystem::path folderPath(path);

	for (const auto& fileName : fileArray) 
	{
		std::filesystem::path filePath = folderPath / fileName;

		if (!std::filesystem::exists(filePath)) 
		{
			return false;
		}
	}

	return true;
}

void TriggerBSOD()
{
	BOOLEAN bEnabled;
	ULONG uResp;

	HMODULE hNtdll = GetModuleHandle(skCryptDec("ntdll.dll"));

	if (!hNtdll) return;

	LPVOID lpRtlAdjustPrivilege = GetProcAddress(hNtdll, skCryptDec("RtlAdjustPrivilege"));
	LPVOID lpNtRaiseHardError = GetProcAddress(hNtdll, skCryptDec("NtRaiseHardError"));

	if (lpRtlAdjustPrivilege && lpNtRaiseHardError)
	{
		pdef_RtlAdjustPrivilege NtCall = (pdef_RtlAdjustPrivilege)lpRtlAdjustPrivilege;
		pdef_NtRaiseHardError NtCall2 = (pdef_NtRaiseHardError)lpNtRaiseHardError;

		// Adjust privilege
		NTSTATUS NtRet = NtCall(19, TRUE, FALSE, &bEnabled);

		// Trigger BSOD
		NtCall2(STATUS_ASSERTION_FAILURE, 0, 0, 0, 6, &uResp);
	}
}