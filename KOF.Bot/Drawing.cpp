#include "pch.h"
#include "Drawing.h"
#include "Bot.h"
#include "Client.h"
#include "ClientHandler.h"
#include "UI.h"
#include "Guard.h"
#include "RouteManager.h"

std::string Drawing::m_szMainWindowName = "";
std::string Drawing::m_szRoutePlannerWindowName = "";

Bot* Drawing::Bot = nullptr;
bool Drawing::bDraw = true;
bool Drawing::bDrawRoutePlanner = false;
bool Drawing::Done = false;

ClientHandler* m_pClient = nullptr;
Ini* m_pUserConfiguration = nullptr;
Ini* m_pAppConfiguration = nullptr;

WorldData* m_pWorldData = nullptr;

PDIRECT3DTEXTURE9 m_pMapTexture = nullptr;
PDIRECT3DTEXTURE9 m_pMinimapTexture = nullptr;

char m_szRouteName[255] = "";

std::string m_szSelectedRoute = "";
std::vector<Route> m_vecRoute;

float m_fScreenWidth = 0.0f;
float m_fScreenHeight = 0.0f;

bool m_bInitPos = false;

int32_t m_iSkillID = 0;

void Drawing::Active()
{
	bDraw = true;
}

bool Drawing::isActive()
{
	return bDraw == true;
}

void Drawing::Initialize()
{
    m_fScreenWidth = (GetSystemMetrics(SM_CXSCREEN)) / 2.0f;
    m_fScreenHeight = (GetSystemMetrics(SM_CYSCREEN)) / 2.0f;
}

void Drawing::InitializeSceneData()
{
    m_pUserConfiguration = Drawing::Bot->GetUserConfiguration();
    m_pAppConfiguration = Drawing::Bot->GetAppConfiguration();
    m_pClient = Drawing::Bot->GetClientHandler();

    uint8_t iZoneID = m_pClient->GetRepresentZone(m_pClient->GetZone());

    if (m_pWorldData == nullptr || (m_pWorldData != nullptr && m_pWorldData->iId != iZoneID))
    {
        World* pWorld = Drawing::Bot->GetWorld();

        if (pWorld)
        {
            m_pWorldData = pWorld->GetWorldData(iZoneID);
        } 

        if (m_pWorldData != nullptr)
        {
            UI::LoadTextureFromMemory(m_pWorldData->pMiniMapImageRawData, &m_pMinimapTexture);
            UI::LoadTextureFromMemory(m_pWorldData->pMapImageRawData, &m_pMapTexture);
        }
    }

    if (Drawing::bDrawRoutePlanner == false)
        m_vecRoute.clear();
}

void Drawing::Draw()
{
	if (isActive())
	{
        InitializeSceneData();

        ImVec2 ScreenRes { 0, 0 };
        ImVec2 WindowPos { 0, 0 };
        ImVec2 WindowSize { 658, 700 };

        if (m_bInitPos == false)
        {
            RECT ScreenRect;
            GetWindowRect(GetDesktopWindow(), &ScreenRect);
            ScreenRes = ImVec2(float(ScreenRect.right), float(ScreenRect.bottom));
            WindowPos.x = (ScreenRes.x - WindowSize.x) * 0.5f;
            WindowPos.y = (ScreenRes.y - WindowSize.y) * 0.5f;
            m_bInitPos = true;
        }

        ImGui::SetNextWindowPos(ImVec2(WindowPos.x, WindowPos.y), ImGuiCond_Once);
        ImGui::SetNextWindowSize(ImVec2(WindowSize.x, WindowSize.y));
        ImGui::SetNextWindowBgAlpha(1.0f);

        ImGuiWindowFlags WindowFlags = ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize;
		ImGui::Begin(m_szMainWindowName.c_str(), &bDraw, WindowFlags);
		{
            DrawGameController();
		}

        ImGui::End();
	}

    DrawRoutePlanner();
}

void Drawing::DrawRoutePlanner()
{
    if (isActive())
    {
        InitializeSceneData();

        if (!Drawing::bDrawRoutePlanner)
            return;

        ImGuiWindowFlags WindowFlags = ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize;

        ImVec2 vec2InitialPos = { m_fScreenWidth, m_fScreenHeight };

        ImVec2 vWindowSize = { 1030, 855 };

        vec2InitialPos.x -= vWindowSize.x / 2;
        vec2InitialPos.y -= vWindowSize.y / 2;

        ImGui::SetNextWindowPos(vec2InitialPos, ImGuiCond_Once);
        ImGui::SetNextWindowSize(vWindowSize);
        ImGui::SetNextWindowBgAlpha(1.0f);
        ImGui::Begin(m_szRoutePlannerWindowName.c_str(), &Drawing::bDrawRoutePlanner, WindowFlags);
        {
            DrawRoutePlannerArea();
        }

        ImGui::End();
    }
}

