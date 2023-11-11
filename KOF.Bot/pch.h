// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

// add headers that you want to pre-compile here
#include "framework.h"

#include "Singleton.h"
#include "SkCrypter.h"
#include "Define.h"
#include "Enum.h"
#include "Struct.h"
#include "Json.h"
using JSON = nlohmann::json;

#include <imgui.h>
#include <d3d9.h>

#include <curl/curl.h>

#define WaitCondition(condition) \
	while(condition) \
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

#define WaitConditionWithTimeout(condition, timeout_ms) \
{ \
	auto start_time = std::chrono::steady_clock::now(); \
	while(condition) \
	{ \
		std::this_thread::sleep_for(std::chrono::milliseconds(100)); \
		auto current_time = std::chrono::steady_clock::now(); \
		auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time).count(); \
		if (elapsed_time >= timeout_ms) \
		{ \
			break; \
		} \
	} \
} 

typedef LONG(NTAPI* NtSuspendProcess)(IN HANDLE ProcessHandle);
typedef LONG(NTAPI* NtResumeProcess)(IN HANDLE ProcessHandle);

extern void SuspendProcess(HANDLE hProcess);
extern void SuspendProcess(DWORD dwProcessId);
extern void ResumeProcess(HANDLE hProcess);
extern void ResumeProcess(DWORD dwProcessId);

extern BOOL StartProcess(std::string strFilePath, std::string strFile, std::string strCommandLine, PROCESS_INFORMATION& processInfo);

extern std::string to_string(wchar_t const* wcstr);
extern std::string to_string(std::wstring const& wstr);

extern BOOL TerminateMyProcess(DWORD dwProcessId, UINT uExitCode);

extern std::string CurlPost(std::string szUrl, JSON jData);

extern bool Injection(DWORD iTargetProcess, std::string szPath);

extern bool ConsoleCommand(const std::string & input, std::string & out);

extern bool KillProcessesByFileName(const char* fileName);

#endif //PCH_H
