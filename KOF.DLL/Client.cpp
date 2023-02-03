#include "pch.h"
#include "Client.h"
#include "Memory.h"
#include "Define.h"

Client::Client()
{
	m_mapAddressList.clear();

	m_byState = INTRO;
	m_bWorking = false;
}

Client::~Client()
{
	m_mapAddressList.clear();
}

void Client::Start()
{
	new std::thread([]() { MainProcess(); });
}

void Client::Stop()
{
	m_bWorking = false;
}

void Client::MainProcess()
{
	Sleep(1);

	if (Client::IsCharacterLoaded())
		SetState(State::GAME);

	m_bWorking = true;

	while (m_bWorking)
	{
		switch (GetState())
		{
			case State::LOST: LostProcess(); break;
			case State::GAME: GameProcess(); break;
		}
	}
}

void Client::LostProcess()
{
	if (Client::IsDisconnect() && Client::IsServerSelectPhase())
		Client::SetState(State::SERVER_SELECT);

	if (!Client::IsDisconnect() && Client::IsCharacterSelectPhase())
		Client::SetState(State::CHARACTER_SELECT);
}

void Client::GameProcess()
{
	std::string strCharacterName = Client::GetName();

	if (Client::IsDisconnect())
	{
		fprintf(stdout, "%s - Connection lost\n", strCharacterName.c_str());
		Client::SetState(State::LOST);
	}
}

void Client::SetAddress(std::string strAddressName, DWORD dwAddress)
{
	m_mapAddressList.insert({ strAddressName, dwAddress });
}

DWORD Client::GetAddress(std::string strAddressName)
{
	auto it = m_mapAddressList.find(strAddressName);

	if (it == m_mapAddressList.end())
		return 0;

	return it->second;
}

bool Client::IsIntroPhase()
{
	return Memory::Read4Byte(GetAddress("KO_PTR_INTRO")) != 0;
};

bool Client::IsLoginPhase()
{
	return Memory::Read4Byte(Memory::Read4Byte(GetAddress("KO_PTR_LOGIN")) + GetAddress("KO_OFF_UI_LOGIN_INTRO")) != 0;
};

bool Client::IsServerSelectPhase()
{
	return Memory::Read4Byte(Memory::Read4Byte(Memory::Read4Byte(GetAddress("KO_PTR_LOGIN")) + GetAddress("KO_OFF_UI_LOGIN_INTRO")) + GetAddress("KO_OFF_LOGIN_SERVER_LIST_READY")) != -1;
};

bool Client::IsCharacterSelectPhase()
{
	return Memory::Read4Byte(Memory::Read4Byte(GetAddress("KO_PTR_CHARACTER_SELECT")) + GetAddress("KO_OFF_UI_CHARACTER_SELECT_TIMER")) != 0;
};

bool Client::IsDisconnect()
{
	return Memory::Read4Byte(Memory::Read4Byte(GetAddress("KO_PTR_PKT")) + GetAddress("KO_OFF_DISCONNECT")) == 0;
};

bool Client::IsCharacterLoaded()
{
	return GetID() != 0;
};

DWORD Client::GetRecvAddress()
{
	return Memory::Read4Byte(Memory::Read4Byte(GetAddress("KO_PTR_DLG"))) + 0x8;
}

DWORD Client::GetRecvCallAddress()
{
	return Memory::Read4Byte(Memory::Read4Byte(Memory::Read4Byte(GetAddress("KO_PTR_DLG"))) + 0x8);
}

DWORD Client::GetID()
{
	return Memory::Read4Byte(Memory::Read4Byte(GetAddress("KO_PTR_CHR")) + GetAddress("KO_OFF_ID"));
}

std::string Client::GetName()
{
	int NameLen = Memory::Read4Byte(Memory::Read4Byte(GetAddress("KO_PTR_CHR")) + GetAddress("KO_OFF_NAME_LEN"));

	if (NameLen > 15)
		return Memory::ReadString(Memory::Read4Byte(Memory::Read4Byte(GetAddress("KO_PTR_CHR")) + GetAddress("KO_OFF_NAME")), NameLen);

	return Memory::ReadString(Memory::Read4Byte(GetAddress("KO_PTR_CHR")) + GetAddress("KO_OFF_NAME"), NameLen);
}