void Drawing::DrawRoutePlannerArea()
{
    if(m_pWorldData == nullptr)
        return;

    ImGui::BeginChild(skCryptDec("RoutePlanner.Map"), ImVec2((float)(m_pWorldData->iMapImageWidth + 17.0f), (float)m_pWorldData->iMapImageHeight + 17.0f), true);
    {
        ImVec2 pOffsetPosition = ImGui::GetCursorScreenPos();

        ImGui::Image((void*)m_pMapTexture, ImVec2((float)m_pWorldData->iMapImageWidth, (float)m_pWorldData->iMapImageHeight));

        if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        {
            ImVec2 mousePositionAbsolute = ImGui::GetMousePos();
            ImVec2 screenPositionAbsolute = ImGui::GetItemRectMin();
            ImVec2 mousePositionRelative = ImVec2(mousePositionAbsolute.x - screenPositionAbsolute.x, mousePositionAbsolute.y - screenPositionAbsolute.y);

            Route pRoute;
            pRoute.fX = std::ceil(mousePositionRelative.x * (float)(m_pWorldData->fMapLength / m_pWorldData->iMapImageWidth));
            pRoute.fY = std::ceil((m_pWorldData->iMapImageHeight - mousePositionRelative.y) * (float)(m_pWorldData->fMapLength / m_pWorldData->iMapImageHeight));
            pRoute.eStepType = RouteStepType::STEP_MOVE;

            m_vecRoute.push_back(pRoute);
        }

        Vector3 v3CurrentPosition = m_pClient->GetPosition();

        ImVec2 currentPosition = ImVec2(
            pOffsetPosition.x + std::ceil(v3CurrentPosition.m_fX / (float)(m_pWorldData->fMapLength / m_pWorldData->iMapImageWidth)),
            pOffsetPosition.y + std::ceil(m_pWorldData->iMapImageHeight - (v3CurrentPosition.m_fY / (float)(m_pWorldData->fMapLength / m_pWorldData->iMapImageHeight))));

        ImGui::GetWindowDrawList()->AddCircle(currentPosition, 1.0f, IM_COL32(0, 255, 0, 255), 0, 3.0f);
        ImGui::GetWindowDrawList()->AddText(currentPosition, IM_COL32(0, 255, 0, 255), skCryptDec("Current Position"));

        size_t iRouteSize = m_vecRoute.size();

        for (size_t i = 0; i < iRouteSize; i++)
        {
            ImVec2 nextPosition = ImVec2(
                pOffsetPosition.x + std::ceil(m_vecRoute[i].fX / (float)(m_pWorldData->fMapLength / m_pWorldData->iMapImageWidth)),
                pOffsetPosition.y + std::ceil(m_pWorldData->iMapImageHeight - (m_vecRoute[i].fY / (float)(m_pWorldData->fMapLength / m_pWorldData->iMapImageHeight))));

            if (i > 0)
            {
                ImVec2 prevPosition = ImVec2(
                    pOffsetPosition.x + std::ceil(m_vecRoute[i - 1].fX / (float)(m_pWorldData->fMapLength / m_pWorldData->iMapImageWidth)),
                    pOffsetPosition.y + std::ceil(m_pWorldData->iMapImageHeight - (m_vecRoute[i - 1].fY / (float)(m_pWorldData->fMapLength / m_pWorldData->iMapImageHeight))));

                if (m_vecRoute[i - 1].eStepType != RouteStepType::STEP_TOWN)
                {
                    ImGui::GetWindowDrawList()->AddLine(prevPosition, nextPosition, IM_COL32(0, 255, 0, 255), 2.0f);
                }
                
            }

            switch (m_vecRoute[i].eStepType)
            {
                case RouteStepType::STEP_MOVE:
                {
                    if (i == 0)
                    {
                        ImVec2 startPosition = ImVec2(
                            pOffsetPosition.x + std::ceil(m_vecRoute[i].fX / (float)(m_pWorldData->fMapLength / m_pWorldData->iMapImageWidth)),
                            pOffsetPosition.y + std::ceil(m_pWorldData->iMapImageHeight - (m_vecRoute[i].fY / (float)(m_pWorldData->fMapLength / m_pWorldData->iMapImageHeight))));

                        ImGui::GetWindowDrawList()->AddCircle(startPosition, 1.0f, IM_COL32(0, 0, 255, 255), 0, 3.0f);
                        std::string szStartPoint = skCryptDec("Start Point(") + std::to_string((int)startPosition.x) + skCryptDec(",") + std::to_string((int)startPosition.y) + skCryptDec(")");
                        ImGui::GetWindowDrawList()->AddText(startPosition, IM_COL32(0, 0, 255, 255), szStartPoint.c_str());
                    }
                    else
                    {
                        std::string szMovePoint = skCryptDec("Move Point(") + std::to_string((int)nextPosition.x) + skCryptDec(",") + std::to_string((int)nextPosition.y) + skCryptDec(")");
                        ImGui::GetWindowDrawList()->AddText(nextPosition, IM_COL32(0, 255, 0, 255), szMovePoint.c_str());
                    }
                }
                break;

                case RouteStepType::STEP_TOWN:
                {
                    std::string szTownPoint = skCryptDec("Town Point(") + std::to_string((int)nextPosition.x) + skCryptDec(",") + std::to_string((int)nextPosition.y) + skCryptDec(")");
                    ImGui::GetWindowDrawList()->AddText(nextPosition, IM_COL32(255, 69, 0, 255), szTownPoint.c_str());
                }
                break;

                case RouteStepType::STEP_SUPPLY:
                {
                    std::string szSupplyPoint = skCryptDec("Supply Point(") + std::to_string((int)nextPosition.x) + skCryptDec(",") + std::to_string((int)nextPosition.y) + skCryptDec(")");
                    ImGui::GetWindowDrawList()->AddText(nextPosition, IM_COL32(75, 0, 130, 255), szSupplyPoint.c_str());
                }
                break;

                case RouteStepType::STEP_INN:
                {
                    std::string szInnPoint = skCryptDec("Inn Point(") + std::to_string((int)nextPosition.x) + skCryptDec(",") + std::to_string((int)nextPosition.y) + skCryptDec(")");
                    ImGui::GetWindowDrawList()->AddText(nextPosition, IM_COL32(128, 0, 0, 255), szInnPoint.c_str());
                }
                break;

                case RouteStepType::STEP_GENIE:
                {
                    std::string szGeniePoint = skCryptDec("Genie Point(") + std::to_string((int)nextPosition.x) + skCryptDec(",") + std::to_string((int)nextPosition.y) + skCryptDec(")");
                    ImGui::GetWindowDrawList()->AddText(nextPosition, IM_COL32(210, 105, 30, 255), szGeniePoint.c_str());
                }
                break;
            }
        }

    }
    ImGui::EndChild();

    ImGui::SameLine();

    ImGui::BeginChild(skCryptDec("RoutePlanner.Controller"), ImVec2(190, 817), true);
    {
        RouteManager* pRouteManager = Drawing::Bot->GetRouteManager();

        RouteManager::RouteList pRouteList;

        uint8_t iZoneID = m_pClient->GetRepresentZone(m_pClient->GetZone());

        if (pRouteManager && pRouteManager->GetRouteList(iZoneID, pRouteList))
        {
            ImGui::TextUnformatted(skCryptDec("Management"));
            ImGui::Separator();

            ImGui::Spacing();
            {
                ImGui::SetNextItemWidth(174);

                if (ImGui::BeginCombo(skCryptDec("##RoutePlanner.RouteList"), m_szSelectedRoute.c_str()))
                {
                    for (auto e : pRouteList)
                    {
                        const bool is_selected = (m_szSelectedRoute == e.first);

                        if (ImGui::Selectable(e.first.c_str(), is_selected))
                        {
                            m_szSelectedRoute = e.first;
                        }

                        if (is_selected)
                            ImGui::SetItemDefaultFocus();
                    }

                    ImGui::EndCombo();
                }

                if (ImGui::Button(skCryptDec("Load Plan"), ImVec2(174.0f, 0.0f)))
                {
                    auto pPlan = pRouteList.find(m_szSelectedRoute);

                    if (pPlan != pRouteList.end())
                    {
                        m_vecRoute = pPlan->second;
                        strcpy(m_szRouteName, m_szSelectedRoute.c_str());
                    }
                }

                if (ImGui::Button(skCryptDec("Delete Plan"), ImVec2(174.0f, 0.0f)))
                {
                    uint8_t iZoneID = m_pClient->GetRepresentZone(m_pClient->GetZone());

                    pRouteManager->Delete(m_szSelectedRoute, iZoneID);

                    std::string szSelectedRouteConfiguration = m_pUserConfiguration->GetString(skCryptDec("Bot"), skCryptDec("SelectedRoute"), "");

                    if (szSelectedRouteConfiguration == m_szSelectedRoute)
                    {
                        m_pUserConfiguration->SetString(skCryptDec("Bot"), skCryptDec("SelectedRoute"), "");
                    }

                    if (m_szRouteName == m_szSelectedRoute)
                    {
                        m_vecRoute.clear();
                        strcpy(m_szRouteName, "");
                    }

                    m_szSelectedRoute = "";
                }
            }
        }

        ImGui::TextUnformatted(skCryptDec("Action"));
        ImGui::Separator();

        size_t iRouteCount = m_vecRoute.size();

        ImGui::Spacing();
        {
            if (iRouteCount == 0)
                ImGui::BeginDisabled();

            if (m_pClient->IsRouting())
            {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.255f, 0.0f, 0.0f, 1.0f));
                if (ImGui::Button(skCryptDec("Stop"), ImVec2(174.0f, 0.0f)))
                {
                    m_pClient->ClearRoute();
                    m_pClient->StopMove();
                }
                ImGui::PopStyleColor(1);
            }
            else
            {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.255f, 0.0f, 1.0f));
                if (ImGui::Button(skCryptDec("Run"), ImVec2(174.0f, 0.0f)))
                {
                    if (m_vecRoute.size() > 0)
                        m_pClient->SetRoute(m_vecRoute);
                }
                ImGui::PopStyleColor(1);
            }

            if (ImGui::Button(skCryptDec("Undo"), ImVec2(83.0f, 0.0f)))
            {
                if(iRouteCount > 0)
                    m_vecRoute.pop_back();
            }

            ImGui::SameLine();

            if (ImGui::Button(skCryptDec("Reset"), ImVec2(83.0f, 0.0f)))
            {
                m_vecRoute.clear();
            }

            if (iRouteCount == 0)
                ImGui::EndDisabled();
        }

        ImGui::TextUnformatted(skCryptDec("Step Point"));
        ImGui::Separator();

        ImGui::Spacing();
        {
            if (iRouteCount == 0)
                ImGui::BeginDisabled();

            if (ImGui::Button(skCryptDec("Move Point"), ImVec2(174.0f, 0.0f)))
            {
                if (iRouteCount > 0)
                {
                    m_vecRoute[m_vecRoute.size() - 1].eStepType = RouteStepType::STEP_MOVE;
                }
            }

            if (ImGui::Button(skCryptDec("Town Point"), ImVec2(174.0f, 0.0f)))
            {
                if (iRouteCount > 0)
                {
                    m_vecRoute[m_vecRoute.size()-1].eStepType = RouteStepType::STEP_TOWN;
                }  
            }

            if (ImGui::Button(skCryptDec("Supply Point"), ImVec2(174.0f, 0.0f)))
            {
                if (iRouteCount > 0)
                {
                    m_vecRoute[m_vecRoute.size() - 1].eStepType = RouteStepType::STEP_SUPPLY;
                }
            }

            if (ImGui::Button(skCryptDec("Inn Point"), ImVec2(174.0f, 0.0f)))
            {
                if (iRouteCount > 0)
                {
                    m_vecRoute[m_vecRoute.size() - 1].eStepType = RouteStepType::STEP_INN;
                }
            }

            if (ImGui::Button(skCryptDec("Genie Point"), ImVec2(174.0f, 0.0f)))
            {
                if (iRouteCount > 0)
                {
                    m_vecRoute[m_vecRoute.size() - 1].eStepType = RouteStepType::STEP_GENIE;
                }
            }

            if (iRouteCount == 0)
                ImGui::EndDisabled();
        }

        ImGui::TextUnformatted(skCryptDec("Save"));
        ImGui::Separator();

        ImGui::Spacing();
        {
            if (iRouteCount == 0)
                ImGui::BeginDisabled();


            ImGui::SetNextItemWidth(174);
            ImGui::InputText(skCryptDec("##FileName"), &m_szRouteName[0], 100);

            size_t routeNameSize = strlen(m_szRouteName);

            if (routeNameSize == 0)
                ImGui::BeginDisabled();

            if (ImGui::Button(skCryptDec("Save Plan"), ImVec2(174.0f, 0.0f)))
            {
                if (pRouteManager)
                {
                    uint8_t iZoneID = m_pClient->GetRepresentZone(m_pClient->GetZone());

                    pRouteManager->Save(m_szRouteName, iZoneID, m_vecRoute);
                }
            }

            if (routeNameSize == 0)
                ImGui::EndDisabled();

            if (iRouteCount == 0)
                ImGui::EndDisabled();
        }
    }
    ImGui::EndChild();
}

void Drawing::CenteredText(std::string strValue)
{
    auto windowWidth = ImGui::GetWindowSize().x;
    auto textWidth = ImGui::CalcTextSize(strValue.c_str()).x;
    ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
    ImGui::TextUnformatted(strValue.c_str());
}

void Drawing::RightText(std::string strValue)
{
    auto windowWidth = ImGui::GetWindowSize().x;
    auto textWidth = ImGui::CalcTextSize(strValue.c_str()).x;
    ImGui::SetCursorPosX((windowWidth - textWidth) * 0.93f);
    ImGui::TextUnformatted(strValue.c_str());
}

