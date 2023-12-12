#pragma once

/// ###############	##########		##########		     #######	   ##########			###			###
/// ###############	############	############	  ####     ####    ############			###			###
/// ###				###        ###	###        ###	 ###         ###   ###        ###		###			###
/// ###				###        ###	###        ###	###           ###  ###        ###		###			###
/// ###				###       ###	###       ###	###           ###  ###       ###		###			###
/// ###############	###########		###########		###           ###  ###########			###############
/// ###############	###########		########### 	###			  ###  ###########			###############
/// ###				###      ###	###		###     ###			  ###  ###		###			###			###
/// ###				###		  ###	###		  ###	###           ###  ###		  ###		###			###
/// ###				###		   ###	###		   ###	 ###         ###   ###		   ###	 #	###			###
/// ###############	###		   ###	###		   ###	  ####     ####    ###		   ###  ###	###			###
/// ###############	###        ###	###        ###	     #######	   ###         ###   #	###			###

//Injection errors:
#define INJ_ERR_SUCCESS					0x00000000
													
													//Source					:	error description

#define INJ_ERR_INVALID_PROC_HANDLE		0x00000001	//GetHandleInformation		:	win32 error
#define INJ_ERR_FILE_DOESNT_EXIST		0x00000002	//GetFileAttributesW		:	win32 error
#define INJ_ERR_OUT_OF_MEMORY_EXT		0x00000003	//VirtualAllocEx			:	win32 error
#define INJ_ERR_OUT_OF_MEMORY_INT		0x00000004	//VirtualAlloc				:	win32 error
#define INJ_ERR_IMAGE_CANT_RELOC		0x00000005	//internal error			:	base relocation directory empty
#define INJ_ERR_LDRLOADDLL_MISSING		0x00000006	//GetProcAddressEx			:	can't find pointer to LdrLoadDll
#define INJ_ERR_REMOTEFUNC_MISSING		0x00000007	//LoadFunctionPointer		:	can't find remote function
#define INJ_ERR_CANT_FIND_MOD_PEB		0x00000008	//internal error			:	module not linked to PEB
#define INJ_ERR_WPM_FAIL				0x00000009	//WriteProcessMemory		:	win32 error
#define INJ_ERR_CANT_ACCESS_PEB			0x0000000A	//ReadProcessMemory			:	win32 error
#define INJ_ERR_CANT_ACCESS_PEB_LDR		0x0000000B	//ReadProcessMemory			:	win32 error
#define INJ_ERR_VPE_FAIL				0x0000000C	//VirtualProtectEx			:	win32 error
#define INJ_ERR_CANT_ALLOC_MEM			0x0000000D	//VirtualAllocEx			:	win32 error
#define INJ_ERR_CT32S_FAIL				0x0000000E	//CreateToolhelp32Snapshot	:	win32 error
#define	INJ_ERR_RPM_FAIL				0x0000000F	//ReadProcessMemory			:	win32 error
#define INJ_ERR_INVALID_PID				0x00000010	//internal error			:	process id is 0
#define INJ_ERR_INVALID_FILEPATH		0x00000011	//internal error			:	INJECTIONDATA::szDllPath is nullptr
#define INJ_ERR_CANT_OPEN_PROCESS		0x00000012	//OpenProcess				:	win32 error
#define INJ_ERR_PLATFORM_MISMATCH		0x00000013	//internal error			:	file error (0x20000001 - 0x20000003, check below)
#define INJ_ERR_NO_HANDLES				0x00000014	//internal error			:	no process handle to hijack
#define INJ_ERR_HIJACK_NO_NATIVE_HANDLE	0x00000015	//internal error			:	no compatible process handle to hijack
#define INJ_ERR_HIJACK_INJ_FAILED		0x00000016	//internal error			:	injecting injection module into handle owner process failed, additional errolog(s) created
#define INJ_ERR_HIJACK_CANT_ALLOC		0x00000017	//VirtualAllocEx			:	win32 error
#define INJ_ERR_HIJACK_CANT_WPM			0x00000018	//WriteProcessMemory		:	win32 error
#define INJ_ERR_HIJACK_INJMOD_MISSING	0x00000019	//internal error			:	can't find remote injection module
#define INJ_ERR_HIJACK_INJECTW_MISSING	0x0000001A	//internal error			:	can't find remote injection function
#define INJ_ERR_GET_MODULE_HANDLE_FAIL	0x0000001B	//GetModuleHandleA			:	win32 error
#define INJ_ERR_OUT_OF_MEMORY_NEW		0x0000001C	//operator new				:	internal memory allocation failed
#define INJ_ERR_REMOTE_CODE_FAILED		0x0000001D	//internal error			:	the remote code wasn't able to load the module

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//Start Routine errors:
#define SR_ERR_SUCCESS					0x00000000
													
													//Source					:	error description

