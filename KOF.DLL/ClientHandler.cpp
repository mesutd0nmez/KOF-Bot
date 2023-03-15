#include "pch.h"
#include "ClientHandler.h"
#include "Client.h"
#include "Memory.h"
#include "Packet.h"
#include "Bot.h"
#include "Service.h"
#include "Guard.h"
#include "Drawing.h"

ClientHandler::ClientHandler(Bot* pBot)
{
	m_Bot = pBot;

	Clear();
}

ClientHandler::~ClientHandler()
{
	m_Bot = nullptr;

	Clear();
}

void ClientHandler::Clear()
{
	m_bWorking = false;

	m_bConfigurationLoaded = false;

#ifdef USE_MAILSLOT
	m_bMailSlotWorking = false;
	m_szMailSlotRecvName.clear();
	m_szMailSlotSendName.clear();

	m_RecvHookAddress = 0;
	m_SendHookAddress = 0;
#endif

	m_szAccountId.clear();
	m_szPassword.clear();

	m_vecOrigDeathEffectFunction.clear();

	m_ClientHook = nullptr;

	m_vecRoute.clear();
}

void ClientHandler::Initialize()
{
#ifdef DEBUG
	printf("Client handler initializing\n");
#endif

	PatchClient();
}

void ClientHandler::StartHandler()
{
#ifdef DEBUG
	printf("Client handler starting\n");
#endif

	m_bWorking = true;

	new std::thread([this]() { BasicAttackProcess(); });
	new std::thread([this]() { AttackProcess(); });
	new std::thread([this]() { SearchTargetProcess(); });
	new std::thread([this]() { AutoLootProcess(); });
	new std::thread([this]() { CharacterProcess(); });
	new std::thread([this]() { ProtectionProcess(); });
	new std::thread([this]() { GodModeProcess(); });
	new std::thread([this]() { MinorProcess(); });
	new std::thread([this]() { RouteProcess(); });
}

void ClientHandler::StopHandler()
{
#ifdef DEBUG
	printf("Client handler stopped\n");
#endif

	m_bWorking = false;

	GetClient()->Clear();
}

void ClientHandler::Process()
{
	if (m_bWorking && IsDisconnect())
	{
#ifdef DEBUG
		printf("Client connection closed\n");
#endif
		StopHandler();

		if (m_Bot->GetPlatformType() == PlatformType::CNKO)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(3000));

			PushPhase(GetAddress(skCryptDec("KO_PTR_INTRO")));

			new std::thread([this]()
			{
				WaitCondition(Read4Byte(Read4Byte(GetAddress(skCryptDec("KO_PTR_INTRO"))) + GetAddress(skCryptDec("KO_OFF_UI_LOGIN_INTRO"))) == 0);

				std::this_thread::sleep_for(std::chrono::milliseconds(500));

				ConnectLoginServer();
			});
		}
	}
}

void ClientHandler::OnReady()
{
#ifdef DEBUG
	printf("Client handler ready\n");
#endif

	new std::thread([this]() { m_Bot->InitializeStaticData(); });
	new std::thread([this]() { m_Bot->InitializeRouteData(); });

	if (m_Bot->GetPlatformType() == PlatformType::CNKO)
	{
		PushPhase(GetAddress(skCryptDec("KO_PTR_INTRO")));

		new std::thread([this]()
		{
			WaitCondition(Read4Byte(Read4Byte(GetAddress(skCryptDec("KO_PTR_INTRO"))) + GetAddress(skCryptDec("KO_OFF_UI_LOGIN_INTRO"))) == 0);

			std::this_thread::sleep_for(std::chrono::milliseconds(500));

			if (m_Bot->m_iSelectedAccount <= (int32_t)m_Bot->m_AccountList.size())
			{
				auto jSelectedAccount = m_Bot->m_AccountList.at(m_Bot->m_iSelectedAccount);

				SetLoginInformation(
					jSelectedAccount["accountId"].get<std::string>(),
					jSelectedAccount["password"].get<std::string>());

				ConnectLoginServer();
			}
		});
	}
}

void ClientHandler::PatchClient()
{
	onClientSendProcess = [=](BYTE* iStream, DWORD iStreamLength)
	{
		SendProcess(iStream, iStreamLength);
	};

	onClientRecvProcess = [=](BYTE* iStream, DWORD iStreamLength)
	{
		RecvProcess(iStream, iStreamLength);
	};

	for (size_t i = 0; i < 11; i++)
		PatchRecvAddress(GetAddress(skCryptDec("KO_PTR_INTRO")) + (4 * i));

	PatchSendAddress();

#ifdef USE_MAILSLOT
	m_bMailSlotWorking = true;
	new std::thread([this]() { MailSlotRecvProcess(); });
	new std::thread([this]() { MailSlotSendProcess(); });
#endif

	OnReady();
}

void ClientHandler::PatchRecvAddress(DWORD dwAddress)
{
#ifndef USE_MAILSLOT
	WaitCondition(Read4Byte(dwAddress) == 0);

	DWORD dwRecvAddress = Read4Byte(Read4Byte(dwAddress)) + 0x8;

	BYTE byPatch[] =
	{
		0x55,									//push ebp
		0x8B, 0xEC,								//mov ebp,esp
		0x83, 0xC4, 0xF8,						//add esp,-08
		0x53,									//push ebx
		0x8B, 0x45, 0x08,						//mov eax,[ebp+08]
		0x83, 0xC0, 0x04,						//add eax,04
		0x8B, 0x10,								//mov edx,[eax]
		0x89, 0x55, 0xFC,						//mov [ebp-04],edx
		0x8B, 0x4D, 0x08,						//mov ecx,[ebp+08]
		0x83, 0xC1, 0x08,						//add ecx,08
		0x8B, 0x01,								//mov eax,[ecx]
		0x89, 0x45, 0xF8,						//mov [ebp-08],eax
		0xFF, 0x75, 0xFC,						//push [ebp-04]
		0xFF, 0x75, 0xF8,						//push [ebp-08]
		0xB8, 0x00, 0x00, 0x00, 0x00,			//mov eax,00000000 <-- ClientHook::RecvProcess()
		0xFF, 0xD0,								//call eax
		0x83, 0xC4, 0x08,						//add esp,08
		0x8B, 0x0D, 0x00, 0x00, 0x00, 0x00,		//mov ecx,[00000000] <-- KO_PTR_DLG
		0xFF, 0x75, 0x0C,						//push [ebp+0C]
		0xFF, 0x75, 0x08,						//push [ebp+08]
		0xB8, 0x00, 0x00, 0x00, 0x00,			//mov eax,00000000 <-- GetRecvCallAddress()
		0xFF, 0xD0,								//call eax
		0x5B,									//pop ebx
		0x59,									//pop ecx
		0x59,									//pop ecx
		0x5D,									//pop ebp
		0xC2, 0x08, 0x00						//ret 0008
	};

	ClientHook* pClientHook = new ClientHook(this);

	DWORD dwRecvProcessFunction = (DWORD)(LPVOID*)pClientHook->RecvProcess;
	CopyBytes(byPatch + 36, dwRecvProcessFunction);

	DWORD dwDlgAddress = dwAddress;
	CopyBytes(byPatch + 47, dwDlgAddress);

	DWORD dwRecvCallAddress = Read4Byte(dwRecvAddress);
	CopyBytes(byPatch + 58, dwRecvCallAddress);

	std::vector<BYTE> vecPatch(byPatch, byPatch + sizeof(byPatch));

	DWORD dwPatchAddress = (DWORD)VirtualAlloc(0, 1, MEM_COMMIT, PAGE_EXECUTE_READWRITE);

	if (dwPatchAddress == 0)
		return;

	WriteBytes(dwPatchAddress, vecPatch);

	DWORD dwOldProtection;
	VirtualProtect((LPVOID)dwRecvAddress, 1, PAGE_EXECUTE_READWRITE, &dwOldProtection);
	Write4Byte(dwRecvAddress, dwPatchAddress);
	VirtualProtect((LPVOID)dwRecvAddress, 1, dwOldProtection, &dwOldProtection);
#else

	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, m_Bot->GetInjectedProcessId());

	if (hProcess == nullptr)
		return;

	DWORD dwAddressReady = 0;
	while (dwAddressReady == 0)
	{
		ReadProcessMemory(hProcess, (LPVOID)dwAddress, &dwAddressReady, 4, 0);
	}

	HMODULE hModuleKernel32 = GetModuleHandle(skCryptDec("kernel32.dll"));

	if (hModuleKernel32 == nullptr)
	{
#ifdef DEBUG
		printf("hModuleKernel32 == nullptr\n");
#endif
		CloseHandle(hProcess);
		return;
	}

	LPVOID pCreateFilePtr = GetProcAddress(hModuleKernel32, skCryptDec("CreateFileA"));
	LPVOID pWriteFilePtr = GetProcAddress(hModuleKernel32, skCryptDec("WriteFile"));
	LPVOID pCloseHandlePtr = GetProcAddress(hModuleKernel32, skCryptDec("CloseHandle"));

	LPVOID pMailSlotNameAddress = VirtualAllocEx(hProcess, 0, 1, MEM_COMMIT, PAGE_EXECUTE_READWRITE);

	if (pMailSlotNameAddress == 0)
	{
		CloseHandle(hProcess);
		return;
	}

	m_szMailSlotRecvName = skCryptDec("\\\\.\\mailslot\\KOF_RECV\\") + std::to_string(m_Bot->GetInjectedProcessId());

	std::vector<BYTE> vecMailSlotName(m_szMailSlotRecvName.begin(), m_szMailSlotRecvName.end());
	WriteBytes((DWORD)pMailSlotNameAddress, vecMailSlotName);

	if (m_RecvHookAddress == 0)
	{
		m_RecvHookAddress = VirtualAllocEx(hProcess, 0, 1, MEM_COMMIT, PAGE_EXECUTE_READWRITE);

		if (m_RecvHookAddress == 0)
		{
			CloseHandle(hProcess);
			return;
		}

		BYTE byHookPatch[] =
		{
			0x55,
			0x8B, 0xEC,
			0x83, 0xC4, 0xF4,
			0x33, 0xC0,
			0x89, 0x45, 0xFC,
			0x33, 0xD2,
			0x89, 0x55, 0xF8,
			0x6A, 0x00,
			0x68, 0x80, 0x00, 0x00, 0x00,
			0x6A, 0x03,
			0x6A, 0x00,
			0x6A, 0x01,
			0x68, 0x00, 0x00, 0x00, 0x40,
			0x68, 0x00, 0x00, 0x00, 0x00,
			0xE8, 0x00, 0x00, 0x00, 0x00,
			0x89, 0x45, 0xF8,
			0x6A, 0x00,
			0x8D, 0x4D, 0xFC,
			0x51,
			0xFF, 0x75, 0x0C,
			0xFF, 0x75, 0x08,
			0xFF, 0x75, 0xF8,
			0xE8, 0x00, 0x00, 0x00, 0x00,
			0x89, 0x45, 0xF4,
			0xFF, 0x75, 0xF8,
			0xE8, 0x00, 0x00, 0x00, 0x00,
			0x8B, 0xE5,
			0x5D,
			0xC3
		};

		CopyBytes(byHookPatch + 35, pMailSlotNameAddress);

		DWORD dwCreateFileDifference = Memory::GetDifference((DWORD)m_RecvHookAddress + 39, (DWORD)pCreateFilePtr);
		CopyBytes(byHookPatch + 40, dwCreateFileDifference);

		DWORD dwWriteFileDifference = Memory::GetDifference((DWORD)m_RecvHookAddress + 62, (DWORD)pWriteFilePtr);
		CopyBytes(byHookPatch + 63, dwWriteFileDifference);

		DWORD dwCloseHandlePtrDifference = Memory::GetDifference((DWORD)m_RecvHookAddress + 73, (DWORD)pCloseHandlePtr);
		CopyBytes(byHookPatch + 74, dwCloseHandlePtrDifference);

		std::vector<BYTE> vecHookPatch(byHookPatch, byHookPatch + sizeof(byHookPatch));
		WriteBytes((DWORD)m_RecvHookAddress, vecHookPatch);
	}

	DWORD dwRecvAddress = Read4Byte(Read4Byte(dwAddress)) + 0x8;

	BYTE byPatch[] =
	{
		0x55,									//push ebp
		0x8B, 0xEC,								//mov ebp,esp
		0x83, 0xC4, 0xF8,						//add esp,-08
		0x53,									//push ebx
		0x8B, 0x45, 0x08,						//mov eax,[ebp+08]
		0x83, 0xC0, 0x04,						//add eax,04
		0x8B, 0x10,								//mov edx,[eax]
		0x89, 0x55, 0xFC,						//mov [ebp-04],edx
		0x8B, 0x4D, 0x08,						//mov ecx,[ebp+08]
		0x83, 0xC1, 0x08,						//add ecx,08
		0x8B, 0x01,								//mov eax,[ecx]
		0x89, 0x45, 0xF8,						//mov [ebp-08],eax
		0xFF, 0x75, 0xFC,						//push [ebp-04]
		0xFF, 0x75, 0xF8,						//push [ebp-08]
		0xB8, 0x00, 0x00, 0x00, 0x00,			//mov eax,00000000 <-- ClientHook::RecvProcess()
		0xFF, 0xD0,								//call eax
		0x83, 0xC4, 0x08,						//add esp,08
		0x8B, 0x0D, 0x00, 0x00, 0x00, 0x00,		//mov ecx,[00000000] <-- KO_PTR_DLG
		0xFF, 0x75, 0x0C,						//push [ebp+0C]
		0xFF, 0x75, 0x08,						//push [ebp+08]
		0xB8, 0x00, 0x00, 0x00, 0x00,			//mov eax,00000000 <-- GetRecvCallAddress()
		0xFF, 0xD0,								//call eax
		0x5B,									//pop ebx
		0x59,									//pop ecx
		0x59,									//pop ecx
		0x5D,									//pop ebp
		0xC2, 0x08, 0x00						//ret 0008
	};

	DWORD dwRecvProcessFunction = (DWORD)(LPVOID*)m_RecvHookAddress;
	CopyBytes(byPatch + 36, dwRecvProcessFunction);

	DWORD dwDlgAddress = dwAddress;
	CopyBytes(byPatch + 47, dwDlgAddress);

	DWORD dwRecvCallAddress = Read4Byte(dwRecvAddress);
	CopyBytes(byPatch + 58, dwRecvCallAddress);

	std::vector<BYTE> vecPatch(byPatch, byPatch + sizeof(byPatch));

	DWORD dwPatchAddress = (DWORD)VirtualAllocEx(hProcess, 0, 1, MEM_COMMIT, PAGE_EXECUTE_READWRITE);

	if (dwPatchAddress == 0)
	{
		CloseHandle(hProcess);
		return;
	}

	WriteBytes(dwPatchAddress, vecPatch);

	DWORD dwOldProtection;
	VirtualProtectEx(hProcess, (LPVOID)dwRecvAddress, 1, PAGE_EXECUTE_READWRITE, &dwOldProtection);
	Write4Byte(Read4Byte(Read4Byte(dwAddress)) + 0x8, dwPatchAddress);
	VirtualProtectEx(hProcess, (LPVOID)dwRecvAddress, 1, dwOldProtection, &dwOldProtection);
	CloseHandle(hProcess);
#endif

#ifdef DEBUG
	printf("PatchRecvAddress: 0x%x patched\n", dwRecvAddress);
#endif
}

void ClientHandler::PatchSendAddress()
{
#ifndef USE_MAILSLOT
	WaitCondition(Read4Byte(GetAddress("KO_SND_FNC")) == 0);

	BYTE byPatch1[] =
	{
		0x55,										//push ebp
		0x8B, 0xEC,									//mov ebp,esp 
		0x60,										//pushad
		0xFF, 0x75, 0x0C,							//push [ebp+0C]
		0xFF, 0x75, 0x08,							//push [ebp+08]
		0xBA, 0x00, 0x00, 0x00, 0x00,				//mov edx,00000000 <-- ClientHook::SendProcess()
		0xFF, 0xD2,									//call edx
		0x5E,										//pop esi
		0x5D,										//pop ebp
		0x61,										//popad
		0x6A, 0xFF,									//push-01
		0xBA, 0x00, 0x00, 0x00, 0x00,				//mov edx,00000000 <-- KO_SND_FNC
		0x83, 0xC2, 0x5,							//add edx,05
		0xFF, 0xE2									//jmp edx
	};

	ClientHook* pClientHook = new ClientHook(this);

	DWORD dwSendProcessFunction = (DWORD)(LPVOID*)pClientHook->SendProcess;
	CopyBytes(byPatch1 + 11, dwSendProcessFunction);

	DWORD dwKoPtrSndFnc = GetAddress("KO_SND_FNC");
	CopyBytes(byPatch1 + 23, dwKoPtrSndFnc);

	std::vector<BYTE> vecPatch1(byPatch1, byPatch1 + sizeof(byPatch1));

	DWORD dwPatchAddress = (DWORD)VirtualAlloc(0, 1, MEM_COMMIT, PAGE_EXECUTE_READWRITE);

	if (dwPatchAddress == 0)
		return;

	WriteBytes(dwPatchAddress, vecPatch1);

	BYTE byPatch2[] =
	{
		0xE9, 0x00, 0x00, 0x00, 0x00,
	};

	DWORD dwCallDifference = Memory::GetDifference(GetAddress("KO_SND_FNC"), dwPatchAddress);
	CopyBytes(byPatch2 + 1, dwCallDifference);

	std::vector<BYTE> vecPatch2(byPatch2, byPatch2 + sizeof(byPatch2));

	DWORD dwOldProtection;
	VirtualProtect((LPVOID)GetAddress("KO_SND_FNC"), 1, PAGE_EXECUTE_READWRITE, &dwOldProtection);
	WriteBytes(GetAddress("KO_SND_FNC"), vecPatch2);
	VirtualProtect((LPVOID)GetAddress("KO_SND_FNC"), 1, dwOldProtection, &dwOldProtection);
#else
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, m_Bot->GetInjectedProcessId());

	if (hProcess == nullptr)
		return;

	DWORD dwAddressReady = 0;
	while (dwAddressReady == 0)
	{
		ReadProcessMemory(hProcess, (LPVOID)GetAddress(skCryptDec("KO_SND_FNC")), &dwAddressReady, 4, 0);
	}

	HMODULE hModuleKernel32 = GetModuleHandle(skCryptDec("kernel32.dll"));

	if (hModuleKernel32 == nullptr)
	{
#ifdef DEBUG
		printf("hModuleKernel32 == nullptr\n");
#endif
		CloseHandle(hProcess);
		return;
	}

	LPVOID pCreateFilePtr = GetProcAddress(hModuleKernel32, skCryptDec("CreateFileA"));
	LPVOID pWriteFilePtr = GetProcAddress(hModuleKernel32, skCryptDec("WriteFile"));
	LPVOID pCloseHandlePtr = GetProcAddress(hModuleKernel32, skCryptDec("CloseHandle"));

	LPVOID pMailSlotNameAddress = VirtualAllocEx(hProcess, 0, 1, MEM_COMMIT, PAGE_EXECUTE_READWRITE);

	if (pMailSlotNameAddress == 0)
	{
		CloseHandle(hProcess);
		return;
	}

	m_szMailSlotSendName = skCryptDec("\\\\.\\mailslot\\KOF_SEND\\") + std::to_string(m_Bot->GetInjectedProcessId());

	std::vector<BYTE> vecMailSlotName(m_szMailSlotSendName.begin(), m_szMailSlotSendName.end());
	WriteBytes((DWORD)pMailSlotNameAddress, vecMailSlotName);

	if (m_SendHookAddress == 0)
	{
		m_SendHookAddress = VirtualAllocEx(hProcess, 0, 1, MEM_COMMIT, PAGE_EXECUTE_READWRITE);

		if (m_SendHookAddress == 0)
		{
			CloseHandle(hProcess);
			return;
		}

		BYTE byHookPatch[] =
		{
			0x55,
			0x8B, 0xEC,
			0x83, 0xC4, 0xF4,
			0x33, 0xC0,
			0x89, 0x45, 0xFC,
			0x33, 0xD2,
			0x89, 0x55, 0xF8,
			0x6A, 0x00,
			0x68, 0x80, 0x00, 0x00, 0x00,
			0x6A, 0x03,
			0x6A, 0x00,
			0x6A, 0x01,
			0x68, 0x00, 0x00, 0x00, 0x40,
			0x68, 0x00, 0x00, 0x00, 0x00,
			0xE8, 0x00, 0x00, 0x00, 0x00,
			0x89, 0x45, 0xF8,
			0x6A, 0x00,
			0x8D, 0x4D, 0xFC,
			0x51,
			0xFF, 0x75, 0x0C,
			0xFF, 0x75, 0x08,
			0xFF, 0x75, 0xF8,
			0xE8, 0x00, 0x00, 0x00, 0x00,
			0x89, 0x45, 0xF4,
			0xFF, 0x75, 0xF8,
			0xE8, 0x00, 0x00, 0x00, 0x00,
			0x8B, 0xE5,
			0x5D,
			0xC3
		};

		CopyBytes(byHookPatch + 35, pMailSlotNameAddress);

		DWORD dwCreateFileDifference = Memory::GetDifference((DWORD)m_SendHookAddress + 39, (DWORD)pCreateFilePtr);
		CopyBytes(byHookPatch + 40, dwCreateFileDifference);

		DWORD dwWriteFileDifference = Memory::GetDifference((DWORD)m_SendHookAddress + 62, (DWORD)pWriteFilePtr);
		CopyBytes(byHookPatch + 63, dwWriteFileDifference);

		DWORD dwCloseHandlePtrDifference = Memory::GetDifference((DWORD)m_SendHookAddress + 73, (DWORD)pCloseHandlePtr);
		CopyBytes(byHookPatch + 74, dwCloseHandlePtrDifference);

		std::vector<BYTE> vecHookPatch(byHookPatch, byHookPatch + sizeof(byHookPatch));
		WriteBytes((DWORD)m_SendHookAddress, vecHookPatch);
	}

	BYTE byPatch1[] =
	{
		0x55,										//push ebp
		0x8B, 0xEC,									//mov ebp,esp 
		0x60,										//pushad
		0xFF, 0x75, 0x0C,							//push [ebp+0C]
		0xFF, 0x75, 0x08,							//push [ebp+08]
		0xBA, 0x00, 0x00, 0x00, 0x00,				//mov edx,00000000 <-- ClientHook::SendProcess()
		0xFF, 0xD2,									//call edx
		0x5E,										//pop esi
		0x5D,										//pop ebp
		0x61,										//popad
		0x6A, 0xFF,									//push-01
		0xBA, 0x00, 0x00, 0x00, 0x00,				//mov edx,00000000 <-- KO_SND_FNC
		0x83, 0xC2, 0x5,							//add edx,05
		0xFF, 0xE2									//jmp edx
	};

	CopyBytes(byPatch1 + 11, m_SendHookAddress);

	DWORD dwKoPtrSndFnc = GetAddress(skCryptDec("KO_SND_FNC"));
	CopyBytes(byPatch1 + 23, dwKoPtrSndFnc);

	std::vector<BYTE> vecPatch1(byPatch1, byPatch1 + sizeof(byPatch1));

	LPVOID dwPatchAddress = VirtualAllocEx(hProcess, 0, vecPatch1.size(), MEM_COMMIT, PAGE_EXECUTE_READWRITE);

	if (dwPatchAddress == 0)
	{
		CloseHandle(hProcess);
		return;
	}

	WriteBytes((DWORD)dwPatchAddress, vecPatch1);

	BYTE byPatch2[] =
	{
		0xE9, 0x00, 0x00, 0x00, 0x00
	};

	DWORD dwCallDifference = Memory::GetDifference(GetAddress(skCryptDec("KO_SND_FNC")), (DWORD)dwPatchAddress);
	CopyBytes(byPatch2 + 1, dwCallDifference);

	std::vector<BYTE> vecPatch2(byPatch2, byPatch2 + sizeof(byPatch2));

	DWORD dwOldProtection;
	VirtualProtectEx(hProcess, (LPVOID)GetAddress(skCryptDec("KO_SND_FNC")), 1, PAGE_EXECUTE_READWRITE, &dwOldProtection);
	WriteBytes(GetAddress(skCryptDec("KO_SND_FNC")), vecPatch2);
	VirtualProtectEx(hProcess, (LPVOID)GetAddress(skCryptDec("KO_SND_FNC")), 1, dwOldProtection, &dwOldProtection);
	CloseHandle(hProcess);