void Drawing::DrawGameController()
{
    ImGui::BeginChild(1, ImVec2(283, 663), true);
    {
        ImGui::TextUnformatted(m_pClient->GetName().c_str());
        ImGui::SameLine();
        ImGui::NextColumn();
        RightText(skCryptDec("Lv ") + std::to_string(m_pClient->GetLevel()));
        ImGui::Separator();

        ImGui::Spacing();
        {
            ImGui::PushItemWidth(265);
            {
                CenteredText(std::to_string(m_pClient->GetHp()) + skCryptDec(" / ") + std::to_string(m_pClient->GetMaxHp()));

                float fHpProgress = (((float)m_pClient->GetHp() / (float)m_pClient->GetMaxHp()) * 100.f) / 100.0f;

                ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(255.0f, 0.0f, 0.0f, 1.0f));
                ImGui::ProgressBar(fHpProgress, ImVec2(0.0f, 0.0f));
                ImGui::PopStyleColor(1);

                CenteredText(std::to_string(m_pClient->GetMp()) + skCryptDec(" / ") + std::to_string(m_pClient->GetMaxMp()));

                float fMpProgress = (((float)m_pClient->GetMp() / (float)m_pClient->GetMaxMp()) * 100.f) / 100.0f;

                ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.0f, 0.0f, 255.0f, 1.0f));
                ImGui::ProgressBar(fMpProgress, ImVec2(0.0f, 0.0f));
                ImGui::PopStyleColor(1);
            }

            ImGui::PopItemWidth();
        }

        ImGui::Spacing();
        {
            if (m_pWorldData)
            {
                ImGui::BeginChild(skCryptDec("Minimap"), ImVec2((float)(m_pWorldData->iMiniMapImageWidth + 17.0f), (float)m_pWorldData->iMiniMapImageHeight + 17.0f), true);
                {
                    ImVec2 pOffsetPosition = ImGui::GetCursorScreenPos();

                    ImGui::Image((void*)m_pMinimapTexture, ImVec2((float)m_pWorldData->iMiniMapImageWidth, (float)m_pWorldData->iMiniMapImageHeight));

                    if (ImGui::IsItemHovered() && ImGui::IsMouseDown(ImGuiMouseButton_Left))
                    {
                        ImVec2 mousePositionAbsolute = ImGui::GetMousePos();
                        ImVec2 screenPositionAbsolute = ImGui::GetItemRectMin();
                        ImVec2 mousePositionRelative = ImVec2(mousePositionAbsolute.x - screenPositionAbsolute.x, mousePositionAbsolute.y - screenPositionAbsolute.y);

                        m_pClient->SetMovePosition(
                            Vector3(
                                std::ceil(mousePositionRelative.x * (float)(m_pWorldData->fMapLength / m_pWorldData->iMiniMapImageWidth)), 0.0f,
                                std::ceil((m_pWorldData->iMiniMapImageHeight - mousePositionRelative.y) * (float)(m_pWorldData->fMapLength / m_pWorldData->iMiniMapImageHeight))
                            )
                        );
                    }

                    std::lock_guard<std::recursive_mutex> lockPlayer(m_pClient->m_mutexPlayer);
                    for (const TPlayer& pPlayer : m_pClient->m_vecPlayer)
                    {
                        ImVec2 pNpcPosition = ImVec2(
                            pOffsetPosition.x + std::ceil(pPlayer.fX / (float)(m_pWorldData->fMapLength / m_pWorldData->iMiniMapImageWidth)),
                            pOffsetPosition.y + std::ceil(m_pWorldData->iMiniMapImageHeight - (pPlayer.fY / (float)(m_pWorldData->fMapLength / m_pWorldData->iMiniMapImageHeight))));

                        ImGui::GetWindowDrawList()->AddCircle(pNpcPosition, 1.0f, IM_COL32(0, 0, 255, 255), 0, 3.0f);
                    }

                    std::lock_guard<std::recursive_mutex> lockNpc(m_pClient->m_mutexNpc);
                    for (const TNpc& pMob : m_pClient->m_vecNpc)
                    {
                        if (pMob.iMonsterOrNpc == 1)
                        {
                            ImVec2 pNpcPosition = ImVec2(
                                pOffsetPosition.x + std::ceil(pMob.fX / (float)(m_pWorldData->fMapLength / m_pWorldData->iMiniMapImageWidth)),
                                pOffsetPosition.y + std::ceil(m_pWorldData->iMiniMapImageHeight - (pMob.fY / (float)(m_pWorldData->fMapLength / m_pWorldData->iMiniMapImageHeight))));

                            ImGui::GetWindowDrawList()->AddCircle(pNpcPosition, 1.0f, IM_COL32(255, 0, 0, 255), 0, 3.0f);
                        }
                        else
                        {
                            ImVec2 pNpcPosition = ImVec2(
                                pOffsetPosition.x + std::ceil(pMob.fX / (float)(m_pWorldData->fMapLength / m_pWorldData->iMiniMapImageWidth)),
                                pOffsetPosition.y + std::ceil(m_pWorldData->iMiniMapImageHeight - (pMob.fY / (float)(m_pWorldData->fMapLength / m_pWorldData->iMiniMapImageHeight))));

                            ImGui::GetWindowDrawList()->AddCircle(pNpcPosition, 1.0f, IM_COL32(0, 191, 255, 255), 0, 3.0f);
                        }
                    }

                    Vector3 v3CurrentPosition = m_pClient->GetPosition();

                    ImVec2 currentPosition = ImVec2(
                        pOffsetPosition.x + std::ceil(v3CurrentPosition.m_fX / (float)(m_pWorldData->fMapLength / m_pWorldData->iMiniMapImageWidth)),
                        pOffsetPosition.y + std::ceil(m_pWorldData->iMiniMapImageHeight - (v3CurrentPosition.m_fY / (float)(m_pWorldData->fMapLength / m_pWorldData->iMiniMapImageHeight))));

                    ImGui::GetWindowDrawList()->AddCircle(currentPosition, 1.0f, IM_COL32(0, 255, 0, 255), 0, 3.0f);

                    float fGoX = m_pClient->GetGoX();
                    float fGoY = m_pClient->GetGoY();

                    if (fGoX > 0.0f && fGoY > 0.0f)
                    {
                        ImVec2 movePosition = ImVec2(
                            pOffsetPosition.x + std::ceil(fGoX / (float)(m_pWorldData->fMapLength / m_pWorldData->iMiniMapImageWidth)),
                            pOffsetPosition.y + std::ceil(m_pWorldData->iMiniMapImageHeight - (fGoY / (float)(m_pWorldData->fMapLength / m_pWorldData->iMiniMapImageHeight))));

                        ImGui::GetWindowDrawList()->AddLine(currentPosition, movePosition, IM_COL32(0, 255, 0, 255), 2.0f);
                    }

                    std::string szSelectedRouteConfiguration = m_pUserConfiguration->GetString(skCryptDec("Bot"), skCryptDec("SelectedRoute"), "");

                    RouteManager* pRouteManager = Drawing::Bot->GetRouteManager();

                    uint8_t iZoneID = m_pClient->GetRepresentZone(m_pClient->GetZone());

                    RouteManager::RouteList pRouteList;
                    if (pRouteManager && pRouteManager->GetRouteList(iZoneID, pRouteList))
                    {
                        auto pPlan = pRouteList.find(szSelectedRouteConfiguration);

                        if (pPlan != pRouteList.end())
                        {
                            size_t iRouteSize = pPlan->second.size();

                            for (size_t i = 0; i < iRouteSize; i++)
                            {
                                ImVec2 nextPosition = ImVec2(
                                    pOffsetPosition.x + std::ceil(pPlan->second[i].fX / (float)(m_pWorldData->fMapLength / m_pWorldData->iMiniMapImageWidth)),
                                    pOffsetPosition.y + std::ceil(m_pWorldData->iMiniMapImageHeight - (pPlan->second[i].fY / (float)(m_pWorldData->fMapLength / m_pWorldData->iMiniMapImageHeight))));

                                if (i > 0)
                                {
                                    ImVec2 prevPosition = ImVec2(
                                        pOffsetPosition.x + std::ceil(pPlan->second[i - 1].fX / (float)(m_pWorldData->fMapLength / m_pWorldData->iMiniMapImageWidth)),
                                        pOffsetPosition.y + std::ceil(m_pWorldData->iMiniMapImageHeight - (pPlan->second[i - 1].fY / (float)(m_pWorldData->fMapLength / m_pWorldData->iMiniMapImageHeight))));

                                    if (pPlan->second[i - 1].eStepType != RouteStepType::STEP_TOWN)
                                    {
                                        ImGui::GetWindowDrawList()->AddLine(prevPosition, nextPosition, IM_COL32(0, 255, 0, 255), 2.0f);
                                    }        
                                }
                            }
                        }
                    }

                    ImGui::EndChild();
                }
            }
        }

        ImGui::Spacing();
        {
            ImGui::TextUnformatted(skCryptDec("Planned Route"));
            ImGui::Separator();

            RouteManager* pRouteManager = Drawing::Bot->GetRouteManager();

            ImGui::SetNextItemWidth(266);
            std::string szSelectedRoute = m_pUserConfiguration->GetString(skCryptDec("Bot"), skCryptDec("SelectedRoute"), "");
            if (ImGui::BeginCombo(skCryptDec("##PlannedRoute.RouteList"), szSelectedRoute.c_str()))
            {
                uint8_t iZoneID = m_pClient->GetRepresentZone(m_pClient->GetZone());

                RouteManager::RouteList pRouteList;
                if (pRouteManager && pRouteManager->GetRouteList(iZoneID, pRouteList))
                {
                    for (auto e : pRouteList)
                    {
                        const bool is_selected = (szSelectedRoute == e.first);

                        if (ImGui::Selectable(e.first.c_str(), is_selected))
                        {
                            m_pUserConfiguration->SetString(skCryptDec("Bot"), skCryptDec("SelectedRoute"), e.first.c_str());
                        }

                        if (is_selected)
                            ImGui::SetItemDefaultFocus();
                    }
                }
                

                ImGui::EndCombo();
            }

            if (m_pClient->IsRouting())
            {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.255f, 0.0f, 0.0f, 1.0f));
                if (ImGui::Button(skCryptDec("Stop"), ImVec2(129.0f, 0.0f)))
                {
                    m_pClient->ClearRoute();
                    m_pClient->StopMove();
                }
                ImGui::PopStyleColor(1);
            }
            else
            {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.255f, 0.0f, 1.0f));
                if (ImGui::Button(skCryptDec("Run"), ImVec2(129.0f, 0.0f)))
                {
                    uint8_t iZoneID = m_pClient->GetRepresentZone(m_pClient->GetZone());

                    RouteManager::RouteList pRouteList;
                    if (pRouteManager && pRouteManager->GetRouteList(iZoneID, pRouteList))
                    {
                        auto pRoute = pRouteList.find(szSelectedRoute);

                        if (pRoute != pRouteList.end())
                        {
                            m_pClient->SetRoute(pRoute->second);
                        }
                    }
                }
                ImGui::PopStyleColor(1);
            }

            ImGui::SameLine();

            if (ImGui::Button(skCryptDec("Planner"), ImVec2(129.0f, 0.0f)))
            {
                bDrawRoutePlanner = true;
            }

            if (ImGui::Button(skCryptDec("Clear"), ImVec2(266, 0.0f)))
            {
                m_pUserConfiguration->SetString(skCryptDec("Bot"), skCryptDec("SelectedRoute"), "");
            }
        }

        ImGui::Spacing();
        {
            ImGui::TextUnformatted(skCryptDec("Quick Action"));
            ImGui::Separator();

            if (ImGui::Button(skCryptDec("Town"), ImVec2(129.0f, 0.0f)))
            {
                m_pClient->SendTownPacket();
            }

            ImGui::SameLine();

            if (ImGui::Button(skCryptDec("Reload Skill"), ImVec2(129.0f, 0.0f)))
            {
                m_pUserConfiguration->SetInt(skCryptDec("Automation"), skCryptDec("AttackSkillList"), std::vector<int>());
                m_pUserConfiguration->SetInt(skCryptDec("Automation"), skCryptDec("CharacterSkillList"), std::vector<int>());

                m_pClient->LoadSkillData();
            }
        }

        ImGui::Spacing();
        {
            auto windowHeight = ImGui::GetWindowSize().y;
            auto fHeight = 30.0f;
            ImGui::SetCursorPosY((windowHeight - fHeight) * 0.97f);

            ImGui::TextUnformatted(skCryptDec("Automation"));
            ImGui::Separator();

            bool bAttackStatus = m_pUserConfiguration->GetBool(skCryptDec("Automation"), skCryptDec("Attack"), false);

            if (bAttackStatus)
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.255f, 0.0f, 1.0f));
            else
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.255f, 0.0f, 0.0f, 1.0f));

            if (ImGui::Button(skCryptDec("Attack"), ImVec2(129.0f, 0.0f)))
            {
                bAttackStatus = !bAttackStatus;
                m_pUserConfiguration->SetInt(skCryptDec("Automation"), skCryptDec("Attack"), bAttackStatus);
            }

            ImGui::PopStyleColor(1);
            ImGui::SameLine();

            bool bCharacterStatus = m_pUserConfiguration->GetBool(skCryptDec("Automation"), skCryptDec("Character"), false);

            if (bCharacterStatus)
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.255f, 0.0f, 1.0f));
            else
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.255f, 0.0f, 0.0f, 1.0f));

            if (ImGui::Button(skCryptDec("Character"), ImVec2(129.0f, 0.0f)))
            {
                bCharacterStatus = !bCharacterStatus;
                m_pUserConfiguration->SetInt(skCryptDec("Automation"), skCryptDec("Character"), bCharacterStatus);
            }

            ImGui::PopStyleColor(1);
        }
    }

    ImGui::EndChild();

    ImGui::SameLine();

    ImGui::BeginChild(2, ImVec2(350, 663), true);
    {
        if (ImGui::BeginTabBar(skCryptDec("##KOF.Toolbar"), ImGuiTabBarFlags_None))
        {
            if (ImGui::BeginTabItem(skCryptDec("Main")))
            {
                DrawMainProtectionArea();
                DrawMainFeaturesArea();
                DrawMainAutoLootArea();

                if (m_pClient->IsRogue())
                {
                    DrawMainRogueArea();
                }

                if (m_pClient->IsPriest())
                {
                    DrawMainPriestArea();
                }

                DrawMainTransformationArea();
                DrawMainSettingsArea();

                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem(skCryptDec("Attack")))
            {
                ImGui::Spacing();
                {
                    ImGui::TextUnformatted(skCryptDec("Configure Automated Attack"));
                    ImGui::Separator();

                    ImGui::Spacing();
                    {
                        bool bAutoTarget = m_pUserConfiguration->GetInt(skCryptDec("Attack"), skCryptDec("AutoTarget"), true);

                        if (ImGui::Checkbox(skCryptDec("##AutoTargetCheckbox"), &bAutoTarget))
                            m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("AutoTarget"), bAutoTarget ? 1 : 0);

                        ImGui::SameLine();

                        ImGui::Text(skCryptDec("Auto Target"));

                        ImGui::SameLine();

                        bool bRangeLimit = m_pUserConfiguration->GetBool(skCryptDec("Attack"), skCryptDec("RangeLimit"), false);

                        if (ImGui::Checkbox(skCryptDec("##RangeLimitCheckbox"), &bRangeLimit))
                            m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("RangeLimit"), bRangeLimit ? 1 : 0);

                        ImGui::SameLine();

                        ImGui::Text(skCryptDec("Search Range Limit"));

                        ImGui::SameLine();

                        ImGui::PushItemWidth(50);

                        int iRangeLimitValue = m_pUserConfiguration->GetInt(skCryptDec("Attack"), skCryptDec("RangeLimitValue"), (int)MAX_VIEW_RANGE);

                        if (ImGui::DragInt(skCryptDec("##RangeLimitValue"), &iRangeLimitValue, 1, 0, 100))
                            m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("RangeLimitValue"), iRangeLimitValue);

                        ImGui::PopItemWidth();
                    }

                    ImGui::Spacing();
                    {
                        bool bTargetSelectedWaitItDieForNew = m_pUserConfiguration->GetBool(skCryptDec("Attack"), skCryptDec("TargetSelectedWaitItDieForNew"), true);

                        if (ImGui::Checkbox(skCryptDec("##TargetSelectedWaitItDieForNew"), &bTargetSelectedWaitItDieForNew))
                            m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("TargetSelectedWaitItDieForNew"), bTargetSelectedWaitItDieForNew ? 1 : 0);

                        ImGui::SameLine();

                        ImGui::Text(skCryptDec("Target selected, wait it die for new"));
                    }

                    ImGui::Spacing();
                    {
                        bool bSearchTargetSpeed = m_pUserConfiguration->GetBool(skCryptDec("Attack"), skCryptDec("SearchTargetSpeed"), false);

                        if (ImGui::Checkbox(skCryptDec("##SearchTargetSpeedCheckbox"), &bSearchTargetSpeed))
                            m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("SearchTargetSpeed"), bSearchTargetSpeed ? 1 : 0);

                        ImGui::SameLine();

                        ImGui::Text(skCryptDec("Search Target Speed"));

                        ImGui::SameLine();

                        ImGui::PushItemWidth(75);

                        int iSearchTargetSpeedValue = m_pUserConfiguration->GetInt(skCryptDec("Attack"), skCryptDec("SearchTargetSpeedValue"), 100);

                        if (ImGui::DragInt(skCryptDec("##SearchTargetSpeedValue"), &iSearchTargetSpeedValue, 1, 0, 65535))
                            m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("SearchTargetSpeedValue"), iSearchTargetSpeedValue);

                        ImGui::PopItemWidth();
                    }

                    ImGui::Spacing();
                    {
                        bool bAttackSpeed = m_pUserConfiguration->GetBool(skCryptDec("Attack"), skCryptDec("AttackSpeed"), false);

                        if (ImGui::Checkbox(skCryptDec("##AttackSpeedCheckbox"), &bAttackSpeed))
                            m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("AttackSpeed"), bAttackSpeed ? 1 : 0);

                        ImGui::SameLine();

                        ImGui::Text(skCryptDec("Attack Speed"));

                        ImGui::SameLine();

                        ImGui::PushItemWidth(75);

                        int iAttackSpeedValue = m_pUserConfiguration->GetInt(skCryptDec("Attack"), skCryptDec("AttackSpeedValue"), 1000);

                        if (ImGui::DragInt(skCryptDec("##AttackSpeedValue"), &iAttackSpeedValue, 1, 0, 65535))
                            m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("AttackSpeedValue"), iAttackSpeedValue);

                        ImGui::PopItemWidth();
                    }

                    ImGui::Spacing();
                    {
                        bool bAttackRangeLimit = m_pUserConfiguration->GetBool(skCryptDec("Attack"), skCryptDec("AttackRangeLimit"), false);

                        if (ImGui::Checkbox(skCryptDec("##AttackRangeLimitCheckbox"), &bAttackRangeLimit))
                            m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("AttackRangeLimit"), bAttackRangeLimit ? 1 : 0);

                        ImGui::SameLine();

                        ImGui::Text(skCryptDec("Attack Range Limit"));

                        ImGui::SameLine();

                        ImGui::PushItemWidth(50);

                        int iAttackRangeLimitValue = m_pUserConfiguration->GetInt(skCryptDec("Attack"), skCryptDec("AttackRangeLimitValue"), (int)MAX_ATTACK_RANGE);

                        if (ImGui::DragInt(skCryptDec("##AttackRangeLimitValue"), &iAttackRangeLimitValue, 1, 0, 100))
                            m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("AttackRangeLimitValue"), iAttackRangeLimitValue);

                        ImGui::PopItemWidth();
                    }

                    bool bBasicAttack = m_pUserConfiguration->GetBool(skCryptDec("Attack"), skCryptDec("BasicAttack"), true);
                    bool bBasicAttackWithPacket = m_pUserConfiguration->GetBool(skCryptDec("Attack"), skCryptDec("BasicAttackWithPacket"), false);

                    ImGui::Spacing();
                    {
                        if (bBasicAttackWithPacket)
                        {
                            ImGui::BeginDisabled();
                        }

                        if (ImGui::Checkbox(skCryptDec("##BasicAttackCheckbox"), &bBasicAttack))
                            m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("BasicAttack"), bBasicAttack ? 1 : 0);

                        ImGui::SameLine();

                        ImGui::Text(skCryptDec("Basic Attack Legal (R)"));

                        if (bBasicAttackWithPacket)
                        {
                            ImGui::EndDisabled();
                        }

                        ImGui::SameLine();

                        bool bMoveToTarget = m_pUserConfiguration->GetBool(skCryptDec("Attack"), skCryptDec("MoveToTarget"), false);

                        if (ImGui::Checkbox(skCryptDec("##MoveToTargetCheckbox"), &bMoveToTarget))
                            m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("MoveToTarget"), bMoveToTarget ? 1 : 0);

                        ImGui::SameLine();

                        ImGui::Text(skCryptDec("Move To Target"));
                    }


                    ImGui::Spacing();
                    {
                        if (bBasicAttack)
                        {
                            ImGui::BeginDisabled();
                        }

                        if (ImGui::Checkbox(skCryptDec("##BasicAttackWithPacket"), &bBasicAttackWithPacket))
                            m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("BasicAttackWithPacket"), bBasicAttackWithPacket ? 1 : 0);

                        ImGui::SameLine();

                        ImGui::Text(skCryptDec("Basic Attack With Packet (R)"));

                        if (bBasicAttack)
                        {
                            ImGui::EndDisabled();
                        }
                    }
                    
                    ImGui::Spacing();
                    {
                       /* bool bRRCombo = m_pUserConfiguration->GetBool(skCryptDec("Attack"), skCryptDec("RRCombo"), true);

                        if (ImGui::Checkbox(skCryptDec("##RRCombo"), &bRRCombo))
                            m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("RRCombo"), bRRCombo ? 1 : 0);

                        ImGui::SameLine();

                        ImGui::Text(skCryptDec("RR Combo"));

                        ImGui::SameLine();*/

                        if (m_pClient->IsRogue())
                        {
                            bool bArcherCombo = m_pUserConfiguration->GetBool(skCryptDec("Attack"), skCryptDec("ArcherCombo"), true);

                            if (ImGui::Checkbox(skCryptDec("##ArcherComboCheckbox"), &bArcherCombo))
                                m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("ArcherCombo"), bArcherCombo ? 1 : 0);

                            ImGui::SameLine();

                            ImGui::Text(skCryptDec("Archer Combo"));
                        }
                    }

                    ImGui::Spacing();

                    DrawMonsterListTree();

                    ImGui::EndTabItem();
                }
            }

            if (ImGui::BeginTabItem(skCryptDec("Skill")))
            {
                ImGui::Spacing();
                {
                    ImGui::TextUnformatted(skCryptDec("Features"));
                    ImGui::Separator();

                    ImGui::Spacing();
                    {
                        bool bDisableCasting = m_pUserConfiguration->GetBool(skCryptDec("Feature"), skCryptDec("DisableCasting"), false);

                        if (ImGui::Checkbox(skCryptDec("##DisableCasting"), &bDisableCasting))
                        {
                            m_pClient->UpdateSkillSuccessRate(bDisableCasting);

                            m_pUserConfiguration->SetInt(skCryptDec("Feature"), skCryptDec("DisableCasting"), bDisableCasting ? 1 : 0);
                        }

                        ImGui::SameLine();
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255.0f, 0.0f, 0.0f, 1.0f));
                        ImGui::Text(skCryptDec("%%100 Skill Success Rate"));
                        ImGui::PopStyleColor();
                    }

                    ImGui::Spacing();
                    {
                        ImGui::TextUnformatted(skCryptDec("Settings"));
                        ImGui::Separator();

                        bool bUseHighLevelSkillFirst = m_pUserConfiguration->GetBool(skCryptDec("Settings"), skCryptDec("UseHighLevelSkillsFirst"), true);

                        if (ImGui::Checkbox(skCryptDec("##UseHighLevelSkillsFirst"), &bUseHighLevelSkillFirst))
                        {
                            m_pClient->UpdateSkillSuccessRate(bUseHighLevelSkillFirst);

                            m_pUserConfiguration->SetInt(skCryptDec("Settings"), skCryptDec("UseHighLevelSkillsFirst"), bUseHighLevelSkillFirst ? 1 : 0);
                        }

                        ImGui::SameLine();
                        ImGui::Text(skCryptDec("Use high-level skills first"));
                    }

                    DrawAutomatedAttackSkillTree();
                    DrawAutomatedCharacterSkillTree();

                    ImGui::EndTabItem();
                }
            }

            if (ImGui::BeginTabItem(skCryptDec("Supply")))
            {
                ImGui::Spacing();
                {
                    DrawMainSupplyArea();

                    ImGui::EndTabItem();
                }
            }

