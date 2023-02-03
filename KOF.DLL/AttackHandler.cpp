#include "pch.h"
#include "Bot.h"
#include "AttackHandler.h"

void AttackHandler::Start()
{
	new std::thread([]() { MainProcess(); });
}

void AttackHandler::Stop()
{
	m_bWorking = false;
}

void AttackHandler::MainProcess()
{
	m_bWorking = true;

	while (m_bWorking)
	{
		Sleep(1000);

		if (!Client::IsWorking())
			continue;

		//DWORD dwPlayerOther = Memory::Read4Byte(Client::GetAddress("KO_PTR_PLAYER_OTHER"));
		//DWORD dwAreaMonsterSize = Memory::Read4Byte(dwPlayerOther + 0x2C);

		//for (size_t i = 0; i < dwAreaMonsterSize; i++)
		//{
		//	DWORD dwAreaMonsterBase = Memory::Read4Byte(Memory::Read4Byte(dwPlayerOther + (0x2C - 0x04)) + 0x0C) + 0x14;

		//	DWORD Base = Memory::Read4Byte(dwAreaMonsterBase);

		//	printf("%u\n", Memory::Read4Byte(Base + Client::GetAddress("KO_OFF_ID")));

		//}




		/*DWORD dwPlayerOther = Memory::Read4Byte(Client::GetAddress("KO_PTR_PLAYER_OTHER"));

		DWORD dwAreaMonsterSize = Memory::Read4Byte(dwPlayerOther + 0x2C);

		std::vector<int> vecMonsterIds;

		for (size_t i = 0; i < dwAreaMonsterSize; i++)

			
		{
			DWORD dwMonsterBase = Memory::Read4Byte(Memory::Read4Byte(dwPlayerOther + 0x2C + 0x4) + 0x14);

			printf("%u\n", Memory::Read4Byte(dwMonsterBase + Client::GetAddress("KO_OFF_ID")));

		}*/


		//DWORD dwAreaPlayerSize = Memory::Read4Byte(dwPlayerOther + 0x34);
		//DWORD dwAreaPlayerList = Memory::Read4Byte(dwPlayerOther + 0x34 + 0x4);

	}
}