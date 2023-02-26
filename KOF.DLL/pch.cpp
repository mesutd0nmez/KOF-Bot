// pch.cpp: source file corresponding to the pre-compiled header

#include "pch.h"

BOOL StartProcess(std::string szFilePath, std::string szFile, std::string szCommandLine, PROCESS_INFORMATION& processInfo)
{
	STARTUPINFO info = { sizeof(info) };

	std::wstring szWorkingDirectory(szFilePath.begin(), szFilePath.end());
	std::wstring strFileName(szFile.begin(), szFile.end());

	std::wostringstream fileArgs;
	fileArgs << szWorkingDirectory.c_str() << "\\" << strFileName.c_str();

	std::wstring file = fileArgs.str();

	std::wostringstream cmdArgs;
	cmdArgs << L"\"" << szWorkingDirectory.c_str() << "\\" << strFileName.c_str() << L"\"";
	cmdArgs << L" ";
	cmdArgs << szCommandLine.c_str();

	std::wstring cmd = cmdArgs.str();

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
