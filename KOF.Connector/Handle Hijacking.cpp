#include "pch.h"

#include "Handle Hijacking.h"

#include "NT Stuff.h"

NTSTATUS EnumHandles(char * pBuffer, ULONG Size, ULONG * SizeOut, UINT & Count);
std::vector<NTStuff::SYSTEM_HANDLE_TABLE_ENTRY_INFO> EnumProcessHandles();

NTSTATUS EnumHandles(char * pBuffer, ULONG Size, ULONG * SizeOut, UINT & Count)
{
	auto h_nt_dll = GetModuleHandleA("ntdll.dll");
	if (!h_nt_dll)
		return -1;

	auto p_NtQuerySystemInformation = ReCa<NTStuff::f_NtQuerySystemInformation>(GetProcAddress(h_nt_dll, "NtQuerySystemInformation"));
	if (!p_NtQuerySystemInformation)
		return -1;

	NTSTATUS ntRet = p_NtQuerySystemInformation(NTStuff::SystemHandleInformation, pBuffer, Size, SizeOut);

	if (NT_FAIL(ntRet))
		return ntRet;

	auto * pHandleInfo	= ReCa<NTStuff::SYSTEM_HANDLE_INFORMATION*>(pBuffer);
	Count = pHandleInfo->NumberOfHandles;
	
	return ntRet;
}

std::vector<NTStuff::SYSTEM_HANDLE_TABLE_ENTRY_INFO> EnumProcessHandles()
{
	UINT Count		= 0;
	ULONG Size		= 0x10000;
	char * pBuffer	= new char[Size];
	NTSTATUS ntRet	= EnumHandles(pBuffer, Size, &Size, Count);

	std::vector<NTStuff::SYSTEM_HANDLE_TABLE_ENTRY_INFO> Ret;

	if (NT_FAIL(ntRet))
	{
		while (ntRet == STATUS_INFO_LENGTH_MISMATCH)
		{
			delete[] pBuffer;
			pBuffer = new char[Size];
			ntRet = EnumHandles(pBuffer, Size, &Size, Count);
		}

		if (NT_FAIL(ntRet))
		{
			delete[] pBuffer;
			return Ret;
		}
	}

	auto * pEntry = ReCa<NTStuff::SYSTEM_HANDLE_INFORMATION*>(pBuffer)->Handles;
	for (UINT i = 0; i != Count; ++i)
		if (pEntry[i].ObjectTypeIndex == NTStuff::OTI_Process)
			Ret.push_back(pEntry[i]);
		
	delete[] pBuffer;

	return Ret;
}

std::vector<handle_data> FindProcessHandles(DWORD TargetPID, DWORD WantedHandleAccess)
{
	std::vector<handle_data> Ret;
	DWORD OwnerPID		= 0;
	HANDLE hOwnerProc	= nullptr;

	for (auto i : EnumProcessHandles())
	{
		if (OwnerPID != i.UniqueProcessId && i.UniqueProcessId != TargetPID && i.UniqueProcessId != GetCurrentProcessId())
		{
			if (hOwnerProc)
				CloseHandle(hOwnerProc);

			OwnerPID = i.UniqueProcessId;
			hOwnerProc = OpenProcess(PROCESS_DUP_HANDLE, FALSE, OwnerPID);

			if (!hOwnerProc)
				continue;
		}
		else if (!hOwnerProc)
			continue;
		
		HANDLE hDup		= nullptr;
		HANDLE hOrig	= ReCa<HANDLE>(i.HandleValue);
		NTSTATUS ntRet	= DuplicateHandle(hOwnerProc, hOrig, GetCurrentProcess(), &hDup, PROCESS_QUERY_LIMITED_INFORMATION, 0, 0);
		if (NT_FAIL(ntRet))
			continue;
		
		if (GetProcessId(hDup) == TargetPID && (i.GrantedAccess - (i.GrantedAccess ^ WantedHandleAccess) == WantedHandleAccess))
		{
			Ret.push_back(handle_data{ OwnerPID, i.HandleValue, i.GrantedAccess });
		}
		
		CloseHandle(hDup);
	}

	if (hOwnerProc)
		CloseHandle(hOwnerProc);

	return Ret;
}