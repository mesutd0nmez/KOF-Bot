#include "pch.h"
#include "RouteManager.h"

RouteManager::RouteManager()
{
	m_mapRouteList.clear();
}

RouteManager::~RouteManager()
{
	m_mapRouteList.clear();
}

void RouteManager::Load()
{
	std::string routePath = skCryptDec("data\\route\\");

	for (const auto& entry : std::filesystem::directory_iterator(routePath))
	{
		try
		{
			std::ifstream i(entry.path());

			JSON jRouteData = JSON::parse(i);

			std::vector<Route> vecRoute;

			std::string szStepListAttribute = skCryptDec("steplist");

			for (size_t i = 0; i < jRouteData[szStepListAttribute.c_str()].size(); i++)
			{
				Route pRoute{};

				std::string szXAttribute = skCryptDec("x");
				std::string szYAttribute = skCryptDec("y");
				std::string szStepTypeAttribute = skCryptDec("steptype");
				std::string szPacketAttribute = skCryptDec("packet");

				pRoute.fX = jRouteData[szStepListAttribute.c_str()][i][szXAttribute.c_str()].get<float>();
				pRoute.fY = jRouteData[szStepListAttribute.c_str()][i][szYAttribute.c_str()].get<float>();
				pRoute.eStepType = (RouteStepType)jRouteData[szStepListAttribute.c_str()][i][szStepTypeAttribute.c_str()].get<int>();
				pRoute.szPacket = jRouteData[szStepListAttribute.c_str()][i][szPacketAttribute.c_str()].get<std::string>();

				vecRoute.push_back(pRoute);
			}

			std::string szNameAttribute = skCryptDec("name");
			std::string szIndexAttribute = skCryptDec("index");

			uint8_t iIndex = jRouteData[szIndexAttribute.c_str()].get<uint8_t>();

			auto pRouteData = m_mapRouteList.find(iIndex);

			if (pRouteData != m_mapRouteList.end())
			{
				pRouteData->second.insert(std::make_pair(jRouteData[szNameAttribute.c_str()].get<std::string>(), vecRoute));
			}
			else
			{
				m_mapRouteList.insert(std::make_pair(iIndex, std::map<std::string, std::vector<Route>> {
					std::make_pair(jRouteData[szNameAttribute.c_str()].get<std::string>(), vecRoute)
				}));
			}
		}
		catch (const std::exception& e)
		{
#ifdef DEBUG
			printf("%s\n", e.what());
#else
			DBG_UNREFERENCED_PARAMETER(e);
#endif
		}
	}
}

void RouteManager::Save(std::string szRouteName, uint8_t iMapIndex, std::vector<Route> vecRoute)
{
	try
	{
		JSON jRouteData;

		std::string szNameAttribute = skCryptDec("name");
		jRouteData[szNameAttribute.c_str()] = szRouteName;
		std::string szIndexAttribute = skCryptDec("index");
		jRouteData[szIndexAttribute.c_str()] = iMapIndex;

		for (auto& e : vecRoute)
		{
			JSON jRoute;

			std::string szXAttribute = skCryptDec("x");
			std::string szYAttribute = skCryptDec("y");
			std::string szStepTypeAttribute = skCryptDec("steptype");
			std::string szPacketAttribute = skCryptDec("packet");

			jRoute[szXAttribute.c_str()] = e.fX;
			jRoute[szYAttribute.c_str()] = e.fY;
			jRoute[szStepTypeAttribute.c_str()] = e.eStepType;
			jRoute[szPacketAttribute.c_str()] = e.szPacket;

			std::string szStepListAttribute = skCryptDec("steplist");
			jRouteData[szStepListAttribute.c_str()].push_back(jRoute);
		}

		std::string routePath = skCryptDec("data\\route\\");
		std::ofstream o(routePath + szRouteName + skCryptDec(".json"));
		o << std::setw(4) << jRouteData << std::endl;

		auto pRouteData = m_mapRouteList.find(iMapIndex);

		if (pRouteData != m_mapRouteList.end())
		{
			pRouteData->second.insert(std::make_pair(jRouteData[szNameAttribute.c_str()].get<std::string>(), vecRoute));
		}
		else
		{
			m_mapRouteList.insert(std::make_pair(iMapIndex, std::map<std::string, std::vector<Route>> {
				std::make_pair(jRouteData[szNameAttribute.c_str()].get<std::string>(), vecRoute)
			}));
		}
	}
	catch (const std::exception& e)
	{
#ifdef DEBUG
		printf("%s\n", e.what());
#else
		DBG_UNREFERENCED_PARAMETER(e);
#endif
	}
}

void RouteManager::Delete(std::string szRouteName, uint8_t iMapIndex)
{
	auto& inner = m_mapRouteList[iMapIndex];
	const auto it = inner.find(szRouteName);

	if (it != inner.end())
	{
		inner.erase(it);

		if (inner.size() == 0)
			m_mapRouteList.erase(iMapIndex);
	}

	std::string routePath = skCryptDec("data\\route\\") + szRouteName + skCryptDec(".json");
	remove(routePath.c_str());
}

bool RouteManager::GetRouteList(uint8_t iMapIndex, RouteList& pRouteList)
{
	auto pRouteData = m_mapRouteList.find(iMapIndex);

	if (pRouteData == m_mapRouteList.end())
		return false;

	pRouteList = pRouteData->second;

	return true;
}