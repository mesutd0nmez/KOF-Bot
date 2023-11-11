#include "pch.h"
#include "Drawing_Common.h"
#include "Bot.h"
#include "Client.h"
#include "ClientHandler.h"
#include "UI.h"
#include "RouteManager.h"

#ifdef UI_COMMON

Bot* Drawing::Bot = nullptr;
bool Drawing::bDraw = true;
bool Drawing::bDrawRoutePlanner = false;
bool Drawing::bDrawInventory = false;
bool Drawing::Done = false;

ClientHandler* m_pClient = nullptr;
Ini* m_pUserConfiguration = nullptr;
Ini* m_pAppConfiguration = nullptr;

PDIRECT3DTEXTURE9 m_pMapTexture = nullptr;
PDIRECT3DTEXTURE9 m_pMinimapTexture = nullptr;

float m_fScreenWidth = 0.0f;
float m_fScreenHeight = 0.0f;

bool m_bInitPos = false;

char m_szRouteName[255] = "";

std::string m_szSelectedRoute = "";
std::vector<Route> m_vecRoute;
bool bEnableAutoRoute = false;

WorldData* m_pWorldData = nullptr;

std::vector<int> m_vecInventorySelectedItem;

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
    {
        bEnableAutoRoute = false;
        m_vecRoute.clear();
    }
}

void Drawing::Draw()
{
    bool bTableLoaded = Drawing::Bot->IsTableLoaded();

    if (!bTableLoaded)
        ImGui::BeginDisabled();

	if (isActive())
	{
        InitializeSceneData();

        ImVec2 ScreenRes { 0, 0 };
        ImVec2 WindowPos { 0, 0 };
        ImVec2 WindowSize { 365, 800 };

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
		ImGui::Begin(UI::m_szMainWindowName.c_str(), &bDraw, WindowFlags);
		{
            DrawScene();
		}

        ImGui::End();

        DrawRoutePlanner();
        DrawInventory();
	}

    if (!bTableLoaded)
        ImGui::EndDisabled();
}

void Drawing::DrawProtectionArea()
{
    ImGui::Spacing();
    {
        ImGui::TextUnformatted(skCryptDec("Koruma Ayarlari"));
        ImGui::Separator();

        ImGui::Spacing();
        {
            bool bHpProtection = m_pUserConfiguration->GetBool(skCryptDec("Protection"), skCryptDec("Hp"), false);

            if (ImGui::Checkbox(skCryptDec("##HpPotionCheckbox"), &bHpProtection))
                m_pUserConfiguration->SetInt(skCryptDec("Protection"), skCryptDec("Hp"), bHpProtection ? 1 : 0);

            ImGui::SameLine();

            ImGui::Text(skCryptDec("Oto HP Pot"));

            ImGui::SameLine();

            ImGui::PushItemWidth(50);

            int iHpProtectionValue = m_pUserConfiguration->GetInt(skCryptDec("Protection"), skCryptDec("HpValue"), 50);

            if (ImGui::DragInt(skCryptDec("##HpPotionValue"), &iHpProtectionValue, 1, 0, 100))
                m_pUserConfiguration->SetInt(skCryptDec("Protection"), skCryptDec("HpValue"), iHpProtectionValue);

            ImGui::PopItemWidth();

            bool bMpProtection = m_pUserConfiguration->GetBool(skCryptDec("Protection"), skCryptDec("Mp"), false);

            if (ImGui::Checkbox(skCryptDec("##MpPotionCheckbox"), &bMpProtection))
                m_pUserConfiguration->SetInt(skCryptDec("Protection"), skCryptDec("Mp"), bMpProtection ? 1 : 0);

            ImGui::SameLine();

            ImGui::Text(skCryptDec("Oto MP Pot"));

            ImGui::SameLine();

            ImGui::PushItemWidth(50);

            int iMpProtectionValue = m_pUserConfiguration->GetInt(skCryptDec("Protection"), skCryptDec("MpValue"), 25);

            if (ImGui::DragInt(skCryptDec("##MpPotionValue"), &iMpProtectionValue, 1, 0, 100))
                m_pUserConfiguration->SetInt(skCryptDec("Protection"), skCryptDec("MpValue"), iMpProtectionValue);

            ImGui::PopItemWidth();

            if (!m_pClient->IsRogue())
                ImGui::BeginDisabled();

            bool bMinorProtection = m_pUserConfiguration->GetBool(skCryptDec("Protection"), skCryptDec("Minor"), false);

            if (ImGui::Checkbox(skCryptDec("##MinorCheckbox"), &bMinorProtection))
                m_pUserConfiguration->SetInt(skCryptDec("Protection"), skCryptDec("Minor"), bMinorProtection ? 1 : 0);

            ImGui::SameLine();

            ImGui::Text(skCryptDec("Oto Minor"));

            ImGui::SameLine();

            ImGui::PushItemWidth(50);

            int iMinorProtectionValue = m_pUserConfiguration->GetInt(skCryptDec("Protection"), skCryptDec("MinorValue"), 30);

            if (ImGui::DragInt(skCryptDec("##MinorValue"), &iMinorProtectionValue, 1, 0, 100))
                m_pUserConfiguration->SetInt(skCryptDec("Protection"), skCryptDec("MinorValue"), iMinorProtectionValue);

            ImGui::PopItemWidth();

            if (!m_pClient->IsRogue())
                ImGui::EndDisabled();
        }
    }
}

void Drawing::DrawMainSettingsArea()
{
    ImGui::Spacing();
    {
        ImGui::TextUnformatted(skCryptDec("Genel Ayarlar"));
        ImGui::Separator();

        ImGui::Spacing();
        {
            bool bWallHack = m_pUserConfiguration->GetBool(skCryptDec("Feature"), skCryptDec("WallHack"), false);

            if (ImGui::Checkbox(skCryptDec("##WallHack"), &bWallHack))
            {
                m_pClient->SetAuthority(bWallHack ? 0 : 1);

                m_pUserConfiguration->SetInt(skCryptDec("Feature"), skCryptDec("WallHack"), bWallHack ? 1 : 0);
            }

            ImGui::SameLine();

            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255.0f, 0.0f, 0.0f, 1.0f));
            ImGui::Text(skCryptDec("Wall Hack (Full)"));
            ImGui::PopStyleColor();

            ImGui::SameLine();

            bool bLegalWallHack = m_pUserConfiguration->GetBool(skCryptDec("Feature"), skCryptDec("LegalWallHack"), false);

            if (ImGui::Checkbox(skCryptDec("##LegalWallHack"), &bLegalWallHack))
            {
                m_pClient->PatchObjectCollision(bLegalWallHack);

                m_pUserConfiguration->SetInt(skCryptDec("Feature"), skCryptDec("LegalWallHack"), bLegalWallHack ? 1 : 0);
            }

            ImGui::SameLine();

            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255.0f, 0.0f, 0.0f, 1.0f));
            ImGui::Text(skCryptDec("Wall Hack (Sadece Objeler)"));
            ImGui::PopStyleColor();

            bool bSpeedHack = m_pUserConfiguration->GetBool(skCryptDec("Feature"), skCryptDec("SpeedHack"), false);

            if (ImGui::Checkbox(skCryptDec("##SpeedHack"), &bSpeedHack))
            {
                if (bSpeedHack)
                {
                    m_pClient->SetCharacterSpeed(1.5);
                    m_pClient->PatchSpeedHack(true);
                }
                else
                {
                    m_pClient->SetCharacterSpeed(1);
                    m_pClient->PatchSpeedHack(false);
                }

                m_pUserConfiguration->SetInt(skCryptDec("Feature"), skCryptDec("SpeedHack"), bSpeedHack ? 1 : 0);
            }

            ImGui::SameLine();

            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255.0f, 0.0f, 0.0f, 1.0f));
            ImGui::Text(skCryptDec("Speed Hack (Swift)"));
            ImGui::PopStyleColor();

            bool bSaveCPUEnable = m_pUserConfiguration->GetBool(skCryptDec("Feature"), skCryptDec("SaveCPU"), false);
            int iSaveCPUValue = m_pUserConfiguration->GetInt(skCryptDec("Feature"), skCryptDec("SaveCPUValue"), 1);

            if (ImGui::Checkbox(skCryptDec("##SaveCPUCheckbox"), &bSaveCPUEnable))
            {
                m_pUserConfiguration->SetInt(skCryptDec("Feature"), skCryptDec("SaveCPU"), bSaveCPUEnable ? 1 : 0);

                if (!bSaveCPUEnable)
                {
                    m_pClient->SetSaveCPUSleepTime(0);
                }
                else
                {
                    m_pClient->SetSaveCPUSleepTime(iSaveCPUValue);
                }
            }

            ImGui::SameLine();

            ImGui::Text(skCryptDec("Save CPU"));

            ImGui::SameLine();

            ImGui::PushItemWidth(160);

            if (ImGui::SliderInt(skCryptDec("##SaveCPUValue"), &iSaveCPUValue, 1, 100))
            {
                m_pUserConfiguration->SetInt(skCryptDec("Feature"), skCryptDec("SaveCPUValue"), iSaveCPUValue);

                if (bSaveCPUEnable)
                {
                    m_pClient->SetSaveCPUSleepTime(iSaveCPUValue);
                }
            }

            ImGui::PopItemWidth();
        }
    }
}