#define SR_ERR_CANT_QUERY_SESSION_ID	0x10000001	//NtQueryInformationProcess	:	NTSTATUS
#define SR_ERR_INVALID_LAUNCH_METHOD	0x10000002	//nigga, u for real?


///////////////////
///NtCreateThreadEx
													//Source					:	error description

#define SR_NTCTE_ERR_NTCTE_MISSING			0x10100001	//GetProcAddress			:	win32 error
#define SR_NTCTE_ERR_CANT_ALLOC_MEM			0x10100002	//VirtualAllocEx			:	win32 error
#define SR_NTCTE_ERR_WPM_FAIL				0x10100003	//WriteProcessMemory		:	win32 error
#define SR_NTCTE_ERR_NTCTE_FAIL				0x10100004	//NtCreateThreadEx			:	NTSTATUS
#define SR_NTCTE_ERR_GET_CONTEXT_FAIL		0x10100005	//(Wow64)GetThreadContext	:	win32 error
#define SR_NTCTE_ERR_SET_CONTEXT_FAIL		0x10100006	//(Wow64)SetThreadContext	:	win32 error
#define SR_NTCTE_ERR_RESUME_FAIL			0x10100007	//ResumeThread				:	win32 error
#define SR_NTCTE_ERR_RPM_FAIL				0x10100008	//ReadProcessMemory			:	win32 error
#define SR_NTCTE_ERR_TIMEOUT				0x10100009	//WaitForSingleObject		:	win32 error
#define SR_NTCTE_ERR_GECT_FAIL				0x1010000A	//GetExitCodeThread			:	win32 error
#define SR_NTCTE_ERR_GET_MODULE_HANDLE_FAIL	0x1010000B	//GetModuleHandle			:	win32 error

///////////////
///HijackThread
												//Source					:	error description

#define SR_HT_ERR_PROC_INFO_FAIL	0x10200001	//internal error			:	can't grab process information
#define SR_HT_ERR_NO_THREADS		0x10200002	//internal error			:	no thread to hijack
#define SR_HT_ERR_OPEN_THREAD_FAIL	0x10200003	//OpenThread				:	win32 error
#define SR_HT_ERR_CANT_ALLOC_MEM	0x10200004	//VirtualAllocEx			:	win32 error
#define SR_HT_ERR_SUSPEND_FAIL		0x10200005	//SuspendThread				:	win32 error
#define SR_HT_ERR_GET_CONTEXT_FAIL	0x10200006	//(Wow64)GetThreadContext	:	win32 error
#define SR_HT_ERR_WPM_FAIL			0x10200007	//WriteProcessMemory		:	win32 error
#define SR_HT_ERR_SET_CONTEXT_FAIL	0x10200008	//(Wow64)SetThreadContext	:	win32 error
#define SR_HT_ERR_RESUME_FAIL		0x10200009	//ResumeThread				:	win32 error
#define SR_HT_ERR_TIMEOUT			0x1020000A	//internal error			:	execution time exceeded SR_REMOTE_TIMEOUT

////////////////////
///SetWindowsHookEx
														//Source					:	error description

