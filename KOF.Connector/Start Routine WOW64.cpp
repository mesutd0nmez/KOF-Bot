#include "pch.h"

#ifdef _WIN64

#include "Start Routine.h"

DWORD SR_NtCreateThreadEx_WOW64	(HANDLE hTargetProc, f_Routine_WOW64 pRoutine, DWORD pArg, DWORD & LastWin32Error, bool CloakThread,		DWORD & Out);
DWORD SR_HijackThread_WOW64		(HANDLE hTargetProc, f_Routine_WOW64 pRoutine, DWORD pArg, DWORD & LastWin32Error,							DWORD & Out);
DWORD SR_SetWindowsHookEx_WOW64	(HANDLE hTargetProc, f_Routine_WOW64 pRoutine, DWORD pArg, DWORD & LastWin32Error, ULONG TargetSessionId,	DWORD & Out);
DWORD SR_QueueUserAPC_WOW64		(HANDLE hTargetProc, f_Routine_WOW64 pRoutine, DWORD pArg, DWORD & LastWin32Error,							DWORD & Out);

DWORD StartRoutine_WOW64(HANDLE hTargetProc, f_Routine_WOW64 pRoutine, DWORD pArg, LAUNCH_METHOD Method, bool CloakThread, DWORD & LastWin32Error, DWORD & Out)
{
	DWORD Ret = 0;
	
	switch (Method)
	{
		case LM_NtCreateThreadEx:
			Ret = SR_NtCreateThreadEx_WOW64(hTargetProc, pRoutine, pArg, LastWin32Error, CloakThread, Out);
			break;

		case LM_HijackThread:
			Ret = SR_HijackThread_WOW64(hTargetProc, pRoutine, pArg, LastWin32Error, Out);
			break;

		case LM_SetWindowsHookEx:
			{
				NTSTATUS ntRet = 0;
				ULONG TargetSession = GetSessionId(hTargetProc, ntRet);
				if (TargetSession == (ULONG)-1)
				{
					LastWin32Error = static_cast<DWORD>(ntRet);
					Ret = SR_ERR_CANT_QUERY_SESSION_ID;
					break;
				}
				else if (TargetSession == GetSessionId(GetCurrentProcess(), ntRet))
				{
					TargetSession = (ULONG)-1;
				}
				Ret = SR_SetWindowsHookEx_WOW64(hTargetProc, pRoutine, pArg, LastWin32Error, TargetSession, Out);
			}
			break;

		case LM_QueueUserAPC:
			Ret = SR_QueueUserAPC_WOW64(hTargetProc, pRoutine, pArg, LastWin32Error, Out);
			break;
	}
	
	return Ret;
}

