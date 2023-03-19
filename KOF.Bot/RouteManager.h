#pragma once

enum RouteStepType
{
    STEP_MOVE,
    STEP_TOWN,
    STEP_SUPPLY,
    STEP_INN,
};

struct Route
{
    float fX;
    float fY;
    RouteStepType eStepType;
};

class RouteManager
{
public:
    typedef std::map<std::string, std::vector<Route>> RouteList;
    typedef std::map<uint8_t, RouteList> RouteData;

public:
    RouteManager();
    ~RouteManager();

public:
    void Load();
    
    void Save(std::string szRouteName, uint8_t iMapIndex, std::vector<Route> vecRoute);
    void Delete(std::string szRouteName, uint8_t iMapIndex);

public:
    bool GetRouteList(uint8_t iMapIndex, RouteList& pRouteList);

private:
    RouteData m_mapRouteList;
};

