#pragma once

struct WorldData
{
	uint8_t iId;
	std::string szName;

	std::string szMapImageFile;
	std::string szMiniMapImageFile;

	int32_t iMapSize;
	float fMapLength;

	unsigned char* pMapImageData;
	std::vector<uint8_t> pMapImageRawData;
	int32_t iMapImageWidth;
	int32_t iMapImageHeight;

	unsigned char* pMiniMapImageData;
	std::vector<uint8_t> pMiniMapImageRawData;
	int32_t iMiniMapImageWidth;
	int32_t iMiniMapImageHeight;
};

class World
{
public:
	World();
	~World();

public:
	void Load(uint8_t iIndex);

public:
	WorldData* GetWorldData(uint8_t iIndex);

private:
	std::map<uint8_t, WorldData> m_mapWorldData;
};

