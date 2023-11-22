#include "pch.h"
#include "ClientHandler.h"
#include "Client.h"
#include "Memory.h"
#include "Packet.h"
#include "Bot.h"
#include "Service.h"
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
#include <base64.h>
#include "UI.h"
#include "Password.h"
#include "Compression.h"

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

	m_bMailSlotWorking = false;
	m_hMailSlotRecv = nullptr;
	m_szMailSlotRecvName.clear();
	m_hMailSlotSend = nullptr;
	m_szMailSlotSendName.clear();

	m_RecvHookAddress = 0;
	m_SendHookAddress = 0;

	m_szAccountId.clear();
	m_szPassword.clear();

	m_ClientHook = nullptr;

	m_vecRoute.clear();

	m_msLastSupplyTime = std::chrono::milliseconds(0);
	m_msLastSelectedTargetTime = std::chrono::milliseconds(0);

	m_iRouteStep = RouteStepType::STEP_NONE;

	m_iOTPRetryCount = 0;

	m_fLastSearchTargetTime = 0.0f;
	m_fLastAttackTime = 0.0f;
	m_fLastCharacterProcessTime = 0.0f;
	m_fLastMinorProcessTime = 0.0f;
	m_fLastMagicHammerProcessTime = 0.0f;
	m_fLastAutoLootProcessTime = 0.0f;
	m_fLastTransformationProcessTime = 0.0f;
	m_fLastRegionProcessTime = 0.0f;
	m_fLastLootRequestTime = 0.0f;
	m_fLastPotionProcessTime = 0.0f;
}

void ClientHandler::Initialize()
{
#ifdef DEBUG
	printf("Client handler initializing\n");
#endif

	PatchSocket();
}

void ClientHandler::StartHandler()
{
#ifdef DEBUG
	printf("Client handler starting\n");
#endif

	m_bWorking = true;
}

void ClientHandler::StopHandler()
{
#ifdef DEBUG
	printf("Client handler stopped\n");
#endif

	m_bWorking = false;
	m_bMailSlotWorking = false;

	GetClient()->Clear();
}

void ClientHandler::Process()
{
	if (m_bMailSlotWorking)
	{
		MailSlotRecvProcess();
		MailSlotSendProcess();
	}

	if (m_bWorking)
	{
		if (IsDisconnect())
		{
#ifdef DEBUG
			printf("Client connection closed\n");
#endif
			StopHandler();
		}
		else
		{

			RegionProcess();
			PotionProcess();

			if (IsRogue())
			{
				MinorProcess();
			}

			CharacterProcess();

			SearchTargetProcess();
			MoveToTargetProcess();
			BasicAttackProcess();
			AttackProcess();

			MagicHammerProcess();
			AutoLootProcess();
			TransformationProcess();
			FlashProcess();
			SpeedHackProcess();

			bool bCharacterSizeEnable = GetUserConfiguration()->GetBool(skCryptDec("Character"), skCryptDec("SizeEnable"), false);

			if (bCharacterSizeEnable)
			{
				int iCharacterSize = GetUserConfiguration()->GetInt(skCryptDec("Character"), skCryptDec("Size"), 0);

				if (GetScaleZ() != (float)iCharacterSize)
				{
					DWORD iMyBase = Read4Byte(GetAddress(skCryptDec("KO_PTR_CHR")));
					SetScale(iMyBase, (float)iCharacterSize, (float)iCharacterSize, (float)iCharacterSize);
				}
			}
		}
	}
}

void ClientHandler::OnReady()
{
#ifdef DEBUG
	printf("Client handler ready\n");
#endif

	new std::thread([&]()
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(500));

		PushPhase(GetAddress(skCryptDec("KO_PTR_INTRO")));

		WaitCondition(Read4Byte(Read4Byte(GetAddress(skCryptDec("KO_PTR_INTRO"))) + GetAddress(skCryptDec("KO_OFF_UI_LOGIN_INTRO"))) == 0);

		if (m_Bot->m_iSelectedAccount != -1 && m_Bot->m_jAccountList.size() > 0)
		{
			m_Bot->m_jSelectedAccount = m_Bot->m_jAccountList.at(m_Bot->m_iSelectedAccount);

			if (m_Bot->m_ePlatformType != PlatformType::STKO)
			{
				std::string szAccountIdAttribute = skCryptDec("accountId");
				std::string szPasswordAttribute = skCryptDec("password");

				SetLoginInformation(
					m_Bot->m_jSelectedAccount[szAccountIdAttribute.c_str()].get<std::string>(),
					m_Bot->m_jSelectedAccount[szPasswordAttribute.c_str()].get<std::string>());

				std::this_thread::sleep_for(std::chrono::milliseconds(500));

				WriteLoginInformation(m_szAccountId, m_szPassword);
				ConnectLoginServer();
			}
		}
	});
}

void ClientHandler::PatchSocket()
{
	onClientSendProcess = [=](BYTE* iStream, DWORD iStreamLength)
	{
		SendProcess(iStream, iStreamLength);
	};

	onClientRecvProcess = [=](BYTE* iStream, DWORD iStreamLength)
	{
		RecvProcess(iStream, iStreamLength);
	};

	for (size_t i = 0; i < 12; i++)
		PatchRecvAddress(GetAddress(skCryptDec("KO_PTR_INTRO")) + (4 * i));

	PatchSendAddress();

	m_bMailSlotWorking = true;

	OnReady();
}

//void ClientHandler::PatchRecvAddress(DWORD iAddress)
//{
//	HANDLE hProcess = m_Bot->GetInjectedProcessHandle();
//
//	DWORD iAddressReady = 0;
//	while (iAddressReady == 0)
//	{
//		ReadProcessMemory(hProcess, (LPVOID)iAddress, &iAddressReady, 4, 0);
//	}
//
//	HMODULE hModuleKernel32 = GetModuleHandle(skCryptDec("kernel32.dll"));
//
//	if (hModuleKernel32 == nullptr)
//	{
//#ifdef DEBUG
//		printf("hModuleKernel32 == nullptr\n");
//#endif
//		return;
//	}
//
//	LPVOID pCreateFilePtr = GetProcAddress(hModuleKernel32, skCryptDec("CreateFileA"));
//	LPVOID pWriteFilePtr = GetProcAddress(hModuleKernel32, skCryptDec("WriteFile"));
//	LPVOID pCloseHandlePtr = GetProcAddress(hModuleKernel32, skCryptDec("CloseHandle"));
//
//	m_szMailSlotRecvName = skCryptDec("\\\\.\\mailslot\\KOF1\\") + std::to_string(m_Bot->GetInjectedProcessId());
//	std::vector<BYTE> vecMailSlotName(m_szMailSlotRecvName.begin(), m_szMailSlotRecvName.end());
//
//	if (m_hMailSlotRecv == nullptr)
//	{
//		m_hMailSlotRecv = CreateMailslotA(m_szMailSlotRecvName.c_str(), 0, MAILSLOT_WAIT_FOREVER, NULL);
//
//		if (m_hMailSlotRecv == INVALID_HANDLE_VALUE)
//		{
//#ifdef DEBUG
//			printf("CreateMailslot recv failed with %d\n", GetLastError());
//#endif
//			return;
//		}
//	}
//
//	LPVOID pMailSlotNameAddress = VirtualAllocEx(hProcess, nullptr, vecMailSlotName.size(), MEM_COMMIT, PAGE_EXECUTE_READWRITE);
//
//	if (pMailSlotNameAddress == 0)
//	{
//		return;
//	}
//
//	WriteBytes((DWORD)pMailSlotNameAddress, vecMailSlotName);
//
//	if (m_RecvHookAddress == 0)
//	{
//		BYTE byHookPatch[] =
//		{
//			0x55,
//			0x8B, 0xEC,
//			0x83, 0xC4, 0xF4,
//			0x33, 0xC0,
//			0x89, 0x45, 0xFC,
//			0x33, 0xD2,
//			0x89, 0x55, 0xF8,
//			0x6A, 0x00,
//			0x68, 0x80, 0x00, 0x00, 0x00,
//			0x6A, 0x03,
//			0x6A, 0x00,
//			0x6A, 0x01,
//			0x68, 0x00, 0x00, 0x00, 0x40,
//			0x68, 0x00, 0x00, 0x00, 0x00,
//			0xE8, 0x00, 0x00, 0x00, 0x00,
//			0x89, 0x45, 0xF8,
//			0x6A, 0x00,
//			0x8D, 0x4D, 0xFC,
//			0x51,
//			0xFF, 0x75, 0x0C,
//			0xFF, 0x75, 0x08,
//			0xFF, 0x75, 0xF8,
//			0xE8, 0x00, 0x00, 0x00, 0x00,
//			0x89, 0x45, 0xF4,
//			0xFF, 0x75, 0xF8,
//			0xE8, 0x00, 0x00, 0x00, 0x00,
//			0x8B, 0xE5,
//			0x5D,
//			0xC3
//		};
//
//		m_RecvHookAddress = VirtualAllocEx(hProcess, nullptr, sizeof(byHookPatch), MEM_COMMIT, PAGE_EXECUTE_READWRITE);
//
//		if (m_RecvHookAddress == 0)
//		{
//			return;
//		}
//
//		CopyBytes(byHookPatch + 35, pMailSlotNameAddress);
//
//		DWORD iCreateFileDifference = Memory::GetDifference((DWORD)m_RecvHookAddress + 39, (DWORD)pCreateFilePtr);
//		CopyBytes(byHookPatch + 40, iCreateFileDifference);
//
//		DWORD iWriteFileDifference = Memory::GetDifference((DWORD)m_RecvHookAddress + 62, (DWORD)pWriteFilePtr);
//		CopyBytes(byHookPatch + 63, iWriteFileDifference);
//
//		DWORD iCloseHandlePtrDifference = Memory::GetDifference((DWORD)m_RecvHookAddress + 73, (DWORD)pCloseHandlePtr);
//		CopyBytes(byHookPatch + 74, iCloseHandlePtrDifference);
//
//		std::vector<BYTE> vecHookPatch(byHookPatch, byHookPatch + sizeof(byHookPatch));
//		WriteBytes((DWORD)m_RecvHookAddress, vecHookPatch);
//	}
//
//	DWORD iRecvAddress = Read4Byte(Read4Byte(iAddress)) + 0x8;
//
//	BYTE byPatch[] =
//	{
//		0x55,									//push ebp
//		0x8B, 0xEC,								//mov ebp,esp
//		0x83, 0xC4, 0xF8,						//add esp,-08
//		0x53,									//push ebx
//		0x8B, 0x45, 0x08,						//mov eax,[ebp+08]
//		0x83, 0xC0, 0x04,						//add eax,04
//		0x8B, 0x10,								//mov edx,[eax]
//		0x89, 0x55, 0xFC,						//mov [ebp-04],edx
//		0x8B, 0x4D, 0x08,						//mov ecx,[ebp+08]
//		0x83, 0xC1, 0x08,						//add ecx,08
//		0x8B, 0x01,								//mov eax,[ecx]
//		0x89, 0x45, 0xF8,						//mov [ebp-08],eax
//		0xFF, 0x75, 0xFC,						//push [ebp-04]
//		0xFF, 0x75, 0xF8,						//push [ebp-08]
//		0xB8, 0x00, 0x00, 0x00, 0x00,			//mov eax,00000000 <-- ClientHook::RecvProcess()
//		0xFF, 0xD0,								//call eax
//		0x83, 0xC4, 0x08,						//add esp,08
//		0x8B, 0x0D, 0x00, 0x00, 0x00, 0x00,		//mov ecx,[00000000] <-- KO_PTR_DLG
//		0xFF, 0x75, 0x0C,						//push [ebp+0C]
//		0xFF, 0x75, 0x08,						//push [ebp+08]
//		0xB8, 0x00, 0x00, 0x00, 0x00,			//mov eax,00000000 <-- GetRecvCallAddress()
//		0xFF, 0xD0,								//call eax
//		0x5B,									//pop ebx
//		0x59,									//pop ecx
//		0x59,									//pop ecx
//		0x5D,									//pop ebp
//		0xC2, 0x08, 0x00						//ret 0008
//	};
//
//	DWORD iRecvProcessFunction = (DWORD)(LPVOID*)m_RecvHookAddress;
//	CopyBytes(byPatch + 36, iRecvProcessFunction);
//
//	DWORD iDlgAddress = iAddress;
//	CopyBytes(byPatch + 47, iDlgAddress);
//
//	DWORD iRecvCallAddress = Read4Byte(iRecvAddress);
//	CopyBytes(byPatch + 58, iRecvCallAddress);
//
//	std::vector<BYTE> vecPatch(byPatch, byPatch + sizeof(byPatch));
//
//	LPVOID pPatchAddress = VirtualAllocEx(hProcess, nullptr, sizeof(byPatch), MEM_COMMIT, PAGE_EXECUTE_READWRITE);
//
//	if (pPatchAddress == nullptr)
//	{
//		return;
//	}
//
//	WriteBytes((DWORD)pPatchAddress, vecPatch);
//
//	DWORD dwOldProtection;
//	VirtualProtectEx(hProcess, (LPVOID)iRecvAddress, 1, PAGE_EXECUTE_READWRITE, &dwOldProtection);
//	Write4Byte(Read4Byte(Read4Byte(iAddress)) + 0x8, (DWORD)pPatchAddress);
//	VirtualProtectEx(hProcess, (LPVOID)iRecvAddress, 1, dwOldProtection, &dwOldProtection);
//
//#ifdef DEBUG
//	printf("PatchRecvAddress: 0x%x patched\n", iRecvAddress);
//#endif
//}

