#include "pch.h"
#include "Client.h"
#include "AttackHandler.h"
#include "Bootstrap.h"

void AttackHandler::Start()
{
	m_bWorking = true;

	new std::thread([]() { SearchTargetProcess(); });
	new std::thread([]() { AttackProcess(); });
}

void AttackHandler::Stop()
{
	m_bWorking = false;
}

void AttackHandler::AttackProcess()
{
	while (m_bWorking)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));

		if (!Client::IsWorking())
			continue;

		auto pUserConfig = Client::GetUserConfig(Client::GetName());

		if (pUserConfig == NULL)
			continue;

		if (Client::GetTarget() == -1)
			continue;

		bool bAttackStatus = pUserConfig->GetBool("Automation", "Attack", false);

		if (bAttackStatus)
		{
			std::vector<int> vecAttackList = pUserConfig->GetInt("Automation", "AttackSkillList", std::vector<int>());

			for (const auto& x : vecAttackList)
			{
				auto pSkillTable = Bootstrap::GetSkillTable().GetData();
				auto pSkillData = pSkillTable.find(x);

				if (pSkillData != pSkillTable.end())
				{
					if (Client::GetMp() >= pSkillData->second.iExhaustMSP)
					{
						Client::UseSkill(pSkillData->second, Client::GetTarget());
					}
				}

				bool bAttackSpeed = pUserConfig->GetBool("Attack", "AttackSpeed", false);

				if (bAttackSpeed)
				{
					int iAttackSpeedValue = pUserConfig->GetInt("Attack", "AttackSpeedValue", 1250);
					std::this_thread::sleep_for(std::chrono::milliseconds(iAttackSpeedValue));
				}
				else
					std::this_thread::sleep_for(std::chrono::milliseconds(1250));
			}
		}
		else
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1250));
		}	
	}
}

void AttackHandler::SearchTargetProcess()
{
	while (m_bWorking)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));

		if (!Client::IsWorking())
			continue;

		auto pUserConfig = Client::GetUserConfig(Client::GetName());

		if (pUserConfig == NULL)
			continue;

		auto pNpcList = Client::GetNpcList();

		if (pNpcList.size() == 0)
			continue;

		if (Client::GetTarget() == -1)
		{
			std::vector<TNpc> vecFilteredNpc;

			std::copy_if(pNpcList.begin(), pNpcList.end(),
				std::back_inserter(vecFilteredNpc),
				[](const TNpc& c) { return c.eState != PSA_DYING && c.eState != PSA_DEATH; });

			auto pSort = [](TNpc const& a, TNpc const& b)
			{
				return Client::GetDistance(a.fX, a.fY) < Client::GetDistance(b.fX, b.fY);
			};

			std::sort(vecFilteredNpc.begin(), vecFilteredNpc.end(), pSort);

			if (vecFilteredNpc.size() > 0)
			{
				auto pFindedTarget = vecFilteredNpc.at(0);

				if ((pFindedTarget.iMonsterOrNpc == 1
					|| (pFindedTarget.iProtoID >= 19067 && pFindedTarget.iProtoID <= 19069) //Scarecrow
					|| (pFindedTarget.iProtoID >= 19070 && pFindedTarget.iProtoID <= 19072)) //Scarecrow
					&& pFindedTarget.iProtoID != 9009) //Mine Guard
				{
					printf("SearchTargetProcess:: %d, Target Selected\n", pFindedTarget.iID);
					Client::SetTarget(pFindedTarget.iID);
				}
			}
		}
		else
		{
			auto it = std::find_if(pNpcList.begin(), pNpcList.end(),
				[](const TNpc& a) { return a.iID == Client::GetTarget(); });

			if (it != pNpcList.end())
			{
				if (it->eState == PSA_DYING || it->eState == PSA_DEATH)
				{
					printf("SearchTargetProcess:: %d, Target Dead\n", it->iID);
					Client::SetTarget(-1);
				}
			}
			else
			{
				printf("SearchTargetProcess:: %d, Target Lost\n", it->iID);
				Client::SetTarget(-1);
			}
		}
	}
}