#endif

#ifdef DEBUG
	printf("PatchSendAddress: 0x%x patched\n", GetAddress("KO_SND_FNC"));
#endif
}

void ClientHandler::MailSlotRecvProcess()
{
	HANDLE hSlot = CreateMailslotA(m_szMailSlotRecvName.c_str(), 0, MAILSLOT_WAIT_FOREVER, NULL);

	if (hSlot == INVALID_HANDLE_VALUE)
	{
#ifdef DEBUG
		printf("CreateMailslot failed with %d\n", GetLastError());
#endif
		return;
	}

	while (m_bMailSlotWorking)
	{
		try
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));

			DWORD dwCurrentMesageSize, dwMesageLeft, dwMessageReadSize;
			OVERLAPPED ov;

			BOOL fResult = GetMailslotInfo(hSlot, NULL, &dwCurrentMesageSize, &dwMesageLeft, NULL);

			if (!fResult)
				continue;

			if (dwCurrentMesageSize == MAILSLOT_NO_MESSAGE)
				continue;

			std::vector<uint8_t> vecMessageBuffer;

			vecMessageBuffer.resize(dwCurrentMesageSize);

			ov.Offset = 0;
			ov.OffsetHigh = 0;
			ov.hEvent = NULL;

			fResult = ReadFile(hSlot, &vecMessageBuffer[0], dwCurrentMesageSize, &dwMessageReadSize, &ov);

			if (!fResult)
				continue;

			vecMessageBuffer.resize(dwMessageReadSize);

			onClientRecvProcess(vecMessageBuffer.data(), vecMessageBuffer.size());
		}
		catch (const std::exception& e)
		{
#ifdef DEBUG
			printf("MailSlotRecvProcess:Exception: %s\n", e.what());
#else
			UNREFERENCED_PARAMETER(e);
#endif
		}
	}
}

void ClientHandler::MailSlotSendProcess()
{
	HANDLE hSlot = CreateMailslotA(m_szMailSlotSendName.c_str(), 0, MAILSLOT_WAIT_FOREVER, NULL);

	if (hSlot == INVALID_HANDLE_VALUE)
	{
#ifdef DEBUG
		printf("CreateMailslot failed with %d\n", GetLastError());
#endif
		return;
	}

	while (m_bMailSlotWorking)
	{
		try
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));

			DWORD dwCurrentMesageSize, dwMesageLeft, dwMessageReadSize;
			OVERLAPPED ov;

			BOOL fResult = GetMailslotInfo(hSlot, NULL, &dwCurrentMesageSize, &dwMesageLeft, NULL);

			if (!fResult)
				continue;

			if (dwCurrentMesageSize == MAILSLOT_NO_MESSAGE)
				continue;

			std::vector<uint8_t> vecMessageBuffer;

			vecMessageBuffer.resize(dwCurrentMesageSize);

			ov.Offset = 0;
			ov.OffsetHigh = 0;
			ov.hEvent = NULL;

			fResult = ReadFile(hSlot, &vecMessageBuffer[0], dwCurrentMesageSize, &dwMessageReadSize, &ov);

			if (!fResult)
				continue;

			vecMessageBuffer.resize(dwMessageReadSize);

			onClientSendProcess(vecMessageBuffer.data(), vecMessageBuffer.size());
		}
		catch (const std::exception& e)
		{
#ifdef DEBUG
			printf("MailSlotSendProcess:Exception: %s\n", e.what());
#else
			UNREFERENCED_PARAMETER(e);
#endif
		}
	}
}

