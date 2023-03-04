// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

// add headers that you want to pre-compile here
#include "framework.h"

#include "SkCrypter.h"
#include "Define.h"
#include "Enum.h"
#include "Struct.h"

#define WaitCondition(condition) \
	while(condition) \
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

typedef LONG(NTAPI* NtSuspendProcess)(IN HANDLE ProcessHandle);
typedef LONG(NTAPI* NtResumeProcess)(IN HANDLE ProcessHandle);

extern void SuspendProcess(HANDLE hProcess);
extern void SuspendProcess(DWORD iProcessId);
extern void ResumeProcess(HANDLE hProcess);
extern void ResumeProcess(DWORD iProcessId);

extern BOOL StartProcess(std::string strFilePath, std::string strFile, std::string strCommandLine, PROCESS_INFORMATION& processInfo);

extern std::string to_string(wchar_t const* wcstr);
extern std::string to_string(std::wstring const& wstr);

extern std::vector<std::string> Tokenize(std::string const& str, const char delimeter);

#endif //PCH_H