DWORD Client::GetHp()
{
	return Memory::Read4Byte(Memory::Read4Byte(GetAddress("KO_PTR_CHR")) + GetAddress("KO_OFF_HP"));
}

DWORD Client::GetMaxHp()
{
	return Memory::Read4Byte(Memory::Read4Byte(GetAddress("KO_PTR_CHR")) + GetAddress("KO_OFF_MAX_HP"));
}

DWORD Client::GetMp()
{
	return Memory::Read4Byte(Memory::Read4Byte(GetAddress("KO_PTR_CHR")) + GetAddress("KO_OFF_MP"));
}

DWORD Client::GetMaxMp()
{
	return Memory::Read4Byte(Memory::Read4Byte(GetAddress("KO_PTR_CHR")) + GetAddress("KO_OFF_MAX_MP"));
}

DWORD Client::GetZone()
{
	return Memory::Read4Byte(Memory::Read4Byte(GetAddress("KO_PTR_CHR")) + GetAddress("KO_OFF_ZONE"));
}

DWORD Client::GetGold()
{
	return Memory::Read4Byte(Memory::Read4Byte(GetAddress("KO_PTR_CHR")) + GetAddress("KO_OFF_GOLD"));
}

DWORD Client::GetLevel()
{
	return Memory::Read4Byte(Memory::Read4Byte(GetAddress("KO_PTR_CHR")) + GetAddress("KO_OFF_LEVEL"));
}

DWORD Client::GetNation()
{
	return Memory::Read4Byte(Memory::Read4Byte(GetAddress("KO_PTR_CHR")) + GetAddress("KO_OFF_NATION"));
}

DWORD Client::GetClass()
{
	return Memory::Read4Byte(Memory::Read4Byte(GetAddress("KO_PTR_CHR")) + GetAddress("KO_OFF_CLASS"));
}

DWORD Client::GetExp()
{
	return Memory::Read4Byte(Memory::Read4Byte(GetAddress("KO_PTR_CHR")) + GetAddress("KO_OFF_EXP"));
}

DWORD Client::GetMaxExp()
{
	return Memory::Read4Byte(Memory::Read4Byte(GetAddress("KO_PTR_CHR")) + GetAddress("KO_OFF_MAXEXP"));
}

DWORD Client::GetGoX()
{
	return Memory::Read4Byte(Memory::Read4Byte(GetAddress("KO_PTR_CHR")) + GetAddress("KO_OFF_GOX"));
}

DWORD Client::GetGoY()
{
	return Memory::Read4Byte(Memory::Read4Byte(GetAddress("KO_PTR_CHR")) + GetAddress("KO_OFF_GOY"));
}

DWORD Client::GetX()
{
	return Memory::Read4Byte(Memory::Read4Byte(GetAddress("KO_PTR_CHR")) + GetAddress("KO_OFF_X"));
}

DWORD Client::GetY()
{
	return Memory::Read4Byte(Memory::Read4Byte(GetAddress("KO_PTR_CHR")) + GetAddress("KO_OFF_Y"));
}

DWORD Client::GetZ()
{
	return Memory::Read4Byte(Memory::Read4Byte(GetAddress("KO_PTR_CHR")) + GetAddress("KO_OFF_Z"));
}

DWORD Client::GetSkillPoint(int Slot)
{
	// Slot: 0 - Available skill points
	// Slot: 5 - Start index of skill tree

	DWORD dwSkillTreeDlg = Memory::Read4Byte(Memory::Read4Byte(GetAddress("KO_PTR_DLG")) + GetAddress("KO_OFF_SKILL_TREE_BASE"));
	return Memory::Read4Byte(dwSkillTreeDlg + GetAddress("KO_OFF_SKILL_TREE_POINT_BASE") + (Slot * 4));
}

void Client::PushPhase(DWORD address)
{
	PushPhaseCall pPushPhaseCallFnc = (PushPhaseCall)GetAddress("KO_PTR_PUSH_PHASE_CALL");
	pPushPhaseCallFnc(*reinterpret_cast<int*>(address));
}