#ifdef DEVELOPER_ONLY
            if (ImGui::BeginTabItem(skCryptDec("Developer")))
            {
                ImGui::Spacing();
                {
                    DrawMainDeveloperOnlyArea();

                    ImGui::EndTabItem();
                }
            }
#endif

            ImGui::EndTabBar();
        }
    }

    ImGui::EndChild();
}

void Drawing::DrawMainProtectionArea()
{
    ImGui::Spacing();
    {
        ImGui::TextUnformatted(skCryptDec("Protection"));
        ImGui::Separator();

        ImGui::Spacing();
        {
            bool bHpProtection = m_pUserConfiguration->GetBool(skCryptDec("Protection"), skCryptDec("Hp"), false);

            if (ImGui::Checkbox(skCryptDec("##HpPotionCheckbox"), &bHpProtection))
                m_pUserConfiguration->SetInt(skCryptDec("Protection"), skCryptDec("Hp"), bHpProtection ? 1 : 0);

            ImGui::SameLine();

            ImGui::Text(skCryptDec("Hp Potion"));

            ImGui::SameLine();

            ImGui::PushItemWidth(50);

            int iHpProtectionValue = m_pUserConfiguration->GetInt(skCryptDec("Protection"), skCryptDec("HpValue"), 50);

            if (ImGui::DragInt(skCryptDec("##HpPotionValue"), &iHpProtectionValue, 1, 0, 100))
                m_pUserConfiguration->SetInt(skCryptDec("Protection"), skCryptDec("HpValue"), iHpProtectionValue);

            ImGui::PopItemWidth();

        }

        ImGui::Spacing();
        {
            bool bMpProtection = m_pUserConfiguration->GetBool(skCryptDec("Protection"), skCryptDec("Mp"), false);

            if (ImGui::Checkbox(skCryptDec("##MpPotionCheckbox"), &bMpProtection))
                m_pUserConfiguration->SetInt(skCryptDec("Protection"), skCryptDec("Mp"), bMpProtection ? 1 : 0);

            ImGui::SameLine();

            ImGui::Text(skCryptDec("Mp Potion"));

            ImGui::SameLine();

            ImGui::PushItemWidth(50);

            int iMpProtectionValue = m_pUserConfiguration->GetInt(skCryptDec("Protection"), skCryptDec("MpValue"), 25);

            if (ImGui::DragInt(skCryptDec("##MpPotionValue"), &iMpProtectionValue, 1, 0, 100))
                m_pUserConfiguration->SetInt(skCryptDec("Protection"), skCryptDec("MpValue"), iMpProtectionValue);

            ImGui::PopItemWidth();
        }

        if (m_pClient->IsRogue())
        {
            ImGui::Spacing();
            {
                bool bMinorProtection = m_pUserConfiguration->GetBool(skCryptDec("Protection"), skCryptDec("Minor"), false);

                if (ImGui::Checkbox(skCryptDec("##MinorCheckbox"), &bMinorProtection))
                    m_pUserConfiguration->SetInt(skCryptDec("Protection"), skCryptDec("Minor"), bMinorProtection ? 1 : 0);

                ImGui::SameLine();

                ImGui::Text(skCryptDec("Minor"));

                ImGui::SameLine();

                ImGui::PushItemWidth(50);

                int iMinorProtectionValue = m_pUserConfiguration->GetInt(skCryptDec("Protection"), skCryptDec("MinorValue"), 30);

                if (ImGui::DragInt(skCryptDec("##MinorValue"), &iMinorProtectionValue, 1, 0, 100))
                    m_pUserConfiguration->SetInt(skCryptDec("Protection"), skCryptDec("MinorValue"), iMinorProtectionValue);

                ImGui::PopItemWidth();


            }
        }
    }
}

