#include "pch.h"
#include "Client.h"
#include "Bootstrap.h"
#include "AutoLootHandler.h"

void AutoLootHandler::Start()
{
	printf("AutoLootHandler::Started\n");

	m_bWorking = true;

	new std::thread([]() { AutoLootProcess(); });
}

void AutoLootHandler::Stop()
{
	m_bWorking = false;

	printf("AutoLootHandler::Stopped\n");
}

void AutoLootHandler::AutoLootProcess()
{
	printf("AutoLootHandler::AutoLootProcess Started\n");

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

		auto pLootList = Client::GetLootList();

		if (pLootList->size() == 0)
			continue;

		bool bAutoLoot = pUserConfig->GetBool("AutoLoot", "Enable", false);

		if (bAutoLoot)
		{
			std::vector<TLoot> vecFilteredLoot;

			std::chrono::milliseconds msNow = duration_cast<std::chrono::milliseconds>(
				std::chrono::system_clock::now().time_since_epoch()
				);

			std::copy_if(pLootList->begin(), pLootList->end(),
				std::back_inserter(vecFilteredLoot),
				[msNow](const TLoot& c)
				{
					return (c.msDropTime.count() + 750) < msNow.count() && c.iRequestedOpen == false;
				});

			auto pSort = [](TLoot const& a, TLoot const& b)
			{
				return a.msDropTime.count() > b.msDropTime.count();
			};

			std::sort(vecFilteredLoot.begin(), vecFilteredLoot.end(), pSort);

			if (vecFilteredLoot.size() > 0)
			{
				auto pFindedLoot = vecFilteredLoot.at(0);

				bool bMoveToLoot = pUserConfig->GetBool("AutoLoot", "MoveToLoot", false);

				if (bMoveToLoot)
				{
					auto pNpcList = Client::GetNpcList();

					if (pNpcList.size() == 0)
					{
						Client::SetMovingToLoot(false);
						continue;
					}

					auto pNpc = std::find_if(pNpcList.begin(), pNpcList.end(),
						[pFindedLoot](const TNpc& a)
						{
							return a.iID == pFindedLoot.iNpcID;
						});

					if (pNpc != pNpcList.end())
					{
						while (Client::GetDistance(pNpc->fX, pNpc->fY) > 3.0f)
						{
							std::this_thread::sleep_for(std::chrono::milliseconds(100));

							Client::SetMovePosition(Vector3(pNpc->fX, pNpc->fZ, pNpc->fY));
							Client::SetMovingToLoot(true);
						}

						Client::SendRequestBundleOpen(pFindedLoot.iBundleID);

						auto pLoot = std::find_if(pLootList->begin(), pLootList->end(),
							[pFindedLoot](const TLoot a) { return a.iBundleID == pFindedLoot.iBundleID; });

						if (pLoot != pLootList->end())
						{
							pLoot->iRequestedOpen = true;
						}

						Client::SetMovingToLoot(false);

						printf("AutoLootProcess:: SendRequestBundleOpen: %d\n", pFindedLoot.iBundleID);
					}
				}
				else
				{
					Client::SendRequestBundleOpen(pFindedLoot.iBundleID);

					auto pLoot = std::find_if(pLootList->begin(), pLootList->end(),
						[pFindedLoot](const TLoot a) { return a.iBundleID == pFindedLoot.iBundleID; });

					if (pLoot != pLootList->end())
					{
						pLoot->iRequestedOpen = true;
					}

					printf("AutoLootProcess:: SendRequestBundleOpen: %d\n", pFindedLoot.iBundleID);
				}
			}
		}
	}

	printf("AutoLootHandler::AutoLootProcess Stopped\n");
}