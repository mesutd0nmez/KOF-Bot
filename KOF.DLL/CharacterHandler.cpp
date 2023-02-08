#include "pch.h"
#include "Client.h"
#include "Bootstrap.h"
#include "CharacterHandler.h"

void CharacterHandler::Start()
{
	m_bWorking = true;

	new std::thread([]() { CharacterProcess(); });
}

void CharacterHandler::Stop()
{
	m_bWorking = false;
}

void CharacterHandler::CharacterProcess()
{
	while (m_bWorking)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));

		if (!Client::IsWorking())
			continue;

		auto pUserConfig = Client::GetUserConfig(Client::GetName());

		if (pUserConfig == NULL)
			continue;

		if (Client::IsBlinking())
			continue;

		bool bCharacterStatus = pUserConfig->GetBool("Automation", "Character", false);

		if (bCharacterStatus)
		{
			std::vector<int> vecCharacterSkillList = pUserConfig->GetInt("Automation", "CharacterSkillList", std::vector<int>());

			for (const auto& x : vecCharacterSkillList)
			{
				auto pSkillTable = Bootstrap::GetSkillTable().GetData();
				auto pSkillData = pSkillTable.find(x);

				if (pSkillData != pSkillTable.end())
				{
					uint32_t iNeedItem = pSkillData->second.dwNeedItem;

					uint32_t iNeedItemCount = 1;
					uint32_t iExistItemCount = 0;

					if (iNeedItem != 0)
					{
						iExistItemCount = Client::GetInventoryItemCount(pSkillData->second.dwNeedItem);

						auto pSkillExtension2 = Bootstrap::GetSkillExtension2Table().GetData();
						auto pSkillExtension2Data = pSkillExtension2.find(pSkillData->second.iID);

						if (pSkillExtension2Data != pSkillExtension2.end())
							iNeedItemCount = pSkillExtension2Data->second.iArrowCount;
					}

					if (Client::GetMp() < pSkillData->second.iExhaustMSP)
						continue;

					if (iNeedItem != 0 && iExistItemCount < iNeedItemCount)
						continue;

					auto pSkillExtension4 = Bootstrap::GetSkillExtension4Table().GetData();
					auto pSkillExtension4Data = pSkillExtension4.find(pSkillData->second.iID);

					if (pSkillExtension4Data != pSkillExtension4.end())
					{
						if (Client::IsBuffActive(pSkillExtension4Data->second.iBuffType))
							continue;
					}

					Client::UseSkill(pSkillData->second, Client::GetID());
				}
			}
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(1250));
	}
}