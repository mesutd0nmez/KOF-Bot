#include "Client.h"
#include "Bot.h"
#include <thread>

void Client::SetLoginPhase()
{
	SetPhase(KO_PTR_LOGIN);
}

void Client::SetPhase(DWORD address)
{
	__asm {
		mov ecx, [address]
		mov ecx, dword ptr ds : [ecx]
		push ecx
		mov edi, KO_PTR_PUSH_PHASE_CALL
		call edi
		add esp, 04
		mov al, 01
	}
}

void Client::SetLoginInformation(std::string strAccountId, std::string strAccountPassword)
{
	DWORD CGameProcIntroLogin = Memory::Read4Byte(KO_PTR_LOGIN);
	DWORD CUILoginIntro = Memory::Read4Byte(CGameProcIntroLogin + KO_OFF_UI_LOGIN_INTRO);

	DWORD CN3UIEditId = Memory::Read4Byte(CUILoginIntro + KO_OFF_UI_LOGIN_INTRO_ID);
	DWORD CN3UIEditPw = Memory::Read4Byte(CUILoginIntro + KO_OFF_UI_LOGIN_INTRO_PW);

	Memory::WriteString(CN3UIEditId + KO_OFF_UI_LOGIN_INTRO_ID_INPUT, strAccountId.c_str());
	Memory::Write4Byte(CN3UIEditId + KO_OFF_UI_LOGIN_INTRO_ID_INPUT_LENGTH, strAccountId.size());
	Memory::WriteString(CN3UIEditPw + KO_OFF_UI_LOGIN_INTRO_PW_INPUT, strAccountPassword.c_str());
	Memory::Write4Byte(CN3UIEditPw + KO_OFF_UI_LOGIN_INTRO_PW_INPUT_LENGTH, strAccountPassword.size());
}

void Client::ConnectLoginServer()
{
	__asm {
		mov ecx, [KO_PTR_LOGIN]
		mov ecx, dword ptr ds : [ecx]
		mov edi, KO_PTR_LOGIN_CALL1
		call edi
		mov ecx, [KO_PTR_LOGIN]
		mov ecx, dword ptr ds : [ecx]
		mov edi, KO_PTR_LOGIN_CALL2
		call edi
	}
}

void Client::SetServerIndex(BYTE byServerId)
{
	DWORD CGameProcIntroLogin = Memory::Read4Byte(KO_PTR_LOGIN);
	DWORD CUILoginIntro = Memory::Read4Byte(CGameProcIntroLogin + KO_OFF_UI_LOGIN_INTRO);

	Memory::Write4Byte(CUILoginIntro + KO_OFF_LOGIN_SERVER_INDEX, byServerId);
}

void Client::ConnectGameServer()
{
	__asm
	{
		mov ecx, [KO_PTR_LOGIN]
		mov ecx, dword ptr ds : [ecx]
		mov edi, KO_PTR_LOGIN_SERVER_CALL
		call edi
	}
}

void Client::SelectCharacterEnter()
{
	__asm
	{
		mov ecx, [KO_PTR_CHARACTER_SELECT]
		mov ecx, dword ptr ds : [ecx]
		mov edi, KO_PTR_CHARACTER_SELECT_ENTER_CALL
		call edi
	}
}

//TODO: Need byCharacterIndex implement
void Client::SelectCharacter(BYTE byCharacterIndex)
{
	__asm
	{
		mov ecx, [KO_PTR_CHARACTER_SELECT]
		mov ecx, dword ptr ds : [ecx]
		mov edi, KO_PTR_CHARACTER_SELECT_CALL
		call edi
	}
}