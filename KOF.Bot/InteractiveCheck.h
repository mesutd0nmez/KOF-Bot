#pragma once

#include "pch.h"

namespace InteractiveCheck
{
	static float m_fLastCheckPrefetchTime = TimeGet();
	static float m_fLastCheckWindowAndTitleClassTime = TimeGet();
	static float m_fLastCheckProcessTime = TimeGet();

	static std::string m_szPayload = "";
	inline static std::string GetLastPayload() { return m_szPayload; }

	std::vector<std::string> vecDatabase =
	{
		skCryptDec("charles"), skCryptDec("cheat"), skCryptDec("comview"), skCryptDec("cpacket"), skCryptDec("Debug"), skCryptDec("disassembly"),
		skCryptDec("dnspy"),skCryptDec("Ethereal"),skCryptDec("EtherApe"), skCryptDec("Fiddler"), skCryptDec("ghidra"), skCryptDec("ghidrarun"),
		skCryptDec("glasswire"), skCryptDec("HTTP Debugger"), skCryptDec("HTTP Debugger (32 bit)"), skCryptDec("HTTP Debugger (64 bit)"),
		skCryptDec("HTTPDebuggerSvc"), skCryptDec("HTTPDebuggerUI"), skCryptDec("ida64"), skCryptDec("idag"), skCryptDec("idag64"), skCryptDec("idau"),
		skCryptDec("idau64"), skCryptDec("idaw"), skCryptDec("idaw64"), skCryptDec("idaq"), skCryptDec("idaq64"), skCryptDec("Immunity"), skCryptDec("IMMUNITYDEBUGGER"),
		skCryptDec("Import reconstructor"),skCryptDec("ImportREC"), skCryptDec("Intercepter"), skCryptDec("Kismet"), skCryptDec("MegaDumper"),
		skCryptDec("MegaDumper 1.0 by CodeCracker / SnD"), skCryptDec("netcheat"), skCryptDec("ollydbg"), skCryptDec("paessler"),
		skCryptDec("procexp"), skCryptDec("procexp32"), skCryptDec("procexp64"), skCryptDec("procexp86"), skCryptDec("procmonx"), skCryptDec("protection_id"),
		skCryptDec("reflector"), skCryptDec("reshacker"), skCryptDec("riverbed"), skCryptDec("scylla"), skCryptDec("scylla_x64"), skCryptDec("scylla_x86"),
		skCryptDec("sectools"), skCryptDec("snpa"), skCryptDec("solarwinds"), skCryptDec("tcpdump"), skCryptDec("telerik"), skCryptDec("wireshark"), skCryptDec("WinDbg"),
		skCryptDec("windbg"), skCryptDec("winpcap"), skCryptDec("x32dbg"), skCryptDec("x64dbg"), skCryptDec("[CPU"), skCryptDec("Process Hacker"), skCryptDec("ProcessHacker")
	};

	inline static BOOL CheckPrefetch()
	{
		std::string prefetchFolderPath = skCryptDec("C:\\Windows\\Prefetch");

		WIN32_FIND_DATA findFileData;
		HANDLE hFind = FindFirstFileA((prefetchFolderPath + skCryptDec("\\*")).c_str(), &findFileData);

		if (hFind == INVALID_HANDLE_VALUE)
		{
#ifdef DEBUG_LOG
			Print("FindFirstFileA handle failed");
#endif
			return FALSE;
		}

		do
		{
			if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				continue;

			std::string fileName(findFileData.cFileName);
			std::transform(fileName.begin(), fileName.end(), fileName.begin(), ::toupper);

			for (auto target : vecDatabase)
			{
				std::transform(target.begin(), target.end(), target.begin(), ::toupper);		

				if (fileName.find(target) != std::string::npos)
				{
					JSON jPayload;

					std::string szPatternAttribute = skCryptDec("pattern");
					jPayload[szPatternAttribute.c_str()] = target;

					std::string szFileAttribute = skCryptDec("file");
					jPayload[szFileAttribute.c_str()] = fileName;

					m_szPayload = jPayload.dump();
#ifdef DEBUG_LOG
					Print("CheckPrefetch Payload: %s", fileName.c_str());
#endif
					return TRUE;
				}
			}
		} while (FindNextFile(hFind, &findFileData) != 0);

		FindClose(hFind);

		return FALSE;
	}