void ClientHandler::PatchRecvAddress(DWORD iAddress)
{
	HANDLE hProcess = m_Bot->GetInjectedProcessHandle();

	DWORD iAddressReady = 0;
	while (iAddressReady == 0)
	{
		ReadProcessMemory(hProcess, (LPVOID)iAddress, &iAddressReady, 4, 0);
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

	m_szMailSlotRecvName = skCryptDec("\\\\.\\mailslot\\KOF1\\") + std::to_string(m_Bot->GetInjectedProcessId());
	std::vector<BYTE> vecMailSlotName(m_szMailSlotRecvName.begin(), m_szMailSlotRecvName.end());

	if (m_hMailSlotRecv == nullptr)
	{
		m_hMailSlotRecv = CreateMailslotA(m_szMailSlotRecvName.c_str(), 0, MAILSLOT_WAIT_FOREVER, NULL);

		if (m_hMailSlotRecv == INVALID_HANDLE_VALUE)
		{
#ifdef DEBUG
			printf("CreateMailslot recv failed with %d\n", GetLastError());
#endif
			return;
		}
	}

	LPVOID pMailSlotNameAddress = VirtualAllocEx(hProcess, nullptr, vecMailSlotName.size(), MEM_COMMIT, PAGE_EXECUTE_READWRITE);

	if (pMailSlotNameAddress == 0)
	{
		return;
	}

	WriteBytes((DWORD)pMailSlotNameAddress, vecMailSlotName);

	m_RecvHookAddress = VirtualAllocEx(hProcess, nullptr, 2000, MEM_COMMIT, PAGE_EXECUTE_READWRITE);

	if (m_RecvHookAddress == 0)
	{
		return;
	}

	DWORD iRecvAddress = Read4Byte(Read4Byte(iAddress)) + 0x8;

	BYTE byPatch[] =
	{
		0x60,										//pushad 
		0x8B, 0x44, 0x24, 0x24,						//mov eax,[esp+24]
		0x8B, 0x40, 0x08,							//mov eax,[eax+08]
		0x89, 0x05, 0x00, 0x00, 0x00, 0x00,			//mov [iRecvProcessLength],eax
		0x8B, 0x44, 0x24, 0x24,						//mov eax,[esp+24]
		0x8B, 0x40, 0x04,							//mov eax,[eax+04]
		0x89, 0x05, 0x00, 0x00, 0x00, 0x00,			//mov [iRecvProcessLength],eax
		0x3D, 0x00, 0x40, 0x00, 0x00,				//cmp eax,00004000
		0x7D, 0x3D,									//jnl 2624005F

		0x6A, 0x00,									//push 00
		0x68, 0x80, 0x00, 0x00, 0x00,				//push 00000080
		0x6A, 0x03,									//push 03
		0x6A, 0x00,									//push 00
		0x6A, 0x01,									//push 01
		0x68, 0x00, 0x00, 0x00, 0x40,				//push 40000000
		0x68, 0x00, 0x00, 0x00, 0x00,				//push pMailSlotNameAddress	
			
		0xE8, 0x00, 0x00, 0x00, 0x00,				//call KERNEL32.CreateFileW
		0x83, 0xF8, 0xFF,							//cmp eax,-01
			
		0x74, 0x1C,									//je 2624005F
		0x6A, 0x00,									//push 00
		0x54,										//push esp
		0x90,										//nop
			
		0xFF, 0x35, 0x00, 0x00, 0x00, 0x00,			//push [iRecvProcessLength]
		0xFF, 0x35, 0x00, 0x00, 0x00, 0x00,			//push [iRecvProcessPacket]

		0x50,										//push eax
		0xE8, 0x00, 0x00, 0x00, 0x00,				//call KERNEL32.WriteFile
		0x50,										//push eax
		0xE8, 0x00, 0x00, 0x00, 0x00,				//call KERNEL32.CloseHandle
		0x61,										//popad 

		0xE9, 0x00, 0x00, 0x00, 0x00,				//jmp KnightOnLine.exe+3009F0
	};

	DWORD iRecvProcessPacket = (DWORD)(LPVOID*)((DWORD)m_RecvHookAddress+0x104);
	CopyBytes(byPatch + 10, iRecvProcessPacket);

	DWORD iRecvProcessLength = (DWORD)(LPVOID*)((DWORD)m_RecvHookAddress+0x100);
	CopyBytes(byPatch + 23, iRecvProcessLength);

	CopyBytes(byPatch + 53, pMailSlotNameAddress);

	DWORD iCreateFileDifference = Memory::GetDifference((DWORD)m_RecvHookAddress + 57, (DWORD)pCreateFilePtr);
	CopyBytes(byPatch + 58, iCreateFileDifference);

	CopyBytes(byPatch + 73, iRecvProcessLength);
	CopyBytes(byPatch + 79, iRecvProcessPacket);

	DWORD iWriteFileDifference = Memory::GetDifference((DWORD)m_RecvHookAddress + 84, (DWORD)pWriteFilePtr);
	CopyBytes(byPatch + 85, iWriteFileDifference);

	DWORD iCloseHandlePtrDifference = Memory::GetDifference((DWORD)m_RecvHookAddress + 90, (DWORD)pCloseHandlePtr);
	CopyBytes(byPatch + 91, iCloseHandlePtrDifference);
	
	DWORD iRecvCallAddress = Read4Byte(iRecvAddress);
	DWORD iCallDifference = Memory::GetDifference((DWORD)m_RecvHookAddress + 96, (DWORD)iRecvCallAddress);

	CopyBytes(byPatch + 97, iCallDifference);

	std::vector<BYTE> vecPatch(byPatch, byPatch + sizeof(byPatch));

	WriteBytes((DWORD)m_RecvHookAddress, vecPatch);

	DWORD dwOldProtection;
	VirtualProtectEx(hProcess, (LPVOID)iRecvAddress, 1, PAGE_EXECUTE_READWRITE, &dwOldProtection);
	Write4Byte(Read4Byte(Read4Byte(iAddress)) + 0x8, (DWORD)m_RecvHookAddress);
	VirtualProtectEx(hProcess, (LPVOID)iRecvAddress, 1, dwOldProtection, &dwOldProtection);

#ifdef DEBUG
	printf("PatchRecvAddress: 0x%x patched\n", iRecvAddress);
#endif
}

void ClientHandler::PatchSendAddress()
{
	HANDLE hProcess = m_Bot->GetInjectedProcessHandle();

	DWORD iAddressReady = 0;

	while (iAddressReady == 0)
	{
		ReadProcessMemory(hProcess, (LPVOID)GetAddress(skCryptDec("KO_SND_FNC")), &iAddressReady, 4, 0);
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

	m_szMailSlotSendName = skCryptDec("\\\\.\\mailslot\\KOF2\\") + std::to_string(m_Bot->GetInjectedProcessId());
	std::vector<BYTE> vecMailSlotName(m_szMailSlotSendName.begin(), m_szMailSlotSendName.end());

	if (m_hMailSlotSend == nullptr)
	{
		m_hMailSlotSend = CreateMailslotA(m_szMailSlotSendName.c_str(), 0, MAILSLOT_WAIT_FOREVER, NULL);

		if (m_hMailSlotSend == INVALID_HANDLE_VALUE)
		{
#ifdef DEBUG
			printf("CreateMailslot send failed with %d\n", GetLastError());
#endif
			return;
		}
	}

	LPVOID pMailSlotNameAddress = VirtualAllocEx(hProcess, nullptr, vecMailSlotName.size(), MEM_COMMIT, PAGE_EXECUTE_READWRITE);

	if (pMailSlotNameAddress == 0)
	{
		return;
	}

	WriteBytes((DWORD)pMailSlotNameAddress, vecMailSlotName);

	if (m_SendHookAddress == 0)
	{
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

		m_SendHookAddress = VirtualAllocEx(hProcess, nullptr, sizeof(byHookPatch), MEM_COMMIT, PAGE_EXECUTE_READWRITE);

		if (m_SendHookAddress == 0)
		{
			return;
		}

		CopyBytes(byHookPatch + 35, pMailSlotNameAddress);

		DWORD iCreateFileDifference = Memory::GetDifference((DWORD)m_SendHookAddress + 39, (DWORD)pCreateFilePtr);
		CopyBytes(byHookPatch + 40, iCreateFileDifference);

		DWORD iWriteFileDifference = Memory::GetDifference((DWORD)m_SendHookAddress + 62, (DWORD)pWriteFilePtr);
		CopyBytes(byHookPatch + 63, iWriteFileDifference);

		DWORD iCloseHandlePtrDifference = Memory::GetDifference((DWORD)m_SendHookAddress + 73, (DWORD)pCloseHandlePtr);
		CopyBytes(byHookPatch + 74, iCloseHandlePtrDifference);

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

	DWORD iKoPtrSndFnc = GetAddress(skCryptDec("KO_SND_FNC"));
	CopyBytes(byPatch1 + 23, iKoPtrSndFnc);

	std::vector<BYTE> vecPatch1(byPatch1, byPatch1 + sizeof(byPatch1));

	LPVOID pPatchAddress = VirtualAllocEx(hProcess, nullptr, vecPatch1.size(), MEM_COMMIT, PAGE_EXECUTE_READWRITE);

	if (pPatchAddress == 0)
	{
		return;
	}

	WriteBytes((DWORD)pPatchAddress, vecPatch1);

	BYTE byPatch2[] =
	{
		0xE9, 0x00, 0x00, 0x00, 0x00
	};

	DWORD iCallDifference = Memory::GetDifference(GetAddress(skCryptDec("KO_SND_FNC")), (DWORD)pPatchAddress);
	CopyBytes(byPatch2 + 1, iCallDifference);

	std::vector<BYTE> vecPatch2(byPatch2, byPatch2 + sizeof(byPatch2));

	DWORD iOldProtection;
	VirtualProtectEx(hProcess, (LPVOID)GetAddress(skCryptDec("KO_SND_FNC")), 1, PAGE_EXECUTE_READWRITE, &iOldProtection);
	WriteBytes(GetAddress(skCryptDec("KO_SND_FNC")), vecPatch2);
	VirtualProtectEx(hProcess, (LPVOID)GetAddress(skCryptDec("KO_SND_FNC")), 1, iOldProtection, &iOldProtection);


#ifdef DEBUG
	printf("PatchSendAddress: 0x%x patched\n", GetAddress("KO_SND_FNC"));
#endif
}

void ClientHandler::MailSlotRecvProcess()
{
	try
	{
		DWORD iCurrentMesageSize, iMesageLeft, iMessageReadSize;

		while (true)
		{
			BOOL bResult = GetMailslotInfo(m_hMailSlotRecv, NULL, &iCurrentMesageSize, &iMesageLeft, NULL);

			if (!bResult)
				return;

			if (iCurrentMesageSize == MAILSLOT_NO_MESSAGE)
				break;

			std::vector<uint8_t> vecMessageBuffer;

			vecMessageBuffer.resize(iCurrentMesageSize);

			OVERLAPPED ov{};
			ov.Offset = 0;
			ov.OffsetHigh = 0;
			ov.hEvent = NULL;

			bResult = ReadFile(m_hMailSlotRecv, &vecMessageBuffer[0], iCurrentMesageSize, &iMessageReadSize, &ov);

			if (!bResult)
				return;

			vecMessageBuffer.resize(iMessageReadSize);

			onClientRecvProcess(vecMessageBuffer.data(), vecMessageBuffer.size());
		}
	}
	catch (const std::exception& e)
	{
#ifdef DEBUG
		printf("MailSlotRecvProcess: Exception: %s\n", e.what());
#else
		UNREFERENCED_PARAMETER(e);
#endif
	}
}

void ClientHandler::MailSlotSendProcess()
{
	try
	{
		DWORD iCurrentMesageSize, iMesageLeft, iMessageReadSize;

		while (true)
		{
			BOOL bResult = GetMailslotInfo(m_hMailSlotSend, NULL, &iCurrentMesageSize, &iMesageLeft, NULL);

			if (!bResult)
				return;

			if (iCurrentMesageSize == MAILSLOT_NO_MESSAGE)
				break;

			std::vector<uint8_t> vecMessageBuffer;

			vecMessageBuffer.resize(iCurrentMesageSize);

			OVERLAPPED ov{};
			ov.Offset = 0;
			ov.OffsetHigh = 0;
			ov.hEvent = NULL;

			bResult = ReadFile(m_hMailSlotSend, &vecMessageBuffer[0], iCurrentMesageSize, &iMessageReadSize, &ov);

			if (!bResult)
				return;

			vecMessageBuffer.resize(iMessageReadSize);

			onClientSendProcess(vecMessageBuffer.data(), vecMessageBuffer.size());
		}
	}
	catch (const std::exception& e)
	{
#ifdef DEBUG
		printf("MailSlotSendProcess: Exception: %s\n", e.what());
#else
		UNREFERENCED_PARAMETER(e);
#endif
	}
}

void ClientHandler::RecvProcess(BYTE* byBuffer, DWORD iLength)
{
	Packet pkt = Packet(byBuffer[0], (size_t)iLength);
	pkt.append(&byBuffer[1], iLength - 1);

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
					if (m_Bot->GetPlatformType() == PlatformType::CNKO)
					{
						new std::thread([this]()
						{
#ifdef DEBUG
							printf("RecvProcess::LS_LOGIN_REQ: Reconnecting login server\n");
#endif
							std::this_thread::sleep_for(std::chrono::milliseconds(500));

							WriteLoginInformation(m_szAccountId, m_szPassword);
							std::this_thread::sleep_for(std::chrono::milliseconds(100));
							ConnectLoginServer(true);
						});
					}
				}
				break;

				case AUTH_SUCCESS:
				{
#ifdef DEBUG
					printf("RecvProcess::LS_LOGIN_REQ: Login Success\n");
#endif

					new std::thread([&]() { m_Bot->InitializeStaticData(); });
				}
				break;

				case AUTH_OTP:
				{
#ifdef DEBUG
					printf("RecvProcess::LS_LOGIN_REQ: OTP Validation\n");
#endif


					if (m_Bot->m_iSelectedAccount != -1
						&& m_Bot->m_jAccountList.size() > 0
						&& m_Bot->m_jSelectedAccount.size() > 0)
					{
						std::string szOtpAttribute = skCryptDec("otp");
						std::string szOtpPasswordAttribute = skCryptDec("otpPassword");
						std::string szOtpHwidAttribute = skCryptDec("otpHwid");

						bool bAutoOTPEnable = m_Bot->m_jSelectedAccount[szOtpAttribute.c_str()].get<bool>();
					
						std::string szOtpPassword = m_Bot->m_jSelectedAccount[szOtpPasswordAttribute.c_str()].get<std::string>();
						std::string szOtpHwid = m_Bot->m_jSelectedAccount[szOtpHwidAttribute.c_str()].get<std::string>();

						if (bAutoOTPEnable 
							&& szOtpPassword.size() > 0)
						{
							std::string szHashedPassword = Password::PasswordHash(m_szPassword);
							std::string szCode = to_string(m_Bot->ReadAnyOTPCode(szOtpPassword, szOtpHwid));

							if (szCode != "000000")
							{
								SendOTPPacket(m_szAccountId, szHashedPassword, szCode);
								m_iOTPRetryCount++;
							}
							else
							{
#ifdef DEBUG
								printf("RecvProcess::LS_OTP: Otp code generation failed! Response: 000000\n");
#endif
							}
						}
					}
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

		case LS_OTP:
		{
			int8_t iResult;
			int8_t iUnknown;

			pkt >> iResult >> iUnknown;

			switch (iResult)
			{
				case 1:
				{
#ifdef DEBUG
					printf("RecvProcess::LS_OTP: OTP Success\n");
#endif
				}
				break;

				case 2:
				{
#ifdef DEBUG
					printf("RecvProcess::LS_OTP: OTP Failed, Retry Count: %d\n", m_iOTPRetryCount);
#endif

					if (m_iOTPRetryCount <= 3)
					{
						printf("RecvProcess::LS_OTP: Retrying OTP\n");


						if (m_Bot->m_iSelectedAccount != -1
							&& m_Bot->m_jAccountList.size() > 0
							&& m_Bot->m_jSelectedAccount.size() > 0)
						{
							new std::thread([this]()
							{
								std::this_thread::sleep_for(std::chrono::milliseconds(3000));

								std::string szOtpAttribute = skCryptDec("otp");
								std::string szOtpPasswordAttribute = skCryptDec("otpPassword");
								std::string szOtpHwidAttribute = skCryptDec("otpHwid");

								bool bAutoOTPEnable = m_Bot->m_jSelectedAccount[szOtpAttribute.c_str()].get<bool>();

								std::string szOtpPassword = m_Bot->m_jSelectedAccount[szOtpPasswordAttribute.c_str()].get<std::string>();
								std::string szOtpHwid = m_Bot->m_jSelectedAccount[szOtpHwidAttribute.c_str()].get<std::string>();

								if (bAutoOTPEnable
									&& szOtpPassword.size() > 0)
								{
									std::string szHashedPassword = Password::PasswordHash(m_szPassword);
									std::string szCode = to_string(m_Bot->ReadAnyOTPCode(szOtpPassword, szOtpHwid));

									if (szCode != "000000")
									{
										SendOTPPacket(m_szAccountId, szHashedPassword, szCode);
										m_iOTPRetryCount++;
									}
									else
									{
#ifdef DEBUG
										printf("RecvProcess::LS_OTP: Otp code generation failed! Response: 000000\n");
#endif
									}
								}
							});
						}
					}
				}
				break;

				case 3:
				{
#ifdef DEBUG
					printf("RecvProcess::LS_OTP: OTP code cannot be reused\n");
#endif

					if (m_Bot->m_iSelectedAccount != -1
						&& m_Bot->m_jAccountList.size() > 0
						&& m_Bot->m_jSelectedAccount.size() > 0)
					{
						new std::thread([this]()
						{
							std::this_thread::sleep_for(std::chrono::milliseconds(60000));

							std::string szOtpAttribute = skCryptDec("otp");
							std::string szOtpPasswordAttribute = skCryptDec("otpPassword");
							std::string szOtpHwidAttribute = skCryptDec("otpHwid");

							bool bAutoOTPEnable = m_Bot->m_jSelectedAccount[szOtpAttribute.c_str()].get<bool>();

							std::string szOtpPassword = m_Bot->m_jSelectedAccount[szOtpPasswordAttribute.c_str()].get<std::string>();
							std::string szOtpHwid = m_Bot->m_jSelectedAccount[szOtpHwidAttribute.c_str()].get<std::string>();

							if (bAutoOTPEnable
								&& szOtpPassword.size() > 0)
							{
								std::string szHashedPassword = Password::PasswordHash(m_szPassword);
								std::string szCode = to_string(m_Bot->ReadAnyOTPCode(szOtpPassword, szOtpHwid));

								if (szCode != "000000")
								{
									SendOTPPacket(m_szAccountId, szHashedPassword, szCode);
								}
								else
								{
#ifdef DEBUG
									printf("RecvProcess::LS_OTP: Otp code generation failed! Response: 000000\n");
#endif
								}
							}
						});
					}
				}
				break;

				case 10:
				{
#ifdef DEBUG
					printf("OTP Login failed with unknown issue. Please Contact customer service\n");
#endif
				}
				break;

				default:
				{
#ifdef DEBUG
					printf("RecvProcess::LS_OTP: %d not implemented!\n", iResult);
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

				if ((m_Bot->m_iSelectedAccount != -1
					&& m_Bot->m_jAccountList.size() > 0
					&& m_Bot->m_jSelectedAccount.size() > 0))
				{
					new std::thread([this]()
					{
#ifdef DEBUG
						printf("RecvProcess::LS_SERVERLIST: Connecting to server\n");
#endif
						std::string szServerIndexAttribute = skCryptDec("serverIndex");
						std::string szChannelIndexAttribute = skCryptDec("channelIndex");

						if (m_Bot->GetPlatformType() != PlatformType::CNKO)
						{
							LoadServerList();
							std::this_thread::sleep_for(std::chrono::milliseconds(100));
						}

						SelectServer(m_Bot->m_jSelectedAccount[szServerIndexAttribute.c_str()].get<int32_t>() - 1);

						if (m_Bot->GetPlatformType() != PlatformType::CNKO)
						{
							std::this_thread::sleep_for(std::chrono::milliseconds(100));
							ShowChannel();
							std::this_thread::sleep_for(std::chrono::milliseconds(100));
							SelectChannel(m_Bot->m_jSelectedAccount[szChannelIndexAttribute.c_str()].get<int32_t>() - 1);
						}

						std::this_thread::sleep_for(std::chrono::milliseconds(100));

						ConnectServer();
					});
				}
			}
		}
		break;

		case WIZ_ALLCHAR_INFO_REQ:
		{
			int8_t byResult;

			pkt >> byResult;

			bool bLoaded =
				pkt[pkt.size() - 3] == 0x0C &&
				pkt[pkt.size() - 2] == 0x02 &&
				pkt[pkt.size() - 1] == 0x00;

			if (bLoaded)
			{
#ifdef DEBUG
				printf("RecvProcess::WIZ_ALLCHAR_INFO_REQ: Character list loaded\n");
#endif

				if ((m_Bot->m_iSelectedAccount != -1
					&& m_Bot->m_jAccountList.size() > 0
					&& m_Bot->m_jSelectedAccount.size() > 0))
				{
					new std::thread([this]()
					{
#ifdef DEBUG
						printf("RecvProcess::WIZ_ALLCHAR_INFO_REQ: Selecting character %d\n", 1);
#endif

						SelectCharacterSkip();

						std::string szCharacterIndexAttribute = skCryptDec("characterIndex");

						int32_t iCharacterIndex = m_Bot->m_jSelectedAccount[szCharacterIndexAttribute.c_str()].get<int32_t>() - 1;

						if (iCharacterIndex > 0)
						{
							for (int32_t i = 0; i < iCharacterIndex; i++)
							{
								std::this_thread::sleep_for(std::chrono::milliseconds(100));
								SelectCharacterLeft();
								std::this_thread::sleep_for(std::chrono::milliseconds(100));
								SelectCharacterSkip();

							}
						}

						std::this_thread::sleep_for(std::chrono::milliseconds(100));

						SelectCharacter();
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

			pkt.readString(m_PlayerMySelf.szName);

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

			pkt.readString(m_PlayerMySelf.szKnights);

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

			LoadSkillData();

			m_Bot->SendLoadUserConfiguration(GetServerId(), m_PlayerMySelf.szName);

#ifdef DEBUG
			printf("RecvProcess::WIZ_MYINFO: %s loaded\n", m_PlayerMySelf.szName.c_str());
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

				if (iLevel <= 10)
				{
					LoadSkillData();
				}

#ifdef DEBUG
				printf("RecvProcess::WIZ_LEVEL_CHANGE: %s %d\n", m_PlayerMySelf.szName.c_str(), iLevel);
#endif
			}
			else
			{
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

		case WIZ_SKILLPT_CHANGE:
		{
			int iType = pkt.read<uint8_t>();
			int iValue = pkt.read<uint8_t>();

			m_PlayerMySelf.iSkillInfo[iType] = (uint8_t)iValue;
			m_PlayerMySelf.iSkillInfo[0]++;

#ifdef DEBUG
			printf("RecvProcess::WIZ_SKILLPT_CHANGE: %d,%d,%d\n", iType, iValue, m_PlayerMySelf.iSkillInfo[0]);
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
							GetUserConfiguration()->SetInt(skCryptDec("Automation"), skCryptDec("AttackSkillList"), std::vector<int>());
							GetUserConfiguration()->SetInt(skCryptDec("Automation"), skCryptDec("CharacterSkillList"), std::vector<int>());
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
							GetUserConfiguration()->SetInt(skCryptDec("Automation"), skCryptDec("AttackSkillList"), std::vector<int>());
							GetUserConfiguration()->SetInt(skCryptDec("Automation"), skCryptDec("CharacterSkillList"), std::vector<int>());
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

					if (m_PlayerMySelf.iID == iID)
					{
						m_PlayerMySelf.eClass = (Class)iClass;

						GetUserConfiguration()->SetInt(skCryptDec("Automation"), skCryptDec("AttackSkillList"), std::vector<int>());
						GetUserConfiguration()->SetInt(skCryptDec("Automation"), skCryptDec("CharacterSkillList"), std::vector<int>());
						LoadSkillData();
					}					
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
				WaitCondition(GetUserConfiguration() == nullptr)
				WaitCondition(GetUserConfiguration()->GetConfigMap()->size() == 0)
				WaitCondition(m_Bot->IsTableLoaded() == false);

				LoadSkillData();

				bool bWallHack = GetUserConfiguration()->GetBool(skCryptDec("Feature"), skCryptDec("WallHack"), false);
				SetAuthority(bWallHack ? 0 : 1);

				bool bLegalWallHack = GetUserConfiguration()->GetBool(skCryptDec("Feature"), skCryptDec("LegalWallHack"), false);
				PatchObjectCollision(bLegalWallHack);

				bool bDeathEffect = GetUserConfiguration()->GetBool(skCryptDec("Feature"), skCryptDec("DeathEffect"), false);
				PatchDeathEffect(bDeathEffect);

				bool bDisableCasting = GetUserConfiguration()->GetBool(skCryptDec("Feature"), skCryptDec("DisableCasting"), false);
				UpdateSkillSuccessRate(bDisableCasting);

				GetUserConfiguration()->SetInt(skCryptDec("Automation"), skCryptDec("Attack"), 0);
				GetUserConfiguration()->SetInt(skCryptDec("Automation"), skCryptDec("Character"), 0);

				bool bSaveCPUEnable = GetUserConfiguration()->GetBool(skCryptDec("Feature"), skCryptDec("SaveCPU"), false);

				if (bSaveCPUEnable)
				{
					int iSaveCPUValue = GetUserConfiguration()->GetInt(skCryptDec("Feature"), skCryptDec("SaveCPUValue"), 1);
					SetSaveCPUSleepTime(iSaveCPUValue);
				}

				bool bSpeedHack = GetUserConfiguration()->GetBool(skCryptDec("Feature"), skCryptDec("SpeedHack"), false);

				if (bSpeedHack)
				{
					SetCharacterSpeed(1.5);
					PatchSpeedHack(true);
				}

				StartHandler();
			});
		}
		break;

		case WIZ_REQ_NPCIN:
		{
			int16_t iNpcCount = pkt.read<int16_t>();

			if (iNpcCount > 0)
			{
				for (int16_t i = 0; i < iNpcCount; i++)
				{
					auto pNpc = InitializeNpc(pkt);

					int32_t iNpcID = pNpc.iID;
					
					auto it = std::find_if(m_vecNpc.begin(), m_vecNpc.end(),
						[&](const TNpc& a) { return a.iID == iNpcID; });

					if (it == m_vecNpc.end())
						m_vecNpc.push_back(pNpc);
					else
						*it = pNpc;
				}
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
					{
						m_vecNpc.push_back(pNpc);
					}
					else
						*it = pNpc;

#ifdef DEBUG
					printf("RecvProcess::WIZ_NPC_INOUT: INOUT_IN - %d,%d\n", iType, pNpc.iID);
#endif
				}
				break;

				case InOut::INOUT_OUT:
				{
					int32_t iNpcID = pkt.read<int32_t>();

					m_vecNpc.erase(
						std::remove_if(m_vecNpc.begin(), m_vecNpc.end(),
							[&](const TNpc& a) { return a.iID == iNpcID; }),
						m_vecNpc.end());

#ifdef DEBUG
					printf("RecvProcess::WIZ_NPC_INOUT: INOUT_OUT - %d,%d\n", iType, iNpcID);
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

			if (iNpcCount > 0)
			{
				std::vector<int32_t> vecRegionNpcList;

				for (int16_t i = 0; i < iNpcCount; i++)
				{
					int32_t iNpcID = pkt.read<int32_t>();
					vecRegionNpcList.push_back(iNpcID);
				}

				for (const auto& pNpc : m_vecNpc)
				{
					if (std::find(vecRegionNpcList.begin(), vecRegionNpcList.end(), pNpc.iID) == vecRegionNpcList.end())
					{
						m_vecNpc.erase(
							std::remove_if(m_vecNpc.begin(), m_vecNpc.end(),
								[&](const TNpc& a) { return a.iID == pNpc.iID; }),
							m_vecNpc.end());
					}
				}

#ifdef DEBUG
				printf("RecvProcess::WIZ_NPC_REGION: Npc region change completed! Region list size %d, current npc list %d\n", vecRegionNpcList.size(), m_vecNpc.size());
#endif
			}
		}
		break;

		case WIZ_REQ_USERIN:
		{
			int16_t iUserCount = pkt.read<int16_t>();

			if (iUserCount > 0)
			{
				for (int16_t i = 0; i < iUserCount; i++)
				{
					uint8_t iUnknown0 = pkt.read<uint8_t>();

					auto pPlayer = InitializePlayer(pkt);

					auto it = std::find_if(m_vecPlayer.begin(), m_vecPlayer.end(),
						[&](const TPlayer& a) { return a.iID == pPlayer.iID; });

					if (it == m_vecPlayer.end())
						m_vecPlayer.push_back(pPlayer);
					else
						*it = pPlayer;

					/*bool bSendTownIfThereIsGMNearby = GetUserConfiguration()->GetBool(skCryptDec("Settings"), skCryptDec("bSendTownIfThereIsGMNearby"), false);
					bool bAttackStatus = GetUserConfiguration()->GetBool(skCryptDec("Automation"), skCryptDec("Attack"), false);
					bool bCharacterStatus = GetUserConfiguration()->GetBool(skCryptDec("Automation"), skCryptDec("Character"), false);

					if ((bAttackStatus || bCharacterStatus)
						&& bSendTownIfThereIsGMNearby && pPlayer.iAuthority == 0)
					{
						SendTownPacket();
#ifdef DEBUG
						printf("RecvProcess::WIZ_REQ_USERIN: !! GM !!, Gamemaster: %s\n", pPlayer.szName.c_str());
#endif

					}*/
				}

			}

#ifdef DEBUG
			printf("RecvProcess::WIZ_REQ_USERIN: Size %d\n", iUserCount);
#endif
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

				auto it = std::find_if(m_vecPlayer.begin(), m_vecPlayer.end(),
					[&](const TPlayer& a) { return a.iID == pPlayer.iID; });

				if (it == m_vecPlayer.end())
					m_vecPlayer.push_back(pPlayer);
				else
					*it = pPlayer;

				/*bool bSendTownIfThereIsGMNearby = GetUserConfiguration()->GetBool(skCryptDec("Settings"), skCryptDec("bSendTownIfThereIsGMNearby"), false);
				bool bAttackStatus = GetUserConfiguration()->GetBool(skCryptDec("Automation"), skCryptDec("Attack"), false);
				bool bCharacterStatus = GetUserConfiguration()->GetBool(skCryptDec("Automation"), skCryptDec("Character"), false);

				if ((bAttackStatus || bCharacterStatus) 
					&& bSendTownIfThereIsGMNearby && pPlayer.iAuthority == 0)
				{
					SendTownPacket();
#ifdef DEBUG
					printf("RecvProcess::WIZ_USER_INOUT: !! GM !!, Gamemaster: %s\n", pPlayer.szName.c_str());
#endif
				}*/
#ifdef DEBUG
				printf("RecvProcess::WIZ_USER_INOUT: INOUT_IN | INOUT_RESPAWN | INOUT_WARP - %d,%d\n", iType, pPlayer.iID);
#endif
			}
			break;

			case InOut::INOUT_OUT:
			{
				int32_t iPlayerID = pkt.read<int32_t>();

				m_vecPlayer.erase(
					std::remove_if(m_vecPlayer.begin(), m_vecPlayer.end(),
						[&](const TPlayer& a) { return a.iID == iPlayerID; }),
					m_vecPlayer.end());

#ifdef DEBUG
				printf("RecvProcess::WIZ_USER_INOUT: INOUT_OUT - %d,%d\n", iType, iPlayerID);
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

		case WIZ_REGIONCHANGE:
		{
			uint8_t iType = pkt.read<uint8_t>();

			switch (iType)
			{
				case 0:
				{
					m_vecRegionUserList.clear();

#ifdef DEBUG
					printf("RecvProcess::WIZ_REGIONCHANGE: Region change activity start, current user count: %d\n", m_vecPlayer.size());
#endif
				}
				break;

				case 1:
				{
					int16_t iUserCount = pkt.read<int16_t>();

					for (int16_t i = 0; i < iUserCount; i++)
					{
						int32_t iUserID = pkt.read<int32_t>();

						if (iUserID == m_PlayerMySelf.iID)
							continue;

						m_vecRegionUserList.push_back(iUserID);
					}

#ifdef DEBUG
					printf("RecvProcess::WIZ_REGIONCHANGE: Region change activity user count: %d\n", m_vecRegionUserList.size());
#endif
				}
				break;

				case 2:
				{			
					std::vector<int> indicesToRemove;

					for (size_t i = 0; i < m_vecPlayer.size(); i++)
					{
						const auto& pPlayer = m_vecPlayer[i];

						if (std::find(m_vecRegionUserList.begin(), m_vecRegionUserList.end(), pPlayer.iID) == m_vecRegionUserList.end())
							indicesToRemove.push_back(i);
					}

					for (int i = indicesToRemove.size() - 1; i >= 0; i--)
					{
						m_vecPlayer.erase(m_vecPlayer.begin() + indicesToRemove[i]);
					}

#ifdef DEBUG
					printf("RecvProcess::WIZ_REGIONCHANGE: Region change activity end region user count: %d, current user list: %d\n", m_vecRegionUserList.size(), m_vecPlayer.size());
#endif

					m_vecRegionUserList.clear();
				}
				break;

				default:
				{
#ifdef DEBUG
					printf("RecvProcess::WIZ_REGIONCHANGE: Type %d not implemented! [%s]\n", iType, pkt.convertToHex().c_str());
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

				bool bStopBotIfDead = GetUserConfiguration()->GetBool(skCryptDec("Settings"), skCryptDec("StopBotIfDead"), true);

				if (bStopBotIfDead)
				{
					GetUserConfiguration()->SetInt(skCryptDec("Automation"), skCryptDec("Attack"), 0);
					GetUserConfiguration()->SetInt(skCryptDec("Automation"), skCryptDec("Character"), 0);
				}

				auto pMember = std::find_if(m_vecPartyMembers.begin(), m_vecPartyMembers.end(),
					[&](const PartyMember& a)
					{
						return a.iMemberID == iID;
					});

				if (pMember != m_vecPartyMembers.end())
				{
					pMember->fHpBuffTime = 0.0f;
					pMember->iHpBuffAttemptCount = 0;
					pMember->fACBuffTime = 0.0f;
					pMember->iACBuffAttemptCount = 0;
					pMember->fMindBuffTime = 0.0f;
					pMember->iMindBuffAttemptCount = 0;
				}
#ifdef DEBUG
				printf("RecvProcess::WIZ_DEAD: MySelf Dead\n");
#endif
			}
			else
			{
				if (iID >= 5000)
				{
					auto it = std::find_if(m_vecNpc.begin(), m_vecNpc.end(),
						[&](const TNpc& a) { return a.iID == iID; });

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
					auto it = std::find_if(m_vecPlayer.begin(), m_vecPlayer.end(),
						[&](const TPlayer& a) { return a.iID == iID; });

					if (it != m_vecPlayer.end())
					{
						it->eState = PSA_DEATH;

						auto pMember = std::find_if(m_vecPartyMembers.begin(), m_vecPartyMembers.end(),
							[&](const PartyMember& a)
							{
								return a.iMemberID == iID;
							});

						if (pMember != m_vecPartyMembers.end())
						{
							pMember->fHpBuffTime = 0.0f;
							pMember->iHpBuffAttemptCount = 0;
							pMember->fACBuffTime = 0.0f;
							pMember->iACBuffAttemptCount = 0;
							pMember->fMindBuffTime = 0.0f;
							pMember->iMindBuffAttemptCount = 0;
						}

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
						auto it = std::find_if(m_vecNpc.begin(), m_vecNpc.end(),
							[&](const TNpc& a) { return a.iID == iTargetID; });

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
						auto it = std::find_if(m_vecPlayer.begin(), m_vecPlayer.end(),
							[&](const TPlayer& a) { return a.iID == iTargetID; });

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

				//if (/*m_PlayerMySelf.iHPMax > 0 && */m_PlayerMySelf.iHP <= 0)
					//m_PlayerMySelf.eState = PSA_DEATH;

#ifdef DEBUG
				printf("RecvProcess::WIZ_TARGET_HP: %s, %d / %d\n",
					m_PlayerMySelf.szName.c_str(), m_PlayerMySelf.iHP, m_PlayerMySelf.iHPMax);
#endif
			}
			else
			{
				if (iID >= 5000)
				{
					auto it = std::find_if(m_vecNpc.begin(), m_vecNpc.end(),
						[&](const TNpc& a) { return a.iID == iID; });

					if (it != m_vecNpc.end())
					{
						it->iHPMax = iTargetHPMax;
						it->iHP = iTargetHPCur;

						//if (/*it->iHPMax > 0 && */it->iHP <= 0)
							//it->eState = PSA_DEATH;

#ifdef DEBUG
						printf("RecvProcess::WIZ_TARGET_HP: %d, %d / %d\n", iID, it->iHP, it->iHPMax);
#endif
					}
				}
				else
				{
					auto it = std::find_if(m_vecPlayer.begin(), m_vecPlayer.end(),
						[iID](const TPlayer& a) { return a.iID == iID; });

					if (it != m_vecPlayer.end())
					{
						it->iHPMax = iTargetHPMax;
						it->iHP = iTargetHPCur;

						//if (it->iHPMax > 0 && it->iHP <= 0)
							//it->eState = PSA_DEATH;

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
				auto it = std::find_if(m_vecPlayer.begin(), m_vecPlayer.end(),
					[&](const TPlayer& a) { return a.iID == iID; });

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

			auto it = std::find_if(m_vecNpc.begin(), m_vecNpc.end(),
				[&](const TNpc& a) { return a.iID == iID; });

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

					if (iSourceID == m_PlayerMySelf.iID)
					{
						if (m_vecPartyMembers.size() > 0 && (IsPriest() || IsRogue()))
						{
							auto pPartyMember = std::find_if(m_vecPartyMembers.begin(), m_vecPartyMembers.end(),
								[&](const PartyMember& a)
								{
									return a.iMemberID == iTargetID;
								});

							if (pPartyMember != m_vecPartyMembers.end())
							{
								std::vector<__TABLE_UPC_SKILL>* vecAvailableSkills;
								if (GetAvailableSkill(&vecAvailableSkills))
								{
									auto it = std::find_if(vecAvailableSkills->begin(), vecAvailableSkills->end(),
										[&](const TABLE_UPC_SKILL& a) { return a.iID == iSkillID; });

									if (it != vecAvailableSkills->end())
									{
										switch (it->iBaseId)
										{
											case 111654: case 112675: case 112670: case 111657:
											case 111655: case 111642: case 111633: case 111624:
											case 111615: case 111606:
											{
												pPartyMember->fHpBuffTime = Bot::TimeGet();
												pPartyMember->iHpBuffAttemptCount = 0;
											}
											break;

											case 112674: case 111660: case 111651: case 111639:
											case 111630: case 111621: case 111612: case 111603:
											{
												pPartyMember->fACBuffTime = Bot::TimeGet();
												pPartyMember->iACBuffAttemptCount = 0;
											}
											break;

											case 111645: case 111636: case 111627: case 111609:
											{
												pPartyMember->fMindBuffTime = Bot::TimeGet();
												pPartyMember->iMindBuffAttemptCount = 0;
											}
											break;

											case 107010:
											{
											}
											break;
										}
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
					memset(iData, 0, sizeof(iData));

					for (size_t i = 0; i < 6; i++)
						iData[i] = pkt.read<int32_t>();

					if (iData[3] == SKILLMAGIC_FAIL_CASTING 
						|| iData[3] == SKILLMAGIC_FAIL_NOEFFECT
						|| iData[3] == SKILLMAGIC_FAIL_ATTACKZERO
						|| iData[3] == SKILLMAGIC_FAIL_UNKNOWN)
					{
#ifdef DEBUG
						printf("RecvProcess::WIZ_MAGIC_PROCESS: %d - %d - Skill failed %d\n", iSourceID, iSkillID, iData[3]);
#endif
						if (iSourceID == m_PlayerMySelf.iID)
						{
							Client::SetSkillNextUseTime(iSkillID, 0.0f);

							if (m_vecPartyMembers.size() > 0 && (IsPriest() || IsRogue()))
							{
								auto pPartyMember = std::find_if(m_vecPartyMembers.begin(), m_vecPartyMembers.end(),
									[&](const PartyMember& a)
									{
										return a.iMemberID == iTargetID;
									});

								if (pPartyMember != m_vecPartyMembers.end())
								{
									std::vector<__TABLE_UPC_SKILL>* vecAvailableSkills;
									if (GetAvailableSkill(&vecAvailableSkills))
									{
										auto it = std::find_if(vecAvailableSkills->begin(), vecAvailableSkills->end(),
											[&](const TABLE_UPC_SKILL& a) { return a.iID == iSkillID; });

										if (it != vecAvailableSkills->end())
										{
											switch (it->iBaseId)
											{
												case 111654: case 112675: case 112670: case 111657:
												case 111655: case 111642: case 111633: case 111624:
												case 111615: case 111606: 
												{
													if (iData[3] == SKILLMAGIC_FAIL_CASTING)
													{
														pPartyMember->fHpBuffTime = 0.0f;
														pPartyMember->iHpBuffAttemptCount = 0;
													}
													else
													{
														if (pPartyMember->iHpBuffAttemptCount < 2)
														{
															pPartyMember->iHpBuffAttemptCount++;
														}
														else
														{
															pPartyMember->fHpBuffTime = Bot::TimeGet();
														}
													}
												}
												break;
												
												case 112674: case 111660: case 111651: case 111639: 
												case 111630: case 111621: case 111612: case 111603: 
												{
													if (iData[3] == SKILLMAGIC_FAIL_CASTING)
													{
														pPartyMember->fACBuffTime = 0;
														pPartyMember->iACBuffAttemptCount = 0;
													}
													else
													{
														if (pPartyMember->iACBuffAttemptCount < 2)
														{
															pPartyMember->iACBuffAttemptCount++;
														}
														else
														{
															pPartyMember->fACBuffTime = Bot::TimeGet();
														}
													}
												}
												break;
												
												case 111645: case 111636: case 111627: case 111609: 
												{
													if (iData[3] == SKILLMAGIC_FAIL_CASTING)
													{
														pPartyMember->fMindBuffTime = 0;
														pPartyMember->iMindBuffAttemptCount = 0;
													}
													else
													{
														if (pPartyMember->iMindBuffAttemptCount < 2)
														{
															pPartyMember->iMindBuffAttemptCount++;
														}
														else 
														{
															pPartyMember->fMindBuffTime = Bot::TimeGet();
														}
													}
												}
												break;
												
												case 107010:
												{
												}
												break;
											}
										}
									}
								}
							}
						}
						
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
				case N3_SP_STATE_CHANGE_SIZE:
				{
					switch (iBuff)
					{
						case ABNORMAL_NORMAL:
						{
							if (m_PlayerMySelf.iID == iID)
							{
	#ifdef DEBUG
								printf("RecvProcess::WIZ_STATE_CHANGE: %s - ABNORMAL_NORMAL\n", m_PlayerMySelf.szName.c_str());
	#endif

								m_PlayerMySelf.bBlinking = false;
							}
							else
							{
								auto it = std::find_if(m_vecPlayer.begin(), m_vecPlayer.end(),
									[&](const TPlayer& a) { return a.iID == iID; });

								if (it != m_vecPlayer.end())
								{
#ifdef DEBUG
									printf("RecvProcess::WIZ_STATE_CHANGE: %s - ABNORMAL_NORMAL\n", it->szName.c_str());
#endif

									it->bBlinking = false;
								}
							}
						}
						break;

						case ABNORMAL_BLINKING:
						{
							if (m_PlayerMySelf.iID == iID)
							{
#ifdef DEBUG
								printf("RecvProcess::WIZ_STATE_CHANGE: %s - ABNORMAL_BLINKING\n", m_PlayerMySelf.szName.c_str());
#endif

								m_PlayerMySelf.bBlinking = true;
							}
							else
							{
								auto it = std::find_if(m_vecPlayer.begin(), m_vecPlayer.end(),
									[&](const TPlayer& a) { return a.iID == iID; });

								if (it != m_vecPlayer.end())
								{
#ifdef DEBUG
									printf("RecvProcess::WIZ_STATE_CHANGE: %s - ABNORMAL_BLINKING\n", it->szName.c_str());
#endif

									it->bBlinking = true;
								}
							}
						}
						break;

						case ABNORMAL_BLINKING_END:
						{
							if (m_PlayerMySelf.iID == iID)
							{
#ifdef DEBUG
								printf("RecvProcess::WIZ_STATE_CHANGE: %s - ABNORMAL_BLINKING_END\n", m_PlayerMySelf.szName.c_str());
#endif

								m_PlayerMySelf.bBlinking = false;
							}
							else
							{		
								auto it = std::find_if(m_vecPlayer.begin(), m_vecPlayer.end(),
									[&](const TPlayer& a) { return a.iID == iID; });

								if (it != m_vecPlayer.end())
								{
#ifdef DEBUG
									printf("RecvProcess::WIZ_STATE_CHANGE: %s - ABNORMAL_BLINKING_END\n", it->szName.c_str());
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

			if (tLoot.iItemCount == 0)
			{
#ifdef DEBUG
				printf("RecvProcess::WIZ_ITEM_DROP: Box empty!\n");
#endif
				if (IsMovingToLoot())
				{
					SetMovingToLoot(false);
				}

				return;
			}

			bool bMoveToLoot = GetUserConfiguration()->GetBool(skCryptDec("AutoLoot"), skCryptDec("MoveToLoot"), false);

			if (bMoveToLoot)
			{
				auto pNpc = std::find_if(m_vecNpc.begin(), m_vecNpc.end(),
					[&](const TNpc& a)
					{
						return a.iID == tLoot.iNpcID;
					});

				if (pNpc == m_vecNpc.end())
				{
#ifdef DEBUG
					printf("RecvProcess::WIZ_ITEM_DROP: Move to loot NPC Position not exist!\n");
#endif
					return;
				}

				tLoot.fNpcX = pNpc->fX;
				tLoot.fNpcZ = pNpc->fZ;
				tLoot.fNpcY = pNpc->fY;
			}

			tLoot.fDropTime = Bot::TimeGet();

			auto pLoot = std::find_if(m_vecLootList.begin(), m_vecLootList.end(),
				[&](const TLoot a) { return a.iBundleID == tLoot.iBundleID; });

			if (pLoot == m_vecLootList.end())
				m_vecLootList.push_back(tLoot);
			else
				*pLoot = tLoot;

#ifdef DEBUG
			printf("RecvProcess::WIZ_ITEM_DROP: %d,%d,%d,%f\n",
				tLoot.iNpcID,
				tLoot.iBundleID,
				tLoot.iItemCount,
				tLoot.fDropTime);
#endif
		}
		break;

		case WIZ_BUNDLE_OPEN_REQ:
		{
			uint32_t iBundleID = pkt.read<uint32_t>();
			uint8_t iResult = pkt.read<uint8_t>();

			auto pLoot = std::find_if(m_vecLootList.begin(), m_vecLootList.end(),
				[&](const TLoot a) { return a.iBundleID == iBundleID; });

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
							{
								SendBundleItemGet(iBundleID, iItemID, (int16_t)i);
#ifdef DEBUG
								printf("RecvProcess::WIZ_BUNDLE_OPEN_REQ: %d,%d,%d,%d\n", iBundleID, iItemID, iItemCount, (int16_t)i);
#endif
							}
							else
							{
								__TABLE_ITEM* pItemData;
								__TABLE_ITEM_EXTENSION* pItemExtensionData;
								if (m_Bot->GetItemData(iItemID, pItemData) 
									&& m_Bot->GetItemExtensionData(iItemID, pItemData->byExtIndex, pItemExtensionData))
								{
									int iLootMinPrice = GetUserConfiguration()->GetInt(skCryptDec("AutoLoot"), skCryptDec("MinPrice"), 0);

									if (iLootMinPrice == 0
										|| (iLootMinPrice > 0 && pItemData->iPriceRepair >= iLootMinPrice)
										|| (pItemExtensionData->iItemType == ITEM_ATTRIBUTE_UNIQUE
											|| pItemExtensionData->iItemType == ITEM_ATTRIBUTE_RED
											|| pItemExtensionData->iItemType == ITEM_ATTRIBUTE_UPGRADE_REVERSE
											|| pItemExtensionData->iItemType == ITEM_ATTRIBUTE_UNIQUE_REVERSE
											|| pItemExtensionData->iItemType == ITEM_ATTRIBUTE_PET
											|| pItemExtensionData->iItemType == ITEM_ATTRIBUTE_EVENT
											|| pItemExtensionData->iItemType == ITEM_ATTRIBUTE_COSPRE
											|| pItemExtensionData->iItemType == ITEM_ATTRIBUTE_MINERVA))
									{
										
										SendBundleItemGet(iBundleID, iItemID, (int16_t)i);
#ifdef DEBUG
										printf("RecvProcess::WIZ_BUNDLE_OPEN_REQ: %d,%d,%d,%d,%d,%d\n", 
											iBundleID, iItemID, iItemCount, (int16_t)i, pItemData->iPriceRepair, pItemExtensionData->iItemType);
#endif
									}
								}
								else
								{
									SendBundleItemGet(iBundleID, iItemID, (int16_t)i);
#ifdef DEBUG
									printf("RecvProcess::WIZ_BUNDLE_OPEN_REQ: pItemData == nullptr || pItemExtensionData == nullptr\n");
									printf("RecvProcess::WIZ_BUNDLE_OPEN_REQ: %d,%d,%d,%d\n", iBundleID, iItemID, iItemCount, (int16_t)i);
#endif
								}
							}						
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
						[&](const TLoot& a) { return a.iBundleID == iBundleID; }),
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

					m_vecLootList.erase(
						std::remove_if(m_vecLootList.begin(), m_vecLootList.end(),
							[&](const TLoot& a) { return a.iBundleID == iBundleID; }),
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

					m_vecLootList.erase(
						std::remove_if(m_vecLootList.begin(), m_vecLootList.end(),
							[&](const TLoot& a) { return a.iBundleID == iBundleID; }),
						m_vecLootList.end());

#ifdef DEBUG
					printf("RecvProcess::WIZ_ITEM_GET: %d,%d,%d,%d,%d,%d\n", iType, iBundleID, iPos, iItemID, iItemCount, iGold);
#endif
				}
				break;

				case 0x03:
				{
					uint32_t iBundleID = pkt.read<uint32_t>();

					m_vecLootList.erase(
						std::remove_if(m_vecLootList.begin(), m_vecLootList.end(),
							[&](const TLoot& a) { return a.iBundleID == iBundleID; }),
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

		case WIZ_PARTY:
		{
			uint8_t iType = pkt.read<uint8_t>();

			switch (iType)
			{
				case PartyUpdateType::PARTY_UPDATE_CREATE:
				{
					pkt.DByte();

					uint8_t iCommandType = pkt.read<uint8_t>();

					std::string szName;
					pkt.readString(szName);
				}
				break;

				case PartyUpdateType::PARTY_UPDATE_INSERT:
				{
					pkt.DByte();

					int32_t iMemberID = pkt.read<int32_t>();

					std::string szName;
					pkt.readString(szName);
				}
				break;

				case PartyUpdateType::PARTY_UPDATE_JOINED:
				{
					pkt.DByte();

					int16_t iJoinResponse = pkt.read<int16_t>();

					switch (iJoinResponse)
					{
						case -1:
						{
							if (m_vecPartyMembers.size() == 1)
							{
								m_vecPartyMembers.clear();
							}
						}
						break;
						case 1:
						{
							PartyMember pPartyMember;
							memset(&pPartyMember, 0, sizeof(pPartyMember));

							pPartyMember.iMemberID = pkt.read<int32_t>();
							pPartyMember.iIndex = pkt.read<uint8_t>();

							pkt.readString(pPartyMember.szName);

							pPartyMember.iMaxHP = pkt.read<uint16_t>();
							pPartyMember.iHP = pkt.read<uint16_t>();
							pPartyMember.iLevel = pkt.read<uint8_t>();
							pPartyMember.iClass = pkt.read<uint16_t>();
							pPartyMember.iMaxMP = pkt.read<uint16_t>();
							pPartyMember.iMP = pkt.read<uint16_t>();

							pPartyMember.iNation = pkt.read<uint8_t>();

							uint8_t iUnknown1 = pkt.read<uint8_t>();
							int32_t iUnknown2 = pkt.read<int32_t>();

							uint8_t iUnknown3 = pkt.read<uint8_t>();
							int8_t iUnknown4 = pkt.read<int8_t>();

							if (m_vecPartyMembers.size() > 0 
								&& pPartyMember.iIndex == 100)
							{
								auto pOldLeader = std::find_if(m_vecPartyMembers.begin(), m_vecPartyMembers.end(),
									[&](const PartyMember& a)
									{
										return a.iIndex == 100;
									});

								auto pNewLeader = std::find_if(m_vecPartyMembers.begin(), m_vecPartyMembers.end(),
									[&](const PartyMember& a)
									{
										return a.iMemberID == pPartyMember.iMemberID;
									});

								if (pOldLeader != m_vecPartyMembers.end() 
									&& pNewLeader != m_vecPartyMembers.end())
								{
									pOldLeader->iIndex = pNewLeader->iIndex;
								}

								if (pNewLeader != m_vecPartyMembers.end())
								{
									pNewLeader->iIndex = pPartyMember.iIndex;
								}
							}
							else
							{
								m_vecPartyMembers.push_back(pPartyMember);
							}	
						}
						break;
					}
				}
				break;

				case PartyUpdateType::PARTY_UPDATE_LEAVE:
				{
					int32_t iMemberID = pkt.read<int32_t>();

					m_vecPartyMembers.erase(
						std::remove_if(m_vecPartyMembers.begin(), m_vecPartyMembers.end(),
							[&](const PartyMember& a) { return a.iMemberID == iMemberID; }),
						m_vecPartyMembers.end());
				}
				break;

				case PartyUpdateType::PARTY_UPDATE_DISMISSED:
				{
					m_vecPartyMembers.clear();
				}
				break;

				case PartyUpdateType::PARTY_UPDATE_HEALTH_MANA_CHANGE:
				{
					int32_t iMemberID = pkt.read<int32_t>();

					auto pMember = std::find_if(m_vecPartyMembers.begin(), m_vecPartyMembers.end(),
						[&](const PartyMember& a)
						{
							return a.iMemberID == iMemberID;
						});

					if (pMember != m_vecPartyMembers.end())
					{
						pMember->iMaxHP = pkt.read<uint16_t>();
						pMember->iHP = pkt.read<uint16_t>();
						pMember->iMaxMP = pkt.read<uint16_t>();
						pMember->iMP = pkt.read<uint16_t>();
					}
				}
				break;

				case PartyUpdateType::PARTY_UPDATE_LEVEL_CHANGE:
				{
					int32_t iMemberID = pkt.read<int32_t>();

					auto pMember = std::find_if(m_vecPartyMembers.begin(), m_vecPartyMembers.end(),
						[&](const PartyMember& a)
						{
							return a.iMemberID == iMemberID;
						});

					if (pMember != m_vecPartyMembers.end())
					{
						pMember->iLevel = pkt.read<uint8_t>();
					}
				}
				break;

				case PartyUpdateType::PARTY_UPDATE_CLASS_CHANGE:
				{
					int32_t iMemberID = pkt.read<int32_t>();

					auto pMember = std::find_if(m_vecPartyMembers.begin(), m_vecPartyMembers.end(),
						[&](const PartyMember& a)
						{
							return a.iMemberID == iMemberID;
						});

					if (pMember != m_vecPartyMembers.end())
					{
						pMember->iClass = pkt.read<uint16_t>();
					}
				}
				break;

				case PartyUpdateType::PARTY_UPDATE_STATUS_CHANGE:
				{
					// TODO: !!!
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
					SetZoneChange(false);
				}
				break;

				case ZoneChangeOpcode::ZoneChangeTeleport:
				{
					SetZoneChange(true);

					uint8_t iCity = (uint8_t)pkt.read<int16_t>();

					if (m_PlayerMySelf.iCity != iCity)
					{
						m_PlayerMySelf.iCity = iCity;

						m_mapSkillUseTime.clear();

						m_bIsMovingToLoot = false;

						m_vecLootList.clear();

						m_vecRoute.clear();

						m_msLastSupplyTime = std::chrono::milliseconds(0);

						m_PlayerMySelf.bBlinking = false;

						GetUserConfiguration()->SetInt(skCryptDec("Automation"), skCryptDec("Attack"), 0);
						GetUserConfiguration()->SetInt(skCryptDec("Automation"), skCryptDec("Character"), 0);
					}

					m_PlayerMySelf.fZ = (pkt.read<int16_t>() / 10.0f);
					m_PlayerMySelf.fX = (pkt.read<uint16_t>() / 10.0f);
					m_PlayerMySelf.fY = (pkt.read<uint16_t>() / 10.0f);

					uint8_t iVictoryNation = pkt.read<uint8_t>();

					m_vecNpc.clear();
					m_vecPlayer.clear();

#ifdef DEBUG
					printf("RecvProcess::WIZ_ZONE_CHANGE: Teleport Zone: [%d] Coordinate: [%f - %f - %f] VictoryNation: [%d]\n",
						m_PlayerMySelf.iCity, m_PlayerMySelf.fX, m_PlayerMySelf.fY, m_PlayerMySelf.fZ, iVictoryNation);
#endif
				}
				break;

				case ZoneChangeOpcode::ZoneChangeMilitaryCamp:
				{
					m_mapSkillUseTime.clear();

					m_bIsMovingToLoot = false;

					m_vecLootList.clear();

					m_vecRoute.clear();

					m_msLastSupplyTime = std::chrono::milliseconds(0);

					m_vecNpc.clear();
					m_vecPlayer.clear();

					GetUserConfiguration()->SetInt(skCryptDec("Automation"), skCryptDec("Attack"), 0);
					GetUserConfiguration()->SetInt(skCryptDec("Automation"), skCryptDec("Character"), 0);
				}
				break;
			}
		}
		break;

		case WIZ_WARP:
		{
#ifdef DEBUG
			printf("SendProcess::WIZ_WARP\n");
#endif

			bool bTownOrTeleportStopBot = GetUserConfiguration()->GetBool(skCryptDec("Bot"), skCryptDec("TownOrTeleportStopBot"), false);

			if (bTownOrTeleportStopBot)
			{
				GetUserConfiguration()->SetInt(skCryptDec("Automation"), skCryptDec("Attack"), 0);
				GetUserConfiguration()->SetInt(skCryptDec("Automation"), skCryptDec("Character"), 0);
			}

			SetMovePosition(Vector3(0.0f, 0.0f, 0.0f));

			m_PlayerMySelf.fX = (pkt.read<uint16_t>() / 10.0f);
			m_PlayerMySelf.fY = (pkt.read<uint16_t>() / 10.0f);
			m_PlayerMySelf.fZ = (pkt.read<int16_t>() / 10.0f);

			m_vecNpc.clear();
			m_vecPlayer.clear();
			m_vecLootList.clear();
		}
		break;	

		case WIZ_CAPTCHA:
		{
			uint8_t iUnknown = pkt.read<uint8_t>();
			uint8_t iOpCode = pkt.read<uint8_t>();

			switch (iOpCode)
			{
				case 1:
				{
					uint8_t iResult = pkt.read<uint8_t>();

					if (iResult == 1)
					{
#ifdef DEBUG
						printf("RecvProcess::WIZ_CAPTCHA: Image loaded\n");
#endif
						int iBufferLength;
						pkt >> iBufferLength;

						std::vector<uint8_t> vecBuffer(iBufferLength);
						pkt.read(&vecBuffer[0], iBufferLength);

						new std::thread([&,vecBuffer]()
						{ 
							if (!SolveCaptcha(vecBuffer))
							{
								new std::thread([&]()
								{
									std::this_thread::sleep_for(std::chrono::milliseconds(3000));

									RefreshCaptcha();
#ifdef DEBUG
									printf("RecvProcess::WIZ_CAPTCHA: Refreshed\n");
#endif
								});
							}
						});					
					}
					else
					{
#ifdef DEBUG
						printf("RecvProcess::WIZ_CAPTCHA: Image loading failed(%d)\n", iResult);
#endif	
						new std::thread([&]()
						{
							std::this_thread::sleep_for(std::chrono::milliseconds(3000));

							RefreshCaptcha();
#ifdef DEBUG
							printf("RecvProcess::WIZ_CAPTCHA: Refreshed\n");
#endif
						});
					}
				}
				break;

				case 2:
				{
					uint8_t iResult = pkt.read<uint8_t>();

					if (iResult == 1)
					{
#ifdef DEBUG
						printf("RecvProcess::WIZ_CAPTCHA: Success\n");
#endif
					}
					else
					{	
#ifdef DEBUG
						printf("RecvProcess::WIZ_CAPTCHA: Failed(%d)\n", iResult);
#endif
						new std::thread([&]()
						{
							std::this_thread::sleep_for(std::chrono::milliseconds(3000));

							RefreshCaptcha();
#ifdef DEBUG
							printf("RecvProcess::WIZ_CAPTCHA: Refreshed\n");
#endif
						});
					}
				}
				break;

				default:
				{
#ifdef DEBUG
					printf("RecvProcess::WIZ_CAPTCHA: Not implemented opcode(%d)\n", iOpCode);
#endif
				}
				break;
			}
		}
		break;

		case WIZ_CHAT:
		{
			uint8_t iChatMode = pkt.read<uint8_t>();

			switch (iChatMode)
			{
				case GENERAL_CHAT:
				case PRIVATE_CHAT:
				case PARTY_CHAT:
				{
					uint8_t iNation = pkt.read<uint8_t>();
					int32_t iSenderID = pkt.read<int32_t>();

					pkt.SByte();
					std::string szSenderName;
					pkt.readString(szSenderName);

					pkt.DByte();
					std::string szMessage;
					pkt.readString(szMessage);

					uint8_t iRank = pkt.read<uint8_t>();
					uint8_t iAuthority = pkt.read<uint8_t>();

					if (iSenderID != m_PlayerMySelf.iID)
					{
						bool bPartyRequest = GetUserConfiguration()->GetBool(skCryptDec("Listener"), skCryptDec("PartyRequest"), false);
						std::string szPartyRequestMessage = GetUserConfiguration()->GetString(skCryptDec("Listener"), skCryptDec("PartyRequestMessage"), "add");

						if (bPartyRequest
							&& szMessage.rfind(szPartyRequestMessage.c_str(), 0) == 0)
						{
							if (m_vecPartyMembers.size() == 0)
								SendPartyCreate(szSenderName);
							else
								SendPartyInsert(szSenderName);
						}

						bool bTeleportRequest = GetUserConfiguration()->GetBool(skCryptDec("Listener"), skCryptDec("TeleportRequest"), false);
						std::string szTeleportRequestMessage = GetUserConfiguration()->GetString(skCryptDec("Listener"), skCryptDec("TeleportRequestMessage"), "tptp");

						if (IsMage()
							&& m_vecPartyMembers.size() > 0
							&& bTeleportRequest
							&& szMessage.rfind(szTeleportRequestMessage.c_str(), 0) == 0)
						{
							std::vector<__TABLE_UPC_SKILL>* vecAvailableSkills;
							if (GetAvailableSkill(&vecAvailableSkills))
							{
								auto pMember = std::find_if(m_vecPartyMembers.begin(), m_vecPartyMembers.end(),
									[&](const PartyMember& a)
									{
										return a.iMemberID == iSenderID;
									});

								if (pMember != m_vecPartyMembers.end())
								{
									auto it = std::find_if(vecAvailableSkills->begin(), vecAvailableSkills->end(),
										[](const TABLE_UPC_SKILL& a) { return a.iBaseId == 109004; });

									if (it != vecAvailableSkills->end())
									{
										new std::thread([=]() { UseSkillWithPacket(*it, iSenderID); });
									}
								}
							}
						}
					}
				}
				break;

				case PUBLIC_CHAT:
				case ANNOUNCEMENT_CHAT:
				{
					uint8_t iNation = pkt.read<uint8_t>();
					int32_t iSenderID = pkt.read<int32_t>();

					pkt.SByte();
					std::string szSenderName;
					pkt.readString(szSenderName);

					pkt.DByte();
					std::string szMessage;
					pkt.readString(szMessage);

					uint8_t iRank = pkt.read<uint8_t>();
					uint8_t iAuthority = pkt.read<uint8_t>();

					bool bSendTownIfBanNotice = GetUserConfiguration()->GetBool(skCryptDec("Settings"), skCryptDec("SendTownIfBanNotice"), false);
					bool bAttackStatus = GetUserConfiguration()->GetBool(skCryptDec("Automation"), skCryptDec("Attack"), false);
					bool bCharacterStatus = GetUserConfiguration()->GetBool(skCryptDec("Automation"), skCryptDec("Character"), false);
					bool bPlayBeepfIfBanNotice = GetUserConfiguration()->GetBool(skCryptDec("Settings"), skCryptDec("PlayBeepfIfBanNotice"), false);

					std::string searchString = "is currently blocked for using illegal software.";
					size_t found = szMessage.find(searchString);

					if (found != std::string::npos)
					{
#ifdef DEBUG
						printf("RecvProcess::WIZ_CHAT: !! GM !!, ban notification received! \n");

						if (bPlayBeepfIfBanNotice)
						{
							Beep(1000, 500);
							Beep(1000, 500);
							Beep(1000, 500);
						}

						if ((bAttackStatus || bCharacterStatus) && bSendTownIfBanNotice)
						{
							SendTownPacket();
						}
#endif
					}

						
#ifdef DEBUG
					printf("RecvProcess::WIZ_CHAT: PUBLIC_CHAT | ANNOUNCEMENT_CHAT : %s \n", szMessage.c_str());
#endif
				}
				break;

				default:
				{
#ifdef DEBUG
					printf("RecvProcess::WIZ_CHAT: Not implemented chatmode(%d)\n", iChatMode);
#endif
				}
				break;
			}
		}
		break;

		case WIZ_WAREHOUSE:
		{
			uint8_t iOpCode = pkt.read<uint8_t>();

			switch (iOpCode)
			{
				case 0x01:
				{
					uint8_t iUnknown = pkt.read<uint8_t>();
					uint32_t iCoins = pkt.read<uint32_t>();

					for (size_t i = 0; i < WAREHOUSE_MAX; i++)
					{
						TItemData* pItem = &m_PlayerMySelf.tWarehouse[i];

						pItem->iPos = i;

						pkt >> pItem->iItemID >> pItem->iDurability >> pItem->iCount >> pItem->iFlag >> pItem->iSerial >> pItem->iExpirationTime;
					}
				}
				break;

				default:
				{
#ifdef DEBUG
					printf("RecvProcess::WIZ_WAREHOUSE: Not implemented opcode(%d)\n", iOpCode);
#endif
				}
				break;
			}
		}
		break;

		case WIZ_VIPWAREHOUSE:
		{
			uint8_t iOpCode = pkt.read<uint8_t>();

			switch (iOpCode)
			{
				case 0x01:
				{
					uint8_t iResult = pkt.read<uint8_t>();

					switch (iResult)
					{
						case 0x01:
						{
							memset(m_PlayerMySelf.tVipWarehouse, 0, sizeof(m_PlayerMySelf.tVipWarehouse));

							uint8_t iIsPasswordUsed = pkt.read<uint8_t>();
							uint8_t iUnknown2 = pkt.read<uint8_t>();
							uint32_t iExpirationTime = pkt.read<uint32_t>();
							uint32_t iVaultKey = pkt.read<uint32_t>();
							uint16_t iUnknown3 = pkt.read<uint16_t>();
							uint16_t iUnknown4 = pkt.read<uint16_t>();
							uint8_t iUnknown5 = pkt.read<uint8_t>();
							uint32_t iUnknown6 = pkt.read<uint32_t>();
							uint16_t iUnknown7 = pkt.read<uint16_t>();

							for (size_t i = 0; i < VIP_HAVE_MAX; i++)
							{
								TItemData* pItem = &m_PlayerMySelf.tVipWarehouse[i];

								pItem->iPos = i;

								pkt >> pItem->iItemID >> pItem->iDurability >> pItem->iCount >> pItem->iFlag >> pItem->iSerial >> pItem->iExpirationTime;
							}

							m_bVipWarehouseInitialized = true;
							m_bVipWarehouseEnabled = true;
							m_bVipWarehouseLoaded = true;

#ifdef DEBUG
							printf("RecvProcess::WIZ_VIPWAREHOUSE: VIP Warehouse initialized\n");
#endif
						}
						break;

						case 0x15:
						{
							m_bVipWarehouseInitialized = true;
							m_bVipWarehouseEnabled = false;
							m_bVipWarehouseLoaded = false;

#ifdef DEBUG
							printf("RecvProcess::WIZ_VIPWAREHOUSE: VIP Key doesn't exist\n");
#endif
						}
						break;

						default:
						{
#ifdef DEBUG
							printf("RecvProcess::WIZ_VIPWAREHOUSE: Opcode(%d), Not implemented result(%d)\n", iOpCode, iResult);
#endif
						}
						break;
					}
				}
				break;

				default:
				{
#ifdef DEBUG
					printf("RecvProcess::WIZ_VIPWAREHOUSE: Not implemented opcode(%d)\n", iOpCode);
#endif
				}
				break;
			}
		}
		break;

		case WIZ_STORY:
		{
			uint8_t iOpCode = pkt.read<uint8_t>();

			switch (iOpCode)
			{
				case 4: // Flash Item Operation
				{
					uint8_t iResult = pkt.read<uint8_t>();

					switch (iResult)
					{
						case 1: //Use
						{
							m_iFlashCount++;
						}
						break;

						case 2: //Delete
						{
							m_iFlashCount = 0;
						}
						break;

						default:
						{
#ifdef DEBUG
							printf("RecvProcess::WIZ_STORY: Opcode(%d), Not implemented result(%d)\n", iOpCode, iResult);
#endif						
						}
						break;
					}

				}
				break;

				default:
				{
#ifdef DEBUG
					printf("RecvProcess::WIZ_STORY: Not implemented opcode(%d)\n", iOpCode);
#endif
					break;
				}
			}
		}
		break;

		case WIZ_GENIE:
		{
			uint8_t iOpCode = pkt.read<uint8_t>();

			switch (iOpCode)
			{
				//Genie Information
				case 0x01: 
				{
					uint8_t iState = pkt.read<uint8_t>();

					switch (iState)
					{
						//Genie Start
						case 0x04: 
						{
							bool bSyncWithGenie = GetUserConfiguration()->GetBool(skCryptDec("Bot"), skCryptDec("SyncWithGenie"), false);

							if (bSyncWithGenie)
							{
								GetUserConfiguration()->SetInt(skCryptDec("Automation"), skCryptDec("Attack"), 1);
								GetUserConfiguration()->SetInt(skCryptDec("Automation"), skCryptDec("Character"), 1);
							}
						}
						break;

						//Genie Stop
						case 0x05: 
						{
							bool bStartGenieIfUserInRegion = GetUserConfiguration()->GetBool(skCryptDec("Settings"), skCryptDec("StartGenieIfUserInRegion"), false);
							bool bSyncWithGenie = GetUserConfiguration()->GetBool(skCryptDec("Bot"), skCryptDec("SyncWithGenie"), false);

							if (bSyncWithGenie && !bStartGenieIfUserInRegion)
							{
								GetUserConfiguration()->SetInt(skCryptDec("Automation"), skCryptDec("Attack"), 0);
								GetUserConfiguration()->SetInt(skCryptDec("Automation"), skCryptDec("Character"), 0);
							}
						}
						break;
					}
				}
				break;
			}

		}
		break;

		case WIZ_COMPRESS_PACKET:
		{
			uint32_t compressedLength = pkt.read<uint32_t>();
			uint32_t originalLength = pkt.read<uint32_t>();
			uint32_t crc = pkt.read<uint32_t>();

			uint8_t* decompressedBuffer = Compression::DecompressWithCRC32(pkt.contents() + pkt.rpos(), compressedLength, originalLength, crc);

			if (decompressedBuffer != nullptr)
			{
				RecvProcess(decompressedBuffer, originalLength);
			}
		}
		break;

		case WIZ_REGENE:
		{
			m_PlayerMySelf.fX = (pkt.read<uint16_t>() / 10.0f);
			m_PlayerMySelf.fY = (pkt.read<uint16_t>() / 10.0f);
			m_PlayerMySelf.fZ = (pkt.read<int16_t>() / 10.0f);

			m_PlayerMySelf.eState = PSA_BASIC;
		}
		break;

		case WIZ_HP_CHANGE:
		{
			m_PlayerMySelf.iHPMax = pkt.read<uint16_t>();
			m_PlayerMySelf.iHP = pkt.read<uint16_t>();
		}
		break;

		case WIZ_MSP_CHANGE:
		{
			m_PlayerMySelf.iMSPMax = pkt.read<uint16_t>();
			m_PlayerMySelf.iMSP = pkt.read<uint16_t>();
		}
		break;
	}
}

bool ClientHandler::SolveCaptcha(std::vector<uint8_t> vecImageBuffer)
{
	try
	{
		int iWidth, iHeight, iChannel;
		unsigned char* captchaImage = (unsigned char*)stbi_load_from_memory((unsigned char*)vecImageBuffer.data(), vecImageBuffer.size(), &iWidth, &iHeight, &iChannel, 3);

		std::vector<unsigned char> captchaConverted;
		int iRet = stbi_write_jpg_to_func(
			[](void* context, void* data, int size) {
				std::vector<unsigned char>* buffer = static_cast<std::vector<unsigned char> *>(context);
				buffer->insert(buffer->end(), static_cast<unsigned char*>(data), static_cast<unsigned char*>(data) + size);
			}, &captchaConverted, iWidth, iHeight, iChannel, captchaImage, 100);

		if (!iRet)
		{
#ifdef DEBUG
			printf("Error writing JPG data to memory\n");
#endif
			return false;
		}

		CryptoPP::Base64Encoder encoder;
		std::string szBase64Output;

		encoder.Put(captchaConverted.data(), captchaConverted.size());
		encoder.MessageEnd();

		size_t iBase64OutputSize = (size_t)encoder.MaxRetrievable();

		if (iBase64OutputSize == 0)
		{
#ifdef DEBUG
			printf("Error writing image buffer to base64 string, output size 0\n");
#endif
			return false;
		}

		szBase64Output.resize(iBase64OutputSize);
		encoder.Get(reinterpret_cast<CryptoPP::byte*>(&szBase64Output[0]), szBase64Output.size());

		m_Bot->SendCaptcha(szBase64Output);

		return true;
	}
	catch (const std::exception& e)
	{
#ifdef DEBUG
		printf("%s\n", e.what());
#else
		DBG_UNREFERENCED_PARAMETER(e);
#endif
	}

	return false;
}

void ClientHandler::SendProcess(BYTE* byBuffer, DWORD iLength)
{
	Packet pkt = Packet(byBuffer[0], (size_t)iLength);
	pkt.append(&byBuffer[1], iLength - 1);

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
			m_vecLootList.clear();

			if (!IsRouting())
			{
				bool bTownStopBot = GetUserConfiguration()->GetBool(skCryptDec("Bot"), skCryptDec("TownStopBot"), true);

				if (bTownStopBot)
				{
					GetUserConfiguration()->SetInt(skCryptDec("Automation"), skCryptDec("Attack"), 0);
					GetUserConfiguration()->SetInt(skCryptDec("Automation"), skCryptDec("Character"), 0);
				}				
			}
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
						if (iSourceID == m_PlayerMySelf.iID)
						{
							if (IsRogue())
							{
								bool bUseSkillWithPacket = GetUserConfiguration()->GetBool(skCryptDec("Skill"), skCryptDec("UseSkillWithPacket"), false);
								bool bArcherCombo = GetUserConfiguration()->GetBool(skCryptDec("Attack"), skCryptDec("ArcherCombo"), true);

								if (!bUseSkillWithPacket && bArcherCombo)
								{
									DWORD iMobBase = GetEntityBase(iTargetID);

									if (iMobBase != 0)
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
				}
				break;
			}
		}
		break;

		case WIZ_SKILLPT_CHANGE:
		{
			int iType = pkt.read<uint8_t>();

			LoadSkillData();

#ifdef DEBUG
			printf("RecvProcess::WIZ_SKILLPT_CHANGE: %d\n", iType);
#endif
		}
		break;

		case WIZ_ATTACK:
		{
#ifdef DEBUG
			printf("SendProcess::WIZ_ATTACK\n");
#endif
			m_fAttackTimeRecent = Bot::TimeGet();
		}
		break;

		case WIZ_PARTY:
		{
			uint8_t iType = pkt.read<uint8_t>();

			switch (iType)
			{
				case PartyUpdateType::PARTY_UPDATE_CREATE:
				{
					m_vecPartyMembers.clear();

					PartyMember pPartyMemberMySelf;

					memset(&pPartyMemberMySelf, 0, sizeof(pPartyMemberMySelf));

					pPartyMemberMySelf.iMemberID = m_PlayerMySelf.iID;
					pPartyMemberMySelf.iIndex = 100;
					pPartyMemberMySelf.szName = m_PlayerMySelf.szName;

					pPartyMemberMySelf.iMaxHP = m_PlayerMySelf.iHPMax;
					pPartyMemberMySelf.iHP = m_PlayerMySelf.iHP;
					pPartyMemberMySelf.iLevel = m_PlayerMySelf.iLevel;
					pPartyMemberMySelf.iClass = (uint16_t)m_PlayerMySelf.eClass;
					pPartyMemberMySelf.iMaxMP = m_PlayerMySelf.iMSPMax;
					pPartyMemberMySelf.iMP = m_PlayerMySelf.iMSP;

					pPartyMemberMySelf.iNation = m_PlayerMySelf.eNation;

					m_vecPartyMembers.push_back(pPartyMemberMySelf);
				}
				break;
			}
		}
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
		pkt.readString(tNpc.szPetOwnerName);
		pkt.readString(tNpc.szPetName);
	}

	tNpc.iModelGroup = pkt.read<uint8_t>();
	tNpc.iLevel = pkt.read<uint8_t>();

	tNpc.fX = (pkt.read<uint16_t>() / 10.0f);
	tNpc.fY = (pkt.read<uint16_t>() / 10.0f);
	tNpc.fZ = (pkt.read<int16_t>() / 10.0f);

	tNpc.iStatus = pkt.read<uint32_t>();

	pkt.read<uint8_t>();
	pkt.read<uint32_t>();

	tNpc.fRotation = pkt.read<int16_t>() / 100.0f;

	return tNpc;
}

TPlayer ClientHandler::InitializePlayer(Packet& pkt)
{
	TPlayer tPlayer;
	memset(&tPlayer, 0, sizeof(tPlayer));

	pkt.SByte();

	tPlayer.iID = pkt.read<int32_t>();

	pkt.readString(tPlayer.szName);

	tPlayer.eNation = (Nation)pkt.read<uint8_t>();

	pkt.read<uint8_t>();
	pkt.read<uint8_t>();
	pkt.read<uint8_t>();

	tPlayer.iKnightsID = pkt.read<int16_t>();
	tPlayer.eKnightsDuty = (KnightsDuty)pkt.read<uint8_t>();

	if (tPlayer.iKnightsID > 0)
	{
		if (m_Bot->GetPlatformType() == PlatformType::USKO 
			|| m_Bot->GetPlatformType() == PlatformType::STKO)
		{
			int16_t iAllianceID = pkt.read<int16_t>();
		}
		else if (m_Bot->GetPlatformType() == PlatformType::CNKO)
		{
			pkt.read<uint8_t>();
		}

		pkt.readString(tPlayer.szKnights);

		tPlayer.iKnightsGrade = pkt.read<uint8_t>();
		tPlayer.iKnightsRank = pkt.read<uint8_t>();

		int16_t sMarkVersion = pkt.read<int16_t>();
		int16_t sCapeID = pkt.read<int16_t>();
		uint8_t iR = pkt.read<uint8_t>();
		uint8_t iG = pkt.read<uint8_t>();
		uint8_t iB = pkt.read<uint8_t>();

		pkt.read<uint8_t>();
		pkt.read<uint8_t>();
	}
	else
	{
		pkt.read<uint32_t>();
		pkt.read<uint16_t>();

		if (m_Bot->GetPlatformType() == PlatformType::USKO 
			|| m_Bot->GetPlatformType() == PlatformType::STKO)
		{
			pkt.read<uint8_t>();
		}

		int16_t sCapeID = pkt.read<int16_t>();
		uint8_t iR = pkt.read<uint8_t>();
		uint8_t iG = pkt.read<uint8_t>();
		uint8_t iB = pkt.read<uint8_t>();

		pkt.read<uint8_t>();
		pkt.read<uint8_t>();
	}

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
		case ABNORMAL_NORMAL:
			tPlayer.bBlinking = false;
			break;
		case ABNORMAL_BLINKING:
			tPlayer.bBlinking = true;
			break;
		case ABNORMAL_BLINKING_END:
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

	if (m_Bot->GetPlatformType() == PlatformType::USKO 
		|| m_Bot->GetPlatformType() == PlatformType::STKO)
	{
		int32_t iLoop = m_bLunarWarDressUp ? 9 : 16;

		for (int32_t i = 0; i < iLoop; i++)
		{
			tPlayer.tInventory[i].iPos = i;
			tPlayer.tInventory[i].iItemID = pkt.read<uint32_t>();
			tPlayer.tInventory[i].iDurability = pkt.read<uint16_t>();
			tPlayer.tInventory[i].iFlag = pkt.read<uint8_t>();
		}
	}
	else if (m_Bot->GetPlatformType() == PlatformType::CNKO)
	{
		int32_t iLoop = m_bLunarWarDressUp ? 9 : 15;

		for (int32_t i = 0; i < iLoop; i++)
		{
			tPlayer.tInventory[i].iPos = i;
			tPlayer.tInventory[i].iItemID = pkt.read<uint32_t>();
			tPlayer.tInventory[i].iDurability = pkt.read<uint16_t>();
			tPlayer.tInventory[i].iFlag = pkt.read<uint8_t>();
		}
	}

	tPlayer.iCity = pkt.read<uint8_t>();

	if (m_Bot->GetPlatformType() == PlatformType::USKO 
		|| m_Bot->GetPlatformType() == PlatformType::STKO)
	{
		uint8_t iTempBuffer[26];
		pkt.read(iTempBuffer, 26);
	}
	else if (m_Bot->GetPlatformType() == PlatformType::CNKO)
	{
		uint8_t iTempBuffer[17];
		pkt.read(iTempBuffer, 17);
	}

	return tPlayer;
}

void ClientHandler::SetLoginInformation(std::string szAccountId, std::string szPassword)
{
	m_szAccountId = szAccountId;
	m_szPassword = szPassword;
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

	bool bEnableTournamentSkills = false;

#ifdef FEATURE_TOURNAMENT_SKILLS
	bEnableTournamentSkills = true;
#endif

	for (const auto& [key, value] : *pSkillTable)
	{
		if ((bEnableTournamentSkills
			&& (value.iID != 490803 
				&& value.iID != 490811 
				&& value.iID != 490808 
				&& value.iID != 490809 
				&& value.iID != 490810 
				&& value.iID != 490800 
				&& value.iID != 490801
				&& value.iID != 490817))
			|| !bEnableTournamentSkills)
		{
			if (0 != std::to_string(value.iNeedSkill).substr(0, 3).compare(std::to_string(m_PlayerMySelf.eClass)))
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

				if (value.dwNeedItem == 811071000) //New Emotes
					continue;

				switch (value.iNeedSkill % 10)
				{
					case 0:
						if (value.iNeedLevel > m_PlayerMySelf.iLevel)
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
		}

		m_vecAvailableSkill.push_back(value);
	}
}

void ClientHandler::MoveToTargetProcess()
{
	try
	{
		if (Bot::TimeGet() < (m_fLastMoveToTargetProcessTime + (150.0f / 1000.0f)))
			return;

		bool bAttackStatus = GetUserConfiguration()->GetBool(skCryptDec("Automation"), skCryptDec("Attack"), false);

		if (!bAttackStatus)
			return;

		bool bMoveToTarget = GetUserConfiguration()->GetBool(skCryptDec("Attack"), skCryptDec("MoveToTarget"), false);

		if (!bMoveToTarget)
			return;

		if (GetTarget() == -1)
			return;

		if (IsMovingToLoot())
			return;

		if (IsBlinking())
			return;

		if (IsRouting())
			return;

		if (m_PlayerMySelf.eState == PSA_DEATH)
			return;

		bool bDisableStun = GetUserConfiguration()->GetBool(skCryptDec("Attack"), skCryptDec("DisableStun"), false);

		if (!bDisableStun)
		{
			if (IsStunned())
				return;
		}

		DWORD iTargetBase = GetTargetBase();

		if (iTargetBase == 0)
			return;

		std::map<uint32_t, __TABLE_ITEM>* pItemTable;
		if (!m_Bot->GetItemTable(&pItemTable))
			return;

		Vector3 v3TargetPosition = GetTargetPosition();

		if (v3TargetPosition == Vector3(0.0f, 0.0f, 0.0f))
			return;

		float fMySelfRadius = GetRadius() * GetScaleZ();
		float fTargetRadius = GetRadius(iTargetBase) * GetScaleZ(iTargetBase);
		float fDistLimit = (fMySelfRadius + fTargetRadius) / 2.0f;

		auto iLeftHandWeapon = GetInventoryItemSlot(6);
		auto iRightHandWeapon = GetInventoryItemSlot(8);

		if (iLeftHandWeapon.iItemID != 0)
		{
			uint32_t iLeftHandWeaponBaseID = iLeftHandWeapon.iItemID / 1000 * 1000;
			auto pLeftHandWeaponItemData = pItemTable->find(iLeftHandWeaponBaseID);

			if (pLeftHandWeaponItemData != pItemTable->end())
			{
				if (pLeftHandWeaponItemData->second.byKind == ITEM_CLASS_BOW
					|| pLeftHandWeaponItemData->second.byKind == ITEM_CLASS_BOW_LONG
					|| pLeftHandWeaponItemData->second.byKind == ITEM_CLASS_BOW_CROSS)
				{
					fDistLimit += 1.0f;
				}
				else
				{
					//fDistLimit += (float)pLeftHandWeaponItemData->second.siAttackRange / 10.0f;
					fDistLimit += 1.0f;
				}
			}
		}
		else if (iRightHandWeapon.iItemID != 0)
		{
			uint32_t iRightHandWeaponBaseID = iRightHandWeapon.iItemID / 1000 * 1000;
			auto pRightHandWeaponItemData = pItemTable->find(iRightHandWeaponBaseID);

			if (pRightHandWeaponItemData != pItemTable->end())
			{
				if (pRightHandWeaponItemData->second.byKind == ITEM_CLASS_BOW
					|| pRightHandWeaponItemData->second.byKind == ITEM_CLASS_BOW_LONG
					|| pRightHandWeaponItemData->second.byKind == ITEM_CLASS_BOW_CROSS)
				{
					fDistLimit += 1.0f;
				}
				else
				{
					//fDistLimit += (float)pRightHandWeaponItemData->second.siAttackRange / 10.0f;
					fDistLimit += 1.0f;
				}
			}
		}

		if (GetDistance(v3TargetPosition) > fDistLimit
			&& GetActionState() != PSA_SPELLMAGIC)
		{
			SetMovePosition(v3TargetPosition);
		}
		else
		{
			if (GetMoveState() != PSM_STOP)
			{
				SetMovePosition(Vector3(0.0f, 0.0f, 0.0f));
			}
		}

		m_fLastMoveToTargetProcessTime = Bot::TimeGet();
	}
	catch (const std::exception& e)
	{
#ifdef DEBUG
		printf("MoveToTargetProcess:Exception: %s\n", e.what());
#else
		UNREFERENCED_PARAMETER(e);
#endif
	}
}

void ClientHandler::BasicAttackProcess()
{
	try
	{
		bool bAttackStatus = GetUserConfiguration()->GetBool(skCryptDec("Automation"), skCryptDec("Attack"), false);

		if (!bAttackStatus)
			return;

		if (IsZoneChanging())
			return;

		if (IsMovingToLoot())
			return;

		if (IsBlinking())
			return;

		if (IsRouting())
			return;

		if (m_PlayerMySelf.eState == PSA_DEATH)
			return;

		if (GetActionState() == PSA_SPELLMAGIC)
			return;

		DWORD iTargetBase = GetTargetBase();

		if (iTargetBase == 0)
			return;

		if (!IsAttackable(iTargetBase))
			return;

		std::map<uint32_t, __TABLE_ITEM>* pItemTable;
		if (!m_Bot->GetItemTable(&pItemTable))
			return;

		bool bBasicAttack = GetUserConfiguration()->GetBool(skCryptDec("Attack"), skCryptDec("BasicAttack"), false);
		bool bBasicAttackWithPacket = GetUserConfiguration()->GetBool(skCryptDec("Attack"), skCryptDec("BasicAttackWithPacket"), false);

		if (bBasicAttack)
		{
			Vector3 v3TargetPosition = GetTargetPosition();

			float fDistance = GetDistance(v3TargetPosition);

			bool bAttackRangeLimit = GetUserConfiguration()->GetBool(skCryptDec("Attack"), skCryptDec("AttackRangeLimit"), false);
			int iAttackRangeLimitValue = GetUserConfiguration()->GetInt(skCryptDec("Attack"), skCryptDec("AttackRangeLimitValue"), 50);

			if (bAttackRangeLimit
				&& fDistance > (float)iAttackRangeLimitValue)
				return;

			float fBasicAttackIntervalTable = 1.0f;
			bool bIsEquippedBow = false;
			float fEffectiveAttackRange = 10.0f;

			auto iLeftHandWeapon = GetInventoryItemSlot(6);
			auto iRightHandWeapon = GetInventoryItemSlot(8);

			if (iRightHandWeapon.iItemID != 0)
			{
				uint32_t iRightHandWeaponBaseID = iRightHandWeapon.iItemID / 1000 * 1000;
				auto pRightHandWeaponItemData = pItemTable->find(iRightHandWeaponBaseID);

				if (pRightHandWeaponItemData != pItemTable->end())
				{
					if (pRightHandWeaponItemData->second.byKind == ITEM_CLASS_BOW
						|| pRightHandWeaponItemData->second.byKind == ITEM_CLASS_BOW_LONG
						|| pRightHandWeaponItemData->second.byKind == ITEM_CLASS_BOW_CROSS)
					{
						bIsEquippedBow = true;
					}

					fEffectiveAttackRange = (float)pRightHandWeaponItemData->second.siAttackRange;
				}
			}

			if (iLeftHandWeapon.iItemID != 0)
			{
				uint32_t iLeftHandWeaponBaseID = iLeftHandWeapon.iItemID / 1000 * 1000;

				auto pLeftHandWeaponItemData = pItemTable->find(iLeftHandWeaponBaseID);

				if (pLeftHandWeaponItemData != pItemTable->end())
				{
					if (pLeftHandWeaponItemData->second.byKind == ITEM_CLASS_BOW
						|| pLeftHandWeaponItemData->second.byKind == ITEM_CLASS_BOW_LONG
						|| pLeftHandWeaponItemData->second.byKind == ITEM_CLASS_BOW_CROSS)
					{
						bIsEquippedBow = true;
					}

					std::map<uint32_t, __TABLE_ITEM_EXTENSION>* pItemExtensionTable;
					if (m_Bot->GetItemExtensionTable(pLeftHandWeaponItemData->second.byExtIndex, &pItemExtensionTable))
					{
						auto pLeftHandWeaponItemExtensionData = pItemExtensionTable->find(iLeftHandWeapon.iItemID % 1000);

						if (pLeftHandWeaponItemExtensionData != pItemExtensionTable->end())
						{
							fBasicAttackIntervalTable = (pLeftHandWeaponItemData->second.siAttackInterval / 100.0f)
								* (pLeftHandWeaponItemExtensionData->second.iAttackIntervalPercentage / 100.0f);
						}
					}
				}

				fEffectiveAttackRange = (float)pLeftHandWeaponItemData->second.siAttackRange;
			}

			if (!bIsEquippedBow)
			{
				float fAttackInterval = fBasicAttackIntervalTable;

				if (Bot::TimeGet() > m_fAttackTimeRecent + fAttackInterval)
				{
					bool bBasicAttackWithPacket = GetUserConfiguration()->GetBool(skCryptDec("Attack"), skCryptDec("BasicAttackWithPacket"), false);

					if (bBasicAttackWithPacket)
					{
						BasicAttackWithPacket(iTargetBase, fAttackInterval);
					}
					else
					{
						BasicAttack(iTargetBase);
					}
				}
			}
		}
	}
	catch (const std::exception& e)
	{
#ifdef DEBUG
		printf("BasicAttackPacketProcess:Exception: %s\n", e.what());
#else
		UNREFERENCED_PARAMETER(e);
#endif
	}
}

void ClientHandler::AttackProcess()
{
	try
	{
		bool bAttackSpeed = GetUserConfiguration()->GetBool("Attack", "AttackSpeed", false);

		if (bAttackSpeed)
		{
			int iAttackSpeedValue = GetUserConfiguration()->GetInt("Attack", "AttackSpeedValue", 1000);

			if (iAttackSpeedValue > 0)
			{
				if (Bot::TimeGet() < (m_fLastAttackTime + (iAttackSpeedValue / 1000.0f)))
					return;
			}
		}
		else 
		{
			if (Bot::TimeGet() < (m_fLastAttackTime + (1000 / 1000.0f)))
				return;
		}

		bool bAttackStatus = GetUserConfiguration()->GetBool(skCryptDec("Automation"), skCryptDec("Attack"), false);

		if (!bAttackStatus)
			return;

		if (IsZoneChanging())
			return;

		if (IsMovingToLoot())
			return;

		if (IsBlinking())
			return;

		if (IsRouting())
			return;

		if (m_PlayerMySelf.eState == PSA_DEATH)
			return;

		std::map<uint32_t, __TABLE_UPC_SKILL>* pSkillTable;
		if (!m_Bot->GetSkillTable(&pSkillTable))
			return;

		std::map<uint32_t, __TABLE_UPC_SKILL_EXTENSION2>* pSkillExtension2;
		if (!m_Bot->GetSkillExtension2Table(&pSkillExtension2))
			return;

		std::map<uint32_t, __TABLE_ITEM>* pItemTable;
		if (!m_Bot->GetItemTable(&pItemTable))
			return;

		if (GetActionState() == PSA_SPELLMAGIC)
			return;

		DWORD iTargetBase = GetTargetBase();

		if (iTargetBase == 0)
			return;

		if (!IsAttackable(iTargetBase))
			return;

		std::vector<int> vecAttackSkillList = GetUserConfiguration()->GetInt(skCryptDec("Automation"), skCryptDec("AttackSkillList"), std::vector<int>());

		bool bAttackRangeLimit = GetUserConfiguration()->GetBool(skCryptDec("Attack"), skCryptDec("AttackRangeLimit"), false);
		int iAttackRangeLimitValue = GetUserConfiguration()->GetInt(skCryptDec("Attack"), skCryptDec("AttackRangeLimitValue"), 50);
		bool bUseSkillWithPacket = GetUserConfiguration()->GetBool(skCryptDec("Skill"), skCryptDec("UseSkillWithPacket"), false);
		bool bBasicAttack = GetUserConfiguration()->GetBool(skCryptDec("Attack"), skCryptDec("BasicAttack"), false);
		bool bBasicAttackWithPacket = GetUserConfiguration()->GetBool(skCryptDec("Attack"), skCryptDec("BasicAttackWithPacket"), false);

		if (vecAttackSkillList.size() == 0)
			return;

		auto iLeftHandWeapon = GetInventoryItemSlot(6);
		auto iRightHandWeapon = GetInventoryItemSlot(8);

		float fEffectiveAttackRange = 10.0f;

		bool bIsEquippedBow = false;

		if (iRightHandWeapon.iItemID != 0)
		{
			uint32_t iRightHandWeaponBaseID = iRightHandWeapon.iItemID / 1000 * 1000;
			auto pRightHandWeaponItemData = pItemTable->find(iRightHandWeaponBaseID);

			if (pRightHandWeaponItemData != pItemTable->end())
			{
				if (pRightHandWeaponItemData->second.byKind == ITEM_CLASS_BOW
					|| pRightHandWeaponItemData->second.byKind == ITEM_CLASS_BOW_LONG
					|| pRightHandWeaponItemData->second.byKind == ITEM_CLASS_BOW_CROSS)
				{
					bIsEquippedBow = true;
				}

				fEffectiveAttackRange = (float)pRightHandWeaponItemData->second.siAttackRange;
			}
		}

		if (iLeftHandWeapon.iItemID != 0)
		{
			uint32_t iLeftHandWeaponBaseID = iLeftHandWeapon.iItemID / 1000 * 1000;

			auto pLeftHandWeaponItemData = pItemTable->find(iLeftHandWeaponBaseID);

			if (pLeftHandWeaponItemData != pItemTable->end())
			{
				if (pLeftHandWeaponItemData->second.byKind == ITEM_CLASS_BOW
					|| pLeftHandWeaponItemData->second.byKind == ITEM_CLASS_BOW_LONG
					|| pLeftHandWeaponItemData->second.byKind == ITEM_CLASS_BOW_CROSS)
				{
					bIsEquippedBow = true;
				}

				fEffectiveAttackRange = (float)pLeftHandWeaponItemData->second.siAttackRange;
			}
		}

		float fTargetRadius = GetRadius(iTargetBase) * GetScaleZ(iTargetBase);
		float fMySelfRadius = GetRadius() * GetScaleZ();

		Vector3 v3TargetPosition = GetTargetPosition();

		float fDistance = GetDistance(v3TargetPosition);

		float fCalculatedEffectiveAttackRange = fEffectiveAttackRange / 10.0f;

		std::vector<int> vecFilteredAttackSkill;

		std::copy_if(vecAttackSkillList.begin(), vecAttackSkillList.end(),
			std::back_inserter(vecFilteredAttackSkill),
			[&](const int& iSkillID)
			{
				auto pSkillData = pSkillTable->find(iSkillID);

				if (pSkillData == pSkillTable->end())
					return false;

				if (IsSkillHasZoneLimit(iSkillID))
					return false;

				float fCurrentTime = Bot::TimeGet();
				float fSkillNextUseTime = GetSkillNextUseTime(iSkillID);

				if (fCurrentTime < fSkillNextUseTime)
					return false;

				if (m_bSkillCasting == true && pSkillData->second.iReCastTime > 0)
					return false;

				uint32_t iNeedItem = pSkillData->second.dwNeedItem;

				uint32_t iNeedItemCount = 1;
				uint32_t iExistItemCount = 0;

				if (iNeedItem != 0)
				{
					iExistItemCount = GetInventoryItemCount(iNeedItem);

					auto pSkillExtension2Data = pSkillExtension2->find(pSkillData->second.iID);

					if (pSkillExtension2Data != pSkillExtension2->end())
						iNeedItemCount = pSkillExtension2Data->second.iArrowCount;
				}

				if (m_PlayerMySelf.iMSP < pSkillData->second.iExhaustMSP
					|| (iNeedItem != 0 && iExistItemCount < iNeedItemCount))
				{
					return false;
				}

				if ((pSkillData->second.iID == 490801
					|| pSkillData->second.iID == 490800
					|| pSkillData->second.iID == 490817))
				{
					if (GetRegionUserCount(true) > 0)
						return false;
				}

				if (pSkillData->second.IsDeBuff())
				{
					int16_t iTargetHp = GetHp(iTargetBase);
					int16_t iTargetMaxHp = GetMaxHp(iTargetBase);

					if (iTargetMaxHp != 0)
					{
						int32_t iTargetHpPercent = (int32_t)std::ceil((iTargetHp * 100) / iTargetMaxHp);
						int32_t iTargetHpNeededValue = 60;

						if (iTargetHpPercent < iTargetHpNeededValue)
							return false;
					}
				}

				if (!bIsEquippedBow
					&& pSkillData->second.dw1stTableType == 2)
				{
					return false;
				}

				if (bAttackRangeLimit
					&& fDistance > (float)iAttackRangeLimitValue)
					return false;

				if (!bUseSkillWithPacket)
				{
					if (pSkillData->second.iValidDist > 0
						&& fDistance > (float)pSkillData->second.iValidDist)
						return false;

					float fDistanceLimit = (fMySelfRadius + fTargetRadius) / 2.0f;
					fDistanceLimit += fCalculatedEffectiveAttackRange;

					if (pSkillData->second.iValidDist == 0
						&& fDistance > fDistanceLimit)
						return false;
				}

				return true;
			});

		if (vecFilteredAttackSkill.size() > 0)
		{
			int iAttackSkillID = vecFilteredAttackSkill.front();

			auto pSkillData = pSkillTable->find(iAttackSkillID);

			if (pSkillData == pSkillTable->end())
				return;

			if (bUseSkillWithPacket
				|| (pSkillData->second.iID == 490808
					|| pSkillData->second.iID == 490809
					|| pSkillData->second.iID == 490810
					|| pSkillData->second.iID == 490800
					|| pSkillData->second.iID == 490801
					|| pSkillData->second.iID == 490817))
			{
				new std::thread([=]() { UseSkillWithPacket(pSkillData->second, GetTarget()); });	
			}
			else
			{
				UseSkill(pSkillData->second, GetTarget(), 0, false);
			}
		}

		m_fLastAttackTime = Bot::TimeGet();
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

void ClientHandler::SearchTargetProcess()
{
	try
	{
		bool bSearchTargetSpeed = GetUserConfiguration()->GetBool("Attack", "SearchTargetSpeed", false);

		if (bSearchTargetSpeed)
		{
			int iSearchTargetSpeedValue = GetUserConfiguration()->GetInt("Attack", "SearchTargetSpeedValue", 100);

			if (iSearchTargetSpeedValue > 0)
			{
				if (Bot::TimeGet() < (m_fLastSearchTargetTime + (iSearchTargetSpeedValue / 1000.0f)))
					return;
			}
		}
		else
		{
			if (Bot::TimeGet() < (m_fLastSearchTargetTime + (100.0f / 1000.0f)))
				return;
		}

		bool bAttackStatus = GetUserConfiguration()->GetBool(skCryptDec("Automation"), skCryptDec("Attack"), false);

		if (!bAttackStatus)
			return;

		if (IsZoneChanging())
			return;

		if (IsBlinking())
			return;

		if (IsMovingToLoot())
			return;

		if (IsRouting())
			return;

		if (m_PlayerMySelf.eState == PSA_DEATH)
			return;

		if (GetActionState() == PSA_SPELLMAGIC)
			return;

		bool bClosestTarget = GetUserConfiguration()->GetInt(skCryptDec("Attack"), skCryptDec("ClosestTarget"), false);

		if (!bClosestTarget)
		{
			if ((GetTarget() != -1 && IsAttackable(GetTargetBase())))
				return;
		}

		std::map<uint32_t, __TABLE_ITEM>* pItemTable;
		if (!m_Bot->GetItemTable(&pItemTable))
			return;

		bool bAutoTarget = GetUserConfiguration()->GetInt(skCryptDec("Attack"), skCryptDec("AutoTarget"), false);
		bool bRangeLimit = GetUserConfiguration()->GetBool(skCryptDec("Attack"), skCryptDec("RangeLimit"), false);
		int iRangeLimitValue = GetUserConfiguration()->GetInt(skCryptDec("Attack"), skCryptDec("RangeLimitValue"), 100);

		std::vector<TNpc> vecFilteredTarget;

		if (bAutoTarget)
		{
			std::copy_if(m_vecNpc.begin(), m_vecNpc.end(),
				std::back_inserter(vecFilteredTarget),
				[&](const TNpc& c)
				{
					return
						c.iMonsterOrNpc == 1
						&& c.iProtoID != 0
						&& c.iProtoID != 9009
						&& c.eState != PSA_DYING
						&& c.eState != PSA_DEATH
						&& ((c.iHPMax == 0) || (c.iHPMax != 0 && c.iHP > 0))
						&& ((bRangeLimit && GetDistance(Vector3(c.fX, c.fZ, c.fY)) <= (float)iRangeLimitValue) || !bRangeLimit);
				});
		}
		else
		{
			std::vector<int> vecSelectedNpcList = GetUserConfiguration()->GetInt(skCryptDec("Attack"), skCryptDec("NpcList"), std::vector<int>());
			std::vector<int> vecSelectedNpcIDList = GetUserConfiguration()->GetInt(skCryptDec("Attack"), skCryptDec("NpcIDList"), std::vector<int>());

			std::copy_if(m_vecNpc.begin(), m_vecNpc.end(),
				std::back_inserter(vecFilteredTarget),
				[&](const TNpc& c)
				{
					return
						c.iMonsterOrNpc == 1
						&& c.iProtoID != 0
						&& c.iProtoID != 9009
						&& c.eState != PSA_DYING
						&& c.eState != PSA_DEATH
						&& ((c.iHPMax == 0) || (c.iHPMax != 0 && c.iHP > 0))
						&& (std::count(vecSelectedNpcList.begin(), vecSelectedNpcList.end(), c.iProtoID)
							|| std::count(vecSelectedNpcIDList.begin(), vecSelectedNpcIDList.end(), c.iID))
						&& ((bRangeLimit && GetDistance(Vector3(c.fX, c.fZ, c.fY)) <= (float)iRangeLimitValue) || !bRangeLimit);
				});
		}

		if (vecFilteredTarget.size() > 0)
		{
			Vector3 v3CurrentPosition = GetPosition();

			auto pSort = [&](TNpc const& a, TNpc const& b)
				{
					auto fADistance = GetDistance(v3CurrentPosition, Vector3(a.fX, a.fZ, a.fY));
					auto fBDistance = GetDistance(v3CurrentPosition, Vector3(b.fX, b.fZ, b.fY));

					if (fADistance != fBDistance)
					{
						return fADistance < fBDistance;
					}

					return false;
				};

			std::sort(vecFilteredTarget.begin(), vecFilteredTarget.end(), pSort);

			auto& pSelectedTarget = vecFilteredTarget.front();

			if (pSelectedTarget.iID != GetTarget())
			{
				DWORD iNewTargetBase = GetEntityBase(pSelectedTarget.iID);

				if (iNewTargetBase == 0)
					return;

				DWORD iNewTargetState = GetActionState(iNewTargetBase);

				if ((iNewTargetState == PSA_DYING || iNewTargetState == PSA_DEATH))
					return;

				SetTarget(iNewTargetBase);

				bool bTargetSizeEnable = GetUserConfiguration()->GetBool(skCryptDec("Target"), skCryptDec("SizeEnable"), false);

				if (bTargetSizeEnable)
				{
					int iTargetSize = GetUserConfiguration()->GetInt(skCryptDec("Target"), skCryptDec("Size"), 1);
					SetScale(iNewTargetBase, (float)iTargetSize, (float)iTargetSize, (float)iTargetSize);
				}
			}
		}

		m_fLastSearchTargetTime = Bot::TimeGet();
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

void ClientHandler::AutoLootProcess()
{
	try
	{
		if (Bot::TimeGet() < (m_fLastAutoLootProcessTime + (100.0f / 1000.0f)))
			return;

		bool bAutoLoot = GetUserConfiguration()->GetBool(skCryptDec("AutoLoot"), skCryptDec("Enable"), false);

		if (!bAutoLoot)
			return;

		if (IsZoneChanging())
			return;

		if (IsBlinking())
			return;

		if (IsRouting())
			return;

		if (m_PlayerMySelf.eState == PSA_DEATH)
			return;

		if (IsStunned())
			return;

		std::vector<TLoot> vecFilteredLoot;
		std::copy_if(m_vecLootList.begin(), m_vecLootList.end(),
			std::back_inserter(vecFilteredLoot),
			[&](const TLoot& c)
			{
				return 
					GetEntityBase(c.iNpcID) == 0 && 
					(Bot::TimeGet() - c.fDropTime < 60000.0f / 1000.0f && Bot::TimeGet() - c.fDropTime > 1000.0f / 1000.0f);
			});

		if (vecFilteredLoot.size() == 0)
		{
			if (IsMovingToLoot())
			{
				SetMovingToLoot(false);
			}
				
			return;
		}

		auto pSort = [=](TLoot const& a, TLoot const& b)
			{
				auto fADistance = GetDistance(Vector3(a.fNpcX, a.fNpcZ, a.fNpcY), GetPosition());
				auto fBDistance = GetDistance(Vector3(b.fNpcX, b.fNpcZ, b.fNpcY), GetPosition());

				if (fADistance != fBDistance)
				{
					return fADistance < fBDistance;
				}

				return false;
			};

		std::sort(vecFilteredLoot.begin(), vecFilteredLoot.end(), pSort);

		auto& pFindedLoot = vecFilteredLoot.front();

		bool bMoveToLoot = GetUserConfiguration()->GetBool(skCryptDec("AutoLoot"), skCryptDec("MoveToLoot"), false);

		Vector3 vec3NpcPosition = Vector3(pFindedLoot.fNpcX, pFindedLoot.fNpcZ, pFindedLoot.fNpcY);

		if (GetDistance(vec3NpcPosition) > 8.0f
			&& GetDistance(vec3NpcPosition) < 50.0f)
		{
			if (bMoveToLoot) 
			{
				if (GetActionState() == PSA_SPELLMAGIC || IsStunned())
					return;

				SetMovePosition(vec3NpcPosition);
				SetMovingToLoot(true);
			}
		}
		else
		{
			SendRequestBundleOpen(pFindedLoot.iBundleID);
		}

		m_fLastAutoLootProcessTime = Bot::TimeGet();
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

void ClientHandler::MinorProcess()
{
	try
	{
		if (Bot::TimeGet() < (m_fLastMinorProcessTime + (500.0f / 1000.0f)))
			return;

		bool bMinorProtection = GetUserConfiguration()->GetBool(skCryptDec("Protection"), skCryptDec("Minor"), false);

		if (!bMinorProtection)
			return;

		if (IsZoneChanging())
			return;

		if (IsBlinking())
			return;

		if (m_PlayerMySelf.eState == PSA_DEATH)
			return;

		std::vector<__TABLE_UPC_SKILL>* vecAvailableSkills;
		if (!GetAvailableSkill(&vecAvailableSkills))
			return;

		int32_t iHpProtectionValue = GetUserConfiguration()->GetInt(skCryptDec("Protection"), skCryptDec("MinorValue"), 30);

		if (m_PlayerMySelf.iHP > 0 && (int32_t)std::ceil((m_PlayerMySelf.iHP * 100) / m_PlayerMySelf.iHPMax) < iHpProtectionValue)
		{
			auto it = std::find_if(vecAvailableSkills->begin(), vecAvailableSkills->end(),
				[](const TABLE_UPC_SKILL& a) { return a.iBaseId == 107705; });

			if (it == vecAvailableSkills->end())
				return;

			if (m_PlayerMySelf.iMSP < it->iExhaustMSP)
				return;

			bool bUseSkillWithPacket = GetUserConfiguration()->GetBool(skCryptDec("Skill"), skCryptDec("UseSkillWithPacket"), false);
			bool bOnlyAttackSkillUseWithPacket = GetUserConfiguration()->GetBool(skCryptDec("Skill"), skCryptDec("OnlyAttackSkillUseWithPacket"), false);

			if (!bOnlyAttackSkillUseWithPacket
				&& bUseSkillWithPacket)
			{
				new std::thread([=]() { UseSkillWithPacket(*it, m_PlayerMySelf.iID); });
			}
			else
			{
				UseSkill(*it, m_PlayerMySelf.iID, 1);
			}

			m_fLastMinorProcessTime = Bot::TimeGet();
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

void ClientHandler::PotionProcess()
{
	try
	{
		if (Bot::TimeGet() < (m_fLastPotionProcessTime + (500.0f / 1000.0f)))
			return;

		if (IsZoneChanging())
			return;

		if (IsBlinking())
			return;

		if (m_PlayerMySelf.eState == PSA_DEATH)
			return;

		bool bHpProtectionEnable = GetUserConfiguration()->GetBool(skCryptDec("Protection"), skCryptDec("Hp"), false);

		if (bHpProtectionEnable)
		{
			int16_t iHp = m_PlayerMySelf.iHP; int16_t iMaxHp = m_PlayerMySelf.iHPMax;

			if (iHp > 0 && iMaxHp > 0)
			{
				int32_t iHpProtectionPercent = (int32_t)std::ceil((iHp * 100) / iMaxHp);
				int32_t iHpProtectionValue = GetUserConfiguration()->GetInt(skCryptDec("Protection"), skCryptDec("HpValue"), 50);

				if (iHpProtectionPercent <= iHpProtectionValue)
				{
					if (HealthPotionProcess())
					{
					}
				}
			}
		}

		bool bMpProtectionEnable = GetUserConfiguration()->GetBool(skCryptDec("Protection"), skCryptDec("Mp"), false);

		if (bMpProtectionEnable)
		{
			int16_t iMp = m_PlayerMySelf.iMSP; int16_t iMaxMp = m_PlayerMySelf.iMSPMax;

			if (iMp > 0 && iMaxMp > 0)
			{
				int32_t iMpProtectionPercent = (int32_t)std::ceil((iMp * 100) / iMaxMp);
				int32_t iMpProtectionValue = GetUserConfiguration()->GetInt(skCryptDec("Protection"), skCryptDec("MpValue"), 50);

				if (iMpProtectionPercent <= iMpProtectionValue)
				{
					if (ManaPotionProcess())
					{
					}
				}
			}
		}

		m_fLastPotionProcessTime = Bot::TimeGet();
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

void ClientHandler::MagicHammerProcess()
{
	try
	{
		if (Bot::TimeGet() < (m_fLastMagicHammerProcessTime + (3000.0f / 1000.0f)))
			return;

		bool bAutoRepairMagicHammer = GetUserConfiguration()->GetBool(skCryptDec("Supply"), skCryptDec("AutoRepairMagicHammer"), false);

		if (!bAutoRepairMagicHammer)
			return;

		if (IsZoneChanging())
			return;

		if (IsBlinking())
			return;

		if (m_PlayerMySelf.eState == PSA_DEATH)
			return;

		if (!IsNeedRepair())
			return;

		std::vector<uint32_t> vecMagicHammers =
		{
			379099000, 399288000, 399291000, 914013000,
			931773000
		};

		auto pSort = [](uint32_t const& a, uint32_t const& b)
			{
				return a < b;
			};

		std::sort(vecMagicHammers.begin(), vecMagicHammers.end(), pSort);

		TItemData tInventoryItem;

		for (const auto& x : vecMagicHammers)
		{
			tInventoryItem = GetInventoryItem(x);

			if (tInventoryItem.iItemID != 0)
			{
				UseItem(tInventoryItem.iItemID);
				break;
			}
		}

		m_fLastMagicHammerProcessTime = Bot::TimeGet();
	}
	catch (const std::exception& e)
	{
#ifdef DEBUG
		printf("MagicHammerProcess:Exception: %s\n", e.what());
#else
		UNREFERENCED_PARAMETER(e);
#endif
	}
}

void ClientHandler::SpeedHackProcess()
{
	try
	{
		if (Bot::TimeGet() < (m_fLastSpeedHackProcessTime + (500.0f / 1000.0f)))
			return;

		bool bSpeedHack = GetUserConfiguration()->GetBool(skCryptDec("Feature"), skCryptDec("SpeedHack"), false);

		if (!bSpeedHack)
			return;

		if (IsZoneChanging())
			return;

		if (m_PlayerMySelf.eState == PSA_DEATH)
			return;

		if (GetActionState() == PSA_SPELLMAGIC)
			return;

		if (IsStunned())
			return;

		if (GetCharacterSpeed() != 1.5)
		{
			SetCharacterSpeed(1.5);
		}

		Vector3 vecMoveCoordinate = GetPosition();

		if (GetMoveState() != 0)
		{
			SendMovePacket(vecMoveCoordinate, vecMoveCoordinate, 45, GetMoveState());
		}

		m_fLastSpeedHackProcessTime = Bot::TimeGet();
	}
	catch (const std::exception& e)
	{
#ifdef DEBUG
		printf("SpeedHackProcess:Exception: %s\n", e.what());
#else
		UNREFERENCED_PARAMETER(e);
#endif
	}
}

void ClientHandler::TransformationProcess()
{
	try
	{
		if (Bot::TimeGet() < (m_fLastTransformationProcessTime + (3000.0f / 1000.0f)))
			return;

		bool bAttackStatus = GetUserConfiguration()->GetBool(skCryptDec("Automation"), skCryptDec("Attack"), false);
		bool bCharacterStatus = GetUserConfiguration()->GetBool(skCryptDec("Automation"), skCryptDec("Character"), false);

		if (bAttackStatus || bCharacterStatus)
		{
			if (IsZoneChanging())
				return;

			if (IsBlinking())
				return;

			if (m_PlayerMySelf.eState == PSA_DEATH)
				return;

			bool bAutoTransformation = GetUserConfiguration()->GetBool(skCryptDec("Transformation"), skCryptDec("Auto"), false);
			int iTransformationItem = GetUserConfiguration()->GetInt(skCryptDec("Transformation"), skCryptDec("Item"), 381001000);
			int iTransformationSkill = GetUserConfiguration()->GetInt(skCryptDec("Transformation"), skCryptDec("Skill"), 472020);

			if (bAutoTransformation
				&& iTransformationSkill > 0
				&& IsTransformationAvailable())
			{
				std::map<uint32_t, __TABLE_UPC_SKILL>* pSkillTable;
				if (m_Bot->GetSkillTable(&pSkillTable))
				{
					switch (iTransformationItem)
					{
					case 379090000:
					{
						auto pTransformationScroll = pSkillTable->find(470001);
						auto pTransformationSkill = pSkillTable->find(iTransformationSkill);

						if (pTransformationScroll != pSkillTable->end()
							&& pTransformationSkill != pSkillTable->end())
						{
							if (GetInventoryItemCount(pTransformationScroll->second.dwNeedItem) > 0
								&& m_PlayerMySelf.iLevel >= pTransformationSkill->second.iNeedLevel)
							{
								new std::thread([=]() 
								{ 
									UseSkillWithPacket(pTransformationScroll->second, m_PlayerMySelf.iID);
									UseSkillWithPacket(pTransformationSkill->second, m_PlayerMySelf.iID);
								});
							}
						}
					}
					break;

					case 379093000:
					{
						auto pTransformationScroll = pSkillTable->find(471001);
						auto pTransformationSkill = pSkillTable->find(iTransformationSkill);

						if (pTransformationScroll != pSkillTable->end()
							&& pTransformationSkill != pSkillTable->end())
						{
							if (GetInventoryItemCount(pTransformationScroll->second.dwNeedItem) > 0
								&& m_PlayerMySelf.iLevel >= pTransformationSkill->second.iNeedLevel)
							{
								new std::thread([=]()
								{
									UseSkillWithPacket(pTransformationScroll->second, m_PlayerMySelf.iID);
									UseSkillWithPacket(pTransformationSkill->second, m_PlayerMySelf.iID);
								});
							}
						}
					}
					break;

					case 381001000:
					{
						auto pTransformationScroll = pSkillTable->find(472001);
						auto pTransformationSkill = pSkillTable->find(iTransformationSkill);

						if (pTransformationScroll != pSkillTable->end()
							&& pTransformationSkill != pSkillTable->end())
						{
							if (GetInventoryItemCount(pTransformationScroll->second.dwNeedItem) > 0
								&& m_PlayerMySelf.iLevel >= pTransformationSkill->second.iNeedLevel)
							{
								new std::thread([=]()
								{
									UseSkillWithPacket(pTransformationScroll->second, m_PlayerMySelf.iID);
									UseSkillWithPacket(pTransformationSkill->second, m_PlayerMySelf.iID);
								});
							}
						}
					}
					break;

					case -1:
					{
						auto pTransformationScroll = pSkillTable->find(472001);
						auto pTransformationSkill = pSkillTable->find(iTransformationSkill);

						if (pTransformationScroll != pSkillTable->end()
							&& pTransformationSkill != pSkillTable->end())
						{
							if (GetInventoryItemCount(pTransformationScroll->second.dwNeedItem) > 0
								&& m_PlayerMySelf.iLevel >= pTransformationSkill->second.iNeedLevel)
							{
								new std::thread([=]()
									{
										UseSkillWithPacket(pTransformationScroll->second, m_PlayerMySelf.iID);
										UseSkillWithPacket(pTransformationSkill->second, m_PlayerMySelf.iID);
									});
							}
						}
					}
					break;
					}
				}
			}
		}

		m_fLastTransformationProcessTime = Bot::TimeGet();
	}
	catch (const std::exception& e)
	{
#ifdef DEBUG
		printf("TransformationProcess:Exception: %s\n", e.what());
#else
		UNREFERENCED_PARAMETER(e);
#endif
	}
}

void ClientHandler::FlashProcess()
{
	try
	{
		if (Bot::TimeGet() < (m_fLastFlashProcessTime + (500.0f / 1000.0f)))
			return;

		bool bAttackStatus = GetUserConfiguration()->GetBool(skCryptDec("Automation"), skCryptDec("Attack"), false);
		bool bCharacterStatus = GetUserConfiguration()->GetBool(skCryptDec("Automation"), skCryptDec("Character"), false);

		if (bAttackStatus || bCharacterStatus)
		{
			if (IsZoneChanging())
				return;

			if (IsBlinking())
				return;

			if (m_PlayerMySelf.eState == PSA_DEATH)
				return;

			bool bAutoDCFlash = GetUserConfiguration()->GetBool(skCryptDec("Settings"), skCryptDec("AutoDCFlash"), false);

			if (bAutoDCFlash)
			{
				int iAutoDCFlashCount = GetUserConfiguration()->GetInt(skCryptDec("Settings"), skCryptDec("AutoDCFlashCount"), 10);

				TItemData tInventoryItem = GetInventoryItem(399128000);

				if (tInventoryItem.iItemID != 0
					&& iAutoDCFlashCount != m_iFlashCount)
				{
					UseItem(tInventoryItem.iItemID);
				}
			}

			bool bAutoWarFlash = GetUserConfiguration()->GetBool(skCryptDec("Settings"), skCryptDec("AutoWarFlash"), false);

			if (bAutoWarFlash)
			{
				int iAutoWarFlashCount = GetUserConfiguration()->GetInt(skCryptDec("Settings"), skCryptDec("AutoWarFlashCount"), 10);

				TItemData tInventoryItem = GetInventoryItem(399129000);

				if (tInventoryItem.iItemID != 0
					&& iAutoWarFlashCount != m_iFlashCount)
				{
					UseItem(tInventoryItem.iItemID);
				}
			}

			bool bAutoExpFlash = GetUserConfiguration()->GetBool(skCryptDec("Settings"), skCryptDec("AutoExpFlash"), false);

			if (bAutoExpFlash)
			{
				int iAutoExpFlashCount = GetUserConfiguration()->GetInt(skCryptDec("Settings"), skCryptDec("AutoExpFlashCount"), 10);

				TItemData tInventoryItem = GetInventoryItem(399127000);

				if (tInventoryItem.iItemID != 0
					&& iAutoExpFlashCount != m_iFlashCount)
				{
					UseItem(tInventoryItem.iItemID);
				}
			}
		}

		m_fLastFlashProcessTime = Bot::TimeGet();
	}
	catch (const std::exception& e)
	{
#ifdef DEBUG
		printf("FlashProcess:Exception: %s\n", e.what());
#else
		UNREFERENCED_PARAMETER(e);
#endif
	}
}

void ClientHandler::RegionProcess()
{
	try
	{
		if (Bot::TimeGet() < (m_fLastRegionProcessTime + (500.0f / 1000.0f)))
			return;

		if (IsZoneChanging())
			return;

		if (IsBlinking())
			return;

		if (m_PlayerMySelf.eState == PSA_DEATH)
			return;

		bool bAttackStatus = GetUserConfiguration()->GetBool(skCryptDec("Automation"), skCryptDec("Attack"), false);
		bool bCharacterStatus = GetUserConfiguration()->GetBool(skCryptDec("Automation"), skCryptDec("Character"), false);

		bool bStartGenieIfUserInRegion = GetUserConfiguration()->GetBool(skCryptDec("Settings"), skCryptDec("StartGenieIfUserInRegion"), false);

		if (!IsRouting()
			&& bStartGenieIfUserInRegion
			&& (bAttackStatus || bCharacterStatus))
		{
			std::chrono::milliseconds msNow = duration_cast<std::chrono::milliseconds>(
				std::chrono::system_clock::now().time_since_epoch()
			);

			if (GetRegionUserCount(true) > 0
				&& m_msLastGenieStartTime == std::chrono::milliseconds(0))
			{
				SendStartGenie();

				m_msLastGenieStartTime = msNow;
			}
			else if (GetRegionUserCount(true) == 0
				&& m_msLastGenieStartTime != std::chrono::milliseconds(0)
				&& (m_msLastGenieStartTime.count() + 180000) < msNow.count())
			{
				SendStopGenie();

				m_msLastGenieStartTime = std::chrono::milliseconds(0);
			}
		}

		m_fLastRegionProcessTime = Bot::TimeGet();
	}
	catch (const std::exception& e)
	{
#ifdef DEBUG
		printf("RegionProcess:Exception: %s\n", e.what());
#else
		UNREFERENCED_PARAMETER(e);
#endif
	}
}

void ClientHandler::PartyProcess()
{
	/*try
	{
		bool bCharacterStatus = GetUserConfiguration()->GetBool(skCryptDec("Automation"), skCryptDec("Character"), false);

		if (!bCharacterStatus)
			return;

		if (IsZoneChanging())
			return;

		if (IsBlinking())
			return;

		if (IsDeath())
			return;

		if (GetActionState() == PSA_SPELLMAGIC)
			return;

		if (IsRouting()
			&& (GetRouteStep() == RouteStepType::STEP_SUNDRIES
				|| GetRouteStep() == RouteStepType::STEP_POTION
				|| GetRouteStep() == RouteStepType::STEP_INN))
			return;

		if (m_vecPartyMembers.size() > 0)
		{
			if (IsPriest() || IsRogue())
			{
				bool bPartySwift = GetUserConfiguration()->GetBool(skCryptDec("Rogue"), skCryptDec("PartySwift"), false);

				if (bPartySwift && IsRogue() && pMember.iMemberID != GetID())
				{
					RogueCharacterProcess(true);
				}
			}
		}
	}
	catch (const std::exception& e)
	{
#ifdef DEBUG
		printf("PartyProcess:Exception: %s\n", e.what());
#else
		UNREFERENCED_PARAMETER(e);
#endif
	}*/
}

void ClientHandler::CharacterProcess()
{
	try
	{
		if (Bot::TimeGet() < (m_fLastCharacterProcessTime + (500.0f / 1000.0f)))
			return;

		bool bCharacterStatus = GetUserConfiguration()->GetBool(skCryptDec("Automation"), skCryptDec("Character"), false);

		if (!bCharacterStatus)
			return;

		if (IsZoneChanging())
			return;

		if (IsBlinking())
			return;

		if (m_PlayerMySelf.eState == PSA_DEATH)
			return;

		std::vector<__TABLE_UPC_SKILL>* vecAvailableSkills;
		if (!GetAvailableSkill(&vecAvailableSkills))
			return;

		std::map<uint32_t, __TABLE_UPC_SKILL>* pSkillTable;
		if (!m_Bot->GetSkillTable(&pSkillTable))
			return;

		std::map<uint32_t, __TABLE_UPC_SKILL_EXTENSION2>* pSkillExtension2;
		if (!m_Bot->GetSkillExtension2Table(&pSkillExtension2))
			return;

		std::map<uint32_t, __TABLE_UPC_SKILL_EXTENSION3>* pSkillExtension3;
		if (!m_Bot->GetSkillExtension3Table(&pSkillExtension3))
			return;

		std::map<uint32_t, __TABLE_UPC_SKILL_EXTENSION4>* pSkillExtension4;
		if (!m_Bot->GetSkillExtension4Table(&pSkillExtension4))
			return;

		if (GetActionState() == PSA_SPELLMAGIC)
			return;

		if (IsRouting()
			&& (GetRouteStep() == RouteStepType::STEP_SUNDRIES
				|| GetRouteStep() == RouteStepType::STEP_POTION
				|| GetRouteStep() == RouteStepType::STEP_INN))
			return;

		std::vector<int> vecCharacterSkillList = GetUserConfiguration()->GetInt(skCryptDec("Automation"), skCryptDec("CharacterSkillList"), std::vector<int>());

		if (vecCharacterSkillList.size() == 0)
			return;

		bool bPartySwift = GetUserConfiguration()->GetBool(skCryptDec("Rogue"), skCryptDec("PartySwift"), false);
		bool bPriestPartyHeal = GetUserConfiguration()->GetBool(skCryptDec("Priest"), skCryptDec("PartyHeal"), false);
		bool bHealProtection = GetUserConfiguration()->GetBool(skCryptDec("Protection"), skCryptDec("Heal"), false);
		bool bPriestPartyBuff = GetUserConfiguration()->GetBool(skCryptDec("Priest"), skCryptDec("PartyBuff"), false);

		std::vector<int> vecFilteredCharacterSkillList;

		std::copy_if(vecCharacterSkillList.begin(), vecCharacterSkillList.end(),
			std::back_inserter(vecFilteredCharacterSkillList),
			[&](const int& iSkillID)
			{
				auto pSkillData = pSkillTable->find(iSkillID);

				if (pSkillData == pSkillTable->end())
					return false;

				if (IsSkillHasZoneLimit(iSkillID))
					return false;

				float fCurrentTime = Bot::TimeGet();
				float fSkillNextUseTime = GetSkillNextUseTime(iSkillID);

				if (fCurrentTime < fSkillNextUseTime)
					return false;

				if (m_bSkillCasting == true && pSkillData->second.iReCastTime > 0)
					return false;

				if (IsRouting()
					&& pSkillData->second.iBaseId != 107725		//Light Feet
					&& pSkillData->second.iBaseId != 107010		//Swift
					&& pSkillData->second.iBaseId != 101001		//Sprint
					&& pSkillData->second.iBaseId != 490811)	//Light Feet [Tournament Use Only]
				{
					return false;
				}

				if (pSkillData->second.iBaseId == 107010)		//Swift				
				{
					if (m_vecPartyMembers.size() > 0
						&& bPartySwift)
					{
						if (PartyMemberNeedSwift() == -1)
							return false;
					}
					else
					{
						auto pSkillExtension4Data = pSkillExtension4->find(pSkillData->second.iID);

						if (pSkillExtension4Data != pSkillExtension4->end())
						{
							if (IsBuffActive(pSkillExtension4Data->second.iBuffType))
								return false;
						}
					}
				}
				else if ((
					pSkillData->second.iBaseId == 111545 ||		//Superior Healing
					pSkillData->second.iBaseId == 111536 ||		//Massive Healing
					pSkillData->second.iBaseId == 111527 ||		//Great Healing
					pSkillData->second.iBaseId == 111518 ||		//Major Healing
					pSkillData->second.iBaseId == 111509 ||		//Healing
					pSkillData->second.iBaseId == 107705))		//Minor Healing
					
				{
					if (m_vecPartyMembers.size() > 0 
						&& bPriestPartyHeal)
					{
						if (PartyMemberNeedHeal(pSkillData->second.iBaseId) == -1)
							return false;
					}
					else
					{
						if (bHealProtection)
						{
							int iHealProtectionValue = GetUserConfiguration()->GetInt(skCryptDec("Protection"), skCryptDec("HealValue"), 75);

							int32_t iHpProtectionPercent = (int32_t)std::ceil((m_PlayerMySelf.iHP * 100) / m_PlayerMySelf.iHPMax);

							if (iHealProtectionValue < iHpProtectionPercent)
								return false;
						}
						else
						{
							auto pSkillExtension3Data = pSkillExtension3->find(pSkillData->second.iID);

							if (pSkillExtension3Data != pSkillExtension3->end())
							{
								int32_t iNeedHealValue = m_PlayerMySelf.iHPMax - m_PlayerMySelf.iHP;

								if (iNeedHealValue <= 0 
									|| iNeedHealValue < pSkillExtension3Data->second.iHealValue)
									return false;
							}
						}
					}
				}
				else if ((
					pSkillData->second.iBaseId == 112675 ||		//Superioris
					pSkillData->second.iBaseId == 112670 ||		//Imposingness
					pSkillData->second.iBaseId == 111657 ||		//massiveness
					pSkillData->second.iBaseId == 111655 ||		//Heapness
					pSkillData->second.iBaseId == 111654 ||		//Undying
					pSkillData->second.iBaseId == 111642 ||		//Mightness
					pSkillData->second.iBaseId == 111633 ||		//Hardness
					pSkillData->second.iBaseId == 111624 ||		//Strong
					pSkillData->second.iBaseId == 111615 ||		//Brave
					pSkillData->second.iBaseId == 111606 ||		//Grace

					pSkillData->second.iBaseId == 112674 ||		//Insensibility Guard
					pSkillData->second.iBaseId == 111660 ||		//Insensibility Peel
					pSkillData->second.iBaseId == 111651 ||		//Insensibility Protector
					pSkillData->second.iBaseId == 111639 ||		//Insensibility barrier
					pSkillData->second.iBaseId == 111630 ||		//Insensibility shield
					pSkillData->second.iBaseId == 111621 ||		//Insensibility armor
					pSkillData->second.iBaseId == 111612 ||		//Insensibility shell
					pSkillData->second.iBaseId == 111603 ||		//Insensibility Skin

					pSkillData->second.iBaseId == 111645 ||		//Fresh mind
					pSkillData->second.iBaseId == 111636 ||		//Calm mind
					pSkillData->second.iBaseId == 111627 ||		//Bright mind
					pSkillData->second.iBaseId == 111609))		//Resist all
				{
					if (m_vecPartyMembers.size() > 0
						&& bPriestPartyBuff)
					{
						if (PartyMemberNeedBuff(pSkillData->second.iBaseId) == -1)
							return false;
					}
					else
					{
						auto pSkillExtension4Data = pSkillExtension4->find(pSkillData->second.iID);

						if (pSkillExtension4Data != pSkillExtension4->end())
						{
							if (IsBuffActive(pSkillExtension4Data->second.iBuffType))
								return false;
						}
					}
				}
				else
				{
					auto pSkillExtension4Data = pSkillExtension4->find(pSkillData->second.iID);

					if (pSkillExtension4Data != pSkillExtension4->end())
					{
						if (IsBuffActive(pSkillExtension4Data->second.iBuffType))
							return false;
					}
				}

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

				if (m_PlayerMySelf.iMSP >= pSkillData->second.iExhaustMSP && (iNeedItem == 0 || (iNeedItem != 0 && iExistItemCount >= iNeedItemCount)))
					return true;
				else
					return false;
			});

		if (vecFilteredCharacterSkillList.size() > 0)
		{
			int iCharacterSkillID = vecFilteredCharacterSkillList.front();

			auto pSkillData = pSkillTable->find(iCharacterSkillID);

			if (pSkillData == pSkillTable->end())
				return;

			bool bUseSkillWithPacket = GetUserConfiguration()->GetBool(skCryptDec("Skill"), skCryptDec("UseSkillWithPacket"), false);
			bool bOnlyAttackSkillUseWithPacket = GetUserConfiguration()->GetBool(skCryptDec("Skill"), skCryptDec("OnlyAttackSkillUseWithPacket"), false);

			if (pSkillData->second.iBaseId == 107010			//Swift
				&& m_vecPartyMembers.size() > 0
				&& bPartySwift)
			{
				int32_t iTargetID = PartyMemberNeedSwift();

				if (iTargetID != -1)
				{
					if (!bOnlyAttackSkillUseWithPacket
						&& bUseSkillWithPacket)
					{
						new std::thread([=]() { UseSkillWithPacket(pSkillData->second, iTargetID); });
					}
					else
					{
						UseSkill(pSkillData->second, iTargetID, 1);
					}
				}
			}
			else if ((
				pSkillData->second.iBaseId == 111545 ||			//Superior Healing
				pSkillData->second.iBaseId == 111536 ||			//Massive Healing
				pSkillData->second.iBaseId == 111527 ||			//Great Healing
				pSkillData->second.iBaseId == 111518 ||			//Major Healing
				pSkillData->second.iBaseId == 111509 ||			//Healing
				pSkillData->second.iBaseId == 107705)			//Minor Healing
				&& m_vecPartyMembers.size() > 0
				&& bPriestPartyHeal)
			{
				int32_t iTargetID = PartyMemberNeedHeal(pSkillData->second.iBaseId);

				if (iTargetID != -1)
				{
					if (!bOnlyAttackSkillUseWithPacket
						&& bUseSkillWithPacket)
					{
						new std::thread([=]() { UseSkillWithPacket(pSkillData->second, iTargetID); });
					}
					else
					{
						UseSkill(pSkillData->second, iTargetID, 1);
					}
				}
			}
			else if ((
					pSkillData->second.iBaseId == 112675 ||		//Superioris
					pSkillData->second.iBaseId == 112670 ||		//Imposingness
					pSkillData->second.iBaseId == 111657 ||		//massiveness
					pSkillData->second.iBaseId == 111655 ||		//Heapness
					pSkillData->second.iBaseId == 111654 ||		//Undying
					pSkillData->second.iBaseId == 111642 ||		//Mightness
					pSkillData->second.iBaseId == 111633 ||		//Hardness
					pSkillData->second.iBaseId == 111624 ||		//Strong
					pSkillData->second.iBaseId == 111615 ||		//Brave
					pSkillData->second.iBaseId == 111606 ||		//Grace

					pSkillData->second.iBaseId == 112674 ||		//Insensibility Guard
					pSkillData->second.iBaseId == 111660 ||		//Insensibility Peel
					pSkillData->second.iBaseId == 111651 ||		//Insensibility Protector
					pSkillData->second.iBaseId == 111639 ||		//Insensibility barrier
					pSkillData->second.iBaseId == 111630 ||		//Insensibility shield
					pSkillData->second.iBaseId == 111621 ||		//Insensibility armor
					pSkillData->second.iBaseId == 111612 ||		//Insensibility shell
					pSkillData->second.iBaseId == 111603 ||		//Insensibility Skin

					pSkillData->second.iBaseId == 111645 ||		//Fresh mind
					pSkillData->second.iBaseId == 111636 ||		//Calm mind
					pSkillData->second.iBaseId == 111627 ||		//Bright mind
					pSkillData->second.iBaseId == 111609)		//Resist all
					&& m_vecPartyMembers.size() > 0
					&& bPriestPartyBuff)
			{
				int32_t iTargetID = PartyMemberNeedBuff(pSkillData->second.iBaseId);

				if (iTargetID != -1)
				{
					if (!bOnlyAttackSkillUseWithPacket
						&& bUseSkillWithPacket)
					{
						new std::thread([=]() { UseSkillWithPacket(pSkillData->second, iTargetID); });
					}
					else
					{
						UseSkill(pSkillData->second, iTargetID, 1);
					}
				}
			}
			else 
			{
				if (pSkillData->second.iID == 490803 || pSkillData->second.iID == 490811)
				{
					SendStartSkillMagicAtTargetPacket(pSkillData->second, m_PlayerMySelf.iID, Vector3(0.0f, 0.0f, 0.0f));
				}
				else
				{
					if (!bOnlyAttackSkillUseWithPacket
						&& bUseSkillWithPacket)
					{
						new std::thread([=]() { UseSkillWithPacket(pSkillData->second, m_PlayerMySelf.iID); });
					}
					else
					{
						UseSkill(pSkillData->second, m_PlayerMySelf.iID, 1);
					}
				}
			}
		}

		m_fLastCharacterProcessTime = Bot::TimeGet();
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

void ClientHandler::PartySwiftProcess()
{
	bool bPartySwift = GetUserConfiguration()->GetBool(skCryptDec("Rogue"), skCryptDec("PartySwift"), false);

	if (!bPartySwift)
		return;

	bool bCharacterStatus = GetUserConfiguration()->GetBool(skCryptDec("Automation"), skCryptDec("Character"), false);

	if (!bCharacterStatus)
		return;

	if (IsZoneChanging())
		return;

	if (IsBlinking())
		return;

	if (m_PlayerMySelf.eState == PSA_DEATH)
		return;

	if (GetActionState() == PSA_SPELLMAGIC)
		return;

	if (m_vecPartyMembers.size() == 0)
		return;

	std::vector<__TABLE_UPC_SKILL>* vecAvailableSkills;
	if (!GetAvailableSkill(&vecAvailableSkills))
		return;

	auto it = std::find_if(vecAvailableSkills->begin(), vecAvailableSkills->end(),
		[&](const TABLE_UPC_SKILL& a) { return a.iBaseId == 107010; });

	bool bUseSkillWithPacket = GetUserConfiguration()->GetBool(skCryptDec("Skill"), skCryptDec("UseSkillWithPacket"), false);
	bool bOnlyAttackSkillUseWithPacket = GetUserConfiguration()->GetBool(skCryptDec("Skill"), skCryptDec("OnlyAttackSkillUseWithPacket"), false);

	if (it != vecAvailableSkills->end())
	{
		
	}
}

void ClientHandler::PriestCharacterProcess(int32_t iTargetID, bool bIsPartyRequest, uint16_t iMaxHp, uint16_t iHp)
{
	/*std::vector<__TABLE_UPC_SKILL>* vecAvailableSkills;
	if (!GetAvailableSkill(&vecAvailableSkills))
		return;

	if (iTargetID == -1)
		iTargetID = GetID();

	if (iTargetID == GetID())
	{
		iMaxHp = GetMaxHp();
		iHp = GetHp();
	}

	bool bUseSkillWithPacket = GetUserConfiguration()->GetBool(skCryptDec("Skill"), skCryptDec("UseSkillWithPacket"), false);

	bool bOnlyAttackSkillUseWithPacket = GetUserConfiguration()->GetBool(skCryptDec("Skill"), skCryptDec("OnlyAttackSkillUseWithPacket"), false);

	bool bAutoHealthBuff = GetUserConfiguration()->GetBool(skCryptDec("Priest"), skCryptDec("AutoHealthBuff"), false);

	int iSelectedHealthBuff = Drawing::Bot->GetUserConfiguration()->GetInt(skCryptDec("Priest"), skCryptDec("SelectedHealthBuff"), -1);

	if (iSelectedHealthBuff == -1)
	{
		iSelectedHealthBuff = GetProperHealthBuff(iMaxHp);
	}

	if (bAutoHealthBuff && iSelectedHealthBuff != -1)
	{
		auto it = std::find_if(vecAvailableSkills->begin(), vecAvailableSkills->end(),
			[&](const TABLE_UPC_SKILL& a) { return a.iBaseId == iSelectedHealthBuff; });

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

			if (IsBlinking(iTargetID))
			{
				bUse = false;
			}

			if (GetMp() < it->iExhaustMSP)
			{
				bUse = false;
			}

			if (!bIsPartyRequest && IsBuffActive(BuffType::BUFF_TYPE_HP_MP))
			{
				bUse = false;
			}

			if (bIsPartyRequest)
			{
				auto pMember = std::find_if(m_vecPartyMembers.begin(), m_vecPartyMembers.end(),
					[&](const PartyMember& a)
					{
						return a.iMemberID == iTargetID;
					});

				if (pMember != m_vecPartyMembers.end())
				{
					if ((pMember->iHpBuffTime.count() + 607000) > msNow.count())
						bUse = false;
				}

				DWORD iPlayerBase = GetEntityBase(iTargetID);

				if (iPlayerBase == 0)
					bUse = false;
				else
				{
					auto iPlayerPosition = Vector3(
						ReadFloat(iPlayerBase + GetAddress("KO_OFF_X")),
						ReadFloat(iPlayerBase + GetAddress("KO_OFF_Z")),
						ReadFloat(iPlayerBase + GetAddress("KO_OFF_Y")));

					if (GetDistance(iPlayerPosition) > 25.0f 
						|| pMember->iHP == 0)
					{
						bUse = false;
					}
				}
			}

			if (bUse)
			{
#ifdef DEBUG
				printf("HP Buff Using Target -> %d\n", iTargetID);
#endif
				bool bUsed = true;

				if (!bOnlyAttackSkillUseWithPacket 
					&& bUseSkillWithPacket)
				{
					if (m_threadSkillPacket && m_threadSkillPacket->joinable())
						bUsed = false;
					else
						UseSkillWithPacket(*it, iTargetID);
				}
				else
				{
					bUsed = UseSkill(*it, iTargetID, 1);
				}

				if (bUsed)
				{
					if (bIsPartyRequest)
					{
						auto pMember = std::find_if(m_vecPartyMembers.begin(), m_vecPartyMembers.end(),
							[&](const PartyMember& a)
							{
								return a.iMemberID == iTargetID;
							});

						if (pMember != m_vecPartyMembers.end())
						{
							pMember->iHpBuffTime = duration_cast<std::chrono::milliseconds>(
								std::chrono::system_clock::now().time_since_epoch()
							);
#ifdef DEBUG
							printf("HP Buff Using Target -> %d - Time: %lld\n", iTargetID, pMember->iHpBuffTime.count());
#endif
						}
					}
				}
			}
		}
	}

	bool bAutoDefenceBuff = GetUserConfiguration()->GetBool(skCryptDec("Priest"), skCryptDec("AutoDefenceBuff"), false);

	int iSelectedDefenceBuff = Drawing::Bot->GetUserConfiguration()->GetInt(skCryptDec("Priest"), skCryptDec("SelectedDefenceBuff"), -1);

	if (iSelectedDefenceBuff == -1)
	{
		iSelectedDefenceBuff = GetProperDefenceBuff();
	}

	if (bAutoDefenceBuff && iSelectedDefenceBuff != -1)
	{
		auto it = std::find_if(vecAvailableSkills->begin(), vecAvailableSkills->end(),
			[&](const TABLE_UPC_SKILL& a) { return a.iBaseId == iSelectedDefenceBuff; });

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

			if (IsBlinking(iTargetID))
			{
				bUse = false;
			}

			if (GetMp() < it->iExhaustMSP)
			{
				bUse = false;
			}

			if (!bIsPartyRequest && IsBuffActive(BuffType::BUFF_TYPE_AC))
			{
				bUse = false;
			}

			if (bIsPartyRequest)
			{
				auto pMember = std::find_if(m_vecPartyMembers.begin(), m_vecPartyMembers.end(),
					[&](const PartyMember& a)
					{
						return a.iMemberID == iTargetID;
					});

				if (pMember != m_vecPartyMembers.end())
				{
					if ((pMember->iACBuffTime.count() + 607000) > msNow.count())
						bUse = false;
				}

				DWORD iPlayerBase = GetEntityBase(iTargetID);

				if (iPlayerBase == 0)
					bUse = false;
				else
				{
					auto iPlayerPosition = Vector3(
						ReadFloat(iPlayerBase + GetAddress("KO_OFF_X")),
						ReadFloat(iPlayerBase + GetAddress("KO_OFF_Z")),
						ReadFloat(iPlayerBase + GetAddress("KO_OFF_Y")));

					if (GetDistance(iPlayerPosition) > 25.0f 
						|| pMember->iHP == 0)
					{
						bUse = false;
					}
				}
			}

			if (bUse)
			{
#ifdef DEBUG
				printf("AC Buff Using Target -> %d\n", iTargetID);
#endif
				bool bUsed = true;

				if (!bOnlyAttackSkillUseWithPacket
					&& bUseSkillWithPacket)
				{
					if (m_threadSkillPacket && m_threadSkillPacket->joinable())
						bUsed = false;
					else
						UseSkillWithPacket(*it, iTargetID);
				}
				else
				{
					bUsed = UseSkill(*it, iTargetID, 1);
				}

				if (bUsed)
				{
					if (bIsPartyRequest)
					{
						auto pMember = std::find_if(m_vecPartyMembers.begin(), m_vecPartyMembers.end(),
							[&](const PartyMember& a)
							{
								return a.iMemberID == iTargetID;
							});

						if (pMember != m_vecPartyMembers.end())
						{
							pMember->iACBuffTime = duration_cast<std::chrono::milliseconds>(
								std::chrono::system_clock::now().time_since_epoch()
							);
#ifdef DEBUG
							printf("AC Buff Using Target -> %d - Time: %lld\n", iTargetID, pMember->iACBuffTime.count());
#endif
						}
					}
				}			
			}
		}
	}

	bool bAutoMindBuff = GetUserConfiguration()->GetBool(skCryptDec("Priest"), skCryptDec("AutoMindBuff"), false);

	int iSelectedMindBuff = Drawing::Bot->GetUserConfiguration()->GetInt(skCryptDec("Priest"), skCryptDec("SelectedMindBuff"), -1);

	if (iSelectedMindBuff == -1)
	{
		iSelectedMindBuff = GetProperMindBuff();
	}

	if (bAutoMindBuff && iSelectedMindBuff != -1)
	{
		auto it = std::find_if(vecAvailableSkills->begin(), vecAvailableSkills->end(),
			[&](const TABLE_UPC_SKILL& a) { return a.iBaseId == iSelectedMindBuff; });

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

			if (IsBlinking(iTargetID))
			{
				bUse = false;
			}

			if (GetMp() < it->iExhaustMSP)
			{
				bUse = false;
			}

			if (!bIsPartyRequest && IsBuffActive(BuffType::BUFF_TYPE_RESISTANCES))
			{
				bUse = false;
			}

			if (bIsPartyRequest)
			{
				auto pMember = std::find_if(m_vecPartyMembers.begin(), m_vecPartyMembers.end(),
					[&](const PartyMember& a)
					{
						return a.iMemberID == iTargetID;
					});

				if (pMember != m_vecPartyMembers.end())
				{
					if ((pMember->iMindBuffTime.count() + 607000) > msNow.count())
						bUse = false;
				}

				DWORD iPlayerBase = GetEntityBase(iTargetID);

				if (iPlayerBase == 0)
					bUse = false;
				else
				{
					auto iPlayerPosition = Vector3(
						ReadFloat(iPlayerBase + GetAddress("KO_OFF_X")),
						ReadFloat(iPlayerBase + GetAddress("KO_OFF_Z")),
						ReadFloat(iPlayerBase + GetAddress("KO_OFF_Y")));

					if (GetDistance(iPlayerPosition) > 25.0f 
						|| pMember->iHP == 0)
					{
						bUse = false;
					}
				}
			}

			if (bUse)
			{
#ifdef DEBUG
				printf("Mind Buff Using Target -> %d\n", iTargetID);
#endif
				bool bUsed = true;

				if (!bOnlyAttackSkillUseWithPacket
					&& bUseSkillWithPacket)
				{
					if (m_threadSkillPacket && m_threadSkillPacket->joinable())
						bUsed = false;
					else
						UseSkillWithPacket(*it, iTargetID);
				}
				else
				{
					bUsed = UseSkill(*it, iTargetID, 1);
				}

				if (bUsed)
				{
					if (bIsPartyRequest)
					{
						auto pMember = std::find_if(m_vecPartyMembers.begin(), m_vecPartyMembers.end(),
							[&](const PartyMember& a)
							{
								return a.iMemberID == iTargetID;
							});

						if (pMember != m_vecPartyMembers.end())
						{
							pMember->iMindBuffTime = duration_cast<std::chrono::milliseconds>(
								std::chrono::system_clock::now().time_since_epoch()
							);
#ifdef DEBUG
							printf("Mind Buff Using Target -> %d - Time: %lld\n", iTargetID, pMember->iMindBuffTime.count());
#endif
						}
					}
				}
			}
		}
	}

	bool bAutoHeal = GetUserConfiguration()->GetBool(skCryptDec("Priest"), skCryptDec("AutoHeal"), false);
	int iSelectedHeal = GetUserConfiguration()->GetInt(skCryptDec("Priest"), skCryptDec("SelectedHeal"), -1);

	if (iSelectedHeal == -1)
	{
		iSelectedHeal = GetProperHeal();
	}

	int iAutoHealValue = GetUserConfiguration()->GetInt(skCryptDec("Priest"), skCryptDec("AutoHealValue"), 75);
	int32_t iHpProtectionPercent = (int32_t)std::ceil((iHp * 100) / iMaxHp);

	if (bAutoHeal && iSelectedHeal != -1 && iHpProtectionPercent <= iAutoHealValue)
	{
		auto it = std::find_if(vecAvailableSkills->begin(), vecAvailableSkills->end(),
			[&](const TABLE_UPC_SKILL& a) { return a.iBaseId == iSelectedHeal; });

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

			if (IsBlinking(iTargetID))
			{
				bUse = false;
			}

			if (GetMp() < it->iExhaustMSP)
			{
				bUse = false;
			}

			if (bIsPartyRequest)
			{
				auto pMember = std::find_if(m_vecPartyMembers.begin(), m_vecPartyMembers.end(),
					[&](const PartyMember& a)
					{
						return a.iMemberID == iTargetID;
					});

				if (pMember != m_vecPartyMembers.end())
				{
					if (pMember->iHP == 0)
						bUse = false;
				}
			}

			DWORD iPlayerBase = GetEntityBase(iTargetID);

			if (iPlayerBase == 0)
				bUse = false;
			else
			{
				auto iPlayerPosition = Vector3(
					ReadFloat(iPlayerBase + GetAddress("KO_OFF_X")),
					ReadFloat(iPlayerBase + GetAddress("KO_OFF_Z")),
					ReadFloat(iPlayerBase + GetAddress("KO_OFF_Y")));

				if (GetDistance(iPlayerPosition) > 25.0f)
				{
					bUse = false;
				}
			}

			if (bUse)
			{
				if (!bOnlyAttackSkillUseWithPacket
					&& bUseSkillWithPacket)
				{
					if (m_threadSkillPacket && m_threadSkillPacket->joinable())
						return;

					UseSkillWithPacket(*it, iTargetID);
				}
				else
				{
					UseSkill(*it, iTargetID, 1);
				}
			}
		}
	}	*/
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

	TItemData tInventoryItem;

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

	TItemData tInventoryItem;

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

//void ClientHandler::RouteProcess()
//{
//#ifdef DEBUG
//	printf("ClientHandler::RouteProcess Started\n");
//#endif
//
//	while (m_bWorking)
//	{
//		try
//		{
//			std::this_thread::sleep_for(std::chrono::milliseconds(100));
//
//			if (IsZoneChanging())
//				continue;
//
//			if (IsBlinking())
//				continue;
//
//			if (IsDeath())
//				continue;
//
//			if (m_vecRoute.size() == 0)
//				continue;
//
//			Route pRoute = m_vecRoute.front();
//
//			if (GetActionState() == PSA_SPELLMAGIC)
//				continue;
//
//			m_iRouteStep = pRoute.eStepType;
//
//			switch (pRoute.eStepType)
//			{
//			case RouteStepType::STEP_MOVE:
//			{
//				if (m_vecRoute.size() == 0)
//					continue;
//
//				float fDistance = GetDistance(Vector3(GetX(), 0.0f, GetY()), Vector3(pRoute.fX, 0.0f, pRoute.fY));
//
//				if (fDistance > 3.0f)
//				{
//					SetMovePosition(Vector3(pRoute.fX, 0.0f, pRoute.fY));
//					continue;
//				}
//			}
//			break;
//
//			case RouteStepType::STEP_TOWN:
//			{
//				if (m_vecRoute.size() == 0)
//					continue;
//
//				float fDistance = GetDistance(Vector3(GetX(), 0.0f, GetY()), Vector3(pRoute.fX, 0.0f, pRoute.fY));
//
//				if (fDistance > 50.0f)
//				{
//					SetMovePosition(Vector3(pRoute.fX, 0.0f, pRoute.fY));
//					continue;
//				}
//
//				SendTownPacket();
//			}
//			break;
//
//			case RouteStepType::STEP_SUNDRIES:
//			case RouteStepType::STEP_POTION:
//			{
//				if (m_vecRoute.size() == 0)
//					continue;
//
//				float fDistance = GetDistance(Vector3(GetX(), 0.0f, GetY()), Vector3(pRoute.fX, 0.0f, pRoute.fY));
//
//				if (fDistance > 3.0f)
//				{
//					SetMovePosition(Vector3(pRoute.fX, 0.0f, pRoute.fY));
//					continue;
//				}
//
//				/*
//				* Find NPC
//				*/
//
//				int iNpcSellingGroup = 253000;
//
//				if (pRoute.eStepType == RouteStepType::STEP_SUNDRIES)
//				{
//					iNpcSellingGroup = 255000;
//				}
//
//				struct SNpcInformation
//				{
//					SNpcInformation(int32_t iNpcID, int32_t iSellingGroup, Vector3 v3NpcPosition) :
//						m_iNpcID(iNpcID), m_iSellingGroup(iSellingGroup), m_v3NpcPosition(v3NpcPosition) {};
//
//					int32_t m_iNpcID;
//					int32_t m_iSellingGroup;
//					Vector3 m_v3NpcPosition;
//				};
//
//				std::vector<SNpcInformation> vecNpcInformation;
//
//				{
//					std::vector<TNpc> tmpVecNpc = m_vecNpc;
//
//					Vector3 v3CurrentPosition = GetPosition();
//
//					auto pSort = [&](TNpc const& a, TNpc const& b)
//					{
//						auto fADistance = GetDistance(v3CurrentPosition, Vector3(a.fX, a.fZ, a.fY));
//						auto fBDistance = GetDistance(v3CurrentPosition, Vector3(b.fX, b.fZ, b.fY));
//
//						if (fADistance != fBDistance)
//						{
//							return fADistance < fBDistance;
//						}
//
//						return false;
//					};
//
//					std::sort(tmpVecNpc.begin(), tmpVecNpc.end(), pSort);
//
//					for (auto& e : tmpVecNpc)
//					{
//						if (e.iSellingGroup == iNpcSellingGroup)
//						{
//							vecNpcInformation.push_back(
//								SNpcInformation(e.iID, iNpcSellingGroup, Vector3(e.fX, e.fZ, e.fY))
//							);
//						}
//					}
//				}
//
//				auto pNpc = std::find_if(vecNpcInformation.begin(), vecNpcInformation.end(),
//					[&](const SNpcInformation& a) {
//						return a.m_iSellingGroup == iNpcSellingGroup;
//					});
//
//				if (pNpc != vecNpcInformation.end())
//				{
//					/*
//					* Move Operations
//					*/
//
//					DWORD iNpcBase = 0;
//
//					while (m_vecRoute.size() > 0)
//					{
//						std::this_thread::sleep_for(std::chrono::milliseconds(100));
//
//						if (GetActionState() == PSA_SPELLMAGIC)
//							continue;
//
//						iNpcBase = GetEntityBase(pNpc->m_iNpcID);
//
//						if (iNpcBase == 0)
//							break;
//
//						float fTargetRadius = GetRadius(iNpcBase) * GetScaleZ(iNpcBase);
//						float fMySelfRadius = GetRadius() * GetScaleZ();
//						float fDistLimit = (fMySelfRadius + fTargetRadius) / 2.0f;
//
//						if (GetDistance(pNpc->m_v3NpcPosition) < 3)
//							break;
//
//						SetMovePosition(pNpc->m_v3NpcPosition);
//					}
//
//					if (iNpcBase != 0)
//					{
//						/**
//						* Selling Operations
//						*/
//
//						bool bAutoSellSlotRange = GetUserConfiguration()->GetBool(skCryptDec("Supply"), skCryptDec("AutoSellSlotRange"), false);
//						int iAutoSellSlotRangeStart = GetUserConfiguration()->GetInt(skCryptDec("Supply"), skCryptDec("AutoSellSlotRangeStart"), 1);
//						int iAutoSellSlotRangeEnd = GetUserConfiguration()->GetInt(skCryptDec("Supply"), skCryptDec("AutoSellSlotRangeEnd"), 14);
//						bool bAutoSellByFlag = GetUserConfiguration()->GetBool(skCryptDec("Supply"), skCryptDec("AutoSellByFlag"), false);
//
//						if (bAutoSellSlotRange || bAutoSellByFlag)
//						{
//							std::vector<TItemData> vecInventoryItemList;
//							GetInventoryItemList(vecInventoryItemList);
//
//							uint8_t iSellPageCount = 0;
//							std::vector<SSItemSell> vecInventoryItemSell[2];
//
//							for (const TItemData& pItem : vecInventoryItemList)
//							{
//								if (pItem.iItemID == 0)
//									continue;
//
//								//Promise Of Training
//								if (pItem.iItemID == 989511000) 
//									continue;
//
//								__TABLE_ITEM* pItemData;
//								if (!m_Bot->GetItemData(pItem.iItemID, pItemData))
//									continue;
//
//								__TABLE_ITEM_EXTENSION* pItemExtensionData;
//								if (!m_Bot->GetItemExtensionData(pItem.iItemID, pItemData->byExtIndex, pItemExtensionData))
//									continue;
//
//								if (pItemData->byNeedRace == RACE_TRADEABLE_IN_72HR
//									|| pItemData->byNeedRace == RACE_NO_TRADE
//									|| pItemData->byNeedRace == RACE_NO_TRADE_SOLD
//									|| pItemData->byNeedRace == RACE_NO_TRADE_SOLD2
//									|| pItemData->byNeedRace == RACE_NO_TRADE_SOLD_STORE)
//									continue;
//
//								if (pItemExtensionData->iItemType == ITEM_ATTRIBUTE_UNIQUE
//									|| pItemExtensionData->iItemType == ITEM_ATTRIBUTE_RED
//									|| pItemExtensionData->iItemType == ITEM_ATTRIBUTE_UPGRADE_REVERSE
//									|| pItemExtensionData->iItemType == ITEM_ATTRIBUTE_UNIQUE_REVERSE
//									|| pItemExtensionData->iItemType == ITEM_ATTRIBUTE_PET
//									|| pItemExtensionData->iItemType == ITEM_ATTRIBUTE_EVENT
//									|| pItemExtensionData->iItemType == ITEM_ATTRIBUTE_COSPRE
//									|| pItemExtensionData->iItemType == ITEM_ATTRIBUTE_MINERVA)
//									continue;
//
//								int iRangeStart = (14 + iAutoSellSlotRangeStart) - 1;
//								int iRangeEnd = (14 + iAutoSellSlotRangeEnd) - 1;
//
//								uint8_t iItemFlag = m_Bot->GetInventoryItemFlag(pItem.iItemID);
//
//								if ((bAutoSellByFlag
//									&& iItemFlag == INVENTORY_ITEM_FLAG_SELL)
//									|| (bAutoSellSlotRange
//										&& pItem.iPos >= iRangeStart
//										&& pItem.iPos <= iRangeEnd))
//								{
//									if (vecInventoryItemSell[iSellPageCount].size() == 14)
//										iSellPageCount++;
//
//									vecInventoryItemSell[iSellPageCount].push_back(SSItemSell(pItem.iItemID, (uint8_t)(pItem.iPos - 14), pItem.iCount));
//								}
//							}
//
//							if (vecInventoryItemSell[0].size() > 0)
//							{
//								SendNpcEvent(pNpc->m_iNpcID);
//
//								std::this_thread::sleep_for(std::chrono::milliseconds(1000));
//
//								for (size_t i = 0; i <= iSellPageCount; i++)
//								{
//									WaitConditionWithTimeout(m_bVipWarehouseLoaded == false, 3000);
//									SendItemTradeSell(iNpcSellingGroup, pNpc->m_iNpcID, vecInventoryItemSell[i]);
//									Write4Byte(GetAddress(skCryptDec("KO_PTR_UI_LOCK")), 1);
//								}
//
//								WaitConditionWithTimeout(m_bVipWarehouseLoaded == false, 3000);
//
//								SendShoppingMall(ShoppingMallType::STORE_CLOSE);
//
//								std::this_thread::sleep_for(std::chrono::milliseconds(1000));
//							}
//						}
//
//						/**
//						* VIP Warehouse Operations
//						*/
//
//						bool bAutoSellVipSlotRange = GetUserConfiguration()->GetBool(skCryptDec("Supply"), skCryptDec("AutoSellVipSlotRange"), false);
//
//						if (bAutoSellVipSlotRange)
//						{
//							bool bRunWarehouseProcess = true;
//
//							while (bRunWarehouseProcess)
//							{
//								if (m_bVipWarehouseInitialized
//									&& !m_bVipWarehouseEnabled)
//									break;
//
//								if (m_vecRoute.size() == 0)
//									break;
//
//								std::this_thread::sleep_for(std::chrono::milliseconds(100));
//
//								OpenVipWarehouse();
//
//								WaitConditionWithTimeout(m_bVipWarehouseLoaded == false, 3000);
//
//								if (!m_bVipWarehouseLoaded)
//									break;
//
//								bool bAutoSellByFlag = GetUserConfiguration()->GetBool(skCryptDec("Supply"), skCryptDec("AutoSellByFlag"), false);
//								int iAutoSellVipSlotRangeStart = GetUserConfiguration()->GetInt(skCryptDec("Supply"), skCryptDec("AutoSellVipSlotRangeStart"), 1);
//								int iAutoSellVipSlotRangeEnd = GetUserConfiguration()->GetInt(skCryptDec("Supply"), skCryptDec("AutoSellVipSlotRangeEnd"), 48);
//
//								std::vector<TItemData> vecVipWarehouseItemList;
//								std::vector<TItemData> vecVipWarehouseGetOutItemList;
//
//								GetVipWarehouseItemList(vecVipWarehouseItemList);
//
//								for (const TItemData& pItem : vecVipWarehouseItemList)
//								{
//									if (pItem.iItemID == 0)
//										continue;
//
//									int iRangeStart = (iAutoSellVipSlotRangeStart)-1;
//									int iRangeEnd = (iAutoSellVipSlotRangeEnd)-1;
//
//									uint8_t iItemFlag = m_Bot->GetInventoryItemFlag(pItem.iItemID);
//
//									if (((bAutoSellByFlag
//										&& iItemFlag == INVENTORY_ITEM_FLAG_SELL))
//										|| (bAutoSellVipSlotRange
//											&& pItem.iPos >= iRangeStart
//											&& pItem.iPos <= iRangeEnd))
//									{
//										vecVipWarehouseGetOutItemList.push_back(pItem);
//									}
//								}
//
//								if (vecVipWarehouseGetOutItemList.size() == 0)
//								{
//									CloseVipWarehouse();
//									break;
//								}	
//
//								uint8_t iVipWarehouseSellPageCount = 0;
//								std::vector<SSItemSell> vecVipWarehouseItemSell[2];
//
//								for (const TItemData& pItem : vecVipWarehouseGetOutItemList)
//								{
//									__TABLE_ITEM* pItemData;
//									if (!m_Bot->GetItemData(pItem.iItemID, pItemData))
//										continue;
//
//									WaitConditionWithTimeout(Read4Byte(GetAddress(skCryptDec("KO_PTR_UI_LOCK"))) == 1, 3000);
//
//									std::vector<TItemData> vecVipWarehouseInventoryItemList;
//									GetVipWarehouseInventoryItemList(vecVipWarehouseInventoryItemList);
//
//									int iTargetPosition = -1;
//
//									if (pItemData->byContable)
//									{
//										auto pInventoryItem = std::find_if(vecVipWarehouseInventoryItemList.begin(), vecVipWarehouseInventoryItemList.end(),
//											[&](const TItemData& a)
//											{
//												return a.iItemID == pItem.iItemID;
//											});
//
//										if (pInventoryItem != vecVipWarehouseInventoryItemList.end())
//										{
//											if ((pItem.iCount + pInventoryItem->iCount) > 9999)
//												continue;
//
//											iTargetPosition = pInventoryItem->iPos;
//										}
//										else
//										{
//											auto pInventoryEmptySlot = std::find_if(vecVipWarehouseInventoryItemList.begin(), vecVipWarehouseInventoryItemList.end(),
//												[&](const TItemData& a)
//												{
//													return a.iItemID == 0;
//												});
//
//											if (pInventoryEmptySlot != vecVipWarehouseInventoryItemList.end())
//											{
//												iTargetPosition = pInventoryEmptySlot->iPos;
//											}
//										}
//									}
//									else
//									{
//										auto pInventoryEmptySlot = std::find_if(vecVipWarehouseInventoryItemList.begin(), vecVipWarehouseInventoryItemList.end(),
//											[&](const TItemData& a)
//											{
//												return a.iItemID == 0;
//											});
//
//										if (pInventoryEmptySlot != vecVipWarehouseInventoryItemList.end())
//										{
//											iTargetPosition = pInventoryEmptySlot->iPos;
//										}
//									}
//
//									if (iTargetPosition == -1)
//									{
//										bRunWarehouseProcess = false;
//										break;
//									}
//
//									WaitConditionWithTimeout(Read4Byte(GetAddress(skCryptDec("KO_PTR_UI_LOCK"))) == 1, 3000);
//
//									Write4Byte(GetAddress(skCryptDec("KO_PTR_UI_WND")), 14);
//									Write4Byte(GetAddress(skCryptDec("KO_PTR_UI_SLOT_ORDER")), pItem.iPos);
//									Write4Byte(GetAddress(skCryptDec("KO_PTR_UI_WND_DISTRICT")), 2);
//									Write4Byte(GetAddress(skCryptDec("KO_PTR_SELECTED_ITEM_BASE")), pItem.iBase);
//
//									VipWarehouseGetOut(iTargetPosition);
//
//									WaitConditionWithTimeout(Read4Byte(GetAddress(skCryptDec("KO_PTR_UI_LOCK"))) == 1, 3000);
//
//									if (pItemData->byContable)
//									{
//										CountableDialogChangeCount(1);
//										CountableDialogChangeCount(pItem.iCount);
//										AcceptCountableDialog();
//									}
//
//									if (vecVipWarehouseItemSell[iVipWarehouseSellPageCount].size() == 14)
//										iVipWarehouseSellPageCount++;
//
//									vecVipWarehouseItemSell[iVipWarehouseSellPageCount].push_back(SSItemSell(pItem.iItemID, (uint8_t)(iTargetPosition), pItem.iCount));
//								}
//
//								WaitConditionWithTimeout(Read4Byte(GetAddress(skCryptDec("KO_PTR_UI_LOCK"))) == 1, 3000);
//
//								CloseVipWarehouse();
//
//								if (vecVipWarehouseItemSell[0].size() > 0)
//								{
//									SendNpcEvent(pNpc->m_iNpcID);
//
//									std::this_thread::sleep_for(std::chrono::milliseconds(1000));
//
//									for (size_t i = 0; i <= iVipWarehouseSellPageCount; i++)
//									{
//										WaitConditionWithTimeout(m_bVipWarehouseLoaded == false, 3000);
//										SendItemTradeSell(iNpcSellingGroup, pNpc->m_iNpcID, vecVipWarehouseItemSell[i]);
//										Write4Byte(GetAddress(skCryptDec("KO_PTR_UI_LOCK")), 1);
//									}
//
//									WaitConditionWithTimeout(m_bVipWarehouseLoaded == false, 3000);
//
//									SendShoppingMall(ShoppingMallType::STORE_CLOSE);
//
//									std::this_thread::sleep_for(std::chrono::milliseconds(1000));
//								}
//							}
//						}
//
//						/**
//						* Buy Operations
//						*/
//
//						bool bAutoSupply = GetUserConfiguration()->GetBool(skCryptDec("Supply"), skCryptDec("AutoSupply"), false);
//
//						if (bAutoSupply)
//						{
//							auto jSupplyList = m_Bot->GetSupplyList();
//
//							uint8_t iBuyPageCount = 0;
//							std::vector<SSItemBuy> vecInventoryItemBuy[2];
//							std::vector<int32_t> vecExceptedBuyPos;
//
//							for (size_t i = 0; i < jSupplyList.size(); i++)
//							{
//								std::vector<int> vecSupplyList = GetUserConfiguration()->GetInt(skCryptDec("Supply"), skCryptDec("Enable"), std::vector<int>());
//
//								std::string szItemIdAttribute = skCryptDec("itemid");
//								std::string szSellingGroupAttribute = skCryptDec("sellinggroup");
//								std::string szCountAttribute = skCryptDec("count");
//
//								int32_t iItemId = jSupplyList[i][szItemIdAttribute.c_str()].get<int32_t>();
//								int32_t iSellingGroup = jSupplyList[i][szSellingGroupAttribute.c_str()].get<int32_t>();
//
//								bool bSelected = std::find(vecSupplyList.begin(), vecSupplyList.end(), iItemId) != vecSupplyList.end();
//
//								if (!bSelected)
//									continue;
//
//								if (iSellingGroup != iNpcSellingGroup)
//									continue;
//
//								std::vector<SShopItem> vecShopItemTable;
//								if (!m_Bot->GetShopItemTable(iNpcSellingGroup, vecShopItemTable))
//									continue;
//
//								uint8_t iPos = -1;
//								int16_t iItemCount = (int16_t)GetUserConfiguration()->GetInt(
//									skCryptDec("Supply"),
//									std::to_string(iItemId).c_str(),
//									jSupplyList[i][szCountAttribute.c_str()].get<int16_t>());
//
//								TItemData pInventoryItem = GetInventoryItem(iItemId);
//
//								if (pInventoryItem.iItemID != 0)
//								{
//									if (pInventoryItem.iCount >= iItemCount)
//										continue;
//
//									iPos = (uint8_t)pInventoryItem.iPos;
//									iItemCount = (int16_t)std::abs(pInventoryItem.iCount - iItemCount);
//								}
//								else
//								{
//									iPos = (uint8_t)GetInventoryEmptySlot(vecExceptedBuyPos);
//								}
//
//								if (iPos == -1 || iItemCount == 0)
//									continue;
//
//								auto pShopItem = std::find_if(vecShopItemTable.begin(), vecShopItemTable.end(),
//									[iItemId](const SShopItem& a) { return a.m_iItemId == iItemId; });
//
//								if (pShopItem == vecShopItemTable.end())
//									continue;
//
//								if (vecInventoryItemBuy[iBuyPageCount].size() == 14)
//									iBuyPageCount++;
//
//								vecInventoryItemBuy[iBuyPageCount].push_back(
//									SSItemBuy(pShopItem->m_iItemId, (iPos - 14), iItemCount, pShopItem->m_iPage, pShopItem->m_iPos));
//
//								vecExceptedBuyPos.push_back(iPos);
//							}
//
//							if (vecInventoryItemBuy[0].size() > 0)
//							{
//								SendNpcEvent(pNpc->m_iNpcID);
//
//								std::this_thread::sleep_for(std::chrono::milliseconds(1000));
//
//								for (size_t i = 0; i <= iBuyPageCount; i++)
//								{
//									WaitConditionWithTimeout(m_bVipWarehouseLoaded == false, 3000);
//									SendItemTradeBuy(iNpcSellingGroup, pNpc->m_iNpcID, vecInventoryItemBuy[i]);
//									Write4Byte(GetAddress(skCryptDec("KO_PTR_UI_LOCK")), 1);
//								}
//
//								WaitConditionWithTimeout(m_bVipWarehouseLoaded == false, 3000);
//
//								SendShoppingMall(ShoppingMallType::STORE_CLOSE);
//
//								std::this_thread::sleep_for(std::chrono::milliseconds(1000));
//							}
//						}
//
//						/**
//						* Repair Operations
//						*/
//
//						bool bAutoRepair = GetUserConfiguration()->GetBool(skCryptDec("Supply"), skCryptDec("AutoRepair"), false);
//
//						if (bAutoRepair 
//							&& pRoute.eStepType == RouteStepType::STEP_SUNDRIES)
//						{
//							SendNpcEvent(pNpc->m_iNpcID);
//
//							std::this_thread::sleep_for(std::chrono::milliseconds(1000));
//
//							for (uint8_t i = 0; i < SLOT_MAX + HAVE_MAX; i++)
//							{
//								switch (i)
//								{
//									// Equipment District
//									case 1:
//									case 4:
//									case 6:
//									case 8:
//									case 10:
//									case 12:
//									case 13:
//									{
//										TItemData pInventory = GetInventoryItemSlot(i);
//
//										if (pInventory.iItemID == 0)
//											continue;
//
//										SendItemRepair(1, (uint8_t)pInventory.iPos, pNpc->m_iNpcID, pInventory.iItemID);
//									}
//									break;
//
//									// Inventory District
//									default:
//									{
//										//TODO: Inventory Repair Operations
//									}
//									break;
//								}
//							}
//
//							SendShoppingMall(ShoppingMallType::STORE_CLOSE);
//							std::this_thread::sleep_for(std::chrono::milliseconds(1000));
//						}
//					}
//				}
//
//				m_msLastSupplyTime = duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
//			}
//			break;
//
//			case RouteStepType::STEP_INN:
//			{
//				std::vector<TItemData> vecInventoryItemList;
//				std::vector<TItemData> vecFlaggedItemList;
//
//				if (GetInventoryItemList(vecInventoryItemList))
//				{
//					for (const TItemData& pItem : vecInventoryItemList)
//					{
//						uint8_t iItemFlag = m_Bot->GetInventoryItemFlag(pItem.iItemID);
//
//						if (iItemFlag != INVENTORY_ITEM_FLAG_INN)
//						{
//							continue;
//						}
//
//						vecFlaggedItemList.push_back(pItem);
//					}
//				}
//
//				if (vecFlaggedItemList.size() > 0)
//				{
//					float fDistance = GetDistance(Vector3(GetX(), 0.0f, GetY()), Vector3(pRoute.fX, 0.0f, pRoute.fY));
//
//					if (fDistance > 3.0f)
//					{
//						SetMovePosition(Vector3(pRoute.fX, 0.0f, pRoute.fY));
//						continue;
//					}
//
//					struct SNpcInformation
//					{
//						SNpcInformation(int32_t iNpcID, int32_t iFamilyType, Vector3 v3NpcPosition) :
//							m_iNpcID(iNpcID), m_iFamilyType(iFamilyType), m_v3NpcPosition(v3NpcPosition) {};
//
//						int32_t m_iNpcID;
//						int32_t m_iFamilyType;
//						Vector3 m_v3NpcPosition;
//					};
//
//					SNpcInformation* pWarehouseNpcInfo = nullptr;
//
//					{
//						std::vector<TNpc> tmpVecNpc = m_vecNpc;
//
//						Vector3 v3CurrentPosition = GetPosition();
//
//						auto pSort = [&](TNpc const& a, TNpc const& b)
//						{
//							auto fADistance = GetDistance(v3CurrentPosition, Vector3(a.fX, a.fZ, a.fY));
//							auto fBDistance = GetDistance(v3CurrentPosition, Vector3(b.fX, b.fZ, b.fY));
//
//							if (fADistance != fBDistance)
//							{
//								return fADistance < fBDistance;
//							}
//
//							return false;
//						};
//
//						std::sort(tmpVecNpc.begin(), tmpVecNpc.end(), pSort);
//
//						for (auto& e : tmpVecNpc)
//						{
//							if (e.iFamilyType == 31)
//							{
//								pWarehouseNpcInfo = new SNpcInformation(e.iID, e.iFamilyType, Vector3(e.fX, e.fZ, e.fY));
//								break;
//							}
//						}
//					}
//
//					if (pWarehouseNpcInfo)
//					{
//						while (
//							m_vecRoute.size() > 0)
//						{
//							std::this_thread::sleep_for(std::chrono::milliseconds(100));
//
//							DWORD iNpcBase = GetEntityBase(pWarehouseNpcInfo->m_iNpcID);
//
//							if (iNpcBase == 0)
//								break;
//
//							if (GetActionState() == PSA_SPELLMAGIC)
//								continue;
//
//							float fTargetRadius = GetRadius(iNpcBase) * GetScaleZ(iNpcBase);
//							float fMySelfRadius = GetRadius() * GetScaleZ();
//							float fDistLimit = (fMySelfRadius + fTargetRadius) / 2.0f;
//
//							if (GetDistance(pWarehouseNpcInfo->m_v3NpcPosition) > fDistLimit)
//							{
//								SetMovePosition(pWarehouseNpcInfo->m_v3NpcPosition);
//							}
//							else
//							{
//								if (GetMoveState() != PSM_STOP)
//								{
//									SetMovePosition(Vector3(0.0f, 0.0f, 0.0f));
//								}
//
//								break;
//							}
//						}
//
//						//std::this_thread::sleep_for(std::chrono::milliseconds(1500));
//
//						//TODO: Test edilecek
//						//SendWarehouseOpen(pWarehouseNpcInfo->m_iNpcID);
//
//						//std::this_thread::sleep_for(std::chrono::milliseconds(3000));
//
//						for (const TItemData& pItem : vecFlaggedItemList)
//						{
//							uint8_t iContable = 0;
//
//							__TABLE_ITEM* pItemData;
//							if (m_Bot->GetItemData(pItem.iItemID, pItemData))
//							{
//								iContable = pItemData->byContable;
//							}
//
//							int iRealSlot = GetWarehouseAvailableSlot(pItem.iItemID, iContable);
//							int iPage = iRealSlot / 24;
//							int iSlot = iRealSlot % 24;
//
//							SendWarehouseGetIn(pWarehouseNpcInfo->m_iNpcID, pItem.iItemID, (uint8_t)iPage, pItem.iPos - 14, (uint8_t)iSlot, pItem.iCount);
//
//							m_PlayerMySelf.tWarehouse[iRealSlot] = pItem;
//							m_PlayerMySelf.tWarehouse[iRealSlot].iPos = iRealSlot;
//
//							if (iContable)
//							{
//								m_PlayerMySelf.tWarehouse[iRealSlot].iCount += pItem.iCount;
//							}
//						}
//
//						std::this_thread::sleep_for(std::chrono::milliseconds(1500));
//						SendNpcEvent(pWarehouseNpcInfo->m_iNpcID);
//
//						std::this_thread::sleep_for(std::chrono::milliseconds(1500));
//						SendShoppingMall(ShoppingMallType::STORE_CLOSE);
//					}
//				}
//			}
//			break;
//
//			case RouteStepType::STEP_GENIE:
//			{
//				std::this_thread::sleep_for(std::chrono::milliseconds(3000));
//				SendStartGenie();
//			}
//			break;
//
//			case RouteStepType::STEP_GATE:
//			{
//				if (m_vecRoute.size() == 0)
//					continue;
//
//				float fDistance = GetDistance(Vector3(GetX(), 0.0f, GetY()), Vector3(pRoute.fX, 0.0f, pRoute.fY));
//
//				if (fDistance > 50.0f)
//				{
//					SetMovePosition(Vector3(pRoute.fX, 0.0f, pRoute.fY));
//					continue;
//				}
//
//				std::vector<__TABLE_UPC_SKILL>* vecAvailableSkills;
//				if (GetAvailableSkill(&vecAvailableSkills))
//				{
//					auto it = std::find_if(vecAvailableSkills->begin(), vecAvailableSkills->end(),
//						[](const TABLE_UPC_SKILL& a) { return a.iBaseId == 109015 || a.iBaseId == 111700; });
//
//					if (it == vecAvailableSkills->end())
//						continue;
//
//					if (GetMp() < it->iExhaustMSP)
//						continue;
//
//					new std::thread([=]() { UseSkillWithPacket(*it, GetID()); });
//				}
//			}
//			break;
//
//			case RouteStepType::STEP_BOT_START:
//			{
//				if (m_vecRoute.size() == 0)
//					continue;
//
//				GetUserConfiguration()->SetInt(skCryptDec("Automation"), skCryptDec("Attack"), 1);
//				GetUserConfiguration()->SetInt(skCryptDec("Automation"), skCryptDec("Character"), 1);
//			}
//			break;
//			}
//
//			if (m_vecRoute.size() > 0)
//				m_vecRoute.erase(m_vecRoute.begin());
//
//			if(m_vecRoute.size() == 0)
//				m_iRouteStep = RouteStepType::STEP_NONE;
//		}
//		catch (const std::exception& e)
//		{
//#ifdef DEBUG
//			printf("RouteProcess:Exception: %s\n", e.what());
//#else
//			UNREFERENCED_PARAMETER(e);
//#endif
//		}
//	}
//
//#ifdef DEBUG
//	printf("ClientHandler::RouteProcess Stopped\n");
//#endif
//}
//
//void ClientHandler::SupplyProcess()
//{
//#ifdef DEBUG
//	printf("ClientHandler::SupplyProcess Started\n");
//#endif
//
//	while (m_bWorking)
//	{
//		try
//		{
//			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
//
//			bool bAttackStatus = GetUserConfiguration()->GetBool(skCryptDec("Automation"), skCryptDec("Attack"), false);
//			bool bCharacterStatus = GetUserConfiguration()->GetBool(skCryptDec("Automation"), skCryptDec("Character"), false);
//
//			if (bAttackStatus || bCharacterStatus)
//			{
//				if (IsZoneChanging())
//					continue;
//
//				if (IsBlinking())
//					continue;
//
//				if (IsRouting())
//					continue;
//
//				if (IsDeath())
//					continue;
//
//				if (IsMovingToLoot())
//					continue;
//
//				//if (IsVipWarehouseOpen())
//				//	continue;
//
//				//if (IsTransactionDialogOpen())
//				//	continue;
//
//				//if (IsWarehouseOpen())
//				//	continue;
//
//				int16_t iHp = GetHp();
//				int16_t iMaxHp = GetMaxHp();
//
//				int32_t iHpPercent = (int32_t)std::ceil((iHp * 100) / iMaxHp);
//				int32_t iHpNeededValue = 60;
//
//				if (iHpPercent <= iHpNeededValue)
//					continue;
//
//				std::chrono::milliseconds msCurrentTime = duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
//
//				if (m_msLastSupplyTime > std::chrono::milliseconds(0) && (msCurrentTime - m_msLastSupplyTime) < std::chrono::milliseconds((60 * 5) * 1000))
//					continue;
//
//				bool bAutoRepair = GetUserConfiguration()->GetBool(skCryptDec("Supply"), skCryptDec("AutoRepair"), false);
//				bool bAutoRepairMagicHammer = GetUserConfiguration()->GetBool(skCryptDec("Supply"), skCryptDec("AutoRepairMagicHammer"), false);
//				bool bAutoSupply = GetUserConfiguration()->GetBool(skCryptDec("Supply"), skCryptDec("AutoSupply"), false);
//				bool bAutoSellSlotRange = GetUserConfiguration()->GetBool(skCryptDec("Supply"), skCryptDec("AutoSellSlotRange"), false);
//				bool bAutoSellByFlag = GetUserConfiguration()->GetBool(skCryptDec("Supply"), skCryptDec("AutoSellByFlag"), false);
//				bool bAutoSellVipSlotRange = GetUserConfiguration()->GetBool(skCryptDec("Supply"), skCryptDec("AutoSellVipSlotRange"), false);
//
//				bool bNeedSupply = bAutoSupply && IsNeedSupply();
//				bool bNeedRepair = !bAutoRepairMagicHammer && bAutoRepair && IsNeedRepair();
//				bool bNeedSell = (bAutoSellSlotRange || bAutoSellByFlag) && IsNeedSell();
//
//				if (bNeedSell && bAutoSellVipSlotRange)
//				{
//					if (!m_bVipWarehouseInitialized)
//						continue;
//
//					if (m_bVipWarehouseEnabled && !IsVipWarehouseFull())
//						continue;
//				}
//
//				if (bNeedSupply || bNeedRepair || bNeedSell)
//				{
//					std::string szSelectedSupplyRoute = GetUserConfiguration()->GetString(skCryptDec("Bot"), skCryptDec("SelectedSupplyRoute"), "");
//
//					RouteManager* pRouteManager = m_Bot->GetRouteManager();
//					RouteManager::RouteList pRouteList;
//
//					uint8_t iZoneID = GetRepresentZone(GetZone());
//
//					if (pRouteManager && pRouteManager->GetRouteList(iZoneID, pRouteList))
//					{
//						auto pRoute = pRouteList.find(szSelectedSupplyRoute);
//
//						if (pRoute != pRouteList.end())
//						{
//							SendStopGenie();
//							SetRoute(pRoute->second);
//						}
//					}
//				}
//			}
//		}
//		catch (const std::exception& e)
//		{
//#ifdef DEBUG
//			printf("SupplyProcess:Exception: %s\n", e.what());
//#else
//			UNREFERENCED_PARAMETER(e);
//#endif
//		}
//	}
//
//#ifdef DEBUG
//	printf("ClientHandler::SupplyProcess Stopped\n");
//#endif
//}
//
//void ClientHandler::VipWarehouseProcess()
//{
//#ifdef DEBUG
//	printf("ClientHandler::VipWarehouseProcess Started\n");
//#endif
//
//	return;
//
//	while (m_bWorking)
//	{
//		try
//		{
//			std::this_thread::sleep_for(std::chrono::milliseconds(15000));
//
//			bool bAttackStatus = GetUserConfiguration()->GetBool(skCryptDec("Automation"), skCryptDec("Attack"), false);
//			bool bCharacterStatus = GetUserConfiguration()->GetBool(skCryptDec("Automation"), skCryptDec("Character"), false);
//
//			if (bAttackStatus || bCharacterStatus)
//			{
//				if (IsZoneChanging())
//					continue;
//
//				if (IsBlinking())
//					continue;
//
//				if (IsRouting())
//					continue;
//
//				if (IsDeath())
//					continue;
//
//				if (IsMovingToLoot())
//					continue;
//
//				if (m_bVipWarehouseInitialized
//					&& !m_bVipWarehouseEnabled)
//					continue;
//
//				std::vector<TItemData> vecInventoryItemList;
//				std::vector<TItemData> vecFlaggedItemList;
//
//				bool bAutoSellSlotRange = GetUserConfiguration()->GetBool(skCryptDec("Supply"), skCryptDec("AutoSellSlotRange"), false);
//				int iAutoSellSlotRangeStart = GetUserConfiguration()->GetInt(skCryptDec("Supply"), skCryptDec("AutoSellSlotRangeStart"), 1);
//				int iAutoSellSlotRangeEnd = GetUserConfiguration()->GetInt(skCryptDec("Supply"), skCryptDec("AutoSellSlotRangeEnd"), 14);
//				bool bAutoSellByFlag = GetUserConfiguration()->GetBool(skCryptDec("Supply"), skCryptDec("AutoSellByFlag"), false);
//
//				GetInventoryItemList(vecInventoryItemList);
//
//				for (const TItemData& pItem : vecInventoryItemList)
//				{
//					if (pItem.iItemID == 0)
//						continue;
//
//					int iRangeStart = (iAutoSellSlotRangeStart)-1;
//					int iRangeEnd = (iAutoSellSlotRangeEnd)-1;
//
//					uint8_t iItemFlag = m_Bot->GetInventoryItemFlag(pItem.iItemID);
//
//					if ((iItemFlag == INVENTORY_ITEM_FLAG_VIP
//						|| (bAutoSellByFlag
//							&& iItemFlag == INVENTORY_ITEM_FLAG_SELL))
//						|| (bAutoSellSlotRange
//							&& pItem.iPos >= iRangeStart
//							&& pItem.iPos <= iRangeEnd))
//					{
//						vecFlaggedItemList.push_back(pItem);
//					}
//				}
//
//				if ((bAutoSellSlotRange
//					&& vecFlaggedItemList.size() >= (size_t)std::abs(iAutoSellSlotRangeEnd - iAutoSellSlotRangeStart))
//					|| (vecFlaggedItemList.size() >= 3))
//				{
//					OpenVipWarehouse();
//
//					WaitConditionWithTimeout(m_bVipWarehouseLoaded == false, 3000);
//
//					if (!m_bVipWarehouseLoaded)
//						continue;
//
//					for (const TItemData& pItem : vecFlaggedItemList)
//					{
//						__TABLE_ITEM* pItemData;
//						if (!m_Bot->GetItemData(pItem.iItemID, pItemData))
//							continue;
//
//						if (pItemData->byNeedRace == RACE_NO_TRADE_SOLD_STORE)
//							continue;
//
//						WaitConditionWithTimeout(Read4Byte(GetAddress(skCryptDec("KO_PTR_UI_LOCK"))) == 1, 3000);
//
//						std::vector<TItemData> vecVipWarehouseItemList;
//						GetVipWarehouseItemList(vecVipWarehouseItemList);
//
//						int iTargetPosition = -1;
//
//						if (pItemData->byContable)
//						{
//							auto pWarehouseItem = std::find_if(vecVipWarehouseItemList.begin(), vecVipWarehouseItemList.end(),
//								[&](const TItemData& a)
//								{
//									return a.iItemID == pItem.iItemID;
//								});
//
//							if (pWarehouseItem != vecVipWarehouseItemList.end())
//							{
//								if ((pItem.iCount + pWarehouseItem->iCount) > 9999)
//									continue;
//
//								iTargetPosition = pWarehouseItem->iPos;
//							}
//							else
//							{
//								auto pWarehouseEmptySlot = std::find_if(vecVipWarehouseItemList.begin(), vecVipWarehouseItemList.end(),
//									[&](const TItemData& a)
//									{
//										return a.iItemID == 0;
//									});
//
//								if (pWarehouseEmptySlot != vecVipWarehouseItemList.end())
//								{
//									iTargetPosition = pWarehouseEmptySlot->iPos;
//								}
//							}
//						}
//						else
//						{
//							auto pWarehouseEmptySlot = std::find_if(vecVipWarehouseItemList.begin(), vecVipWarehouseItemList.end(),
//								[&](const TItemData& a)
//								{
//									return a.iItemID == 0;
//								});
//
//							if (pWarehouseEmptySlot != vecVipWarehouseItemList.end())
//							{
//								iTargetPosition = pWarehouseEmptySlot->iPos;
//							}
//						}
//
//						if (iTargetPosition == -1)
//							break;
//
//						WaitConditionWithTimeout(Read4Byte(GetAddress(skCryptDec("KO_PTR_UI_LOCK"))) == 1, 3000);
//
//						Write4Byte(GetAddress(skCryptDec("KO_PTR_UI_WND")), 14);
//						Write4Byte(GetAddress(skCryptDec("KO_PTR_UI_SLOT_ORDER")), pItem.iPos - 14);
//						Write4Byte(GetAddress(skCryptDec("KO_PTR_UI_WND_DISTRICT")), 10);
//						Write4Byte(GetAddress(skCryptDec("KO_PTR_SELECTED_ITEM_BASE")), pItem.iBase);
//
//						VipWarehouseGetIn(iTargetPosition);
//
//						WaitConditionWithTimeout(Read4Byte(GetAddress(skCryptDec("KO_PTR_UI_LOCK"))) == 1, 3000);
//
//						if (pItemData->byContable)
//						{
//							CountableDialogChangeCount(1);
//							CountableDialogChangeCount(pItem.iCount);
//							AcceptCountableDialog();
//						}
//					}
//
//					WaitConditionWithTimeout(Read4Byte(GetAddress(skCryptDec("KO_PTR_UI_LOCK"))) == 1, 3000);
//
//					CloseVipWarehouse();
//				}
//			}
//		}
//		catch (const std::exception& e)
//		{
//#ifdef DEBUG
//			printf("VipWarehouseProcess:Exception: %s\n", e.what());
//#else
//			UNREFERENCED_PARAMETER(e);
//#endif
//		}
//	}
//
//#ifdef DEBUG
//	printf("ClientHandler::VipWarehouseProcess Stopped\n");
//#endif
//}

void ClientHandler::LevelDownerProcess()
{
#ifdef DEBUG
	printf("ClientHandler::LevelDownerProcess Started\n");
#endif

	while (m_bWorking)
	{
		try
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(100));

			bool bLevelDownerEnable = GetUserConfiguration()->GetInt(skCryptDec("LevelDowner"), skCryptDec("Enable"), false);

			if (!bLevelDownerEnable)
				continue;

			int iLevelDownerNpcId = GetUserConfiguration()->GetInt(skCryptDec("LevelDowner"), skCryptDec("NpcId"), -1);

			if (iLevelDownerNpcId == -1)
				continue;

			if (IsZoneChanging())
				continue;

			if (IsBlinking())
				continue;

			if (IsRouting())
				continue;

			bool bLevelDownerStopNearbyPlayer = GetUserConfiguration()->GetInt(skCryptDec("LevelDowner"), skCryptDec("StopIfNearbyPlayer"), true);

			if (bLevelDownerStopNearbyPlayer 
				&& GetRegionUserCount(true) > 0)
				continue;

			bool bLevelDownerLevelLimitEnable = GetUserConfiguration()->GetInt(skCryptDec("LevelDowner"), skCryptDec("LevelLimitEnable"), true);
			int iLevelDownerLevelLimit = GetUserConfiguration()->GetInt(skCryptDec("LevelDowner"), skCryptDec("LevelLimit"), 35);

			if (bLevelDownerLevelLimitEnable 
				&& m_PlayerMySelf.iLevel < iLevelDownerLevelLimit)
				continue;

			if (m_PlayerMySelf.eState == PSA_DEATH)
				SendRegenePacket();
			else
			{
				Packet pkt = Packet(WIZ_MAGIC_PROCESS);

				pkt
					<< uint8_t(SkillMagicType::SKILL_MAGIC_TYPE_EFFECTING)
					<< uint32_t(300215)
					<< iLevelDownerNpcId
					<< m_PlayerMySelf.iID;

				SendPacket(pkt);
			}
		}
		catch (const std::exception& e)
		{
#ifdef DEBUG
			printf("LevelDownerProcess:Exception: %s\n", e.what());
#else
			UNREFERENCED_PARAMETER(e);
#endif
		}
	}

#ifdef DEBUG
	printf("ClientHandler::LevelDownerProcess Stopped\n");
#endif
}

void ClientHandler::Test1()
{
	//Packet pkt1 = Packet(WIZ_MAGIC_PROCESS);

	//pkt1
	//	<< uint8_t(SkillMagicType::SKILL_MAGIC_TYPE_EFFECTING)
	//	<< uint32_t(472001)
	//	<< GetID()
	//	<< GetID();
	//pkt1
	//	<< uint32_t(0)
	//	<< int32_t(0)
	//	<< uint32_t(0);

	//pkt1 << uint16_t(0)
	//	<< uint32_t(0) << uint32_t(0) << int16_t(0) << int16_t(0) << int16_t(0);

	//SendPacket(pkt1);

	//std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	Packet pkt2 = Packet(WIZ_MAGIC_PROCESS);

	pkt2
		<< uint8_t(SkillMagicType::SKILL_MAGIC_TYPE_EFFECTING)
		<< uint32_t(472020)
		<< GetID()
		<< GetID();

	pkt2 
		<< uint32_t(0) << uint32_t(0) << uint32_t(0) << uint32_t(0);

	SendPacket(pkt2);


	//SendNpcEvent(51216);

	//std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	//SendQuestUnknown1(7516);

	//std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	//SendQuestUnknown1(7518);

	//std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	//SendQuestUnknown1(7521);

	//std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	//SendQuestUnknown1(7520);

	//std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	//SendQuestUnknown1(7517);

	//std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	//SendQuestUnknown1(7519);


	/*if (m_bVipWarehouseInitialized
		&& !m_bVipWarehouseEnabled)
		return;

	std::vector<TItemData> vecInventoryItemList;
	std::vector<TItemData> vecFlaggedItemList;

	bool bAutoSellSlotRange = GetUserConfiguration()->GetBool(skCryptDec("Supply"), skCryptDec("AutoSellSlotRange"), false);
	int iAutoSellSlotRangeStart = GetUserConfiguration()->GetInt(skCryptDec("Supply"), skCryptDec("AutoSellSlotRangeStart"), 1);
	int iAutoSellSlotRangeEnd = GetUserConfiguration()->GetInt(skCryptDec("Supply"), skCryptDec("AutoSellSlotRangeEnd"), 14);
	bool bAutoSellByFlag = GetUserConfiguration()->GetBool(skCryptDec("Supply"), skCryptDec("AutoSellByFlag"), false);

	GetInventoryItemList(vecInventoryItemList);

	for (const TItemData& pItem : vecInventoryItemList)
	{
		if (pItem.iItemID == 0)
			continue;

		int iRangeStart = (iAutoSellSlotRangeStart)-1;
		int iRangeEnd = (iAutoSellSlotRangeEnd)-1;

		uint8_t iItemFlag = m_Bot->GetInventoryItemFlag(pItem.iItemID);

		if ((iItemFlag == INVENTORY_ITEM_FLAG_VIP
			|| (bAutoSellByFlag
				&& iItemFlag == INVENTORY_ITEM_FLAG_SELL))
			|| (bAutoSellSlotRange
				&& pItem.iPos >= iRangeStart
				&& pItem.iPos <= iRangeEnd))
		{
			vecFlaggedItemList.push_back(pItem);
		}
	}

	if ((bAutoSellSlotRange
		&& vecFlaggedItemList.size() >= (size_t)std::abs(iAutoSellSlotRangeEnd - iAutoSellSlotRangeStart))
		|| (vecFlaggedItemList.size() >= 3))
	{
		OpenVipWarehouse();

		WaitConditionWithTimeout(m_bVipWarehouseLoaded == false, 3000);

		if (!m_bVipWarehouseLoaded)
			return;

		for (const TItemData& pItem : vecFlaggedItemList)
		{
			__TABLE_ITEM* pItemData;
			if (!m_Bot->GetItemData(pItem.iItemID, pItemData))
				continue;

			if (pItemData->byNeedRace == RACE_NO_TRADE_SOLD_STORE)
				continue;

			WaitConditionWithTimeout(Read4Byte(GetAddress(skCryptDec("KO_PTR_UI_LOCK"))) == 1, 3000);

			std::vector<TItemData> vecVipWarehouseItemList;
			GetVipWarehouseItemList(vecVipWarehouseItemList);

			int iTargetPosition = -1;

			if (pItemData->byContable)
			{
				auto pWarehouseItem = std::find_if(vecVipWarehouseItemList.begin(), vecVipWarehouseItemList.end(),
					[&](const TItemData& a)
					{
						return a.iItemID == pItem.iItemID;
					});

				if (pWarehouseItem != vecVipWarehouseItemList.end())
				{
					if ((pItem.iCount + pWarehouseItem->iCount) > 9999)
						continue;

					iTargetPosition = pWarehouseItem->iPos;
				}
				else
				{
					auto pWarehouseEmptySlot = std::find_if(vecVipWarehouseItemList.begin(), vecVipWarehouseItemList.end(),
						[&](const TItemData& a)
						{
							return a.iItemID == 0;
						});

					if (pWarehouseEmptySlot != vecVipWarehouseItemList.end())
					{
						iTargetPosition = pWarehouseEmptySlot->iPos;
					}
				}
			}
			else
			{
				auto pWarehouseEmptySlot = std::find_if(vecVipWarehouseItemList.begin(), vecVipWarehouseItemList.end(),
					[&](const TItemData& a)
					{
						return a.iItemID == 0;
					});

				if (pWarehouseEmptySlot != vecVipWarehouseItemList.end())
				{
					iTargetPosition = pWarehouseEmptySlot->iPos;
				}
			}

			if (iTargetPosition == -1)
				break;

			WaitConditionWithTimeout(Read4Byte(GetAddress(skCryptDec("KO_PTR_UI_LOCK"))) == 1, 3000);

			Write4Byte(GetAddress(skCryptDec("KO_PTR_UI_WND")), 14);
			Write4Byte(GetAddress(skCryptDec("KO_PTR_UI_SLOT_ORDER")), pItem.iPos - 14);
			Write4Byte(GetAddress(skCryptDec("KO_PTR_UI_WND_DISTRICT")), 10);
			Write4Byte(GetAddress(skCryptDec("KO_PTR_SELECTED_ITEM_BASE")), pItem.iBase);

			VipWarehouseGetIn(iTargetPosition);

			WaitConditionWithTimeout(Read4Byte(GetAddress(skCryptDec("KO_PTR_UI_LOCK"))) == 1, 3000);

			if (pItemData->byContable)
			{
				CountableDialogChangeCount(1);
				CountableDialogChangeCount(31);
				AcceptCountableDialog();
			}
		}

		WaitConditionWithTimeout(Read4Byte(GetAddress(skCryptDec("KO_PTR_UI_LOCK"))) == 1, 3000);

		CloseVipWarehouse();
	}*/
}

//void ClientHandler::Test2()
//{
//	bool bRunProcess = true;
//
//	while (bRunProcess)
//	{
//		if (m_bVipWarehouseInitialized
//			&& !m_bVipWarehouseEnabled)
//			break;
//
//		OpenVipWarehouse();
//
//		WaitConditionWithTimeout(m_bVipWarehouseLoaded == false, 3000);
//
//		if (!m_bVipWarehouseLoaded)
//			break;
//
//		bool bAutoSellByFlag = GetUserConfiguration()->GetBool(skCryptDec("Supply"), skCryptDec("AutoSellByFlag"), false);
//		bool bAutoSellVipSlotRange = GetUserConfiguration()->GetBool(skCryptDec("Supply"), skCryptDec("AutoSellVipSlotRange"), false);
//		int iAutoSellVipSlotRangeStart = GetUserConfiguration()->GetInt(skCryptDec("Supply"), skCryptDec("AutoSellVipSlotRangeStart"), 1);
//		int iAutoSellVipSlotRangeEnd = GetUserConfiguration()->GetInt(skCryptDec("Supply"), skCryptDec("AutoSellVipSlotRangeEnd"), 48);
//
//		std::vector<TItemData> vecVipWarehouseItemList;
//		std::vector<TItemData> vecFlaggedItemList;
//
//		GetVipWarehouseItemList(vecVipWarehouseItemList);
//
//		for (const TItemData& pItem : vecVipWarehouseItemList)
//		{
//			if (pItem.iItemID == 0)
//				continue;
//
//			int iRangeStart = (iAutoSellVipSlotRangeStart)-1;
//			int iRangeEnd = (iAutoSellVipSlotRangeEnd)-1;
//
//			uint8_t iItemFlag = m_Bot->GetInventoryItemFlag(pItem.iItemID);
//
//			if (((bAutoSellByFlag
//				&& iItemFlag == INVENTORY_ITEM_FLAG_SELL))
//				|| (bAutoSellVipSlotRange
//					&& pItem.iPos >= iRangeStart
//					&& pItem.iPos <= iRangeEnd))
//			{
//				vecFlaggedItemList.push_back(pItem);
//			}
//		}
//
//		if (vecFlaggedItemList.size() == 0)
//			break;
//
//		for (const TItemData& pItem : vecFlaggedItemList)
//		{
//			__TABLE_ITEM* pItemData;
//			if (!m_Bot->GetItemData(pItem.iItemID, pItemData))
//				continue;
//
//			WaitConditionWithTimeout(Read4Byte(GetAddress(skCryptDec("KO_PTR_UI_LOCK"))) == 1, 3000);
//
//			std::vector<TItemData> vecVipWarehouseInventoryItemList;
//			GetVipWarehouseInventoryItemList(vecVipWarehouseInventoryItemList);
//
//			int iTargetPosition = -1;
//
//			if (pItemData->byContable)
//			{
//				auto pInventoryItem = std::find_if(vecVipWarehouseInventoryItemList.begin(), vecVipWarehouseInventoryItemList.end(),
//					[&](const TItemData& a)
//					{
//						return a.iItemID == pItem.iItemID;
//					});
//
//				if (pInventoryItem != vecVipWarehouseInventoryItemList.end())
//				{
//					if ((pItem.iCount + pInventoryItem->iCount) > 9999)
//						continue;
//
//					iTargetPosition = pInventoryItem->iPos;
//				}
//				else
//				{
//					auto pInventoryEmptySlot = std::find_if(vecVipWarehouseInventoryItemList.begin(), vecVipWarehouseInventoryItemList.end(),
//						[&](const TItemData& a)
//						{
//							return a.iItemID == 0;
//						});
//
//					if (pInventoryEmptySlot != vecVipWarehouseInventoryItemList.end())
//					{
//						iTargetPosition = pInventoryEmptySlot->iPos;
//					}
//				}
//			}
//			else
//			{
//				auto pInventoryEmptySlot = std::find_if(vecVipWarehouseInventoryItemList.begin(), vecVipWarehouseInventoryItemList.end(),
//					[&](const TItemData& a)
//					{
//						return a.iItemID == 0;
//					});
//
//				if (pInventoryEmptySlot != vecVipWarehouseInventoryItemList.end())
//				{
//					iTargetPosition = pInventoryEmptySlot->iPos;
//				}
//			}
//
//			if (iTargetPosition == -1)
//			{
//				bRunProcess = false;
//				break;
//			}
//
//			WaitConditionWithTimeout(Read4Byte(GetAddress(skCryptDec("KO_PTR_UI_LOCK"))) == 1, 3000);
//
//			Write4Byte(GetAddress(skCryptDec("KO_PTR_UI_WND")), 14);
//			Write4Byte(GetAddress(skCryptDec("KO_PTR_UI_SLOT_ORDER")), pItem.iPos);
//			Write4Byte(GetAddress(skCryptDec("KO_PTR_UI_WND_DISTRICT")), 2);
//			Write4Byte(GetAddress(skCryptDec("KO_PTR_SELECTED_ITEM_BASE")), pItem.iBase);
//
//			VipWarehouseGetOut(iTargetPosition);
//
//			WaitConditionWithTimeout(Read4Byte(GetAddress(skCryptDec("KO_PTR_UI_LOCK"))) == 1, 3000);
//
//			if (pItemData->byContable)
//			{
//				CountableDialogChangeCount(1);
//				CountableDialogChangeCount(31);
//				AcceptCountableDialog();
//			}
//		}
//
//		WaitConditionWithTimeout(Read4Byte(GetAddress(skCryptDec("KO_PTR_UI_LOCK"))) == 1, 3000);
//
//		CloseVipWarehouse();
//	}
//}

void ClientHandler::SetRoute(std::vector<Route> vecRoute)
{
	m_vecRoute = vecRoute;
}

void ClientHandler::ClearRoute()
{
	m_iRouteStep = RouteStepType::STEP_NONE;
	m_vecRoute.clear();
}

int ClientHandler::GetRegionUserCount(bool bExceptPartyMember)
{
	int iUserCount = 0;

	if (bExceptPartyMember)
	{
		for (size_t i = 0; i < m_vecPlayer.size(); i++)
		{
			const auto& pPlayer = m_vecPlayer[i];

			auto pPartyMember = std::find_if(m_vecPartyMembers.begin(), m_vecPartyMembers.end(),
				[&](const PartyMember& a)
				{
					return a.iMemberID == pPlayer.iID;
				});

			if (pPartyMember != m_vecPartyMembers.end())
				continue;

			iUserCount++;
		}
	}
	else
	{
		iUserCount = m_vecPlayer.size();
	}

	return iUserCount;
}

int32_t ClientHandler::PartyMemberNeedSwift()
{
	std::vector<__TABLE_UPC_SKILL>* vecAvailableSkills;
	if (!GetAvailableSkill(&vecAvailableSkills))
		return -1;

	std::map<uint32_t, __TABLE_UPC_SKILL_EXTENSION4>* pSkillExtension4;
	if (!m_Bot->GetSkillExtension4Table(&pSkillExtension4))
		return -1;

	auto pSkillData = std::find_if(vecAvailableSkills->begin(), vecAvailableSkills->end(),
		[&](const TABLE_UPC_SKILL& a) { return a.iBaseId == 107010; });

	if (pSkillData == vecAvailableSkills->end())
		return -1;

	std::vector<PartyMember> tmpVecPartyMembers;
	std::copy(m_vecPartyMembers.begin(), m_vecPartyMembers.end(), std::back_inserter(tmpVecPartyMembers));

	auto pSort = [](PartyMember const& a, PartyMember const& b)
		{
			return a.iIndex < b.iIndex;
		};

	std::sort(tmpVecPartyMembers.begin(), tmpVecPartyMembers.end(), pSort);

	for (auto& pMember : tmpVecPartyMembers)
	{
		if (m_PlayerMySelf.iMSP < pSkillData->iExhaustMSP)
			continue;

		if (m_PlayerMySelf.iID == pMember.iMemberID)
		{
			auto pSkillExtension4Data = pSkillExtension4->find(pSkillData->iID);

			if (pSkillExtension4Data != pSkillExtension4->end())
			{
				if (IsBuffActive(pSkillExtension4Data->second.iBuffType))
					continue;
			}
		}

		DWORD iPlayerBase = GetEntityBase(pMember.iMemberID);

		if (iPlayerBase == 0)
			continue;

		auto iPlayerPosition = Vector3(
			ReadFloat(iPlayerBase + GetAddress("KO_OFF_X")),
			ReadFloat(iPlayerBase + GetAddress("KO_OFF_Z")),
			ReadFloat(iPlayerBase + GetAddress("KO_OFF_Y")));

		if (GetDistance(iPlayerPosition) > 25.0f)
			continue;

		if (IsStunned() || m_PlayerMySelf.eState == PSA_DEATH || IsDeath(iPlayerBase))
			continue;

		if (IsBlinking() || IsBlinking(iPlayerBase))
			continue;

		auto pFindedPlayer = std::find_if(m_vecPlayer.begin(), m_vecPlayer.end(),
			[&](const TPlayer& a) { return a.iID == pMember.iMemberID; });

		if (pFindedPlayer == m_vecPlayer.end())
			continue;

		if (m_PlayerMySelf.iID == pMember.iMemberID
			|| (pFindedPlayer->iMoveSpeed < 45 || pFindedPlayer->iMoveSpeed > 45))
			continue;

		if (m_bSkillCasting == true || GetActionState() == PSA_SPELLMAGIC)
			continue;

		return pMember.iMemberID;
	}

	return -1;
}

int32_t ClientHandler::PartyMemberNeedHeal(uint32_t iSkillBaseID)
{
	std::vector<__TABLE_UPC_SKILL>* vecAvailableSkills;
	if (!GetAvailableSkill(&vecAvailableSkills))
		return -1;

	std::map<uint32_t, __TABLE_UPC_SKILL_EXTENSION3>* pSkillExtension3;
	if (!m_Bot->GetSkillExtension3Table(&pSkillExtension3))
		return -1;

	auto pSkillData = std::find_if(vecAvailableSkills->begin(), vecAvailableSkills->end(),
		[=](const TABLE_UPC_SKILL& a) { return a.iBaseId == iSkillBaseID; });

	if (pSkillData == vecAvailableSkills->end())
		return -1;

	std::vector<PartyMember> tmpVecPartyMembers;
	std::copy(m_vecPartyMembers.begin(), m_vecPartyMembers.end(), std::back_inserter(tmpVecPartyMembers));

	auto pSort = [](PartyMember const& a, PartyMember const& b)
		{
			return a.iHP < b.iHP;
		};

	std::sort(tmpVecPartyMembers.begin(), tmpVecPartyMembers.end(), pSort);

	bool bHealProtection = GetUserConfiguration()->GetBool(skCryptDec("Protection"), skCryptDec("Heal"), false);

	for (auto& pMember : tmpVecPartyMembers)
	{
		if (m_PlayerMySelf.iMSP < pSkillData->iExhaustMSP)
			continue;

		if (bHealProtection)
		{
			int iHealProtectionValue = GetUserConfiguration()->GetInt(skCryptDec("Protection"), skCryptDec("HealValue"), 75);

			int32_t iHpProtectionPercent = (int32_t)std::ceil((pMember.iHP * 100) / pMember.iMaxHP);

			if (iHealProtectionValue < iHpProtectionPercent)
				continue;
		}
		else
		{
			auto pSkillExtension3Data = pSkillExtension3->find(pSkillData->iID);

			if (pSkillExtension3Data != pSkillExtension3->end())
			{
				int32_t iNeedHealValue = pMember.iMaxHP - pMember.iHP;

				if (iNeedHealValue <= 0
					|| iNeedHealValue < pSkillExtension3Data->second.iHealValue)
					continue;
			}
		}

		DWORD iPlayerBase = GetEntityBase(pMember.iMemberID);

		if (iPlayerBase == 0)
			continue;

		auto iPlayerPosition = Vector3(
			ReadFloat(iPlayerBase + GetAddress("KO_OFF_X")),
			ReadFloat(iPlayerBase + GetAddress("KO_OFF_Z")),
			ReadFloat(iPlayerBase + GetAddress("KO_OFF_Y")));

		if (GetDistance(iPlayerPosition) > 25.0f)
			continue;

		if (IsStunned() || m_PlayerMySelf.eState == PSA_DEATH || IsDeath(iPlayerBase))
			continue;

		if (IsBlinking() || IsBlinking(iPlayerBase))
			continue;

		auto pFindedPlayer = std::find_if(m_vecPlayer.begin(), m_vecPlayer.end(),
			[&](const TPlayer& a) { return a.iID == pMember.iMemberID; });

		if (m_PlayerMySelf.iID != pMember.iMemberID
			&& pFindedPlayer == m_vecPlayer.end())
			continue;

		if (m_bSkillCasting == true || GetActionState() == PSA_SPELLMAGIC)
			continue;

		return pMember.iMemberID;
	}

	return -1;
}

int32_t ClientHandler::PartyMemberNeedBuff(uint32_t iSkillBaseID)
{
	std::vector<__TABLE_UPC_SKILL>* vecAvailableSkills;
	if (!GetAvailableSkill(&vecAvailableSkills))
		return -1;

	std::map<uint32_t, __TABLE_UPC_SKILL_EXTENSION4>* pSkillExtension4;
	if (!m_Bot->GetSkillExtension4Table(&pSkillExtension4))
		return -1;

	auto pSkillData = std::find_if(vecAvailableSkills->begin(), vecAvailableSkills->end(),
		[=](const TABLE_UPC_SKILL& a) { return a.iBaseId == iSkillBaseID; });

	if (pSkillData == vecAvailableSkills->end())
		return -1;

	std::vector<PartyMember> tmpVecPartyMembers;
	std::copy(m_vecPartyMembers.begin(), m_vecPartyMembers.end(), std::back_inserter(tmpVecPartyMembers));

	auto pSort = [](PartyMember const& a, PartyMember const& b)
		{
			return a.iIndex < b.iIndex;
		};

	std::sort(tmpVecPartyMembers.begin(), tmpVecPartyMembers.end(), pSort);

	bool bHealProtection = GetUserConfiguration()->GetBool(skCryptDec("Protection"), skCryptDec("Heal"), false);

	for (auto& pMember : tmpVecPartyMembers)
	{
		if (m_PlayerMySelf.iMSP < pSkillData->iExhaustMSP)
			continue;

		auto pSkillExtension4Data = pSkillExtension4->find(pSkillData->iID);

		if (m_PlayerMySelf.iID == pMember.iMemberID)
		{
			if (pSkillExtension4Data != pSkillExtension4->end())
			{
				if (IsBuffActive(pSkillExtension4Data->second.iBuffType))
					continue;
			}
		}
		else
		{
			if (pSkillExtension4Data != pSkillExtension4->end())
			{
				float fBuffDuration = pSkillExtension4Data->second.iBuffDuration * 1000.0f;

				if (pSkillExtension4Data->second.iBuffType == BUFF_TYPE_HP_MP)
				{
					if (pMember.fHpBuffTime > 0
						&& Bot::TimeGet() < pMember.fHpBuffTime + (fBuffDuration / 1000.0f))
					{
						continue;
					}
				}

				if (pSkillExtension4Data->second.iBuffType == BUFF_TYPE_AC)
				{
					if (pMember.fACBuffTime > 0
						&& Bot::TimeGet() < pMember.fACBuffTime + (fBuffDuration / 1000.0f))
					{
						continue;
					}
				}

				if (pSkillExtension4Data->second.iBuffType == BUFF_TYPE_RESISTANCES)
				{
					if (pMember.fMindBuffTime > 0
						&& Bot::TimeGet() < pMember.fMindBuffTime + (fBuffDuration / 1000.0f))
					{
						continue;
					}
				}
			}
		}

		DWORD iPlayerBase = GetEntityBase(pMember.iMemberID);

		if (iPlayerBase == 0)
			continue;

		auto iPlayerPosition = Vector3(
			ReadFloat(iPlayerBase + GetAddress("KO_OFF_X")),
			ReadFloat(iPlayerBase + GetAddress("KO_OFF_Z")),
			ReadFloat(iPlayerBase + GetAddress("KO_OFF_Y")));

		if (GetDistance(iPlayerPosition) > 25.0f)
			continue;

		if (IsStunned() || m_PlayerMySelf.eState == PSA_DEATH || IsDeath(iPlayerBase))
			continue;

		if (IsBlinking() || IsBlinking(iPlayerBase))
			continue;

		auto pFindedPlayer = std::find_if(m_vecPlayer.begin(), m_vecPlayer.end(),
			[&](const TPlayer& a) { return a.iID == pMember.iMemberID; });

		if (m_PlayerMySelf.iID != pMember.iMemberID && pFindedPlayer == m_vecPlayer.end())
			continue;

		if (m_bSkillCasting == true || GetActionState() == PSA_SPELLMAGIC)
			continue;

		return pMember.iMemberID;
	}

	return -1;
}


bool ClientHandler::IsSkillHasZoneLimit(uint32_t iSkillBaseID)
{
	int iZoneIndex = m_PlayerMySelf.iCity;

	switch (iSkillBaseID)
	{
		case 490803:
		case 490811:
		case 490808:
		case 490809:
		case 490810:
		case 490800:
		case 490801:
		case 490817:
		{
			if (iZoneIndex == ZONE_DRAKI_TOWER ||
				iZoneIndex == ZONE_MONSTER_STONE1 ||
				iZoneIndex == ZONE_MONSTER_STONE2 ||
				iZoneIndex == ZONE_MONSTER_STONE3)
				return true;
		}
		break;
	}

	return false;
}