void Drawing::DrawMainFeaturesArea()
{
    ImGui::Spacing();
    {
        ImGui::TextUnformatted(skCryptDec("Features"));
        ImGui::Separator();

        ImGui::Spacing();
        {
            if (Drawing::Bot->GetPlatformType() == PlatformType::CNKO)
            {
                bool bGodMode = m_pUserConfiguration->GetBool(skCryptDec("Protection"), skCryptDec("GodMode"), false);

                if (ImGui::Checkbox(skCryptDec("##GodMode"), &bGodMode))
                    m_pUserConfiguration->SetInt(skCryptDec("Protection"), skCryptDec("GodMode"), bGodMode ? 1 : 0);

                ImGui::SameLine();

                ImGui::Text(skCryptDec("God Mode"));

                ImGui::SameLine();
            }

            bool bWallHack = m_pUserConfiguration->GetBool(skCryptDec("Feature"), skCryptDec("WallHack"), false);

            if (ImGui::Checkbox(skCryptDec("##WallHack"), &bWallHack))
            {
                m_pClient->SetAuthority(bWallHack ? 0 : 1);

                m_pUserConfiguration->SetInt(skCryptDec("Feature"), skCryptDec("WallHack"), bWallHack ? 1 : 0);
            }

            ImGui::SameLine();

            ImGui::Text(skCryptDec("Wall Hack"));

            ImGui::SameLine();

            bool bDeathEffect = m_pUserConfiguration->GetBool(skCryptDec("Feature"), skCryptDec("DeathEffect"), false);

            if (ImGui::Checkbox(skCryptDec("##DeathEffect"), &bDeathEffect))
            {
                m_pClient->PatchDeathEffect(bDeathEffect);
                m_pUserConfiguration->SetInt(skCryptDec("Feature"), skCryptDec("DeathEffect"), bDeathEffect ? 1 : 0);
            }

            ImGui::SameLine();

            ImGui::Text(skCryptDec("Disable Death Effect"));

            if (Drawing::Bot->GetPlatformType() == PlatformType::CNKO)
            {
                bool bHyperNoah = m_pUserConfiguration->GetBool(skCryptDec("Feature"), skCryptDec("HyperNoah"), false);

                if (ImGui::Checkbox(skCryptDec("##HyperNoah"), &bHyperNoah))
                    m_pUserConfiguration->SetInt(skCryptDec("Feature"), skCryptDec("HyperNoah"), bHyperNoah ? 1 : 0);

                ImGui::SameLine();

                ImGui::Text(skCryptDec("Hyper Noah"));
            }

            
        }
    }
}

void Drawing::DrawMainAutoLootArea()
{
    ImGui::Spacing();
    {
        ImGui::TextUnformatted(skCryptDec("Auto Loot"));
        ImGui::Separator();

        ImGui::Spacing();
        {
            bool bAutoLoot = m_pUserConfiguration->GetBool(skCryptDec("AutoLoot"), skCryptDec("Enable"), false);

            if (ImGui::Checkbox(skCryptDec("##AutoLoot"), &bAutoLoot))
                m_pUserConfiguration->SetInt(skCryptDec("AutoLoot"), skCryptDec("Enable"), bAutoLoot ? 1 : 0);

            ImGui::SameLine();

            ImGui::Text(skCryptDec("Enable"));

            ImGui::SameLine();

            bool bMoveToLoot = m_pUserConfiguration->GetBool(skCryptDec("AutoLoot"), skCryptDec("MoveToLoot"), false);

            if (ImGui::Checkbox(skCryptDec("##MoveToLoot"), &bMoveToLoot))
            {
                m_pClient->SetAuthority(bMoveToLoot ? 0 : 1);
                m_pUserConfiguration->SetInt(skCryptDec("AutoLoot"), skCryptDec("MoveToLoot"), bMoveToLoot ? 1 : 0);
            }

            ImGui::SameLine();

            ImGui::Text(skCryptDec("Move To Loot"));
        }
    }

    ImGui::Spacing();
    {
        ImGui::Text(skCryptDec("Loot Min Price"));

        ImGui::SameLine();

        ImGui::PushItemWidth(100);

        int iLootMinPrice = m_pUserConfiguration->GetInt(skCryptDec("AutoLoot"), skCryptDec("MinPrice"), 0);

        if (ImGui::DragInt(skCryptDec("##LootMinPrice"), &iLootMinPrice, 1, 0, INT_MAX))
            m_pUserConfiguration->SetInt(skCryptDec("AutoLoot"), skCryptDec("MinPrice"), iLootMinPrice);

        ImGui::PopItemWidth();
    }
}

