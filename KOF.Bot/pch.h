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
#include "crc32.h"
using JSON = nlohmann::json;

#include <imgui.h>
#include <d3d9.h>

#if defined(__linux__) || defined(__APPLE__)
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>
#elif _WIN32
#include <winsock2.h>
#include <Ws2ipdef.h>
#include <Ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")
#endif

#include <string>
#include <functional>
#include <cerrno>

#include "imgui.h"
#include "imgui_impl_dx9.h"
#include "imgui_impl_win32.h"

#ifdef VMPROTECT
#include "VMProtectSDK.h"
#endif

#define WaitCondition(condition) \
	while(condition) \
		std::this_thread::sleep_for(std::chrono::milliseconds(1));

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
typedef NTSTATUS(NTAPI* pdef_NtRaiseHardError)(NTSTATUS ErrorStatus, ULONG NumberOfParameters, ULONG UnicodeStringParameterMask OPTIONAL, PULONG_PTR Parameters, ULONG ResponseOption, PULONG Response);
typedef NTSTATUS(NTAPI* pdef_RtlAdjustPrivilege)(ULONG Privilege, BOOLEAN Enable, BOOLEAN CurrentThread, PBOOLEAN Enabled);

#ifdef ENABLE_ATTACH_PROTECT
#define DbgBreakPoint_FUNC_SIZE 2
#define DbgUiRemoteBreakin_FUNC_SIZE 0x54
#define NtContinue_FUNC_SIZE 0x18

typedef struct _PATCH_FUNC
{
	std::string funcName;
	PVOID funcAddr;
	SIZE_T funcSize;
} PATCH_FUNC;
#endif

extern void SuspendProcess(HANDLE hProcess);
extern void SuspendProcess(DWORD dwProcessId);
extern void ResumeProcess(HANDLE hProcess);
extern void ResumeProcess(DWORD dwProcessId);

extern BOOL StartProcess(std::string strFilePath, std::string strFile, std::string strCommandLine, PROCESS_INFORMATION& processInfo);

extern BOOL TerminateMyProcess(DWORD dwProcessId, UINT uExitCode);

extern bool Injection(DWORD iTargetProcess, std::string szPath);

extern bool ConsoleCommand(const std::string & input, std::string & out);

extern bool KillProcessesByFileName(const char* fileName);
extern bool KillProcessesByFileNames(const std::vector<const char*>&fileNames);

extern uint8_t hexCharToUint8(char c);
extern std::vector<uint8_t> fromHexString(const std::string& hexString);
extern std::string toHexString(const std::vector<uint8_t>&bytes);

extern const char* stristr(const char* haystack, const char* needle);

extern std::string calculateElapsedTime(const std::chrono::time_point<std::chrono::system_clock>&start_time);
extern DWORD CalculateCRC32(const std::string & filePath);

extern bool IsProcessRunning(const char* fileName);

extern std::string GenerateAlphanumericString(int length);

extern std::string to_string(wchar_t const* wcstr);

extern bool IsFolderExists(const std::string & folderPath);
extern bool IsFolderExistsOrCreate(const std::string & folderPath);
extern bool CreateFolder(const std::string & folderPath);

extern std::string RemainingTime(long long int seconds);
extern bool CheckFileExistence(const std::string & path, const std::vector<std::string>&fileArray);
extern void TriggerBSOD();

#endif //PCH_H
