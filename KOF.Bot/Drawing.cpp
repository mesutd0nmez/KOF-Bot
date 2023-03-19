#include "pch.h"
#include "Drawing.h"
#include "Bot.h"
#include "Client.h"
#include "ClientHandler.h"
#include "UI.h"
#include "Guard.h"
#include "RouteManager.h"

Bot* Drawing::Bot = nullptr;
bool Drawing::bDraw = true;
bool Drawing::bDrawRoutePlanner = false;
bool Drawing::Done = false;

ClientHandler* m_pClient = nullptr;
Ini* m_pConfiguration = nullptr;

WorldData* m_pWorldData = nullptr;

ID3D11ShaderResourceView* m_pMapTexture = nullptr;
ID3D11ShaderResourceView* m_pMinimapTexture = nullptr;

char m_szRouteName[255] = "";

std::string m_szSelectedRoute = "";
std::vector<Route> m_vecRoute;

float m_fScreenWidth = 0.0f;
float m_fScreenHeight = 0.0f;

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
    m_pConfiguration = Drawing::Bot->GetConfiguration();
    m_pClient = Drawing::Bot->GetClientHandler();

    if (m_pWorldData == nullptr || (m_pWorldData != nullptr && m_pWorldData->iId != m_pClient->GetZone()))
    {
        World* pWorld = Drawing::Bot->GetWorld();

        if (pWorld)
        {
            m_pWorldData = pWorld->GetWorldData(m_pClient->GetZone());
        } 

        if (m_pWorldData != nullptr)
        {
            UI::LoadTextureFromMemory(m_pWorldData->pMiniMapImageData, &m_pMinimapTexture, m_pWorldData->iMiniMapImageWidth, m_pWorldData->iMiniMapImageHeight);
            UI::LoadTextureFromMemory(m_pWorldData->pMapImageData, &m_pMapTexture, m_pWorldData->iMapImageWidth, m_pWorldData->iMapImageHeight);
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

        std::string lpWindowName = skCryptEnc("KOF.Bot");
       
        ImVec2 vWindowSize = { 658, 700 };
        ImGuiWindowFlags WindowFlags = ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize;

        ImVec2 vec2InitialPos = { m_fScreenWidth, m_fScreenHeight };

        vec2InitialPos.x -= vWindowSize.x / 2;
        vec2InitialPos.y -= vWindowSize.y / 2;

		ImGui::SetNextWindowPos(vec2InitialPos, ImGuiCond_Once);
		ImGui::SetNextWindowSize(vWindowSize);
		ImGui::SetNextWindowBgAlpha(1.0f);
		ImGui::Begin(lpWindowName.c_str(), &bDraw, WindowFlags);
		{
            DrawGameController();
		}

        ImGui::End();
	}

#ifdef _WINDLL
	if (GetAsyncKeyState(VK_INSERT) & 1)
		bDraw = !bDraw;
#endif
}

void Drawing::DrawRoutePlanner()
{
    if (isActive())
    {
        InitializeSceneData();

        if (!Drawing::bDrawRoutePlanner)
            return;

        std::string lpWindowName = skCryptEnc("KOF.RoutePlanner");

        ImGuiWindowFlags WindowFlags = ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize;

        ImVec2 vec2InitialPos = { m_fScreenWidth, m_fScreenHeight };

        ImVec2 vWindowSize = { 1030, 855 };

        vec2InitialPos.x -= vWindowSize.x / 2;
        vec2InitialPos.y -= vWindowSize.y / 2;

        ImGui::SetNextWindowPos(vec2InitialPos, ImGuiCond_Once);
        ImGui::SetNextWindowSize(vWindowSize);
        ImGui::SetNextWindowBgAlpha(1.0f);
        ImGui::Begin(lpWindowName.c_str(), &Drawing::bDrawRoutePlanner, WindowFlags);
        {
            DrawRoutePlannerArea();
        }

        ImGui::End();
    }

#ifdef _WINDLL
    if (GetAsyncKeyState(VK_INSERT) & 1)
        bDraw = !bDraw;
#endif
}