void Drawing::DrawTransformationArea()
{
    ImGui::Spacing();
    {
        ImGui::TextUnformatted(skCryptDec("Oto TS Ayarlari"));
        ImGui::Separator();

        ImGui::Spacing();
        {
            bool bAutoTransformation = m_pUserConfiguration->GetBool(skCryptDec("Transformation"), skCryptDec("Auto"), false);

            if (ImGui::Checkbox(skCryptDec("##AutoTransformationCheckBox"), &bAutoTransformation))
                m_pUserConfiguration->SetInt(skCryptDec("Transformation"), skCryptDec("Auto"), bAutoTransformation ? 1 : 0);

            ImGui::SameLine();

            ImGui::Text(skCryptDec("Otomatik"));

            ImGui::SameLine();
            ImGui::SetNextItemWidth(174);

            struct DisguiseItem
            {
                uint32_t iID;
                std::string szName;
            };

            std::vector<DisguiseItem> vecDisguiseItems;

            vecDisguiseItems.push_back(DisguiseItem(381001000, "TS Scroll"));
            vecDisguiseItems.push_back(DisguiseItem(379090000, "TS Totem 1"));
            vecDisguiseItems.push_back(DisguiseItem(379093000, "TS Totem 2"));

            int iTransformationItem = m_pUserConfiguration->GetInt(skCryptDec("Transformation"), skCryptDec("Item"), 381001000);

            std::string szSelectedTransformationItem = "TS Scroll";

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

            ImGui::SetNextItemWidth(266);

            struct DisguiseSkill
            {
                uint32_t iID;
                std::string szName;
            };

            std::vector<DisguiseSkill> vecDisguiseSkills;

            int iTransformationSkill = m_pUserConfiguration->GetInt(skCryptDec("Transformation"), skCryptDec("Skill"), 472020);

            std::string szSelectedTransformationSkill = "";

            std::map<uint32_t, __TABLE_DISGUISE_RING>* mapDisguiseTable;
            if (Drawing::Bot->GetDisguiseRingTable(&mapDisguiseTable))
            {
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

void Drawing::DrawPriestManagementArea()
{
    ImGui::Spacing();
    {
        ImGui::TextUnformatted(skCryptDec("Priest Ayarlari"));
        ImGui::Separator();

        ImGui::Spacing();
        {
            bool bAutoHealthBuff = m_pUserConfiguration->GetBool(skCryptDec("Priest"), skCryptDec("AutoHealthBuff"), false);

            if (ImGui::Checkbox(skCryptDec("##AutoHealthBuff"), &bAutoHealthBuff))
                m_pUserConfiguration->SetInt(skCryptDec("Priest"), skCryptDec("AutoHealthBuff"), bAutoHealthBuff ? 1 : 0);

            ImGui::SameLine();

            ImGui::Text(skCryptDec("Oto Buff"));

            std::string szSelectedHealthBuff = "Otomatik";
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
                for (auto& e : mapHealthBuffList)
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

            bool bAutoDefenceBuff = m_pUserConfiguration->GetBool(skCryptDec("Priest"), skCryptDec("AutoDefenceBuff"), false);

            if (ImGui::Checkbox(skCryptDec("##AutoDefenceBuff"), &bAutoDefenceBuff))
                m_pUserConfiguration->SetInt(skCryptDec("Priest"), skCryptDec("AutoDefenceBuff"), bAutoDefenceBuff ? 1 : 0);

            ImGui::SameLine();

            ImGui::Text(skCryptDec("Oto AC"));

            std::string szSelectedDefenceBuff = "Otomatik";
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
                for (auto& e : mapDefenceBuffList)
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

            bool bAutoMindBuff = m_pUserConfiguration->GetBool(skCryptDec("Priest"), skCryptDec("AutoMindBuff"), false);

            if (ImGui::Checkbox(skCryptDec("##AutoMindBuff"), &bAutoMindBuff))
                m_pUserConfiguration->SetInt(skCryptDec("Priest"), skCryptDec("AutoMindBuff"), bAutoMindBuff ? 1 : 0);

            ImGui::SameLine();

            ImGui::Text(skCryptDec("Oto Mind"));

            std::string szSelectedMindBuff = "Otomatik";
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
                for (auto& e : mapMindBuffList)
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

            bool bAutoHeal = m_pUserConfiguration->GetBool(skCryptDec("Priest"), skCryptDec("AutoHeal"), false);

            if (ImGui::Checkbox(skCryptDec("##AutoHeal"), &bAutoHeal))
                m_pUserConfiguration->SetInt(skCryptDec("Priest"), skCryptDec("AutoHeal"), bAutoHeal ? 1 : 0);

            ImGui::SameLine();

            ImGui::Text(skCryptDec("Oto Heal"));

            ImGui::SameLine();

            ImGui::PushItemWidth(50);

            int iAutoHealValue = m_pUserConfiguration->GetInt(skCryptDec("Priest"), skCryptDec("AutoHealValue"), 75);

            if (ImGui::DragInt(skCryptDec("##AutoHealValue"), &iAutoHealValue, 1, 0, 100))
                m_pUserConfiguration->SetInt(skCryptDec("Priest"), skCryptDec("AutoHealValue"), iAutoHealValue);

            ImGui::PopItemWidth();

            std::string szSelectedHeal = "Otomatik";
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
                for (auto& e : mapHealList)
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

            bool bAutoStrength = m_pUserConfiguration->GetBool(skCryptDec("Priest"), skCryptDec("AutoStrength"), false);

            if (ImGui::Checkbox(skCryptDec("##AutoStrength"), &bAutoStrength))
                m_pUserConfiguration->SetInt(skCryptDec("Priest"), skCryptDec("AutoStrength"), bAutoStrength ? 1 : 0);

            ImGui::SameLine();

            ImGui::Text(skCryptDec("Oto Str"));

            ImGui::SameLine();

            bool bDebuffProtection = m_pUserConfiguration->GetBool(skCryptDec("Priest"), skCryptDec("DebuffProtection"), false);

            if (ImGui::Checkbox(skCryptDec("##DebuffProtection"), &bDebuffProtection))
                m_pUserConfiguration->SetInt(skCryptDec("Priest"), skCryptDec("DebuffProtection"), bDebuffProtection ? 1 : 0);

            ImGui::SameLine();

            ImGui::Text(skCryptDec("Oto Cure"));

            ImGui::SameLine();

            bool bPartyProtection = m_pUserConfiguration->GetBool(skCryptDec("Priest"), skCryptDec("PartyProtection"), false);

            if (ImGui::Checkbox(skCryptDec("##PartyProtection"), &bPartyProtection))
                m_pUserConfiguration->SetInt(skCryptDec("Priest"), skCryptDec("PartyProtection"), bPartyProtection ? 1 : 0);

            ImGui::SameLine();

            ImGui::Text(skCryptDec("Party Priest"));
        }
    }
}

void Drawing::DrawRogueManagementArea()
{
    ImGui::Spacing();
    {
        ImGui::TextUnformatted(skCryptDec("Rogue Ayarlari"));
        ImGui::Separator();

        ImGui::Spacing();
        {
            bool bPartySwift = m_pUserConfiguration->GetBool(skCryptDec("Rogue"), skCryptDec("PartySwift"), false);

            if (ImGui::Checkbox(skCryptDec("##RoguePartySwift"), &bPartySwift))
                m_pUserConfiguration->SetInt(skCryptDec("Rogue"), skCryptDec("PartySwift"), bPartySwift ? 1 : 0);

            ImGui::SameLine();

            ImGui::Text(skCryptDec("Party Swift"));
        }
    }
}

void Drawing::DrawSpeedController()
{
    ImGui::Spacing();
    {
        ImGui::TextUnformatted(skCryptDec("Saldiri Hizi Ayarlari"));
        ImGui::Separator();

        ImGui::Spacing();
        {
            bool bSearchTargetSpeed = m_pUserConfiguration->GetBool(skCryptDec("Attack"), skCryptDec("SearchTargetSpeed"), false);

            if (ImGui::Checkbox(skCryptDec("##SearchTargetSpeedCheckbox"), &bSearchTargetSpeed))
                m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("SearchTargetSpeed"), bSearchTargetSpeed ? 1 : 0);

            ImGui::SameLine();

            ImGui::Text(skCryptDec("Mob Tarama Hizi (ms)"));

            ImGui::SameLine();

            ImGui::PushItemWidth(75);

            int iSearchTargetSpeedValue = m_pUserConfiguration->GetInt(skCryptDec("Attack"), skCryptDec("SearchTargetSpeedValue"), 100);

            if (ImGui::DragInt(skCryptDec("##SearchTargetSpeedValue"), &iSearchTargetSpeedValue, 1, 0, 65535))
                m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("SearchTargetSpeedValue"), iSearchTargetSpeedValue);

            ImGui::PopItemWidth();

            bool bAttackSpeed = m_pUserConfiguration->GetBool(skCryptDec("Attack"), skCryptDec("AttackSpeed"), false);

            if (ImGui::Checkbox(skCryptDec("##AttackSpeedCheckbox"), &bAttackSpeed))
                m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("AttackSpeed"), bAttackSpeed ? 1 : 0);

            ImGui::SameLine();

            ImGui::Text(skCryptDec("Saldiri Hizi (ms)"));

            ImGui::SameLine();

            ImGui::PushItemWidth(75);

            int iAttackSpeedValue = m_pUserConfiguration->GetInt(skCryptDec("Attack"), skCryptDec("AttackSpeedValue"), 1000);

            if (ImGui::DragInt(skCryptDec("##AttackSpeedValue"), &iAttackSpeedValue, 1, 0, 65535))
                m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("AttackSpeedValue"), iAttackSpeedValue);

            ImGui::PopItemWidth();

            bool bRAttackSpeed = m_pUserConfiguration->GetBool(skCryptDec("Attack"), skCryptDec("RAttackSpeed"), false);

            if (ImGui::Checkbox(skCryptDec("##RAttackSpeedCheckbox"), &bRAttackSpeed))
                m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("RAttackSpeed"), bRAttackSpeed ? 1 : 0);

            ImGui::SameLine();

            ImGui::Text(skCryptDec("R Saldiri Hizi (ms)"));

            ImGui::SameLine();

            ImGui::PushItemWidth(75);

            int iRAttackSpeedValue = m_pUserConfiguration->GetInt(skCryptDec("Attack"), skCryptDec("RAttackSpeedValue"), 1100);

            if (ImGui::DragInt(skCryptDec("##RAttackSpeedValue"), &iRAttackSpeedValue, 1, 0, 65535))
                m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("RAttackSpeedValue"), iRAttackSpeedValue);

            ImGui::PopItemWidth();
        }
    }
}

void Drawing::DrawDistanceController()
{
    ImGui::Spacing();
    {
        ImGui::TextUnformatted(skCryptDec("Mesafe Ayarlari"));
        ImGui::Separator();

        ImGui::Spacing();
        {
            bool bRangeLimit = m_pUserConfiguration->GetBool(skCryptDec("Attack"), skCryptDec("RangeLimit"), false);

            if (ImGui::Checkbox(skCryptDec("##RangeLimitCheckbox"), &bRangeLimit))
                m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("RangeLimit"), bRangeLimit ? 1 : 0);

            ImGui::SameLine();

            ImGui::Text(skCryptDec("Mob Tarama Mesafesi (m)"));

            ImGui::SameLine();

            ImGui::PushItemWidth(50);

            int iRangeLimitValue = m_pUserConfiguration->GetInt(skCryptDec("Attack"), skCryptDec("RangeLimitValue"), 100);

            if (ImGui::DragInt(skCryptDec("##RangeLimitValue"), &iRangeLimitValue, 1, 0, 100))
                m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("RangeLimitValue"), iRangeLimitValue);

            ImGui::PopItemWidth();

            bool bAttackRangeLimit = m_pUserConfiguration->GetBool(skCryptDec("Attack"), skCryptDec("AttackRangeLimit"), false);

            if (ImGui::Checkbox(skCryptDec("##AttackRangeLimitCheckbox"), &bAttackRangeLimit))
                m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("AttackRangeLimit"), bAttackRangeLimit ? 1 : 0);

            ImGui::SameLine();

            ImGui::Text(skCryptDec("Saldiri Mesafesi (m)"));

            ImGui::SameLine();

            ImGui::PushItemWidth(50);

            int iAttackRangeLimitValue = m_pUserConfiguration->GetInt(skCryptDec("Attack"), skCryptDec("AttackRangeLimitValue"), 50);

            if (ImGui::DragInt(skCryptDec("##AttackRangeLimitValue"), &iAttackRangeLimitValue, 1, 0, 100))
                m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("AttackRangeLimitValue"), iAttackRangeLimitValue);

            ImGui::PopItemWidth();
        }
    }
}