void Drawing::DrawMainDeveloperOnlyArea()
{
#ifdef DEVELOPER_ONLY
    ImGui::TextUnformatted(skCryptDec("PUS Bug"));
    ImGui::Separator();

    ImGui::Spacing();
    {
        if (ImGui::Button(skCryptDec("Inventory (1) -> MBag (1)"), ImVec2(300.0f, 0.0f)))
        {
            auto iInvenSlot0 = m_pClient->GetInventoryItemSlot(14);

            if (iInvenSlot0.iItemID != 0)
            {
                m_pClient->SendItemMovePacket(1, ITEM_INVEN_TO_MBAG, iInvenSlot0.iItemID, 0, 0);
                m_pClient->SendShoppingMall(ShoppingMallType::STORE_CLOSE);
            }
        }

        if (ImGui::Button(skCryptDec("Inventory (2) -> MBag (2)"), ImVec2(300.0f, 0.0f)))
        {
            auto iInvenSlot1 = m_pClient->GetInventoryItemSlot(15);

            if (iInvenSlot1.iItemID != 0)
            {
                m_pClient->SendItemMovePacket(1, ITEM_INVEN_TO_MBAG, iInvenSlot1.iItemID, 1, 1);
                m_pClient->SendShoppingMall(ShoppingMallType::STORE_CLOSE);
            }
        }

        ImGui::TextUnformatted(skCryptDec("Fake Item"));
        ImGui::Separator();

        if (ImGui::Button(skCryptDec("Dagger (+1) -> Oreads (Peri)"), ImVec2(300.0f, 0.0f)))
        {
            auto iDagger = m_pClient->GetInventoryItem(110110001); // +1 Dagger 

            if (iDagger.iItemID != 0)
            {
                m_pClient->SendItemMovePacket(1, ITEM_INVEN_INVEN, iDagger.iItemID, iDagger.iPos - 14, 35);
                m_pClient->SendShoppingMall(ShoppingMallType::STORE_CLOSE);
            }
        }

        if (ImGui::Button(skCryptDec("Inn Hostess (1) -> MBag Left"), ImVec2(300.0f, 0.0f)))
        {
            std::vector<TNpc> tmpVecNpc = m_pClient->m_vecNpc;

            auto findedNpc = std::find_if(tmpVecNpc.begin(), tmpVecNpc.end(),
                [&](const TNpc& a) { return a.iProtoID == 16096; });

            if (findedNpc != tmpVecNpc.end())
            {
                m_pClient->SendWarehouseGetIn(findedNpc->iID, 110110001, 0, 0, 54, 1);
            }
        }

        if (ImGui::Button(skCryptDec("Inn Hostess (2) -> MBag Right"), ImVec2(300.0f, 0.0f)))
        {
            std::vector<TNpc> tmpVecNpc = m_pClient->m_vecNpc;

            auto findedNpc = std::find_if(tmpVecNpc.begin(), tmpVecNpc.end(),
                [&](const TNpc& a) { return a.iProtoID == 16096; });

            if (findedNpc != tmpVecNpc.end())
            {
                m_pClient->SendWarehouseGetIn(findedNpc->iID, 110110001, 0, 1, 68, 1);
            }
        }

        ImGui::TextUnformatted(skCryptDec("Use skill with ID"));
        ImGui::Separator();

        ImGui::DragInt(skCryptDec("##UseSkillID"), &m_iSkillID, 1, 0, 10);

        if (ImGui::Button(skCryptDec("Use Skill"), ImVec2(300.0f, 0.0f)))
        {
            std::map<uint32_t, __TABLE_UPC_SKILL>* pSkillTable;
            if (Drawing::Bot->GetSkillTable(&pSkillTable))
            {
                auto pSkillData = pSkillTable->find(m_iSkillID);

                if (pSkillData != pSkillTable->end())
                {
                    if (m_pClient->GetTarget())
                    {
                        m_pClient->UseSkillWithPacket(pSkillData->second, m_pClient->GetTarget());
                    }
                    else
                    {
                        m_pClient->UseSkillWithPacket(pSkillData->second, m_pClient->GetID());
                    }            
                }
            }
        }
    }
#endif
}
void Drawing::DrawMainSupplyArea()
{
    ImGui::TextUnformatted(skCryptDec("Supply Management"));
    ImGui::Separator();

    ImGui::Spacing();
    {
        bool bAutoRepair = m_pUserConfiguration->GetBool(skCryptDec("Supply"), skCryptDec("AutoRepair"), false);

        if (ImGui::Checkbox(skCryptDec("##AutoRepair"), &bAutoRepair))
            m_pUserConfiguration->SetInt(skCryptDec("Supply"), skCryptDec("AutoRepair"), bAutoRepair ? 1 : 0);

        ImGui::SameLine();

        ImGui::Text(skCryptDec("Auto Repair"));

        ImGui::SameLine();

        bool bAutoSupply = m_pUserConfiguration->GetBool(skCryptDec("Supply"), skCryptDec("AutoSupply"), false);

        if (ImGui::Checkbox(skCryptDec("##AutoSupply"), &bAutoSupply))
            m_pUserConfiguration->SetInt(skCryptDec("Supply"), skCryptDec("AutoSupply"), bAutoSupply ? 1 : 0);

        ImGui::SameLine();

        ImGui::Text(skCryptDec("Auto Supply"));
    }

    ImGui::Spacing();
    {
        ImGui::TextUnformatted(skCryptDec("Supply List"));
        ImGui::Separator();

        ImGui::Spacing();
        {
            std::vector<int> vecSupplyList = m_pUserConfiguration->GetInt(skCryptDec("Supply"), skCryptDec("Enable"), std::vector<int>());

            auto jSupplyList = Drawing::Bot->GetSupplyList();

            for (size_t i = 0; i < jSupplyList.size(); i++)
            {
                ImGui::PushID(i);

                std::string szItemIdAttribute = skCryptDec("itemid");
                std::string szNameAttribute = skCryptDec("name");
                std::string szCountAttribute = skCryptDec("count");

                bool bSelected = std::find(vecSupplyList.begin(), vecSupplyList.end(), jSupplyList[i][szItemIdAttribute.c_str()].get<int>()) != vecSupplyList.end();

                if (ImGui::Checkbox(skCryptDec("##Enable"), &bSelected))
                {
                    if (bSelected)
                        vecSupplyList.push_back(jSupplyList[i][szItemIdAttribute.c_str()].get<int>());
                    else
                        vecSupplyList.erase(std::find(vecSupplyList.begin(), vecSupplyList.end(), jSupplyList[i][szItemIdAttribute.c_str()].get<int>()));

                    m_pUserConfiguration->SetInt(skCryptDec("Supply"), skCryptDec("Enable"), vecSupplyList);
                }

                ImGui::SameLine();

                ImGui::Text(jSupplyList[i][szNameAttribute.c_str()].get<std::string>().c_str());

                ImGui::SameLine();

                int iCount = m_pUserConfiguration->GetInt(skCryptDec("Supply"), std::to_string(jSupplyList[i][szItemIdAttribute.c_str()].get<int>()).c_str(), jSupplyList[i][szCountAttribute.c_str()].get<int>());

                ImGui::SetCursorPosX(205);
                ImGui::PushItemWidth(133);
                if (ImGui::DragInt(skCryptDec("##Count"), &iCount, 1, 1, 9998))
                {
                    m_pUserConfiguration->SetInt(skCryptDec("Supply"), std::to_string(jSupplyList[i][szItemIdAttribute.c_str()].get<int>()).c_str(), iCount);
                }

                ImGui::PopID();
            }
        }
    }
}

void Drawing::DrawMainRogueArea()
{
    ImGui::Spacing();
    {
        ImGui::TextUnformatted(skCryptDec("Rogue Management"));
        ImGui::Separator();

        ImGui::Spacing();
        {
            bool bPartySwift = m_pUserConfiguration->GetBool(skCryptDec("Rogue"), skCryptDec("PartySwift"), true);

            if (ImGui::Checkbox(skCryptDec("##RoguePartySwift"), &bPartySwift))
                m_pUserConfiguration->SetInt(skCryptDec("Rogue"), skCryptDec("PartySwift"), bPartySwift ? 1 : 0);

            ImGui::SameLine();

            ImGui::Text(skCryptDec("Party Swift"));
        }
    }
}

