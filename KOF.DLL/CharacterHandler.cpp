#include "pch.h"
#include "Client.h"
#include "Bootstrap.h"
#include "CharacterHandler.h"

void CharacterHandler::Start()
{
	printf("CharacterHandler::Started\n");

	m_bWorking = true;

	new std::thread([]() { CharacterProcess(); });
}

void CharacterHandler::Stop()
{
	m_bWorking = false;

	printf("CharacterHandler::Stopped\n");
}

void CharacterHandler::CharacterProcess()
{
	printf("CharacterHandler::CharacterProcess Started\n");

	while (m_bWorking)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

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

			auto pSort = [](int& a, int& b)
			{
				return a > b;
			};

			std::sort(vecCharacterSkillList.begin(), vecCharacterSkillList.end(), pSort);

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

					std::chrono::milliseconds msNow = duration_cast<std::chrono::milliseconds>(
						std::chrono::system_clock::now().time_since_epoch()
					);

					std::chrono::milliseconds msLastSkillUseItem = Client::GetSkillUseTime(pSkillData->second.iID);

					if (pSkillData->second.iCooldown > 0 && msLastSkillUseItem.count() > 0)
					{
						int64_t iSkillCooldownTime = static_cast<int64_t>(pSkillData->second.iCooldown) * 100;

						if ((msLastSkillUseItem.count() + iSkillCooldownTime) > msNow.count())
							continue;
					}

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
	}

	printf("CharacterHandler::CharacterProcess Stopped\n");
}