DWORD SR_NtCreateThreadEx_WOW64(HANDLE hTargetProc, f_Routine_WOW64 pRoutine, DWORD pArg, DWORD & LastWin32Error, bool CloakThread, DWORD & Out)
{
	auto h_nt_dll = GetModuleHandleA("ntdll.dll");
	if (!h_nt_dll)
	{
		LastWin32Error = GetLastError();

		return SR_NTCTE_ERR_GET_MODULE_HANDLE_FAIL;
	}

	auto p_NtCreateThreadEx = ReCa<f_NtCreateThreadEx>(GetProcAddress(h_nt_dll, "NtCreateThreadEx"));
	if (!p_NtCreateThreadEx)
	{
		LastWin32Error = GetLastError();

		return SR_NTCTE_ERR_NTCTE_MISSING;
	}

	DWORD pEntrypoint = pRoutine;
	if (CloakThread)
	{
		ProcessInfo pi;
		pi.SetProcess(hTargetProc);
		pEntrypoint = MDWD(pi.GetEntrypoint());
	}

	DWORD Flags		= THREAD_CREATE_FLAGS_CREATE_SUSPENDED | THREAD_CREATE_FLAGS_SKIP_THREAD_ATTACH | THREAD_CREATE_FLAGS_HIDE_FROM_DEBUGGER;
	HANDLE hThread	= nullptr;
		
	NTSTATUS ntRet = p_NtCreateThreadEx(&hThread, THREAD_ALL_ACCESS, nullptr, hTargetProc, CloakThread ? MPTR(pEntrypoint) : MPTR(pRoutine), MPTR(pArg), CloakThread ? Flags : NULL, 0, 0, 0, nullptr);
		
	if (NT_FAIL(ntRet) || !hThread)
	{
		LastWin32Error = ntRet;
		
		return SR_NTCTE_ERR_NTCTE_FAIL;
	}

	if (CloakThread)
	{
		WOW64_CONTEXT ctx{ 0 };
		ctx.ContextFlags = CONTEXT_INTEGER;

		if (!Wow64GetThreadContext(hThread, &ctx))
		{
			LastWin32Error = GetLastError();
			
			TerminateThread(hThread, 0);
			CloseHandle(hThread);

			return SR_NTCTE_ERR_GET_CONTEXT_FAIL;
		}

		ctx.Eax = pRoutine;

		if (!Wow64SetThreadContext(hThread, &ctx))
		{
			LastWin32Error = GetLastError();
			
			TerminateThread(hThread, 0);
			CloseHandle(hThread);

			return SR_NTCTE_ERR_SET_CONTEXT_FAIL;
		}

		if (ResumeThread(hThread) == (DWORD)-1)
		{
			LastWin32Error = GetLastError();
			
			TerminateThread(hThread, 0);
			CloseHandle(hThread);

			return SR_NTCTE_ERR_RESUME_FAIL;
		}
	}
	
	Sleep(200);

	DWORD dwWaitRet = WaitForSingleObject(hThread, SR_REMOTE_TIMEOUT);
	if (dwWaitRet != WAIT_OBJECT_0)
	{
		LastWin32Error = GetLastError();

		TerminateThread(hThread, 0);
		CloseHandle(hThread);

		return SR_NTCTE_ERR_TIMEOUT;
	}

	BOOL bRet = GetExitCodeThread(hThread, &Out);
	if (!bRet)
	{
		LastWin32Error = GetLastError();
		
		CloseHandle(hThread);

		return SR_NTCTE_ERR_GECT_FAIL;
	}

	CloseHandle(hThread);

	return SR_ERR_SUCCESS;
}

