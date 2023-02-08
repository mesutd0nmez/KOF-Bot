#include "pch.h"
#include "Client.h"
#include "Bootstrap.h"
#include "ProtectionHandler.h"

void ProtectionHandler::Start()
{
	m_bWorking = true;

	new std::thread([]() { ProtectionProcess(); });
}

void ProtectionHandler::Stop()
{
	m_bWorking = false;
}

void ProtectionHandler::ProtectionProcess()
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

		bool bGodMode = pUserConfig->GetBool("Protection", "GodMode", false);

		if (bGodMode)
		{
			new std::thread([pUserConfig]()
			{
				auto pSkillTable = Bootstrap::GetSkillTable().GetData();
				auto pSkillData = pSkillTable.find(500344);

				if (pSkillData != pSkillTable.end())
				{
					int32_t iHpGodModePercent = (int32_t)std::ceil((Client::GetHp() * 100) / Client::GetMaxHp());
					int32_t iMpGodModePercent = (int32_t)std::ceil((Client::GetMp() * 100) / Client::GetMaxMp());

					int32_t iHpProtectionValue = pUserConfig->GetInt("Protection", "HpValue", 50);
					int32_t iMpProtectionValue = pUserConfig->GetInt("Protection", "MpValue", 25);

					if (iHpGodModePercent <= iHpProtectionValue
						|| iMpGodModePercent <= iMpProtectionValue
						|| !Client::IsBuffActive(1))
					{
						Client::SendCancelSkillPacket(pSkillData->second);
						Client::UseSkill(pSkillData->second, Client::GetID());
					}
				}
			});
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(1250));
	}
}