void Client::SetLoginInformation(std::string strAccountId, std::string strAccountPassword)
{
	DWORD dwCGameProcIntroLogin = Memory::Read4Byte(GetAddress("KO_PTR_LOGIN"));
	DWORD dwCUILoginIntro = Memory::Read4Byte(dwCGameProcIntroLogin + GetAddress("KO_OFF_UI_LOGIN_INTRO"));

	DWORD dwCN3UIEditIdBase = Memory::Read4Byte(dwCUILoginIntro + GetAddress("KO_OFF_UI_LOGIN_INTRO_ID"));
	DWORD dwCN3UIEditPwBase = Memory::Read4Byte(dwCUILoginIntro + GetAddress("KO_OFF_UI_LOGIN_INTRO_PW"));

	Memory::WriteString(dwCN3UIEditIdBase + GetAddress("KO_OFF_UI_LOGIN_INTRO_ID_INPUT"), strAccountId.c_str());
	Memory::Write4Byte(dwCN3UIEditIdBase + GetAddress("KO_OFF_UI_LOGIN_INTRO_ID_INPUT_LENGTH"), strAccountId.size());
	Memory::WriteString(dwCN3UIEditPwBase + GetAddress("KO_OFF_UI_LOGIN_INTRO_PW_INPUT"), strAccountPassword.c_str());
	Memory::Write4Byte(dwCN3UIEditPwBase + GetAddress("KO_OFF_UI_LOGIN_INTRO_PW_INPUT_LENGTH"), strAccountPassword.size());
}

void Client::ConnectLoginServer()
{
	LoginCall1 pLoginCallFnc1 = (LoginCall1)GetAddress("KO_PTR_LOGIN_CALL1");
	pLoginCallFnc1(*reinterpret_cast<DWORD*>(GetAddress("KO_PTR_LOGIN")));

	LoginCall2 pLoginCallFnc2 = (LoginCall2)GetAddress("KO_PTR_LOGIN_CALL2");
	pLoginCallFnc2(*reinterpret_cast<DWORD*>(GetAddress("KO_PTR_LOGIN")));
}

void Client::SetServerIndex(BYTE byServerId)
{
	DWORD dwCGameProcIntroLogin = Memory::Read4Byte(GetAddress("KO_PTR_LOGIN"));
	DWORD dwCUILoginIntro = Memory::Read4Byte(dwCGameProcIntroLogin + GetAddress("KO_OFF_UI_LOGIN_INTRO"));

	Memory::Write4Byte(dwCUILoginIntro + GetAddress("KO_OFF_LOGIN_SERVER_INDEX"), byServerId);
}

void Client::ConnectGameServer()
{
	LoginServerCall pLoginServerCallFnc = (LoginServerCall)GetAddress("KO_PTR_LOGIN_SERVER_CALL");
	pLoginServerCallFnc(*reinterpret_cast<DWORD*>(GetAddress("KO_PTR_LOGIN")));
}

void Client::SelectCharacterEnter()
{
	CharacterSelectEnterCall pCharacterSelectEnterCallFnc = (CharacterSelectEnterCall)GetAddress("KO_PTR_CHARACTER_SELECT_ENTER_CALL");
	pCharacterSelectEnterCallFnc(*reinterpret_cast<DWORD*>(GetAddress("KO_PTR_CHARACTER_SELECT")));
}

void Client::SelectCharacter(BYTE byCharacterIndex)
{
	CharacterSelectCall pCharacterSelectCallFnc = (CharacterSelectCall)GetAddress("KO_PTR_CHARACTER_SELECT_CALL");
	pCharacterSelectCallFnc(*reinterpret_cast<DWORD*>(GetAddress("KO_PTR_CHARACTER_SELECT")));
}

void Client::SendPacket(Packet vecBuffer)
{
	Send pSendFnc = (Send)GetAddress("KO_SND_FNC");
	pSendFnc(*reinterpret_cast<DWORD*>(GetAddress("KO_PTR_PKT")), vecBuffer.contents(), vecBuffer.size());
}

void Client::RouteStart(float fX, float fY, float fZ)
{
	Vector3 *pVector3 = new Vector3(fX, fZ, fY);
	RouteStartCall pRouteStartCallFnc = (RouteStartCall)GetAddress("KO_PTR_ROUTE_START_CALL");
	pRouteStartCallFnc(*reinterpret_cast<int*>(GetAddress("KO_PTR_CHR")), pVector3);
}


void Client::Move(int iX, int iY)
{
	MoveCall pMoveCallFnc = (MoveCall)0x654A10;
	pMoveCallFnc(*reinterpret_cast<int*>(GetAddress("KO_PTR_CHR")), iX, iY);
}

void Client::Town()
{
	SendPacket(Packet(WIZ_HOME));
}