void Drawing::DrawMainPriestArea()
{
    ImGui::Spacing();
    {
        ImGui::TextUnformatted(skCryptDec("Priest Management"));
        ImGui::Separator();

        bool bAutoHealthBuff = m_pUserConfiguration->GetBool(skCryptDec("Priest"), skCryptDec("AutoHealthBuff"), true);

        if (ImGui::Checkbox(skCryptDec("##AutoHealthBuff"), &bAutoHealthBuff))
            m_pUserConfiguration->SetInt(skCryptDec("Priest"), skCryptDec("AutoHealthBuff"), bAutoHealthBuff ? 1 : 0);

        ImGui::SameLine();

        ImGui::Text(skCryptDec("Auto Health Buff"));

        std::string szSelectedHealthBuff = "Auto";
        int iSelectedHealthBuff = m_pUserConfiguration->GetInt(skCryptDec("Priest"), skCryptDec("SelectedHealthBuff"), -1);

        auto jHealthBuffList = Drawing::Bot->GetHealthBuffList();

        std::map<int, std::string> mapHealthBuffList;

        for (size_t i = 0; i < jHealthBuffList.size(); i++)
        {
            int iId = jHealthBuffList[i]["id"].get<int>();

            if (iId == iSelectedHealthBuff)
            {
                szSelectedHealthBuff = jHealthBuffList[i]["name"].get<std::string>();
            }

            mapHealthBuffList.insert(std::make_pair(iId, jHealthBuffList[i]["name"].get<std::string>()));
        }

        ImGui::SameLine();
        ImGui::SetNextItemWidth(185);

        if (ImGui::BeginCombo(skCryptDec("##PriestManagement.HealthBuffList"), szSelectedHealthBuff.c_str()))
        {
            for (auto e : mapHealthBuffList)
            {
                const bool is_selected = (iSelectedHealthBuff == e.first);

                if (ImGui::Selectable(e.second.c_str(), is_selected))
                {
                    m_pUserConfiguration->SetInt(skCryptDec("Priest"), skCryptDec("SelectedHealthBuff"), e.first);
                }

                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }

            ImGui::EndCombo();
        }

        bool bAutoDefenceBuff = m_pUserConfiguration->GetBool(skCryptDec("Priest"), skCryptDec("AutoDefenceBuff"), true);

        if (ImGui::Checkbox(skCryptDec("##AutoDefenceBuff"), &bAutoDefenceBuff))
            m_pUserConfiguration->SetInt(skCryptDec("Priest"), skCryptDec("AutoDefenceBuff"), bAutoDefenceBuff ? 1 : 0);

        ImGui::SameLine();

        ImGui::Text(skCryptDec("Auto Defence Buff"));

        std::string szSelectedDefenceBuff = "Auto";
        int iSelectedDefenceBuff = m_pUserConfiguration->GetInt(skCryptDec("Priest"), skCryptDec("SelectedDefenceBuff"), -1);

        auto jDefenceBuffList = Drawing::Bot->GetDefenceBuffList();

        std::map<int, std::string> mapDefenceBuffList;

        for (size_t i = 0; i < jDefenceBuffList.size(); i++)
        {
            int iId = jDefenceBuffList[i]["id"].get<int>();

            if (iId == iSelectedDefenceBuff)
            {
                szSelectedDefenceBuff = jDefenceBuffList[i]["name"].get<std::string>();
            }

            mapDefenceBuffList.insert(std::make_pair(iId, jDefenceBuffList[i]["name"].get<std::string>()));
        }

        ImGui::SameLine();
        ImGui::SetNextItemWidth(178);

        if (ImGui::BeginCombo(skCryptDec("##PriestManagement.DefenceBuffList"), szSelectedDefenceBuff.c_str()))
        {
            for (auto e : mapDefenceBuffList)
            {
                const bool is_selected = (iSelectedDefenceBuff == e.first);

                if (ImGui::Selectable(e.second.c_str(), is_selected))
                {
                    m_pUserConfiguration->SetInt(skCryptDec("Priest"), skCryptDec("SelectedDefenceBuff"), e.first);
                }

                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }

            ImGui::EndCombo();
        }

        bool bAutoMindBuff = m_pUserConfiguration->GetBool(skCryptDec("Priest"), skCryptDec("AutoMindBuff"), true);

        if (ImGui::Checkbox(skCryptDec("##AutoMindBuff"), &bAutoMindBuff))
            m_pUserConfiguration->SetInt(skCryptDec("Priest"), skCryptDec("AutoMindBuff"), bAutoMindBuff ? 1 : 0);

        ImGui::SameLine();

        ImGui::Text(skCryptDec("Auto Mind Buff"));

        std::string szSelectedMindBuff = "Auto";
        int iSelectedMindBuff = m_pUserConfiguration->GetInt(skCryptDec("Priest"), skCryptDec("SelectedMindBuff"), -1);

        auto jMindBuffList = Drawing::Bot->GetMindBuffList();

        std::map<int, std::string> mapMindBuffList;

        for (size_t i = 0; i < jMindBuffList.size(); i++)
        {
            int iId = jMindBuffList[i]["id"].get<int>();

            if (iId == iSelectedMindBuff)
            {
                szSelectedMindBuff = jMindBuffList[i]["name"].get<std::string>();
            }

            mapMindBuffList.insert(std::make_pair(iId, jMindBuffList[i]["name"].get<std::string>()));
        }

        ImGui::SameLine();
        ImGui::SetNextItemWidth(199);

        if (ImGui::BeginCombo(skCryptDec("##PriestManagement.MindBuffList"), szSelectedMindBuff.c_str()))
        {
            for (auto e : mapMindBuffList)
            {
                const bool is_selected = (iSelectedMindBuff == e.first);

                if (ImGui::Selectable(e.second.c_str(), is_selected))
                {
                    m_pUserConfiguration->SetInt(skCryptDec("Priest"), skCryptDec("SelectedMindBuff"), e.first);
                }

                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }

            ImGui::EndCombo();
        }

        bool bAutoHeal = m_pUserConfiguration->GetBool(skCryptDec("Priest"), skCryptDec("AutoHeal"), true);

        if (ImGui::Checkbox(skCryptDec("##AutoHeal"), &bAutoHeal))
            m_pUserConfiguration->SetInt(skCryptDec("Priest"), skCryptDec("AutoHeal"), bAutoHeal ? 1 : 0);

        ImGui::SameLine();

        ImGui::Text(skCryptDec("Auto Heal"));

        ImGui::SameLine();

        ImGui::PushItemWidth(50);

        int iAutoHealValue = m_pUserConfiguration->GetInt(skCryptDec("Priest"), skCryptDec("AutoHealValue"), 75);

        if (ImGui::DragInt(skCryptDec("##AutoHealValue"), &iAutoHealValue, 1, 0, 100))
            m_pUserConfiguration->SetInt(skCryptDec("Priest"), skCryptDec("AutoHealValue"), iAutoHealValue);

        ImGui::PopItemWidth();

        std::string szSelectedHeal = "Auto";
        int iSelectedHeal = m_pUserConfiguration->GetInt(skCryptDec("Priest"), skCryptDec("SelectedHeal"), -1);

        auto jHealList = Drawing::Bot->GetHealList();

        std::map<int, std::string> mapHealList;

        for (size_t i = 0; i < jHealList.size(); i++)
        {
            int iId = jHealList[i]["id"].get<int>();

            if (iId == iSelectedHeal)
            {
                szSelectedHeal = jHealList[i]["name"].get<std::string>();
            }

            mapHealList.insert(std::make_pair(iId, jHealList[i]["name"].get<std::string>()));
        }

        ImGui::SameLine();
        ImGui::SetNextItemWidth(176);

        if (ImGui::BeginCombo(skCryptDec("##PriestManagement.HealList"), szSelectedHeal.c_str()))
        {
            for (auto e : mapHealList)
            {
                const bool is_selected = (iSelectedHeal == e.first);

                if (ImGui::Selectable(e.second.c_str(), is_selected))
                {
                    m_pUserConfiguration->SetInt(skCryptDec("Priest"), skCryptDec("SelectedHeal"), e.first);
                }

                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }

            ImGui::EndCombo();
        }

        bool bAutoStrength = m_pUserConfiguration->GetBool(skCryptDec("Priest"), skCryptDec("AutoStrength"), true);

        if (ImGui::Checkbox(skCryptDec("##AutoStrength"), &bAutoStrength))
            m_pUserConfiguration->SetInt(skCryptDec("Priest"), skCryptDec("AutoStrength"), bAutoStrength ? 1 : 0);

        ImGui::SameLine();

        ImGui::Text(skCryptDec("Auto Strength"));

        bool bDebuffProtection = m_pUserConfiguration->GetBool(skCryptDec("Priest"), skCryptDec("DebuffProtection"), true);

        if (ImGui::Checkbox(skCryptDec("##DebuffProtection"), &bDebuffProtection))
            m_pUserConfiguration->SetInt(skCryptDec("Priest"), skCryptDec("DebuffProtection"), bDebuffProtection ? 1 : 0);

        ImGui::SameLine();

        ImGui::Text(skCryptDec("Debuff Protection (Cure)"));

        bool bPartyProtection = m_pUserConfiguration->GetBool(skCryptDec("Priest"), skCryptDec("PartyProtection"), true);

        if (ImGui::Checkbox(skCryptDec("##PartyProtection"), &bPartyProtection))
            m_pUserConfiguration->SetInt(skCryptDec("Priest"), skCryptDec("PartyProtection"), bPartyProtection ? 1 : 0);

        ImGui::SameLine();

        ImGui::Text(skCryptDec("Party Protection"));
    }
}

void Drawing::DrawMainTransformationArea()
{
    std::map<uint32_t, __TABLE_DISGUISE_RING>* mapDisguiseTable;
    if (!Drawing::Bot->GetDisguiseRingTable(&mapDisguiseTable))
        return;

    ImGui::Spacing();
    {
        ImGui::TextUnformatted(skCryptDec("Transformation"));
        ImGui::Separator();

        ImGui::Spacing();
        {
            bool bAutoTransformation = m_pUserConfiguration->GetBool(skCryptDec("Transformation"), skCryptDec("Auto"), false);

            if (ImGui::Checkbox(skCryptDec("##AutoTransformationCheckBox"), &bAutoTransformation))
                m_pUserConfiguration->SetInt(skCryptDec("Transformation"), skCryptDec("Auto"), bAutoTransformation ? 1 : 0);

            ImGui::SameLine();

            ImGui::Text(skCryptDec("Auto"));

            ImGui::SameLine();
            ImGui::SetNextItemWidth(272);

            struct DisguiseItem
            {
                uint32_t iID;
                std::string szName;
            };

            std::vector<DisguiseItem> vecDisguiseItems;

            vecDisguiseItems.push_back(DisguiseItem(381001000, "Transformation Scroll"));
            vecDisguiseItems.push_back(DisguiseItem(379090000, "Transformation Totem 1"));
            vecDisguiseItems.push_back(DisguiseItem(379093000, "Transformation Totem 2"));

            int iTransformationItem = m_pUserConfiguration->GetInt(skCryptDec("Transformation"), skCryptDec("Item"), 381001000);

            std::string szSelectedTransformationItem = "Transformation Scroll";

            const auto pFindedTransformationItem = std::find_if(vecDisguiseItems.begin(), vecDisguiseItems.end(),
                [&](const DisguiseItem& a) { return a.iID == iTransformationItem; });

            if (pFindedTransformationItem != vecDisguiseItems.end())
                szSelectedTransformationItem = pFindedTransformationItem->szName;

            if (ImGui::BeginCombo(skCryptDec("##Transformation.ItemList"), szSelectedTransformationItem.c_str()))
            {
                for (auto& e : vecDisguiseItems)
                {
                    const bool bIsSelected = iTransformationItem == e.iID;

                    if (ImGui::Selectable(e.szName.c_str(), bIsSelected))
                    {
                        m_pUserConfiguration->SetInt(skCryptDec("Transformation"), skCryptDec("Item"), e.iID);
                        m_pUserConfiguration->SetInt(skCryptDec("Transformation"), skCryptDec("Skill"), 0);
                    }

                    if (bIsSelected)
                        ImGui::SetItemDefaultFocus();
                }

                ImGui::EndCombo();
            }

            ImGui::SetNextItemWidth(335);

            struct DisguiseSkill
            {
                uint32_t iID;
                std::string szName;
            };

            std::vector<DisguiseSkill> vecDisguiseSkills;

            int iTransformationSkill = m_pUserConfiguration->GetInt(skCryptDec("Transformation"), skCryptDec("Skill"), 472020);

            std::string szSelectedTransformationSkill = "";

            for (auto& [k, v] : *mapDisguiseTable)
            {
                if (m_pClient->GetLevel() < v.iRequiredLevel)
                    continue;

                if (iTransformationItem != v.iItemID)
                    continue;

                if (iTransformationSkill == v.iSkillID)
                {
                    szSelectedTransformationSkill = v.szName;
                }

                vecDisguiseSkills.push_back(DisguiseSkill(v.iSkillID, v.szName));
            }

            if (ImGui::BeginCombo(skCryptDec("##Transformation.SkillList"), szSelectedTransformationSkill.c_str()))
            {
                for (auto& e : vecDisguiseSkills)
                {
                    const bool bIsSelected = iTransformationSkill == e.iID;

                    if (ImGui::Selectable(e.szName.c_str(), bIsSelected))
                    {
                        m_pUserConfiguration->SetInt(skCryptDec("Transformation"), skCryptDec("Skill"), e.iID);
                    }

                    if (bIsSelected)
                        ImGui::SetItemDefaultFocus();
                }

                ImGui::EndCombo();
            }
        }
    }
}

