#include "pch.h"

#include "Process Info.h"

#pragma comment(lib, "Psapi.lib")

#define NEXT_SYSTEM_PROCESS_ENTRY(pCurrent) ReCa<NTStuff::SYSTEM_PROCESS_INFORMATION*>(ReCa<BYTE*>(pCurrent) + pCurrent->NextEntryOffset)

NTStuff::PEB * ProcessInfo::GetPEB_Native()
{
	if (!m_pFirstProcess)
		return nullptr;

	NTStuff::PROCESS_BASIC_INFORMATION PBI{ 0 };
	ULONG size_out = 0;
	NTSTATUS ntRet = m_pNtQueryInformationProcess(m_hCurrentProcess, NTStuff::ProcessBasicInformation, &PBI, sizeof(NTStuff::PROCESS_BASIC_INFORMATION), &size_out);

	if (NT_FAIL(ntRet))
		return nullptr;

	return PBI.pPEB;
}

NTStuff::LDR_DATA_TABLE_ENTRY * ProcessInfo::GetLdrEntry_Native(HINSTANCE hMod)
{
	if (!m_pFirstProcess)
		return nullptr;

	NTStuff::PEB * ppeb = GetPEB();
	if (!ppeb)
		return nullptr;

	NTStuff::PEB	peb{ 0 };
	if (!ReadProcessMemory(m_hCurrentProcess, ppeb, &peb, sizeof(NTStuff::PEB), nullptr))
		return nullptr;

	NTStuff::PEB_LDR_DATA ldrdata{ 0 };
	if (!ReadProcessMemory(m_hCurrentProcess, peb.Ldr, &ldrdata, sizeof(NTStuff::PEB_LDR_DATA), nullptr))
		return nullptr;

	LIST_ENTRY * pCurrentEntry = ldrdata.InLoadOrderModuleListHead.Flink;
	LIST_ENTRY * pLastEntry = ldrdata.InLoadOrderModuleListHead.Blink;

	while (true)
	{
		NTStuff::LDR_DATA_TABLE_ENTRY CurrentEntry{ 0 };
		ReadProcessMemory(m_hCurrentProcess, pCurrentEntry, &CurrentEntry, sizeof(NTStuff::LDR_DATA_TABLE_ENTRY), nullptr);

		if (CurrentEntry.DllBase == hMod)
			return ReCa<NTStuff::LDR_DATA_TABLE_ENTRY *>(pCurrentEntry);

		if (pCurrentEntry == pLastEntry)
			break;

		pCurrentEntry = CurrentEntry.InLoadOrder.Flink;
	}

	return nullptr;
}

ProcessInfo::ProcessInfo()
{
	HINSTANCE hNTDLL = GetModuleHandleA("ntdll.dll");
	if (!hNTDLL)
		return;

	m_pNtQueryInformationProcess	= ReCa<NTStuff::f_NtQueryInformationProcess>	(GetProcAddress(hNTDLL, "NtQueryInformationProcess"));
	m_pNtQuerySystemInformation		= ReCa<NTStuff::f_NtQuerySystemInformation>	(GetProcAddress(hNTDLL, "NtQuerySystemInformation"));

	if (!m_pNtQueryInformationProcess || !m_pNtQuerySystemInformation)
		return;

	m_BufferSize	= 0x10000;
	m_pFirstProcess = nullptr;

	RefreshInformation();
}

ProcessInfo::~ProcessInfo()
{
	if (m_pFirstProcess)
		delete[] m_pFirstProcess;
}

bool ProcessInfo::SetProcess(HANDLE hTargetProc)
{
	if (!hTargetProc)
		return false;

	if (!m_pFirstProcess)
		if (!RefreshInformation())
			return false;

	m_hCurrentProcess = hTargetProc;

	ULONG_PTR PID = GetProcessId(m_hCurrentProcess);

	while (NEXT_SYSTEM_PROCESS_ENTRY(m_pCurrentProcess) != m_pCurrentProcess)
	{
		if (m_pCurrentProcess->UniqueProcessId == ReCa<void*>(PID))
			break;

		m_pCurrentProcess = NEXT_SYSTEM_PROCESS_ENTRY(m_pCurrentProcess);
	}

	if (m_pCurrentProcess->UniqueProcessId != ReCa<void*>(PID))
	{
		m_pCurrentProcess = m_pFirstProcess;
		return false;
	}

	m_pCurrentThread = &m_pCurrentProcess->Threads[0];

	return true;
}