DWORD SR_HijackThread_WOW64(HANDLE hTargetProc, f_Routine_WOW64 pRoutine, DWORD pArg, DWORD & LastWin32Error, DWORD & Out)
{
	ProcessInfo PI;
	if (!PI.SetProcess(hTargetProc))
	{
		return SR_HT_ERR_PROC_INFO_FAIL;
	}

	HINSTANCE hRemoteNTDll = GetModuleHandleEx_WOW64(hTargetProc, TEXT("ntdll.dll"));
	MODULEINFO mi{ 0 };
	GetModuleInformation(hTargetProc, hRemoteNTDll, &mi, sizeof(mi));
	BYTE * pRemoteNTDll = ReCa<BYTE*>(hRemoteNTDll);

	DWORD ThreadID = 0;
	do
	{
		KWAIT_REASON reason;
		THREAD_STATE state;
		PI.GetThreadState(state, reason);

		if (reason != KWAIT_REASON::WrQueue || state == THREAD_STATE::Running)
		{
			if (pRemoteNTDll)
			{
				void * pStartAddress = nullptr;
				bool bStartAddress = PI.GetThreadStartAddress(pStartAddress);
				if (bStartAddress && pStartAddress)
				{
					if ((ReCa<BYTE*>(pStartAddress) < pRemoteNTDll) || (ReCa<BYTE*>(pStartAddress) >= pRemoteNTDll + mi.SizeOfImage))
					{
						ThreadID = PI.GetThreadId();
						break;
					}
				}
			}

			ThreadID = PI.GetThreadId();
			break;
		}

	} while (PI.NextThread());

	if (!ThreadID)
	{
		return SR_HT_ERR_NO_THREADS;
	}

	HANDLE hThread = OpenThread(THREAD_SET_CONTEXT | THREAD_GET_CONTEXT | THREAD_SUSPEND_RESUME, FALSE, ThreadID);
	if (!hThread)
	{
		LastWin32Error = GetLastError();

		return SR_HT_ERR_OPEN_THREAD_FAIL;
	}

	if (SuspendThread(hThread) == (DWORD)-1)
	{
		LastWin32Error = GetLastError();

		CloseHandle(hThread);

		return SR_HT_ERR_SUSPEND_FAIL;
	}

	WOW64_CONTEXT OldContext{ 0 };
	OldContext.ContextFlags = CONTEXT_CONTROL;
	
	if (!Wow64GetThreadContext(hThread, &OldContext))
	{
		LastWin32Error = GetLastError();

		ResumeThread(hThread);
		CloseHandle(hThread);

		return SR_HT_ERR_GET_CONTEXT_FAIL;
	}

	void * pCodecave = VirtualAllocEx(hTargetProc, nullptr, 0x100, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (!pCodecave)
	{
		LastWin32Error = GetLastError();

		CloseHandle(hThread);

		return SR_HT_ERR_CANT_ALLOC_MEM;
	}

	BYTE Shellcode[] =
	{
		0x00, 0x00, 0x00, 0x00,						// - 0x04 (pCodecave)	-> returned value							;buffer to store returned value (eax)

		0x83, 0xEC, 0x04,							// + 0x00				-> sub esp, 0x04							;prepare stack for ret
		0xC7, 0x04, 0x24, 0x00, 0x00, 0x00, 0x00,	// + 0x03 (+ 0x06)		-> mov [esp], OldEip						;store old eip as return address

		0x50, 0x51, 0x52,							// + 0x0A				-> psuh e(a/c/d)							;save e(a/c/d)x
		0x9C,										// + 0x0D				-> pushfd									;save flags register

		0xB9, 0x00, 0x00, 0x00, 0x00,				// + 0x0E (+ 0x0F)		-> mov ecx, pArg							;load pArg into ecx
		0xB8, 0x00, 0x00, 0x00, 0x00,				// + 0x13 (+ 0x14)		-> mov eax, pRoutine

		0x51,										// + 0x18				-> push ecx									;push pArg
		0xFF, 0xD0,									// + 0x19				-> call eax									;call target function

		0xA3, 0x00, 0x00, 0x00, 0x00,				// + 0x1B (+ 0x1C)		-> mov dword ptr[pCodecave], eax			;store returned value
		
		0x9D,										// + 0x20				-> popfd									;restore flags register
		0x5A, 0x59, 0x58,							// + 0x21				-> pop e(d/c/a)								;restore e(d/c/a)x
		
		0xC6, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00,	// + 0x24 (+ 0x26)		-> mov byte ptr[pCodecave + 0x06], 0x00		;set checkbyte to 0

		0xC3										// + 0x2B				-> ret										;return to OldEip
	}; // SIZE = 0x2C (+ 0x04)

	DWORD FuncOffset		= 0x04;
	DWORD CheckByteOffset	= 0x02 + FuncOffset;
	
	*ReCa<DWORD*>(Shellcode + 0x06 + FuncOffset) = OldContext.Eip;
	*ReCa<DWORD*>(Shellcode + 0x0F + FuncOffset) = pArg;
	*ReCa<DWORD*>(Shellcode + 0x14 + FuncOffset) = pRoutine;
	*ReCa<DWORD*>(Shellcode + 0x1C + FuncOffset) = MDWD(pCodecave);
	*ReCa<DWORD*>(Shellcode + 0x26 + FuncOffset) = MDWD(pCodecave) + CheckByteOffset;

	OldContext.Eip = MDWD(pCodecave) + FuncOffset;

	if (!WriteProcessMemory(hTargetProc, pCodecave, Shellcode, sizeof(Shellcode), nullptr))
	{	
		LastWin32Error = GetLastError();

		ResumeThread(hThread);
		CloseHandle(hThread);
		VirtualFreeEx(hTargetProc, pCodecave, 0, MEM_RELEASE);

		return SR_HT_ERR_WPM_FAIL;
	}

	if (!Wow64SetThreadContext(hThread, &OldContext))
	{		
		LastWin32Error = GetLastError();

		ResumeThread(hThread);
		CloseHandle(hThread);
		VirtualFreeEx(hTargetProc, pCodecave, 0, MEM_RELEASE);

		return SR_HT_ERR_SET_CONTEXT_FAIL;
	}

	PostThreadMessage(ThreadID, 0, 0, 0);

	if (ResumeThread(hThread) == (DWORD)-1)
	{
		LastWin32Error = GetLastError();

		CloseHandle(hThread);
		VirtualFreeEx(hTargetProc, pCodecave, 0, MEM_RELEASE);

		return SR_HT_ERR_RESUME_FAIL;
	}

	CloseHandle(hThread);

	ULONGLONG Timer	= GetTickCount64();
	BYTE CheckByte	= 1;
	
	do
	{
		ReadProcessMemory(hTargetProc, ReCa<BYTE*>(pCodecave) + CheckByteOffset, &CheckByte, 1, nullptr);

		if (GetTickCount64() - Timer > SR_REMOTE_TIMEOUT)
		{
			return SR_HT_ERR_TIMEOUT;
		}

		Sleep(10);

	} while (CheckByte != 0);

	ReadProcessMemory(hTargetProc, pCodecave, &Out, sizeof(Out), nullptr);

	VirtualFreeEx(hTargetProc, pCodecave, 0, MEM_RELEASE);

	return SR_ERR_SUCCESS;
}

DWORD SR_SetWindowsHookEx_WOW64(HANDLE hTargetProc, f_Routine_WOW64 pRoutine, DWORD pArg, DWORD & LastWin32Error, ULONG TargetSessionId, DWORD & Out)
{
	wchar_t RootPath[MAX_PATH * 2]{ 0 };
	if (!GetOwnModulePath(RootPath, sizeof(RootPath) / sizeof(RootPath[0])))
	{
		return SR_SWHEX_ERR_CANT_QUERY_INFO_PATH;
	}

	wchar_t InfoPath[sizeof(RootPath) / sizeof(RootPath[0])]{ 0 };
	memcpy(InfoPath, RootPath, sizeof(RootPath));
	StringCbCatW(InfoPath, sizeof(InfoPath), SWHEX_INFO_FILENAME86);

	if (FileExists(InfoPath))
		DeleteFileW(InfoPath);

	std::wofstream swhex_info(InfoPath, std::ios_base::out | std::ios_base::app);
	if (!swhex_info.good())
	{
		swhex_info.close();
		return SR_SWHEX_ERR_CANT_OPEN_INFO_TXT;
	}

	void * pCodecave = VirtualAllocEx(hTargetProc, nullptr, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (!pCodecave)
	{
		LastWin32Error = GetLastError();

		swhex_info.close();

		return SR_SWHEX_ERR_VAE_FAIL;
	}

	void * pCallNextHookEx = nullptr;
	GetProcAddressEx_WOW64(hTargetProc, TEXT("user32.dll"), "CallNextHookEx", pCallNextHookEx);
	if (!pCallNextHookEx)
	{
		VirtualFreeEx(hTargetProc, pCodecave, 0, MEM_RELEASE);

		return SR_SWHEX_ERR_CNHEX_MISSING;
	}
		
	BYTE Shellcode[] =
	{
		0x00, 0x00, 0x00, 0x00,			// - 0x08				-> pArg						;pointer to argument
		0x00, 0x00, 0x00, 0x00,			// - 0x04				-> pRoutine					;pointer to target function

		0x55,							// + 0x00				-> push ebp					;x86 stack frame creation
		0x8B, 0xEC,						// + 0x01				-> mov ebp, esp

		0xFF, 0x75, 0x10,				// + 0x03				-> push [ebp + 0x10]		;push CallNextHookEx arguments
		0xFF, 0x75, 0x0C,				// + 0x06				-> push [ebp + 0x0C] 
		0xFF, 0x75, 0x08, 				// + 0x09				-> push [ebp + 0x08]
		0x6A, 0x00,						// + 0x0C				-> push 0x00
		0xE8, 0x00, 0x00, 0x00, 0x00,	// + 0x0E (+ 0x0F)		-> call CallNextHookEx		;call CallNextHookEx

		0xEB, 0x00,						// + 0x13				-> jmp $ + 0x02				;jmp to next instruction

		0x50,							// + 0x15				-> push eax					;save eax (CallNextHookEx retval)
		0x53,							// + 0x16				-> push ebx					;save ebx (non volatile)

		0xBB, 0x00, 0x00, 0x00, 0x00,	// + 0x17 (+ 0x18)		-> mov ebx, pArg			;move pArg (pCodecave) into ebx
		0xC6, 0x43, 0x1C, 0x14,			// + 0x1C				-> mov [ebx + 0x1C], 0x17	;hotpatch jmp above to skip shellcode

		0xFF, 0x33,						// + 0x20				-> push [ebx] (default)		;push pArg (__cdecl/__stdcall)

		0xFF, 0x53, 0x04,				// + 0x22				-> call [ebx + 0x04]		;call target function

		0x89, 0x03,						// + 0x25				-> mov [ebx], eax			;store returned value

		0x5B,							// + 0x27				-> pop ebx					;restore old ebx
		0x58,							// + 0x28				-> pop eax					;restore eax (CallNextHookEx retval)

		0x5D,							// + 0x29				-> pop ebp					;restore ebp
		0xC2, 0x0C, 0x00				// + 0x2A				-> ret 0x000C				;return
	}; // SIZE = 0x3D (+ 0x08)

	DWORD CodeOffset		= 0x08;
	DWORD CheckByteOffset	= 0x14 + CodeOffset;

	*ReCa<DWORD*>(Shellcode + 0x00) = pArg;
	*ReCa<DWORD*>(Shellcode + 0x04) = pRoutine;
	
	*ReCa<DWORD*>(Shellcode + 0x0F + CodeOffset) = MDWD(pCallNextHookEx) - (MDWD(pCodecave) + 0x0E + CodeOffset) - 5;
	*ReCa<DWORD*>(Shellcode + 0x18 + CodeOffset) = MDWD(pCodecave);

	if (!WriteProcessMemory(hTargetProc, pCodecave, Shellcode, sizeof(Shellcode), nullptr))
	{
		LastWin32Error = GetLastError();

		VirtualFreeEx(hTargetProc, pCodecave, 0, MEM_RELEASE);
		swhex_info.close();

		return SR_SWHEX_ERR_WPM_FAIL;
	}

	swhex_info << std::dec << GetProcessId(hTargetProc) << '!' << std::hex << MDWD(pCodecave) + CodeOffset << std::endl;
	swhex_info.close();

	StringCbCatW(RootPath, sizeof(RootPath), SWHEX_EXE_FILENAME86);
	
	PROCESS_INFORMATION pi{ 0 };
	STARTUPINFOW si{ 0 };
	si.cb			= sizeof(si);
	si.dwFlags		= STARTF_USESHOWWINDOW;
	si.wShowWindow	= SW_HIDE;

	if (TargetSessionId != -1) 
	{
		HANDLE hUserToken = nullptr;
		if (!WTSQueryUserToken(TargetSessionId, &hUserToken))
		{
			LastWin32Error = GetLastError();

			VirtualFreeEx(hTargetProc, pCodecave, 0, MEM_RELEASE);

			return SR_SWHEX_ERR_WTSQUERY_FAIL;
		}

		HANDLE hNewToken = nullptr;
		if (!DuplicateTokenEx(hUserToken, MAXIMUM_ALLOWED, nullptr, SecurityIdentification, TokenPrimary, &hNewToken))
		{
			LastWin32Error = GetLastError();

			CloseHandle(hUserToken);
			VirtualFreeEx(hTargetProc, pCodecave, 0, MEM_RELEASE);

			return SR_SWHEX_ERR_DUP_TOKEN_FAIL;
		}

		DWORD SizeOut = 0;
		TOKEN_LINKED_TOKEN admin_token{ 0 };
		if (!GetTokenInformation(hNewToken, TokenLinkedToken, &admin_token, sizeof(admin_token), &SizeOut))
		{
			LastWin32Error = GetLastError();

			CloseHandle(hNewToken);
			CloseHandle(hUserToken);
			VirtualFreeEx(hTargetProc, pCodecave, 0, MEM_RELEASE);

			return SR_SWHEX_ERR_GET_ADMIN_TOKEN_FAIL;
		}
		HANDLE hAdminToken = admin_token.LinkedToken;

		if (!CreateProcessAsUserW(hAdminToken, RootPath, nullptr, nullptr, nullptr, FALSE, CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi))
		{
			LastWin32Error = GetLastError();
			
			CloseHandle(hAdminToken);
			CloseHandle(hNewToken);
			CloseHandle(hUserToken);
			VirtualFreeEx(hTargetProc, pCodecave, 0, MEM_RELEASE);

			return SR_SWHEX_ERR_CANT_CREATE_PROCESS;
		}
		
		CloseHandle(hAdminToken);
		CloseHandle(hNewToken);
		CloseHandle(hUserToken);
	}
	else
	{
		if (!CreateProcessW(RootPath, nullptr, nullptr, nullptr, FALSE, CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi))
		{
			LastWin32Error = GetLastError();
			
			VirtualFreeEx(hTargetProc, pCodecave, 0, MEM_RELEASE);

			return SR_SWHEX_ERR_CANT_CREATE_PROCESS;
		}
	}
	
	DWORD dwWaitRet = WaitForSingleObject(pi.hProcess, SR_REMOTE_TIMEOUT);
	if (dwWaitRet != WAIT_OBJECT_0)
	{
		LastWin32Error = GetLastError();
	
		TerminateProcess(pi.hProcess, 0);

		return SR_SWHEX_ERR_SWHEX_TIMEOUT;
	}

	DWORD ExitCode = 0;
	GetExitCodeProcess(pi.hProcess, &ExitCode);

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	if (ExitCode != SWHEX_ERR_SUCCESS)
	{
		VirtualFreeEx(hTargetProc, pCodecave, 0, MEM_RELEASE);

		return ExitCode;
	}

	ULONGLONG TimeOut	= GetTickCount64();
	BYTE CheckByte		= 0;
	while (!CheckByte)
	{
		ReadProcessMemory(hTargetProc, ReCa<BYTE*>(pCodecave) + CheckByteOffset, &CheckByte, 1, nullptr);
		Sleep(10);
		if (GetTickCount64() - TimeOut > SR_REMOTE_TIMEOUT)
			return SR_SWHEX_ERR_REMOTE_TIMEOUT;
	}

	ReadProcessMemory(hTargetProc, pCodecave, &Out, sizeof(Out), nullptr);
	
	VirtualFreeEx(hTargetProc, pCodecave, 0, MEM_RELEASE);

	return SR_ERR_SUCCESS;
}

DWORD SR_QueueUserAPC_WOW64(HANDLE hTargetProc, f_Routine_WOW64 pRoutine, DWORD pArg, DWORD & LastWin32Error, DWORD & Out)
{
	HINSTANCE h_nt_dll = GetModuleHandleA("ntdll.dll");
	if (!h_nt_dll)
	{
		LastWin32Error = GetLastError();

		return SR_QUAPC_ERR_GET_MODULE_HANDLE_FAIL;
	}

	auto p_RtlQueueApcWow64Thread = reinterpret_cast<f_RtlQueueApcWow64Thread>(GetProcAddress(h_nt_dll, "RtlQueueApcWow64Thread"));
	if (!p_RtlQueueApcWow64Thread)
	{
		LastWin32Error = GetLastError();
		return SR_QUAPC_ERR_RTLQAW64_MISSING;
	}

	void * pCodecave = VirtualAllocEx(hTargetProc, nullptr, 0x100, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (!pCodecave)
	{
		LastWin32Error = GetLastError();

		return SR_QUAPC_ERR_CANT_ALLOC_MEM;
	}

	BYTE Shellcode[] =
	{
		0x00, 0x00, 0x00, 0x00, // - 0x0C	-> returned value					;buffer to store returned value
		0x00, 0x00, 0x00, 0x00, // - 0x08	-> pArg								;buffer to store argument
		0x00, 0x00, 0x00, 0x00, // - 0x04	-> pRoutine							;pointer to the routine to call

		0x55,					// + 0x00	-> push ebp							;x86 stack frame creation
		0x8B, 0xEC,				// + 0x01	-> mov ebp, esp

		0xEB, 0x00,				// + 0x03	-> jmp pCodecave + 0x05 (+ 0x0C)	;jump to next instruction

		0x53,					// + 0x05	-> push ebx							;save ebx
		0x8B, 0x5D, 0x08,		// + 0x06	-> mov ebx, [ebp + 0x08]			;move pCodecave into ebx (non volatile)

		0xFF, 0x73, 0x04,		// + 0x09	-> push [ebx + 0x04]				;push pArg on stack
		0xFF, 0x53, 0x08,		// + 0x0C	-> call dword ptr[ebx + 0x08]		;call pRoutine

		0x85, 0xC0,				// + 0x0F	-> test eax, eax					;check if eax indicates success/failure
		0x74, 0x06,				// + 0x11	-> je pCodecave + 0x19 (+ 0x0C)		;jmp to cleanup if routine failed

		0x89, 0x03,				// + 0x13	-> mov [ebx], eax					;store returned value
		0xC6, 0x43, 0x10, 0x15, // + 0x15	-> mov byte ptr [ebx + 0x10], 0x15	;hot patch jump to skip shellcode

		0x5B,					// + 0x19	-> pop ebx							;restore old ebx

		0x5D,					// + 0x1A	-> pop ebp							;restore ebp
		0xC2, 0x04, 0x00		// + 0x1B	-> ret 0x0004						;return
	}; // SIZE = 0x1E (+ 0x0C)

	DWORD CodeOffset = 0x0C;

	*ReCa<DWORD*>(Shellcode + 0x04) = pArg;
	*ReCa<DWORD*>(Shellcode + 0x08) = pRoutine;

	BOOL bRet = WriteProcessMemory(hTargetProc, pCodecave, Shellcode, sizeof(Shellcode), nullptr);
	if (!bRet)
	{
		LastWin32Error = GetLastError();
		VirtualFreeEx(hTargetProc, pCodecave, 0, MEM_RELEASE);

		return SR_QUAPC_ERR_WPM_FAIL;
	}

	ProcessInfo PI;
	if (!PI.SetProcess(hTargetProc))
	{
		return SR_HT_ERR_PROC_INFO_FAIL;
	}

	bool APCQueued = false;
	PAPCFUNC pShellcode = (PAPCFUNC)((ULONG_PTR)(MDWD((ULONG_PTR)pCodecave + CodeOffset)));

	do
	{
		KWAIT_REASON reason;
		THREAD_STATE state;
		if (!PI.GetThreadState(state, reason))
			continue;

		if (reason == KWAIT_REASON::WrQueue)
			continue;

		DWORD ThreadId = PI.GetTID();
		if (!ThreadId)
			continue;

		HANDLE hThread = OpenThread(THREAD_SET_CONTEXT, FALSE, ThreadId);
		if (!hThread)
			continue;

		if (NT_SUCCESS(p_RtlQueueApcWow64Thread(hThread, pShellcode, pCodecave, nullptr, nullptr)))
		{
			APCQueued = true;
			PostThreadMessageW(ThreadId, WM_NULL, 0, 0);
		}
		else
		{
			LastWin32Error = GetLastError();
		}

		CloseHandle(hThread);

	} while (PI.NextThread());

	if (!APCQueued)
	{
		VirtualFreeEx(hTargetProc, pCodecave, 0, MEM_RELEASE);

		return SR_QUAPC_ERR_NO_APC_THREAD;
	}
	else
	{
		LastWin32Error = 0;
	}

	
	ULONGLONG Timer = GetTickCount64();
	Out = 0;

	do
	{
		ReadProcessMemory(hTargetProc, pCodecave, &Out, sizeof(Out), nullptr);

		if (GetTickCount64() - Timer > SR_REMOTE_TIMEOUT)
		{
			return SR_QUAPC_ERR_TIMEOUT;
		}

		Sleep(10);
	} while (!Out);

	return SR_ERR_SUCCESS;
}

#endif