void Drawing::DrawTargetListController()
{
    ImGui::Spacing();
    {
        ImGui::TextUnformatted(skCryptDec("Hedef Ayarlari"));
        ImGui::Separator();

        ImGui::Spacing();
        {
            bool bMoveToTarget = m_pUserConfiguration->GetBool(skCryptDec("Attack"), skCryptDec("MoveToTarget"), false);

            if (ImGui::Checkbox(skCryptDec("##MoveToTargetCheckbox"), &bMoveToTarget))
                m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("MoveToTarget"), bMoveToTarget ? 1 : 0);

            ImGui::SameLine();

            ImGui::Text(skCryptDec("Hedefe Kos"));

            bool bDeathEffect = m_pUserConfiguration->GetBool(skCryptDec("Feature"), skCryptDec("DeathEffect"), false);

            if (ImGui::Checkbox(skCryptDec("##DeathEffect"), &bDeathEffect))
            {
                m_pClient->PatchDeathEffect(bDeathEffect);
                m_pUserConfiguration->SetInt(skCryptDec("Feature"), skCryptDec("DeathEffect"), bDeathEffect ? 1 : 0);
            }

            ImGui::SameLine();

            ImGui::Text(skCryptDec("Death Efektini Kaldir"));

            bool bAutoTarget = m_pUserConfiguration->GetInt(skCryptDec("Attack"), skCryptDec("AutoTarget"), false);

            if (ImGui::Checkbox(skCryptDec("##AutoTargetCheckbox"), &bAutoTarget))
                m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("AutoTarget"), bAutoTarget ? 1 : 0);

            ImGui::SameLine();

            ImGui::Text(skCryptDec("Tum Hedeflere Saldir"));

            ImGui::BeginChild(skCryptDec("TargetListController"), ImVec2(335, 478), true);
            {
                bool bRangeLimit = m_pUserConfiguration->GetBool(skCryptDec("Attack"), skCryptDec("RangeLimit"), false);
                int iRangeLimitValue = m_pUserConfiguration->GetInt(skCryptDec("Attack"), skCryptDec("RangeLimitValue"), 100);

                if (bAutoTarget)
                    ImGui::BeginDisabled();

                if (ImGui::Button(skCryptDec("Listeye Mob ID Ekle"), ImVec2(320.0f, 0.0f)))
                {
                    int32_t iTargetID = m_pClient->GetTarget();

                    if (iTargetID != -1)
                    {
                        std::vector<int> vecSelectedNpcIDList = m_pUserConfiguration->GetInt(skCryptDec("Attack"), skCryptDec("NpcIDList"), std::vector<int>());

                        const auto pFindedID = std::find_if(vecSelectedNpcIDList.begin(), vecSelectedNpcIDList.end(),
                            [iTargetID](const auto& a) { return a == iTargetID; });

                        if (pFindedID == vecSelectedNpcIDList.end())
                        {
                            vecSelectedNpcIDList.push_back(iTargetID);
                            m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("NpcIDList"), vecSelectedNpcIDList);
                        }
                    }
                }

                ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Framed;
                if (ImGui::TreeNodeEx(skCryptDec("Yakinlardaki Hedefler"), flags))
                {
                    std::vector<SNpcData> vecTargetList;
                    std::vector<int> vecSelectedNpcList = m_pUserConfiguration->GetInt(skCryptDec("Attack"), skCryptDec("NpcList"), std::vector<int>());

                    for (const auto& x : vecSelectedNpcList)
                    {
                        SNpcData pNpcData{};

                        pNpcData.iProtoID = x;

                        vecTargetList.push_back(pNpcData);
                    }

                    std::shared_lock<std::shared_mutex> lock(m_pClient->m_mutexNpc);
                    for (const auto& x : m_pClient->m_vecNpc)
                    {
                        if (x.iMonsterOrNpc != 1)
                            continue;

                        if (x.iProtoID == 0)
                            continue;

                        if (x.iProtoID == 9009)
                            continue;

                        const auto pFindedNpc = std::find_if(vecTargetList.begin(), vecTargetList.end(),
                            [x](const SNpcData& a) { return a.iProtoID == x.iProtoID; });

                        if (pFindedNpc != vecTargetList.end())
                            continue;

                        SNpcData pNpcData{};

                        pNpcData.iProtoID = x.iProtoID;

                        vecTargetList.push_back(pNpcData);
                    }

                    for (const auto& x : vecTargetList)
                    {
                        ImGui::PushID(x.iProtoID);

                        bool bSelected = std::find(vecSelectedNpcList.begin(), vecSelectedNpcList.end(), x.iProtoID) != vecSelectedNpcList.end();

                        std::string szNpcName = skCryptDec("~Bilinmiyor~");

                        std::map<uint32_t, __TABLE_NPC>* pNpcTable;
                        if (Drawing::Bot->GetNpcTable(&pNpcTable))
                        {
                            auto pNpcInfo = pNpcTable->find(x.iProtoID);

                            if (pNpcInfo != pNpcTable->end())
                                szNpcName = pNpcInfo->second.szText;
                        }

                        std::map<uint32_t, __TABLE_MOB>* pMobTable;
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
 
                if (ImGui::TreeNodeEx(skCryptDec("Mob ID Listesi"), ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Framed))
                {
                    std::vector<int> vecSelectedNpcIDList = m_pUserConfiguration->GetInt(skCryptDec("Attack"), skCryptDec("NpcIDList"), std::vector<int>());

                    for (const auto& x : vecSelectedNpcIDList)
                    {
                        if (ImGui::Selectable(std::to_string(x).c_str(), true))
                        {
                            vecSelectedNpcIDList.erase(std::find(vecSelectedNpcIDList.begin(), vecSelectedNpcIDList.end(), x));
                            m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("NpcIDList"), vecSelectedNpcIDList);
                        }
                    }

                    ImGui::TreePop();
                }

                if (bAutoTarget)
                    ImGui::EndDisabled();


                ImGui::EndChild();
            }
        }
    }
}

void Drawing::DrawSkillController()
{
    ImGui::Spacing();
    {
        ImGui::TextUnformatted(skCryptDec("Skill Ayarlari"));
        ImGui::Separator();

        ImGui::Spacing();
        {
            bool bUseSkillWithPacket = m_pUserConfiguration->GetBool(skCryptDec("Skill"), skCryptDec("UseSkillWithPacket"), false);

            if (ImGui::Checkbox(skCryptDec("##UseSkillWithPacket"), &bUseSkillWithPacket))
            {
                m_pUserConfiguration->SetInt(skCryptDec("Skill"), skCryptDec("UseSkillWithPacket"), bUseSkillWithPacket ? 1 : 0);
            }

            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255.0f, 0.0f, 0.0f, 1.0f));
            ImGui::Text(skCryptDec("Skilleri Illegal Vur"));
            ImGui::PopStyleColor();

            if (!bUseSkillWithPacket)
                ImGui::BeginDisabled();

            bool bOnlyAttackSkillUseWithPacket = m_pUserConfiguration->GetBool(skCryptDec("Skill"), skCryptDec("OnlyAttackSkillUseWithPacket"), false);

            if (ImGui::Checkbox(skCryptDec("##OnlyAttackSkillUseWithPacket"), &bOnlyAttackSkillUseWithPacket))
            {
                m_pUserConfiguration->SetInt(skCryptDec("Skill"), skCryptDec("OnlyAttackSkillUseWithPacket"), bOnlyAttackSkillUseWithPacket ? 1 : 0);
            }

            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255.0f, 0.0f, 0.0f, 1.0f));
            ImGui::Text(skCryptDec("Sadece Attack Skiller Illegal"));
            ImGui::PopStyleColor();

            if (!bUseSkillWithPacket)
                ImGui::EndDisabled();

            bool bDisableCasting = m_pUserConfiguration->GetBool(skCryptDec("Feature"), skCryptDec("DisableCasting"), false);

            if (ImGui::Checkbox(skCryptDec("##DisableCasting"), &bDisableCasting))
            {
                m_pClient->UpdateSkillSuccessRate(bDisableCasting);

                m_pUserConfiguration->SetInt(skCryptDec("Feature"), skCryptDec("DisableCasting"), bDisableCasting ? 1 : 0);
            }

            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255.0f, 0.0f, 0.0f, 1.0f));
            ImGui::Text(skCryptDec("El Dusurmeyi Iptal Et"));
            ImGui::PopStyleColor();

            ImGui::BeginChild(skCryptDec("SkillController"), ImVec2(335, 478), true);
            {
                std::vector<int> vecAttackList = m_pUserConfiguration->GetInt(skCryptDec("Automation"), skCryptDec("AttackSkillList"), std::vector<int>());

                if (ImGui::TreeNodeEx(skCryptDec("Saldiri Skilleri"), ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Framed))
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

                            ImGui::PopID();
                        }
                    }

                    ImGui::TreePop();
                }

                std::vector<int> vecCharacterSkillList = m_pUserConfiguration->GetInt(skCryptDec("Automation"), skCryptDec("CharacterSkillList"), std::vector<int>());

                if (ImGui::TreeNodeEx(skCryptDec("Karakter Skilleri"), ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Framed))
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

                            ImGui::PopID();
                        }
                    }

                    ImGui::TreePop();
                }

                ImGui::EndChild();
            }
        }
    }
}

void Drawing::DrawSizeController()
{
    ImGui::Spacing();
    {
        ImGui::TextUnformatted(skCryptDec("Boyut Ayarlari"));
        ImGui::Separator();

        ImGui::Spacing();
        {
            bool bTargetSizeEnable = m_pUserConfiguration->GetBool(skCryptDec("Target"), skCryptDec("SizeEnable"), false);

            if (ImGui::Checkbox(skCryptDec("##TargetSizeCheckbox"), &bTargetSizeEnable))
                m_pUserConfiguration->SetInt(skCryptDec("Target"), skCryptDec("SizeEnable"), bTargetSizeEnable ? 1 : 0);

            ImGui::SameLine();

            ImGui::Text(skCryptDec("Hedef Boyutu"));

            ImGui::SameLine();

            int iTargetSize = m_pUserConfiguration->GetInt(skCryptDec("Target"), skCryptDec("Size"), 1);

            ImGui::PushItemWidth(139);

            if (ImGui::SliderInt(skCryptDec("##TargetSize"), &iTargetSize, 1, 10))
                m_pUserConfiguration->SetInt(skCryptDec("Target"), skCryptDec("Size"), iTargetSize);

            ImGui::PopItemWidth();

            bool bCharacterSizeEnable = m_pUserConfiguration->GetBool(skCryptDec("Character"), skCryptDec("SizeEnable"), false);

            if (ImGui::Checkbox(skCryptDec("##CharacterSizeCheckbox"), &bCharacterSizeEnable))
                m_pUserConfiguration->SetInt(skCryptDec("Character"), skCryptDec("SizeEnable"), bCharacterSizeEnable ? 1 : 0);

            ImGui::SameLine();

            ImGui::Text(skCryptDec("Karakter Boyutu"));

            ImGui::SameLine();

            int iCharacterSize = m_pUserConfiguration->GetInt(skCryptDec("Character"), skCryptDec("Size"), 1);

            ImGui::PushItemWidth(125);

            if (ImGui::SliderInt(skCryptDec("##CharacterSize"), &iCharacterSize, 1, 10))
                m_pUserConfiguration->SetInt(skCryptDec("Character"), skCryptDec("Size"), iCharacterSize);

            ImGui::PopItemWidth();
        }
    }
}