void Drawing::DrawRoutePlannerArea()
{
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

        Vector3 vec3CurrentPosition = m_pClient->GetPosition();

        ImVec2 currentPosition = ImVec2(
            pOffsetPosition.x + std::ceil(vec3CurrentPosition.m_fX / (float)(m_pWorldData->fMapLength / m_pWorldData->iMapImageWidth)),
            pOffsetPosition.y + std::ceil(m_pWorldData->iMapImageHeight - (vec3CurrentPosition.m_fY / (float)(m_pWorldData->fMapLength / m_pWorldData->iMapImageHeight))));

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
            }
        }

    }
    ImGui::EndChild();

    ImGui::SameLine();

    ImGui::BeginChild(skCryptDec("RoutePlanner.Controller"), ImVec2(190, 817), true);
    {
        RouteManager* pRouteManager = Drawing::Bot->GetRouteManager();

        RouteManager::RouteList pRouteList;
        if (pRouteManager && pRouteManager->GetRouteList(m_pClient->GetZone(), pRouteList))
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
                    pRouteManager->Delete(m_szSelectedRoute, m_pClient->GetZone());

                    std::string szSelectedRouteConfiguration = Drawing::Bot->GetConfiguration()->GetString(skCryptDec("Bot"), skCryptDec("SelectedRoute"), "");

                    if (szSelectedRouteConfiguration == m_szSelectedRoute)
                    {
                        Drawing::Bot->GetConfiguration()->SetString(skCryptDec("Bot"), skCryptDec("SelectedRoute"), "");
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
                    m_pClient->SetMovePosition(Vector3(0.0f, 0.0f, 0.0f));
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
                    pRouteManager->Save(m_szRouteName, m_pClient->GetZone(), m_vecRoute);
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

                    Guard npcListLock(m_pClient->m_vecNpcLock);
                    std::vector<TNpc>* vecNpcList;
                    if (m_pClient->GetNpcList(&vecNpcList))
                    {
                        for (const TNpc& pNpc : *vecNpcList)
                        {
                            if (m_pClient->GetDistance(pNpc.fX, pNpc.fY) > MAX_VIEW_RANGE)
                                continue;

                            ImVec2 pNpcPosition = ImVec2(
                                pOffsetPosition.x + std::ceil(pNpc.fX / (float)(m_pWorldData->fMapLength / m_pWorldData->iMiniMapImageWidth)),
                                pOffsetPosition.y + std::ceil(m_pWorldData->iMiniMapImageHeight - (pNpc.fY / (float)(m_pWorldData->fMapLength / m_pWorldData->iMiniMapImageHeight))));

                            if (pNpc.iMonsterOrNpc == 1)
                                ImGui::GetWindowDrawList()->AddCircle(pNpcPosition, 1.0f, IM_COL32(255, 0, 0, 255), 0, 3.0f);
                            else
                                ImGui::GetWindowDrawList()->AddCircle(pNpcPosition, 1.0f, IM_COL32(0, 0, 255, 255), 0, 3.0f);
                        }
                    }

                    Guard playerListLock(m_pClient->m_vecPlayerLock);
                    std::vector<TPlayer>* vecPlayerList;
                    if (m_pClient->GetPlayerList(&vecPlayerList))
                    {
                        for (const TPlayer& pPlayer : *vecPlayerList)
                        {
                            if (m_pClient->GetDistance(pPlayer.fX, pPlayer.fY) > MAX_VIEW_RANGE)
                                continue;

                            ImVec2 pPlayerPosition = ImVec2(
                                pOffsetPosition.x + std::ceil(pPlayer.fX / (float)(m_pWorldData->fMapLength / m_pWorldData->iMiniMapImageWidth)),
                                pOffsetPosition.y + std::ceil(m_pWorldData->iMiniMapImageHeight - (pPlayer.fY / (float)(m_pWorldData->fMapLength / m_pWorldData->iMiniMapImageHeight))));

                            ImGui::GetWindowDrawList()->AddCircle(pPlayerPosition, 1.0f, IM_COL32(0, 191, 255, 255), 0, 3.0f);
                        }
                    }

                    Vector3 vec3CurrentPosition = m_pClient->GetPosition();

                    ImVec2 currentPosition = ImVec2(
                        pOffsetPosition.x + std::ceil(vec3CurrentPosition.m_fX / (float)(m_pWorldData->fMapLength / m_pWorldData->iMiniMapImageWidth)),
                        pOffsetPosition.y + std::ceil(m_pWorldData->iMiniMapImageHeight - (vec3CurrentPosition.m_fY / (float)(m_pWorldData->fMapLength / m_pWorldData->iMiniMapImageHeight))));

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

                    std::string szSelectedRouteConfiguration = Drawing::Bot->GetConfiguration()->GetString(skCryptDec("Bot"), skCryptDec("SelectedRoute"), "");

                    RouteManager* pRouteManager = Drawing::Bot->GetRouteManager();

                    RouteManager::RouteList pRouteList;
                    if (pRouteManager && pRouteManager->GetRouteList(m_pClient->GetZone(), pRouteList))
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
            std::string szSelectedRoute = Drawing::Bot->GetConfiguration()->GetString(skCryptDec("Bot"), skCryptDec("SelectedRoute"), "");
            if (ImGui::BeginCombo(skCryptDec("##PlannedRoute.RouteList"), szSelectedRoute.c_str()))
            {
                RouteManager::RouteList pRouteList;
                if (pRouteManager && pRouteManager->GetRouteList(m_pClient->GetZone(), pRouteList))
                {
                    for (auto e : pRouteList)
                    {
                        const bool is_selected = (szSelectedRoute == e.first);

                        if (ImGui::Selectable(e.first.c_str(), is_selected))
                        {
                            Drawing::Bot->GetConfiguration()->SetString(skCryptDec("Bot"), skCryptDec("SelectedRoute"), e.first.c_str());
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
                    m_pClient->SetMovePosition(Vector3(0.0f, 0.0f, 0.0f));
                }
                ImGui::PopStyleColor(1);
            }
            else
            {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.255f, 0.0f, 1.0f));
                if (ImGui::Button(skCryptDec("Run"), ImVec2(129.0f, 0.0f)))
                {
                    RouteManager::RouteList pRouteList;
                    if (pRouteManager && pRouteManager->GetRouteList(m_pClient->GetZone(), pRouteList))
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
                Drawing::Bot->GetConfiguration()->SetString(skCryptDec("Bot"), skCryptDec("SelectedRoute"), "");
            }
        }

        ImGui::Spacing();
        {
            ImGui::TextUnformatted(skCryptDec("Quick Action"));
            ImGui::Separator();

            bool bLegalStatus = Drawing::Bot->GetConfiguration()->GetBool(skCryptDec("Bot"), skCryptDec("Legal"), false);

            if (bLegalStatus)
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.255f, 0.0f, 1.0f));
            else
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.255f, 0.0f, 0.0f, 1.0f));

            if (ImGui::Button(skCryptDec("Legal"), ImVec2(129.0f, 0.0f)))
            {
                bLegalStatus = !bLegalStatus;
                m_pConfiguration->SetInt(skCryptDec("Bot"), skCryptDec("Legal"), bLegalStatus);
            }

            ImGui::PopStyleColor(1);

            ImGui::SameLine();

            if (ImGui::Button(skCryptDec("Town"), ImVec2(129.0f, 0.0f)))
            {
                m_pClient->SendTownPacket();
            }
        }

        ImGui::Spacing();
        {
            auto windowHeight = ImGui::GetWindowSize().y;
            auto fHeight = 30.0f;
            ImGui::SetCursorPosY((windowHeight - fHeight) * 0.97f);

            ImGui::TextUnformatted(skCryptDec("Automation"));
            ImGui::Separator();

            bool bAttackStatus = m_pConfiguration->GetBool(skCryptDec("Automation"), skCryptDec("Attack"), false);

            if (bAttackStatus)
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.255f, 0.0f, 1.0f));
            else
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.255f, 0.0f, 0.0f, 1.0f));

            if (ImGui::Button(skCryptDec("Attack"), ImVec2(129.0f, 0.0f)))
            {
                bAttackStatus = !bAttackStatus;
                m_pConfiguration->SetInt(skCryptDec("Automation"), skCryptDec("Attack"), bAttackStatus);
            }

            ImGui::PopStyleColor(1);
            ImGui::SameLine();

            bool bCharacterStatus = m_pConfiguration->GetBool(skCryptDec("Automation"), skCryptDec("Character"), false);

            if (bCharacterStatus)
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.255f, 0.0f, 1.0f));
            else
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.255f, 0.0f, 0.0f, 1.0f));

            if (ImGui::Button(skCryptDec("Character"), ImVec2(129.0f, 0.0f)))
            {
                bCharacterStatus = !bCharacterStatus;
                m_pConfiguration->SetInt(skCryptDec("Automation"), skCryptDec("Character"), bCharacterStatus);
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
                DrawMainSettingsArea();

                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem(skCryptDec("Attack")))
            {
                ImGui::TextUnformatted(skCryptDec("Configure Automated Attack"));
                ImGui::Separator();

                ImGui::Spacing();
                {
                    bool bAutoTarget = m_pConfiguration->GetInt(skCryptDec("Attack"), skCryptDec("AutoTarget"), true);

                    if (ImGui::Checkbox(skCryptDec("##AutoTargetCheckbox"), &bAutoTarget))
                        m_pConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("AutoTarget"), bAutoTarget ? 1 : 0);

                    ImGui::SameLine();

                    ImGui::Text(skCryptDec("Auto Target"));

                    ImGui::SameLine();

                    bool bRangeLimit = m_pConfiguration->GetBool(skCryptDec("Attack"), skCryptDec("RangeLimit"), false);

                    if (ImGui::Checkbox(skCryptDec("##RangeLimitCheckbox"), &bRangeLimit))
                        m_pConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("RangeLimit"), bRangeLimit ? 1 : 0);

                    ImGui::SameLine();

                    ImGui::Text(skCryptDec("Range Limit"));

                    ImGui::SameLine();

                    ImGui::PushItemWidth(50);

                    int iRangeLimitValue = m_pConfiguration->GetInt(skCryptDec("Attack"), skCryptDec("RangeLimitValue"), (int)MAX_ATTACK_RANGE);

                    if (ImGui::DragInt(skCryptDec("##RangeLimitValue"), &iRangeLimitValue, 1, 0, 100))
                        m_pConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("RangeLimitValue"), iRangeLimitValue);

                    ImGui::PopItemWidth();
                }

                ImGui::Spacing();
                {
                    bool bAttackSpeed = m_pConfiguration->GetBool(skCryptDec("Attack"), skCryptDec("AttackSpeed"), false);

                    if (ImGui::Checkbox(skCryptDec("##AttackSpeedCheckbox"), &bAttackSpeed))
                        m_pConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("AttackSpeed"), bAttackSpeed ? 1 : 0);

                    ImGui::SameLine();

                    ImGui::Text(skCryptDec("Attack Speed"));

                    ImGui::SameLine();

                    ImGui::PushItemWidth(75);

                    int iAttackSpeedValue = m_pConfiguration->GetInt(skCryptDec("Attack"), skCryptDec("AttackSpeedValue"), 1000);

                    if (ImGui::DragInt(skCryptDec("##AttackSpeedValue"), &iAttackSpeedValue, 1, 0, 65535))
                        m_pConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("AttackSpeedValue"), iAttackSpeedValue);

                    ImGui::PopItemWidth();
                }

                ImGui::Spacing();
                {
                    bool bBasicAttack = m_pConfiguration->GetBool(skCryptDec("Attack"), skCryptDec("BasicAttack"), true);

                    if (ImGui::Checkbox(skCryptDec("##BasicAttackCheckbox"), &bBasicAttack))
                        m_pConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("BasicAttack"), bBasicAttack ? 1 : 0);

                    ImGui::SameLine();

                    ImGui::Text(skCryptDec("Basic Attack (R)"));

                    ImGui::SameLine();

                    bool bMoveToTarget = m_pConfiguration->GetBool(skCryptDec("Attack"), skCryptDec("MoveToTarget"), false);

                    if (ImGui::Checkbox(skCryptDec("##MoveToTargetCheckbox"), &bMoveToTarget))
                        m_pConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("MoveToTarget"), bMoveToTarget ? 1 : 0);

                    ImGui::SameLine();

                    ImGui::Text(skCryptDec("Move To Target"));
                }

                ImGui::Spacing();

                DrawMonsterListTree();

                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem(skCryptDec("Skill")))
            {
                DrawAutomatedAttackSkillTree();
                DrawAutomatedCharacterSkillTree();

                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem(skCryptDec("Supply")))
            {
                DrawMainSupplyArea();

                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }
    }

    ImGui::EndChild();
}