void ClientHandler::RecvProcess(BYTE* byBuffer, DWORD dwLength)
{
	Packet pkt = Packet(byBuffer[0], (size_t)dwLength);
	pkt.append(&byBuffer[1], dwLength - 1);

#ifdef DEBUG
#ifdef PRINT_RECV_PACKET
	printf("RecvProcess: %s\n", pkt.convertToHex().c_str());
#endif
#endif

	uint8_t iHeader;

	pkt >> iHeader;

	switch (iHeader)
	{
		case LS_LOGIN_REQ:
		{
			int16_t sUnknown;
			int8_t byResult;

			pkt >> sUnknown >> byResult;

			switch (byResult)
			{
				case AUTH_BANNED:
				{
#ifdef DEBUG
					printf("RecvProcess::LS_LOGIN_REQ: Account Banned\n");
#endif
				}
				break;

				case AUTH_IN_GAME:
				{
#ifdef DEBUG
					printf("RecvProcess::LS_LOGIN_REQ: Account already in-game\n");
#endif
					new std::thread([this]()
					{
#ifdef DEBUG
						printf("RecvProcess::LS_LOGIN_REQ: Reconnecting login server\n");

						std::this_thread::sleep_for(std::chrono::milliseconds(500));
#endif
						ConnectLoginServer(true);
					});
				}
				break;

				case AUTH_SUCCESS:
				{
#ifdef DEBUG
					printf("RecvProcess::LS_LOGIN_REQ: Login Success\n");
#endif
				}
				break;

				default:
				{
#ifdef DEBUG
					printf("RecvProcess::LS_LOGIN_REQ: %d not implemented!\n", byResult);
#endif
				}
				break;
			}
		}
		break;

		case LS_SERVERLIST:
		{
			int8_t byServerCount;

			pkt >> byServerCount;

			if (byServerCount > 0)
			{
#ifdef DEBUG
				printf("RecvProcess::LS_SERVERLIST: %d Server loaded\n", byServerCount);
#endif

				if (m_Bot->GetPlatformType() == PlatformType::CNKO)
				{
					new std::thread([this]()
					{
#ifdef DEBUG
						printf("RecvProcess::LS_SERVERLIST: Connecting to server: %d\n", 1);

						std::this_thread::sleep_for(std::chrono::milliseconds(500));
#endif
						ConnectGameServer(1);
					});
				}

			}
		}
		break;

		case WIZ_ALLCHAR_INFO_REQ:
		{
			int8_t byResult;

			pkt >> byResult;

			//Packet End: 0C 02 00
			bool bLoaded =
				pkt[pkt.size() - 3] == 0x0C &&
				pkt[pkt.size() - 2] == 0x02 &&
				pkt[pkt.size() - 1] == 0x00;

			if (bLoaded)
			{
#ifdef DEBUG
				printf("RecvProcess::WIZ_ALLCHAR_INFO_REQ: Character list loaded\n");
#endif

				if (m_Bot->GetPlatformType() == PlatformType::CNKO)
				{
					new std::thread([this]()
					{
#ifdef DEBUG
						printf("RecvProcess::WIZ_ALLCHAR_INFO_REQ: Selecting character %d\n", 1);
#endif

						std::this_thread::sleep_for(std::chrono::milliseconds(500));

						SelectCharacterSkip();
						SelectCharacter(1);
					});
				}
			}
		}
		break;

		case WIZ_SEL_CHAR:
		{
			int iRet = pkt.read<uint8_t>();

			if (iRet == 1)
			{
				SetMovePosition(Vector3(0.0f, 0.0f, 0.0f));

				m_PlayerMySelf.iCity = pkt.read<uint8_t>();

				m_PlayerMySelf.fX = (pkt.read<uint16_t>()) / 10.0f;
				m_PlayerMySelf.fY = (pkt.read<uint16_t>()) / 10.0f;
				m_PlayerMySelf.fZ = (pkt.read<int16_t>()) / 10.0f;

				uint8_t iVictoryNation = pkt.read<uint8_t>();

				m_Bot->GetWorld()->Load(m_PlayerMySelf.iCity);

#ifdef DEBUG
				printf("RecvProcess::WIZ_SEL_CHAR Zone: [%d] Coordinate: [%f - %f - %f] VictoryNation: [%d]\n", 
					m_PlayerMySelf.iCity, m_PlayerMySelf.fX, m_PlayerMySelf.fY, m_PlayerMySelf.fZ, iVictoryNation);
#endif
			}
		}
		break;

		case WIZ_MYINFO:
		{
			pkt.SByte();

			m_PlayerMySelf.iID = pkt.read<int32_t>();

			int iNameLen = pkt.read<uint8_t>();
			pkt.readString(m_PlayerMySelf.szName, iNameLen);

			m_PlayerMySelf.fX = (pkt.read<uint16_t>()) / 10.0f;
			m_PlayerMySelf.fY = (pkt.read<uint16_t>()) / 10.0f;
			m_PlayerMySelf.fZ = (pkt.read<int16_t>()) / 10.0f;

			m_PlayerMySelf.eNation = (Nation)pkt.read<uint8_t>();
			m_PlayerMySelf.eRace = (Race)pkt.read<uint8_t>();
			m_PlayerMySelf.eClass = (Class)pkt.read<int16_t>();

			m_PlayerMySelf.iFace = pkt.read<uint8_t>();
			m_PlayerMySelf.iHair = pkt.read<int32_t>();
			m_PlayerMySelf.iRank = pkt.read<uint8_t>();
			m_PlayerMySelf.iTitle = pkt.read<uint8_t>();

			m_PlayerMySelf.iUnknown1 = pkt.read<uint8_t>();
			m_PlayerMySelf.iUnknown2 = pkt.read<uint8_t>();

			m_PlayerMySelf.iLevel = pkt.read<uint8_t>();

			m_PlayerMySelf.iBonusPointRemain = pkt.read<uint16_t>();
			m_PlayerMySelf.iExpNext = pkt.read<uint64_t>();
			m_PlayerMySelf.iExp = pkt.read<uint64_t>();

			m_PlayerMySelf.iRealmPoint = pkt.read<uint32_t>();
			m_PlayerMySelf.iRealmPointMonthly = pkt.read<uint32_t>();

			m_PlayerMySelf.iKnightsID = pkt.read<int16_t>();
			m_PlayerMySelf.eKnightsDuty = (KnightsDuty)pkt.read<uint8_t>();

			int16_t iAllianceID = pkt.read<int16_t>();
			uint8_t byFlag = pkt.read<uint8_t>();

			uint8_t iKnightNameLen = pkt.read<uint8_t>();
			pkt.readString(m_PlayerMySelf.szKnights, iKnightNameLen);

			m_PlayerMySelf.iKnightsGrade = pkt.read<uint8_t>();
			m_PlayerMySelf.iKnightsRank = pkt.read<uint8_t>();

			int16_t sMarkVersion = pkt.read<int16_t>();
			int16_t sCapeID = pkt.read<int16_t>();

			uint8_t iR = pkt.read<uint8_t>();
			uint8_t iG = pkt.read<uint8_t>();
			uint8_t iB = pkt.read<uint8_t>();

			uint8_t iUnknown1 = pkt.read<uint8_t>();

			uint8_t iUnknown2 = pkt.read<uint8_t>();
			uint8_t iUnknown3 = pkt.read<uint8_t>();
			uint8_t iUnknown4 = pkt.read<uint8_t>();
			uint8_t iUnknown5 = pkt.read<uint8_t>();

			m_PlayerMySelf.iHPMax = pkt.read<int16_t>();
			m_PlayerMySelf.iHP = pkt.read<int16_t>();

			m_PlayerMySelf.iMSPMax = pkt.read<int16_t>();
			m_PlayerMySelf.iMSP = pkt.read<int16_t>();

			m_PlayerMySelf.iWeightMax = pkt.read<uint32_t>();
			m_PlayerMySelf.iWeight = pkt.read<uint32_t>();

			m_PlayerMySelf.iStrength = pkt.read<uint8_t>();
			m_PlayerMySelf.iStrength_Delta = pkt.read<uint8_t>();
			m_PlayerMySelf.iStamina = pkt.read<uint8_t>();
			m_PlayerMySelf.iStamina_Delta = pkt.read<uint8_t>();
			m_PlayerMySelf.iDexterity = pkt.read<uint8_t>();
			m_PlayerMySelf.iDexterity_Delta = pkt.read<uint8_t>();
			m_PlayerMySelf.iIntelligence = pkt.read<uint8_t>();
			m_PlayerMySelf.iIntelligence_Delta = pkt.read<uint8_t>();
			m_PlayerMySelf.iMagicAttak = pkt.read<uint8_t>();
			m_PlayerMySelf.iMagicAttak_Delta = pkt.read<uint8_t>();

			m_PlayerMySelf.iAttack = pkt.read<int16_t>();
			m_PlayerMySelf.iGuard = pkt.read<int16_t>();

			m_PlayerMySelf.iRegistFire = pkt.read<uint8_t>();
			m_PlayerMySelf.iRegistCold = pkt.read<uint8_t>();
			m_PlayerMySelf.iRegistLight = pkt.read<uint8_t>();
			m_PlayerMySelf.iRegistMagic = pkt.read<uint8_t>();
			m_PlayerMySelf.iRegistCurse = pkt.read<uint8_t>();
			m_PlayerMySelf.iRegistPoison = pkt.read<uint8_t>();

			m_PlayerMySelf.iGold = pkt.read<uint32_t>();
			m_PlayerMySelf.iAuthority = pkt.read<uint8_t>();

			uint8_t bUserRank = pkt.read<uint8_t>();
			uint8_t bPersonalRank = pkt.read<uint8_t>();

			for (int i = 0; i < 9; i++)
			{
				m_PlayerMySelf.iSkillInfo[i] = pkt.read<uint8_t>();
			}

			for (int i = 0; i < INVENTORY_TOTAL; i++)
			{
				m_PlayerMySelf.tInventory[i].iPos = i;
				m_PlayerMySelf.tInventory[i].iItemID = pkt.read<uint32_t>();
				m_PlayerMySelf.tInventory[i].iDurability = pkt.read<uint16_t>();
				m_PlayerMySelf.tInventory[i].iCount = pkt.read<uint16_t>();
				m_PlayerMySelf.tInventory[i].iFlag = pkt.read<uint8_t>();
				m_PlayerMySelf.tInventory[i].iRentalTime = pkt.read<int16_t>();
				m_PlayerMySelf.tInventory[i].iSerial = pkt.read<uint32_t>();
				m_PlayerMySelf.tInventory[i].iExpirationTime = pkt.read<uint32_t>();
			}

			m_PlayerMySelf.bBlinking = true;

			LoadSkillData();

			m_Bot->SendLoadUserConfiguration(1, m_PlayerMySelf.szName);

#ifdef DEBUG
			printf("RecvProcess::WIZ_MYINFO: %s loaded\n", m_PlayerMySelf.szName.c_str());
#endif
		}
		break;

		case WIZ_HP_CHANGE:
		{
			m_PlayerMySelf.iHPMax = pkt.read<int16_t>();
			m_PlayerMySelf.iHP = pkt.read<int16_t>();

#ifdef DEBUG
			printf("RecvProcess::WIZ_HP_CHANGE: %d / %d\n", m_PlayerMySelf.iHP, m_PlayerMySelf.iHPMax);
#endif
		}
		break;

		case WIZ_MSP_CHANGE:
		{
			m_PlayerMySelf.iMSPMax = pkt.read<int16_t>();
			m_PlayerMySelf.iMSP = pkt.read<int16_t>();

#ifdef DEBUG
			printf("RecvProcess::WIZ_MSP_CHANGE: %d / %d\n", m_PlayerMySelf.iMSP, m_PlayerMySelf.iMSPMax);
#endif
		}
		break;

		case WIZ_EXP_CHANGE:
		{
			uint8_t iUnknown1 = pkt.read<uint8_t>();
			m_PlayerMySelf.iExp = pkt.read<uint64_t>();

#ifdef DEBUG
			printf("RecvProcess::WIZ_EXP_CHANGE: %llu\n", m_PlayerMySelf.iExp);
#endif
		}
		break;

		case WIZ_LEVEL_CHANGE:
		{
			int32_t iID = pkt.read<int32_t>();
			uint8_t iLevel = pkt.read<uint8_t>();

			if (m_PlayerMySelf.iID == iID)
			{
				m_PlayerMySelf.iLevel = iLevel;
				m_PlayerMySelf.iBonusPointRemain = pkt.read<uint16_t>();
				m_PlayerMySelf.iSkillInfo[0] = pkt.read<uint8_t>();
				m_PlayerMySelf.iExpNext = pkt.read<int64_t>();
				m_PlayerMySelf.iExp = pkt.read<int64_t>();
				m_PlayerMySelf.iHPMax = pkt.read<int16_t>();
				m_PlayerMySelf.iHP = pkt.read<int16_t>();
				m_PlayerMySelf.iMSPMax = pkt.read<int16_t>();
				m_PlayerMySelf.iMSP = pkt.read<int16_t>();
				m_PlayerMySelf.iWeightMax = pkt.read<uint32_t>();
				m_PlayerMySelf.iWeight = pkt.read<uint32_t>();

#ifdef DEBUG
				printf("RecvProcess::WIZ_LEVEL_CHANGE: %s %d\n", m_PlayerMySelf.szName.c_str(), iLevel);
#endif
			}
			else
			{
				Guard lock(m_vecPlayerLock);
				auto it = std::find_if(m_vecPlayer.begin(), m_vecPlayer.end(),
					[iID](const TPlayer& a) { return a.iID == iID; });

				if (it != m_vecPlayer.end())
				{
					it->iLevel = iLevel;

#ifdef DEBUG
					printf("RecvProcess::WIZ_LEVEL_CHANGE: %s %d\n", it->szName.c_str(), iLevel);
#endif
				}
			}
		}
		break;

		case WIZ_POINT_CHANGE:
		{
			uint8_t iType = pkt.read<uint8_t>();
			int16_t iVal = pkt.read<int16_t>();

			m_PlayerMySelf.iHPMax = pkt.read<int16_t>();
			m_PlayerMySelf.iMSPMax = pkt.read<int16_t>();
			m_PlayerMySelf.iAttack = pkt.read<int16_t>();
			m_PlayerMySelf.iWeightMax = pkt.read<uint32_t>();

			switch (iType)
			{
				case 0x01:
				{
					m_PlayerMySelf.iStrength = (uint8_t)iVal;
#ifdef DEBUG
					printf("RecvProcess::WIZ_POINT_CHANGE: STR %d\n", iVal);
#endif
				}
				break;

				case 0x02:
				{
					m_PlayerMySelf.iStamina = (uint8_t)iVal;
#ifdef DEBUG
					printf("RecvProcess::WIZ_POINT_CHANGE: HP %d\n", iVal);
#endif
				}
				break;

				case 0x03:
				{
					m_PlayerMySelf.iDexterity = (uint8_t)iVal;
#ifdef DEBUG
					printf("RecvProcess::WIZ_POINT_CHANGE: DEX %d\n", iVal);
#endif
				}
				break;

				case 0x04:
				{
					m_PlayerMySelf.iIntelligence = (uint8_t)iVal;
#ifdef DEBUG
					printf("RecvProcess::WIZ_POINT_CHANGE: INT %d\n", iVal);
#endif
				}
				break;

				case 0x05:
				{
					m_PlayerMySelf.iMagicAttak = (uint8_t)iVal;
#ifdef DEBUG
					printf("RecvProcess::WIZ_POINT_CHANGE: MP %d\n", iVal);
#endif
				}
				break;
			}

			if (iType >= 1 && iType <= 5)
			{
				m_PlayerMySelf.iBonusPointRemain--;
#ifdef DEBUG
				printf("RecvProcess::WIZ_POINT_CHANGE: POINT %d\n", m_PlayerMySelf.iBonusPointRemain);
#endif
			}
		}
		break;

		case WIZ_WEIGHT_CHANGE:
		{
			m_PlayerMySelf.iWeight = pkt.read<uint32_t>();
#ifdef DEBUG
			printf("RecvProcess::WIZ_WEIGHT_CHANGE: %d\n", m_PlayerMySelf.iWeight);
#endif
		}
		break;

		case WIZ_DURATION:
		{
			uint8_t iPos = pkt.read<uint8_t>();
			uint16_t iDurability = pkt.read<uint16_t>();

			m_PlayerMySelf.tInventory[iPos].iDurability = iDurability;

#ifdef DEBUG
			printf("RecvProcess::WIZ_DURATION: %d,%d\n", iPos, iDurability);
#endif
		}
		break;

		case WIZ_ITEM_REMOVE:
		{
			uint8_t	iResult = pkt.read<uint8_t>();

			switch (iResult)
			{
				case 0x00:
				{
#ifdef DEBUG
					printf("RecvProcess::WIZ_ITEM_REMOVE: 0\n");
#endif
				}
				break;

				case 0x01:
				{
#ifdef DEBUG
					printf("RecvProcess::WIZ_ITEM_REMOVE: 1\n");
#endif
				}
				break;

				default:
				{
#ifdef DEBUG
					printf("RecvProcess::WIZ_ITEM_REMOVE: %d Result Not Implemented\n", iResult);
#endif
				}
				break;
			}
		}
		break;

		case WIZ_ITEM_COUNT_CHANGE:
		{
			int16_t iTotalCount = pkt.read<int16_t>();

			for (int i = 0; i < iTotalCount; i++)
			{
				uint8_t iDistrict = pkt.read<uint8_t>();
				uint8_t iIndex = pkt.read<uint8_t>();
				uint32_t iID = pkt.read<uint32_t>();
				uint32_t iCount = pkt.read<uint32_t>();
				uint8_t iNewItem = pkt.read<uint8_t>();
				uint16_t iDurability = pkt.read<uint16_t>();

				uint32_t iSerial = pkt.read<uint32_t>();
				uint32_t iExpirationTime = pkt.read<uint32_t>();

				m_PlayerMySelf.tInventory[14 + iIndex].iItemID = iID;
				m_PlayerMySelf.tInventory[14 + iIndex].iCount = (uint16_t)iCount;
				m_PlayerMySelf.tInventory[14 + iIndex].iDurability = iDurability;
				m_PlayerMySelf.tInventory[14 + iIndex].iSerial = iSerial;
				m_PlayerMySelf.tInventory[14 + iIndex].iExpirationTime = iExpirationTime;

#ifdef DEBUG
				printf("RecvProcess::WIZ_ITEM_COUNT_CHANGE: %d,%d,%d,%d,%d,%d,%d,%d,%d\n",
					iDistrict,
					iIndex,
					iID,
					iCount,
					iNewItem,
					iNewItem,
					iDurability,
					iSerial,
					iExpirationTime);
#endif

			}
		}
		break;

		case WIZ_SKILLPT_CHANGE:
		{
			int iType = pkt.read<uint8_t>();
			int iValue = pkt.read<uint8_t>();

			m_PlayerMySelf.iSkillInfo[iType] = (uint8_t)iValue;
			m_PlayerMySelf.iSkillInfo[0]++;

#ifdef DEBUG
			printf("RecvProcess::WIZ_ITEM_REMOVE: %d,%d,%d\n", iType, iValue, m_PlayerMySelf.iSkillInfo[0]);
#endif
		}
		break;

		case WIZ_CLASS_CHANGE:
		{
			uint8_t	iType = pkt.read<uint8_t>();

			switch (iType)
			{
				case N3_SP_CLASS_CHANGE_PURE:
				{
#ifdef DEBUG
					printf("RecvProcess::WIZ_CLASS_CHANGE: N3_SP_CLASS_CHANGE_PURE\n");
#endif
				}
				break;

				case N3_SP_CLASS_CHANGE_REQ:
				{
					SubPacketClassChange eSP = (SubPacketClassChange)pkt.read<uint8_t>();

					switch (eSP)
					{
						case N3_SP_CLASS_CHANGE_SUCCESS:
						{
#ifdef DEBUG
							printf("RecvProcess::WIZ_CLASS_CHANGE: N3_SP_CLASS_CHANGE_SUCCESS\n");
#endif
						}
						break;

						case N3_SP_CLASS_CHANGE_NOT_YET:
						{
#ifdef DEBUG
							printf("RecvProcess::WIZ_CLASS_CHANGE: N3_SP_CLASS_CHANGE_NOT_YET\n");
#endif
						}
						break;

						case N3_SP_CLASS_CHANGE_ALREADY:
						{
#ifdef DEBUG
							printf("RecvProcess::WIZ_CLASS_CHANGE: N3_SP_CLASS_CHANGE_ALREADY\n");
#endif
						}
						break;

						case N3_SP_CLASS_CHANGE_FAILURE:
						{
#ifdef DEBUG
							printf("RecvProcess::WIZ_CLASS_CHANGE: N3_SP_CLASS_CHANGE_FAILURE\n");
#endif
						}
						break;
					}
				}
				break;

				case N3_SP_CLASS_ALL_POINT:
				{
					uint8_t	iType = pkt.read<uint8_t>();
					uint32_t iGold = pkt.read<uint32_t>();

					switch (iType)
					{
						case 0x00: 
						{
						}
						break;

						case 0x01:
						{
							m_PlayerMySelf.iStrength = (uint8_t)pkt.read<int16_t>();
							m_PlayerMySelf.iStamina = (uint8_t)pkt.read<int16_t>();
							m_PlayerMySelf.iDexterity = (uint8_t)pkt.read<int16_t>();
							m_PlayerMySelf.iIntelligence = (uint8_t)pkt.read<int16_t>();
							m_PlayerMySelf.iMagicAttak = (uint8_t)pkt.read<int16_t>();

							m_PlayerMySelf.iHPMax = pkt.read<int16_t>();
							m_PlayerMySelf.iMSPMax = pkt.read<int16_t>();
							m_PlayerMySelf.iAttack = pkt.read<int16_t>();
							m_PlayerMySelf.iWeightMax = pkt.read<uint32_t>();

							m_PlayerMySelf.iBonusPointRemain = pkt.read<int16_t>();

							m_PlayerMySelf.iGold = iGold;

#ifdef DEBUG
							printf("RecvProcess::WIZ_CLASS_CHANGE: Stat point reset\n");
							printf("RecvProcess::WIZ_CLASS_CHANGE: %d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
								m_PlayerMySelf.iStrength,
								m_PlayerMySelf.iStamina,
								m_PlayerMySelf.iDexterity,
								m_PlayerMySelf.iIntelligence,
								m_PlayerMySelf.iMagicAttak,
								m_PlayerMySelf.iHPMax,
								m_PlayerMySelf.iMSPMax,
								m_PlayerMySelf.iAttack,
								m_PlayerMySelf.iWeightMax,
								m_PlayerMySelf.iBonusPointRemain,
								m_PlayerMySelf.iGold);

#endif
						}
						break;

						case 0x02: 
						{
						}
						break;
					}
				}
				break;

				case N3_SP_CLASS_SKILL_POINT:
				{
					uint8_t	iType = pkt.read<uint8_t>();
					uint32_t iGold = pkt.read<uint32_t>();

					switch (iType)
					{
						case 0x00: 
						{
						}
						break;

						case 0x01:
						{
							m_PlayerMySelf.iSkillInfo[0] = pkt.read<uint8_t>();

							for (int i = 1; i < 9; i++)
								m_PlayerMySelf.iSkillInfo[i] = 0;
#ifdef DEBUG
							printf("RecvProcess::WIZ_CLASS_CHANGE: Skill point reset, new points: %d\n",
								m_PlayerMySelf.iSkillInfo[0]);
#endif
						}
						break;

						case 0x02: 
						{
						}
						break;
					}
				}
				break;

				case N3_SP_CLASS_POINT_CHANGE_PRICE_QUERY:
				{
					uint32_t iGold = pkt.read<uint32_t>();
#ifdef DEBUG
					printf("RecvProcess::WIZ_CLASS_CHANGE: Point change price %d\n", iGold);
#endif
				}
				break;

				case N3_SP_CLASS_PROMOTION:
				{
					uint16_t iClass = pkt.read<uint16_t>();
					uint32_t iID = pkt.read<uint32_t>();

					if (m_PlayerMySelf.iID == iID)
					{
						m_PlayerMySelf.eClass = (Class)iClass;
#ifdef DEBUG
						printf("RecvProcess::WIZ_CLASS_CHANGE: %s class changed to %d\n",
							m_PlayerMySelf.szName.c_str(), m_PlayerMySelf.eClass);
#endif
					}
					else
					{
						Guard lock(m_vecPlayerLock);
						auto it = std::find_if(m_vecPlayer.begin(), m_vecPlayer.end(),
							[iID](const TPlayer& a) { return a.iID == iID; });

						if (it != m_vecPlayer.end())
						{
							it->eClass = (Class)iClass;
#ifdef DEBUG
							printf("RecvProcess::WIZ_CLASS_CHANGE: %s class changed to %d\n",
								it->szName.c_str(), it->eClass);
#endif
						}
					}
				}
				break;
			}
		}
		break;

		case WIZ_GOLD_CHANGE:
		{
			uint8_t iType = pkt.read<uint8_t>();
			uint32_t iGoldOffset = pkt.read<uint32_t>();
			uint32_t iGold = pkt.read<uint32_t>();

			m_PlayerMySelf.iGold = iGold;

#ifdef DEBUG
			printf("RecvProcess::WIZ_GOLD_CHANGE: %d,%d,%d\n", iType, iGoldOffset, iGold);
#endif
		}
		break;

		case WIZ_ITEM_MOVE:
		{
			uint8_t iType = pkt.read<uint8_t>();

			if (iType != 0)
			{
				uint8_t iSubType = pkt.read<uint8_t>();

				if (iSubType != 0)
				{
					m_PlayerMySelf.iAttack = pkt.read<int16_t>();
					m_PlayerMySelf.iGuard = pkt.read<int16_t>();
					m_PlayerMySelf.iWeightMax = pkt.read<uint32_t>();

					uint16_t iUnknown1 = pkt.read<uint16_t>();

					m_PlayerMySelf.iHPMax = pkt.read<int16_t>();
					m_PlayerMySelf.iMSPMax = pkt.read<int16_t>();

					m_PlayerMySelf.iStrength_Delta = (uint8_t)pkt.read<uint16_t>();
					m_PlayerMySelf.iStamina_Delta = (uint8_t)pkt.read<uint16_t>();
					m_PlayerMySelf.iDexterity_Delta = (uint8_t)pkt.read<uint16_t>();
					m_PlayerMySelf.iIntelligence_Delta = (uint8_t)pkt.read<uint16_t>();
					m_PlayerMySelf.iMagicAttak_Delta = (uint8_t)pkt.read<uint16_t>();

					m_PlayerMySelf.iRegistFire = (uint8_t)pkt.read<uint16_t>();
					m_PlayerMySelf.iRegistCold = (uint8_t)pkt.read<uint16_t>();
					m_PlayerMySelf.iRegistLight = (uint8_t)pkt.read<uint16_t>();
					m_PlayerMySelf.iRegistMagic = (uint8_t)pkt.read<uint16_t>();
					m_PlayerMySelf.iRegistCurse = (uint8_t)pkt.read<uint16_t>();
					m_PlayerMySelf.iRegistPoison = (uint8_t)pkt.read<uint16_t>();

#ifdef DEBUG
					printf("RecvProcess::WIZ_ITEM_MOVE: %d,%d,%d,Unknown1(%d),%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
						m_PlayerMySelf.iAttack,
						m_PlayerMySelf.iGuard,
						m_PlayerMySelf.iWeightMax,
						iUnknown1,
						m_PlayerMySelf.iHPMax,
						m_PlayerMySelf.iMSPMax,
						m_PlayerMySelf.iStrength_Delta,
						m_PlayerMySelf.iStamina_Delta,
						m_PlayerMySelf.iDexterity_Delta,
						m_PlayerMySelf.iIntelligence_Delta,
						m_PlayerMySelf.iMagicAttak_Delta,
						m_PlayerMySelf.iRegistFire,
						m_PlayerMySelf.iRegistCold,
						m_PlayerMySelf.iRegistLight,
						m_PlayerMySelf.iRegistMagic,
						m_PlayerMySelf.iRegistCurse,
						m_PlayerMySelf.iRegistPoison);
#endif
				}
				else
				{
#ifdef DEBUG
					printf("RecvProcess::WIZ_ITEM_MOVE: %d SubType Not Implemented\n", iSubType);
#endif
				}
			}
			else
			{
#ifdef DEBUG
				printf("RecvProcess::WIZ_ITEM_MOVE: %d Type Not Implemented\n", iType);
#endif
			}
		}
		break;

		case WIZ_LOYALTY_CHANGE:
		{
			uint8_t bType = pkt.read<uint8_t>();

			if (bType == 1)
			{
				uint32_t iLoyalty = pkt.read<uint32_t>();
				uint32_t iLoyaltyMonthly = pkt.read<uint32_t>();
				uint32_t iUnknown1 = pkt.read<uint32_t>();
				uint32_t iClanLoyaltyAmount = pkt.read<uint32_t>();

#ifdef DEBUG
				printf("RecvProcess::WIZ_LOYALTY_CHANGE: %d,%d,%d,Unknown1(%d),%d\n",
					bType, iLoyalty, iLoyaltyMonthly, iUnknown1, iClanLoyaltyAmount);
#endif
			}
			else
			{
#ifdef DEBUG
				printf("RecvProcess::WIZ_LOYALTY_CHANGE: %d Type Not Implemented\n", bType);
#endif
			}
		}
		break;

		case WIZ_GAMESTART:
		{
#ifdef DEBUG
			printf("RecvProcess::WIZ_GAMESTART: Started\n");
#endif
			new std::thread([this]()
			{
				WaitCondition(GetConfiguration()->GetConfigMap()->size() == 0)

				bool bWallHack = GetConfiguration()->GetBool(skCryptDec("Feature"), skCryptDec("WallHack"), false);

				if (bWallHack)
					SetAuthority(0);

				bool bOreads = GetConfiguration()->GetBool(skCryptDec("Feature"), skCryptDec("Oreads"), false);

				if (bOreads)
				{
					EquipOreads(700039000);
					SetOreads(bOreads);
				}

				bool bDeathEffect = GetConfiguration()->GetBool(skCryptDec("Feature"), skCryptDec("DeathEffect"), false);

				if (bDeathEffect)
				{
					PatchDeathEffect(true);
				}
			});
		}
		break;

		case WIZ_REQ_NPCIN:
		{
			int16_t iNpcCount = pkt.read<int16_t>();

			if (0 == iNpcCount)
				if (iNpcCount < 0 || iNpcCount >= 1000)
					return;

			Guard lock(m_vecNpcLock);

			for (int16_t i = 0; i < iNpcCount; i++)
			{
				auto pNpc = InitializeNpc(pkt);

				int32_t iNpcID = pNpc.iID;
				auto it = std::find_if(m_vecNpc.begin(), m_vecNpc.end(),
					[iNpcID](const TNpc& a) { return a.iID == iNpcID; });

				if (it == m_vecNpc.end())
					m_vecNpc.push_back(pNpc);
				else
					*it = pNpc;
			}

#ifdef DEBUG
			printf("RecvProcess::WIZ_REQ_NPCIN: Size %d\n", iNpcCount);
#endif
		}
		break;

		case WIZ_REQ_USERIN:
		{
			int16_t iUserCount = pkt.read<int16_t>();

			if (0 == iUserCount)
				if (iUserCount < 0 || iUserCount >= 1000)
					return;

			Guard lock(m_vecPlayerLock);
			for (int16_t i = 0; i < iUserCount; i++)
			{
				uint8_t iUnknown0 = pkt.read<uint8_t>();

				auto pUser = InitializePlayer(pkt);
				
				auto it = std::find_if(m_vecPlayer.begin(), m_vecPlayer.end(),
					[pUser](const TPlayer& a) { return a.iID == pUser.iID; });

				if (it == m_vecPlayer.end())
					m_vecPlayer.push_back(pUser);
				else
					*it = pUser;
			}

#ifdef DEBUG
			printf("RecvProcess::WIZ_REQ_USERIN: Size %d\n", iUserCount);
#endif
		}
		break;

		case WIZ_NPC_INOUT:
		{
			uint8_t iType = pkt.read<uint8_t>();

			switch (iType)
			{
				case InOut::INOUT_IN:
				{
					auto pNpc = InitializeNpc(pkt);

					Guard lock(m_vecNpcLock);
					auto it = std::find_if(m_vecNpc.begin(), m_vecNpc.end(),
						[pNpc](const TNpc& a) { return a.iID == pNpc.iID; });

					if (it == m_vecNpc.end())
						m_vecNpc.push_back(pNpc);
					else
						*it = pNpc;

#ifdef DEBUG
					printf("RecvProcess::WIZ_NPC_INOUT: %d,%d\n", iType, pNpc.iID);
#endif
				}
				break;

				case InOut::INOUT_OUT:
				{
					int32_t iNpcID = pkt.read<int32_t>();

					Guard lock(m_vecNpcLock);
					m_vecNpc.erase(
						std::remove_if(m_vecNpc.begin(), m_vecNpc.end(),
							[iNpcID](const TNpc& a) { return a.iID == iNpcID; }),
						m_vecNpc.end());

#ifdef DEBUG
					printf("RecvProcess::WIZ_NPC_INOUT: %d,%d\n", iType, iNpcID);
#endif
				}
				break;

				default:
				{
#ifdef DEBUG
					printf("RecvProcess::WIZ_NPC_INOUT: %d not implemented\n", iType);
#endif
				}
				break;
			}
		}
		break;

		case WIZ_USER_INOUT:
		{
			uint8_t iType = pkt.read<uint8_t>();
			uint8_t iUnknown0 = pkt.read<uint8_t>();

			switch (iType)
			{
				case InOut::INOUT_IN:
				case InOut::INOUT_RESPAWN:
				case InOut::INOUT_WARP:
				{
					auto pPlayer = InitializePlayer(pkt);

					Guard lock(m_vecPlayerLock);
					auto it = std::find_if(m_vecPlayer.begin(), m_vecPlayer.end(),
						[pPlayer](const TPlayer& a) { return a.iID == pPlayer.iID; });

					if (it == m_vecPlayer.end())
						m_vecPlayer.push_back(pPlayer);
					else
						*it = pPlayer;

#ifdef DEBUG
					printf("RecvProcess::WIZ_USER_INOUT: %d,%d\n", iType, pPlayer.iID);
#endif
				}
				break;

				case InOut::INOUT_OUT:
				{
					int32_t iPlayerID = pkt.read<int32_t>();

					Guard lock(m_vecPlayerLock);
					m_vecPlayer.erase(
						std::remove_if(m_vecPlayer.begin(), m_vecPlayer.end(),
							[iPlayerID](const TPlayer& a) { return a.iID == iPlayerID; }),
						m_vecPlayer.end());

#ifdef DEBUG
					printf("RecvProcess::WIZ_USER_INOUT: %d,%d\n", iType, iPlayerID);
#endif
				}
				break;

				default:
				{
#ifdef DEBUG
					printf("RecvProcess::WIZ_USER_INOUT: %d not implemented\n", iType);
#endif
				}

				break;
			}
		}
		break;

		case WIZ_NPC_REGION:
		{
			int16_t iNpcCount = pkt.read<int16_t>();

#ifdef DEBUG
			printf("RecvProcess::WIZ_NPC_REGION: New npc count %d\n", iNpcCount);
#endif
		}
		break;

		case WIZ_REGIONCHANGE:
		{
			uint8_t iType = pkt.read<uint8_t>();

			switch (iType)
			{
				case 1:
				{
					int16_t iUserCount = pkt.read<int16_t>();

#ifdef DEBUG
					printf("RecvProcess::WIZ_REGIONCHANGE: New user count %d\n", iUserCount);
#endif
				}
				break;

				default:
				{
#ifdef DEBUG
					printf("RecvProcess::WIZ_REGIONCHANGE: Type %d not implemented!\n", iType);
#endif
				}
				break;
			}
		}
		break;

		case WIZ_DEAD:
		{
			int32_t iID = pkt.read<int32_t>();

			if (m_PlayerMySelf.iID == iID)
			{
				m_PlayerMySelf.eState = PSA_DEATH;

#ifdef DEBUG
				printf("RecvProcess::WIZ_DEAD: MySelf Dead\n");
#endif
			}
			else
			{
				if (iID >= 5000)
				{
					Guard lock(m_vecNpcLock);
					auto it = std::find_if(m_vecNpc.begin(), m_vecNpc.end(),
						[iID](const TNpc& a) { return a.iID == iID; });

					if (it != m_vecNpc.end())
					{
						it->eState = PSA_DEATH;

#ifdef DEBUG
						printf("RecvProcess::WIZ_DEAD: %d Npc Dead\n", iID);
#endif
					}
				}
				else
				{
					Guard lock(m_vecPlayerLock);
					auto it = std::find_if(m_vecPlayer.begin(), m_vecPlayer.end(),
						[iID](const TPlayer& a) { return a.iID == iID; });

					if (it != m_vecPlayer.end())
					{
						it->eState = PSA_DEATH;

#ifdef DEBUG
						printf("RecvProcess::WIZ_DEAD: %d Player Dead\n", iID);
#endif
					}
				}
			}
		}
		break;

		case WIZ_ATTACK:
		{
			uint8_t iType = pkt.read<uint8_t>();
			uint8_t iResult = pkt.read<uint8_t>();

			switch (iResult)
			{
				case ATTACK_FAIL:
				{
					int32_t iAttackID = pkt.read<int32_t>();
					int32_t iTargetID = pkt.read<int32_t>();

#ifdef DEBUG
					printf("RecvProcess::WIZ_ATTACK: %d,%d FAIL\n", iAttackID, iTargetID);
#endif
				}
				break;

				case ATTACK_SUCCESS:
				{
					int32_t iAttackID = pkt.read<int32_t>();
					int32_t iTargetID = pkt.read<int32_t>();

#ifdef DEBUG
					printf("RecvProcess::WIZ_ATTACK: %d,%d SUCCESS\n", iAttackID, iTargetID);
#endif
				}
				break;

				case ATTACK_TARGET_DEAD:
				case ATTACK_TARGET_DEAD_OK:
				{
					int32_t iAttackID = pkt.read<int32_t>();
					int32_t iTargetID = pkt.read<int32_t>();

					if (iTargetID >= 5000)
					{
						Guard lock(m_vecNpcLock);
						auto it = std::find_if(m_vecNpc.begin(), m_vecNpc.end(),
							[iTargetID](const TNpc& a) { return a.iID == iTargetID; });

						if (it != m_vecNpc.end())
						{
							it->eState = PSA_DEATH;

#ifdef DEBUG
							printf("RecvProcess::WIZ_ATTACK: TARGET_DEAD | TARGET_DEAD_OK - %d Npc Dead\n", iTargetID);
#endif
						}
					}
					else
					{
						Guard lock(m_vecPlayerLock);
						auto it = std::find_if(m_vecPlayer.begin(), m_vecPlayer.end(),
							[iTargetID](const TPlayer& a) { return a.iID == iTargetID; });

						if (it != m_vecPlayer.end())
						{
							it->eState = PSA_DEATH;

#ifdef DEBUG
							printf("RecvProcess::WIZ_ATTACK: TARGET_DEAD | TARGET_DEAD_OK - %d Player Dead\n", iTargetID);
#endif
						}
					}
				}
				break;

				default:
				{
#ifdef DEBUG
					printf("RecvProcess::WIZ_ATTACK: %d not implemented\n", iResult);
#endif
				}
				break;

			}
		}
		break;

		case WIZ_TARGET_HP:
		{
			int32_t iID = pkt.read<int32_t>();
			uint8_t iUpdateImmediately = pkt.read<uint8_t>();

			int32_t iTargetHPMax = pkt.read<int32_t>();
			int32_t iTargetHPCur = pkt.read<int32_t>();

			int16_t iTargetHPChange = pkt.read<int16_t>();

			if (m_PlayerMySelf.iID == iID)
			{
				m_PlayerMySelf.iHPMax = iTargetHPMax;
				m_PlayerMySelf.iHP = iTargetHPCur;

				if (m_PlayerMySelf.iHPMax > 0 && m_PlayerMySelf.iHP <= 0)
					m_PlayerMySelf.eState = PSA_DEATH;

#ifdef DEBUG
				printf("RecvProcess::WIZ_TARGET_HP: %s, %d / %d\n",
					m_PlayerMySelf.szName.c_str(), m_PlayerMySelf.iHP, m_PlayerMySelf.iHPMax);
#endif
			}
			else
			{
				if (iID >= 5000)
				{
					Guard lock(m_vecNpcLock);
					auto it = std::find_if(m_vecNpc.begin(), m_vecNpc.end(),
						[iID](const TNpc& a) { return a.iID == iID; });

					if (it != m_vecNpc.end())
					{
						it->iHPMax = iTargetHPMax;
						it->iHP = iTargetHPCur;

						if (it->iHPMax > 0 && it->iHP <= 0)
							it->eState = PSA_DEATH;

#ifdef DEBUG
						printf("RecvProcess::WIZ_TARGET_HP: %d, %d / %d\n", iID, it->iHP, it->iHPMax);
#endif
					}
				}
				else
				{
					Guard lock(m_vecPlayerLock);
					auto it = std::find_if(m_vecPlayer.begin(), m_vecPlayer.end(),
						[iID](const TPlayer& a) { return a.iID == iID; });

					if (it != m_vecPlayer.end())
					{
						it->iHPMax = iTargetHPMax;
						it->iHP = iTargetHPCur;

						if (it->iHPMax > 0 && it->iHP <= 0)
							it->eState = PSA_DEATH;

#ifdef DEBUG
						printf("RecvProcess::WIZ_TARGET_HP: %s, %d / %d\n",
							it->szName.c_str(), it->iHP, it->iHPMax);
#endif
					}
				}
			}
		}
		break;

		case WIZ_MOVE:
		{
			int32_t iID = pkt.read<int32_t>();

			float fX = pkt.read<uint16_t>() / 10.0f;
			float fY = pkt.read<uint16_t>() / 10.0f;
			float fZ = pkt.read<int16_t>() / 10.0f;

			int16_t iSpeed = pkt.read<int16_t>();
			uint8_t iMoveType = pkt.read<uint8_t>();

			if (m_PlayerMySelf.iID == iID)
			{
				m_PlayerMySelf.fX = fX;
				m_PlayerMySelf.fY = fY;
				m_PlayerMySelf.fZ = fZ;

				m_PlayerMySelf.iMoveSpeed = iSpeed;
				m_PlayerMySelf.iMoveType = iMoveType;
			}
			else
			{
				Guard lock(m_vecPlayerLock);
				auto it = std::find_if(m_vecPlayer.begin(), m_vecPlayer.end(),
					[iID](const TPlayer& a) { return a.iID == iID; });

				if (it != m_vecPlayer.end())
				{
					it->fX = fX;
					it->fY = fY;
					it->fZ = fZ;

					it->iMoveSpeed = iSpeed;
					it->iMoveType = iMoveType;
				}
#ifdef DEBUG
				else
					printf("RecvProcess::WIZ_MOVE: %d not in m_vecPlayer list, is ghost player\n", iID);
#endif
			}

#ifdef DEBUG
			//printf("RecvProcess::WIZ_MOVE: %d,%f,%f,%f,%d,%d\n", 
			//	iID, fX, fY, fZ, iSpeed, iMoveType);
#endif
		}
		break;

		case WIZ_NPC_MOVE:
		{
			uint8_t iMoveType = pkt.read<uint8_t>();

			int32_t iID = pkt.read<int32_t>();

			float fX = pkt.read<uint16_t>() / 10.0f;
			float fY = pkt.read<uint16_t>() / 10.0f;
			float fZ = pkt.read<int16_t>() / 10.0f;

			uint16_t iSpeed = pkt.read<uint16_t>();

			Guard lock(m_vecNpcLock);
			auto it = std::find_if(m_vecNpc.begin(), m_vecNpc.end(),
				[iID](const TNpc& a) { return a.iID == iID; });

			if (it != m_vecNpc.end())
			{
				it->fX = fX;
				it->fY = fY;
				it->fZ = fZ;

				it->iMoveSpeed = iSpeed;
				it->iMoveType = iMoveType;
			}
#ifdef DEBUG
			else
				printf("RecvProcess::WIZ_NPC_MOVE: %d not in m_vecNpc list, is ghost npc\n", iID);
#endif

#ifdef DEBUG
			//printf("RecvProcess::WIZ_NPC_MOVE: %d,%d,%f,%f,%f,%d\n",
			//	iMoveType, iID, fX, fY, fZ, iSpeed);
#endif
		}
		break;

		case WIZ_MAGIC_PROCESS:
		{
			uint8_t iType = pkt.read<uint8_t>();

			switch (iType)
			{
				case SkillMagicType::SKILL_MAGIC_TYPE_EFFECTING:
				{
					uint32_t iSkillID = pkt.read<uint32_t>();
					int32_t iSourceID = pkt.read<int32_t>();
					int32_t iTargetID = pkt.read<int32_t>();

					auto pSkillTable = m_Bot->GetSkillTable()->GetData();
					auto pSkillData = pSkillTable.find(iSkillID);

					if (pSkillData != pSkillTable.end())
					{
						if (pSkillData->second.dw1stTableType == 4)
						{
							if (iTargetID == GetID())
							{
								auto pSkillExtension4 = m_Bot->GetSkillExtension4Table()->GetData();
								auto pSkillExtension4Data = pSkillExtension4.find(pSkillData->second.iID);

								if (pSkillExtension4Data != pSkillExtension4.end())
								{
									Guard lock(m_mapActiveBuffListLock);
									auto it = m_mapActiveBuffList.find(pSkillExtension4Data->second.iBuffType);

									if (it != m_mapActiveBuffList.end())
										m_mapActiveBuffList.erase(pSkillExtension4Data->second.iBuffType);

									m_mapActiveBuffList.insert(std::pair(pSkillExtension4Data->second.iBuffType, pSkillData->second.iID));

									switch (pSkillData->second.iBaseId)
									{
									case 101001:
									case 107010:
										m_PlayerMySelf.iMoveSpeed = 67;
										break;

									case 107725:
										m_PlayerMySelf.iMoveSpeed = 90;
										break;
									}

#ifdef DEBUG
									printf("RecvProcess::WIZ_MAGIC_PROCESS: %s %s Buff added\n", GetName().c_str(), pSkillData->second.szEngName.c_str());
#endif
								}
#ifdef DEBUG
								else
								{
									printf("RecvProcess::WIZ_MAGIC_PROCESS: %s %s Buff added but extension not		exist\n", GetName().c_str(), pSkillData->second.szEngName.c_str());
								}
#endif
							}
							else
							{
								if (iTargetID < 5000)
								{
									Guard lock(m_vecPlayerLock);
									auto it = std::find_if(m_vecPlayer.begin(), m_vecPlayer.end(),
										[iTargetID](const TPlayer& a) { return a.iID == iTargetID; });

									if (it != m_vecPlayer.end())
									{
										switch (pSkillData->second.iBaseId)
										{
										case 101001:
										case 107010:
											it->iMoveSpeed = 67;
											break;

										case 107725:
											it->iMoveSpeed = 90;
											break;
										}

#ifdef DEBUG
										printf("RecvProcess::WIZ_MAGIC_PROCESS: %s %s Buff added\n", it->szName.c_str(), pSkillData->second.szEngName.c_str());
#endif
									}
								}
							}
						}
					}
				}
				break;

				case SkillMagicType::SKILL_MAGIC_TYPE_FAIL:
				{
					uint32_t iSkillID = pkt.read<uint32_t>();
					int32_t iSourceID = pkt.read<int32_t>();
					int32_t iTargetID = pkt.read<int32_t>();

					int32_t iData[6];

					for (size_t i = 0; i < 6; i++)
						iData[i] = pkt.read<int32_t>();

					if (iData[3] == -100 || iData[3] == -103)
					{
#ifdef DEBUG
						printf("RecvProcess::WIZ_MAGIC_PROCESS: %d - Skill failed %d\n", iSkillID, iData[3]);
#endif
						Client::SetSkillUseTime(iSkillID, (std::chrono::milliseconds)0);
					}
				}
				break;

				case SkillMagicType::SKILL_MAGIC_TYPE_BUFF:
				{
					uint8_t iBuffType = pkt.read<uint8_t>();

					Guard lock(m_mapActiveBuffListLock);
					auto it = m_mapActiveBuffList.find(iBuffType);

					if (it != m_mapActiveBuffList.end())
					{
						auto pSkillTable = m_Bot->GetSkillTable()->GetData();
						auto pSkillData = pSkillTable.find(it->second);

#ifdef DEBUG
						if (pSkillData != pSkillTable.end())
							printf("RecvProcess::WIZ_MAGIC_PROCESS: %s %s buff removed\n", GetName().c_str(), pSkillData->second.szEngName.c_str());
						else
							printf("RecvProcess::WIZ_MAGIC_PROCESS: %s %d buff removed\n", GetName().c_str(), it->second);
#endif
					}

					switch (iBuffType)
					{
					case (byte)BuffType::BUFF_TYPE_SPEED:
						m_PlayerMySelf.iMoveSpeed = 45;
						break;
					}

					m_mapActiveBuffList.erase(iBuffType);
				}
				break;
				default:
				{
#ifdef DEBUG
					printf("RecvProcess::WIZ_MAGIC_PROCESS: %d Type Not Implemented!\n", iType);
#endif
				}
				break;
			}
		}
		break;

		case WIZ_STATE_CHANGE:
		{
			int32_t iID = pkt.read<int32_t>();
			uint8_t iType = pkt.read<uint8_t>();
			uint64_t iBuff = pkt.read<uint64_t>();

			switch (iType)
			{
				case 3:
				{
					switch (iBuff)
					{
						case 4:
						{
							if (m_PlayerMySelf.iID == iID)
							{
#ifdef DEBUG
								printf("RecvProcess::WIZ_STATE_CHANGE: %s blinking start\n", m_PlayerMySelf.szName.c_str());
#endif

								m_PlayerMySelf.bBlinking = true;
							}
							else
							{
								Guard lock(m_vecPlayerLock);
								auto it = std::find_if(m_vecPlayer.begin(), m_vecPlayer.end(),
									[iID](const TPlayer& a) { return a.iID == iID; });

								if (it != m_vecPlayer.end())
								{
#ifdef DEBUG
									printf("RecvProcess::WIZ_STATE_CHANGE: %s blinking start\n", it->szName.c_str());
#endif

									it->bBlinking = true;
								}
							}
						}
						break;

						case 7:
						{
							if (m_PlayerMySelf.iID == iID)
							{
#ifdef DEBUG
								printf("RecvProcess::WIZ_STATE_CHANGE: %s blinking end\n", m_PlayerMySelf.szName.c_str());
#endif

								m_PlayerMySelf.bBlinking = false;
							}
							else
							{
								Guard lock(m_vecPlayerLock);
								auto it = std::find_if(m_vecPlayer.begin(), m_vecPlayer.end(),
									[iID](const TPlayer& a) { return a.iID == iID; });

								if (it != m_vecPlayer.end())
								{
#ifdef DEBUG
									printf("RecvProcess::WIZ_STATE_CHANGE: %s blinking end\n", it->szName.c_str());
#endif

									it->bBlinking = false;
								}
							}
						}
						break;

						default:
						{
#ifdef DEBUG
							printf("RecvProcess::WIZ_STATE_CHANGE: Abnormal Type - %llu Buff not implemented\n", iBuff);
#endif
						}
						break;
					}
				}
				break;

				default:
				{
#ifdef DEBUG
					printf("RecvProcess::WIZ_STATE_CHANGE: Type %d not implemented\n", iType);
#endif
				}
				break;
			}
		}
		break;

		case WIZ_SHOPPING_MALL:
		{
			uint8_t iType = pkt.read<uint8_t>();

			switch (iType)
			{
				case ShoppingMallType::STORE_CLOSE:
				{
					for (int i = SLOT_MAX; i < SLOT_MAX + HAVE_MAX; i++)
					{
						m_PlayerMySelf.tInventory[i].iPos = i;
						m_PlayerMySelf.tInventory[i].iItemID = pkt.read<uint32_t>();
						m_PlayerMySelf.tInventory[i].iDurability = pkt.read<uint16_t>();
						m_PlayerMySelf.tInventory[i].iCount = pkt.read<uint16_t>();
						m_PlayerMySelf.tInventory[i].iFlag = pkt.read<uint8_t>();
						m_PlayerMySelf.tInventory[i].iRentalTime = pkt.read<int16_t>();
						m_PlayerMySelf.tInventory[i].iSerial = pkt.read<uint32_t>();
					}

#ifdef DEBUG
					printf("RecvProcess::WIZ_SHOPPING_MALL: STORE_CLOSE\n");
#endif
				}
				break;

				default:
				{
#ifdef DEBUG
					printf("RecvProcess::WIZ_SHOPPING_MALL: Type %d not implemented\n", iType);
#endif
				}
				break;
			}
		}
		break;

		case WIZ_ITEM_TRADE:
		{
			uint8_t iType = pkt.read<uint8_t>();

			switch (iType)
			{
				case 0:
				{
					uint8_t iErrorCode = pkt.read<uint8_t>();
#ifdef DEBUG
					printf("RecvProcess::WIZ_ITEM_TRADE: Error Code: %d\n", iErrorCode);
#endif
				}
				break;

				case 1:
				{
					uint32_t iGold = pkt.read<uint32_t>();
					uint32_t iTransactionFee = pkt.read<uint32_t>();
					uint8_t iSellingGroup = pkt.read<uint8_t>();

					SendShoppingMall(ShoppingMallType::STORE_CLOSE);

#ifdef DEBUG
					printf("RecvProcess::WIZ_ITEM_TRADE: %d,%d,%d\n", iGold, iTransactionFee, iSellingGroup);
#endif
				}
				break;

				default:
				{
#ifdef DEBUG
					printf("RecvProcess::WIZ_ITEM_TRADE: Type %d not implemented\n", iType);
#endif
				}
				break;
			}
		}
		break;

		case WIZ_MAP_EVENT:
		{
			uint8_t iType = pkt.read<uint8_t>();

			switch (iType)
			{
				case 9:
				{
					m_bLunarWarDressUp = pkt.read<uint8_t>();
				}
				break;

				default:
				{
#ifdef DEBUG
					printf("RecvProcess::WIZ_MAP_EVENT: Type %d not implemented\n", iType);
#endif
				}
				break;
			}
		}
		break;

		case WIZ_EXCHANGE:
		{
			uint8_t iType = pkt.read<uint8_t>();

			switch (iType)
			{
				case TradeSubPacket::TRADE_REQUEST:
				{
				}
				break;

				case TradeSubPacket::TRADE_AGREE:
				{
				}
				break;

				case TradeSubPacket::TRADE_ADD:
				{
				}
				break;

				case TradeSubPacket::TRADE_OTHER_ADD:
				{
				}
				break;

				case TradeSubPacket::TRADE_DECIDE:
				{
				}
				break;

				case TradeSubPacket::TRADE_OTHER_DECIDE:
				{
				}
				break;

				case TradeSubPacket::TRADE_DONE:
				{
					uint8_t iResult = pkt.read<uint8_t>();

					if (iResult == 1)
					{
						m_PlayerMySelf.iGold = pkt.read<uint32_t>();

						int16_t iItemCount = pkt.read<int16_t>();

						for (int32_t i = 0; i < iItemCount; i++)
						{
							uint8_t iItemPos = pkt.read<uint8_t>();

							m_PlayerMySelf.tInventory[14 + iItemPos].iItemID = pkt.read<uint32_t>();
							m_PlayerMySelf.tInventory[14 + iItemPos].iCount = pkt.read<int16_t>();
							m_PlayerMySelf.tInventory[14 + iItemPos].iDurability = pkt.read<int16_t>();

#ifdef DEBUG
							printf("RecvProcess::WIZ_EXCHANGE: TRADE_DONE - Item - %d,%d,%d \n",
								m_PlayerMySelf.tInventory[14 + iItemPos].iItemID,
								m_PlayerMySelf.tInventory[14 + iItemPos].iCount,
								m_PlayerMySelf.tInventory[14 + iItemPos].iDurability);
#endif
						}

#ifdef DEBUG
						printf("RecvProcess::WIZ_EXCHANGE: TRADE_DONE - Success - %d,%d \n", m_PlayerMySelf.iGold, iItemCount);
#endif
					}
#ifdef DEBUG
					else
						printf("RecvProcess::WIZ_EXCHANGE: TRADE_DONE - Failed\n");
#endif
				}
				break;

				case TradeSubPacket::TRADE_CANCEL:
				{
				}
				break;

				default:
				{
	#ifdef DEBUG
					printf("RecvProcess::WIZ_EXCHANGE: Type %d not implemented\n", iType);
	#endif
				}
				break;
			}
		}
		break;

		case WIZ_ROTATE:
		{
			int32_t iID = pkt.read<int32_t>();
			float fRotation = pkt.read<int16_t>() / 100.0f;

			if (m_PlayerMySelf.iID == iID)
			{
				m_PlayerMySelf.fRotation = fRotation;

#ifdef DEBUG
				printf("RecvProcess::WIZ_ROTATE: %s MySelf Rotate %f\n", m_PlayerMySelf.szName.c_str(), fRotation);
#endif
			}
			else
			{
				if (iID >= 5000)
				{
					Guard lock(m_vecNpcLock);
					auto it = std::find_if(m_vecNpc.begin(), m_vecNpc.end(),
						[iID](const TNpc& a) { return a.iID == iID; });

					if (it != m_vecNpc.end())
					{
						it->fRotation = fRotation;

#ifdef DEBUG
						printf("RecvProcess::WIZ_ROTATE: %d Npc Rotate %f\n", iID, fRotation);
#endif
					}
				}
				else
				{
					Guard lock(m_vecPlayerLock);
					auto it = std::find_if(m_vecPlayer.begin(), m_vecPlayer.end(),
						[iID](const TPlayer& a) { return a.iID == iID; });

					if (it != m_vecPlayer.end())
					{
						it->fRotation = fRotation;

#ifdef DEBUG
						printf("RecvProcess::WIZ_ROTATE: %s Player Rotate %f\n", it->szName.c_str(), fRotation);
#endif
					}
				}
			}
		}
		break;

		case WIZ_ITEM_DROP:
		{
			TLoot tLoot;
			memset(&tLoot, 0, sizeof(tLoot));

			tLoot.iNpcID = pkt.read<int32_t>();
			tLoot.iBundleID = pkt.read<uint32_t>();
			tLoot.iItemCount = pkt.read<uint8_t>();

			tLoot.msDropTime = duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());

			tLoot.iRequestedOpen = false;

			Guard lock(m_vecLootListLock);
			auto pLoot = std::find_if(m_vecLootList.begin(), m_vecLootList.end(),
				[tLoot](const TLoot a) { return a.iBundleID == tLoot.iBundleID; });

			if (pLoot == m_vecLootList.end())
				m_vecLootList.push_back(tLoot);
			else
				*pLoot = tLoot;

#ifdef DEBUG
			printf("RecvProcess::WIZ_ITEM_DROP: %d,%d,%d,%lld\n",
				tLoot.iNpcID,
				tLoot.iBundleID,
				tLoot.iItemCount,
				tLoot.msDropTime.count());
#endif
		}
		break;

		case WIZ_BUNDLE_OPEN_REQ:
		{
			uint32_t iBundleID = pkt.read<uint32_t>();
			uint8_t iResult = pkt.read<uint8_t>();

			Guard lock(m_vecLootListLock);
			auto pLoot = std::find_if(m_vecLootList.begin(), m_vecLootList.end(),
				[iBundleID](const TLoot a) { return a.iBundleID == iBundleID; });

			if (pLoot != m_vecLootList.end())
			{
				switch (iResult)
				{
					case 0:
					{
#ifdef DEBUG
						printf("RecvProcess::WIZ_BUNDLE_OPEN_REQ: Bundle open req failed\n");
#endif
					}
					break;

					case 1:
					{
						for (size_t i = 0; i < pLoot->iItemCount; i++)
						{
							uint32_t iItemID = pkt.read<uint32_t>();
							uint32_t iItemCount = pkt.read<int16_t>();

							SendBundleItemGet(iBundleID, iItemID, (int16_t)i);

#ifdef DEBUG
							printf("RecvProcess::WIZ_BUNDLE_OPEN_REQ: %d,%d,%d,%d\n", iBundleID, iItemID, iItemCount, (int16_t)i);
#endif
						}
					}
					break;

					default:
					{
#ifdef DEBUG
						printf("RecvProcess::WIZ_BUNDLE_OPEN_REQ: Result %d not implemented\n", iResult);
#endif
					}
					break;
				}

				Guard lock(m_vecLootListLock);
				m_vecLootList.erase(
					std::remove_if(m_vecLootList.begin(), m_vecLootList.end(),
						[iBundleID](const TLoot& a) { return a.iBundleID == iBundleID; }),
					m_vecLootList.end());
			}
		}
		break;

		case WIZ_ITEM_GET:
		{
			uint8_t iType = pkt.read<uint8_t>();

			switch (iType)
			{
				case 0x00:
				{
					uint32_t iBundleID = pkt.read<uint32_t>();

					Guard lock(m_vecLootListLock);
					m_vecLootList.erase(
						std::remove_if(m_vecLootList.begin(), m_vecLootList.end(),
							[iBundleID](const TLoot& a) { return a.iBundleID == iBundleID; }),
						m_vecLootList.end());

#ifdef DEBUG
					printf("RecvProcess::WIZ_ITEM_GET: %d,%d\n", iType, iBundleID);
#endif
				}
				break;

				case 0x01:
				case 0x02:
				case 0x05:
				{
					uint32_t iBundleID = pkt.read<uint32_t>();
					uint8_t iPos = pkt.read<uint8_t>();
					uint32_t iItemID = pkt.read<uint32_t>();

					uint16_t iItemCount = 0;

					if (iType == 1 || iType == 5)
						iItemCount = pkt.read<uint16_t>();

					uint32_t iGold = pkt.read<uint32_t>();

					m_PlayerMySelf.tInventory[14 + iPos].iItemID = iItemID;
					m_PlayerMySelf.tInventory[14 + iPos].iCount = iItemCount;

					m_PlayerMySelf.iGold = iGold;

					Guard lock(m_vecLootListLock);
					m_vecLootList.erase(
						std::remove_if(m_vecLootList.begin(), m_vecLootList.end(),
							[iBundleID](const TLoot& a) { return a.iBundleID == iBundleID; }),
						m_vecLootList.end());

#ifdef DEBUG
					printf("RecvProcess::WIZ_ITEM_GET: %d,%d,%d,%d,%d,%d\n", iType, iBundleID, iPos, iItemID, iItemCount, iGold);
#endif
				}
				break;

				case 0x03:
				{
					uint32_t iBundleID = pkt.read<uint32_t>();

					Guard lock(m_vecLootListLock);
					m_vecLootList.erase(
						std::remove_if(m_vecLootList.begin(), m_vecLootList.end(),
							[iBundleID](const TLoot& a) { return a.iBundleID == iBundleID; }),
						m_vecLootList.end());

#ifdef DEBUG
					printf("RecvProcess::WIZ_ITEM_GET: %d,%d\n", iType, iBundleID);
#endif
				}
				break;

				case 0x06:
				{
#ifdef DEBUG
					printf("RecvProcess::WIZ_ITEM_GET: Inventory Full\n");
#endif
				}
				break;


				default:
				{
#ifdef DEBUG
					printf("RecvProcess::WIZ_ITEM_GET: %d Type Not Implemented\n", iType);
#endif
				}
				break;
			}
		}
		break;

		case WIZ_ZONE_CHANGE:
		{
			uint8_t iOpCode = pkt.read<uint8_t>();

			switch (iOpCode)
			{
				case ZoneChangeOpcode::ZoneChangeLoading:
				{

				}
				break;
				case ZoneChangeOpcode::ZoneChangeLoaded:
				{

				}
				break;
				case ZoneChangeOpcode::ZoneChangeTeleport:
				{
					SetMovePosition(Vector3(0.0f, 0.0f, 0.0f));

					m_PlayerMySelf.iCity = (uint8_t)pkt.read<int16_t>();

					m_PlayerMySelf.fZ = (pkt.read<int16_t>() / 10.0f);
					m_PlayerMySelf.fX = (pkt.read<uint16_t>() / 10.0f);
					m_PlayerMySelf.fY = (pkt.read<uint16_t>() / 10.0f);

					uint8_t iVictoryNation = pkt.read<uint8_t>();

					m_Bot->GetWorld()->Load(m_PlayerMySelf.iCity);

					printf("RecvProcess::WIZ_ZONE_CHANGE: Teleport Zone: [%d] Coordinate: [%f - %f - %f] VictoryNation: [%d]\n",
						m_PlayerMySelf.iCity, m_PlayerMySelf.fX, m_PlayerMySelf.fY, m_PlayerMySelf.fZ, iVictoryNation);

				}
				break;
				case ZoneChangeOpcode::ZoneChangeMilitaryCamp:
				{

				}
				break;
			}
		}
		break;

		case WIZ_WARP:
		{
			SetMovePosition(Vector3(0.0f, 0.0f, 0.0f));

			m_PlayerMySelf.fX = (pkt.read<uint16_t>() / 10.0f);
			m_PlayerMySelf.fY = (pkt.read<uint16_t>() / 10.0f);
			m_PlayerMySelf.fZ = (pkt.read<int16_t>() / 10.0f);
		}
		break;
	}
}

