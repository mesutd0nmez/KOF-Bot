#include "pch.h"

#include "Manul Mapping.h"
#pragma comment (lib, "Psapi.lib")

HINSTANCE __stdcall ManualMapping_Shell(MANUAL_MAPPING_DATA	* pData);
DWORD ManualMapping_Shell_End();

DWORD MANUAL_MAPPER::AllocateMemory(DWORD & LastWin32Error)
{
	pLocalImageBase = (BYTE*)VirtualAlloc(nullptr, ImageSize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (!pLocalImageBase)
	{
		LastWin32Error = GetLastError();

		return INJ_ERR_OUT_OF_MEMORY_INT;
	}

	if (Flags & INJ_SHIFT_MODULE)
	{
		srand(GetTickCount64() & 0xFFFFFFFF);
		ShiftOffset = ALIGN_UP(rand() % 0x1000 + 0x100, 0x10);
	}

	DWORD ShellcodeSize = (DWORD)((ULONG_PTR)ManualMapping_Shell_End - (ULONG_PTR)ManualMapping_Shell);

	AllocationSize = ShiftOffset + ImageSize + sizeof(MANUAL_MAPPING_DATA) + ShellcodeSize;

	if(Flags & INJ_SHIFT_MODULE)
	{
		pAllocationBase = (BYTE*)VirtualAllocEx(hTargetProcess, nullptr, AllocationSize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
		if (!pAllocationBase)
		{
			LastWin32Error = GetLastError();

			VirtualFree(pLocalImageBase, 0, MEM_RELEASE);

			return INJ_ERR_OUT_OF_MEMORY_EXT;
		}
	}
	else
	{
		pAllocationBase = (BYTE*)VirtualAllocEx(hTargetProcess, (void*)pLocalOptionalHeader->ImageBase, AllocationSize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
		if (!pAllocationBase)
		{
			pAllocationBase = (BYTE*)VirtualAllocEx(hTargetProcess, nullptr, AllocationSize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
			if (!pAllocationBase)
			{
				LastWin32Error = GetLastError();

				VirtualFree(pLocalImageBase, 0, MEM_RELEASE);

				return INJ_ERR_OUT_OF_MEMORY_EXT;
			}
		}
	}

	pTargetImageBase	= pAllocationBase + ShiftOffset;
	pManualMappingData	= pTargetImageBase + ImageSize;
	pShellcode			= pManualMappingData + sizeof(MANUAL_MAPPING_DATA);
	
	return INJ_ERR_SUCCESS;
}

DWORD MANUAL_MAPPER::CopyData(DWORD & LastWin32Error)
{
	memcpy(pLocalImageBase, pRawData, pLocalOptionalHeader->SizeOfHeaders);

	auto * pCurrentSectionHeader = IMAGE_FIRST_SECTION(pLocalNtHeaders);
	for (UINT i = 0; i != pLocalFileHeader->NumberOfSections; ++i, ++pCurrentSectionHeader)
	{
		if (pCurrentSectionHeader->SizeOfRawData)
		{
			memcpy(pLocalImageBase + pCurrentSectionHeader->VirtualAddress, pRawData + pCurrentSectionHeader->PointerToRawData, pCurrentSectionHeader->SizeOfRawData);
		}
	}
	
	if (Flags & INJ_SHIFT_MODULE)
	{
		DWORD* pJunk = new DWORD[ShiftOffset / sizeof(DWORD)];
		DWORD SuperJunk = GetTickCount64() & 0xFFFFFFFF;

		for (UINT i = 0; i < ShiftOffset / sizeof(DWORD) - 1; ++i)
		{
			pJunk[i] = SuperJunk;
			SuperJunk ^= (i << (i % 32));
			SuperJunk -= 0x11111111;
		}

		WriteProcessMemory(hTargetProcess, pAllocationBase, pJunk, ShiftOffset, nullptr);

		delete[] pJunk;
	}

	auto LoadFunctionPointer = [=](HINSTANCE hLib, const char* szFunc, void*& pOut)
	{
		if (!GetProcAddressEx(hTargetProcess, hLib, szFunc, pOut))
		{
			return false;
		}

		return true;
	};

	HINSTANCE hK32 = GetModuleHandleEx(hTargetProcess, TEXT("kernel32.dll"));

	void * pLoadLibraryA = nullptr;
	if (!LoadFunctionPointer(hK32, "LoadLibraryA", pLoadLibraryA))
		return INJ_ERR_REMOTEFUNC_MISSING;

	void * pGetProcAddress = nullptr;
	if (!LoadFunctionPointer(hK32, "GetProcAddress", pGetProcAddress))
		return INJ_ERR_REMOTEFUNC_MISSING;

	MANUAL_MAPPING_DATA data{ 0 };
	data.pLoadLibraryA		= ReCa<f_LoadLibraryA*>(pLoadLibraryA);
	data.pGetProcAddress	= ReCa<f_GetProcAddress>(pGetProcAddress);
	data.pModuleBase		= pTargetImageBase;
	data.Flags				= Flags;


	if (!WriteProcessMemory(hTargetProcess, pManualMappingData, &data, sizeof(data), nullptr))
	{
		LastWin32Error = GetLastError();

		return INJ_ERR_WPM_FAIL;
	}

	DWORD ShellcodeSize = (UINT_PTR)ManualMapping_Shell_End - (UINT_PTR)ManualMapping_Shell;
	if (!WriteProcessMemory(hTargetProcess, pShellcode, ManualMapping_Shell, ShellcodeSize, nullptr))
	{
		LastWin32Error = GetLastError();
	
		return INJ_ERR_WPM_FAIL;
	}

	return INJ_ERR_SUCCESS;
}

DWORD MANUAL_MAPPER::RelocateImage(DWORD & LastWin32Error)
{
	BYTE * LocationDelta = pTargetImageBase - pLocalOptionalHeader->ImageBase;

	if (!LocationDelta)
		return INJ_ERR_SUCCESS;

	if (!pLocalOptionalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size)
		return INJ_ERR_IMAGE_CANT_RELOC;

	auto * pRelocData = ReCa<IMAGE_BASE_RELOCATION*>(pLocalImageBase + pLocalOptionalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress);
	while (pRelocData->VirtualAddress)
	{
		WORD * pRelativeInfo = ReCa<WORD*>(pRelocData + 1);
		for (UINT i = 0; i < ((pRelocData->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD)); ++i, ++pRelativeInfo)
		{
			if (RELOC_FLAG(*pRelativeInfo))
			{
				ULONG_PTR* pPatch = ReCa<ULONG_PTR*>(pLocalImageBase + pRelocData->VirtualAddress + ((*pRelativeInfo) & 0xFFF));
				*pPatch += ReCa<ULONG_PTR>(LocationDelta);
			}
		}
		pRelocData = ReCa<IMAGE_BASE_RELOCATION*>(ReCa<BYTE*>(pRelocData) + pRelocData->SizeOfBlock);
	}
	
	return INJ_ERR_SUCCESS;
}

DWORD MANUAL_MAPPER::CopyImage(DWORD & LastWin32Error)
{
	if (!WriteProcessMemory(hTargetProcess, pTargetImageBase, pLocalImageBase, ImageSize, nullptr))
	{
		LastWin32Error = GetLastError();

		return INJ_ERR_WPM_FAIL;
	}

	return INJ_ERR_SUCCESS;
}

DWORD _ManualMap(const wchar_t * szDllFile, HANDLE hTargetProc, LAUNCH_METHOD Method, DWORD Flags, HINSTANCE & hOut, DWORD & LastWin32Error)
{
	MANUAL_MAPPER Module{ 0 };
	BYTE * pRawData{ nullptr };
	
	std::ifstream File(szDllFile, std::ios::binary | std::ios::ate);

	auto FileSize = File.tellg();

	pRawData = new BYTE[static_cast<size_t>(FileSize)];

	if (!pRawData)
	{
		File.close();

		return INJ_ERR_OUT_OF_MEMORY_NEW;
	}

	File.seekg(0, std::ios::beg);
	File.read(ReCa<char*>(pRawData), FileSize);
	File.close();

	Module.hTargetProcess = hTargetProc;

	Module.pRawData			= pRawData;
	Module.pLocalDosHeader	= ReCa<IMAGE_DOS_HEADER*>(Module.pRawData);
	Module.pLocalNtHeaders	= ReCa<IMAGE_NT_HEADERS*>(Module.pRawData + Module.pLocalDosHeader->e_lfanew);
	Module.pLocalOptionalHeader = &Module.pLocalNtHeaders->OptionalHeader;
	Module.pLocalFileHeader		= &Module.pLocalNtHeaders->FileHeader;
	Module.ImageSize = Module.pLocalOptionalHeader->SizeOfImage;

	Module.Flags = Flags;
	
	DWORD ret = Module.AllocateMemory(LastWin32Error);
	if(ret != INJ_ERR_SUCCESS)
	{
		delete[] pRawData;

		return ret;
	}

	ret = Module.CopyData(LastWin32Error);
	if(ret != INJ_ERR_SUCCESS)
	{
		VirtualFree(Module.pLocalImageBase, 0, MEM_RELEASE);
		VirtualFreeEx(Module.hTargetProcess, Module.pAllocationBase, 0, MEM_RELEASE);
		delete[] pRawData;

		return ret;
	}

	ret = Module.RelocateImage(LastWin32Error);
	if(ret != INJ_ERR_SUCCESS)
	{
		VirtualFree(Module.pLocalImageBase, 0, MEM_RELEASE);
		VirtualFreeEx(Module.hTargetProcess, Module.pAllocationBase, 0, MEM_RELEASE);
		delete[] pRawData;

		return ret;
	}
	
	ret = Module.CopyImage(LastWin32Error);

	VirtualFree(Module.pLocalImageBase, 0, MEM_RELEASE);
	delete[] pRawData;

	if(ret != INJ_ERR_SUCCESS)
	{
		VirtualFreeEx(Module.hTargetProcess, Module.pAllocationBase, 0, MEM_RELEASE);

		return ret;
	}

	ULONG_PTR remote_ret = 0;
	ret = StartRoutine(hTargetProc, ReCa<f_Routine*>(Module.pShellcode), Module.pManualMappingData, Method, (Flags & INJ_THREAD_CREATE_CLOAKED) != 0, LastWin32Error, remote_ret);
	hOut = ReCa<HINSTANCE>(remote_ret);
	
	if (Method != LM_QueueUserAPC)
	{
		auto zero_size = Module.AllocationSize - (Module.pManualMappingData - Module.pAllocationBase);
		BYTE * zero_bytes = new BYTE[zero_size];
		memset(zero_bytes, 0, zero_size);

		WriteProcessMemory(hTargetProc, Module.pManualMappingData, zero_bytes, zero_size, nullptr);

		delete[] zero_bytes;
	}

	if (Flags & INJ_FAKE_HEADER)
	{
		void * pK32 = ReCa<void*>(GetModuleHandleA("kernel32.dll"));
		if (pK32)
		{
			WriteProcessMemory(hTargetProc, Module.pTargetImageBase, pK32, 0x1000, nullptr);
		}
	}
	else if (Flags & INJ_ERASE_HEADER)
	{
		BYTE zero_bytes[0x1000]{ 0 };
		WriteProcessMemory(hTargetProc, Module.pTargetImageBase, zero_bytes, 0x1000, nullptr);
	}

	return ret;
}

HINSTANCE __stdcall ManualMapping_Shell(MANUAL_MAPPING_DATA * pData)
{
	if (!pData || !pData->pLoadLibraryA)
		return NULL;

	BYTE * pBase			= pData->pModuleBase;
	auto * pOp				= &ReCa<IMAGE_NT_HEADERS*>(pBase + ReCa<IMAGE_DOS_HEADER*>(pBase)->e_lfanew)->OptionalHeader;

	auto _GetProcAddress	= pData->pGetProcAddress;
	DWORD _Flags			= pData->Flags;
	auto _DllMain			= ReCa<f_DLL_ENTRY_POINT>(pBase + pOp->AddressOfEntryPoint);

	if (pOp->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size)
	{
		auto * pImportDescr = ReCa<IMAGE_IMPORT_DESCRIPTOR*>(pBase + pOp->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
		while (pImportDescr->Name)
		{
			char * szMod = ReCa<char*>(pBase + pImportDescr->Name);
			HINSTANCE hDll = pData->pLoadLibraryA(szMod);
			
			ULONG_PTR * pThunkRef	= ReCa<ULONG_PTR*>(pBase + pImportDescr->OriginalFirstThunk);
			ULONG_PTR * pFuncRef	= ReCa<ULONG_PTR*>(pBase + pImportDescr->FirstThunk);
		
			if (!pImportDescr->OriginalFirstThunk)
				pThunkRef = pFuncRef;

			for (; *pThunkRef; ++pThunkRef, ++pFuncRef)
			{
				if (IMAGE_SNAP_BY_ORDINAL(*pThunkRef))
				{
					*pFuncRef = _GetProcAddress(hDll, ReCa<char*>(*pThunkRef & 0xFFFF));
				}
				else
				{
					auto * pImport = ReCa<IMAGE_IMPORT_BY_NAME*>(pBase + (*pThunkRef));
					*pFuncRef = _GetProcAddress(hDll, pImport->Name);
				}
			}
			++pImportDescr;
		}
	}

	if (pOp->DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].Size)
	{
		auto * pTLS = ReCa<IMAGE_TLS_DIRECTORY*>(pBase + pOp->DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].VirtualAddress);
		auto * pCallback = ReCa<PIMAGE_TLS_CALLBACK*>(pTLS->AddressOfCallBacks);
		for (; pCallback && (*pCallback); ++pCallback)
		{
			auto Callback = *pCallback;
			Callback(pBase, DLL_PROCESS_ATTACH, nullptr);
		}
	}

	_DllMain(pBase, DLL_PROCESS_ATTACH, nullptr);
	
	if (_Flags & INJ_CLEAN_DATA_DIR)
	{	
		DWORD Size = pOp->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size;
		if (Size)
		{
			auto * pImportDescr = ReCa<IMAGE_IMPORT_DESCRIPTOR*>(pBase + pOp->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
			while (pImportDescr->Name)
			{
				char * szMod = ReCa<char*>(pBase + pImportDescr->Name);
				for (; *szMod++; *szMod = 0);
				pImportDescr->Name = 0;
				
				ULONG_PTR * pThunkRef	= ReCa<ULONG_PTR*>(pBase + pImportDescr->OriginalFirstThunk);
				ULONG_PTR * pFuncRef	= ReCa<ULONG_PTR*>(pBase + pImportDescr->FirstThunk);

				if (!pImportDescr->OriginalFirstThunk)
					pThunkRef = pFuncRef;

				for (; *pThunkRef; ++pThunkRef, ++pFuncRef)
				{
					if (!IMAGE_SNAP_BY_ORDINAL(*pThunkRef))
					{
						auto * pImport = ReCa<IMAGE_IMPORT_BY_NAME*>(pBase + (*pThunkRef));
						char * szFunc = pImport->Name;
						for (; *szFunc++; *szFunc = 0);
					}
					else
					{
						*(WORD*)pThunkRef = 0;
					}
				}

				++pImportDescr;
			}

			pOp->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress = 0;
			pOp->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size			= 0;
		}

		Size = pOp->DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].Size;
		if (Size)
		{
			auto * pIDD = reinterpret_cast<IMAGE_DEBUG_DIRECTORY*>(pBase + pOp->DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress);
			BYTE * pDataa = pBase + pIDD->AddressOfRawData;
			for (UINT i = 0; i != pIDD->SizeOfData; ++i)
			{
				pDataa[i] = 0;
			}
			pIDD->AddressOfRawData	= 0;
			pIDD->PointerToRawData	= 0;
			pIDD->SizeOfData		= 0;
			pIDD->TimeDateStamp		= 0;
			pOp->DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress	= 0;
			pOp->DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].Size			= 0;
		}

		Size = pOp->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size;
		if (Size)
		{
			auto * pRelocData = ReCa<IMAGE_BASE_RELOCATION*>(pBase + pOp->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress);
			while (pRelocData->VirtualAddress)
			{
				pRelocData->VirtualAddress = 0;
				pRelocData = ReCa<IMAGE_BASE_RELOCATION*>(ReCa<BYTE*>(pRelocData) + pRelocData->SizeOfBlock);
			}

			pOp->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress	= 0;
			pOp->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size			= 0;
		}

		Size = pOp->DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].Size;
		if (Size)
		{
			auto * pTLS = ReCa<IMAGE_TLS_DIRECTORY*>(pBase + pOp->DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].VirtualAddress);
			auto * pCallback = ReCa<PIMAGE_TLS_CALLBACK*>(pTLS->AddressOfCallBacks);
			for (; pCallback && (*pCallback); ++pCallback)
			{
				*pCallback = nullptr;
			}

			pTLS->AddressOfCallBacks	= 0;
			pTLS->AddressOfIndex		= 0;
			pTLS->EndAddressOfRawData	= 0;
			pTLS->SizeOfZeroFill		= 0;
			pTLS->StartAddressOfRawData = 0;
			pOp->DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].VirtualAddress	= 0;
			pOp->DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].Size				= 0;
		}
	}

	if (_Flags & INJ_ERASE_HEADER)
	{
		for (UINT i = 0; i < 0x1000; i += sizeof(ULONG64))
		{
			*ReCa<ULONG64*>(pBase + i) = 0;
		}
	}

	pData->hRet = ReCa<HINSTANCE>(pBase);

	return pData->hRet;
}

DWORD ManualMapping_Shell_End() { return 2; }