void Drawing::DrawMainProtectionArea()
{
    ImGui::TextUnformatted(skCryptDec("Protection"));
    ImGui::Separator();

    ImGui::Spacing();
    {
        bool bHpProtection = m_pConfiguration->GetBool(skCryptDec("Protection"), skCryptDec("Hp"), false);

        if (ImGui::Checkbox(skCryptDec("##HpPotionCheckbox"), &bHpProtection))
            m_pConfiguration->SetInt(skCryptDec("Protection"), skCryptDec("Hp"), bHpProtection ? 1 : 0);

        ImGui::SameLine();

        ImGui::Text(skCryptDec("Hp Potion"));

        ImGui::SameLine();

        ImGui::PushItemWidth(50);

        int iHpProtectionValue = m_pConfiguration->GetInt(skCryptDec("Protection"), skCryptDec("HpValue"), 50);

        if (ImGui::DragInt(skCryptDec("##HpPotionValue"), &iHpProtectionValue, 1, 0, 100))
            m_pConfiguration->SetInt(skCryptDec("Protection"), skCryptDec("HpValue"), iHpProtectionValue);

        ImGui::PopItemWidth();

    }

    ImGui::Spacing();
    {
        bool bMpProtection = m_pConfiguration->GetBool(skCryptDec("Protection"), skCryptDec("Mp"), false);

        if (ImGui::Checkbox(skCryptDec("##MpPotionCheckbox"), &bMpProtection))
            m_pConfiguration->SetInt(skCryptDec("Protection"), skCryptDec("Mp"), bMpProtection ? 1 : 0);

        ImGui::SameLine();

        ImGui::Text(skCryptDec("Mp Potion"));

        ImGui::SameLine();

        ImGui::PushItemWidth(50);

        int iMpProtectionValue = m_pConfiguration->GetInt(skCryptDec("Protection"), skCryptDec("MpValue"), 25);

        if (ImGui::DragInt(skCryptDec("##MpPotionValue"), &iMpProtectionValue, 1, 0, 100))
            m_pConfiguration->SetInt(skCryptDec("Protection"), skCryptDec("MpValue"), iMpProtectionValue);

        ImGui::PopItemWidth();
    }

    ImGui::Spacing();
    {
        bool bMinorProtection = m_pConfiguration->GetBool(skCryptDec("Protection"), skCryptDec("Minor"), false);

        if (ImGui::Checkbox(skCryptDec("##MinorCheckbox"), &bMinorProtection))
            m_pConfiguration->SetInt(skCryptDec("Protection"), skCryptDec("Minor"), bMinorProtection ? 1 : 0);

        ImGui::SameLine();

        ImGui::Text(skCryptDec("Minor"));

        ImGui::SameLine();

        ImGui::PushItemWidth(50);

        int iMinorProtectionValue = m_pConfiguration->GetInt(skCryptDec("Protection"), skCryptDec("MinorValue"), 30);

        if (ImGui::DragInt(skCryptDec("##MinorValue"), &iMinorProtectionValue, 1, 0, 100))
            m_pConfiguration->SetInt(skCryptDec("Protection"), skCryptDec("MinorValue"), iMinorProtectionValue);

        ImGui::PopItemWidth();
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
            bool bGodMode = m_pConfiguration->GetBool(skCryptDec("Protection"), skCryptDec("GodMode"), false);

            if (ImGui::Checkbox(skCryptDec("##GodMode"), &bGodMode))
                m_pConfiguration->SetInt(skCryptDec("Protection"), skCryptDec("GodMode"), bGodMode ? 1 : 0);

            ImGui::SameLine();

            ImGui::Text(skCryptDec("God Mode"));

            ImGui::SameLine();

            bool bWallHack = m_pConfiguration->GetBool(skCryptDec("Feature"), skCryptDec("WallHack"), false);

            if (ImGui::Checkbox(skCryptDec("##WallHack"), &bWallHack))
            {
                m_pClient->SetAuthority(bWallHack ? 0 : 1);

                m_pConfiguration->SetInt(skCryptDec("Feature"), skCryptDec("WallHack"), bWallHack ? 1 : 0);
            }

            ImGui::SameLine();

            ImGui::Text(skCryptDec("Wall Hack"));

            ImGui::SameLine();

            bool bHyperNoah = m_pConfiguration->GetBool(skCryptDec("Feature"), skCryptDec("HyperNoah"), false);

            if (ImGui::Checkbox(skCryptDec("##HyperNoah"), &bHyperNoah))
                m_pConfiguration->SetInt(skCryptDec("Feature"), skCryptDec("HyperNoah"), bHyperNoah ? 1 : 0);

            ImGui::SameLine();

            ImGui::Text(skCryptDec("Hyper Noah"));
        }

        ImGui::Spacing();
        {
            bool bOreads = m_pConfiguration->GetBool(skCryptDec("Feature"), skCryptDec("Oreads"), false);

            if (ImGui::Checkbox(skCryptDec("##Oreads"), &bOreads))
            {
                m_pClient->SetOreads(bOreads);

                if (bOreads)
                {
                    m_pClient->EquipOreads(700039000);

                    //TODO: Need For Class

                    TInventory iItem = m_pClient->GetInventoryItem(110110001); // +1 Dagger 

                    if (iItem.iItemID != 0)
                    {
                        m_pClient->SendItemMovePacket(1, ITEM_INVEN_INVEN, iItem.iItemID, iItem.iPos - 14, 35);
                    }
                }

                m_pConfiguration->SetInt(skCryptDec("Feature"), skCryptDec("Oreads"), bOreads ? 1 : 0);
            }

            ImGui::SameLine();

            ImGui::Text(skCryptDec("Oreads"));

            ImGui::SameLine();

            bool bDeathEffect = m_pConfiguration->GetBool(skCryptDec("Feature"), skCryptDec("DeathEffect"), false);

            if (ImGui::Checkbox(skCryptDec("##DeathEffect"), &bDeathEffect))
            {
                m_pClient->PatchDeathEffect(bDeathEffect);
                m_pConfiguration->SetInt(skCryptDec("Feature"), skCryptDec("DeathEffect"), bDeathEffect ? 1 : 0);
            }
                

            ImGui::SameLine();

            ImGui::Text(skCryptDec("Patch Death Effect"));
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
            bool bAutoLoot = m_pConfiguration->GetBool(skCryptDec("AutoLoot"), skCryptDec("Enable"), false);

            if (ImGui::Checkbox(skCryptDec("##AutoLoot"), &bAutoLoot))
                m_pConfiguration->SetInt(skCryptDec("AutoLoot"), skCryptDec("Enable"), bAutoLoot ? 1 : 0);

            ImGui::SameLine();

            ImGui::Text(skCryptDec("Enable"));

            ImGui::SameLine();

            bool bMoveToLoot = m_pConfiguration->GetBool(skCryptDec("AutoLoot"), skCryptDec("MoveToLoot"), false);

            if (ImGui::Checkbox(skCryptDec("##MoveToLoot"), &bMoveToLoot))
            {
                m_pClient->SetAuthority(bMoveToLoot ? 0 : 1);
                m_pConfiguration->SetInt(skCryptDec("AutoLoot"), skCryptDec("MoveToLoot"), bMoveToLoot ? 1 : 0);
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

        int iLootMinPrice = m_pConfiguration->GetInt(skCryptDec("AutoLoot"), skCryptDec("MinPrice"), 0);

        if (ImGui::DragInt(skCryptDec("##LootMinPrice"), &iLootMinPrice, 1, 0, INT_MAX))
            m_pConfiguration->SetInt(skCryptDec("AutoLoot"), skCryptDec("MinPrice"), iLootMinPrice);

        ImGui::PopItemWidth();
    }
}

void Drawing::DrawMainSupplyArea()
{
    ImGui::Spacing();
    {
        ImGui::TextUnformatted(skCryptDec("Supply Management"));
        ImGui::Separator();

        bool bAutoRepair = m_pConfiguration->GetBool(skCryptDec("Supply"), skCryptDec("AutoRepair"), false);

        if (ImGui::Checkbox(skCryptDec("##AutoRepair"), &bAutoRepair))
            m_pConfiguration->SetInt(skCryptDec("Supply"), skCryptDec("AutoRepair"), bAutoRepair ? 1 : 0);

        ImGui::SameLine();

        ImGui::Text(skCryptDec("Auto Repair"));

        ImGui::SameLine();

        bool bAutoSupply = m_pConfiguration->GetBool(skCryptDec("Supply"), skCryptDec("AutoSupply"), false);

        if (ImGui::Checkbox(skCryptDec("##AutoSupply"), &bAutoSupply))
            m_pConfiguration->SetInt(skCryptDec("Supply"), skCryptDec("AutoSupply"), bAutoSupply ? 1 : 0);

        ImGui::SameLine();

        ImGui::Text(skCryptDec("Auto Supply"));

    }

    ImGui::Spacing();
    {
        ImGui::TextUnformatted(skCryptDec("Supply List"));
        ImGui::Separator();

        ImGui::Spacing();
        {
            ImGui::BeginTable(skCryptDec("##SupplyTable"), 3, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Borders);
            {
                std::vector<int> vecSupplyList = m_pConfiguration->GetInt(skCryptDec("Supply"), skCryptDec("Enable"), std::vector<int>());

                auto jSupplyList = Drawing::Bot->GetSupplyList();

                for (size_t i = 0; i < jSupplyList.size(); i++)
                {
                    ImGui::PushID(i);
                    ImGui::TableNextRow();
                    {
                        std::string szItemIdAttribute = skCryptDec("itemid");
                        std::string szNameAttribute = skCryptDec("name");
                        std::string szCountAttribute = skCryptDec("count");

                        ImGui::TableNextColumn();
                        {
                            bool bSelected = std::find(vecSupplyList.begin(), vecSupplyList.end(), jSupplyList[i][szItemIdAttribute.c_str()].get<int>()) != vecSupplyList.end();

                            if (ImGui::Checkbox(skCryptDec("##Enable"), &bSelected))
                            {
                                if (bSelected)
                                    vecSupplyList.push_back(jSupplyList[i][szItemIdAttribute.c_str()].get<int>());
                                else
                                    vecSupplyList.erase(std::find(vecSupplyList.begin(), vecSupplyList.end(), jSupplyList[i][szItemIdAttribute.c_str()].get<int>()));

                                m_pConfiguration->SetInt(skCryptDec("Supply"), skCryptDec("Enable"), vecSupplyList);
                            }
                        }

                        ImGui::TableNextColumn();
                        {
                            ImGui::Text(jSupplyList[i][szNameAttribute.c_str()].get<std::string>().c_str());
                        }
                        
                        ImGui::TableNextColumn();
                        {
                            int iCount = m_pConfiguration->GetInt(skCryptDec("Supply"), std::to_string(jSupplyList[i][szItemIdAttribute.c_str()].get<int>()).c_str(), jSupplyList[i][szCountAttribute.c_str()].get<int>());

                            ImGui::PushItemWidth(133);
                            if (ImGui::DragInt(skCryptDec("##Count"), &iCount, 1, 1, 9998))
                            {
                                m_pConfiguration->SetInt(skCryptDec("Supply"), std::to_string(jSupplyList[i][szItemIdAttribute.c_str()].get<int>()).c_str(), iCount);
                            }
                        }
                    }
                    ImGui::PopID();
                }

                ImGui::EndTable();
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
            bool bStopBotIfDead = m_pConfiguration->GetBool(skCryptDec("Settings"), skCryptDec("StopBotIfDead"), false);

            if (ImGui::Checkbox(skCryptDec("##StopBotIfDead"), &bStopBotIfDead))
                m_pConfiguration->SetInt(skCryptDec("Settings"), skCryptDec("StopBotIfDead"), bStopBotIfDead ? 1 : 0);

            ImGui::SameLine();

            ImGui::Text(skCryptDec("Stop bot if character dead"));

            bool bStopBotIfTeleported = m_pConfiguration->GetBool(skCryptDec("Settings"), skCryptDec("StopBotIfTeleported"), false);

            if (ImGui::Checkbox(skCryptDec("##StopBotIfTeleported"), &bStopBotIfTeleported))
                m_pConfiguration->SetInt(skCryptDec("Settings"), skCryptDec("StopBotIfTeleported"), bStopBotIfTeleported ? 1 : 0);

            ImGui::SameLine();

            ImGui::Text(skCryptDec("Stop bot if character teleported"));
        }
    }
}

void Drawing::DrawAutomatedAttackSkillTree()
{
    ImGui::TextUnformatted(skCryptDec("Select automated attack or character skill"));
    ImGui::Separator();

    std::vector<int> vecAttackList = m_pConfiguration->GetInt(skCryptDec("Automation"), skCryptDec("AttackSkillList"), std::vector<int>());

    std::stringstream strTreeText;

    if (!Drawing::Bot->IsTableLoaded())
        strTreeText << "|/-\\"[(int)(ImGui::GetTime() / 0.05f) & 3] << " ";

    strTreeText << skCryptDec("Automated attack skills");

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Bullet;
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

                    m_pConfiguration->SetInt(skCryptDec("Automation"), skCryptDec("AttackSkillList"), vecAttackList);
                }

                if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                    ImGui::SetTooltip(x.szDesc.c_str());

                ImGui::PopID();
            }
        }

        ImGui::TreePop();
    }
}

