#pragma once

#include "pch.h"

namespace AntiDebug
{
	// =======================================================================
	// Hardware Breakpoint
	// =======================================================================

	inline static BOOL CheckHardwareDebugRegisters()
	{
		BOOL bIsDebugging = FALSE;
		CONTEXT ctx = { 0 };
		HANDLE hThread = GetCurrentThread();

		ctx.ContextFlags = CONTEXT_DEBUG_REGISTERS;

		if (GetThreadContext(hThread, &ctx))
		{
			if ((ctx.Dr0 != 0x00) || (ctx.Dr1 != 0x00) || (ctx.Dr2 != 0x00) || (ctx.Dr3 != 0x00) || (ctx.Dr6 != 0x00) || (ctx.Dr7 != 0x00))
				bIsDebugging = TRUE;
		}

		return bIsDebugging;
	}

	// =======================================================================
	// NtGlobalFlag
	// =======================================================================

	inline static BOOL CheckNtGlobalFlag()
	{
		BOOL bIsDebugging = FALSE;

		_asm
		{
			xor eax, eax;			// clear eax
			mov eax, fs: [0x30] ;	// Reference start of the PEB
			mov eax, [eax + 0x68];	// PEB+0x68 points to NtGlobalFlag
			and eax, 0x00000070;	// check three flags:
			//   FLG_HEAP_ENABLE_TAIL_CHECK   (0x10)
			//   FLG_HEAP_ENABLE_FREE_CHECK   (0x20)
			//   FLG_HEAP_VALIDATE_PARAMETERS (0x40)
			mov bIsDebugging, eax;	// Copy result into 'bIsDebugging'
		}

		return bIsDebugging;
	}

	// =======================================================================
	// NtQueryInformationProcess
	// =======================================================================

	inline static BOOL CheckNtQueryInformationProcess()
	{
		HANDLE hProcess = INVALID_HANDLE_VALUE;
		PROCESS_BASIC_INFORMATION pProcBasicInfo = { 0 };
		ULONG returnLength = 0;

		hProcess = GetCurrentProcess();

		// Note: There are many options for the 2nd parameter NtQueryInformationProcess
		// (ProcessInformationClass) many of them are opaque. While we use ProcessBasicInformation (0), 
		// we could also use:
		//      ProcessDebugPort (7)
		//      ProcessDebugObjectHandle (30)
		//      ProcessDebugFlags (31)
		// There are likely others. You can find many other options for ProcessInformationClass over at PINVOKE:
		//      https://www.pinvoke.net/default.aspx/ntdll/PROCESSINFOCLASS.html
		// Keep in mind that NtQueryInformationProcess will return different things depending on the ProcessInformationClass used.
		// Many online articles using NtQueryInformationProcess for anti-debugging will use DWORD types for NtQueryInformationProcess 
		// paramters. This is fine for 32-builds with some ProcessInformationClass values, but it will cause some to fail on 64-bit builds.
		// In the event of a failure NtQueryInformationProcess will likely return STATUS_INFO_LENGTH_MISMATCH (0xC0000004). 

		// Query ProcessDebugPort
		NTSTATUS status = NtQueryInformationProcess(hProcess, ProcessBasicInformation, &pProcBasicInfo, sizeof(pProcBasicInfo), &returnLength);

		if (NT_SUCCESS(status))
		{
			PPEB pPeb = pProcBasicInfo.PebBaseAddress;

			if (pPeb)
			{
				if (pPeb->BeingDebugged)
					return TRUE;
			}
		}

		return FALSE;
	}

	// =======================================================================
	// Remote Debugger Present
	// =======================================================================

	inline static BOOL CheckRemoteDebuggerPresent()
	{
		HANDLE hProcess = INVALID_HANDLE_VALUE;
		BOOL bIsDebugging = FALSE;

		hProcess = GetCurrentProcess();
		::CheckRemoteDebuggerPresent(hProcess, &bIsDebugging);

		return bIsDebugging;
	}

	// =======================================================================
	// DBG_PRINTEXCEPTION_C
	// =======================================================================

