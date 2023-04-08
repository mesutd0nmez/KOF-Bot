#include "pch.h"
#include "Json.h"
#include "World.h"
#include <stb_image.h>

World::World()
{
	m_mapWorldData.clear();
}

World::~World()
{
	m_mapWorldData.clear();
}

void World::Load(uint8_t iIndex)
{
	auto worldData = m_mapWorldData.find(iIndex);

	if (worldData == m_mapWorldData.end()) 
	{
		try
		{
			std::string szFileName = 
				std::filesystem::current_path().string() 
				+ skCryptDec("\\data\\")
				+ skCryptDec("\\world\\") 
				+ std::to_string(iIndex) 
				+ skCryptDec(".json");

			std::ifstream i(szFileName);
			JSON jWorldData = JSON::parse(i);

			WorldData pWorldData;
			memset(&pWorldData, 0, sizeof(pWorldData));

			std::string szIdAttribute = skCryptDec("Id");
			std::string szNameAttribute = skCryptDec("Name");
			std::string szMapImageAttribute = skCryptDec("MapImage");
			std::string szMiniMapImageAttribute = skCryptDec("MiniMapImage");
			std::string szMapSizeAttribute = skCryptDec("MapSize");
			std::string szMapLengthAttribute = skCryptDec("MapLength");

			pWorldData.iId = jWorldData[szIdAttribute.c_str()].get<uint8_t>();
			pWorldData.szName = jWorldData[szNameAttribute.c_str()].get<std::string>();
			pWorldData.szMapImageFile = jWorldData[szMapImageAttribute.c_str()].get<std::string>();
			pWorldData.szMiniMapImageFile = jWorldData[szMiniMapImageAttribute.c_str()].get<std::string>();
			pWorldData.iMapSize = jWorldData[szMapSizeAttribute.c_str()].get<int32_t>();
			pWorldData.fMapLength = jWorldData[szMapLengthAttribute.c_str()].get<float>();

			std::string szMapImageFilePath =
				std::filesystem::current_path().string()
				+ skCryptDec("\\data\\")
				+ skCryptDec("\\image\\")
				+ pWorldData.szMapImageFile;

			std::ifstream pMapImageRawDataStream(szMapImageFilePath.c_str(), std::ios::in | std::ios::binary);

			if (!pMapImageRawDataStream)
			{
#ifdef _DEBUG
				printf("World::Load: %s not loaded\n", szMapImageFilePath.c_str());
#endif
				return;
			}

			std::vector<uint8_t> pMapImageRawData((std::istreambuf_iterator<char>(pMapImageRawDataStream)), std::istreambuf_iterator<char>());
			pWorldData.pMapImageRawData = pMapImageRawData;

			stbi_load_from_memory(
				pMapImageRawData.data(),
				pMapImageRawData.size(),
				&pWorldData.iMapImageWidth,
				&pWorldData.iMapImageHeight,
				NULL,
				4);

			std::string szMinimapImageFilePath =
				std::filesystem::current_path().string()
				+ skCryptDec("\\data\\")
				+ skCryptDec("\\image\\")
				+ pWorldData.szMiniMapImageFile;

			std::ifstream pMiniMapImageRawDataStream(szMinimapImageFilePath.c_str(), std::ios::in | std::ios::binary);

			if (!pMiniMapImageRawDataStream)
			{
#ifdef _DEBUG
				printf("World::Load: %s not loaded\n", szMinimapImageFilePath.c_str());
#endif
				return;
			}

			std::vector<uint8_t> pMiniMapImageRawData((std::istreambuf_iterator<char>(pMiniMapImageRawDataStream)), std::istreambuf_iterator<char>());
			pWorldData.pMiniMapImageRawData = pMiniMapImageRawData;

			stbi_load_from_memory(
				pMiniMapImageRawData.data(),
				pMiniMapImageRawData.size(),
				&pWorldData.iMiniMapImageWidth,
				&pWorldData.iMiniMapImageHeight,
				NULL,
				4);

			m_mapWorldData.insert(std::pair<uint8_t, WorldData>(iIndex, pWorldData));
		}
		catch (const std::exception& e)
		{
			DBG_UNREFERENCED_PARAMETER(e);

#ifdef _DEBUG
			printf("%s\n", e.what());
#endif
		}
	}
}

WorldData* World::GetWorldData(uint8_t iIndex)
{
	auto worldData = m_mapWorldData.find(iIndex);

	if (worldData == m_mapWorldData.end())
		return nullptr;

	return &worldData->second;
}
