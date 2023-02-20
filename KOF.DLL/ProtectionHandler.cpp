#include "pch.h"
#include "Client.h"
#include "Bootstrap.h"
#include "ProtectionHandler.h"

void ProtectionHandler::Start()
{
#ifdef _DEBUG
	printf("ProtectionHandler::Started\n");
#endif

	m_bWorking = true;

	new std::thread([]() { ProtectionProcess(); });
	new std::thread([]() { GodModeProcess(); });
	new std::thread([]() { MinorProcess(); });
}

void ProtectionHandler::Stop()
{
	m_bWorking = false;

#ifdef _DEBUG
	printf("ProtectionHandler::Stopped\n");
#endif
}

void ProtectionHandler::ProtectionProcess()
{
#ifdef _DEBUG
	printf("ProtectionHandler::ProtectionProcess Started\n");
#endif

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

		bool bHpProtectionEnable = pUserConfig->GetBool("Protection", "Hp", false);

		if (bHpProtectionEnable)
		{
			int32_t iHpProtectionPercent = (int32_t)std::ceil((Client::GetHp() * 100) / Client::GetMaxHp());
			int32_t iHpProtectionValue = pUserConfig->GetInt("Protection", "HpValue", 50);

			if (iHpProtectionPercent <= iHpProtectionValue)
				HealthPotionProcess();
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(500));

		bool bMpProtectionEnable = pUserConfig->GetBool("Protection", "Mp", false);

		if (bMpProtectionEnable)
		{
			int32_t iMpProtectionPercent = (int32_t)std::ceil((Client::GetMp() * 100) / Client::GetMaxMp());
			int32_t iMpProtectionValue = pUserConfig->GetInt("Protection", "MpValue", 50);

			if (iMpProtectionPercent <= iMpProtectionValue)
				ManaPotionProcess();
		}
	}

#ifdef _DEBUG
	printf("ProtectionHandler::ProtectionProcess Stopped\n");
#endif
}

void ProtectionHandler::HealthPotionProcess()
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

	TInventory* tInventoryItem = NULL;

	for (const auto& x : vecPotions)
	{
		tInventoryItem = Client::GetInventoryItem(x);

		if (tInventoryItem != NULL)
			break;
	}

	if (tInventoryItem != NULL)
		Client::UseItem(tInventoryItem->iItemID);
}

void ProtectionHandler::ManaPotionProcess()
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
		899997000, 399020000, 389082000
	};

	auto pSort = [](uint32_t const& a, uint32_t const& b)
	{
		return a < b;
	};

	std::sort(vecPotions.begin(), vecPotions.end(), pSort);

	TInventory* tInventoryItem = NULL;

	for (const auto& x : vecPotions)
	{
		tInventoryItem = Client::GetInventoryItem(x);

		if (tInventoryItem != NULL)
			break;
	}

	if (tInventoryItem != NULL)
		Client::UseItem(tInventoryItem->iItemID);
}

void ProtectionHandler::GodModeProcess()
{
#ifdef _DEBUG
	printf("ProtectionHandler::GodModeProcess Started\n");
#endif

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

		bool bGodMode = pUserConfig->GetBool("Protection", "GodMode", false);

		if (bGodMode)
		{
			auto pSkillTable = Bootstrap::GetSkillTable().GetData();
			auto pSkillData = pSkillTable.find(500344);

			if (pSkillData != pSkillTable.end())
			{
				int32_t iHpGodModePercent = (int32_t)std::ceil((Client::GetHp() * 100) / Client::GetMaxHp());
				int32_t iMpGodModePercent = (int32_t)std::ceil((Client::GetMp() * 100) / Client::GetMaxMp());

				int32_t iHpProtectionValue = pUserConfig->GetInt("Protection", "HpValue", 50);
				int32_t iMpProtectionValue = pUserConfig->GetInt("Protection", "MpValue", 25);

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

				if (iHpGodModePercent <= iHpProtectionValue
					|| iMpGodModePercent <= iMpProtectionValue
					|| !Client::IsBuffActive(1))
				{
					Client::SendCancelSkillPacket(pSkillData->second);
					Client::UseSkill(pSkillData->second, Client::GetID());
				}
			}
		}
	}

#ifdef _DEBUG
	printf("ProtectionHandler::GodModeProcess Stopped\n");
#endif
}

void ProtectionHandler::MinorProcess()
{
#ifdef _DEBUG
	printf("ProtectionHandler::MinorProcess Started\n");
#endif

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

		bool bMinorProtection = pUserConfig->GetBool("Protection", "Minor", false);

		if (bMinorProtection)
		{
			int32_t iHpProtectionPercent = (int32_t)std::ceil((Client::GetHp() * 100) / Client::GetMaxHp());
			int32_t iHpProtectionValue = pUserConfig->GetInt("Protection", "MinorValue", 30);

			if (iHpProtectionPercent <= iHpProtectionValue)
			{
				auto vecAvailableSkill = Client::GetAvailableSkill();

				auto it = std::find_if(vecAvailableSkill.begin(), vecAvailableSkill.end(),
					[](const TABLE_UPC_SKILL& a) { return a.szEngName == "minor healing"; });

				if (it != vecAvailableSkill.end())
				{
					Client::UseSkill(*it, Client::GetID());
				}

			}
		}
	}

#ifdef _DEBUG
	printf("ProtectionHandler::MinorProcess Stopped\n");
#endif
}