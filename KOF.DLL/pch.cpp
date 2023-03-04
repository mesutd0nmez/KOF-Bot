// pch.cpp: source file corresponding to the pre-compiled header

#include "pch.h"

void SuspendProcess(HANDLE hProcess)
{
	HMODULE hModuleNtdll = GetModuleHandle("ntdll");

	if (hModuleNtdll != 0)
	{
		NtSuspendProcess pNtSuspendProcess = (NtSuspendProcess)GetProcAddress(
			hModuleNtdll, "NtSuspendProcess");

		if (!pNtSuspendProcess)
			return;

		pNtSuspendProcess(hProcess);
	}
}

void SuspendProcess(DWORD iProcessId)
{
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, iProcessId);

	if (hProcess != 0)
	{
		ResumeProcess(hProcess);
		CloseHandle(hProcess);
	}
}

void ResumeProcess(HANDLE hProcess)
{
	HMODULE hModuleNtdll = GetModuleHandle("ntdll");

	if (hModuleNtdll != 0)
	{
		NtResumeProcess pResumeProcess = (NtResumeProcess)GetProcAddress(
			hModuleNtdll, "NtResumeProcess");

		if (!pResumeProcess)
			return;

		pResumeProcess(hProcess);
	}
}

void ResumeProcess(DWORD iProcessId)
{
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, iProcessId);

	if (hProcess != 0)
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
	fileArgs << szWorkingDirectory.c_str() << "\\" << strFileName.c_str();

	std::string file = fileArgs.str();

	std::ostringstream cmdArgs;
	cmdArgs << "\"" << szWorkingDirectory.c_str() << "\\" << strFileName.c_str() << "\"";
	cmdArgs << " ";
	cmdArgs << szCommandLine.c_str();

	std::string cmd = cmdArgs.str();

	BOOL result = CreateProcess(&file[0], &cmd[0], NULL, NULL, TRUE, 0, NULL, &szWorkingDirectory[0], &info, &processInfo);

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
		throw std::logic_error("Illegal byte sequence");
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

std::vector<std::string> Tokenize(std::string const& str, const char delimeter)
{
	std::vector<std::string> out;

	std::stringstream ss(str);

	std::string s;
	while (std::getline(ss, s, delimeter))
	{
		out.push_back(s);
	}

	return out;
}