bool ProcessInfo::SetThread(DWORD TID)
{
	if (!m_pFirstProcess)
		if (!RefreshInformation())
			return false;

	m_pCurrentThread = nullptr;

	for (UINT i = 0; i != m_pCurrentProcess->NumberOfThreads; ++i)
	{
		if (m_pCurrentProcess->Threads[i].ClientId.UniqueThread == ReCa<void*>(ULONG_PTR(TID)))
		{
			m_pCurrentThread = &m_pCurrentProcess->Threads[i];
			break;
		}
	}
	
	if (m_pCurrentThread == nullptr)
	{
		m_pCurrentThread = &m_pCurrentProcess->Threads[0];
		return false;
	}

	return true;
}

bool ProcessInfo::NextThread()
{
	if (!m_pFirstProcess)
		if (!RefreshInformation())
			return false;

	for (UINT i = 0; i != m_pCurrentProcess->NumberOfThreads; ++i)
	{
		if (m_pCurrentProcess->Threads[i].ClientId.UniqueThread == m_pCurrentThread->ClientId.UniqueThread)
		{
			if (i + 1 != m_pCurrentProcess->NumberOfThreads)
			{
				m_pCurrentThread++;
				return true;
			}
			else
			{
				m_pCurrentThread = &m_pCurrentProcess->Threads[0];
				return false;
			}
		}
	}
		
	m_pCurrentThread = &m_pCurrentProcess->Threads[0];

	return false;
}

bool ProcessInfo::RefreshInformation()
{
	if (!m_pFirstProcess)
	{
		m_pFirstProcess = ReCa<NTStuff::SYSTEM_PROCESS_INFORMATION*>(new BYTE[m_BufferSize]);
		if (!m_pFirstProcess)
			return false;
	}
	else
	{
		delete[] m_pFirstProcess;
		m_pFirstProcess = nullptr;

		return RefreshInformation();
	}

	ULONG size_out = 0;
	NTSTATUS ntRet = m_pNtQuerySystemInformation(NTStuff::SystemProcessInformation, m_pFirstProcess, m_BufferSize, &size_out);

	while (ntRet == STATUS_INFO_LENGTH_MISMATCH)
	{
		delete[] m_pFirstProcess;

		m_BufferSize = size_out + 0x1000;
		m_pFirstProcess = ReCa<NTStuff::SYSTEM_PROCESS_INFORMATION*>(new BYTE[m_BufferSize]);
		if (!m_pFirstProcess)
			return false;

		ntRet = m_pNtQuerySystemInformation(NTStuff::SystemProcessInformation, m_pFirstProcess, m_BufferSize, &size_out);
	}

	if (NT_FAIL(ntRet))
	{
		delete[] m_pFirstProcess;
		m_pFirstProcess = nullptr;

		return false;
	}

	m_pCurrentProcess = m_pFirstProcess;
	m_pCurrentThread = &m_pCurrentProcess->Threads[0];

	return true;
}

NTStuff::PEB * ProcessInfo::GetPEB()
{
	return GetPEB_Native();
}

NTStuff::LDR_DATA_TABLE_ENTRY * ProcessInfo::GetLdrEntry(HINSTANCE hMod)
{
	return GetLdrEntry_Native(hMod);
}

DWORD ProcessInfo::GetPID()
{
	return GetProcessId(m_hCurrentProcess);
}

bool ProcessInfo::IsNative()
{
	BOOL bOut = FALSE;
	IsWow64Process(m_hCurrentProcess, &bOut);
	return (bOut == FALSE);
}

void * ProcessInfo::GetEntrypoint()
{
	if (!m_pFirstProcess)
		return nullptr;

	NTStuff::PEB * ppeb = GetPEB();
	if (!ppeb)
		return nullptr;

	NTStuff::PEB	peb;
	if (!ReadProcessMemory(m_hCurrentProcess, ppeb, &peb, sizeof(NTStuff::PEB), nullptr))
		return nullptr;

	NTStuff::PEB_LDR_DATA ldrdata;
	if (!ReadProcessMemory(m_hCurrentProcess, peb.Ldr, &ldrdata, sizeof(NTStuff::PEB_LDR_DATA), nullptr))
		return nullptr;

	LIST_ENTRY * pCurrentEntry	= ldrdata.InLoadOrderModuleListHead.Flink;
	LIST_ENTRY * pLastEntry		= ldrdata.InLoadOrderModuleListHead.Blink;
	
	wchar_t NameBuffer[MAX_PATH]{ 0 };
	while (true)
	{
		NTStuff::LDR_DATA_TABLE_ENTRY CurrentEntry;
		if (ReadProcessMemory(m_hCurrentProcess, pCurrentEntry, &CurrentEntry, sizeof(NTStuff::LDR_DATA_TABLE_ENTRY), nullptr))
		{
			if (ReadProcessMemory(m_hCurrentProcess, CurrentEntry.BaseDllName.szBuffer, NameBuffer, CurrentEntry.BaseDllName.Length, nullptr))
			{
				if (NameBuffer[CurrentEntry.BaseDllName.Length / 2 - 1] == 'e')
				{
					return CurrentEntry.EntryPoint;
				}
			}
		}

		if (pCurrentEntry == pLastEntry)
			break;
		else
			pCurrentEntry = CurrentEntry.InLoadOrder.Flink;
	}

	return nullptr;
}