void Drawing::DrawAttackSettingsController()
{
    ImGui::Spacing();
    {
        ImGui::TextUnformatted(skCryptDec("Saldiri Ayarlari"));
        ImGui::Separator();

        ImGui::Spacing();
        {
            bool bBasicAttack = m_pUserConfiguration->GetBool(skCryptDec("Attack"), skCryptDec("BasicAttack"), false);

            if (ImGui::Checkbox(skCryptDec("##BasicAttackCheckbox"), &bBasicAttack))
                m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("BasicAttack"), bBasicAttack ? 1 : 0);

            ImGui::SameLine();

            ImGui::Text(skCryptDec("R Vur"));

            bool bBasicAttackWithPacket = m_pUserConfiguration->GetBool(skCryptDec("Attack"), skCryptDec("BasicAttackWithPacket"), false);

            if (ImGui::Checkbox(skCryptDec("##BasicAttackWithPacketCheckbox"), &bBasicAttackWithPacket))
                m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("BasicAttackWithPacket"), bBasicAttackWithPacket ? 1 : 0);

            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255.0f, 0.0f, 0.0f, 1.0f));
            ImGui::Text(skCryptDec("R Illegal Vur"));
            ImGui::PopStyleColor();
        }
    }
}

void Drawing::DrawSupplySettingsArea()
{
    ImGui::Spacing();
    {
        ImGui::TextUnformatted(skCryptDec("Tedarik & RPR Ayarlari"));
        ImGui::Separator();

        ImGui::Spacing();
        {
            bool bAutoSupply = m_pUserConfiguration->GetBool(skCryptDec("Supply"), skCryptDec("AutoSupply"), false);

            if (ImGui::Checkbox(skCryptDec("##AutoSupply"), &bAutoSupply))
                m_pUserConfiguration->SetInt(skCryptDec("Supply"), skCryptDec("AutoSupply"), bAutoSupply ? 1 : 0);

            ImGui::SameLine();

            ImGui::Text(skCryptDec("Oto Tedarik"));

            bool bAutoRepair = m_pUserConfiguration->GetBool(skCryptDec("Supply"), skCryptDec("AutoRepair"), false);

            if (ImGui::Checkbox(skCryptDec("##AutoRepair"), &bAutoRepair))
                m_pUserConfiguration->SetInt(skCryptDec("Supply"), skCryptDec("AutoRepair"), bAutoRepair ? 1 : 0);

            ImGui::SameLine();

            ImGui::Text(skCryptDec("Oto RPR (NPC)"));

            bool bAutoRepairMagicHammer = m_pUserConfiguration->GetBool(skCryptDec("Supply"), skCryptDec("AutoRepairMagicHammer"), false);

            if (ImGui::Checkbox(skCryptDec("##AutoRepairMagicHammer"), &bAutoRepairMagicHammer))
            {
                m_pUserConfiguration->SetInt(skCryptDec("Supply"), skCryptDec("AutoRepairMagicHammer"), bAutoRepairMagicHammer ? 1 : 0);
            }

            ImGui::SameLine();

            ImGui::Text(skCryptDec("Oto RPR (Magic Hammer)"));

            bool bAutoSellSlotRange = m_pUserConfiguration->GetBool(skCryptDec("Supply"), skCryptDec("AutoSellSlotRange"), false);

            if (ImGui::Checkbox(skCryptDec("##AutoSellSlotRange"), &bAutoSellSlotRange))
            {
                m_pUserConfiguration->SetInt(skCryptDec("Supply"), skCryptDec("AutoSellSlotRange"), bAutoSellSlotRange ? 1 : 0);
            }

            ImGui::SameLine();

            ImGui::Text(skCryptDec("Oto Satis (Inventory) Slot"));

            ImGui::SameLine();

            int iAutoSellSlotRangeStart = m_pUserConfiguration->GetInt(skCryptDec("Supply"), skCryptDec("AutoSellSlotRangeStart"), 1);
            int iAutoSellSlotRangeEnd = m_pUserConfiguration->GetInt(skCryptDec("Supply"), skCryptDec("AutoSellSlotRangeEnd"), 14);

            ImGui::PushItemWidth(50);
            if (ImGui::DragInt(skCryptDec("##AutoSellSlotRangeStart"), &iAutoSellSlotRangeStart, 1, 1, 28))
            {
                if (iAutoSellSlotRangeStart <= iAutoSellSlotRangeEnd)
                {
                    m_pUserConfiguration->SetInt(skCryptDec("Supply"), skCryptDec("AutoSellSlotRangeStart"), iAutoSellSlotRangeStart);
                }
            }
            ImGui::PopItemWidth();

            ImGui::SameLine();

            ImGui::Text(skCryptDec("~"));

            ImGui::SameLine();

            ImGui::PushItemWidth(50);
            if (ImGui::DragInt(skCryptDec("##AutoSellSlotRangeEnd"), &iAutoSellSlotRangeEnd, 1, 1, 28))
            {
                if (iAutoSellSlotRangeEnd >= iAutoSellSlotRangeStart)
                {
                    m_pUserConfiguration->SetInt(skCryptDec("Supply"), skCryptDec("AutoSellSlotRangeEnd"), iAutoSellSlotRangeEnd);
                }
            }
            ImGui::PopItemWidth();

            bool bAutoSellVipSlotRange = m_pUserConfiguration->GetBool(skCryptDec("Supply"), skCryptDec("AutoSellVipSlotRange"), false);

            if (ImGui::Checkbox(skCryptDec("##AutoSellVipSlotRange"), &bAutoSellVipSlotRange))
            {
                m_pUserConfiguration->SetInt(skCryptDec("Supply"), skCryptDec("AutoSellVipSlotRange"), bAutoSellVipSlotRange ? 1 : 0);
            }

            ImGui::SameLine();

            ImGui::Text(skCryptDec("Oto Satis (VIP) Slot"));

            ImGui::SameLine();

            int iAutoSellVipSlotRangeStart = m_pUserConfiguration->GetInt(skCryptDec("Supply"), skCryptDec("AutoSellVipSlotRangeStart"), 1);
            int iAutoSellVipSlotRangeEnd = m_pUserConfiguration->GetInt(skCryptDec("Supply"), skCryptDec("AutoSellVipSlotRangeEnd"), 48);

            ImGui::PushItemWidth(50);
            if (ImGui::DragInt(skCryptDec("##AutoSellVipSlotRangeStart"), &iAutoSellVipSlotRangeStart, 1, 1, 48))
            {
                if (iAutoSellVipSlotRangeStart <= iAutoSellVipSlotRangeEnd)
                {
                    m_pUserConfiguration->SetInt(skCryptDec("Supply"), skCryptDec("AutoSellVipSlotRangeStart"), iAutoSellVipSlotRangeStart);
                }
            }
            ImGui::PopItemWidth();

            ImGui::SameLine();

            ImGui::Text(skCryptDec("~"));

            ImGui::SameLine();

            ImGui::PushItemWidth(50);
            if (ImGui::DragInt(skCryptDec("##AutoSellVipSlotRangeEnd"), &iAutoSellVipSlotRangeEnd, 1, 1, 28))
            {
                if (iAutoSellVipSlotRangeEnd >= iAutoSellVipSlotRangeStart)
                {
                    m_pUserConfiguration->SetInt(skCryptDec("Supply"), skCryptDec("AutoSellVipSlotRangeEnd"), iAutoSellVipSlotRangeEnd);
                }
            }
            ImGui::PopItemWidth();

            bool bAutoSellByFlag = m_pUserConfiguration->GetBool(skCryptDec("Supply"), skCryptDec("AutoSellByFlag"), false);

            if (ImGui::Checkbox(skCryptDec("##AutoSellByFlag"), &bAutoSellByFlag))
                m_pUserConfiguration->SetInt(skCryptDec("Supply"), skCryptDec("AutoSellByFlag"), bAutoSellByFlag ? 1 : 0);

            ImGui::SameLine();

            ImGui::Text(skCryptDec("Oto Satis (Isaretli Itemler)"));

            if (ImGui::Button(skCryptDec("Inventory Yonetimi"), ImVec2(332.0f, 0.0f)))
            {
                bDrawInventory = true;
            }
        }
    }
}

void Drawing::DrawSupplyListArea()
{
    ImGui::Spacing();
    {
        ImGui::TextUnformatted(skCryptDec("Tedarik Listesi"));
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

void Drawing::DrawBottomControllerArea()
{
    bool bAttackStatus = m_pUserConfiguration->GetBool(skCryptDec("Automation"), skCryptDec("Attack"), false);

    if (bAttackStatus)
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.255f, 0.0f, 1.0f));
    else
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.255f, 0.0f, 0.0f, 1.0f));

    if (ImGui::Button(bAttackStatus ? skCryptDec("Saldiri Durdur") : skCryptDec("Saldiri Baslat"), ImVec2(163.0f, 0.0f)))
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

    if (ImGui::Button(bCharacterStatus ? skCryptDec("Bot Durdur") : skCryptDec("Bot Baslat"), ImVec2(163.0f, 0.0f)))
    {
        bCharacterStatus = !bCharacterStatus;
        m_pUserConfiguration->SetInt(skCryptDec("Automation"), skCryptDec("Character"), bCharacterStatus);
    }

    ImGui::PopStyleColor(1);

    if (ImGui::Button(skCryptDec("Town At"), ImVec2(163.0f, 0.0f)))
    {
        m_pClient->SendTownPacket();
    }

    ImGui::SameLine();

    if (ImGui::Button(skCryptDec("Skilleri Yenile"), ImVec2(163.0f, 0.0f)))
    {
        m_pUserConfiguration->SetInt(skCryptDec("Automation"), skCryptDec("AttackSkillList"), std::vector<int>());
        m_pUserConfiguration->SetInt(skCryptDec("Automation"), skCryptDec("CharacterSkillList"), std::vector<int>());

        m_pClient->LoadSkillData();
    }

    if (ImGui::Button(skCryptDec("Ayarlari Sifirla"), ImVec2(334.0f, 0.0f)))
    {
        m_pUserConfiguration->Reset();
    }
}