void ClientHandler::SendProcess(BYTE* byBuffer, DWORD dwLength)
{
	Packet pkt = Packet(byBuffer[0], (size_t)dwLength);
	pkt.append(&byBuffer[1], dwLength - 1);

#ifdef DEBUG
#ifdef PRINT_SEND_PACKET
	printf("SendProcess: %s\n", pkt.convertToHex().c_str());
#endif
#endif

	uint8_t iHeader;

	pkt >> iHeader;

	switch (iHeader)
	{
		case WIZ_HOME:
		{
#ifdef DEBUG
			printf("SendProcess::WIZ_HOME\n");
#endif
			SetTarget(-1);
		}
		break;

		case WIZ_ITEM_MOVE:
		{
			uint8_t iType = pkt.read<uint8_t>();

			switch (iType)
			{
				case 1:
				{
					uint8_t iDirection = pkt.read<uint8_t>();

					uint32_t iItemID = pkt.read<uint32_t>();

					uint8_t iCurrentPosition = pkt.read<uint8_t>();
					uint8_t iTargetPosition = pkt.read<uint8_t>();

					switch (iDirection)
					{
					case ITEM_INVEN_SLOT:
						break;
					case ITEM_SLOT_INVEN:
						break;
					case ITEM_INVEN_INVEN:
						break;
					case ITEM_SLOT_SLOT:
						break;
					case ITEM_INVEN_ZONE:
						break;
					case ITEM_ZONE_INVEN:
						break;
					case ITEM_INVEN_TO_COSP:
						break;
					case ITEM_COSP_TO_INVEN:
						break;
					case ITEM_INVEN_TO_MBAG:
						break;
					case ITEM_MBAG_TO_INVEN:
						break;
					case ITEM_MBAG_TO_MBAG:
						break;
					default:
#ifdef DEBUG
						printf("SendProcess::WIZ_ITEM_MOVE: Direction %d not implemented\n", iDirection);
#endif
						break;
					}

#ifdef DEBUG
					printf("SendProcess::WIZ_ITEM_MOVE: iDirection(%d), iItemID(%d), iCurrentPosition(%d), iTargetPosition(%d)\n",
						iDirection,
						iItemID,
						iCurrentPosition,
						iTargetPosition
					);
#endif
				}
				break;

				default:
				{
#ifdef DEBUG
					printf("SendProcess::WIZ_ITEM_MOVE: Type %d not implemented\n", iType);
#endif
				}
				break;
			}
		}
		break;
	}
}

