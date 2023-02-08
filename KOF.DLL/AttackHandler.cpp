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

		if (Client::IsBlinking())
			continue;

		bool bAttackStatus = pUserConfig->GetBool("Automation", "Attack", false);

		if (!bAttackStatus)
			continue;

		std::vector<int> vecAttackList = pUserConfig->GetInt("Automation", "AttackSkillList", std::vector<int>());

		for (const auto& x : vecAttackList)
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

		if (Client::IsBlinking())
			continue;

		bool bAttackStatus = pUserConfig->GetBool("Automation", "Attack", false);

		if (!bAttackStatus)
			continue;

		auto pNpcList = Client::GetNpcList();

		if (pNpcList.size() == 0)
			continue;

		bool bAutoTarget = pUserConfig->GetInt("Attack", "AutoTarget", true);

		std::vector<int> vecSelectedNpcList = pUserConfig->GetInt("Attack", "NpcList", std::vector<int>());

		if (Client::GetTarget() == -1)
		{
			std::vector<TNpc> vecFilteredNpc;

			if (bAutoTarget)
			{
				std::copy_if(pNpcList.begin(), pNpcList.end(),
					std::back_inserter(vecFilteredNpc),
					[](const TNpc& c) 
					{ 
						return (c.iMonsterOrNpc == 1
							|| (c.iProtoID >= 19067 && c.iProtoID <= 19069)
							|| (c.iProtoID >= 19070 && c.iProtoID <= 19072))
							&& c.iProtoID != 9009
							&& c.eState != PSA_DYING 
							&& c.eState != PSA_DEATH
							&& Client::GetDistance(c.fX, c.fY) <= 45.0f;
					});
			}
			else
			{
				std::copy_if(pNpcList.begin(), pNpcList.end(),
					std::back_inserter(vecFilteredNpc),
					[vecSelectedNpcList](const TNpc& c) 
					{ 
						return (c.iMonsterOrNpc == 1
							|| (c.iProtoID >= 19067 && c.iProtoID <= 19069)
							|| (c.iProtoID >= 19070 && c.iProtoID <= 19072))
							&& c.iProtoID != 9009
							&& c.eState != PSA_DYING 
							&& c.eState != PSA_DEATH 
							&& std::count(vecSelectedNpcList.begin(), vecSelectedNpcList.end(), c.iProtoID)
							&& Client::GetDistance(c.fX, c.fY) <= 45.0f;
					});
			}

			auto pSort = [](TNpc const& a, TNpc const& b)
			{
				return Client::GetDistance(a.fX, a.fY) < Client::GetDistance(b.fX, b.fY);
			};

			std::sort(vecFilteredNpc.begin(), vecFilteredNpc.end(), pSort);

			if (vecFilteredNpc.size() > 0)
			{
				auto pFindedTarget = vecFilteredNpc.at(0);
				
				Client::SetTarget(pFindedTarget.iID);
				printf("SearchTargetProcess:: %d, Target Selected\n", pFindedTarget.iID);
			}
		}
		else
		{
			auto it = std::find_if(pNpcList.begin(), pNpcList.end(),
				[](const TNpc& a) 
				{ 
					return a.iID == Client::GetTarget(); 
				});

			if (it != pNpcList.end())
			{
				if (!bAutoTarget && !std::count(vecSelectedNpcList.begin(), vecSelectedNpcList.end(), it->iProtoID))
				{
					printf("SearchTargetProcess:: %d, Target not selected, selecting new target\n", it->iID);
					Client::SetTarget(-1);
				}

				if (Client::GetDistance(it->fX, it->fY) > 45.0f)
				{
					printf("SearchTargetProcess:: %d, Target out of range, selecting new target\n", it->iID);
					Client::SetTarget(-1);
				}

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