void Drawing::DrawMainSettingsArea()
{
    ImGui::Spacing();
    {
        ImGui::TextUnformatted(skCryptDec("Settings"));
        ImGui::Separator();

        ImGui::Spacing();
        {
            bool bStopBotIfDead = m_pUserConfiguration->GetBool(skCryptDec("Settings"), skCryptDec("StopBotIfDead"), false);

            if (ImGui::Checkbox(skCryptDec("##StopBotIfDead"), &bStopBotIfDead))
                m_pUserConfiguration->SetInt(skCryptDec("Settings"), skCryptDec("StopBotIfDead"), bStopBotIfDead ? 1 : 0);

            ImGui::SameLine();

            ImGui::Text(skCryptDec("Stop bot if character dead"));

            bool bStopBotIfTeleported = m_pUserConfiguration->GetBool(skCryptDec("Settings"), skCryptDec("StopBotIfTeleported"), false);

            if (ImGui::Checkbox(skCryptDec("##StopBotIfTeleported"), &bStopBotIfTeleported))
                m_pUserConfiguration->SetInt(skCryptDec("Settings"), skCryptDec("StopBotIfTeleported"), bStopBotIfTeleported ? 1 : 0);

            ImGui::SameLine();

            ImGui::Text(skCryptDec("Stop bot if character teleported"));
        }
    }
}

void Drawing::DrawAutomatedAttackSkillTree()
{
    ImGui::Spacing();
    {
        ImGui::TextUnformatted(skCryptDec("Select automated attack or character skill"));
        ImGui::Separator();

        std::vector<int> vecAttackList = m_pUserConfiguration->GetInt(skCryptDec("Automation"), skCryptDec("AttackSkillList"), std::vector<int>());

        std::stringstream strTreeText;

        if (!Drawing::Bot->IsTableLoaded())
            strTreeText << "|/-\\"[(int)(ImGui::GetTime() / 0.05f) & 3] << " ";

        strTreeText << skCryptDec("Automated attack skills");

        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_DefaultOpen;
        if (ImGui::TreeNodeEx(strTreeText.str().c_str(), flags))
        {
            std::vector<__TABLE_UPC_SKILL>* vecAvailableSkills;
            if (m_pClient->GetAvailableSkill(&vecAvailableSkills))
            {
                for (const auto& x : *vecAvailableSkills)
                {
                    if (x.iTarget != SkillTargetType::TARGET_ENEMY_ONLY && x.iTarget != SkillTargetType::TARGET_AREA_ENEMY)
                        continue;

                    ImGui::PushID(x.iID);

                    bool bSelected = std::find(vecAttackList.begin(), vecAttackList.end(), x.iID) != vecAttackList.end();

                    if (ImGui::Selectable(x.szName.c_str(), &bSelected))
                    {
                        if (bSelected)
                            vecAttackList.push_back(x.iID);
                        else
                            vecAttackList.erase(std::find(vecAttackList.begin(), vecAttackList.end(), x.iID));

                        m_pUserConfiguration->SetInt(skCryptDec("Automation"), skCryptDec("AttackSkillList"), vecAttackList);
                    }

                    /*if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                        ImGui::SetTooltip(x.szDesc.c_str());*/

                    ImGui::PopID();
                }
            }

            ImGui::TreePop();
        }
    }
}

void Drawing::DrawAutomatedCharacterSkillTree()
{
    std::vector<int> vecCharacterSkillList = m_pUserConfiguration->GetInt(skCryptDec("Automation"), skCryptDec("CharacterSkillList"), std::vector<int>());

    std::stringstream strTreeText;

    if (!Drawing::Bot->IsTableLoaded())
        strTreeText << "|/-\\"[(int)(ImGui::GetTime() / 0.05f) & 3] << " ";

    strTreeText << skCryptDec("Automated character skills");

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Bullet;
    if (ImGui::TreeNodeEx(strTreeText.str().c_str(), flags))
    {
        std::vector<__TABLE_UPC_SKILL>* vecAvailableSkills;
        if (m_pClient->GetAvailableSkill(&vecAvailableSkills))
        {
            for (const auto& x : *vecAvailableSkills)
            {
                if (x.iTarget != SkillTargetType::TARGET_SELF 
                    && x.iTarget != SkillTargetType::TARGET_PARTY 
                    && x.iTarget != SkillTargetType::TARGET_PARTY_ALL 
                    && x.iTarget != SkillTargetType::TARGET_FRIEND_WITHME
                    && x.iTarget != SkillTargetType::TARGET_FRIEND_ONLY)
                    continue;

                ImGui::PushID(x.iID);

                bool bSelected = std::find(vecCharacterSkillList.begin(), vecCharacterSkillList.end(), x.iID) != vecCharacterSkillList.end();

                if (ImGui::Selectable(x.szName.c_str(), &bSelected))
                {
                    if (bSelected)
                        vecCharacterSkillList.push_back(x.iID);
                    else
                        vecCharacterSkillList.erase(std::find(vecCharacterSkillList.begin(), vecCharacterSkillList.end(), x.iID));

                    m_pUserConfiguration->SetInt(skCryptDec("Automation"), skCryptDec("CharacterSkillList"), vecCharacterSkillList);
                }

                /*if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                    ImGui::SetTooltip(x.szDesc.c_str());*/

                ImGui::PopID();
            }
        }

        ImGui::TreePop();
    }
}

void Drawing::DrawMonsterListTree()
{
    bool bRangeLimit = m_pUserConfiguration->GetBool(skCryptDec("Attack"), skCryptDec("RangeLimit"), false);
    int iRangeLimitValue = m_pUserConfiguration->GetInt(skCryptDec("Attack"), skCryptDec("RangeLimitValue"), (int)MAX_VIEW_RANGE);
    bool bAutoTarget = m_pUserConfiguration->GetInt(skCryptDec("Attack"), skCryptDec("AutoTarget"), true);

    ImGui::TextUnformatted(skCryptDec("Select attackable target"));
    ImGui::Separator();

    if (bAutoTarget)
        ImGui::BeginDisabled();

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Bullet;
    if (ImGui::TreeNodeEx(skCryptDec("Nearby Entity List"), flags))
    {
        std::vector<SNpcData> vecTargetList;
        std::vector<int> vecSelectedNpcList = m_pUserConfiguration->GetInt(skCryptDec("Attack"), skCryptDec("NpcList"), std::vector<int>());

        for (const auto& x : vecSelectedNpcList)
        {
            SNpcData pNpcData;

            pNpcData.iProtoID = x;

            vecTargetList.push_back(pNpcData);
        }

        std::lock_guard<std::recursive_mutex> lock(m_pClient->m_mutexNpc);
        for (const auto& x : m_pClient->m_vecNpc)
        {
            if (x.iMonsterOrNpc != 1)
                continue;

            const auto pFindedNpc = std::find_if(vecTargetList.begin(), vecTargetList.end(),
                [x](const SNpcData& a) { return a.iProtoID == x.iProtoID; });

            if (pFindedNpc != vecTargetList.end())
                continue;

            SNpcData pNpcData;

            pNpcData.iProtoID = x.iProtoID;

            vecTargetList.push_back(pNpcData);
        }

        for (const auto& x : vecTargetList)
        {
            ImGui::PushID(x.iProtoID);

            bool bSelected = std::find(vecSelectedNpcList.begin(), vecSelectedNpcList.end(), x.iProtoID) != vecSelectedNpcList.end();

            std::string szNpcName = skCryptDec("~Unknown~");

            std::map<uint32_t, __TABLE_NPC>* pNpcTable;
            if (Drawing::Bot->GetNpcTable(&pNpcTable))
            {
                auto pNpcInfo = pNpcTable->find(x.iProtoID);

                if (pNpcInfo != pNpcTable->end())
                    szNpcName = pNpcInfo->second.szText;
            }

            std::map<uint32_t, __TABLE_MOB_USKO>* pMobTable;
            if (Drawing::Bot->GetMobTable(&pMobTable))
            {
                auto pMobInfo = pMobTable->find(x.iProtoID);

                if (pMobInfo != pMobTable->end())
                    szNpcName = pMobInfo->second.szText;
            }

            if (ImGui::Selectable(szNpcName.c_str(), &bSelected))
            {
                if (bSelected)
                    vecSelectedNpcList.push_back(x.iProtoID);
                else
                    vecSelectedNpcList.erase(std::find(vecSelectedNpcList.begin(), vecSelectedNpcList.end(), x.iProtoID));

                m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("NpcList"), vecSelectedNpcList);
            }

            ImGui::PopID();
        }

        ImGui::TreePop();
    }

    if (bAutoTarget)
        ImGui::EndDisabled();
}