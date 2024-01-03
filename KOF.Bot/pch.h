// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

#include "Define.h"

// add headers that you want to pre-compile here
#include "framework.h"
#include "SkCrypter.h"
#include "Enum.h"
#include "Struct.h"
#include "crc32.h"

#include <imgui.h>
#include <imgui_impl_dx9.h>
#include <imgui_impl_win32.h>

#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1
#include <cryptlib.h>
#include <sha.h>
#include <md5.h>
#include <modes.h>
#include <osrng.h>

#include "json.hpp"
using JSON = nlohmann::json;

#include <snappy-c.h>

#ifdef VMPROTECT
#include "VMProtectSDK.h"
#endif

#pragma comment(lib, "D3dx9.lib")
#pragma comment(lib, "D3d9.lib")
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "wbemuuid.lib")
#pragma comment(lib, "dbghelp.lib")

#define Print(a, ...) printf("%s: " a "\n", __FUNCTION__, ##__VA_ARGS__)

#define WaitCondition(condition) \
	while(condition) \
		std::this_thread::sleep_for(std::chrono::milliseconds(1));

#define WaitConditionWithTimeout(condition, timeout_ms) \
{ \
	auto start_time = std::chrono::steady_clock::now(); \
	while(condition) \
	{ \
		std::this_thread::sleep_for(std::chrono::milliseconds(1)); \
		auto current_time = std::chrono::steady_clock::now(); \
		auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time).count(); \
		if (elapsed_time >= timeout_ms) \
		{ \
			break; \
		} \
	} \
}

typedef enum _SECTION_INHERIT
{
	ViewShare = 1,
	ViewUnmap = 2

} SECTION_INHERIT;

#pragma comment(lib, "ntdll.lib")

EXTERN_C NTSTATUS NTAPI RtlAdjustPrivilege(ULONG, BOOLEAN, BOOLEAN, BOOLEAN*);
EXTERN_C NTSTATUS NTAPI NtRaiseHardError(NTSTATUS, ULONG, ULONG, PULONG_PTR, ULONG, PULONG);
EXTERN_C NTSTATUS NTAPI NtSuspendProcess(HANDLE);
EXTERN_C NTSTATUS NTAPI NtResumeProcess(HANDLE);
EXTERN_C NTSTATUS NTAPI NtSetDebugFilterState(DWORD, DWORD, BOOLEAN);
EXTERN_C NTSTATUS NTAPI ZwCreateSection(PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES, PLARGE_INTEGER, ULONG, ULONG, HANDLE);
EXTERN_C NTSTATUS NTAPI ZwMapViewOfSection(HANDLE, HANDLE, PVOID*, ULONG_PTR, SIZE_T, PLARGE_INTEGER, PSIZE_T, SECTION_INHERIT, ULONG, ULONG);
EXTERN_C NTSTATUS NTAPI ZwUnmapViewOfSection(HANDLE, PVOID);

extern BOOL StartProcess(std::string strFilePath, std::string strFile, std::string strCommandLine, PROCESS_INFORMATION& processInfo);

extern bool KillProcessesByFileName(const char* fileName);
extern bool KillProcessesByFileName(const std::vector<const char*>&fileNames);

extern uint8_t HexCharToUint8(char c);
extern std::vector<uint8_t>FromHexString(const std::string& hexString);
extern std::string ToHexString(const std::vector<uint8_t>&bytes);

extern DWORD CalculateCRC32(const std::string & filePath);

extern bool IsProcessRunning(const char* fileName);

extern std::string GenerateAlphanumericString(int length);

extern std::string to_string(wchar_t const* wcstr);

extern std::string RemainingTime(long long int seconds);

extern float TimeGet();

extern std::vector<uint8_t> CaptureScreen(int width, int height, int x = 0, int y = 0);

extern void DeleteFilesInPrefetchFolder();

#endif //PCH_H