	inline static BOOL CheckDbgPrintRaiseException()
	{
		__try
		{
			RaiseException(DBG_PRINTEXCEPTION_C, 0, 0, 0);
		}
		__except (GetExceptionCode() == DBG_PRINTEXCEPTION_C)
		{
			return false;
		}

		return true;
	}

	// =======================================================================
	// DBG_PRINTEXCEPTION_C
	// =======================================================================

	inline static BOOL CheckNtSetDebugFilterState()
	{
		return NT_SUCCESS(NtSetDebugFilterState(0, 0, TRUE));
	}

	// =======================================================================
	// Debugger Present
	// =======================================================================

	inline static BOOL CheckIsDebuggerPresent()
	{
		return IsDebuggerPresent();
	}

	// =======================================================================
	// Being Debugged PEB
	// =======================================================================

	inline static BOOL CheckBeingDebuggedPEB()
	{
		BOOL bIsDebugging = FALSE;

		_asm
		{
			xor eax, eax;				// clear eax
			mov eax, fs: [0x30] ;		// Reference start of the PEB
			mov eax, [eax + 0x02];		// PEB+2 points to BeingDebugged
			and eax, 0xFF;				// only reference one byte
			mov bIsDebugging, eax;		// Copy BeingDebugged into 'bIsDebugging'
		}

		return bIsDebugging;
	}

	inline static BOOL CheckWrittenPages()
	{
		BOOL result = FALSE, error = FALSE;

		const int SIZE_TO_CHECK = 4096;

		PVOID* addresses = static_cast<PVOID*>(VirtualAlloc(NULL, SIZE_TO_CHECK * sizeof(PVOID), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE));
		if (addresses == NULL)
		{
			return true;
		}

		int* buffer = static_cast<int*>(VirtualAlloc(NULL, SIZE_TO_CHECK * SIZE_TO_CHECK, MEM_RESERVE | MEM_COMMIT | MEM_WRITE_WATCH, PAGE_READWRITE));
		if (buffer == NULL)
		{
			VirtualFree(addresses, 0, MEM_RELEASE);
			return true;
		}

		// Make some calls where a buffer *can* be written to, but isn't actually edited because we pass invalid parameters    
		if (GlobalGetAtomName(INVALID_ATOM, (LPTSTR)buffer, 1) != FALSE
			|| GetEnvironmentVariable("This variable does not exist", (LPSTR)buffer, 4096 * 4096) != FALSE
			|| GetBinaryType("This name does not exist", (LPDWORD)buffer) != FALSE
			|| HeapQueryInformation(0, (HEAP_INFORMATION_CLASS)69, buffer, 4096, NULL) != FALSE
			|| ReadProcessMemory(INVALID_HANDLE_VALUE, (LPCVOID)0x69696969, buffer, 4096, NULL) != FALSE
			|| GetThreadContext(INVALID_HANDLE_VALUE, (LPCONTEXT)buffer) != FALSE
			|| GetWriteWatch(0, &result, 0, NULL, NULL, (PULONG)buffer) == 0)
		{
			result = false;
			error = true;
		}

		if (error == FALSE)
		{
			// A this point all calls failed as they're supposed to
			ULONG_PTR hits = SIZE_TO_CHECK;
			DWORD granularity;
			if (GetWriteWatch(0, buffer, SIZE_TO_CHECK, addresses, &hits, &granularity) != 0)
			{
				result = FALSE;
			}
			else
			{
				// Should have zero reads here because GlobalGetAtomName doesn't probe the buffer until other checks have succeeded
				// If there's an API hook or debugger in here it'll probably try to probe the buffer, which will be caught here
				result = hits != 0;
			}
		}

		VirtualFree(addresses, 0, MEM_RELEASE);
		VirtualFree(buffer, 0, MEM_RELEASE);

		return result;
	}

	LONG WINAPI UnhandledExceptionFilter(PEXCEPTION_POINTERS pExceptionInfo)
	{
		PCONTEXT ctx = pExceptionInfo->ContextRecord;
		ctx->Eip += 3; // Skip \xCC\xEB\x??
		return EXCEPTION_CONTINUE_EXECUTION;
	}