void Drawing::DrawRouteListController()
{
    ImGui::Spacing();
    {
        ImGui::TextUnformatted(skCryptDec("Rotalar"));
        ImGui::Separator();

        ImGui::Spacing();
        {
            RouteManager* pRouteManager = Drawing::Bot->GetRouteManager();

            uint8_t iZoneID = m_pClient->GetRepresentZone(m_pClient->GetZone());

            bool bSupplyRouteStatus = m_pUserConfiguration->GetInt(skCryptDec("Bot"), skCryptDec("SupplyRouteStatus"), true);

            if (ImGui::Checkbox(skCryptDec("##SupplyRouteCheckbox"), &bSupplyRouteStatus))
                m_pUserConfiguration->SetInt(skCryptDec("Bot"), skCryptDec("SupplyRouteStatus"), bSupplyRouteStatus ? 1 : 0);

            ImGui::SameLine();

            ImGui::Text(skCryptDec("Tedarik"));
            ImGui::SameLine();

            ImGui::SetCursorPosX(87);
            ImGui::SetNextItemWidth(253);
            std::string szSelectedSupplyRoute = m_pUserConfiguration->GetString(skCryptDec("Bot"), skCryptDec("SelectedSupplyRoute"), "");
            if (ImGui::BeginCombo(skCryptDec("##PlannedRoute.SupplyRouteList"), szSelectedSupplyRoute.c_str()))
            {
                RouteManager::RouteList pRouteList;
                if (pRouteManager
                    && pRouteManager->GetRouteList(iZoneID, pRouteList))
                {
                    for (auto& e : pRouteList)
                    {
                        const bool is_selected = (szSelectedSupplyRoute == e.first);

                        if (ImGui::Selectable(e.first.c_str(), is_selected))
                        {
                            m_pUserConfiguration->SetString(skCryptDec("Bot"), skCryptDec("SelectedSupplyRoute"), e.first.c_str());
                        }

                        if (is_selected)
                            ImGui::SetItemDefaultFocus();
                    }
                }

                ImGui::EndCombo();
            }

            bool bDeathRouteStatus = m_pUserConfiguration->GetInt(skCryptDec("Bot"), skCryptDec("DeathRouteStatus"), true);

            if (ImGui::Checkbox(skCryptDec("##DeathRouteCheckbox"), &bDeathRouteStatus))
                m_pUserConfiguration->SetInt(skCryptDec("Bot"), skCryptDec("DeathRouteStatus"), bDeathRouteStatus ? 1 : 0);

            ImGui::SameLine();

            ImGui::Text(skCryptDec("Death"));
            ImGui::SameLine();

            ImGui::SetCursorPosX(87);
            ImGui::SetNextItemWidth(253);
            std::string szSelectedDeathRoute = m_pUserConfiguration->GetString(skCryptDec("Bot"), skCryptDec("SelectedDeathRoute"), "");
            if (ImGui::BeginCombo(skCryptDec("##PlannedRoute.DeathRouteList"), szSelectedDeathRoute.c_str()))
            {
                RouteManager::RouteList pRouteList;
                if (pRouteManager
                    && pRouteManager->GetRouteList(iZoneID, pRouteList))
                {
                    for (auto& e : pRouteList)
                    {
                        const bool is_selected = (szSelectedDeathRoute == e.first);

                        if (ImGui::Selectable(e.first.c_str(), is_selected))
                        {
                            m_pUserConfiguration->SetString(skCryptDec("Bot"), skCryptDec("SelectedDeathRoute"), e.first.c_str());
                        }

                        if (is_selected)
                            ImGui::SetItemDefaultFocus();
                    }
                }

                ImGui::EndCombo();
            }


            bool bLoginRouteStatus = m_pUserConfiguration->GetInt(skCryptDec("Bot"), skCryptDec("LoginRouteStatus"), true);

            if (ImGui::Checkbox(skCryptDec("##LoginRouteCheckbox"), &bLoginRouteStatus))
                m_pUserConfiguration->SetInt(skCryptDec("Bot"), skCryptDec("LoginRouteStatus"), bLoginRouteStatus ? 1 : 0);

            ImGui::SameLine();

            ImGui::Text(skCryptDec("Login"));
            ImGui::SameLine();

            ImGui::SetCursorPosX(87);
            ImGui::SetNextItemWidth(253);
            std::string szSelectedLoginRoute = m_pUserConfiguration->GetString(skCryptDec("Bot"), skCryptDec("SelectedLoginRoute"), "");
            if (ImGui::BeginCombo(skCryptDec("##PlannedRoute.LoginRouteList"), szSelectedLoginRoute.c_str()))
            {
                RouteManager::RouteList pRouteList;
                if (pRouteManager
                    && pRouteManager->GetRouteList(iZoneID, pRouteList))
                {
                    for (auto& e : pRouteList)
                    {
                        const bool is_selected = (szSelectedLoginRoute == e.first);

                        if (ImGui::Selectable(e.first.c_str(), is_selected))
                        {
                            m_pUserConfiguration->SetString(skCryptDec("Bot"), skCryptDec("SelectedLoginRoute"), e.first.c_str());
                        }

                        if (is_selected)
                            ImGui::SetItemDefaultFocus();
                    }
                }

                ImGui::EndCombo();
            }

            if (ImGui::Button(skCryptDec("Rota Planlayici"), ImVec2(332.0f, 0.0f)))
            {
                bDrawRoutePlanner = true;
            }
        }
    }
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
        ImGui::Begin(UI::m_szRoutePlannerWindowName.c_str(), &Drawing::bDrawRoutePlanner, WindowFlags);
        {
            DrawRoutePlannerArea();
        }

        ImGui::End();
    }
}