TNpc ClientHandler::InitializeNpc(Packet& pkt)
{
	TNpc tNpc;
	memset(&tNpc, 0, sizeof(tNpc));

	pkt.SByte();

	tNpc.iID = pkt.read<int32_t>();
	tNpc.iProtoID = pkt.read<uint16_t>();
	tNpc.iMonsterOrNpc = pkt.read<uint8_t>();
	tNpc.iPictureId = pkt.read<uint16_t>();
	tNpc.iUnknown1 = pkt.read<uint32_t>();
	tNpc.iFamilyType = pkt.read<uint8_t>();
	tNpc.iSellingGroup = pkt.read<uint32_t>();
	tNpc.iModelsize = pkt.read<uint16_t>();
	tNpc.iWeapon1 = pkt.read<uint32_t>();
	tNpc.iWeapon2 = pkt.read<uint32_t>();

	if (tNpc.iProtoID == 0)
	{
		int iPetOwnerNameLen = pkt.read<uint8_t>();
		pkt.readString(tNpc.szPetOwnerName, iPetOwnerNameLen);

		int iPetNameLen = pkt.read<uint8_t>();
		pkt.readString(tNpc.szPetName, iPetNameLen);
	}

	tNpc.iModelGroup = pkt.read<uint8_t>();
	tNpc.iLevel = pkt.read<uint8_t>();

	tNpc.fX = (pkt.read<uint16_t>() / 10.0f);
	tNpc.fY = (pkt.read<uint16_t>() / 10.0f);
	tNpc.fZ = (pkt.read<int16_t>() / 10.0f);

	tNpc.iStatus = pkt.read<uint32_t>();

	tNpc.iUnknown2 = pkt.read<uint8_t>();
	tNpc.iUnknown3 = pkt.read<uint32_t>();

	tNpc.fRotation = pkt.read<int16_t>() / 100.0f;

	return tNpc;
}

TPlayer ClientHandler::InitializePlayer(Packet& pkt)
{
	TPlayer tPlayer;
	memset(&tPlayer, 0, sizeof(tPlayer));

	pkt.SByte();

	tPlayer.iID = pkt.read<int32_t>();

	int iNameLen = pkt.read<uint8_t>();
	pkt.readString(tPlayer.szName, iNameLen);

	tPlayer.eNation = (Nation)pkt.read<uint8_t>();

	uint8_t iUnknown1 = pkt.read<uint8_t>();
	uint8_t iUnknown2 = pkt.read<uint8_t>();

	tPlayer.iKnightsID = pkt.read<int16_t>();
	tPlayer.eKnightsDuty = (KnightsDuty)pkt.read<uint8_t>();

	int16_t iAllianceID = pkt.read<int16_t>();
	uint8_t iKnightNameLen = pkt.read<uint8_t>();
	pkt.readString(tPlayer.szKnights, iKnightNameLen);

	tPlayer.iKnightsGrade = pkt.read<uint8_t>();
	tPlayer.iKnightsRank = pkt.read<uint8_t>();

	int16_t sMarkVersion = pkt.read<int16_t>();
	int16_t sCapeID = pkt.read<int16_t>();

	uint8_t iR = pkt.read<uint8_t>();
	uint8_t iG = pkt.read<uint8_t>();
	uint8_t iB = pkt.read<uint8_t>();

	uint8_t iUnknown3 = pkt.read<uint8_t>();
	uint8_t iUnknown4 = pkt.read<uint8_t>();

	tPlayer.iLevel = pkt.read<uint8_t>();

	tPlayer.eRace = (Race)pkt.read<uint8_t>();
	tPlayer.eClass = (Class)pkt.read<int16_t>();

	tPlayer.fX = (pkt.read<uint16_t>()) / 10.0f;
	tPlayer.fY = (pkt.read<uint16_t>()) / 10.0f;
	tPlayer.fZ = (pkt.read<int16_t>()) / 10.0f;

	tPlayer.iFace = pkt.read<uint8_t>();
	tPlayer.iHair = pkt.read<int32_t>();

	uint8_t iResHpType = pkt.read<uint8_t>();

	uint32_t iAbnormalType = pkt.read<uint32_t>();

	switch (iAbnormalType)
	{
		case 4:
			tPlayer.bBlinking = true;
			break;

		case 7:
			tPlayer.bBlinking = false;
			break;
	}

	uint8_t iNeedParty = pkt.read<uint8_t>();

	tPlayer.iAuthority = pkt.read<uint8_t>();

	uint8_t iPartyLeader = pkt.read<uint8_t>();
	uint8_t iInvisibilityType = pkt.read<uint8_t>();
	uint8_t iTeamColor = pkt.read<uint8_t>();
	uint8_t iIsHidingHelmet = pkt.read<uint8_t>();
	uint8_t iIsHidingCospre = pkt.read<uint8_t>();
	uint8_t iIsDevil = pkt.read<uint8_t>();
	uint8_t iIsHidingWings = pkt.read<uint8_t>();

	int16_t iDirection = pkt.read<int16_t>();

	uint8_t iIsChicken = pkt.read<uint8_t>();
	uint8_t iRank = pkt.read<uint8_t>();
	int8_t iKnightsRank = pkt.read<int8_t>();
	int8_t iPersonalRank = pkt.read<int8_t>();

	uint8_t iUnknown5 = pkt.read<uint8_t>(); // 2/8/2023 New

	int32_t iLoop = m_bLunarWarDressUp ? 9 : 15;

	for (int32_t i = 0; i < iLoop; i++)
	{
		tPlayer.tInventory[i].iPos = i;
		tPlayer.tInventory[i].iItemID = pkt.read<uint32_t>();
		tPlayer.tInventory[i].iDurability = pkt.read<uint16_t>();
		tPlayer.tInventory[i].iFlag = pkt.read<uint8_t>();
	}

	tPlayer.iCity = pkt.read<uint8_t>();

	//TODO: Parse remaining bytes
	uint8_t iTempBuffer[16];
	pkt.read(iTempBuffer, 16);

	return tPlayer;
}

void ClientHandler::PushPhase(DWORD dwAddress)
{
#ifdef _WINDLL
	PushPhaseFunction pPushPhaseFunction = (PushPhaseFunction)GetAddress("KO_PTR_PUSH_PHASE");
	pPushPhaseFunction(*reinterpret_cast<int*>(dwAddress));
#else
	BYTE byCode[] =
	{
		0x60,
		0xC6,0x81,0x0C,0x01,0x0,0x0,0x01,
		0xFF,0x35,0x0,0x0,0x0,0x0,
		0xBF,0,0,0,0,
		0xFF,0xD7,
		0x83,0xC4,0x04,
		0xB0,0x01,
		0x61,
		0xC2,0x04,0x0,
	};

	CopyBytes(byCode + 10, dwAddress);

	DWORD dwPushPhase = GetAddress(skCryptDec("KO_PTR_PUSH_PHASE"));
	CopyBytes(byCode + 15, dwPushPhase);

	ExecuteRemoteCode(byCode, sizeof(byCode));
#endif
}

void ClientHandler::SetLoginInformation(std::string szAccountId, std::string szPassword)
{
	m_szAccountId = szAccountId;
	m_szPassword = szPassword;
}

