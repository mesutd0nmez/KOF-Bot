#include "pch.h"
#include "HookHandler.h"
#include "Bot.h"
#include "Memory.h"

void HookHandler::Start()
{
	new std::thread([]() { MainProcess(); });
}

void HookHandler::Stop()
{
	m_bWorking = false;
}

void HookHandler::MainProcess()
{
	HookSendAddress();

	m_bWorking = true;

	while (m_bWorking)
	{
		Sleep(1000);

		if (!Client::IsWorking())
			continue;

		if (Memory::Read4Byte(Client::GetRecvAddress()) != GetRecvHookAddress())
			HookRecvAddress();
	}
}

DWORD HookHandler::GetRecvHookAddress()
{
	return m_dwRecvHookAddress;
};

DWORD HookHandler::GetSendHookAddress()
{
	return m_dwSendHookAddress;
};

void HookHandler::HookRecvAddress()
{
	m_dwRecvHookAddress = (DWORD)VirtualAllocEx(GetCurrentProcess(), (LPVOID*)GetRecvHookAddress(), sizeof(int), MEM_COMMIT, PAGE_EXECUTE_READWRITE);

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
		0xB8, 0x00, 0x00, 0x00, 0x00,			//mov eax,00000000 <-- ClientProcessor::RecvProcess()
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

	DWORD dwRecvProcessFunction = (DWORD)(LPVOID*)HookHandler::RecvProcess;
	CopyBytes(byPatch + 36, dwRecvProcessFunction);

	DWORD dwDlgAddress = Client::GetAddress("KO_PTR_DLG");
	CopyBytes(byPatch + 47, dwDlgAddress);

	DWORD dwRecvCallAddress = Client::GetRecvCallAddress();
	CopyBytes(byPatch + 58, dwRecvCallAddress);

	std::vector<BYTE> vecPatch(byPatch, byPatch + sizeof(byPatch));

	Memory::WriteBytes(GetRecvHookAddress(), vecPatch);

	DWORD oldProtection;
	VirtualProtect((LPVOID)Client::GetRecvAddress(), 1, PAGE_EXECUTE_READWRITE, &oldProtection);
	Memory::Write4Byte(Client::GetRecvAddress(), GetRecvHookAddress());
	VirtualProtect((LPVOID*)Client::GetRecvAddress(), 1, oldProtection, &oldProtection);

	fprintf(stdout, "Recv hooked\n");
}

void HookHandler::HookSendAddress()
{
	m_dwSendHookAddress = (DWORD)VirtualAllocEx(GetCurrentProcess(), (LPVOID*)GetSendHookAddress(), sizeof(int), MEM_COMMIT, PAGE_EXECUTE_READWRITE);

	BYTE byPatch1[] = 
	{
		0x55,										//push ebp
		0x8B, 0xEC,									//mov ebp,esp 
		0x60,										//pushad
		0xFF, 0x75, 0x0C,							//push [ebp+0C]
		0xFF, 0x75, 0x08,							//push [ebp+08]
		0xBA, 0x00, 0x00, 0x00, 0x00,				//mov edx,00000000 <-- ClientProcessor::SendProcess()
		0xFF, 0xD2,									//call edx
		0x5E,										//pop esi
		0x5D,										//pop ebp
		0x61,										//popad
		0x6A, 0xFF,									//push-01
		0xBA, 0x00, 0x00, 0x00, 0x00,				//mov edx,00000000 <-- KO_SND_FNC
		0x83, 0xC2, 0x5,							//add edx,05
		0xFF, 0xE2									//jmp edx
	};

	DWORD dwSendProcessFunction = (DWORD)(LPVOID*)HookHandler::SendProcess;
	CopyBytes(byPatch1 + 11, dwSendProcessFunction);

	DWORD dwKoPtrSndFnc = Client::GetAddress("KO_SND_FNC");
	CopyBytes(byPatch1 + 23, dwKoPtrSndFnc);

	std::vector<BYTE> vecPatch1(byPatch1, byPatch1 + sizeof(byPatch1));
	Memory::WriteBytes(GetSendHookAddress(), vecPatch1);

	BYTE byPatch2[] = 
	{
		0xE9, 0x00, 0x00, 0x00, 0x00,
	};

	DWORD dwCallDifference = Memory::GetDifference(Client::GetAddress("KO_SND_FNC"), GetSendHookAddress());
	CopyBytes(byPatch2 + 1, dwCallDifference);

	std::vector<BYTE> vecPatch2(byPatch2, byPatch2 + sizeof(byPatch2));
	Memory::WriteBytes(Client::GetAddress("KO_SND_FNC"), vecPatch2);

	fprintf(stdout, "Send hooked\n");
}


void HookHandler::RecvProcess(BYTE* byBuffer, DWORD dwLength)
{
	if (!m_bWorking)
		return;

	auto pkt = Packet(byBuffer[0], (size_t)dwLength);
	pkt.append(&byBuffer[1], dwLength - 1);

	uint8_t byHeader = pkt.GetOpcode();

	switch (byHeader)
	{
		/*case WIZ_XIGNCODE:
		{
			fprintf(stdout, "Xigncode Packet: %s\n", pkt.toHex().c_str());
		}
		break;*/
	default:
		break;
	}

	fprintf(stdout, "Recv Packet: %s\n", pkt.toHex().c_str());
}

void HookHandler::SendProcess(BYTE* byBuffer, DWORD dwLength)
{
	if (!m_bWorking)
		return;

	auto pkt = Packet(byBuffer[0], (size_t)dwLength);
	pkt.append(&byBuffer[1], dwLength - 1);

	uint8_t byHeader = pkt.GetOpcode();

	//switch (byHeader)
	//{
	//	default:
	//		break;
	//}

	fprintf(stdout, "Send Packet: %s\n", pkt.toHex().c_str());
}