void Drawing::DrawRoutePlannerArea()
{
    if (m_pWorldData == nullptr)
        return;

    if (bEnableAutoRoute)
    {
        if (m_vecRoute.size() == 0)
        {
            Route pAutoRoute{};

            pAutoRoute.fX = m_pClient->GetX();
            pAutoRoute.fY = m_pClient->GetY();
            pAutoRoute.eStepType = RouteStepType::STEP_MOVE;

            m_vecRoute.push_back(pAutoRoute);
        }
        else
        {
            Route pLastRoute = m_vecRoute.back();

            float fDistance = m_pClient->GetDistance(Vector3(pLastRoute.fX, 0.0f, pLastRoute.fY));

            if (fDistance > 3.0f)
            {
                Route pAutoRoute{};
                pAutoRoute.fX = m_pClient->GetX();
                pAutoRoute.fY = m_pClient->GetY();
                pAutoRoute.eStepType = RouteStepType::STEP_MOVE;

                m_vecRoute.push_back(pAutoRoute);
            }
        }
    }

    ImGui::BeginChild(skCryptDec("RoutePlanner.Map"), ImVec2((float)(m_pWorldData->iMapImageWidth + 17.0f), (float)m_pWorldData->iMapImageHeight + 17.0f), true);
    {
        ImVec2 pOffsetPosition = ImGui::GetCursorScreenPos();

        ImGui::Image((void*)m_pMapTexture, ImVec2((float)m_pWorldData->iMapImageWidth, (float)m_pWorldData->iMapImageHeight));

        if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        {
            ImVec2 mousePositionAbsolute = ImGui::GetMousePos();
            ImVec2 screenPositionAbsolute = ImGui::GetItemRectMin();
            ImVec2 mousePositionRelative = ImVec2(mousePositionAbsolute.x - screenPositionAbsolute.x, mousePositionAbsolute.y - screenPositionAbsolute.y);

            Route pRoute{};
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
        ImGui::GetWindowDrawList()->AddText(currentPosition, IM_COL32(0, 255, 0, 255), skCryptDec("Buradasin"));

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

                if (m_vecRoute[i - 1].eStepType != RouteStepType::STEP_TOWN
                    && m_vecRoute[i - 1].eStepType != RouteStepType::STEP_GATE)
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
                    std::string szStartPoint = skCryptDec("Baslangic Noktasi");
                    ImGui::GetWindowDrawList()->AddText(startPosition, IM_COL32(0, 0, 255, 255), szStartPoint.c_str());
                }
            }
            break;

            case RouteStepType::STEP_TOWN:
            {
                std::string szTownPoint = skCryptDec("Town Noktasi");
                ImGui::GetWindowDrawList()->AddText(nextPosition, IM_COL32(255, 69, 0, 255), szTownPoint.c_str());
            }
            break;

            case RouteStepType::STEP_SUNDRIES:
            {
                std::string szSupplyPoint = skCryptDec("Sundries Noktasi");
                ImGui::GetWindowDrawList()->AddText(nextPosition, IM_COL32(255, 69, 0, 255), szSupplyPoint.c_str());
            }
            break;

            case RouteStepType::STEP_POTION:
            {
                std::string szSupplyPoint = skCryptDec("Potion Noktasi");
                ImGui::GetWindowDrawList()->AddText(nextPosition, IM_COL32(255, 69, 0, 255), szSupplyPoint.c_str());
            }
            break;

            case RouteStepType::STEP_INN:
            {
                std::string szInnPoint = skCryptDec("Inn Noktasi");
                ImGui::GetWindowDrawList()->AddText(nextPosition, IM_COL32(255, 69, 0, 255), szInnPoint.c_str());
            }
            break;

            case RouteStepType::STEP_GENIE:
            {
                std::string szGeniePoint = skCryptDec("Genie Noktasi");
                ImGui::GetWindowDrawList()->AddText(nextPosition, IM_COL32(255, 69, 0, 255), szGeniePoint.c_str());
            }
            break;

            case RouteStepType::STEP_GATE:
            {
                std::string szGatePoint = skCryptDec("Gate Noktasi");
                ImGui::GetWindowDrawList()->AddText(nextPosition, IM_COL32(255, 69, 0, 255), szGatePoint.c_str());
            }
            break;

            case RouteStepType::STEP_BOT_START:
            {
                std::string szGatePoint = skCryptDec("Bot Baslatma Noktasi");
                ImGui::GetWindowDrawList()->AddText(nextPosition, IM_COL32(255, 69, 0, 255), szGatePoint.c_str());
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
            ImGui::Spacing();
            {
                ImGui::TextUnformatted(skCryptDec("Yonetim"));
                ImGui::Separator();

                ImGui::Spacing();
                {
                    if (bEnableAutoRoute)
                        ImGui::BeginDisabled();

                    ImGui::SetNextItemWidth(174);

                    if (ImGui::BeginCombo(skCryptDec("##RoutePlanner.RouteList"), m_szSelectedRoute.c_str()))
                    {
                        for (auto& e : pRouteList)
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

                    if (ImGui::Button(skCryptDec("Plani Yukle"), ImVec2(174.0f, 0.0f)))
                    {
                        auto pPlan = pRouteList.find(m_szSelectedRoute);

                        if (pPlan != pRouteList.end())
                        {
                            m_vecRoute = pPlan->second;
                            strcpy(m_szRouteName, m_szSelectedRoute.c_str());
                        }
                    }

                    if (ImGui::Button(skCryptDec("Plani Sil"), ImVec2(174.0f, 0.0f)))
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

                    if (bEnableAutoRoute)
                        ImGui::EndDisabled();
                }
            }
        }

        size_t iRouteCount = m_vecRoute.size();

        ImGui::Spacing();
        {
            ImGui::TextUnformatted(skCryptDec("Aksiyon"));
            ImGui::Separator();

            ImGui::Spacing();
            {
                if (bEnableAutoRoute || iRouteCount == 0)
                    ImGui::BeginDisabled();

                if (m_pClient->IsRouting())
                {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.255f, 0.0f, 0.0f, 1.0f));
                    if (ImGui::Button(skCryptDec("Durdur"), ImVec2(174.0f, 0.0f)))
                    {
                        m_pClient->ClearRoute();
                        m_pClient->StopMove();
                    }
                    ImGui::PopStyleColor(1);
                }
                else
                {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.255f, 0.0f, 1.0f));
                    if (ImGui::Button(skCryptDec("Calistir"), ImVec2(174.0f, 0.0f)))
                    {
                        if (m_vecRoute.size() > 0)
                            m_pClient->SetRoute(m_vecRoute);
                    }
                    ImGui::PopStyleColor(1);
                }

                if (ImGui::Button(skCryptDec("Geri Al"), ImVec2(83.0f, 0.0f)))
                {
                    if (iRouteCount > 0)
                        m_vecRoute.pop_back();
                }

                ImGui::SameLine();

                if (ImGui::Button(skCryptDec("Resetle"), ImVec2(83.0f, 0.0f)))
                {
                    m_vecRoute.clear();
                }

                if (bEnableAutoRoute || iRouteCount == 0)
                    ImGui::EndDisabled();

                if (bEnableAutoRoute)
                {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.255f, 0.0f, 0.0f, 1.0f));
                    if (ImGui::Button(skCryptDec("Oto Rotayi Kapat"), ImVec2(174.0f, 0.0f)))
                    {
                        bEnableAutoRoute = false;
                    }
                    ImGui::PopStyleColor(1);
                }
                else
                {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.255f, 0.0f, 1.0f));
                    if (ImGui::Button(skCryptDec("Oto Rotayi Ac"), ImVec2(174.0f, 0.0f)))
                    {
                        bEnableAutoRoute = true;
                    }
                    ImGui::PopStyleColor(1);
                }
            }
        }

        ImGui::Spacing();
        {
            ImGui::TextUnformatted(skCryptDec("Adimlar"));
            ImGui::Separator();

            ImGui::Spacing();
            {
                if (iRouteCount == 0)
                    ImGui::BeginDisabled();

                if (ImGui::Button(skCryptDec("Yurume Noktasi"), ImVec2(174.0f, 0.0f)))
                {
                    if (iRouteCount > 0)
                    {
                        m_vecRoute[m_vecRoute.size() - 1].eStepType = RouteStepType::STEP_MOVE;
                    }
                }

                if (ImGui::Button(skCryptDec("Town Noktasi"), ImVec2(174.0f, 0.0f)))
                {
                    if (iRouteCount > 0)
                    {
                        m_vecRoute[m_vecRoute.size() - 1].eStepType = RouteStepType::STEP_TOWN;

                        if (bEnableAutoRoute)
                        {
                            m_pClient->SendTownPacket();
                        }
                    }
                }

                if (ImGui::Button(skCryptDec("Sundries Noktasi"), ImVec2(174.0f, 0.0f)))
                {
                    if (iRouteCount > 0)
                    {
                        m_vecRoute[m_vecRoute.size() - 1].eStepType = RouteStepType::STEP_SUNDRIES;
                    }
                }

                if (ImGui::Button(skCryptDec("Potcu Noktasi"), ImVec2(174.0f, 0.0f)))
                {
                    if (iRouteCount > 0)
                    {
                        m_vecRoute[m_vecRoute.size() - 1].eStepType = RouteStepType::STEP_POTION;
                    }
                }

                if (ImGui::Button(skCryptDec("Inn Noktasi"), ImVec2(174.0f, 0.0f)))
                {
                    if (iRouteCount > 0)
                    {
                        m_vecRoute[m_vecRoute.size() - 1].eStepType = RouteStepType::STEP_INN;
                    }
                }

                if (ImGui::Button(skCryptDec("Genie Noktasi"), ImVec2(174.0f, 0.0f)))
                {
                    if (iRouteCount > 0)
                    {
                        m_vecRoute[m_vecRoute.size() - 1].eStepType = RouteStepType::STEP_GENIE;
                    }
                }

                if (!m_pClient->IsMage() && !m_pClient->IsPriest())
                    ImGui::BeginDisabled();

                if (ImGui::Button(skCryptDec("Gate Noktasi"), ImVec2(174.0f, 0.0f)))
                {
                    if (iRouteCount > 0)
                    {
                        m_vecRoute[m_vecRoute.size() - 1].eStepType = RouteStepType::STEP_GATE;

                        if (bEnableAutoRoute)
                        {
                            std::vector<__TABLE_UPC_SKILL>* vecAvailableSkills;
                            if (m_pClient->GetAvailableSkill(&vecAvailableSkills))
                            {
                                auto it = std::find_if(vecAvailableSkills->begin(), vecAvailableSkills->end(),
                                    [](const TABLE_UPC_SKILL& a) { return a.iBaseId == 109015 || a.iBaseId == 111700; });

                                if (it != vecAvailableSkills->end()
                                    && m_pClient->GetMp() >= it->iExhaustMSP)
                                {
                                    m_pClient->UseSkillWithPacket(*it, m_pClient->GetID());
                                }
                            }
                        }
                    }
                }

                if (!m_pClient->IsMage() && !m_pClient->IsPriest())
                    ImGui::EndDisabled();

                if (ImGui::Button(skCryptDec("Bot Baslatma Noktasi"), ImVec2(174.0f, 0.0f)))
                {
                    if (iRouteCount > 0)
                    {
                        m_vecRoute[m_vecRoute.size() - 1].eStepType = RouteStepType::STEP_BOT_START;
                    }
                }

                if (iRouteCount == 0)
                    ImGui::EndDisabled();
            }
        }

        ImGui::Spacing();
        {
            ImGui::TextUnformatted(skCryptDec("Kaydet"));
            ImGui::Separator();

            ImGui::Spacing();
            {
                if (bEnableAutoRoute || iRouteCount == 0)
                    ImGui::BeginDisabled();

                ImGui::SetNextItemWidth(174);
                ImGui::InputText(skCryptDec("##FileName"), &m_szRouteName[0], 100);

                size_t routeNameSize = strlen(m_szRouteName);

                if (routeNameSize == 0)
                    ImGui::BeginDisabled();

                if (ImGui::Button(skCryptDec("Plani Kaydet"), ImVec2(174.0f, 0.0f)))
                {
                    if (pRouteManager)
                    {
                        uint8_t iZoneID = m_pClient->GetRepresentZone(m_pClient->GetZone());

                        pRouteManager->Save(m_szRouteName, iZoneID, m_vecRoute);
                    }
                }

                if (routeNameSize == 0)
                    ImGui::EndDisabled();

                if (bEnableAutoRoute || iRouteCount == 0)
                    ImGui::EndDisabled();
            }
        }
    }

    ImGui::EndChild();
}

void Drawing::DrawFlashController()
{
    ImGui::Spacing();
    {
        ImGui::TextUnformatted(skCryptDec("Flash Ayarlari"));
        ImGui::Separator();

        ImGui::Spacing();
        {
            bool bAutoDCFlash = m_pUserConfiguration->GetBool(skCryptDec("Settings"), skCryptDec("AutoDCFlash"), false);

            if (ImGui::Checkbox(skCryptDec("##AutoDCFlash"), &bAutoDCFlash))
                m_pUserConfiguration->SetInt(skCryptDec("Settings"), skCryptDec("AutoDCFlash"), bAutoDCFlash ? 1 : 0);

            ImGui::SameLine();

            ImGui::Text(skCryptDec("DC Flash"));

            ImGui::SameLine();

            ImGui::PushItemWidth(100);

            int iAutoDCFlashCount = m_pUserConfiguration->GetInt(skCryptDec("Settings"), skCryptDec("AutoDCFlashCount"), 10);

            if (ImGui::DragInt(skCryptDec("##AutoDCFlashCount"), &iAutoDCFlashCount, 10, 1, 10))
                m_pUserConfiguration->SetInt(skCryptDec("Settings"), skCryptDec("AutoDCFlashCount"), iAutoDCFlashCount);

            ImGui::PopItemWidth();

            bool bAutoWarFlash = m_pUserConfiguration->GetBool(skCryptDec("Settings"), skCryptDec("AutoWarFlash"), false);

            if (ImGui::Checkbox(skCryptDec("##AutoWarFlash"), &bAutoWarFlash))
                m_pUserConfiguration->SetInt(skCryptDec("Settings"), skCryptDec("AutoWarFlash"), bAutoWarFlash ? 1 : 0);

            ImGui::SameLine();

            ImGui::Text(skCryptDec("War Flash"));

            ImGui::SameLine();

            ImGui::PushItemWidth(100);

            int iAutoWarFlashCount = m_pUserConfiguration->GetInt(skCryptDec("Settings"), skCryptDec("AutoWarFlashCount"), 10);

            if (ImGui::DragInt(skCryptDec("##AutoWarFlashCount"), &iAutoWarFlashCount, 10, 1, 10))
                m_pUserConfiguration->SetInt(skCryptDec("Settings"), skCryptDec("AutoWarFlashCount"), iAutoWarFlashCount);

            ImGui::PopItemWidth();

            bool bAutoExpFlash = m_pUserConfiguration->GetBool(skCryptDec("Settings"), skCryptDec("AutoExpFlash"), false);

            if (ImGui::Checkbox(skCryptDec("##AutoExpFlash"), &bAutoExpFlash))
                m_pUserConfiguration->SetInt(skCryptDec("Settings"), skCryptDec("AutoExpFlash"), bAutoExpFlash ? 1 : 0);

            ImGui::SameLine();

            ImGui::Text(skCryptDec("Exp Flash"));

            ImGui::SameLine();

            ImGui::PushItemWidth(100);

            int iAutoExpFlashCount = m_pUserConfiguration->GetInt(skCryptDec("Settings"), skCryptDec("AutoExpFlashCount"), 10);

            if (ImGui::DragInt(skCryptDec("##AutoExpFlashCount"), &iAutoExpFlashCount, 10, 1, 10))
                m_pUserConfiguration->SetInt(skCryptDec("Settings"), skCryptDec("AutoExpFlashCount"), iAutoExpFlashCount);

            ImGui::PopItemWidth();
        }
    }
}