void ClientHandler::ConnectLoginServer(bool bDisconnect)
{
	DWORD dwCGameProcIntroLogin = Read4Byte(GetAddress(skCryptDec("KO_PTR_INTRO")));
	DWORD dwCUILoginIntro = Read4Byte(dwCGameProcIntroLogin + GetAddress(skCryptDec("KO_OFF_UI_LOGIN_INTRO")));

	DWORD dwCN3UIEditIdBase = Read4Byte(dwCUILoginIntro + GetAddress(skCryptDec("KO_OFF_UI_LOGIN_INTRO_ID")));
	DWORD dwCN3UIEditPwBase = Read4Byte(dwCUILoginIntro + GetAddress(skCryptDec("KO_OFF_UI_LOGIN_INTRO_PW")));

	WriteString(dwCN3UIEditIdBase + GetAddress(skCryptDec("KO_OFF_UI_LOGIN_INTRO_ID_INPUT")), m_szAccountId.c_str());
	Write4Byte(dwCN3UIEditIdBase + GetAddress(skCryptDec("KO_OFF_UI_LOGIN_INTRO_ID_INPUT_LENGTH")), m_szPassword.size());
	WriteString(dwCN3UIEditPwBase + GetAddress(skCryptDec("KO_OFF_UI_LOGIN_INTRO_PW_INPUT")), m_szPassword.c_str());
	Write4Byte(dwCN3UIEditPwBase + GetAddress(skCryptDec("KO_OFF_UI_LOGIN_INTRO_PW_INPUT_LENGTH")), m_szPassword.size());

#ifdef _WINDLL
	if (bDisconnect)
	{
		DisconnectFunction pDisconnectFunction = (DisconnectFunction)GetAddress("KO_PTR_LOGIN_DC_REQUEST");
		pDisconnectFunction(*reinterpret_cast<DWORD*>(GetAddress("KO_PTR_INTRO")));
	}

	LoginRequestFunction1 pLoginRequestFunction1 = (LoginRequestFunction1)GetAddress("KO_PTR_LOGIN_REQUEST1");
	pLoginRequestFunction1(*reinterpret_cast<DWORD*>(GetAddress("KO_PTR_INTRO")));

	LoginRequestFunction2 pLoginRequestFunction2 = (LoginRequestFunction2)GetAddress("KO_PTR_LOGIN_REQUEST2");
	pLoginRequestFunction2(*reinterpret_cast<DWORD*>(GetAddress("KO_PTR_INTRO")));
#else
	if (bDisconnect)
	{
		BYTE byCode[] =
		{
			0x60,
			0x8B,0x0D,0x0,0x0,0x0,0x0,
			0xBF,0,0,0,0,
			0xFF,0xD7,
			0x8B,0x0D,0x0,0x0,0x0,0x0,
			0xBF,0,0,0,0,
			0xFF,0xD7,
			0x8B,0x0D,0x0,0x0,0x0,0x0,
			0xBF,0,0,0,0,
			0xFF,0xD7,
			0x61,
			0xC3,
		};

		DWORD dwPtrIntro = GetAddress(skCryptDec("KO_PTR_INTRO"));
		CopyBytes(byCode + 3, dwPtrIntro);
		CopyBytes(byCode + 16, dwPtrIntro);
		CopyBytes(byCode + 29, dwPtrIntro);

		DWORD dwPtrDisconnect = GetAddress(skCryptDec("KO_PTR_LOGIN_DC_REQUEST"));
		CopyBytes(byCode + 8, dwPtrDisconnect);

		DWORD dwPtrLoginRequest1 = GetAddress(skCryptDec("KO_PTR_LOGIN_REQUEST1"));
		CopyBytes(byCode + 21, dwPtrLoginRequest1);

		DWORD dwPtrLoginRequest2 = GetAddress(skCryptDec("KO_PTR_LOGIN_REQUEST2"));
		CopyBytes(byCode + 34, dwPtrLoginRequest2);

		ExecuteRemoteCode(byCode, sizeof(byCode));
	}
	else
	{
		BYTE byCode[] =
		{
			0x60,
			0x8B,0x0D,0x0,0x0,0x0,0x0,
			0xBF,0,0,0,0,
			0xFF,0xD7,
			0x8B,0x0D,0x0,0x0,0x0,0x0,
			0xBF,0,0,0,0,
			0xFF,0xD7,
			0x61,
			0xC3,
		};

		DWORD dwPtrIntro = GetAddress(skCryptDec("KO_PTR_INTRO"));
		CopyBytes(byCode + 3, dwPtrIntro);
		CopyBytes(byCode + 16, dwPtrIntro);

		DWORD dwPtrLoginRequest1 = GetAddress(skCryptDec("KO_PTR_LOGIN_REQUEST1"));
		CopyBytes(byCode + 8, dwPtrLoginRequest1);

		DWORD dwPtrLoginRequest2 = GetAddress(skCryptDec("KO_PTR_LOGIN_REQUEST2"));
		CopyBytes(byCode + 21, dwPtrLoginRequest2);

		ExecuteRemoteCode(byCode, sizeof(byCode));
	}
#endif
}

void ClientHandler::ConnectGameServer(BYTE byServerId)
{
	DWORD dwCGameProcIntroLogin = Read4Byte(GetAddress(skCryptDec("KO_PTR_INTRO")));
	DWORD dwCUILoginIntro = Read4Byte(dwCGameProcIntroLogin + GetAddress(skCryptDec("KO_OFF_UI_LOGIN_INTRO")));

	Write4Byte(dwCUILoginIntro + GetAddress(skCryptDec("KO_OFF_UI_LOGIN_INTRO_SERVER_INDEX")), byServerId);

#ifdef _WINDLL
	LoginServerFunction pLoginServerFunction = (LoginServerFunction)GetAddress("KO_PTR_SERVER_SELECT");
	pLoginServerFunction(*reinterpret_cast<DWORD*>(GetAddress("KO_PTR_INTRO")));
#else
	BYTE byCode[] =
	{
		0x60,
		0x8B, 0x0D, 0x0, 0x0, 0x0, 0x0,
		0xBF, 0x0, 0x0, 0x0, 0x0,
		0xFF, 0xD7,
		0x61,
		0xC3,
	};

	DWORD dwPtrIntro = GetAddress(skCryptDec("KO_PTR_INTRO"));
	CopyBytes(byCode + 3, dwPtrIntro);

	DWORD dwPtrServerSelect = GetAddress(skCryptDec("KO_PTR_SERVER_SELECT"));
	CopyBytes(byCode + 8, dwPtrServerSelect);

	ExecuteRemoteCode(byCode, sizeof(byCode));
#endif
}

void ClientHandler::SelectCharacterSkip()
{
#ifdef _WINDLL
	CharacterSelectSkipFunction pCharacterSelectSkipFunction = (CharacterSelectSkipFunction)GetAddress("KO_PTR_CHARACTER_SELECT_SKIP");
	pCharacterSelectSkipFunction(*reinterpret_cast<DWORD*>(GetAddress("KO_PTR_CHARACTER_SELECT")));
#else

	BYTE byCode[] =
	{
		0x60,
		0x8B, 0x0D, 0x0, 0x0, 0x0, 0x0,
		0xBF, 0x0, 0x0, 0x0, 0x0,
		0xFF, 0xD7,
		0x61,
		0xC3,
	};

	DWORD dwPtrCharacterSelect = GetAddress(skCryptDec("KO_PTR_CHARACTER_SELECT"));
	CopyBytes(byCode + 3, dwPtrCharacterSelect);

	DWORD dwPtrCharacterSelectSkip = GetAddress(skCryptDec("KO_PTR_CHARACTER_SELECT_SKIP"));
	CopyBytes(byCode + 8, dwPtrCharacterSelectSkip);

	ExecuteRemoteCode(byCode, sizeof(byCode));

#endif
}

void ClientHandler::SelectCharacterLeft()
{
#ifdef _WINDLL
	CharacterSelectLeftFunction pCharacterSelectLeftFunction = (CharacterSelectLeftFunction)GetAddress("KO_PTR_CHARACTER_SELECT_LEFT");
	pCharacterSelectLeftFunction(*reinterpret_cast<DWORD*>(GetAddress("KO_PTR_CHARACTER_SELECT")));
#else
	BYTE byCode[] =
	{
		0x60,
		0x8B, 0x0D, 0x0, 0x0, 0x0, 0x0,
		0xBF, 0x0, 0x0, 0x0, 0x0,
		0xFF, 0xD7,
		0x61,
		0xC3,
	};

	DWORD dwPtrCharacterSelect = GetAddress(skCryptDec("KO_PTR_CHARACTER_SELECT"));
	CopyBytes(byCode + 3, dwPtrCharacterSelect);

	DWORD dwPtrCharacterSelectLeft = GetAddress(skCryptDec("KO_PTR_CHARACTER_SELECT_LEFT"));
	CopyBytes(byCode + 8, dwPtrCharacterSelectLeft);

	ExecuteRemoteCode(byCode, sizeof(byCode));
#endif
}

void ClientHandler::SelectCharacterRight()
{
#ifdef _WINDLL
	CharacterSelectRightFunction pCharacterSelectRightFunction = (CharacterSelectRightFunction)GetAddress("KO_PTR_CHARACTER_SELECT_RIGHT");
	pCharacterSelectRightFunction(*reinterpret_cast<DWORD*>(GetAddress("KO_PTR_CHARACTER_SELECT")));
#else
	BYTE byCode[] =
	{
		0x60,
		0x8B, 0x0D, 0x0, 0x0, 0x0, 0x0,
		0xBF, 0x0, 0x0, 0x0, 0x0,
		0xFF, 0xD7,
		0x61,
		0xC3,
	};

	DWORD dwPtrCharacterSelect = GetAddress(skCryptDec("KO_PTR_CHARACTER_SELECT"));
	CopyBytes(byCode + 3, dwPtrCharacterSelect);

	DWORD dwPtrCharacterSelectRight = GetAddress(skCryptDec("KO_PTR_CHARACTER_SELECT_RIGHT"));
	CopyBytes(byCode + 8, dwPtrCharacterSelectRight);

	ExecuteRemoteCode(byCode, sizeof(byCode));
#endif
}

void ClientHandler::SelectCharacter(BYTE byCharacterIndex)
{
#ifdef _WINDLL
	CharacterSelectFunction pCharacterSelectFunction = (CharacterSelectFunction)GetAddress("KO_PTR_CHARACTER_SELECT_ENTER");
	pCharacterSelectFunction(*reinterpret_cast<DWORD*>(GetAddress("KO_PTR_CHARACTER_SELECT")));
#else
	BYTE byCode[] =
	{
		0x60,
		0x8B, 0x0D, 0x0, 0x0, 0x0, 0x0,
		0xBF, 0x0, 0x0, 0x0, 0x0,
		0xFF, 0xD7,
		0x61,
		0xC3,
	};

	DWORD dwPtrCharacterSelect = GetAddress(skCryptDec("KO_PTR_CHARACTER_SELECT"));
	CopyBytes(byCode + 3, dwPtrCharacterSelect);

	DWORD dwPtrCharacterSelectEnter = GetAddress(skCryptDec("KO_PTR_CHARACTER_SELECT_ENTER"));
	CopyBytes(byCode + 8, dwPtrCharacterSelectEnter);

	ExecuteRemoteCode(byCode, sizeof(byCode));
#endif
}

void ClientHandler::SendPacket(Packet vecBuffer)
{
#ifdef _WINDLL
	SendFunction pSendFunction = (SendFunction)GetAddress("KO_SND_FNC");
	pSendFunction(*reinterpret_cast<DWORD*>(GetAddress("KO_PTR_PKT")), vecBuffer.contents(), vecBuffer.size());
#else

	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, m_Bot->GetInjectedProcessId());

	if (hProcess == nullptr)
		return;

	LPVOID pPacketAddress = VirtualAllocEx(hProcess, 0, 1, MEM_COMMIT, PAGE_EXECUTE_READWRITE);

	if (pPacketAddress == 0)
	{
		CloseHandle(hProcess);
		return;
	}

	WriteProcessMemory(hProcess, pPacketAddress, vecBuffer.contents(), vecBuffer.size(), 0);

	BYTE byCode[] =
	{
		0x60,
		0x8B, 0x0D, 0x0, 0x0, 0x0, 0x0,
		0x68, 0x0, 0x0, 0x0, 0x0,
		0x68, 0x0, 0x0, 0x0, 0x0,
		0xBF, 0x0, 0x0, 0x0, 0x0,
		0xFF, 0xD7,
		0x61,
		0xC3,
	};

	DWORD dwPtrPkt = GetAddress(skCryptDec("KO_PTR_PKT"));

	CopyBytes(byCode + 3, dwPtrPkt);

	size_t dwPacketSize = vecBuffer.size();

	CopyBytes(byCode + 8, dwPacketSize);
	CopyBytes(byCode + 13, pPacketAddress);

	DWORD dwPtrSndFnc = GetAddress(skCryptDec("KO_SND_FNC"));
	CopyBytes(byCode + 18, dwPtrSndFnc);

	ExecuteRemoteCode(byCode, sizeof(byCode));
	VirtualFreeEx(hProcess, pPacketAddress, 0, MEM_RELEASE);
	CloseHandle(hProcess);
#endif
}

void ClientHandler::LoadSkillData()
{
#ifdef DEBUG
	printf("Client::LoadSkillData: Start Load Character Skill Data\n");
#endif

	m_vecAvailableSkill.clear();

	auto pSkillList = m_Bot->GetSkillTable()->GetData();

	for (const auto& [key, value] : pSkillList)
	{
		if (0 != std::to_string(value.iNeedSkill).substr(0, 3).compare(std::to_string(GetClass())))
			continue;

		if (value.iTarget != SkillTargetType::TARGET_SELF && value.iTarget != SkillTargetType::TARGET_PARTY_ALL && value.iTarget != SkillTargetType::TARGET_FRIEND_WITHME &&
			value.iTarget != SkillTargetType::TARGET_ENEMY_ONLY && value.iTarget != SkillTargetType::TARGET_AREA_ENEMY)
			continue;

		if ((value.iSelfAnimID1 == 153 || value.iSelfAnimID1 == 154) || (value.iSelfFX1 == 32038 || value.iSelfFX1 == 32039))
			continue;

		switch (value.iNeedSkill % 10)
		{
			case 0:
				if (value.iNeedLevel > GetLevel())
					continue;
				break;
			case 5:
				if (value.iNeedLevel > GetSkillPoint(5))
					continue;
				break;
			case 6:
				if (value.iNeedLevel > GetSkillPoint(6))
					continue;
				break;
			case 7:
				if (value.iNeedLevel > GetSkillPoint(7))
					continue;
				break;
			case 8:
				if (value.iNeedLevel > GetSkillPoint(8))
					continue;
				break;
		}

		m_vecAvailableSkill.push_back(value);
	}
}

void ClientHandler::UseSkill(TABLE_UPC_SKILL pSkillData, int32_t iTargetID)
{
	switch (pSkillData.iTarget)
	{
		case SkillTargetType::TARGET_SELF:
		case SkillTargetType::TARGET_FRIEND_WITHME:
		case SkillTargetType::TARGET_FRIEND_ONLY:
		case SkillTargetType::TARGET_PARTY:
		case SkillTargetType::TARGET_NPC_ONLY:
		case SkillTargetType::TARGET_ENEMY_ONLY:
		case SkillTargetType::TARGET_ALL:
		case 9: // For God Mode
		case SkillTargetType::TARGET_AREA_FRIEND:
		case SkillTargetType::TARGET_AREA_ALL:
		case SkillTargetType::TARGET_DEAD_FRIEND_ONLY:
		{
			if (pSkillData.iReCastTime != 0)
			{
				if (m_PlayerMySelf.iMoveType != 0)
				{
					SendMovePacket(GetPosition(), GetPosition(), 0, 0);
				}

				SendStartSkillCastingAtTargetPacket(pSkillData, iTargetID);
				/*std::this_thread::sleep_for(std::chrono::milliseconds(pSkillData.iCastTime * 100));*/
			}

			auto pSkillExtension2 = m_Bot->GetSkillExtension2Table()->GetData();
			auto pSkillExtension2Data = pSkillExtension2.find(pSkillData.iID);

			uint32_t iArrowCount = 0;

			if (pSkillExtension2Data != pSkillExtension2.end())
				iArrowCount = pSkillExtension2Data->second.iArrowCount;

			Vector3 v3TargetPosition = GetTargetPosition();

			if ((iArrowCount == 0 && pSkillData.iFlyingFX != 0) || iArrowCount == 1)
				SendStartFlyingAtTargetPacket(pSkillData, iTargetID, v3TargetPosition);

			if (iArrowCount > 1)
			{
				SendStartFlyingAtTargetPacket(pSkillData, iTargetID, Vector3(0.0f, 0.0f, 0.0f), 1);

				float fDistance = GetDistance(GetTargetPosition());

				switch (iArrowCount)
				{
				case 3:
				{
					iArrowCount = 1;

					if (fDistance <= 5.0f)
						iArrowCount = 3;
					else if (fDistance < 16.0f)
						iArrowCount = 2;
				};
				break;

				case 5:
				{
					iArrowCount = 1;

					if (fDistance <= 5.0f)
						iArrowCount = 5;
					else if (fDistance <= 6.0f)
						iArrowCount = 4;
					else if (fDistance <= 8.0f)
						iArrowCount = 3;
					else if (fDistance < 16.0f)
						iArrowCount = 2;
				}
				break;
				}

				for (uint32_t i = 0; i < iArrowCount; i++)
				{
					SendStartSkillMagicAtTargetPacket(pSkillData, iTargetID, Vector3(0.0f, 0.0f, 0.0f), (i + 1));
					SendStartMagicAtTarget(pSkillData, iTargetID, v3TargetPosition, (i + 1));
				}
			}
			else
				SendStartSkillMagicAtTargetPacket(pSkillData, iTargetID, v3TargetPosition);
		}
		break;

		case SkillTargetType::TARGET_AREA:
		case SkillTargetType::TARGET_PARTY_ALL:
		case SkillTargetType::TARGET_AREA_ENEMY:
		{
			if (pSkillData.iReCastTime != 0)
			{
				if (m_PlayerMySelf.iMoveType != 0)
				{
					SendMovePacket(GetPosition(), GetPosition(), 0, 0);
				}

				SendStartSkillCastingAtPosPacket(pSkillData, GetTargetPosition());
				std::this_thread::sleep_for(std::chrono::milliseconds(pSkillData.iCastTime * 100));
			}

			if (pSkillData.iFlyingFX != 0)
				SendStartFlyingAtTargetPacket(pSkillData, iTargetID, GetTargetPosition());

			SendStartSkillMagicAtPosPacket(pSkillData, GetTargetPosition());
		}
		break;
	}

	SetSkillUseTime(pSkillData.iID,
		duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
	);
}

void ClientHandler::SendStartSkillCastingAtTargetPacket(TABLE_UPC_SKILL pSkillData, int32_t iTargetID)
{
	Packet pkt = Packet(WIZ_MAGIC_PROCESS);

	pkt
		<< uint8_t(SkillMagicType::SKILL_MAGIC_TYPE_CASTING)
		<< pSkillData.iID
		<< m_PlayerMySelf.iID
		<< iTargetID
		<< uint32_t(0) << uint32_t(0) << uint32_t(0) << uint32_t(0) << uint32_t(0) << uint32_t(0) << uint32_t(0)
		<< int16_t(pSkillData.iReCastTime);

	SendPacket(pkt);
}

void ClientHandler::SendStartSkillCastingAtPosPacket(TABLE_UPC_SKILL pSkillData, Vector3 v3TargetPosition)
{
	Packet pkt = Packet(WIZ_MAGIC_PROCESS);

	pkt
		<< uint8_t(SkillMagicType::SKILL_MAGIC_TYPE_CASTING)
		<< pSkillData.iID
		<< m_PlayerMySelf.iID
		<< int32_t(-1)
		<< uint32_t(v3TargetPosition.m_fX * 10.0f)
		<< int32_t(v3TargetPosition.m_fZ * 10.0f)
		<< uint32_t(v3TargetPosition.m_fY * 10.0f)
		<< uint32_t(0) << uint32_t(0) << uint32_t(0) << uint32_t(0)
		<< int16_t(pSkillData.iReCastTime);

	SendPacket(pkt);
}

void ClientHandler::SendStartFlyingAtTargetPacket(TABLE_UPC_SKILL pSkillData, int32_t iTargetID, Vector3 v3TargetPosition, uint16_t arrowIndex)
{
	Packet pkt = Packet(WIZ_MAGIC_PROCESS);

	pkt
		<< uint8_t(SkillMagicType::SKILL_MAGIC_TYPE_FLYING)
		<< pSkillData.iID
		<< m_PlayerMySelf.iID
		<< iTargetID
		<< uint32_t(v3TargetPosition.m_fX * 10.0f)
		<< int32_t(v3TargetPosition.m_fZ * 10.0f)
		<< uint32_t(v3TargetPosition.m_fY * 10.0f)
		<< arrowIndex
		<< uint32_t(0) << uint32_t(0) << int16_t(0);

	SendPacket(pkt);
}

