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

	m_msLastSupplyTime = std::chrono::milliseconds(0);
	m_msLastPotionUseTime = std::chrono::milliseconds(0);

	m_mapSkillBase.clear();

	m_pClientSkillAddress = nullptr;
	m_pClientSkillFunctionAddress = nullptr;

	m_pClientBasicAttackAddress = nullptr;
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

	new std::thread([this]() { AttackProcess(); });
	new std::thread([this]() { SearchTargetProcess(); });
	new std::thread([this]() { PotionProcess(); });
	new std::thread([this]() { CharacterProcess(); });

	if (m_Bot->GetPlatformType() == PlatformType::CNKO)
	{
		new std::thread([this]() { GodModeProcess(); });
	}

	if (IsRogue())
	{
		new std::thread([this]() { MinorProcess(); });
	}
	
	new std::thread([this]() { AutoLootProcess(); });
	new std::thread([this]() { RouteProcess(); });
	new std::thread([this]() { SupplyProcess(); });
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
	new std::thread([this]() { m_Bot->InitializeSupplyData(); });
	new std::thread([this]() { m_Bot->InitializePriestData(); });

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

	DWORD dwAddressReady = 0;
	while (dwAddressReady == 0)
	{
		ReadProcessMemory(m_Bot->GetInjectedProcessHandle(), (LPVOID)dwAddress, &dwAddressReady, 4, 0);
	}

	HMODULE hModuleKernel32 = GetModuleHandle(skCryptDec("kernel32.dll"));

	if (hModuleKernel32 == nullptr)
	{
#ifdef DEBUG
		printf("hModuleKernel32 == nullptr\n");
#endif
		return;
	}

	LPVOID pCreateFilePtr = GetProcAddress(hModuleKernel32, skCryptDec("CreateFileA"));
	LPVOID pWriteFilePtr = GetProcAddress(hModuleKernel32, skCryptDec("WriteFile"));
	LPVOID pCloseHandlePtr = GetProcAddress(hModuleKernel32, skCryptDec("CloseHandle"));

	LPVOID pMailSlotNameAddress = VirtualAllocEx(m_Bot->GetInjectedProcessHandle(), 0, 1, MEM_COMMIT, PAGE_EXECUTE_READWRITE);

	if (pMailSlotNameAddress == 0)
	{
		return;
	}

	m_szMailSlotRecvName = skCryptDec("\\\\.\\mailslot\\KOF_RECV\\") + std::to_string(m_Bot->GetInjectedProcessId());

	std::vector<BYTE> vecMailSlotName(m_szMailSlotRecvName.begin(), m_szMailSlotRecvName.end());
	WriteBytes((DWORD)pMailSlotNameAddress, vecMailSlotName);

	if (m_RecvHookAddress == 0)
	{
		m_RecvHookAddress = VirtualAllocEx(m_Bot->GetInjectedProcessHandle(), 0, 1, MEM_COMMIT, PAGE_EXECUTE_READWRITE);

		if (m_RecvHookAddress == 0)
		{
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

	DWORD dwPatchAddress = (DWORD)VirtualAllocEx(m_Bot->GetInjectedProcessHandle(), 0, 1, MEM_COMMIT, PAGE_EXECUTE_READWRITE);

	if (dwPatchAddress == 0)
	{
		return;
	}

	WriteBytes(dwPatchAddress, vecPatch);

	DWORD dwOldProtection;
	VirtualProtectEx(m_Bot->GetInjectedProcessHandle(), (LPVOID)dwRecvAddress, 1, PAGE_EXECUTE_READWRITE, &dwOldProtection);
	Write4Byte(Read4Byte(Read4Byte(dwAddress)) + 0x8, dwPatchAddress);
	VirtualProtectEx(m_Bot->GetInjectedProcessHandle(), (LPVOID)dwRecvAddress, 1, dwOldProtection, &dwOldProtection);
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
	DWORD dwAddressReady = 0;
	while (dwAddressReady == 0)
	{
		ReadProcessMemory(m_Bot->GetInjectedProcessHandle(), (LPVOID)GetAddress(skCryptDec("KO_SND_FNC")), &dwAddressReady, 4, 0);
	}

	HMODULE hModuleKernel32 = GetModuleHandle(skCryptDec("kernel32.dll"));

	if (hModuleKernel32 == nullptr)
	{
#ifdef DEBUG
		printf("hModuleKernel32 == nullptr\n");
#endif
		return;
	}

	LPVOID pCreateFilePtr = GetProcAddress(hModuleKernel32, skCryptDec("CreateFileA"));
	LPVOID pWriteFilePtr = GetProcAddress(hModuleKernel32, skCryptDec("WriteFile"));
	LPVOID pCloseHandlePtr = GetProcAddress(hModuleKernel32, skCryptDec("CloseHandle"));

	LPVOID pMailSlotNameAddress = VirtualAllocEx(m_Bot->GetInjectedProcessHandle(), 0, 1, MEM_COMMIT, PAGE_EXECUTE_READWRITE);

	if (pMailSlotNameAddress == 0)
	{
		return;
	}

	m_szMailSlotSendName = skCryptDec("\\\\.\\mailslot\\KOF_SEND\\") + std::to_string(m_Bot->GetInjectedProcessId());

	std::vector<BYTE> vecMailSlotName(m_szMailSlotSendName.begin(), m_szMailSlotSendName.end());
	WriteBytes((DWORD)pMailSlotNameAddress, vecMailSlotName);

	if (m_SendHookAddress == 0)
	{
		m_SendHookAddress = VirtualAllocEx(m_Bot->GetInjectedProcessHandle(), 0, 1, MEM_COMMIT, PAGE_EXECUTE_READWRITE);

		if (m_SendHookAddress == 0)
		{
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

	LPVOID dwPatchAddress = VirtualAllocEx(m_Bot->GetInjectedProcessHandle(), 0, vecPatch1.size(), MEM_COMMIT, PAGE_EXECUTE_READWRITE);

	if (dwPatchAddress == 0)
	{
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
	VirtualProtectEx(m_Bot->GetInjectedProcessHandle(), (LPVOID)GetAddress(skCryptDec("KO_SND_FNC")), 1, PAGE_EXECUTE_READWRITE, &dwOldProtection);
	WriteBytes(GetAddress(skCryptDec("KO_SND_FNC")), vecPatch2);
	VirtualProtectEx(m_Bot->GetInjectedProcessHandle(), (LPVOID)GetAddress(skCryptDec("KO_SND_FNC")), 1, dwOldProtection, &dwOldProtection);
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

			new std::thread([this]() { m_Bot->GetWorld()->Load(GetZone()); });

			m_Bot->SendLoadUserConfiguration(1, m_PlayerMySelf.szName);

#ifdef DEBUG
			printf("RecvProcess::WIZ_MYINFO: %s loaded\n", m_PlayerMySelf.szName.c_str());
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
							GetConfiguration()->SetInt(skCryptDec("Automation"), skCryptDec("AttackSkillList"), std::vector<int>());
							GetConfiguration()->SetInt(skCryptDec("Automation"), skCryptDec("CharacterSkillList"), std::vector<int>());

							LoadSkillData();
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
							GetConfiguration()->SetInt(skCryptDec("Automation"), skCryptDec("AttackSkillList"), std::vector<int>());
							GetConfiguration()->SetInt(skCryptDec("Automation"), skCryptDec("CharacterSkillList"), std::vector<int>());

							LoadSkillData();
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
				}
				break;
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

		case WIZ_NPC_INOUT:
		{
			uint8_t iType = pkt.read<uint8_t>();

			switch (iType)
			{
				case InOut::INOUT_IN:
				{
					auto pNpc = InitializeNpc(pkt);

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

		}
		break;

		case WIZ_MAGIC_PROCESS:
		{
			uint8_t iType = pkt.read<uint8_t>();

			switch (iType)
			{
				case SkillMagicType::SKILL_MAGIC_TYPE_FLYING:
				{
					uint32_t iSkillID = pkt.read<uint32_t>();
					int32_t iSourceID = pkt.read<int32_t>();
					int32_t iTargetID = pkt.read<int32_t>();
				}
				break;

				case SkillMagicType::SKILL_MAGIC_TYPE_EFFECTING:
				{
					uint32_t iSkillID = pkt.read<uint32_t>();
					int32_t iSourceID = pkt.read<int32_t>();
					int32_t iTargetID = pkt.read<int32_t>();
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

		case WIZ_EXCHANGE:
		{
			uint8_t iType = pkt.read<uint8_t>();

			switch (iType)
			{
				case TradeSubPacket::TRADE_DONE:
				{
					uint8_t iResult = pkt.read<uint8_t>();

					if (iResult == 1)
					{
						uint32_t iGold = pkt.read<uint32_t>();
						int16_t iItemCount = pkt.read<int16_t>();

#ifdef DEBUG
						printf("RecvProcess::WIZ_EXCHANGE: TRADE_DONE - Success - %d,%d \n", iGold, iItemCount);
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

		case WIZ_ITEM_DROP:
		{
			TLoot tLoot;
			memset(&tLoot, 0, sizeof(tLoot));

			tLoot.iNpcID = pkt.read<int32_t>();
			tLoot.iBundleID = pkt.read<uint32_t>();
			tLoot.iItemCount = pkt.read<uint8_t>();

			tLoot.msDropTime = duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());

			tLoot.iRequestedOpen = false;

			Guard lock(m_mutexLootList);
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

			Guard lock(m_mutexLootList);
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

							if (iItemID == 900000000)
								SendBundleItemGet(iBundleID, iItemID, (int16_t)i);
							else
							{
								std::map<uint32_t, __TABLE_ITEM>* pItemTable;
								if (m_Bot->GetItemTable(&pItemTable))
								{
									auto pItemData = pItemTable->find(iItemID);

									if (pItemData != pItemTable->end())
									{
										int iLootMinPrice = GetConfiguration()->GetInt(skCryptDec("AutoLoot"), skCryptDec("MinPrice"), 0);

										if (iLootMinPrice == 0 || (iLootMinPrice > 0 && pItemData->second.iPriceRepair >= iLootMinPrice))
										{
											SendBundleItemGet(iBundleID, iItemID, (int16_t)i);
										}
									}
								}
								else
								{
									SendBundleItemGet(iBundleID, iItemID, (int16_t)i);
								}
							}						
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

					Guard lock(m_mutexLootList);
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

					Guard lock(m_mutexLootList);
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

					Guard lock(m_mutexLootList);
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

					uint8_t iCity = (uint8_t)pkt.read<int16_t>();

					if (m_PlayerMySelf.iCity != iCity)
					{
						m_PlayerMySelf.iCity = iCity;

						m_vecNpc.clear();
						m_mapSkillUseTime.clear();

						m_bIsMovingToLoot = false;
						m_vecLootList.clear();

						m_vecRoute.clear();

						m_msLastSupplyTime = std::chrono::milliseconds(0);
						m_msLastPotionUseTime = std::chrono::milliseconds(0);
					}

					m_PlayerMySelf.fZ = (pkt.read<int16_t>() / 10.0f);
					m_PlayerMySelf.fX = (pkt.read<uint16_t>() / 10.0f);
					m_PlayerMySelf.fY = (pkt.read<uint16_t>() / 10.0f);

					uint8_t iVictoryNation = pkt.read<uint8_t>();

					new std::thread([this]() { m_Bot->GetWorld()->Load(m_PlayerMySelf.iCity); });

#ifdef DEBUG
					printf("RecvProcess::WIZ_ZONE_CHANGE: Teleport Zone: [%d] Coordinate: [%f - %f - %f] VictoryNation: [%d]\n",
						m_PlayerMySelf.iCity, m_PlayerMySelf.fX, m_PlayerMySelf.fY, m_PlayerMySelf.fZ, iVictoryNation);
#endif

				}
				break;

				case ZoneChangeOpcode::ZoneChangeMilitaryCamp:
				{
					m_vecNpc.clear();
					m_mapSkillUseTime.clear();

					m_bIsMovingToLoot = false;
					m_vecLootList.clear();

					m_vecRoute.clear();

					m_msLastSupplyTime = std::chrono::milliseconds(0);
					m_msLastPotionUseTime = std::chrono::milliseconds(0);
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
			m_vecLootList.clear();
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

		case WIZ_MAGIC_PROCESS:
		{
			uint8_t iType = pkt.read<uint8_t>();

			switch (iType)
			{
				case SkillMagicType::SKILL_MAGIC_TYPE_FLYING:
				{
					uint32_t iSkillID = pkt.read<uint32_t>();
					int32_t iSourceID = pkt.read<int32_t>();
					int32_t iTargetID = pkt.read<int32_t>();

					std::map<uint32_t, __TABLE_UPC_SKILL>* pSkillTable;
					if (!m_Bot->GetSkillTable(&pSkillTable))
						return;

					auto pSkillData = pSkillTable->find(iSkillID);

					if (pSkillData != pSkillTable->end())
					{
						if (iSourceID == GetID())
						{
							if (IsRogue())
							{
								bool bArcherCombo = GetConfiguration()->GetBool(skCryptDec("Attack"), skCryptDec("ArcherCombo"), true);
								DWORD iMobBase = GetMobBase(iTargetID);

								if (bArcherCombo && iMobBase != 0)
								{
									DWORD iHp = GetHp(iMobBase);
									DWORD iMaxHp = GetMaxHp(iMobBase);
									DWORD iState = GetActionState(iMobBase);

									if (iState != PSA_DYING && iState != PSA_DEATH && iMaxHp != 0 && iHp != 0)
									{
										StepCharacterForward(true);
										StepCharacterForward(false);
									}
								}
							}	
						}
					}
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

void ClientHandler::PushPhase(DWORD dwAddress)
{
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
		0xC2,0x04,0x0
	};

	CopyBytes(byCode + 10, dwAddress);

	DWORD dwPushPhase = GetAddress(skCryptDec("KO_PTR_PUSH_PHASE"));
	CopyBytes(byCode + 15, dwPushPhase);

	ExecuteRemoteCode(byCode, sizeof(byCode));
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
	Write4Byte(dwCN3UIEditIdBase + GetAddress(skCryptDec("KO_OFF_UI_LOGIN_INTRO_ID_INPUT_LENGTH")), m_szAccountId.size());
	WriteString(dwCN3UIEditPwBase + GetAddress(skCryptDec("KO_OFF_UI_LOGIN_INTRO_PW_INPUT")), m_szPassword.c_str());
	Write4Byte(dwCN3UIEditPwBase + GetAddress(skCryptDec("KO_OFF_UI_LOGIN_INTRO_PW_INPUT_LENGTH")), m_szPassword.size());

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
}

void ClientHandler::ConnectGameServer(BYTE byServerId)
{
	DWORD dwCGameProcIntroLogin = Read4Byte(GetAddress(skCryptDec("KO_PTR_INTRO")));
	DWORD dwCUILoginIntro = Read4Byte(dwCGameProcIntroLogin + GetAddress(skCryptDec("KO_OFF_UI_LOGIN_INTRO")));

	Write4Byte(dwCUILoginIntro + GetAddress(skCryptDec("KO_OFF_UI_LOGIN_INTRO_SERVER_INDEX")), byServerId);

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
}

void ClientHandler::SelectCharacterSkip()
{
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
}

void ClientHandler::SelectCharacterLeft()
{
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
}

void ClientHandler::SelectCharacterRight()
{
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
}

void ClientHandler::SelectCharacter(BYTE byCharacterIndex)
{
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
}

void ClientHandler::SendPacket(Packet vecBuffer)
{
	LPVOID pPacketAddress = VirtualAllocEx(m_Bot->GetInjectedProcessHandle(), 0, 1, MEM_COMMIT, PAGE_EXECUTE_READWRITE);

	if (pPacketAddress == nullptr)
		return;

	WriteProcessMemory(m_Bot->GetInjectedProcessHandle(), pPacketAddress, vecBuffer.contents(), vecBuffer.size(), 0);

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
	VirtualFreeEx(m_Bot->GetInjectedProcessHandle(), pPacketAddress, 0, MEM_RELEASE);
}

void ClientHandler::LoadSkillData()
{
#ifdef DEBUG
	printf("Client::LoadSkillData: Start Load Character Skill Data\n");
#endif

	m_vecAvailableSkill.clear();

	std::map<uint32_t, __TABLE_UPC_SKILL>* pSkillTable;
	if (!m_Bot->GetSkillTable(&pSkillTable))
		return;

	for (const auto& [key, value] : *pSkillTable)
	{
		if (0 != std::to_string(value.iNeedSkill).substr(0, 3).compare(std::to_string(GetClass())))
			continue;

		if (value.iTarget != SkillTargetType::TARGET_SELF 
			&& value.iTarget != SkillTargetType::TARGET_PARTY 
			&& value.iTarget != SkillTargetType::TARGET_PARTY_ALL 
			&& value.iTarget != SkillTargetType::TARGET_FRIEND_WITHME 
			&& value.iTarget != SkillTargetType::TARGET_FRIEND_ONLY
			&& value.iTarget != SkillTargetType::TARGET_ENEMY_ONLY 
			&& value.iTarget != SkillTargetType::TARGET_AREA_ENEMY)
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
				if (value.iNeedLevel > GetSkillPoint(0))
					continue;
				break;
			case 6:
				if (value.iNeedLevel > GetSkillPoint(1))
					continue;
				break;
			case 7:
				if (value.iNeedLevel > GetSkillPoint(2))
					continue;
				break;
			case 8:
				if (value.iNeedLevel > GetSkillPoint(3))
					continue;
				break;
		}

		m_vecAvailableSkill.push_back(value);
	}
}

void ClientHandler::UseSkill(TABLE_UPC_SKILL pSkillData, int32_t iTargetID, bool iAttacking)
{
	bool bLegalStatus = GetConfiguration()->GetBool(skCryptDec("Bot"), skCryptDec("Legal"), false);

	if (bLegalStatus)
	{
		//Fix for bug report -_-
		BYTE byFix1Patch[] =
		{
			0xE9, 0xC5, 0x00, 0x00, 0x00,
			0x90
		};

		WriteProcessMemory(m_Bot->m_hInjectedProcessHandle, (LPVOID)GetAddress(skCryptDec("KO_FIX1")), byFix1Patch, sizeof(byFix1Patch), NULL);

		if (pSkillData.iReCastTime != 0)
		{
			if (GetActionState() == PSA_SPELLMAGIC)
				return;
		}

		m_mutexUseSkill.lock();

		BYTE byCode[] =
		{
			0x60,
			0x8B, 0x0D, 0x00, 0x00, 0x00, 0x00,
			0x68, 0x00, 0x00, 0x00, 0x00,
			0xBF, 0x00, 0x00, 0x00, 0x00,
			0xFF, 0xD7,
			0x50,
			0x8B, 0x0D, 0x00, 0x00, 0x00, 0x00,
			0x8B, 0x89, 0x00, 0x00, 0x00, 0x00,
			0x68, 0x00, 0x00, 0x00, 0x00,
			0xBF, 0x00, 0x00, 0x00, 0x00,
			0xFF, 0xD7,
			0x61,
			0xC3,
		};

		DWORD iSBEC = GetAddress(skCryptDec("KO_SBEC"));
		CopyBytes(byCode + 3, iSBEC);
		CopyBytes(byCode + 8, pSkillData.iID);

		DWORD iSBCA = GetAddress(skCryptDec("KO_SBCA"));
		CopyBytes(byCode + 13, iSBCA);

		DWORD iDLG = GetAddress(skCryptDec("KO_PTR_DLG"));
		CopyBytes(byCode + 22, iDLG);

		DWORD iLPEO = GetAddress(skCryptDec("KO_OFF_LPEO"));
		CopyBytes(byCode + 28, iLPEO);
		CopyBytes(byCode + 33, iTargetID);

		DWORD iLSCA = GetAddress(skCryptDec("KO_LSCA"));
		CopyBytes(byCode + 38, iLSCA);

		ExecuteRemoteCode(byCode, sizeof(byCode));

		m_mutexUseSkill.unlock();

		SetSkillUseTime(pSkillData.iID,
			duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
		);
	}
	else
	{
		Vector3 v3MyPosition = GetPosition();
		Vector3 v3TargetPosition = GetTargetPosition();

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
					if (GetMoveState() != PSM_STOP)
					{
						SendMovePacket(v3MyPosition, v3MyPosition, 0, 0);
					}

					SendStartSkillCastingAtTargetPacket(pSkillData, iTargetID);
					std::this_thread::sleep_for(std::chrono::milliseconds(pSkillData.iReCastTime * 100));
				}

				uint32_t iArrowCount = 0;

				std::map<uint32_t, __TABLE_UPC_SKILL_EXTENSION2>* pSkillExtension2;
				if (m_Bot->GetSkillExtension2Table(&pSkillExtension2))
				{
					auto pSkillExtension2Data = pSkillExtension2->find(pSkillData.iID);

					if (pSkillExtension2Data != pSkillExtension2->end())
						iArrowCount = pSkillExtension2Data->second.iArrowCount;
				}

				if ((iArrowCount == 0 && pSkillData.iFlyingFX != 0) || iArrowCount == 1)
					SendStartFlyingAtTargetPacket(pSkillData, iTargetID, v3TargetPosition);

				if (iArrowCount > 1)
				{
					SendStartFlyingAtTargetPacket(pSkillData, iTargetID, Vector3(0.0f, 0.0f, 0.0f), 1);

					float fDistance = GetDistance(v3TargetPosition);

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
					if (GetMoveState() != PSM_STOP)
					{
						SendMovePacket(v3MyPosition, v3MyPosition, 0, 0);
					}

					SendStartSkillCastingAtPosPacket(pSkillData, v3TargetPosition);
					std::this_thread::sleep_for(std::chrono::milliseconds(pSkillData.iReCastTime * 100));
				}

				if (pSkillData.iFlyingFX != 0)
					SendStartFlyingAtTargetPacket(pSkillData, iTargetID, v3TargetPosition);

				SendStartSkillMagicAtPosPacket(pSkillData, v3TargetPosition);
			}
			break;
		}

		SetSkillUseTime(pSkillData.iID,
			duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
		);
	}
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

	if (pSkillData.iTarget == SkillTargetType::TARGET_PARTY)
	{
		pkt << uint32_t(0);
	}

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

	if (pSkillData.iTarget == SkillTargetType::TARGET_PARTY)
	{
		pkt << arrowIndex
			<< uint32_t(0) << uint32_t(0) << int16_t(0);
	}
	else
	{
		pkt << arrowIndex
			<< uint32_t(0) << uint32_t(0) << int16_t(0) << int16_t(0) << int16_t(0);
	}

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
		<< uint32_t(0) << uint32_t(0) << int16_t(0) << int16_t(0) << int16_t(0);

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
		<< uint16_t(fInterval)
		<< uint16_t(fDistance)
		<< uint8_t(0) << uint8_t(1);

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

void ClientHandler::SetTarget(int32_t iTargetID, bool bSelectFromClient)
{
	DWORD iMobBase = GetMobBase(iTargetID);

	BYTE byCode[] =
	{
		0x60,
		0xB9, 0x00, 0x00, 0x00, 0x00,
		0x8B, 0x09,
		0xB8, 0x00, 0x00, 0x00, 0x00,
		0x50,
		0xBE, 0x00, 0x00, 0x00, 0x00,
		0xFF, 0xD6,
		0x61,
		0xC3,
	};

	DWORD iDlg = GetAddress(skCryptDec("KO_PTR_DLG"));
	CopyBytes(byCode + 2, iDlg);
	CopyBytes(byCode + 9, iMobBase);

	DWORD iSelectMob = GetAddress(skCryptDec("KO_SELECT_MOB"));
	CopyBytes(byCode + 15, iSelectMob);

	ExecuteRemoteCode(byCode, sizeof(byCode));
}

bool ClientHandler::UseItem(uint32_t iItemID)
{
	std::map<uint32_t, __TABLE_ITEM>* pItemTable;
	if (!m_Bot->GetItemTable(&pItemTable))
		return false;

	std::map<uint32_t, __TABLE_UPC_SKILL>* pSkillTable;
	if (!m_Bot->GetSkillTable(&pSkillTable))
		return false;

	auto pItemData = pItemTable->find(iItemID);

	if (pItemData != pItemTable->end())
	{
		auto pSkillData = pSkillTable->find(pItemData->second.dwEffectID1);

		if (pSkillData != pSkillTable->end())
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

void ClientHandler::AttackProcess()
{
#ifdef DEBUG
	printf("ClientHandler::AttackProcess Started\n");
#endif

	while (m_bWorking)
	{
		try
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));

			if (GetTarget() == -1)
				continue;

			if (IsMovingToLoot())
				continue;

			if (IsBlinking())
				continue;

			if (IsRouting())
				continue;

			std::map<uint32_t, __TABLE_UPC_SKILL>* pSkillTable;
			if (!m_Bot->GetSkillTable(&pSkillTable))
				continue;

			std::map<uint32_t, __TABLE_UPC_SKILL_EXTENSION2>* pSkillExtension2;
			if (!m_Bot->GetSkillExtension2Table(&pSkillExtension2))
				continue;

			bool bLegalStatus = GetConfiguration()->GetBool(skCryptDec("Bot"), skCryptDec("Legal"), false);

			if (bLegalStatus && GetActionState() == PSA_SPELLMAGIC)
				continue;

			bool bAttackStatus = GetConfiguration()->GetBool(skCryptDec("Automation"), skCryptDec("Attack"), false);

			if (!bAttackStatus)
				continue;

			bool bBasicAttack = GetConfiguration()->GetBool(skCryptDec("Attack"), skCryptDec("BasicAttack"), true);
			bool bMoveToTarget = GetConfiguration()->GetBool(skCryptDec("Attack"), skCryptDec("MoveToTarget"), false);

			Vector3 v3TargetPosition = GetTargetPosition();

			DWORD iTargetBase = GetMobBase(GetClientSelectedTarget());

			if (iTargetBase == 0)
				continue;
			else
			{
				if (!IsEnemy(iTargetBase))
					continue;

				DWORD iHp = GetHp(iTargetBase);
				DWORD iMaxHp = GetMaxHp(iTargetBase);
				DWORD iState = GetActionState(iTargetBase);

				if ((iState == PSA_DYING || iState == PSA_DEATH) || (iMaxHp != 0 && iHp == 0))
					continue;

				if (bBasicAttack)
				{
					DWORD iIsInBasicAttackProcess = Read4Byte(Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR"))) + GetAddress(skCryptDec("KO_OFF_BASIC_ATTACK_STATE")));

					if (bMoveToTarget
						&& iIsInBasicAttackProcess == 0
						&& GetDistance(v3TargetPosition) >= 1.0f
						&& GetActionState() != PSA_SPELLMAGIC)
						BasicAttack();
				}
				else
				{
					if (bMoveToTarget
						&& GetDistance(v3TargetPosition) >= 1.0f
						&& GetActionState() != PSA_SPELLMAGIC)
					{
						SetMovePosition(v3TargetPosition);
					}
				}
			}

			bool bAttackRangeLimit = GetConfiguration()->GetBool(skCryptDec("Attack"), skCryptDec("AttackRangeLimit"), false);
			int iAttackRangeLimitValue = GetConfiguration()->GetInt(skCryptDec("Attack"), skCryptDec("AttackRangeLimitValue"), (int)MAX_ATTACK_RANGE);

			if (bAttackRangeLimit && GetDistance(v3TargetPosition) > (float)iAttackRangeLimitValue)
				continue;

			std::vector<int> vecAttackSkillList = GetConfiguration()->GetInt(skCryptDec("Automation"), skCryptDec("AttackSkillList"), std::vector<int>());
			bool bAttackSpeed = GetConfiguration()->GetBool("Attack", "AttackSpeed", false);

			if (vecAttackSkillList.size() == 0)
			{
				Vector3 v3TargetPosition = GetTargetPosition();

				if (GetDistance(v3TargetPosition) <= 3.0f)
				{
					if (bBasicAttack)
					{
						BasicAttack();
					}
				}

				if (bAttackSpeed)
				{
					int iAttackSpeedValue = GetConfiguration()->GetInt("Attack", "AttackSpeedValue", 1000);

					if (iAttackSpeedValue > 0)
						std::this_thread::sleep_for(std::chrono::milliseconds(iAttackSpeedValue));
					else
						std::this_thread::sleep_for(std::chrono::milliseconds(1));
				}
				else
					std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			} 
			else
			{
				auto pSort = [](int& a, int& b)
				{
					return a > b;
				};

				std::sort(vecAttackSkillList.begin(), vecAttackSkillList.end(), pSort);

				for (const auto& x : vecAttackSkillList)
				{
					DWORD iTargetBase = GetMobBase(GetClientSelectedTarget());

					if (iTargetBase == 0)
						break;
					else
					{
						DWORD iHp = GetHp(iTargetBase);
						DWORD iMaxHp = GetMaxHp(iTargetBase);
						DWORD iState = GetActionState(iTargetBase);

						if ((iState == PSA_DYING || iState == PSA_DEATH) || (iMaxHp != 0 && iHp == 0))
							break;

						Vector3 v3TargetPosition = GetTargetPosition();
						if (bAttackRangeLimit && GetDistance(v3TargetPosition) > (float)iAttackRangeLimitValue)
							break;

						if (!IsEnemy(iTargetBase))
							break;
					}

					auto pSkillData = pSkillTable->find(x);

					if (pSkillData != pSkillTable->end())
					{
						uint32_t iNeedItem = pSkillData->second.dwNeedItem;

						uint32_t iNeedItemCount = 1;
						uint32_t iExistItemCount = 0;

						if (iNeedItem != 0)
						{
							iExistItemCount = GetInventoryItemCount(pSkillData->second.dwNeedItem);

							auto pSkillExtension2Data = pSkillExtension2->find(pSkillData->second.iID);

							if (pSkillExtension2Data != pSkillExtension2->end())
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

						UseSkill(pSkillData->second, GetTarget(), true);

						if (bBasicAttack)
							BasicAttack();

						if (bAttackSpeed)
						{
							int iAttackSpeedValue = GetConfiguration()->GetInt("Attack", "AttackSpeedValue", 1000);

							if (iAttackSpeedValue > 0)
								std::this_thread::sleep_for(std::chrono::milliseconds(iAttackSpeedValue));
							else
								std::this_thread::sleep_for(std::chrono::milliseconds(1));
						}
						else
							std::this_thread::sleep_for(std::chrono::milliseconds(1000));
					}
				}
			}
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
			std::this_thread::sleep_for(std::chrono::milliseconds(1));

			if (IsBlinking())
				continue;

			if (IsMovingToLoot())
				continue;

			if (IsRouting())
				continue;

			bool bSearchTargetSpeed = GetConfiguration()->GetBool("Attack", "SearchTargetSpeed", false);

			if (bSearchTargetSpeed)
			{
				int iSearchTargetSpeedValue = GetConfiguration()->GetInt("Attack", "SearchTargetSpeedValue", 100);

				if (iSearchTargetSpeedValue > 0)
					std::this_thread::sleep_for(std::chrono::milliseconds(iSearchTargetSpeedValue));
				else
					std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}
			else
				std::this_thread::sleep_for(std::chrono::milliseconds(100));

			bool bLegalStatus = GetConfiguration()->GetBool(skCryptDec("Bot"), skCryptDec("Legal"), false);

			if (bLegalStatus && GetActionState() == PSA_SPELLMAGIC)
				continue;
			
			bool bAttackStatus = GetConfiguration()->GetBool(skCryptDec("Automation"), skCryptDec("Attack"), false);

			if (!bAttackStatus)
				continue;

			std::vector<EntityInfo> vecOutMobList;
			if (SearchMob(vecOutMobList) > 0)
			{
				std::vector<EntityInfo> vecFilteredTarget;

				bool bAutoTarget = GetConfiguration()->GetInt(skCryptDec("Attack"), skCryptDec("AutoTarget"), true);
				bool bRangeLimit = GetConfiguration()->GetBool(skCryptDec("Attack"), skCryptDec("RangeLimit"), false);
				int iRangeLimitValue = GetConfiguration()->GetInt(skCryptDec("Attack"), skCryptDec("RangeLimitValue"), (int)MAX_VIEW_RANGE);

				if (bAutoTarget)
				{
					std::copy_if(vecOutMobList.begin(), vecOutMobList.end(),
						std::back_inserter(vecFilteredTarget),
						[&](const EntityInfo& c)
						{
							return
								c.m_bEnemy == true
								&& c.m_iState != PSA_DYING
								&& c.m_iState != PSA_DEATH
								&& ((c.m_iMaxHP == 0) || (c.m_iMaxHP != 0 && c.m_iHP != 0))
								&& ((bRangeLimit && GetDistance(c.m_v3Position) <= (float)iRangeLimitValue) || !bRangeLimit);
						});
				}
				else
				{
					std::vector<int> vecSelectedNpcList = GetConfiguration()->GetInt(skCryptDec("Attack"), skCryptDec("NpcList"), std::vector<int>());

					std::copy_if(vecOutMobList.begin(), vecOutMobList.end(),
						std::back_inserter(vecFilteredTarget),
						[&](const EntityInfo& c)
						{
							return 
								c.m_bEnemy == true
								&& c.m_iState != PSA_DYING
								&& c.m_iState != PSA_DEATH
								&& ((c.m_iMaxHP == 0) || (c.m_iMaxHP != 0 && c.m_iHP != 0))
								&& std::count(vecSelectedNpcList.begin(), vecSelectedNpcList.end(), c.m_iProtoId)
								&& ((bRangeLimit && GetDistance(c.m_v3Position) <= (float)iRangeLimitValue) || !bRangeLimit);
						});
				}

				if (vecFilteredTarget.size() > 0)
				{
					auto pSort = [&](EntityInfo const& a, EntityInfo const& b)
					{
						if (a.m_fDistance != b.m_fDistance)
							return a.m_fDistance < b.m_fDistance;

						return false;
					};

					std::sort(vecFilteredTarget.begin(), vecFilteredTarget.end(), pSort);

					auto pSelectedTarget = vecFilteredTarget.at(0);

					if (pSelectedTarget.m_iId != GetClientSelectedTarget())
					{
						SetTarget(pSelectedTarget.m_iId);

						bool bBasicAttack = GetConfiguration()->GetBool(skCryptDec("Attack"), skCryptDec("BasicAttack"), true);
						bool bMoveToTarget = GetConfiguration()->GetBool(skCryptDec("Attack"), skCryptDec("MoveToTarget"), false);

						if (bBasicAttack)
						{
							DWORD iIsInBasicAttackProcess = Read4Byte(Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR"))) + GetAddress(skCryptDec("KO_OFF_BASIC_ATTACK_STATE")));

							if (bMoveToTarget
								&& GetDistance(pSelectedTarget.m_v3Position) >= 1.0f
								&& GetActionState() != PSA_SPELLMAGIC)
							{
								SetMovePosition(Vector3(0.0f, 0.0f, 0.0f));
								BasicAttack();
							}
						}
						else
						{
							if (bMoveToTarget
								&& GetDistance(pSelectedTarget.m_v3Position) >= 1.0f
								&& GetActionState() != PSA_SPELLMAGIC)
							{
								SetMovePosition(pSelectedTarget.m_v3Position);
							}
						}
					}
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

			if (IsRouting())
				continue;

			bool bAutoLoot = GetConfiguration()->GetBool(skCryptDec("AutoLoot"), skCryptDec("Enable"), false);

			if (bAutoLoot)
			{
				std::vector<TLoot> vecFilteredLoot;

				std::chrono::milliseconds msNow = duration_cast<std::chrono::milliseconds>(
					std::chrono::system_clock::now().time_since_epoch()
				);

				Guard lock(m_mutexLootList);
				std::copy_if(m_vecLootList.begin(), m_vecLootList.end(),
					std::back_inserter(vecFilteredLoot),
					[&](const TLoot& c)
					{
						return  (c.msDropTime.count() + 500) < msNow.count() && c.iRequestedOpen == false;
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

						auto pNpc = std::find_if(m_vecNpc.begin(), m_vecNpc.end(),
							[pFindedLoot](const TNpc& a)
							{
								return a.iID == pFindedLoot.iNpcID;
							});

						if (pNpc != m_vecNpc.end())
						{
							while (m_bWorking 
								&& (pFindedLoot.msDropTime.count() + 30000) > msNow.count()
								&& GetDistance(pNpc->fX, pNpc->fY) > 1.0f 
								&& GetDistance(pNpc->fX, pNpc->fY) < 50.0f)
							{
								std::this_thread::sleep_for(std::chrono::milliseconds(250));

								bool bLegalStatus = GetConfiguration()->GetBool(skCryptDec("Bot"), skCryptDec("Legal"), false);

								if (bLegalStatus && GetActionState() == PSA_SPELLMAGIC)
									continue;

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

			std::vector<__TABLE_UPC_SKILL>* vecAvailableSkills;
			if (!GetAvailableSkill(&vecAvailableSkills))
				continue;

			bool bMinorProtection = GetConfiguration()->GetBool(skCryptDec("Protection"), skCryptDec("Minor"), false);

			if (bMinorProtection)
			{
				int32_t iHpProtectionValue = GetConfiguration()->GetInt(skCryptDec("Protection"), skCryptDec("MinorValue"), 30);

				if (GetHp() > 0 && (int32_t)std::ceil((GetHp() * 100) / GetMaxHp()) < iHpProtectionValue)
				{
					auto it = std::find_if(vecAvailableSkills->begin(), vecAvailableSkills->end(),
						[](const TABLE_UPC_SKILL& a) { return a.szEngName == skCryptDec("minor healing"); });

					if (it == vecAvailableSkills->end())
						break;

					bool bUse = true;

					if (GetMp() < it->iExhaustMSP)
					{
						bUse = false;
					}

					if (bUse)
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

void ClientHandler::PotionProcess()
{
#ifdef DEBUG
	printf("ClientHandler::PotionProcess Started\n");
#endif

	while (m_bWorking)
	{
		try
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(100));

			if (IsBlinking())
				continue;

			bool bMpProtectionEnable = GetConfiguration()->GetBool(skCryptDec("Protection"), skCryptDec("Mp"), false);

			if (bMpProtectionEnable)
			{
				int16_t iMp = GetMp(); int16_t iMaxMp = GetMaxMp();

				if (iMp > 0 && iMaxMp > 0)
				{
					int32_t iMpProtectionPercent = (int32_t)std::ceil((iMp * 100) / iMaxMp);
					int32_t iMpProtectionValue = GetConfiguration()->GetInt(skCryptDec("Protection"), skCryptDec("MpValue"), 50);

					std::chrono::milliseconds msNow = duration_cast<std::chrono::milliseconds>(
						std::chrono::system_clock::now().time_since_epoch()
					);

					if (iMpProtectionPercent <= iMpProtectionValue && (m_msLastPotionUseTime.count() + 2000) <= msNow.count())
					{

						if (ManaPotionProcess())
						{
							m_msLastPotionUseTime = duration_cast<std::chrono::milliseconds>(
								std::chrono::system_clock::now().time_since_epoch()
							);
						}
					}
				}
			}

			bool bHpProtectionEnable = GetConfiguration()->GetBool(skCryptDec("Protection"), skCryptDec("Hp"), false);

			if (bHpProtectionEnable)
			{
				int16_t iHp = GetHp(); int16_t iMaxHp = GetMaxHp();

				if (iHp > 0 && iMaxHp > 0)
				{
					int32_t iHpProtectionPercent = (int32_t)std::ceil((iHp * 100) / iMaxHp);
					int32_t iHpProtectionValue = GetConfiguration()->GetInt(skCryptDec("Protection"), skCryptDec("HpValue"), 50);

					std::chrono::milliseconds msNow = duration_cast<std::chrono::milliseconds>(
						std::chrono::system_clock::now().time_since_epoch()
					);

					if (iHpProtectionPercent <= iHpProtectionValue && (m_msLastPotionUseTime.count() + 2000) <= msNow.count())
					{
						if (HealthPotionProcess())
						{
							m_msLastPotionUseTime = duration_cast<std::chrono::milliseconds>(
								std::chrono::system_clock::now().time_since_epoch()
							);
						}
					}
				}
			}
		}
		catch (const std::exception& e)
		{
#ifdef DEBUG
			printf("PotionProcess:Exception: %s\n", e.what());
#else
			UNREFERENCED_PARAMETER(e);
#endif
		}
	}

#ifdef DEBUG
		printf("ClientHandler::PotionProcess Stopped\n");
#endif
}

void ClientHandler::CharacterProcess()
{
#ifdef DEBUG
	printf("ClientHandler::CharacterProcess Started\n");
#endif

	while (m_bWorking)
	{
		try
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(100));

			if (IsBlinking())
				continue;

			std::vector<__TABLE_UPC_SKILL>* vecAvailableSkills;
			if (!GetAvailableSkill(&vecAvailableSkills))
				continue;

			std::map<uint32_t, __TABLE_UPC_SKILL>* pSkillTable;
			if (!m_Bot->GetSkillTable(&pSkillTable))
				continue;

			std::map<uint32_t, __TABLE_UPC_SKILL_EXTENSION2>* pSkillExtension2;
			if (!m_Bot->GetSkillExtension2Table(&pSkillExtension2))
				continue;

			std::map<uint32_t, __TABLE_UPC_SKILL_EXTENSION4>* pSkillExtension4;
			if (!m_Bot->GetSkillExtension4Table(&pSkillExtension4))
				continue;

			bool bLegalStatus = GetConfiguration()->GetBool(skCryptDec("Bot"), skCryptDec("Legal"), false);

			if (bLegalStatus && GetActionState() == PSA_SPELLMAGIC)
				continue;

			bool bCharacterStatus = GetConfiguration()->GetBool(skCryptDec("Automation"), skCryptDec("Character"), false);

			if (bCharacterStatus)
			{
				std::vector<int> vecCharacterSkillList = GetConfiguration()->GetInt(skCryptDec("Automation"), skCryptDec("CharacterSkillList"), std::vector<int>());

				if (vecCharacterSkillList.size() > 0)
				{
					auto pSort = [](int& a, int& b)
					{
						return a > b;
					};

					std::sort(vecCharacterSkillList.begin(), vecCharacterSkillList.end(), pSort);

					for (const auto& x : vecCharacterSkillList)
					{
						auto pSkillData = pSkillTable->find(x);

						if (pSkillData != pSkillTable->end())
						{
							uint32_t iNeedItem = pSkillData->second.dwNeedItem;

							uint32_t iNeedItemCount = 1;
							uint32_t iExistItemCount = 0;

							if (iNeedItem != 0)
							{
								iExistItemCount = GetInventoryItemCount(pSkillData->second.dwNeedItem);

								auto pSkillExtension2Data = pSkillExtension2->find(pSkillData->second.iID);

								if (pSkillExtension2Data != pSkillExtension2->end())
									iNeedItemCount = pSkillExtension2Data->second.iArrowCount;
							}

							if (GetMp() >= pSkillData->second.iExhaustMSP && (iNeedItem == 0 || (iNeedItem != 0 && iExistItemCount >= iNeedItemCount)))
							{
								std::chrono::milliseconds msNow = duration_cast<std::chrono::milliseconds>(
									std::chrono::system_clock::now().time_since_epoch()
								);

								std::chrono::milliseconds msLastSkillUseItem = GetSkillUseTime(pSkillData->second.iID);

								bool bUse = true;

								if (pSkillData->second.iCooldown > 0 && msLastSkillUseItem.count() > 0)
								{
									int64_t iSkillCooldownTime = static_cast<int64_t>(pSkillData->second.iCooldown) * 100;

									if ((msLastSkillUseItem.count() + iSkillCooldownTime) > msNow.count())
										bUse = false;
								}

								auto pSkillExtension4Data = pSkillExtension4->find(pSkillData->second.iID);

								if (pSkillExtension4Data != pSkillExtension4->end())
								{
									if (IsBuffActive(pSkillExtension4Data->second.iBuffType))
										bUse = false;
								}

								if (bUse)
								{
									UseSkill(pSkillData->second, GetID());
								}
							}
						}
					}
				}
			}

			if (IsPriest())
			{
				bool bAutoHealthBuff = GetConfiguration()->GetBool(skCryptDec("Priest"), skCryptDec("AutoHealthBuff"), true);

				int iSelectedHealthBuff = Drawing::Bot->GetConfiguration()->GetInt(skCryptDec("Priest"), skCryptDec("SelectedHealthBuff"), -1);

				if (iSelectedHealthBuff == -1)
				{
					iSelectedHealthBuff = GetProperHealthBuff(GetMaxHp());
				}

				if (bAutoHealthBuff && iSelectedHealthBuff != -1 && !IsBuffActive(BuffType::BUFF_TYPE_HP_MP))
				{
					auto it = std::find_if(vecAvailableSkills->begin(), vecAvailableSkills->end(),
						[iSelectedHealthBuff](const TABLE_UPC_SKILL& a) { return a.iBaseId == iSelectedHealthBuff; });

					if (it != vecAvailableSkills->end())
					{
						std::chrono::milliseconds msNow = duration_cast<std::chrono::milliseconds>(
							std::chrono::system_clock::now().time_since_epoch()
						);

						std::chrono::milliseconds msLastSkillUseItem = Client::GetSkillUseTime(it->iID);

						bool bUse = true;

						if (it->iCooldown > 0 && msLastSkillUseItem.count() > 0)
						{
							int64_t iSkillCooldownTime = static_cast<int64_t>(it->iCooldown) * 100;

							if ((msLastSkillUseItem.count() + iSkillCooldownTime) > msNow.count())
								bUse = false;
						}

						if (GetMp() < it->iExhaustMSP)
						{
							bUse = false;
						}

						if (bUse)
						{
							UseSkill(*it, GetID());
						}
					}
				}

				bool bAutoDefenceBuff = GetConfiguration()->GetBool(skCryptDec("Priest"), skCryptDec("AutoDefenceBuff"), true);

				int iSelectedDefenceBuff = Drawing::Bot->GetConfiguration()->GetInt(skCryptDec("Priest"), skCryptDec("SelectedDefenceBuff"), -1);

				if (iSelectedDefenceBuff == -1)
				{
					iSelectedDefenceBuff = GetProperDefenceBuff();
				}

				if (bAutoDefenceBuff && iSelectedDefenceBuff != -1 && !IsBuffActive(BuffType::BUFF_TYPE_AC))
				{
					auto it = std::find_if(vecAvailableSkills->begin(), vecAvailableSkills->end(),
						[iSelectedDefenceBuff](const TABLE_UPC_SKILL& a) { return a.iBaseId == iSelectedDefenceBuff; });

					if (it != vecAvailableSkills->end())
					{
						std::chrono::milliseconds msNow = duration_cast<std::chrono::milliseconds>(
							std::chrono::system_clock::now().time_since_epoch()
						);

						std::chrono::milliseconds msLastSkillUseItem = Client::GetSkillUseTime(it->iID);

						bool bUse = true;

						if (it->iCooldown > 0 && msLastSkillUseItem.count() > 0)
						{
							int64_t iSkillCooldownTime = static_cast<int64_t>(it->iCooldown) * 100;

							if ((msLastSkillUseItem.count() + iSkillCooldownTime) > msNow.count())
								bUse = false;
						}

						if (GetMp() < it->iExhaustMSP)
						{
							bUse = false;
						}

						if (bUse)
						{
							UseSkill(*it, GetID());
						}
					}
				}

				bool bAutoMindBuff = GetConfiguration()->GetBool(skCryptDec("Priest"), skCryptDec("AutoMindBuff"), true);

				int iSelectedMindBuff = Drawing::Bot->GetConfiguration()->GetInt(skCryptDec("Priest"), skCryptDec("SelectedMindBuff"), -1);

				if (iSelectedMindBuff == -1)
				{
					iSelectedMindBuff = GetProperMindBuff();
				}

				if (bAutoMindBuff && iSelectedMindBuff != -1 && !IsBuffActive(BuffType::BUFF_TYPE_RESISTANCES))
				{
					auto it = std::find_if(vecAvailableSkills->begin(), vecAvailableSkills->end(),
						[iSelectedMindBuff](const TABLE_UPC_SKILL& a) { return a.iBaseId == iSelectedMindBuff; });

					if (it != vecAvailableSkills->end())
					{
						std::chrono::milliseconds msNow = duration_cast<std::chrono::milliseconds>(
							std::chrono::system_clock::now().time_since_epoch()
						);

						std::chrono::milliseconds msLastSkillUseItem = Client::GetSkillUseTime(it->iID);

						bool bUse = true;

						if (it->iCooldown > 0 && msLastSkillUseItem.count() > 0)
						{
							int64_t iSkillCooldownTime = static_cast<int64_t>(it->iCooldown) * 100;

							if ((msLastSkillUseItem.count() + iSkillCooldownTime) > msNow.count())
								bUse = false;
						}

						if (GetMp() < it->iExhaustMSP)
						{
							bUse = false;
						}

						if (bUse)
						{
							UseSkill(*it, GetID());
						}
					}
				}

				bool bAutoHeal = GetConfiguration()->GetBool(skCryptDec("Priest"), skCryptDec("AutoHeal"), true);
				int iSelectedHeal = GetConfiguration()->GetInt(skCryptDec("Priest"), skCryptDec("SelectedHeal"), -1);

				if (iSelectedHeal == -1)
				{
					iSelectedHeal = GetProperHeal();
				}

				int16_t iHp = GetHp(); int16_t iMaxHp = GetMaxHp();

				int iAutoHealValue = GetConfiguration()->GetInt(skCryptDec("Priest"), skCryptDec("AutoHealValue"), 75);
				int32_t iHpProtectionPercent = (int32_t)std::ceil((iHp * 100) / iMaxHp);

				if (bAutoHeal && iSelectedHeal != -1 && iHpProtectionPercent <= iAutoHealValue)
				{
					auto it = std::find_if(vecAvailableSkills->begin(), vecAvailableSkills->end(),
						[iSelectedHeal](const TABLE_UPC_SKILL& a) { return a.iBaseId == iSelectedHeal; });

					if (it != vecAvailableSkills->end())
					{
						std::chrono::milliseconds msNow = duration_cast<std::chrono::milliseconds>(
							std::chrono::system_clock::now().time_since_epoch()
						);

						std::chrono::milliseconds msLastSkillUseItem = Client::GetSkillUseTime(it->iID);

						bool bUse = true;

						if (it->iCooldown > 0 && msLastSkillUseItem.count() > 0)
						{
							int64_t iSkillCooldownTime = static_cast<int64_t>(it->iCooldown) * 100;

							if ((msLastSkillUseItem.count() + iSkillCooldownTime) > msNow.count())
								bUse = false;
						}

						if (GetMp() < it->iExhaustMSP)
						{
							bUse = false;
						}

						if (bUse)
						{
							UseSkill(*it, GetID());
						}
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

bool ClientHandler::HealthPotionProcess()
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

	TInventory tInventoryItem;

	for (const auto& x : vecPotions)
	{
		tInventoryItem = GetInventoryItem(x);

		if (tInventoryItem.iItemID != 0)
		{
			return UseItem(tInventoryItem.iItemID);
		}
	}

	return false;
}

bool ClientHandler::ManaPotionProcess()
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
		899997000, 399020000, 389082000, 931787000,
	};

	auto pSort = [](uint32_t const& a, uint32_t const& b)
	{
		return a < b;
	};

	std::sort(vecPotions.begin(), vecPotions.end(), pSort);

	TInventory tInventoryItem;

	for (const auto& x : vecPotions)
	{
		tInventoryItem = GetInventoryItem(x);

		if (tInventoryItem.iItemID != 0)
		{
			return UseItem(tInventoryItem.iItemID);
		}
	}	

	return false;
}

void ClientHandler::GodModeProcess()
{
#ifdef DEBUG
	printf("ClientHandler::GodModeProcess Started\n");
#endif

	while (m_bWorking)
	{
		try
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));

			if (IsBlinking())
				continue;

			std::map<uint32_t, __TABLE_UPC_SKILL>* pSkillTable;
			if (!m_Bot->GetSkillTable(&pSkillTable))
				continue;

			bool bGodMode = GetConfiguration()->GetBool(skCryptDec("Protection"), skCryptDec("GodMode"), false);

			if (bGodMode)
			{
				auto pSkillData = pSkillTable->find(500344);

				if (pSkillData != pSkillTable->end())
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
						|| !IsBuffActive(BuffType::BUFF_TYPE_HP_MP))
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

			Route pRoute = m_vecRoute.front();

			float fDistance = GetDistance(Vector3(GetX(), 0.0f, GetY()), Vector3(pRoute.fX, 0.0f, pRoute.fY));

			if (fDistance > 3.0f)
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
						auto jSupplyList = m_Bot->GetSupplyList();

						std::map<int32_t, SSupplyBuyList> mapSupplyBuyList;

						bool bAutoSupply = GetConfiguration()->GetBool(skCryptDec("Supply"), skCryptDec("AutoSupply"), false);
						bool bAutoRepair = GetConfiguration()->GetBool(skCryptDec("Supply"), skCryptDec("AutoRepair"), false);

						if (bAutoSupply && jSupplyList.size() > 0)
						{
							std::vector<int32_t> vecExceptedSlotPos;

							for (size_t i = 0; i < jSupplyList.size(); i++)
							{
								std::vector<int> vecSupplyList = GetConfiguration()->GetInt(skCryptDec("Supply"), skCryptDec("Enable"), std::vector<int>());

								std::string szItemIdAttribute = skCryptDec("itemid");
								std::string szSellingGroupAttribute = skCryptDec("sellinggroup");
								std::string szCountAttribute = skCryptDec("count");

								int32_t iItemId = jSupplyList[i][szItemIdAttribute.c_str()].get<int32_t>();
								int32_t iSellingGroup = jSupplyList[i][szSellingGroupAttribute.c_str()].get<int32_t>();

								bool bSelected = std::find(vecSupplyList.begin(), vecSupplyList.end(), iItemId) != vecSupplyList.end();

								if (!bSelected)
									continue;

								auto findedNpc = std::find_if(m_vecNpc.begin(), m_vecNpc.end(),
									[iSellingGroup](const TNpc& a) { return a.iSellingGroup == iSellingGroup; });

								if (findedNpc != m_vecNpc.end())
								{
									int32_t iFindedNpcId = findedNpc->iID;
									Vector3 vec3FindedNpcPosition = Vector3(findedNpc->fX, 0.0f, findedNpc->fY);

									std::vector<SShopItem> vecShopItemTable;
									if (!m_Bot->GetShopItemTable(iSellingGroup, vecShopItemTable))
										continue;

									uint8_t iInventoryPosition = -1;
									int16_t iItemCount = (int16_t)GetConfiguration()->GetInt(
										skCryptDec("Supply"),
										std::to_string(iItemId).c_str(),
										jSupplyList[i][szCountAttribute.c_str()].get<int16_t>());

									TInventory pInventoryItem = GetInventoryItem(iItemId);

									if (pInventoryItem.iItemID != 0)
									{
										iInventoryPosition = (uint8_t)pInventoryItem.iPos;
										iItemCount = (int16_t)std::abs(pInventoryItem.iCount - iItemCount);
									}
									else
									{
										iInventoryPosition = (uint8_t)GetInventoryEmptySlot(vecExceptedSlotPos);
									}

									if (iInventoryPosition == -1 || iItemCount == 0)
										continue;

									auto findedShopItem = std::find_if(vecShopItemTable.begin(), vecShopItemTable.end(),
										[iItemId](const SShopItem& a) { return a.m_iItemId == iItemId; });

									if (findedShopItem != vecShopItemTable.end())
									{
										auto pSupplyBuyList = mapSupplyBuyList.find(iSellingGroup);

										if (pSupplyBuyList == mapSupplyBuyList.end())
										{
											std::vector<SSItemBuy> m_vecItemBuy;
											m_vecItemBuy.push_back(SSItemBuy(findedShopItem->m_iItemId, (iInventoryPosition - 14), iItemCount, findedShopItem->m_iPage, findedShopItem->m_iPos));
											mapSupplyBuyList.insert(std::make_pair(iSellingGroup, SSupplyBuyList(iFindedNpcId, vec3FindedNpcPosition, m_vecItemBuy)));
										}
										else
										{
											pSupplyBuyList->second.m_vecItemBuy.push_back(
												SSItemBuy(findedShopItem->m_iItemId, (iInventoryPosition - 14), iItemCount, findedShopItem->m_iPage, findedShopItem->m_iPos));
										}

										vecExceptedSlotPos.push_back(iInventoryPosition);
									}
								}
							}

							for (auto& e : mapSupplyBuyList)
							{
								while (GetDistance(Vector3(GetX(), 0.0f, GetY()), e.second.m_vec3NpcPosition) > 3.0f)
								{
									std::this_thread::sleep_for(std::chrono::milliseconds(100));
									SetMovePosition(e.second.m_vec3NpcPosition);
								}

								std::this_thread::sleep_for(std::chrono::milliseconds(1500));
								SendNpcEvent(e.second.m_iNpcId);

								std::this_thread::sleep_for(std::chrono::milliseconds(1500));
								SendItemTradeBuy(e.first, e.second.m_iNpcId, e.second.m_vecItemBuy);

								std::this_thread::sleep_for(std::chrono::milliseconds(1500));
								SendShoppingMall(ShoppingMallType::STORE_CLOSE);

								if (bAutoRepair && e.first == 255000)
								{
									std::this_thread::sleep_for(std::chrono::milliseconds(1500));

									for (uint8_t i = 0; i < SLOT_MAX; i++)
									{
										switch (i)
										{
											case 1:
											case 4:
											case 6:
											case 8:
											case 10:
											case 12:
											case 13:
											{
												TInventory pInventory = GetInventoryItemSlot(i);

												if (pInventory.iItemID == 0)
													continue;

												if (pInventory.iDurability > 1000)
													continue;

												SendItemRepair(1, (uint8_t)pInventory.iPos, e.second.m_iNpcId, pInventory.iItemID);
												std::this_thread::sleep_for(std::chrono::milliseconds(500));
											}
											break;
										}
									}
								}
							}
						}

						if (bAutoRepair && (jSupplyList.size() == 0 || mapSupplyBuyList.size() == 0))
						{
							auto findedNpc = std::find_if(m_vecNpc.begin(), m_vecNpc.end(),
								[](const TNpc& a) { return a.iSellingGroup == 255000; });

							if (findedNpc != m_vecNpc.end())
							{
								int32_t iFindedNpcId = findedNpc->iID;
								Vector3 vec3FindedNpcPosition = Vector3(findedNpc->fX, 0.0f, findedNpc->fY);

								while (GetDistance(Vector3(GetX(), 0.0f, GetY()), vec3FindedNpcPosition) > 3.0f)
								{
									std::this_thread::sleep_for(std::chrono::milliseconds(100));
									SetMovePosition(vec3FindedNpcPosition);
								}

								std::this_thread::sleep_for(std::chrono::milliseconds(1500));
								SendNpcEvent(iFindedNpcId);

								std::this_thread::sleep_for(std::chrono::milliseconds(1500));

								for (uint8_t i = 0; i < SLOT_MAX; i++)
								{
									switch (i)
									{
										case 1:
										case 4:
										case 6:
										case 8:
										case 10:
										case 12:
										case 13:
										{
											TInventory pInventory = GetInventoryItemSlot(i);

											if (pInventory.iItemID == 0)
												continue;

											if (pInventory.iDurability > 1000)
												continue;

											SendItemRepair(1, (uint8_t)pInventory.iPos, iFindedNpcId, pInventory.iItemID);
											std::this_thread::sleep_for(std::chrono::milliseconds(500));
										}
										break;
									}
								}
							}
						}

						m_msLastSupplyTime = duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());

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

void ClientHandler::SupplyProcess()
{
#ifdef DEBUG
	printf("ClientHandler::SupplyProcess Started\n");
#endif

	while (m_bWorking)
	{
		try
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(100));

			if (IsBlinking())
				continue;

			if (IsRouting())
				continue;

			std::chrono::milliseconds msCurrentTime = duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());

			if (m_msLastSupplyTime > std::chrono::milliseconds(0) && (msCurrentTime - m_msLastSupplyTime) < std::chrono::milliseconds((60 * 5) * 1000))
				continue;

			bool bAutoRepair = GetConfiguration()->GetBool(skCryptDec("Supply"), skCryptDec("AutoRepair"), false);
			bool bAutoSupply = GetConfiguration()->GetBool(skCryptDec("Supply"), skCryptDec("AutoSupply"), false);

			if (bAutoSupply && IsNeedSupply() || bAutoRepair && IsNeedRepair())
			{
				std::string szSelectedRoute = GetConfiguration()->GetString(skCryptDec("Bot"), skCryptDec("SelectedRoute"), "");

				RouteManager* pRouteManager = m_Bot->GetRouteManager();
				RouteManager::RouteList pRouteList;

				if (pRouteManager && pRouteManager->GetRouteList(GetZone(), pRouteList))
				{
					auto pRoute = pRouteList.find(szSelectedRoute);

					if (pRoute != pRouteList.end())
					{
						SetRoute(pRoute->second);
					}
				}
			}
		}
		catch (const std::exception& e)
		{
#ifdef DEBUG
			printf("SupplyProcess:Exception: %s\n", e.what());
#else
			UNREFERENCED_PARAMETER(e);
#endif
		}
	}

#ifdef DEBUG
	printf("ClientHandler::SupplyProcess Stopped\n");
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
	m_vecRoute = vecRoute;
}

void ClientHandler::ClearRoute()
{
	m_vecRoute.clear();
}

void ClientHandler::SendNpcEvent(int32_t iTargetID)
{
	Packet pkt = Packet(WIZ_NPC_EVENT);

	pkt << uint8_t(1) << int32_t(iTargetID) << int32_t(-1);

	SendPacket(pkt);
}

void ClientHandler::SendItemTradeBuy(uint32_t iSellingGroup, int32_t iNpcId, int32_t iItemId, uint8_t iInventoryPosition, int16_t iCount, uint8_t iShopPage, uint8_t iShopPosition)
{
	Packet pkt = Packet(WIZ_ITEM_TRADE);

	pkt
		<< uint8_t(1)
		<< uint32_t(iSellingGroup)
		<< uint32_t(iNpcId)
		<< uint8_t(1)
		<< int32_t(iItemId)
		<< uint8_t(iInventoryPosition)
		<< int16_t(iCount)
		<< uint8_t(iShopPage)
		<< uint8_t(iShopPosition);

	SendPacket(pkt);
}

void ClientHandler::SendItemTradeBuy(uint32_t iSellingGroup, int32_t iNpcId, std::vector<SSItemBuy> vecItemList)
{
	Packet pkt = Packet(WIZ_ITEM_TRADE);

	pkt
		<< uint8_t(1)
		<< uint32_t(iSellingGroup)
		<< uint32_t(iNpcId)
		<< uint8_t(vecItemList.size());

	for (auto &e : vecItemList)
	{
		pkt  
			<< int32_t(e.m_iItemId)
			<< uint8_t(e.m_iInventoryPosition)
			<< int16_t(e.m_iCount)
			<< uint8_t(e.m_iShopPage)
			<< uint8_t(e.m_iShopPosition);
	}

	SendPacket(pkt);
}

void ClientHandler::SendItemTradeSell(uint32_t iSellingGroup, int32_t iNpcId, int32_t iItemId, uint8_t iInventoryPosition, int16_t iCount)
{
	Packet pkt = Packet(WIZ_ITEM_TRADE);

	pkt
		<< uint8_t(2)
		<< uint32_t(iSellingGroup)
		<< uint32_t(iNpcId)
		<< uint8_t(1)
		<< int32_t(iItemId)
		<< uint8_t(iInventoryPosition)
		<< int16_t(iCount);

	SendPacket(pkt);
}

void ClientHandler::SendItemTradeSell(uint32_t iSellingGroup, int32_t iNpcId, std::vector<SSItemSell> vecItemList)
{
	Packet pkt = Packet(WIZ_ITEM_TRADE);

	pkt
		<< uint8_t(2)
		<< uint32_t(iSellingGroup)
		<< uint32_t(iNpcId)
		<< uint8_t(vecItemList.size());

	for (auto& e : vecItemList)
	{
		pkt
			<< int32_t(e.m_iItemId)
			<< uint8_t(e.m_iInventoryPosition)
			<< int16_t(e.m_iCount);
	}

	SendPacket(pkt);
}

void ClientHandler::SendItemRepair(uint8_t iDirection, uint8_t iInventoryPosition, int32_t iNpcId, int32_t iItemId)
{
	Packet pkt = Packet(WIZ_ITEM_REPAIR);

	pkt
		<< uint8_t(iDirection)
		<< uint8_t(iInventoryPosition)
		<< int32_t(iNpcId)
		<< int32_t(iItemId);

	SendPacket(pkt);
}

bool ClientHandler::IsNeedRepair()
{
	for (uint8_t i = 0; i < SLOT_MAX; i++)
	{
		switch (i)
		{
			case 1:
			case 4:
			case 6:
			case 8:
			case 10:
			case 12:
			case 13:
			{
				TInventory pInventory = GetInventoryItemSlot(i);

				if (pInventory.iItemID == 0)
					continue;

				if (pInventory.iDurability != 0)
					continue;

				return true;
			}
			break;
		}
	}

	return false;
}

bool ClientHandler::IsNeedSupply()
{
	auto jSupplyList = m_Bot->GetSupplyList();

	if (jSupplyList.size() > 0)
	{
		for (size_t i = 0; i < jSupplyList.size(); i++)
		{
			std::string szItemIdAttribute = skCryptDec("itemid");
			std::string szSellingGroupAttribute = skCryptDec("sellinggroup");
			std::string szCountAttribute = skCryptDec("count");

			int32_t iSupplyItemId = jSupplyList[i][szItemIdAttribute.c_str()].get<int32_t>();
			int32_t iSupplyItemCount = jSupplyList[i][szCountAttribute.c_str()].get<int32_t>();

			std::vector<int> vecSupplyList = GetConfiguration()->GetInt(skCryptDec("Supply"), skCryptDec("Enable"), std::vector<int>());

			bool bSelected = std::find(vecSupplyList.begin(), vecSupplyList.end(), iSupplyItemId) != vecSupplyList.end();

			if (!bSelected)
				continue;

			TInventory pInventoryItem = GetInventoryItem(iSupplyItemId);

			if (pInventoryItem.iItemID == 0)
				return true;

			if (pInventoryItem.iItemID != 0)
			{
				if (pInventoryItem.iCount <= 3 && pInventoryItem.iCount < iSupplyItemCount)
					return true;
			}
		}
	}

	return false;
}

void ClientHandler::StepCharacterForward(bool bStart)
{
	BYTE byCode[] =
	{
		0x60,
		0x8B, 0x0D, 0x00, 0x00, 0x00, 0x00,
		0x6A, 0x01,
		0x6A, 0x00,
		0xB8, 0x00, 0x00, 0x00, 0x00,
		0xFF, 0xD0,
		0x61,
		0xC3,
	};

	DWORD iDlg = GetAddress(skCryptDec("KO_PTR_DLG"));
	CopyBytes(byCode + 3, iDlg);

	int8_t iStart = bStart ? 1 : 0;
	CopyBytes(byCode + 10, iStart);

	DWORD iWscb = GetAddress(skCryptDec("KO_WSCB"));
	CopyBytes(byCode + 12, iWscb);

	ExecuteRemoteCode(byCode, sizeof(byCode));
}

void ClientHandler::BasicAttack()
{
	DWORD iMobBase = GetMobBase(GetTarget());

	if (iMobBase == 0)
		return;

	if (!IsEnemy(iMobBase))
		return;

	DWORD iHp = GetHp(iMobBase);
	DWORD iMaxHp = GetMaxHp(iMobBase);
	DWORD iState = GetActionState(iMobBase);

	if ((iState == PSA_DYING || iState == PSA_DEATH) || (iMaxHp != 0 && iHp == 0))
		return;

	BYTE byCode[] =
	{
		0x60,
		0x8B, 0x0D, 0x00, 0x00, 0x00, 0x00,
		0xB8, 0x00, 0x00, 0x00, 0x00,
		0xFF, 0xD0,
		0x61,
		0xC3
	};

	DWORD iDlg = GetAddress(skCryptDec("KO_PTR_DLG"));
	CopyBytes(byCode + 3, iDlg);

	DWORD iLrca = GetAddress(skCryptDec("KO_LRCA"));
	CopyBytes(byCode + 8, iLrca);

	ExecuteRemoteCode(byCode, sizeof(byCode));
}

DWORD ClientHandler::GetSkillBase(uint32_t iSkillID)
{
	DWORD iFlags;
	if (!GetHandleInformation(m_Bot->m_hInjectedProcessHandle, &iFlags))
		m_Bot->m_hInjectedProcessHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, m_Bot->GetInjectedProcessId());

	LPVOID pBaseAddress = VirtualAllocEx(m_Bot->m_hInjectedProcessHandle, 0, 1, MEM_COMMIT, PAGE_EXECUTE_READWRITE);

	if (pBaseAddress == 0)
	{
		return 0;
	}

	BYTE byCode[] =
	{
		0x60,
		0x8B, 0x0D, 0x00, 0x00, 0x00, 0x00,
		0x68, 0x00, 0x00, 0x00, 0x00,
		0xBF, 0x00, 0x00, 0x00, 0x00,
		0xFF, 0xD7,
		0xA3, 0x00, 0x00, 0x00, 0x00,
		0x61,
		0xC3,
	};

	DWORD iSbec = GetAddress(skCryptDec("KO_SBEC"));
	CopyBytes(byCode + 3, iSbec);
	CopyBytes(byCode + 8, iSkillID);

	DWORD iSbca = GetAddress(skCryptDec("KO_SBCA"));
	CopyBytes(byCode + 13, iSbca);
	CopyBytes(byCode + 20, pBaseAddress);

	ExecuteRemoteCode(byCode, sizeof(byCode));

	DWORD iSkillBase = Read4Byte((DWORD)pBaseAddress);

	VirtualFreeEx(m_Bot->GetInjectedProcessHandle(), pBaseAddress, 0, MEM_RELEASE);

	return iSkillBase;
}

void ClientHandler::StopMove()
{
	if (GetMoveState() == PSM_STOP)
		return;

	DWORD iFlags;
	if (!GetHandleInformation(m_Bot->m_hInjectedProcessHandle, &iFlags))
		m_Bot->m_hInjectedProcessHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, m_Bot->GetInjectedProcessId());

	LPVOID pBaseAddress = VirtualAllocEx(m_Bot->m_hInjectedProcessHandle, 0, 1, MEM_COMMIT, PAGE_EXECUTE_READWRITE);

	if (pBaseAddress == nullptr)
		return;

	BYTE byCode[] =
	{
		0x60,
		0x6A, 0x00,
		0x6A, 0x00,
		0xB9, 0x00, 0x00, 0x00, 0x00,
		0x8B, 0x09,
		0xB8, 0x00, 0x00, 0x00, 0x00,
		0xFF, 0xD0,
		0x61,
		0xC3,
	};

	DWORD iDLG = GetAddress(skCryptDec("KO_PTR_DLG"));
	CopyBytes(byCode + 6, iDLG);

	DWORD i06 = GetAddress(skCryptDec("KO_PTR_06"));
	CopyBytes(byCode + 13, i06);

	ExecuteRemoteCode(byCode, sizeof(byCode));
}