void Drawing::DrawListenerController()
{
    ImGui::Spacing();
    {
        ImGui::TextUnformatted(skCryptDec("Dinleyici Ayarlari"));
        ImGui::Separator();

        ImGui::Spacing();
        {
            bool bPartyRequest = m_pUserConfiguration->GetBool(skCryptDec("Listener"), skCryptDec("PartyRequest"), false);

            if (ImGui::Checkbox(skCryptDec("##PartyRequest"), &bPartyRequest))
                m_pUserConfiguration->SetInt(skCryptDec("Listener"), skCryptDec("PartyRequest"), bPartyRequest ? 1 : 0);

            ImGui::SameLine();

            ImGui::Text(skCryptDec("Oto Party"));

            ImGui::SameLine();

            std::string szPartyRequestMessage = m_pUserConfiguration->GetString(skCryptDec("Listener"), skCryptDec("PartyRequestMessage"), "add");

            ImGui::PushItemWidth(169);

            if (ImGui::InputText(skCryptDec("##PartyRequestMessage"), &szPartyRequestMessage[0], 100))
            {
                m_pUserConfiguration->SetString(skCryptDec("Listener"), skCryptDec("PartyRequestMessage"), &szPartyRequestMessage[0]);
            }

            ImGui::PopItemWidth();

            if (!m_pClient->IsMage())
                ImGui::BeginDisabled();

            bool bTeleportRequest = m_pUserConfiguration->GetBool(skCryptDec("Listener"), skCryptDec("TeleportRequest"), false);

            if (ImGui::Checkbox(skCryptDec("##TeleportRequest"), &bTeleportRequest))
                m_pUserConfiguration->SetInt(skCryptDec("Listener"), skCryptDec("TeleportRequest"), bTeleportRequest ? 1 : 0);

            ImGui::SameLine();

            ImGui::Text(skCryptDec("Oto TP"));

            ImGui::SameLine();

            std::string szTeleportRequestMessage = m_pUserConfiguration->GetString(skCryptDec("Listener"), skCryptDec("TeleportRequestMessage"), "tptp");

            if (ImGui::InputText(skCryptDec("##TeleportRequestMessage"), &szTeleportRequestMessage[0], 100))
            {
                m_pUserConfiguration->SetString(skCryptDec("Listener"), skCryptDec("TeleportRequestMessage"), &szTeleportRequestMessage[0]);
            }

            if (!m_pClient->IsMage())
                ImGui::EndDisabled();
        }
    }
}

void Drawing::DrawSystemSettingsController()
{
    ImGui::Spacing();
    {
        ImGui::TextUnformatted(skCryptDec("Sistem Ayarlari"));
        ImGui::Separator();

        ImGui::Spacing();
        {
            bool bTownStopBot = m_pUserConfiguration->GetBool(skCryptDec("Bot"), skCryptDec("TownStopBot"), true);

            if (ImGui::Checkbox(skCryptDec("##TownStopBot"), &bTownStopBot))
                m_pUserConfiguration->SetInt(skCryptDec("Bot"), skCryptDec("TownStopBot"), bTownStopBot ? 1 : 0);

            ImGui::SameLine();

            ImGui::Text(skCryptDec("Town atinca botu durdur"));

            bool bTownOrTeleportStopBot = m_pUserConfiguration->GetBool(skCryptDec("Bot"), skCryptDec("TownOrTeleportStopBot"), false);

            if (ImGui::Checkbox(skCryptDec("##TownOrTeleportStopBot"), &bTownOrTeleportStopBot))
                m_pUserConfiguration->SetInt(skCryptDec("Bot"), skCryptDec("TownOrTeleportStopBot"), bTownOrTeleportStopBot ? 1 : 0);

            ImGui::SameLine();

            ImGui::Text(skCryptDec("Town veya Teleport olunca botu durdur"));

            bool bSyncWithGenie = m_pUserConfiguration->GetBool(skCryptDec("Bot"), skCryptDec("SyncWithGenie"), false);

            if (ImGui::Checkbox(skCryptDec("##SyncWithGenie"), &bSyncWithGenie))
                m_pUserConfiguration->SetInt(skCryptDec("Bot"), skCryptDec("SyncWithGenie"), bSyncWithGenie ? 1 : 0);

            ImGui::SameLine();

            ImGui::Text(skCryptDec("Genie ile botu esitle"));

            bool bStartGenieIfUserInRegion = m_pUserConfiguration->GetBool(skCryptDec("Settings"), skCryptDec("StartGenieIfUserInRegion"), false);

            if (ImGui::Checkbox(skCryptDec("##StartGenieIfUserInRegion"), &bStartGenieIfUserInRegion))
                m_pUserConfiguration->SetInt(skCryptDec("Settings"), skCryptDec("StartGenieIfUserInRegion"), bStartGenieIfUserInRegion ? 1 : 0);

            ImGui::SameLine();

            ImGui::Text(skCryptDec("Yakinlarda oyuncu varsa Genie baslat"));

            bool bSendTownIfBanNotice = m_pUserConfiguration->GetBool(skCryptDec("Settings"), skCryptDec("SendTownIfBanNotice"), false);

            if (ImGui::Checkbox(skCryptDec("##SendTownIfBanNotice"), &bSendTownIfBanNotice))
                m_pUserConfiguration->SetInt(skCryptDec("Settings"), skCryptDec("SendTownIfBanNotice"), bSendTownIfBanNotice ? 1 : 0);

            ImGui::SameLine();

            ImGui::Text(skCryptDec("Ban Notice gecerse Town at"));

            bool bPlayBeepfIfBanNotice = m_pUserConfiguration->GetBool(skCryptDec("Settings"), skCryptDec("PlayBeepfIfBanNotice"), false);

            if (ImGui::Checkbox(skCryptDec("##PlayBeepfIfBanNotice"), &bPlayBeepfIfBanNotice))
                m_pUserConfiguration->SetInt(skCryptDec("Settings"), skCryptDec("PlayBeepfIfBanNotice"), bPlayBeepfIfBanNotice ? 1 : 0);

            ImGui::SameLine();

            ImGui::Text(skCryptDec("Ban Notice gecerse BEEP sesi cal"));
        }
    }
}

void Drawing::DrawInventory()
{
    if (isActive())
    {
        InitializeSceneData();

        if (!Drawing::bDrawInventory)
            return;

        ImGuiWindowFlags WindowFlags = ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize;

        ImVec2 vec2InitialPos = { m_fScreenWidth, m_fScreenHeight };

        ImVec2 vWindowSize = { 1030, 620 };

        vec2InitialPos.x -= vWindowSize.x / 2;
        vec2InitialPos.y -= vWindowSize.y / 2;

        ImGui::SetNextWindowPos(vec2InitialPos, ImGuiCond_Once);
        ImGui::SetNextWindowSize(vWindowSize);
        ImGui::SetNextWindowBgAlpha(1.0f);
        ImGui::Begin(UI::m_szRoutePlannerWindowName.c_str(), &Drawing::bDrawInventory, WindowFlags);
        {
            DrawInventoryArea();
        }

        ImGui::End();
    }
}