void ClientHandler::SendStartSkillMagicAtTargetPacket(TABLE_UPC_SKILL pSkillData, int32_t iTargetID, Vector3 v3TargetPosition, uint16_t arrowIndex)
{
	Packet pkt = Packet(WIZ_MAGIC_PROCESS);

	pkt
		<< uint8_t(SkillMagicType::SKILL_MAGIC_TYPE_EFFECTING)
		<< pSkillData.iID
		<< m_PlayerMySelf.iID
		<< iTargetID;

	if (pSkillData.iCastTime == 0)
		pkt << uint32_t(1) << uint32_t(1) << uint32_t(0);
	else
	{
		pkt
			<< uint32_t(v3TargetPosition.m_fX * 10.0f)
			<< int32_t(v3TargetPosition.m_fZ * 10.0f)
			<< uint32_t(v3TargetPosition.m_fY * 10.0f);
	}

	pkt << arrowIndex
		<< uint32_t(0) << uint32_t(0) << uint32_t(0) << uint32_t(0);

	SendPacket(pkt);
}

void ClientHandler::SendStartSkillMagicAtPosPacket(TABLE_UPC_SKILL pSkillData, Vector3 v3TargetPosition)
{
	Packet pkt = Packet(WIZ_MAGIC_PROCESS);

	pkt
		<< uint8_t(SkillMagicType::SKILL_MAGIC_TYPE_EFFECTING)
		<< pSkillData.iID
		<< m_PlayerMySelf.iID
		<< int32_t(-1)
		<< uint32_t(v3TargetPosition.m_fX * 10.0f)
		<< int32_t(v3TargetPosition.m_fZ * 10.0f)
		<< uint32_t(v3TargetPosition.m_fY * 10.0f)
		<< uint32_t(0) << uint32_t(0) << uint32_t(0);

	SendPacket(pkt);
}

void ClientHandler::SendStartMagicAtTarget(TABLE_UPC_SKILL pSkillData, int32_t iTargetID, Vector3 v3TargetPosition, uint16_t arrowIndex)
{
	Packet pkt = Packet(WIZ_MAGIC_PROCESS);

	pkt
		<< uint8_t(SkillMagicType::SKILL_MAGIC_TYPE_FAIL)
		<< pSkillData.iID
		<< m_PlayerMySelf.iID
		<< iTargetID
		<< uint32_t(v3TargetPosition.m_fX * 10.0f)
		<< int32_t(v3TargetPosition.m_fZ * 10.0f)
		<< uint32_t(v3TargetPosition.m_fY * 10.0f)
		<< int32_t(-101)
		<< arrowIndex
		<< uint32_t(0) << uint32_t(0) << uint32_t(0);

	SendPacket(pkt);
}

void ClientHandler::SendCancelSkillPacket(TABLE_UPC_SKILL pSkillData)
{
	Packet pkt = Packet(WIZ_MAGIC_PROCESS);

	pkt
		<< uint8_t(SkillMagicType::SKILL_MAGIC_TYPE_CANCEL)
		<< pSkillData.iID
		<< m_PlayerMySelf.iID
		<< m_PlayerMySelf.iID
		<< uint32_t(0) << uint32_t(0) << uint32_t(0)
		<< uint32_t(0) << uint32_t(0) << uint32_t(0);

	SendPacket(pkt);
}

void ClientHandler::SendMovePacket(Vector3 vecStartPosition, Vector3 vecTargetPosition, int16_t iMoveSpeed, uint8_t iMoveType)
{
	Packet pkt = Packet(WIZ_MOVE);

	pkt
		<< uint16_t(vecStartPosition.m_fX * 10.0f)
		<< uint16_t(vecStartPosition.m_fY * 10.0f)
		<< int16_t(vecStartPosition.m_fZ * 10.0f)
		<< iMoveSpeed
		<< iMoveType
		<< uint16_t(vecTargetPosition.m_fX * 10.0f)
		<< uint16_t(vecTargetPosition.m_fY * 10.0f)
		<< int16_t(vecTargetPosition.m_fZ * 10.0f);

	SendPacket(pkt);
}

void ClientHandler::SendTownPacket()
{
	Packet pkt = Packet(WIZ_HOME);

	SendPacket(pkt);
}

void ClientHandler::SetPosition(Vector3 v3Position)
{
	WriteFloat(Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR"))) + GetAddress(skCryptDec("KO_OFF_X")), v3Position.m_fX);
	WriteFloat(Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR"))) + GetAddress(skCryptDec("KO_OFF_Y")), v3Position.m_fY);
	WriteFloat(Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR"))) + GetAddress(skCryptDec("KO_OFF_Z")), v3Position.m_fZ);
}

void ClientHandler::SetMovePosition(Vector3 v3MovePosition)
{
	//new std::thread([this,v3MovePosition]()
	//	{ 
	//		WorldData* pWorldData = m_World->GetWorldData(GetZone());

	//		if (pWorldData == nullptr)
	//			return;

	//		///TEST
	//		int iStartX = (int)(GetX() / ((int)pWorldData->fMapLength / pWorldData->iMapSize));
	//		int iStartY = (int)(GetY() / ((int)pWorldData->fMapLength / pWorldData->iMapSize));

	//		int iEndX = (int)(v3MovePosition.m_fX / ((int)pWorldData->fMapLength / pWorldData->iMapSize));
	//		int iEndY = (int)(v3MovePosition.m_fY / ((int)pWorldData->fMapLength / pWorldData->iMapSize));

	//		auto vecPath = pWorldData->AStar->findPath({ iStartX, iStartY }, { iEndX, iEndY });

	//		for (auto& coordinate : vecPath)
	//		{
	//			int iNextX = (int)(coordinate.x * ((int)pWorldData->fMapLength / pWorldData->iMapSize));
	//			int iNextY = (int)(coordinate.y * ((int)pWorldData->fMapLength / pWorldData->iMapSize));

	//			WriteByte(Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR"))) + GetAddress(skCryptDec("KO_OFF_MOVE_TYPE")), 2);
	//			WriteFloat(Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR"))) + GetAddress(skCryptDec("KO_OFF_GOX")), iNextX);
	//			WriteFloat(Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR"))) + GetAddress(skCryptDec("KO_OFF_GOY")), iNextY);
	//			WriteByte(Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR"))) + GetAddress(skCryptDec("KO_OFF_MOVE")), 1);

	//			Sleep(1000);

	//			//std::cout
	//			//	<< (int)(coordinate.x * ((int)pWorldData->fMapLength / pWorldData->iMapSize))
	//			//	<< " "
	//			//	<< (int)(coordinate.y * ((int)pWorldData->fMapLength / pWorldData->iMapSize))
	//			//	<< "\n";
	//		}
	//	});


	if (v3MovePosition.m_fX == 0.0f && v3MovePosition.m_fY == 0.0f && v3MovePosition.m_fZ == 0.0f)
	{
		WriteByte(Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR"))) + GetAddress(skCryptDec("KO_OFF_MOVE_TYPE")), 0);
		WriteFloat(Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR"))) + GetAddress(skCryptDec("KO_OFF_GOX")), v3MovePosition.m_fX);
		WriteFloat(Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR"))) + GetAddress(skCryptDec("KO_OFF_GOY")), v3MovePosition.m_fY);
		WriteFloat(Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR"))) + GetAddress(skCryptDec("KO_OFF_GOZ")), v3MovePosition.m_fZ);
		WriteByte(Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR"))) + GetAddress(skCryptDec("KO_OFF_MOVE")), 0);
	}
	else
	{
		WriteByte(Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR"))) + GetAddress(skCryptDec("KO_OFF_MOVE_TYPE")), 2);
		WriteFloat(Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR"))) + GetAddress(skCryptDec("KO_OFF_GOX")), v3MovePosition.m_fX);
		WriteFloat(Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR"))) + GetAddress(skCryptDec("KO_OFF_GOY")), v3MovePosition.m_fY);
		WriteFloat(Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR"))) + GetAddress(skCryptDec("KO_OFF_GOZ")), v3MovePosition.m_fZ);
		WriteByte(Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR"))) + GetAddress(skCryptDec("KO_OFF_MOVE")), 1);
	}
}

void ClientHandler::SendBasicAttackPacket(int32_t iTargetID, float fInterval, float fDistance)
{
	Packet pkt = Packet(WIZ_ATTACK);

	pkt
		<< uint8_t(1) << uint8_t(1)
		<< iTargetID
		<< uint16_t(fInterval * 100.0f)
		<< uint16_t(fDistance * 10.0f)
		<< uint8_t(0) << uint8_t(0);

	SendPacket(pkt); 
}

void ClientHandler::SendShoppingMall(ShoppingMallType eType)
{
	Packet pkt = Packet(WIZ_SHOPPING_MALL);

	switch (eType)
	{
	case ShoppingMallType::STORE_CLOSE:
		pkt << uint8_t(eType);
		break;

	case ShoppingMallType::STORE_PROCESS:
	case ShoppingMallType::STORE_LETTER:
		pkt << uint8_t(eType) << uint8_t(1);
		break;
	}

	SendPacket(pkt);
}

void ClientHandler::SendRotation(float fRotation)
{
	Packet pkt = Packet(WIZ_ROTATE);

	pkt << int16_t(fRotation * 100.f);

	SendPacket(pkt);
}

void ClientHandler::SendRequestBundleOpen(uint32_t iBundleID)
{
	Packet pkt = Packet(WIZ_BUNDLE_OPEN_REQ);

	pkt << uint32_t(iBundleID);

	SendPacket(pkt);
}

void ClientHandler::SendBundleItemGet(uint32_t iBundleID, uint32_t iItemID, int16_t iIndex)
{
	Packet pkt = Packet(WIZ_ITEM_GET);

	pkt << uint32_t(iBundleID) << uint32_t(iItemID) << int16_t(iIndex);

	SendPacket(pkt);
}

void ClientHandler::SendItemMovePacket(uint8_t iType, uint8_t iDirection, uint32_t iItemID, uint8_t iCurrentPosition, uint8_t iTargetPosition)
{
	Packet pkt = Packet(WIZ_ITEM_MOVE);

	pkt << uint8_t(iType) << uint8_t(iDirection) << uint32_t(iItemID) << uint8_t(iCurrentPosition) << uint8_t(iTargetPosition);

	SendPacket(pkt);
}

void ClientHandler::SendTargetHpRequest(int32_t bTargetID, bool bBroadcast)
{
	Packet pkt = Packet(WIZ_TARGET_HP);

	pkt << int32_t(bTargetID) << uint8_t(bBroadcast ? 1 : 0);

	SendPacket(pkt);
}

void ClientHandler::SetTarget(int32_t iTargetID)
{
	if (iTargetID != -1)
		SendTargetHpRequest(iTargetID, true);

	m_iTargetID = iTargetID;
}

void ClientHandler::EquipOreads(int32_t iItemID)
{
#ifdef _WINDLL
	EquipOreadsFunction pEquipOreadsFunction = (EquipOreadsFunction)GetAddress("KO_PTR_EQUIP_ITEM");
	pEquipOreadsFunction(*reinterpret_cast<int*>(GetAddress("KO_PTR_CHR")), iItemID, 0);
#else
#endif
}

void ClientHandler::SetOreads(bool bValue)
{
	Write4Byte(Read4Byte(0xF7F35C) + GetAddress(skCryptDec("KO_OFF_LOOT")), bValue ? 1 : 0);
}

bool ClientHandler::UseItem(uint32_t iItemID)
{
	auto pItemTable = m_Bot->GetItemTable()->GetData();
	auto pSkillTable = m_Bot->GetSkillTable()->GetData();

	auto pItemData = pItemTable.find(iItemID);

	if (pItemData != pItemTable.end())
	{
		auto pSkillData = pSkillTable.find(pItemData->second.dwEffectID1);

		if (pSkillData != pSkillTable.end())
		{
			std::chrono::milliseconds msNow = duration_cast<std::chrono::milliseconds>(
				std::chrono::system_clock::now().time_since_epoch()
				);

			std::chrono::milliseconds msLastSkillUseItem = Client::GetSkillUseTime(pSkillData->second.iID);

			if (pSkillData->second.iCooldown > 0 && msLastSkillUseItem.count() > 0)
			{
				int64_t iSkillCooldownTime = static_cast<int64_t>(pSkillData->second.iCooldown) * 100;

				if ((msLastSkillUseItem.count() + iSkillCooldownTime) > msNow.count())
					return false;
			}

			UseSkill(pSkillData->second, GetID());

			return true;
		}
	}

	return false;
}

void ClientHandler::BasicAttackProcess()
{
#ifdef DEBUG
	printf("ClientHandler::BasicAttackProcess Started\n");
#endif

	auto pItemTable = m_Bot->GetItemTable()->GetData();

	while (m_bWorking)
	{
		try
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(100));

			if (GetTarget() == -1)
				continue;

			if (IsMovingToLoot())
				continue;

			if (IsBlinking())
				continue;

			bool bAttackStatus = GetConfiguration()->GetBool(skCryptDec("Automation"), skCryptDec("Attack"), false);

			if (!bAttackStatus)
				continue;

			bool bBasicAttack = GetConfiguration()->GetBool(skCryptDec("Attack"), skCryptDec("BasicAttack"), true);

			if (!bBasicAttack)
				continue;

			bool bMoveToTarget = GetConfiguration()->GetBool(skCryptDec("Attack"), skCryptDec("MoveToTarget"), false);

			if (bMoveToTarget && GetDistance(GetTargetPosition()) > 1.0f)
				SetMovePosition(GetTargetPosition());

			bool bRangeLimit = GetConfiguration()->GetBool(skCryptDec("Attack"), skCryptDec("RangeLimit"), false);
			int iRangeLimitValue = GetConfiguration()->GetInt(skCryptDec("Attack"), skCryptDec("RangeLimitValue"), (int)MAX_ATTACK_RANGE);

			if (bRangeLimit && GetDistance(GetTargetPosition()) > (float)iRangeLimitValue)
				continue;

			auto iLeftHandWeapon = GetInventoryItemSlot(6);

			if (iLeftHandWeapon != nullptr)
			{
				uint32_t iLeftHandWeaponBaseID = iLeftHandWeapon->iItemID / 1000 * 1000;
				auto pLeftHandWeaponItemData = pItemTable.find(iLeftHandWeaponBaseID);

				if (pLeftHandWeaponItemData != pItemTable.end())
				{
					if (pLeftHandWeaponItemData->second.byKind == ITEM_CLASS_BOW
						|| pLeftHandWeaponItemData->second.byKind == ITEM_CLASS_BOW_LONG
						|| pLeftHandWeaponItemData->second.byKind == ITEM_CLASS_BOW_CROSS)
						continue;
				}
			}

			auto iRightHandWeapon = GetInventoryItemSlot(8);

			if (iRightHandWeapon != nullptr)
			{
				uint32_t iRightHandWeaponBaseID = iRightHandWeapon->iItemID / 1000 * 1000;
				auto pRightHandWeaponItemData = pItemTable.find(iRightHandWeaponBaseID);

				if (pRightHandWeaponItemData != pItemTable.end())
				{
					if (pRightHandWeaponItemData->second.byKind == ITEM_CLASS_BOW
						|| pRightHandWeaponItemData->second.byKind == ITEM_CLASS_BOW_LONG
						|| pRightHandWeaponItemData->second.byKind == ITEM_CLASS_BOW_CROSS)
						continue;
				}
			}

			SendBasicAttackPacket(GetTarget(), 1.10f, 1.0f);
			std::this_thread::sleep_for(std::chrono::milliseconds(1110));
		}
		catch (const std::exception& e)
		{
#ifdef DEBUG
			printf("BasicAttackProcess:Exception: %s\n", e.what());
#else
			UNREFERENCED_PARAMETER(e);
#endif
		}
	}

#ifdef DEBUG
	printf("ClientHandler::BasicAttackProcess Stopped\n");
#endif
}

void ClientHandler::AttackProcess()
{
#ifdef DEBUG
	printf("ClientHandler::AttackProcess Started\n");
#endif

	auto pSkillTable = m_Bot->GetSkillTable()->GetData();
	auto pSkillExtension2 = m_Bot->GetSkillExtension2Table()->GetData();

	while (m_bWorking)
	{
		try
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(100));

			if (GetTarget() == -1)
				continue;

			if (IsMovingToLoot())
				continue;

			if (IsBlinking())
				continue;

			bool bAttackStatus = GetConfiguration()->GetBool(skCryptDec("Automation"), skCryptDec("Attack"), false);

			if (!bAttackStatus)
				continue;

			bool bMoveToTarget = GetConfiguration()->GetInt(skCryptDec("Attack"), skCryptDec("MoveToTarget"), false);

			if (bMoveToTarget && GetDistance(GetTargetPosition()) > 1.0f)
				SetMovePosition(GetTargetPosition());

			bool bRangeLimit = GetConfiguration()->GetBool(skCryptDec("Attack"), skCryptDec("RangeLimit"), false);
			int iRangeLimitValue = GetConfiguration()->GetInt(skCryptDec("Attack"), skCryptDec("RangeLimitValue"), (int)MAX_ATTACK_RANGE);

			if (bRangeLimit && GetDistance(GetTargetPosition()) > (float)iRangeLimitValue)
				continue;

			std::vector<int> vecAttackSkillList = GetConfiguration()->GetInt(skCryptDec("Automation"), skCryptDec("AttackSkillList"), std::vector<int>());

			if (vecAttackSkillList.size() == 0)
				continue;

			auto pSort = [](int& a, int& b)
			{
				return a > b;
			};

			std::sort(vecAttackSkillList.begin(), vecAttackSkillList.end(), pSort);

			for (const auto& x : vecAttackSkillList)
			{
				auto pSkillData = pSkillTable.find(x);

				if (pSkillData != pSkillTable.end())
				{
					uint32_t iNeedItem = pSkillData->second.dwNeedItem;

					uint32_t iNeedItemCount = 1;
					uint32_t iExistItemCount = 0;

					if (iNeedItem != 0)
					{
						iExistItemCount = GetInventoryItemCount(pSkillData->second.dwNeedItem);

						auto pSkillExtension2Data = pSkillExtension2.find(pSkillData->second.iID);

						if (pSkillExtension2Data != pSkillExtension2.end())
							iNeedItemCount = pSkillExtension2Data->second.iArrowCount;
					}

					std::chrono::milliseconds msNow = duration_cast<std::chrono::milliseconds>(
						std::chrono::system_clock::now().time_since_epoch()
					);

					std::chrono::milliseconds msLastSkillUseItem = GetSkillUseTime(pSkillData->second.iID);

					if (pSkillData->second.iCooldown > 0 && msLastSkillUseItem.count() > 0)
					{
						int64_t iSkillCooldownTime = static_cast<int64_t>(pSkillData->second.iCooldown) * 100;

						if ((msLastSkillUseItem.count() + iSkillCooldownTime) > msNow.count())
						{
							continue;
						}
					}

					if (GetMp() < pSkillData->second.iExhaustMSP)
						continue;

					if (iNeedItem != 0 && iExistItemCount < iNeedItemCount)
						continue;

					UseSkill(pSkillData->second, GetTarget());

					int iAttackSpeedValue = GetConfiguration()->GetInt(skCryptDec("Attack"), skCryptDec("AttackSpeedValue"), 1000);

					std::this_thread::sleep_for(std::chrono::milliseconds(iAttackSpeedValue));
				}
			}

			//bool bAttackSpeed = GetConfiguration()->GetBool("Attack", "AttackSpeed", false);

			//if (bAttackSpeed)
			//{
			//	int iAttackSpeedValue = GetConfiguration()->GetInt("Attack", "AttackSpeedValue", 1000);

			//	if (iAttackSpeedValue > 0)
			//		std::this_thread::sleep_for(std::chrono::milliseconds(iAttackSpeedValue));
			//}
			//else
			//	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		}
		catch (const std::exception& e)
		{
#ifdef DEBUG
			printf("AttackProcess:Exception: %s\n", e.what());
#else
			UNREFERENCED_PARAMETER(e);
#endif
		}
	}

#ifdef DEBUG
	printf("ClientHandler::AttackProcess Stopped\n");
#endif
}

void ClientHandler::SearchTargetProcess()
{
#ifdef DEBUG
	printf("ClientHandler::SearchTargetProcess Started\n");
#endif

	while (m_bWorking)
	{
		try
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(100));

			if (IsBlinking())
				continue;

			if (IsMovingToLoot())
				continue;

			bool bAttackStatus = GetConfiguration()->GetBool(skCryptDec("Automation"), skCryptDec("Attack"), false);

			if (!bAttackStatus)
				continue;

			if (m_vecNpc.size() == 0)
				continue;

			bool bAutoTarget = GetConfiguration()->GetInt(skCryptDec("Attack"), skCryptDec("AutoTarget"), true);

			std::vector<int> vecSelectedNpcList = GetConfiguration()->GetInt(skCryptDec("Attack"), skCryptDec("NpcList"), std::vector<int>());

			if (GetTarget() == -1)
			{
				Guard lock(m_vecNpcLock);
				std::vector<TNpc> vecFilteredNpc;

				if (bAutoTarget)
				{
					std::copy_if(m_vecNpc.begin(), m_vecNpc.end(),
						std::back_inserter(vecFilteredNpc),
						[this](const TNpc& c)
						{
							return (c.iMonsterOrNpc == 1
								|| (c.iProtoID >= 19067 && c.iProtoID <= 19069)
								|| (c.iProtoID >= 19070 && c.iProtoID <= 19072))
								&& c.iProtoID != 9009
								&& c.eState != PSA_DYING
								&& c.eState != PSA_DEATH
								&& GetDistance(c.fX, c.fY) <= (float)MAX_ATTACK_RANGE;
						});
				}
				else
				{
					std::copy_if(m_vecNpc.begin(), m_vecNpc.end(),
						std::back_inserter(vecFilteredNpc),
						[this, vecSelectedNpcList](const TNpc& c)
						{
							return (c.iMonsterOrNpc == 1
								|| (c.iProtoID >= 19067 && c.iProtoID <= 19069)
								|| (c.iProtoID >= 19070 && c.iProtoID <= 19072))
								&& c.iProtoID != 9009
								&& c.eState != PSA_DYING
								&& c.eState != PSA_DEATH
								&& std::count(vecSelectedNpcList.begin(), vecSelectedNpcList.end(), c.iProtoID)
								&& GetDistance(c.fX, c.fY) <= (float)MAX_ATTACK_RANGE;
						});
				}

				if (vecFilteredNpc.size() > 0)
				{
					auto pSort = [this](TNpc const& a, TNpc const& b)
					{
						return GetDistance(a.fX, a.fY) < GetDistance(b.fX, b.fY);
					};

					std::sort(vecFilteredNpc.begin(), vecFilteredNpc.end(), pSort);

					auto pFindedTarget = vecFilteredNpc.at(0);

					SetTarget(pFindedTarget.iID);
#ifdef DEBUG
					printf("SearchTargetProcess:: %d, Target Selected\n", pFindedTarget.iID);
#endif
				}
			}
			else
			{
				Guard lock(m_vecNpcLock);
				auto pTarget = std::find_if(m_vecNpc.begin(), m_vecNpc.end(),
					[this](const TNpc& a)
					{
						return a.iID == GetTarget();
					});

				if (&(pTarget))
				{
					if (!bAutoTarget && !std::count(vecSelectedNpcList.begin(), vecSelectedNpcList.end(), pTarget->iProtoID))
					{
						SetTarget(-1);
						continue;
					}

					if (GetDistance(pTarget->fX, pTarget->fY) > (float)MAX_ATTACK_RANGE)
					{
						SetTarget(-1);
						continue;
					}

					if (pTarget->eState == PSA_DYING || pTarget->eState == PSA_DEATH)
					{
						SetTarget(-1);
						continue;
					}
				}
				else
				{
					SetTarget(-1);
					continue;
				}
			}
		}
		catch (const std::exception& e)
		{
#ifdef DEBUG
			printf("SearchTargetProcess:Exception: %s\n", e.what());
#else
			UNREFERENCED_PARAMETER(e);
#endif
		}
	}

#ifdef DEBUG
	printf("ClientHandler::SearchTargetProcess Stopped\n");
#endif
}

void ClientHandler::AutoLootProcess()
{
#ifdef DEBUG
	printf("ClientHandler::AutoLootProcess Started\n");
#endif

	while (m_bWorking)
	{
		try
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(100));

			if (IsBlinking())
				continue;

			if (m_vecLootList.size() == 0)
				continue;

			bool bAutoLoot = GetConfiguration()->GetBool(skCryptDec("AutoLoot"), skCryptDec("Enable"), false);

			if (bAutoLoot)
			{
				std::vector<TLoot> vecFilteredLoot;

				std::chrono::milliseconds msNow = duration_cast<std::chrono::milliseconds>(
					std::chrono::system_clock::now().time_since_epoch()
				);

				Guard lock(m_vecLootListLock);
				std::copy_if(m_vecLootList.begin(), m_vecLootList.end(),
					std::back_inserter(vecFilteredLoot),
					[msNow](const TLoot& c)
					{
						return (c.msDropTime.count() + 1000) < msNow.count() && c.iRequestedOpen == false;
					});

				if (vecFilteredLoot.size() > 0)
				{
					auto pSort = [](TLoot const& a, TLoot const& b)
					{
						return a.msDropTime.count() > b.msDropTime.count();
					};

					std::sort(vecFilteredLoot.begin(), vecFilteredLoot.end(), pSort);

					auto pFindedLoot = vecFilteredLoot.at(0);

					bool bMoveToLoot = GetConfiguration()->GetBool(skCryptDec("AutoLoot"), skCryptDec("MoveToLoot"), false);

					if (bMoveToLoot)
					{
						if (m_vecNpc.size() == 0)
						{
							SetMovingToLoot(false);
							continue;
						}

						Guard lock(m_vecNpcLock);
						auto pNpc = std::find_if(m_vecNpc.begin(), m_vecNpc.end(),
							[pFindedLoot](const TNpc& a)
							{
								return a.iID == pFindedLoot.iNpcID;
							});

						if (pNpc != m_vecNpc.end())
						{
							while (m_bWorking && GetDistance(pNpc->fX, pNpc->fY) > 1.0f)
							{
								std::this_thread::sleep_for(std::chrono::milliseconds(250));

								SetMovePosition(Vector3(pNpc->fX, pNpc->fZ, pNpc->fY));
								SetMovingToLoot(true);
							}

							SendRequestBundleOpen(pFindedLoot.iBundleID);

							auto pLoot = std::find_if(m_vecLootList.begin(), m_vecLootList.end(),
								[pFindedLoot](const TLoot a) { return a.iBundleID == pFindedLoot.iBundleID; });

							if (pLoot != m_vecLootList.end())
							{
								pLoot->iRequestedOpen = true;
							}

							SetMovingToLoot(false);

#ifdef DEBUG
							printf("ClientHandler:: SendRequestBundleOpen: %d\n", pFindedLoot.iBundleID);
#endif
						}
					}
					else
					{
						SendRequestBundleOpen(pFindedLoot.iBundleID);

						auto pLoot = std::find_if(m_vecLootList.begin(), m_vecLootList.end(),
							[pFindedLoot](const TLoot a) { return a.iBundleID == pFindedLoot.iBundleID; });

						if (pLoot != m_vecLootList.end())
						{
							pLoot->iRequestedOpen = true;
						}

#ifdef DEBUG
						printf("ClientHandler:: SendRequestBundleOpen: %d\n", pFindedLoot.iBundleID);
#endif
					}
				}
			}
		}
		catch (const std::exception& e)
		{
#ifdef DEBUG
			printf("AutoLootProcess:Exception: %s\n", e.what());
#else
			UNREFERENCED_PARAMETER(e);
#endif
		}
	}

#ifdef DEBUG
	printf("ClientHandler::AutoLootProcess Stopped\n");
#endif
}

void ClientHandler::CharacterProcess()
{
#ifdef DEBUG
	printf("ClientHandler::CharacterProcess Started\n");
#endif

	auto pSkillTable = m_Bot->GetSkillTable()->GetData();
	auto pSkillExtension2 = m_Bot->GetSkillExtension2Table()->GetData();
	auto pSkillExtension4 = m_Bot->GetSkillExtension4Table()->GetData();

	while (m_bWorking)
	{
		try
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(100));

			if (IsBlinking())
				continue;

			bool bCharacterStatus = GetConfiguration()->GetBool(skCryptDec("Automation"), skCryptDec("Character"), false);

			if (bCharacterStatus)
			{
				std::vector<int> vecCharacterSkillList = GetConfiguration()->GetInt(skCryptDec("Automation"), skCryptDec("CharacterSkillList"), std::vector<int>());

				if (vecCharacterSkillList.size() == 0)
					continue;

				auto pSort = [](int& a, int& b)
				{
					return a > b;
				};

				std::sort(vecCharacterSkillList.begin(), vecCharacterSkillList.end(), pSort);

				for (const auto& x : vecCharacterSkillList)
				{
					auto pSkillData = pSkillTable.find(x);

					if (pSkillData != pSkillTable.end())
					{
						uint32_t iNeedItem = pSkillData->second.dwNeedItem;

						uint32_t iNeedItemCount = 1;
						uint32_t iExistItemCount = 0;

						if (iNeedItem != 0)
						{
							iExistItemCount = GetInventoryItemCount(pSkillData->second.dwNeedItem);

							auto pSkillExtension2Data = pSkillExtension2.find(pSkillData->second.iID);

							if (pSkillExtension2Data != pSkillExtension2.end())
								iNeedItemCount = pSkillExtension2Data->second.iArrowCount;
						}

						if (GetMp() < pSkillData->second.iExhaustMSP)
							continue;

						if (iNeedItem != 0 && iExistItemCount < iNeedItemCount)
							continue;

						std::chrono::milliseconds msNow = duration_cast<std::chrono::milliseconds>(
							std::chrono::system_clock::now().time_since_epoch()
						);

						std::chrono::milliseconds msLastSkillUseItem = GetSkillUseTime(pSkillData->second.iID);

						if (pSkillData->second.iCooldown > 0 && msLastSkillUseItem.count() > 0)
						{
							int64_t iSkillCooldownTime = static_cast<int64_t>(pSkillData->second.iCooldown) * 100;

							if ((msLastSkillUseItem.count() + iSkillCooldownTime) > msNow.count())
								continue;
						}

						auto pSkillExtension4Data = pSkillExtension4.find(pSkillData->second.iID);

						if (pSkillExtension4Data != pSkillExtension4.end())
						{
							if (IsBuffActive(pSkillExtension4Data->second.iBuffType))
								continue;
						}

						UseSkill(pSkillData->second, GetID());
						std::this_thread::sleep_for(std::chrono::milliseconds(1000));
					}
				}
			}
		}
		catch (const std::exception& e)
		{
#ifdef DEBUG
			printf("CharacterProcess:Exception: %s\n", e.what());
#else
			UNREFERENCED_PARAMETER(e);
#endif
		}
	}

#ifdef DEBUG
	printf("ClientHandler::CharacterProcess Stopped\n");
#endif
}

void ClientHandler::ProtectionProcess()
{
#ifdef DEBUG
	printf("ClientHandler::ProtectionProcess Started\n");
#endif

	while (m_bWorking)
	{
		try
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(100));

			if (IsBlinking())
				continue;

			bool bHpProtectionEnable = GetConfiguration()->GetBool(skCryptDec("Protection"), skCryptDec("Hp"), false);

			if (bHpProtectionEnable)
			{
				int16_t iHp = GetHp(); int16_t iMaxHp = GetMaxHp();

				if (iHp == 0 || iMaxHp == 0)
					continue;

				int32_t iHpProtectionPercent = (int32_t)std::ceil((iHp * 100) / iMaxHp);
				int32_t iHpProtectionValue = GetConfiguration()->GetInt(skCryptDec("Protection"), skCryptDec("HpValue"), 50);

				if (iHpProtectionPercent <= iHpProtectionValue)
					HealthPotionProcess();
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(500));

			bool bMpProtectionEnable = GetConfiguration()->GetBool(skCryptDec("Protection"), skCryptDec("Mp"), false);

			if (bMpProtectionEnable)
			{
				int16_t iMp = GetMp(); int16_t iMaxMp = GetMaxMp();

				if (iMp == 0 || iMaxMp == 0)
					continue;

				int32_t iMpProtectionPercent = (int32_t)std::ceil((iMp * 100) / iMaxMp);
				int32_t iMpProtectionValue = GetConfiguration()->GetInt(skCryptDec("Protection"), skCryptDec("MpValue"), 50);

				if (iMpProtectionPercent <= iMpProtectionValue)
					ManaPotionProcess();
			}
		}
		catch (const std::exception& e)
		{
#ifdef DEBUG
			printf("ProtectionProcess:Exception: %s\n", e.what());
#else
			UNREFERENCED_PARAMETER(e);
#endif
		}
	}

#ifdef DEBUG
	printf("ClientHandler::ProtectionProcess Stopped\n");
#endif
}

void ClientHandler::HealthPotionProcess()
{
	std::vector<uint32_t> vecPotions =
	{
		389064000, 910005000, 389063000, 399014000,
		810265000, 810267000, 810269000, 810272000,
		890229000, 899996000, 910004000, 930665000,
		931786000, 389062000, 900790000, 910003000,
		930664000, 389061000, 900780000, 910002000,
		389060000, 900770000, 910001000, 910012000,
		389310000, 389320000, 389330000, 389390000,
		900817000, 389015000, 389014000, 389013000,
		389012000, 389011000, 389010000, 389070000,
		389071000, 800124000, 800126000, 810189000,
		810247000, 811006000, 811008000, 814679000,
		900486000
	};

	auto pSort = [](uint32_t const& a, uint32_t const& b)
	{
		return a < b;
	};

	std::sort(vecPotions.begin(), vecPotions.end(), pSort);

	TInventory* tInventoryItem = NULL;

	for (const auto& x : vecPotions)
	{
		tInventoryItem = GetInventoryItem(x);

		if (tInventoryItem != NULL)
			break;
	}

	if (tInventoryItem != NULL)
		UseItem(tInventoryItem->iItemID);
}

void ClientHandler::ManaPotionProcess()
{
	std::vector<uint32_t> vecPotions =
	{
		389072000, 800125000, 800127000, 810192000,
		810248000, 900487000, 811006000, 811008000,
		814679000, 900486000, 389020000, 389019000,
		389018000, 389017000, 389016000, 389340000,
		389350000, 389360000, 389400000, 900818000,
		910006000, 389078000, 910007000, 900800000,
		389079000, 910008000, 900810000, 389080000,
		910009000, 900820000, 389081000, 910010000,
		899997000, 399020000, 389082000
	};

	auto pSort = [](uint32_t const& a, uint32_t const& b)
	{
		return a < b;
	};

	std::sort(vecPotions.begin(), vecPotions.end(), pSort);

	TInventory* tInventoryItem = NULL;

	for (const auto& x : vecPotions)
	{
		tInventoryItem = GetInventoryItem(x);

		if (tInventoryItem != NULL)
			break;
	}

	if (tInventoryItem != NULL)
		UseItem(tInventoryItem->iItemID);
}

void ClientHandler::GodModeProcess()
{
#ifdef DEBUG
	printf("ClientHandler::GodModeProcess Started\n");
#endif

	auto pSkillTable = m_Bot->GetSkillTable()->GetData();

	while (m_bWorking)
	{
		try
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(100));

			if (IsBlinking())
				continue;

			bool bGodMode = GetConfiguration()->GetBool(skCryptDec("Protection"), skCryptDec("GodMode"), false);

			if (bGodMode)
			{
				auto pSkillData = pSkillTable.find(500344);

				if (pSkillData != pSkillTable.end())
				{
					int16_t iHp = GetHp(); int16_t iMaxHp = GetMaxHp();
					int16_t iMp = GetMp(); int16_t iMaxMp = GetMaxMp();

					if (iHp == 0 || iMaxHp == 0 || iMp == 0 || iMaxMp == 0)
						continue;

					int32_t iHpGodModePercent = (int32_t)std::ceil((iHp * 100) / iMaxHp);
					int32_t iMpGodModePercent = (int32_t)std::ceil((iMp * 100) / iMaxMp);

					int32_t iHpProtectionValue = GetConfiguration()->GetInt(skCryptDec("Protection"), skCryptDec("HpValue"), 50);
					int32_t iMpProtectionValue = GetConfiguration()->GetInt(skCryptDec("Protection"), skCryptDec("MpValue"), 25);

					std::chrono::milliseconds msNow = duration_cast<std::chrono::milliseconds>(
						std::chrono::system_clock::now().time_since_epoch()
					);

					std::chrono::milliseconds msLastSkillUseItem = GetSkillUseTime(pSkillData->second.iID);

					if (pSkillData->second.iCooldown > 0 && msLastSkillUseItem.count() > 0)
					{
						int64_t iSkillCooldownTime = static_cast<int64_t>(pSkillData->second.iCooldown) * 100;

						if ((msLastSkillUseItem.count() + iSkillCooldownTime) > msNow.count())
							continue;
					}

					if (iHpGodModePercent <= iHpProtectionValue
						|| iMpGodModePercent <= iMpProtectionValue
						|| !IsBuffActive(1))
					{
						SendCancelSkillPacket(pSkillData->second);
						UseSkill(pSkillData->second, GetID());
					}
				}
			}
		}
		catch (const std::exception& e)
		{
#ifdef DEBUG
			printf("GodModeProcess:Exception: %s\n", e.what());
#else
			UNREFERENCED_PARAMETER(e);
#endif
		}	
	}

#ifdef DEBUG
	printf("ClientHandler::GodModeProcess Stopped\n");
#endif
}