DWORD ProcessInfo::GetTID()
{
	if (!m_pFirstProcess)
		return 0;

	return DWORD(ReCa<ULONG_PTR>(m_pCurrentThread->ClientId.UniqueThread) & 0xFFFFFFFF);
}

DWORD ProcessInfo::GetThreadId()
{
	if (!m_pFirstProcess)
		return 0;

	return DWORD(ReCa<ULONG_PTR>(m_pCurrentThread->ClientId.UniqueThread) & 0xFFFFFFFF);
}

bool ProcessInfo::GetThreadState(NTStuff::THREAD_STATE & state, NTStuff::KWAIT_REASON & reason)
{
	if (!m_pFirstProcess)
		return false;

	state	= m_pCurrentThread->ThreadState;
	reason	= m_pCurrentThread->WaitReason;

	return true;
}

bool ProcessInfo::GetThreadStartAddress(void * &start_address)
{
	if (!m_pFirstProcess)
		return false;

	start_address = m_pCurrentThread->StartAddress;

	return true;
}

const NTStuff::SYSTEM_PROCESS_INFORMATION * ProcessInfo::GetProcessInfo()
{
	if (m_pFirstProcess)
		return m_pCurrentProcess;

	return nullptr;
}

const NTStuff::SYSTEM_THREAD_INFORMATION * ProcessInfo::GetThreadInfo()
{
	if (m_pFirstProcess)
		return m_pCurrentThread;

	return nullptr;
}

#ifdef _WIN64

PEB32 * ProcessInfo::GetPEB_WOW64()
{
	if (!m_pFirstProcess)
		return nullptr;

	ULONG_PTR pPEB;
	ULONG size_out = 0;
	NTSTATUS ntRet = m_pNtQueryInformationProcess(m_hCurrentProcess, ProcessWow64Information, &pPEB, sizeof(pPEB), &size_out);

	if (NT_FAIL(ntRet))
		return nullptr;

	return ReCa<PEB32 *>(pPEB);
}

LDR_DATA_TABLE_ENTRY32 * ProcessInfo::GetLdrEntry_WOW64(HINSTANCE hMod)
{
	if (!m_pFirstProcess)
		return nullptr;
	
	PEB32 * ppeb = GetPEB_WOW64();
	if (!ppeb)
		return nullptr;

	PEB32 peb{ 0 };
	if (!ReadProcessMemory(m_hCurrentProcess, ppeb, &peb, sizeof(PEB32), nullptr))
		return nullptr;
	
	PEB_LDR_DATA32 ldrdata{ 0 };
	if (!ReadProcessMemory(m_hCurrentProcess, MPTR(peb.Ldr), &ldrdata, sizeof(PEB_LDR_DATA32), nullptr))
		return nullptr;
		
	LIST_ENTRY32 * pCurrentEntry	= ReCa<LIST_ENTRY32*>((ULONG_PTR)ldrdata.InLoadOrderModuleListHead.Flink);
	LIST_ENTRY32 * pLastEntry		= ReCa<LIST_ENTRY32*>((ULONG_PTR)ldrdata.InLoadOrderModuleListHead.Blink);

	while (true)
	{
		LDR_DATA_TABLE_ENTRY32 CurrentEntry{ 0 };
		ReadProcessMemory(m_hCurrentProcess, pCurrentEntry, &CurrentEntry, sizeof(LDR_DATA_TABLE_ENTRY32), nullptr);

		if (CurrentEntry.DllBase == MDWD(hMod))
			return ReCa<LDR_DATA_TABLE_ENTRY32*>(pCurrentEntry);

		if (pCurrentEntry == pLastEntry)
			break;

		pCurrentEntry = ReCa<LIST_ENTRY32*>((ULONG_PTR)CurrentEntry.InLoadOrder.Flink);
	}

	return nullptr;
}
#endif