void Drawing::DrawAutomatedCharacterSkillTree()
{
    std::vector<int> vecCharacterSkillList = m_pConfiguration->GetInt(skCryptDec("Automation"), skCryptDec("CharacterSkillList"), std::vector<int>());

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
                if (x.iTarget != SkillTargetType::TARGET_SELF && x.iTarget != SkillTargetType::TARGET_PARTY_ALL && x.iTarget != SkillTargetType::TARGET_FRIEND_WITHME)
                    continue;

                ImGui::PushID(x.iID);

                bool bSelected = std::find(vecCharacterSkillList.begin(), vecCharacterSkillList.end(), x.iID) != vecCharacterSkillList.end();

                if (ImGui::Selectable(x.szName.c_str(), &bSelected))
                {
                    if (bSelected)
                        vecCharacterSkillList.push_back(x.iID);
                    else
                        vecCharacterSkillList.erase(std::find(vecCharacterSkillList.begin(), vecCharacterSkillList.end(), x.iID));

                    m_pConfiguration->SetInt(skCryptDec("Automation"), skCryptDec("CharacterSkillList"), vecCharacterSkillList);
                }

                if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                    ImGui::SetTooltip(x.szDesc.c_str());

                ImGui::PopID();
            }
        }

        ImGui::TreePop();
    }
}