void ClientHandler::MinorProcess()
{
#ifdef DEBUG
	printf("ClientHandler::MinorProcess Started\n");
#endif

	while (m_bWorking)
	{
		try
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(100));

			if (IsBlinking())
				continue;

			bool bMinorProtection = GetConfiguration()->GetBool(skCryptDec("Protection"), skCryptDec("Minor"), false);

			if (bMinorProtection)
			{
				int16_t iHp = GetHp(); int16_t iMaxHp = GetMaxHp();

				if (iHp == 0 || iMaxHp == 0)
					continue;

				int32_t iHpProtectionPercent = (int32_t)std::ceil((iHp * 100) / iMaxHp);
				int32_t iHpProtectionValue = GetConfiguration()->GetInt(skCryptDec("Protection"), skCryptDec("MinorValue"), 30);

				if (iHpProtectionPercent <= iHpProtectionValue)
				{
					auto vecAvailableSkill = GetAvailableSkill();

					auto it = std::find_if(vecAvailableSkill.begin(), vecAvailableSkill.end(),
						[](const TABLE_UPC_SKILL& a) { return a.szEngName == skCryptDec("minor healing"); });

					if (it != vecAvailableSkill.end())
					{
						UseSkill(*it, GetID());
					}
				}
			}
		}
		catch (const std::exception& e)
		{
#ifdef DEBUG
			printf("MinorProcess:Exception: %s\n", e.what());
#else
			UNREFERENCED_PARAMETER(e);
#endif
		}
	}

#ifdef DEBUG
	printf("ClientHandler::MinorProcess Stopped\n");
#endif
}

void ClientHandler::RouteProcess()
{
#ifdef DEBUG
	printf("ClientHandler::RouteProcess Started\n");
#endif

	while (m_bWorking)
	{
		try
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(100));

			if (IsBlinking())
				continue;

			if (m_vecRoute.size() == 0)
				continue;

			Guard guard(m_vecRouteLock);

			Route pRoute = m_vecRoute.front();

			float fDistance = GetDistance(Vector3(GetX(false), 0.0f, GetY(false)), Vector3(pRoute.fX, 0.0f, pRoute.fY));

			if (fDistance > 1.0f)
				SetMovePosition(Vector3(pRoute.fX, 0.0f, pRoute.fY));
			else
			{
				switch (pRoute.eStepType)
				{
					case RouteStepType::STEP_MOVE:
					{
						m_vecRoute.erase(m_vecRoute.begin());
					}
					break;
					case RouteStepType::STEP_TOWN:
					{
						SendTownPacket();
						m_vecRoute.erase(m_vecRoute.begin());
					}
					break;
					case RouteStepType::STEP_SUPPLY:
					{
						m_vecRoute.erase(m_vecRoute.begin());
					}
					break;
					case RouteStepType::STEP_INN:
					{
						m_vecRoute.erase(m_vecRoute.begin());
					}
					break;
				}
			}
		}
		catch (const std::exception& e)
		{
#ifdef DEBUG
			printf("RouteProcess:Exception: %s\n", e.what());
#else
			UNREFERENCED_PARAMETER(e);
#endif
		}
	}

#ifdef DEBUG
	printf("ClientHandler::RouteProcess Stopped\n");
#endif
}

void ClientHandler::PatchDeathEffect(bool bValue)
{
	if (bValue)
	{
		if (m_vecOrigDeathEffectFunction.size() == 0)
			m_vecOrigDeathEffectFunction = ReadBytes(GetAddress(skCryptDec("KO_DEATH_EFFECT")), 2);

		std::vector<uint8_t> vecPatch = { 0x90, 0x90 };
		WriteBytes(GetAddress(skCryptDec("KO_DEATH_EFFECT")), vecPatch);
	}
	else
	{
		if (m_vecOrigDeathEffectFunction.size() > 0)
			WriteBytes(GetAddress(skCryptDec("KO_DEATH_EFFECT")), m_vecOrigDeathEffectFunction);
	}
}

void ClientHandler::SetRoute(std::vector<Route> vecRoute)
{
	Guard guard(m_vecRouteLock);
	m_vecRoute = vecRoute;
}

void ClientHandler::ClearRoute()
{
	Guard guard(m_vecRouteLock);
	m_vecRoute.clear();
}