	inline static BOOL CheckWindowTitleAndClass()
	{
		auto callback = [](HWND hwnd, LPARAM lParam) -> BOOL 
		{
			char windowTitle[256];
			GetWindowText(hwnd, windowTitle, sizeof(windowTitle));

			char windowClass[256];
			GetClassName(hwnd, windowClass, sizeof(windowClass));

			for (auto target : vecDatabase)
			{
				std::string szWindowTitle(windowTitle);
				std::string szWindowClass(windowClass);

				std::transform(target.begin(), target.end(), target.begin(), ::tolower);
				std::transform(szWindowTitle.begin(), szWindowTitle.end(), szWindowTitle.begin(), ::tolower);
				std::transform(szWindowClass.begin(), szWindowClass.end(), szWindowClass.begin(), ::tolower);

				if (szWindowTitle.find(target) != std::string::npos)
				{
					JSON jPayload;

					std::string szPatternAttribute = skCryptDec("pattern");
					jPayload[szPatternAttribute.c_str()] = target;

					std::string szTitleAttribute = skCryptDec("title");
					jPayload[szTitleAttribute.c_str()] = szWindowTitle;		

					DWORD dwProcessId;
					GetWindowThreadProcessId(hwnd, &dwProcessId);

					HANDLE hProcessHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, dwProcessId);

					if (hProcessHandle != NULL)
					{
						CHAR szFilePath[MAX_PATH];

						if (GetModuleFileNameEx(hProcessHandle, NULL, szFilePath, MAX_PATH) != 0) 
						{
							std::string szProcessAttribute = skCryptDec("file");
							jPayload[szProcessAttribute.c_str()] = szFilePath;
						}

						CloseHandle(hProcessHandle);
					}

					m_szPayload = jPayload.dump();
#ifdef DEBUG_LOG
					Print("CheckWindowTitle Payload: %s", m_szPayload.c_str());
#endif
					return FALSE;
				}

				if (szWindowClass.find(target) != std::string::npos)
				{
					JSON jPayload;

					std::string szPatternAttribute = skCryptDec("pattern");
					jPayload[szPatternAttribute.c_str()] = target;

					std::string szClassAttribute = skCryptDec("class");
					jPayload[szClassAttribute.c_str()] = szWindowClass;

					DWORD dwProcessId;
					GetWindowThreadProcessId(hwnd, &dwProcessId);

					HANDLE hProcessHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, dwProcessId);

					if (hProcessHandle != NULL)
					{
						CHAR szFilePath[MAX_PATH];

						if (GetModuleFileNameEx(hProcessHandle, NULL, szFilePath, MAX_PATH) != 0)
						{
							std::string szProcessAttribute = skCryptDec("file");
							jPayload[szProcessAttribute.c_str()] = szFilePath;
						}

						CloseHandle(hProcessHandle);
					}

					m_szPayload = jPayload.dump();
#ifdef DEBUG_LOG
					Print("CheckWindowClass Payload: %s", m_szPayload.c_str());
#endif
					return FALSE;
				}
			}

			return TRUE;
		};

		return !EnumWindows(callback, NULL);
	}

	inline static BOOL CheckProcess()
	{
		PROCESSENTRY32 pe32;
		pe32.dwSize = sizeof(PROCESSENTRY32);
		HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

		if (hProcessSnap == INVALID_HANDLE_VALUE)
		{
#ifdef DEBUG_LOG
			Print("Error creating process snapshot");
#endif
			return FALSE;
		}

		if (!Process32First(hProcessSnap, &pe32))
		{
			CloseHandle(hProcessSnap);
#ifdef DEBUG_LOG
			Print("Error getting process information");
#endif
			return FALSE;
		}

		do
		{
			std::string processName = pe32.szExeFile;
			std::transform(processName.begin(), processName.end(), processName.begin(), ::tolower);

			for (auto target : vecDatabase)
			{
				std::transform(target.begin(), target.end(), target.begin(), ::tolower);

				if (processName.find(target) != std::string::npos)
				{
					JSON jPayload;

					std::string szPatternAttribute = skCryptDec("pattern");
					jPayload[szPatternAttribute.c_str()] = target;

					std::string szClassAttribute = skCryptDec("file");
					jPayload[szClassAttribute.c_str()] = pe32.szExeFile;

					m_szPayload = jPayload.dump();

#ifdef DEBUG_LOG
					Print("CheckProcess Payload: %s", m_szPayload.c_str());
#endif
					CloseHandle(hProcessSnap);
					return TRUE;
				}
			}

		} while (Process32Next(hProcessSnap, &pe32));

		CloseHandle(hProcessSnap);

		return FALSE;
	}

	inline static ReportCode IsDetected()
	{
#ifdef VMPROTECT
		VMProtectBeginMutation("InteractiveCheck::IsDetected");
#endif

		float fCurrentTime = TimeGet();

		if (fCurrentTime > (m_fLastCheckPrefetchTime + (2064.0f / 1000.0f)))
		{
			if (CheckPrefetch())
			{
#ifdef DEBUG_LOG
				Print("CheckPrefetchFolder");
#endif
				return REPORT_CODE_DETECT_CHECK_PREFETCH_FOLDER;
			}

			m_fLastCheckPrefetchTime = TimeGet();
		}

		if (fCurrentTime > (m_fLastCheckProcessTime + (2464.0f / 1000.0f)))
		{
			if (CheckProcess())
			{
#ifdef DEBUG_LOG
				Print("CheckProcess");
#endif
				return REPORT_CODE_DETECT_CHECK_PROCESS;
			}

			m_fLastCheckProcessTime = TimeGet();
		}

		if (fCurrentTime > (m_fLastCheckWindowAndTitleClassTime + (2864.0f / 1000.0f)))
		{
			if (CheckWindowTitleAndClass())
			{
#ifdef DEBUG_LOG
				Print("CheckWindowTitleAndClass");
#endif
				return REPORT_CODE_DETECT_CHECK_WINDOW_TITLE_AND_CLASS;
			}

			m_fLastCheckWindowAndTitleClassTime = TimeGet();
		}

		return REPORT_CODE_NONE;

#ifdef VMPROTECT
		VMProtectEnd();
#endif
	}
}