void Drawing::DrawMonsterListTree()
{
    bool bRangeLimit = m_pConfiguration->GetBool(skCryptDec("Attack"), skCryptDec("RangeLimit"), false);
    int iRangeLimitValue = m_pConfiguration->GetInt(skCryptDec("Attack"), skCryptDec("RangeLimitValue"), (int)MAX_ATTACK_RANGE);
    bool bAutoTarget = m_pConfiguration->GetInt(skCryptDec("Attack"), skCryptDec("AutoTarget"), true);

    ImGui::TextUnformatted(skCryptDec("Select attackable target"));
    ImGui::Separator();

    if (bAutoTarget)
        ImGui::BeginDisabled();

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Bullet;
    if (ImGui::TreeNodeEx(skCryptDec("Target Monster List"), flags))
    {
        std::vector<SNpcData> vecTargetList;

        std::vector<int> vecSelectedNpcList = m_pConfiguration->GetInt(skCryptDec("Attack"), skCryptDec("NpcList"), std::vector<int>());

        Guard lock(m_pClient->m_vecNpcLock);
        std::vector<TNpc>* vecNpcList;
        if (m_pClient->GetNpcList(&vecNpcList))
        {
            for (const auto& x : vecSelectedNpcList)
            {
                SNpcData pNpcData;

                pNpcData.iProtoID = x;
                pNpcData.fDistance = 0.0f;

                const auto pFindedNpc = std::find_if(vecNpcList->begin(), vecNpcList->end(),
                    [x](const TNpc& a) { return a.iProtoID == x; });

                if (pFindedNpc != vecNpcList->end())
                    pNpcData.fDistance = m_pClient->GetDistance(pFindedNpc->fX, pFindedNpc->fY);

                vecTargetList.push_back(pNpcData);
            }

            for (const auto& x : *vecNpcList)
            {
                const auto pFindedNpc = std::find_if(vecTargetList.begin(), vecTargetList.end(),
                    [x](const SNpcData& a) { return a.iProtoID == x.iProtoID; });

                if ((x.iMonsterOrNpc == 1
                    || (x.iProtoID >= 19067 && x.iProtoID <= 19069)
                    || (x.iProtoID >= 19070 && x.iProtoID <= 19072))
                    && x.iProtoID != 9009
                    && m_pClient->GetDistance(x.fX, x.fY) <= MAX_VIEW_RANGE
                    && pFindedNpc == vecTargetList.end())
                {
                    SNpcData pNpcData;

                    pNpcData.iProtoID = x.iProtoID;
                    pNpcData.fDistance = m_pClient->GetDistance(x.fX, x.fY);

                    vecTargetList.push_back(pNpcData);
                }
            }

            for (const auto& x : vecTargetList)
            {
                bool bIsAttackable = false;

                if (bRangeLimit)
                {
                    if (x.fDistance != 0.0f && x.fDistance <= (float)MAX_ATTACK_RANGE)
                        bIsAttackable = true;
                }
                else
                    bIsAttackable = (x.fDistance != 0.0f && x.fDistance <= (float)MAX_ATTACK_RANGE);

                if (bIsAttackable)
                    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
                else
                    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 0, 0, 255));

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

                    m_pConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("NpcList"), vecSelectedNpcList);
                }

                ImGui::SameLine();
                ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 165, 0, 255));
                RightText(std::to_string((int)x.fDistance) + skCryptDec("m"));
                ImGui::PopStyleColor();
                ImGui::PopID();

                ImGui::PopStyleColor();
            }
        }

        ImGui::TreePop();
    }

    if (bAutoTarget)
        ImGui::EndDisabled();
}