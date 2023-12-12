#pragma once

#include "Tools.h"

//Wrapper class which relies on information from NtQuerySystemInformation and NtQueryInformationProcess

//Used to enumerate threads, retrieve PEBs, etc...

//Honestly, too lazy to document

class ProcessInfo
{
	NTStuff::SYSTEM_PROCESS_INFORMATION	* m_pCurrentProcess = nullptr;
	NTStuff::SYSTEM_PROCESS_INFORMATION	* m_pFirstProcess	= nullptr;
	NTStuff::SYSTEM_THREAD_INFORMATION	* m_pCurrentThread	= nullptr;

	ULONG m_BufferSize = 0;

	HANDLE m_hCurrentProcess = nullptr;

	NTStuff::f_NtQueryInformationProcess m_pNtQueryInformationProcess	= nullptr;
	NTStuff::f_NtQuerySystemInformation	m_pNtQuerySystemInformation		= nullptr;

	NTStuff::PEB						* GetPEB_Native();
	NTStuff::LDR_DATA_TABLE_ENTRY	* GetLdrEntry_Native(HINSTANCE hMod);

public:

	ProcessInfo();
	~ProcessInfo();

	bool SetProcess(HANDLE hTargetProc);
	bool SetThread(DWORD TID);
	bool NextThread();

	bool RefreshInformation();

	NTStuff::PEB						* GetPEB();
	NTStuff::LDR_DATA_TABLE_ENTRY	* GetLdrEntry(HINSTANCE hMod);

	DWORD GetPID();

	bool IsNative();

	void * GetEntrypoint();

	DWORD GetTID();
	DWORD GetThreadId();
	bool GetThreadState(NTStuff::THREAD_STATE & state, NTStuff::KWAIT_REASON & reason);
	bool GetThreadStartAddress(void * & start_address);

	const NTStuff::SYSTEM_PROCESS_INFORMATION	* GetProcessInfo();
	const NTStuff::SYSTEM_THREAD_INFORMATION		* GetThreadInfo();

#ifdef _WIN64

	PEB32					* GetPEB_WOW64();
	LDR_DATA_TABLE_ENTRY32	* GetLdrEntry_WOW64(HINSTANCE hMod);

#endif
};