#define SR_SWHEX_ERR_CANT_QUERY_INFO_PATH	0x10300001	//internal error		:	can't resolve own module filepath
#define SR_SWHEX_ERR_CANT_OPEN_INFO_TXT		0x10300002	//internal error		:	can't open swhex info file
#define SR_SWHEX_ERR_VAE_FAIL				0x10300003	//VirtualAllocEx		:	win32 error
#define SR_SWHEX_ERR_CNHEX_MISSING			0x10300004	//GetProcAddressEx		:	can't find pointer to CallNextHookEx
#define SR_SWHEX_ERR_WPM_FAIL				0x10300005	//WriteProcessMemory	:	win32 error
#define SR_SWHEX_ERR_WTSQUERY_FAIL			0x10300006	//WTSQueryUserToken		:	win32 error
#define SR_SWHEX_ERR_DUP_TOKEN_FAIL			0x10300007	//DuplicateTokenEx		:	win32 error
#define SR_SWHEX_ERR_GET_ADMIN_TOKEN_FAIL	0x10300008	//GetTokenInformation	:	win32 error
#define SR_SWHEX_ERR_CANT_CREATE_PROCESS	0x10300009	//CreateProcessAsUserW	:	win32 error
														//CreateProcessW
#define SR_SWHEX_ERR_SWHEX_TIMEOUT			0x1030000A	//WaitForSingleObject	:	win32 error
#define SR_SWHEX_ERR_REMOTE_TIMEOUT			0x1030000B	//internal error		:	execution time exceeded SR_REMOTE_TIMEOUT

///////////////
///QueueUserAPC
														//Source					:	error description

#define SR_QUAPC_ERR_RTLQAW64_MISSING		0x10400001	//GetProcAddress			:	win32 error
#define SR_QUAPC_ERR_CANT_ALLOC_MEM			0x10400001	//VirtualAllocEx			:	win32 error
#define SR_QUAPC_ERR_WPM_FAIL				0x10400002	//WriteProcessMemory		:	win32 error
#define SR_QUAPC_ERR_TH32_FAIL				0x10400003	//CreateToolhelp32Snapshot	:	win32 error
#define SR_QUAPC_ERR_T32FIRST_FAIL			0x10400004	//Thread32First				:	win32 error
#define SR_QUAPC_ERR_NO_APC_THREAD			0x10400005	//QueueUserAPC				:	win32 error
#define SR_QUAPC_ERR_TIMEOUT				0x10400006	//internal error			:	execution time exceeded SR_REMOTE_TIMEOUT
#define SR_QUAPC_ERR_GET_MODULE_HANDLE_FAIL	0x10100007	//GetModuleHandle			:	win32 error


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//File errors:
#define FILE_ERR_SUCCESS			0x00000000

												//Source				:	error description
#define FILE_ERR_CANT_OPEN_FILE		0x20000001	//std::ifstream::good	:	openening the file failed
#define FILE_ERR_INVALID_FILE_SIZE	0x20000002	//internal error		:	file isn't a valid PE
#define FILE_ERR_INVALID_FILE		0x20000003	//internal error		:	PE isn't compatible with the injection settings


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//SWHEX - XX.exe errors:
#define SWHEX_ERR_SUCCESS			0x00000000

												//Source				:	error description

#define SWHEX_ERR_INVALID_PATH		0x30000001	//StringCchLengthW		:	path exceeds MAX_PATH * 2 chars
#define SWHEX_ERR_CANT_OPEN_FILE	0x30000002	//std::ifstream::good	:	openening the file failed
#define SWHEX_ERR_EMPTY_FILE		0x30000003	//internal error		:	file is empty
#define SWHEX_ERR_INVALID_INFO		0x30000004	//internal error		:	provided info is wrong / invalid
#define SWHEX_ERR_ENUM_WINDOWS_FAIL 0x30000005	//EnumWindows			:	API fail
#define SWHEX_ERR_NO_WINDOWS		0x30000006	//internal error		:	no compatible window found