	BOOL CheckUnhandledExceptionFilter()
	{
		bool bDebugged = TRUE;

		SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)UnhandledExceptionFilter);

		__asm
		{
			int 3                      // CC
			jmp near being_debugged    // EB ??
		}

		bDebugged = FALSE;

		being_debugged:
			return bDebugged;
	}

	BOOL CheckMemoryBreakpointC3()
	{
		unsigned char* pMem = nullptr;
		SYSTEM_INFO sysinfo = { 0 };
		DWORD OldProtect = 0;
		void* pAllocation = nullptr;

		GetSystemInfo(&sysinfo);
		pAllocation = VirtualAlloc(NULL, sysinfo.dwPageSize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

		//Not Found
		if (pAllocation == NULL)
			return FALSE;

		//Write a ret to the buffer (opcode 0xC3)
		pMem = (unsigned char*)pAllocation;
		*pMem = 0xC3;

		//Make the page a guard page         
		if (VirtualProtect(pAllocation, sysinfo.dwPageSize, PAGE_EXECUTE_READWRITE | PAGE_GUARD, &OldProtect) == 0)
			return FALSE;

		__try
		{
			__asm
			{
				mov eax, pAllocation
				//This is the address we'll return to if we're under a debugger
				push MemBpBeingDebugged
				jmp eax //Exception or execution, which shall it be :D?
			}
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			//The exception occured and no debugger was detected
			VirtualFree(pAllocation, NULL, MEM_RELEASE);
			return FALSE;
		}

		__asm {MemBpBeingDebugged:}
		VirtualFree(pAllocation, NULL, MEM_RELEASE);

		return TRUE;
	}

	BOOL CheckMemoryBreakpoint2D()
	{
		unsigned char* pMem = nullptr;
		SYSTEM_INFO sysinfo = { 0 };
		DWORD OldProtect = 0;
		void* pAllocation = nullptr;

		GetSystemInfo(&sysinfo);
		pAllocation = VirtualAlloc(NULL, sysinfo.dwPageSize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

		//Not Found
		if (pAllocation == NULL)
			return FALSE;

		//Write a ret to the buffer (opcode 0x2D)
		pMem = (unsigned char*)pAllocation;
		*pMem = 0x2D;

		//Make the page a guard page         
		if (VirtualProtect(pAllocation, sysinfo.dwPageSize, PAGE_EXECUTE_READWRITE | PAGE_GUARD, &OldProtect) == 0)
			return FALSE;

		__try
		{
			__asm
			{
				mov eax, pAllocation
				//This is the address we'll return to if we're under a debugger
				push MemBpBeingDebugged
				jmp eax //Exception or execution, which shall it be :D?
			}
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			//The exception occured and no debugger was detected
			VirtualFree(pAllocation, NULL, MEM_RELEASE);
			return FALSE;
		}

		__asm {MemBpBeingDebugged:}
		VirtualFree(pAllocation, NULL, MEM_RELEASE);

		return TRUE;
	}

	BOOL CheckMemoryBreakpointF1()
	{
		unsigned char* pMem = nullptr;
		SYSTEM_INFO sysinfo = { 0 };
		DWORD OldProtect = 0;
		void* pAllocation = nullptr;

		GetSystemInfo(&sysinfo);
		pAllocation = VirtualAlloc(NULL, sysinfo.dwPageSize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

		//Not Found
		if (pAllocation == NULL)
			return FALSE;

		//Write a ret to the buffer (opcode 0xF1)
		pMem = (unsigned char*)pAllocation;
		*pMem = 0xF1;

		//Make the page a guard page         
		if (VirtualProtect(pAllocation, sysinfo.dwPageSize, PAGE_EXECUTE_READWRITE | PAGE_GUARD, &OldProtect) == 0)
			return FALSE;

		__try
		{
			__asm
			{
				mov eax, pAllocation
				//This is the address we'll return to if we're under a debugger
				push MemBpBeingDebugged
				jmp eax //Exception or execution, which shall it be :D?
			}
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			//The exception occured and no debugger was detected
			VirtualFree(pAllocation, NULL, MEM_RELEASE);
			return FALSE;
		}

		__asm {MemBpBeingDebugged:}
		VirtualFree(pAllocation, NULL, MEM_RELEASE);

		return TRUE;
	}
	// =======================================================================
	// Check Is Debugging
	// =======================================================================

#define NtCurrentThread ((HANDLE)-2)

	bool CheckThreadHideFromDebugger()
	{
		NTSTATUS status = NtSetInformationThread(
			NtCurrentThread,
			(THREADINFOCLASS)0x11,
			NULL,
			0);

		return !(status >= 0);
	}

	inline static ReportCode IsDebugged()
	{
#ifdef VMPROTECT
		VMProtectBeginMutation("AntiDebug::IsDebugged");
#endif

		if(CheckThreadHideFromDebugger())
		{
#ifdef DEBUG_LOG
			Print("CheckThreadHideFromDebugger");
#endif
			return REPORT_CODE_DETECT_THREAD_HIDE_FROM_DEBUGGER;
	}

		if (CheckHardwareDebugRegisters())
		{
#ifdef DEBUG_LOG
			Print("CheckHardwareDebugRegisters");
#endif
			return REPORT_CODE_DETECT_HARDWARE_DEBUG_REGISTERS;
		}

		if (CheckNtQueryInformationProcess())
		{
#ifdef DEBUG_LOG
			Print("CheckNtQueryInformationProcess");
#endif
			return REPORT_CODE_DETECT_NT_QUERY_INFORMATION_PROCESS;
		}

		if (CheckNtGlobalFlag())
		{
#ifdef DEBUG_LOG
			Print("CheckNtGlobalFlag");
#endif
			return REPORT_CODE_DETECT_NT_GLOBAL_FLAG;
		}

		if (CheckRemoteDebuggerPresent())
		{
#ifdef DEBUG_LOG
			Print("CheckRemoteDebuggerPresent");
#endif
			return REPORT_CODE_DETECT_REMOTE_DEBUGGER_PRESENT;
		}

		if (CheckIsDebuggerPresent())
		{
#ifdef DEBUG_LOG
			Print("CheckIsDebuggerPresent");
#endif
			return REPORT_CODE_DETECT_IS_DEBUGGER_PRESENT;
		}

		if (CheckBeingDebuggedPEB())
		{
#ifdef DEBUG_LOG
			Print("CheckBeingDebuggedPEB");
#endif
			return REPORT_CODE_DETECT_BEING_DEBUGGED_PEB;
		}

		if (CheckDbgPrintRaiseException())
		{
#ifdef DEBUG_LOG
			Print("CheckDbgPrintRaiseException");
#endif
			return REPORT_CODE_DETECT_DBG_PRINT_RAISE_EXCEPTION;
		}

		if (CheckNtSetDebugFilterState())
		{
#ifdef DEBUG_LOG
			Print("CheckNtSetDebugFilterState");
#endif
			return REPORT_CODE_DETECT_SET_DEBUG_FILTER_STATE;
		}

		if (CheckWrittenPages())
		{
#ifdef DEBUG_LOG
			Print("CheckWrittenPages");
#endif
			return REPORT_CODE_DETECT_WRITTEN_PAGES;
		}

		if(CheckUnhandledExceptionFilter())
		{
#ifdef DEBUG_LOG
			Print("CheckUnhandledExceptionFilter");
#endif
			return REPORT_CODE_DETECT_UNHANDLED_EXCEPTION_FILTER;
		}

		if (CheckMemoryBreakpointC3())
		{
#ifdef DEBUG_LOG
			Print("CheckMemoryBreakpointC3");
#endif
			return REPORT_CODE_DETECT_MEMORY_BREAKBPOINT_C3;
		}

		if (CheckMemoryBreakpoint2D())
		{
#ifdef DEBUG_LOG
			Print("CheckMemoryBreakpoint2D");
#endif
			return REPORT_CODE_DETECT_MEMORY_BREAKBPOINT_2D;
		}

		if (CheckMemoryBreakpointF1())
		{
#ifdef DEBUG_LOG
			Print("CheckMemoryBreakpointF1");
#endif
			return REPORT_CODE_DETECT_MEMORY_BREAKBPOINT_F1;
		}

		return REPORT_CODE_NONE;

#ifdef VMPROTECT
		VMProtectEnd();
#endif
	}

}