void Drawing::DrawInventoryArea()
{
    ImGui::BeginChild(skCryptDec("Inventory.Child"), ImVec2(817.0f, 580.0f), true);
    {
        std::vector<TItemData> vecItemList;
        std::map<uint32_t, __TABLE_ITEM>* pItemTable;
        if (m_pClient->GetInventoryItemList(vecItemList) && Drawing::Bot->GetItemTable(&pItemTable))
        {
            if (ImGui::BeginTable(skCryptDec("Inventory.Table"), 4, ImGuiTableFlags_Resizable | ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_Borders))
            {
                for (const TItemData& pItem : vecItemList)
                {
                    ImGui::PushID((pItem.iPos - SLOT_MAX) + 1);

                    ImGui::TableNextRow();

                    uint8_t iItemFlag = Drawing::Bot->GetInventoryItemFlag(pItem.iItemID);

                    switch (iItemFlag)
                    {
                    case INVENTORY_ITEM_FLAG_DELETE:
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255.0f, 0.0f, 0.0f, 1.0f));
                        break;
                    case INVENTORY_ITEM_FLAG_SELL:
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255.0f, 215.0f, 0.0f, 1.0f));
                        break;
                    case INVENTORY_ITEM_FLAG_INN:
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 255.0f, 0.0f, 1.0f));
                        break;
                    case INVENTORY_ITEM_FLAG_VIP:
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 255.0f, 215.0f, 1.0f));
                        break;
                    case INVENTORY_ITEM_FLAG_EQUIP:
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255, 0, 255, 1.0f));
                        break;
                    default:
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255.0f, 255.0f, 255.0f, 1.0f));
                        break;
                    }

                    ImGui::TableNextColumn();
                    ImGui::Text("%d", (pItem.iPos - SLOT_MAX) + 1);
                    ImGui::TableNextColumn();

                    uint32_t iItemBaseID = pItem.iItemID / 1000 * 1000;

                    auto pItemData = pItemTable->find(iItemBaseID);

                    std::string szItemName = "";

                    if (pItemData != pItemTable->end())
                    {
                        szItemName = pItemData->second.szName;
                    }

                    bool bSelected = std::find(m_vecInventorySelectedItem.begin(), m_vecInventorySelectedItem.end(), pItem.iItemID) != m_vecInventorySelectedItem.end();

                    if (ImGui::Selectable(szItemName.c_str(), &bSelected, ImGuiSelectableFlags_SpanAllColumns) && pItem.iItemID != 0)
                    {
                        if (bSelected)
                            m_vecInventorySelectedItem.push_back(pItem.iItemID);
                        else
                            m_vecInventorySelectedItem.erase(std::find(m_vecInventorySelectedItem.begin(), m_vecInventorySelectedItem.end(), pItem.iItemID));
                    }

                    ImGui::TableNextColumn();
                    ImGui::Text("%d", pItem.iCount);
                    ImGui::TableNextColumn();
                    ImGui::Text("%d", pItem.iDurability);

                    ImGui::PopStyleColor(1);

                    ImGui::PopID();
                }

                ImGui::EndTable();
            }
        }
    }
    ImGui::EndChild();

    ImGui::SameLine();

    ImGui::BeginChild(skCryptDec("Inventory.Controller"), ImVec2(190, 560), true);
    {
        ImGui::TextUnformatted(skCryptDec("Otomasyon Isaretleri"));
        ImGui::Separator();

        ImGui::Spacing();
        {
            ImGui::SetNextItemWidth(174);

            auto jInventoryFlags = Drawing::Bot->GetInventoryFlags();

            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255.0f, 0.0f, 0.0f, 1.0f));

            if (ImGui::Button(skCryptDec("Sil"), ImVec2(174.0f, 0.0f)))
            {
                for (auto e : m_vecInventorySelectedItem)
                {
                    bool bExist = false;

                    for (size_t i = 0; i < jInventoryFlags.size(); i++)
                    {
                        if (jInventoryFlags[i]["id"].get<uint32_t>() == e)
                        {
                            jInventoryFlags[i]["flag"] = INVENTORY_ITEM_FLAG_DELETE;

                            bExist = true;
                            break;
                        }
                    }

                    if (!bExist)
                    {
                        JSON jNewInventoryFlag;

                        std::string szIdAttribute = skCryptDec("id");
                        std::string szFlagAttribute = skCryptDec("flag");

                        jNewInventoryFlag[szIdAttribute.c_str()] = e;
                        jNewInventoryFlag[szFlagAttribute.c_str()] = INVENTORY_ITEM_FLAG_DELETE;

                        jInventoryFlags.push_back(jNewInventoryFlag);
                    }
                }

                Drawing::Bot->UpdateInventoryItemFlag(jInventoryFlags);

                m_vecInventorySelectedItem.clear();
            }

            ImGui::PopStyleColor(1);

            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255.0f, 215.0f, 0.0f, 1.0f));

            if (ImGui::Button(skCryptDec("Sat"), ImVec2(174.0f, 0.0f)))
            {
                for (auto e : m_vecInventorySelectedItem)
                {
                    bool bExist = false;

                    for (size_t i = 0; i < jInventoryFlags.size(); i++)
                    {
                        if (jInventoryFlags[i]["id"].get<uint32_t>() == e)
                        {
                            jInventoryFlags[i]["flag"] = INVENTORY_ITEM_FLAG_SELL;

                            bExist = true;
                            break;
                        }
                    }

                    if (!bExist)
                    {
                        JSON jNewInventoryFlag;

                        std::string szIdAttribute = skCryptDec("id");
                        std::string szFlagAttribute = skCryptDec("flag");

                        jNewInventoryFlag[szIdAttribute.c_str()] = e;
                        jNewInventoryFlag[szFlagAttribute.c_str()] = INVENTORY_ITEM_FLAG_SELL;

                        jInventoryFlags.push_back(jNewInventoryFlag);
                    }
                }

                Drawing::Bot->UpdateInventoryItemFlag(jInventoryFlags);

                m_vecInventorySelectedItem.clear();
            }

            ImGui::PopStyleColor(1);

            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 255.0f, 0.0f, 1.0f));

            if (ImGui::Button(skCryptDec("Inn Hostes"), ImVec2(174.0f, 0.0f)))
            {
                for (auto e : m_vecInventorySelectedItem)
                {
                    bool bExist = false;

                    for (size_t i = 0; i < jInventoryFlags.size(); i++)
                    {
                        if (jInventoryFlags[i]["id"].get<uint32_t>() == e)
                        {
                            jInventoryFlags[i]["flag"] = INVENTORY_ITEM_FLAG_INN;

                            bExist = true;
                            break;
                        }
                    }

                    if (!bExist)
                    {
                        JSON jNewInventoryFlag;

                        std::string szIdAttribute = skCryptDec("id");
                        std::string szFlagAttribute = skCryptDec("flag");

                        jNewInventoryFlag[szIdAttribute.c_str()] = e;
                        jNewInventoryFlag[szFlagAttribute.c_str()] = INVENTORY_ITEM_FLAG_INN;

                        jInventoryFlags.push_back(jNewInventoryFlag);
                    }
                }

                Drawing::Bot->UpdateInventoryItemFlag(jInventoryFlags);

                m_vecInventorySelectedItem.clear();

            }

            ImGui::PopStyleColor(1);

            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 255.0f, 215.0f, 1.0f));

            if (ImGui::Button(skCryptDec("VIP Storage"), ImVec2(174.0f, 0.0f)))
            {
                for (auto e : m_vecInventorySelectedItem)
                {
                    bool bExist = false;

                    for (size_t i = 0; i < jInventoryFlags.size(); i++)
                    {
                        if (jInventoryFlags[i]["id"].get<uint32_t>() == e)
                        {
                            jInventoryFlags[i]["flag"] = INVENTORY_ITEM_FLAG_VIP;

                            bExist = true;
                            break;
                        }
                    }

                    if (!bExist)
                    {
                        JSON jNewInventoryFlag;

                        std::string szIdAttribute = skCryptDec("id");
                        std::string szFlagAttribute = skCryptDec("flag");

                        jNewInventoryFlag[szIdAttribute.c_str()] = e;
                        jNewInventoryFlag[szFlagAttribute.c_str()] = INVENTORY_ITEM_FLAG_VIP;

                        jInventoryFlags.push_back(jNewInventoryFlag);
                    }
                }

                Drawing::Bot->UpdateInventoryItemFlag(jInventoryFlags);

                m_vecInventorySelectedItem.clear();
            }

            ImGui::PopStyleColor(1);

            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255, 0, 255, 1.0f));

            if (ImGui::Button(skCryptDec("RPR Gelince Giy"), ImVec2(174.0f, 0.0f)))
            {
                for (auto e : m_vecInventorySelectedItem)
                {
                    bool bExist = false;

                    for (size_t i = 0; i < jInventoryFlags.size(); i++)
                    {
                        if (jInventoryFlags[i]["id"].get<uint32_t>() == e)
                        {
                            jInventoryFlags[i]["flag"] = INVENTORY_ITEM_FLAG_EQUIP;

                            bExist = true;
                            break;
                        }
                    }

                    if (!bExist)
                    {
                        JSON jNewInventoryFlag;

                        std::string szIdAttribute = skCryptDec("id");
                        std::string szFlagAttribute = skCryptDec("flag");

                        jNewInventoryFlag[szIdAttribute.c_str()] = e;
                        jNewInventoryFlag[szFlagAttribute.c_str()] = INVENTORY_ITEM_FLAG_EQUIP;

                        jInventoryFlags.push_back(jNewInventoryFlag);
                    }
                }

                Drawing::Bot->UpdateInventoryItemFlag(jInventoryFlags);

                m_vecInventorySelectedItem.clear();
            }

            ImGui::PopStyleColor(1);

            if (ImGui::Button(skCryptDec("Isareti Sifirla"), ImVec2(174.0f, 0.0f)))
            {
                for (auto e : m_vecInventorySelectedItem)
                {
                    for (auto it = jInventoryFlags.begin(); it != jInventoryFlags.end(); ++it)
                    {
                        std::string szIdAttribute = skCryptDec("id");

                        if ((*it)[szIdAttribute.c_str()] == e)
                        {
                            jInventoryFlags.erase(it);
                            break;
                        }
                    }
                }

                Drawing::Bot->UpdateInventoryItemFlag(jInventoryFlags);

                m_vecInventorySelectedItem.clear();
            }
        }
    }
    ImGui::EndChild();
}

void Drawing::DrawAutoLootArea()
{
    ImGui::Spacing();
    {
        ImGui::TextUnformatted(skCryptDec("Oto Kutu Ayarlari"));
        ImGui::Separator();

        ImGui::Spacing();
        {
            bool bAutoLoot = m_pUserConfiguration->GetBool(skCryptDec("AutoLoot"), skCryptDec("Enable"), false);

            if (ImGui::Checkbox(skCryptDec("##AutoLoot"), &bAutoLoot))
                m_pUserConfiguration->SetInt(skCryptDec("AutoLoot"), skCryptDec("Enable"), bAutoLoot ? 1 : 0);

            ImGui::SameLine();

            ImGui::Text(skCryptDec("Topla"));

            ImGui::SameLine();

            bool bMoveToLoot = m_pUserConfiguration->GetBool(skCryptDec("AutoLoot"), skCryptDec("MoveToLoot"), false);

            if (ImGui::Checkbox(skCryptDec("##MoveToLoot"), &bMoveToLoot))
            {
                m_pClient->SetAuthority(bMoveToLoot ? 0 : 1);
                m_pUserConfiguration->SetInt(skCryptDec("AutoLoot"), skCryptDec("MoveToLoot"), bMoveToLoot ? 1 : 0);
            }

            ImGui::SameLine();

            ImGui::Text(skCryptDec("Kutuya Kos"));

            ImGui::Text(skCryptDec("Satis Fiyatina Gore Topla"));

            ImGui::SameLine();

            ImGui::PushItemWidth(80);

            int iLootMinPrice = m_pUserConfiguration->GetInt(skCryptDec("AutoLoot"), skCryptDec("MinPrice"), 0);

            if (ImGui::DragInt(skCryptDec("##LootMinPrice"), &iLootMinPrice, 1, 0, INT_MAX))
                m_pUserConfiguration->SetInt(skCryptDec("AutoLoot"), skCryptDec("MinPrice"), iLootMinPrice);

            ImGui::PopItemWidth();
        }
    }
}

void Drawing::DrawScene()
{
    ImGui::BeginChild(1, ImVec2(350, 670), true);
    {
        if (ImGui::BeginTabBar(skCryptDec("##KOF.TabBar"), ImGuiTabBarFlags_None))
        {
            if (ImGui::BeginTabItem(skCryptDec("Genel")))
            {
                DrawProtectionArea();

                ImGui::Separator();

                DrawMainSettingsArea();

                ImGui::Separator();

                DrawAutoLootArea();

                ImGui::Separator();

                DrawTransformationArea();

                if (!m_pClient->IsPriest())
                    ImGui::BeginDisabled();

                ImGui::Separator();

                DrawPriestManagementArea();

                if (!m_pClient->IsPriest())
                    ImGui::EndDisabled();

                if(!m_pClient->IsRogue())
                    ImGui::BeginDisabled();

                ImGui::Separator();

                DrawRogueManagementArea();

                if (!m_pClient->IsRogue())
                    ImGui::EndDisabled();

                ImGui::EndTabItem();

            }      

            if (ImGui::BeginTabItem(skCryptDec("Saldiri")))
            {
                DrawAttackSettingsController();

                ImGui::Separator();

                DrawSpeedController();

                ImGui::Separator();

                DrawDistanceController();

                ImGui::Separator();

                DrawSizeController();

                ImGui::EndTabItem();
            }    

            if (ImGui::BeginTabItem(skCryptDec("Hedef")))
            {
                DrawTargetListController();

                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem(skCryptDec("Skill")))
            {
                DrawSkillController();

                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem(skCryptDec("Tedarik")))
            {
                DrawSupplySettingsArea();

                ImGui::Separator();

                DrawSupplyListArea();

                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem(skCryptDec("Diger")))
            {
                DrawRouteListController();

                ImGui::Separator();

                DrawFlashController();

                ImGui::Separator();

                DrawListenerController();

                ImGui::Separator();

                DrawSystemSettingsController();

                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }

        ImGui::EndChild();
    }

    ImGui::BeginChild(2, ImVec2(350, 90), true);
    {
        DrawBottomControllerArea();

        ImGui::EndChild();
    }
}

#endif