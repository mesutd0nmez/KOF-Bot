#include "pch.h"
#include "Drawing.h"
#include "ClientHandler.h"
#include <algorithm>
#include <cstring>
#include <iostream>

LPCSTR Drawing::lpWindowName = "   ";
ImVec2 Drawing::vWindowSize = { 0, 0 };
ImGuiWindowFlags Drawing::WindowFlags = ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize;
bool Drawing::bDraw = true;
UI::WindowItem Drawing::lpSelectedWindow = { nullptr, "", "" };
LPDIRECT3DDEVICE9 Drawing::pD3DDevice = nullptr;

Bot* Drawing::Bot = nullptr;

ClientHandler* m_pClient = nullptr;
Ini* m_pUserConfiguration = nullptr;
Ini* m_iniAppConfiguration = nullptr;

char m_szEmail[255] = "";
char m_szPassword[255] = "";

char m_szPlannedRouteName[255] = "";

char m_szItemSearchName[255] = "";

int m_iInventoryManagementWindow = 0;

/**
	@brief : function that check if the menu is drawed.
	@retval : true if the function is drawed else false.
**/
bool Drawing::isActive()
{
	return bDraw == true;
}

/**
	@brief : Function that draw the ImGui menu.
**/
void Drawing::Draw()
{
	if (isActive())
	{
        m_pUserConfiguration = Drawing::Bot->GetUserConfiguration();
        m_iniAppConfiguration = Drawing::Bot->GetAppConfiguration();
        m_pClient = Drawing::Bot->GetClientHandler();

		if (!UI::IsWindowTargeted())
		{
            HWND hTargetWindow = FindWindow(skCryptDec("Knight OnLine Client"), NULL);

            if (hTargetWindow != nullptr)
            {
                Injection(Drawing::Bot->GetInjectedProcessId(), skCryptDec("Adapter.dll"));
                //Injection(Drawing::Bot->GetInjectedProcessId(), skCryptDec("C:\\Users\\Administrator\\Documents\\GitHub\\Pipeline\\Debug\\Pipeline.dll"));
                UI::SetTargetWindow(hTargetWindow);
                bDraw = false;
            }
            else
            {
                vWindowSize = { 265, 170 };

                float fScreenWidth = (GetSystemMetrics(SM_CXSCREEN)) / 2.0f;
                float fScreenHeight = (GetSystemMetrics(SM_CYSCREEN)) / 2.0f;

                ImVec2 vec2InitialPos = { fScreenWidth, fScreenHeight };

                vec2InitialPos.x -= vWindowSize.x / 2;
                vec2InitialPos.y -= vWindowSize.y / 2;
               
                ImGui::SetNextWindowPos(vec2InitialPos, ImGuiCond_Once);
                ImGui::SetNextWindowSize(vWindowSize, ImGuiCond_Once);
                ImGui::SetNextWindowBgAlpha(1.0f);

                ImGui::Begin(lpWindowName, &bDraw, WindowFlags);
                {
                    bool bIsConnected = Drawing::Bot->IsConnected();
                    bool bIsAuthenticated = Drawing::Bot->IsAuthenticated();

                    if (!bIsConnected || bIsAuthenticated)
                        ImGui::BeginDisabled();

                    if (bIsAuthenticated)
                    {
                        std::string strEmail = m_iniAppConfiguration->GetString(skCryptDec("Internal"), skCryptDec("User"), "");
                        strncpy(m_szEmail, strEmail.c_str(), sizeof(m_szEmail) - 1);
                        m_szEmail[sizeof(m_szEmail) - 1] = '\0';

                        std::string strPassword = skCryptDec("****************");
                        strncpy(m_szPassword, strPassword.c_str(), sizeof(m_szPassword) - 1);
                        m_szPassword[sizeof(m_szPassword) - 1] = '\0';
                    }

                    ImGui::BeginChild(skCryptDec("##LoginBoard"), ImVec2(250, 95), true);
                    {
                        ImGui::SetCursorPos(ImVec2(10, 10));
                        ImGui::Text(skCryptDec("E-Posta"));
                        ImGui::SameLine();
                        ImGui::SetCursorPos(ImVec2(80, 7));
                        ImGui::InputText(skCryptDec("##EMail"), &m_szEmail[0], 100);

                        ImGui::SetCursorPos(ImVec2(10, 36));
                        ImGui::Text(skCryptDec("Sifre"));
                        ImGui::SameLine();
                        ImGui::SetCursorPos(ImVec2(80, 35));
                        ImGui::InputText(skCryptDec("##Password"), &m_szPassword[0], 25, ImGuiInputTextFlags_Password);

                        ImGui::Spacing();

                        if (ImGui::Button(skCryptDec("Etkinlestir"), ImVec2(235.0f, 0.0f)))
                        {
                            Drawing::Bot->SendLogin(m_szEmail, m_szPassword);
                            m_iniAppConfiguration->SetString(skCryptDec("Internal"), skCryptDec("User"), m_szEmail);
                            memset(m_szPassword, 0, sizeof(m_szPassword));
                        }
                    }

                    ImGui::EndChild();

                    if (!bIsConnected || bIsAuthenticated)
                        ImGui::EndDisabled();

                    ImGui::BeginChild(skCryptDec("##ActivationFooter"), ImVec2(250, 33), true);
                    {
                        if (Drawing::Bot->IsAuthenticated())
                        {
                            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
                            ImGui::Text(skCryptDec("Oyun baslatiliyor, lutfen bekleyin %c"), "|/-\\"[(int)(ImGui::GetTime() / 0.05f) & 3]);
                            ImGui::PopStyleColor();
                        }
                        else
                        {
                            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
                            ImGui::Text(skCryptDec("Aktivasyon islemi bekleniyor"));
                            ImGui::PopStyleColor();
                        }
                    }

                    ImGui::EndChild();
                }

                ImGui::End();
            }
		}
        else
        {
            if (Drawing::Bot->GetUserConfiguration() != nullptr)
            {
                vWindowSize = { 800, 650 };

                RECT rect;
                GetWindowRect(UI::hTargetWindow, &rect);
                int width = rect.right - rect.left;
                int height = rect.bottom - rect.top;

                float fScreenWidth = width / 2.0f;
                float fScreenHeight = height / 2.0f;

                ImVec2 vec2InitialPos = { fScreenWidth, fScreenHeight };

                vec2InitialPos.x -= vWindowSize.x / 2;
                vec2InitialPos.y -= vWindowSize.y / 2;

                ImGui::SetNextWindowPos(vec2InitialPos, ImGuiCond_Appearing);
                ImGui::SetNextWindowSize(vWindowSize, ImGuiCond_Appearing);
                ImGui::SetNextWindowBgAlpha(1.0f);

                ImGui::Begin(lpWindowName, &bDraw, WindowFlags);
                {
                    bool bTableLoaded = Drawing::Bot->IsTableLoaded();

                    if (!bTableLoaded)
                        ImGui::BeginDisabled();

                    ImGui::BeginChild(skCryptDec("##Main.Child1"), ImVec2(288, 610), true);
                    {

                        DrawMainController();
                        ImGui::Separator();
                        DrawStatisticsController();
                       
                        ImGui::Separator();
                        DrawModeController();
                    }
                    ImGui::EndChild();

                    ImGui::SameLine();

                    ImGui::BeginChild(skCryptDec("##Main.Child2"), ImVec2(488, 610), true);
                    {
                        if (ImGui::BeginTabBar(skCryptDec("##Main.TabBar"), ImGuiTabBarFlags_None))
                        {
                            if (ImGui::BeginTabItem(skCryptDec("Genel")))
                            {
                                DrawProtectionController();
                                ImGui::Separator();
                              
                                DrawMainSettingsArea();
                                ImGui::Separator();

                                DrawTransformationController();
                                ImGui::Separator();

                                DrawPartyController();
                                ImGui::Separator();

                                DrawFlashController();
                                ImGui::Separator();

                                DrawSizeController();
                                ImGui::Separator();

                                DrawSaveCPUController();
                                ImGui::Separator();

                                DrawListenerController();

                                ImGui::EndTabItem();
                            }

                            if (ImGui::BeginTabItem(skCryptDec("Hedef")))
                            {
                                DrawDistanceController();
                                ImGui::Separator();

                                DrawTargetListController();

                                ImGui::EndTabItem();
                            }

                            if (ImGui::BeginTabItem(skCryptDec("Skill")))
                            {
                                DrawSkillController();

                                ImGui::EndTabItem();
                            }

                            if (ImGui::BeginTabItem(skCryptDec("Saldiri")))
                            {
                                DrawAttackController();

                                ImGui::Separator();
                                DrawSpeedController();

                                ImGui::EndTabItem();
                            }

                            if (ImGui::BeginTabItem(skCryptDec("Rota")))
                            {
                                DrawRouteListController();
                                ImGui::Separator();
                                DrawRoutePlannerController();

                                ImGui::EndTabItem();
                            }

                            if (ImGui::BeginTabItem(skCryptDec("Item")))
                            {
                                DrawAutoLootController();
                                ImGui::Separator();
                                DrawItemListController();

                                ImGui::EndTabItem();
                            }

                            if (ImGui::BeginTabItem(skCryptDec("Tedarik")))
                            {
                                DrawSupplyController();
                                ImGui::Separator();
                                DrawSupplyListController();

                                ImGui::EndTabItem();
                            }

                            if (ImGui::BeginTabItem(skCryptDec("Sistem")))
                            {
                                DrawSettingsController();

                                ImGui::EndTabItem();
                            }

                            if (ImGui::BeginTabItem(skCryptDec("Ekstra")))
                            {
                                DrawExtraController();
                                ImGui::Separator();

                                DrawLevelDownerController();

                                ImGui::EndTabItem();
                            }

                            ImGui::EndTabBar();
                        }
                    }
                    ImGui::EndChild();

                    if (!bTableLoaded)
                        ImGui::EndDisabled();
                }

                ImGui::End();
            }   
        }
	}
}

/**
	@brief : Function that set the D3D9 device and execute user D3D9 rendering.
	@param pCurrentD3DDevice : current D3D9 device.
**/
void Drawing::DXDraw(const LPDIRECT3DDEVICE9 pCurrentD3DDevice)
{
	pD3DDevice = pCurrentD3DDevice;
	DrawFilledRectangle(100, 100, 500, 500, 255, 255, 255);
}

/**
	@brief : Function that draw a filed rectangle.
**/
void Drawing::DrawFilledRectangle(const int x, const int y, const int w, const int h, const unsigned char r, const unsigned char g, const unsigned char b)
{
	const D3DCOLOR rectColor = D3DCOLOR_XRGB(r, g, b);	//No point in using alpha because clear & alpha dont work!
	const D3DRECT BarRect = { x, y, x + w, y + h };

	pD3DDevice->Clear(1, &BarRect, D3DCLEAR_TARGET | D3DCLEAR_TARGET, rectColor, 0, 0);
}

void Drawing::DrawStatisticsController()
{
    ImGui::BulletText(skCryptDec("Kontrol Paneli"));
    ImGui::Separator();

    ImGui::Spacing();
    {
        ImGui::Text(skCryptDec("Uptime"));

        ImGui::SameLine();
        ImGui::SetCursorPosX(145);
        ImGui::Text(skCryptDec(":"));
        ImGui::SameLine();
        ImGui::Text(calculateElapsedTime(Drawing::Bot->m_startTime).c_str());

        ImGui::Text(skCryptDec("Baslangic Coin"));
        ImGui::SameLine();
        ImGui::SetCursorPosX(145);
        ImGui::Text(skCryptDec(":"));
        ImGui::SameLine();

        std::ostringstream iStartCoinFormatted;
        iStartCoinFormatted.imbue(std::locale(skCryptDec("tr_TR")));
        iStartCoinFormatted << std::fixed << m_pClient->m_iStartCoin;
        ImGui::Text("%s", iStartCoinFormatted.str().c_str());

        ImGui::Text(skCryptDec("Kazanilan Coin"));
        ImGui::SameLine();
        ImGui::SetCursorPosX(145);
        ImGui::Text(skCryptDec(":"));
        ImGui::SameLine();

        std::ostringstream iCoinCounterFormatted;
        iCoinCounterFormatted.imbue(std::locale(skCryptDec("tr_TR")));
        iCoinCounterFormatted << std::fixed << m_pClient->m_iCoinCounter;
        ImGui::Text("%s", iCoinCounterFormatted.str().c_str());

        ImGui::Text(skCryptDec("Kazanilan Exp"));
        ImGui::SameLine();
        ImGui::SetCursorPosX(145);
        ImGui::Text(skCryptDec(":"));
        ImGui::SameLine();

        std::ostringstream iExpCounterFormatted;
        iExpCounterFormatted.imbue(std::locale(skCryptDec("tr_TR")));
        iExpCounterFormatted << std::fixed << m_pClient->m_iExpCounter;
        ImGui::Text("%s", iExpCounterFormatted.str().c_str());

        ImGui::Text(skCryptDec("Dakikada Gelen Coin"));
        ImGui::SameLine();
        ImGui::SetCursorPosX(145);
        ImGui::Text(skCryptDec(":"));
        ImGui::SameLine();

        std::ostringstream iEveryMinuteCoinCounterFormatted;
        iEveryMinuteCoinCounterFormatted.imbue(std::locale(skCryptDec("tr_TR")));
        iEveryMinuteCoinCounterFormatted << std::fixed << m_pClient->m_iEveryMinuteCoinCounter - m_pClient->m_iEveryMinuteCoinPrevCounter;
        ImGui::Text("%s", iEveryMinuteCoinCounterFormatted.str().c_str());

        ImGui::Text(skCryptDec("Dakikada Gelen Exp"));
        ImGui::SameLine();
        ImGui::SetCursorPosX(145);
        ImGui::Text(skCryptDec(":"));
        ImGui::SameLine();

        std::ostringstream iEveryMinuteExpCounterFormatted;
        iEveryMinuteExpCounterFormatted.imbue(std::locale(skCryptDec("tr_TR")));
        iEveryMinuteExpCounterFormatted << std::fixed << m_pClient->m_iEveryMinuteExpCounter - m_pClient->m_iEveryMinuteExpPrevCounter;
        ImGui::Text("%s", iEveryMinuteExpCounterFormatted.str().c_str());

        ImGui::Separator();

        if (ImGui::Button(skCryptDec("Sayaclari Sifirla"), ImVec2(272.0f, 0.0f)))
        {
            m_pClient->m_iStartCoin = m_pClient->m_PlayerMySelf.iGold;
            m_pClient->m_iCoinCounter = 0;
            m_pClient->m_iExpCounter = 0;
            m_pClient->m_iEveryMinuteCoinPrevCounter = 0;
            m_pClient->m_iEveryMinuteExpPrevCounter = 0;
            m_pClient->m_iEveryMinuteCoinCounter = 0;
            m_pClient->m_iEveryMinuteExpCounter = 0;

            m_pClient->RemoveItem(0);
        }

        if (ImGui::Button(skCryptDec("Tum Ayarlari Sifirla"), ImVec2(272.0f, 0.0f)))
        {
            m_pUserConfiguration->Reset();
            m_pClient->ClearUserConfiguration();
        }

        if (ImGui::Button(skCryptDec("Skill Listesini Sifirla"), ImVec2(272.0f, 0.0f)))
        {
            m_pClient->m_vecAttackSkillList = m_pUserConfiguration->SetInt(skCryptDec("Automation"), skCryptDec("AttackSkillList"), std::unordered_set<int>());
            m_pClient->m_vecCharacterSkillList = m_pUserConfiguration->SetInt(skCryptDec("Automation"), skCryptDec("CharacterSkillList"), std::unordered_set<int>());

            m_pClient->LoadSkillData();
        }
    };
}

void Drawing::DrawMainController()
{
    ImGui::BulletText(skCryptDec("Ana Kontroller"));
    ImGui::Separator();

    ImGui::Spacing();
    {
        if (m_pClient->m_bAttackStatus)
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.255f, 0.0f, 1.0f));
        else
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.255f, 0.0f, 0.0f, 1.0f));

        if (ImGui::Button(m_pClient->m_bAttackStatus ? skCryptDec("Saldiri Durdur") : skCryptDec("Saldiri Baslat"), ImVec2(132.0f, 0.0f)))
        {
            m_pClient->m_bAttackStatus = !m_pClient->m_bAttackStatus;
            m_pUserConfiguration->SetInt(skCryptDec("Automation"), skCryptDec("Attack"), m_pClient->m_bAttackStatus);
        }

        ImGui::PopStyleColor(1);
        ImGui::SameLine();

        if (m_pClient->m_bCharacterStatus)
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.255f, 0.0f, 1.0f));
        else
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.255f, 0.0f, 0.0f, 1.0f));

        if (ImGui::Button(m_pClient->m_bCharacterStatus ? skCryptDec("Bot Durdur") : skCryptDec("Bot Baslat"), ImVec2(132.0f, 0.0f)))
        {
            m_pClient->m_bCharacterStatus = !m_pClient->m_bCharacterStatus;
            m_pUserConfiguration->SetInt(skCryptDec("Automation"), skCryptDec("Character"), m_pClient->m_bCharacterStatus);
        }

        ImGui::PopStyleColor(1);

        if (ImGui::Button(skCryptDec("Town At"), ImVec2(132.0f, 0.0f)))
        {
            m_pClient->SendTownPacket();
        }

        ImGui::SameLine();

        if (ImGui::Button(skCryptDec("Oyunu Kapat"), ImVec2(132.0f, 0.0f)))
        {
            TerminateMyProcess(Drawing::Bot->GetInjectedProcessId(), -1);
        }
    };
}

void Drawing::DrawTransformationController()
{
    ImGui::BulletText(skCryptDec("Oto TS Ayarlari"));
    ImGui::Separator();

    ImGui::Spacing();
    {
        if (ImGui::Checkbox(skCryptDec("##AutoTransformationCheckBox"), &m_pClient->m_bAutoTransformation))
            m_pUserConfiguration->SetInt(skCryptDec("Transformation"), skCryptDec("Auto"), m_pClient->m_bAutoTransformation ? 1 : 0);

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

        vecDisguiseItems.push_back(DisguiseItem(381001000, "TS Scroll (60 Dakika)"));
        vecDisguiseItems.push_back(DisguiseItem(-1, "TS Scroll (90 Dakika)"));
        vecDisguiseItems.push_back(DisguiseItem(379090000, "TS Totem 1"));
        vecDisguiseItems.push_back(DisguiseItem(379093000, "TS Totem 2"));

        std::string szSelectedTransformationItem = "TS Scroll";

        const auto pFindedTransformationItem = std::find_if(vecDisguiseItems.begin(), vecDisguiseItems.end(),
            [&](const DisguiseItem& a) { return a.iID == m_pClient->m_iTransformationItem; });

        if (pFindedTransformationItem != vecDisguiseItems.end())
            szSelectedTransformationItem = pFindedTransformationItem->szName;

        if (ImGui::BeginCombo(skCryptDec("##Transformation.ItemList"), szSelectedTransformationItem.c_str()))
        {
            for (auto& e : vecDisguiseItems)
            {
                const bool bIsSelected = m_pClient->m_iTransformationItem == e.iID;

                if (ImGui::Selectable(e.szName.c_str(), bIsSelected))
                {
                    m_pClient->m_iTransformationItem = m_pUserConfiguration->SetInt(skCryptDec("Transformation"), skCryptDec("Item"), e.iID);
                    m_pClient->m_iTransformationSkill = m_pUserConfiguration->SetInt(skCryptDec("Transformation"), skCryptDec("Skill"), 0);
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

        std::string szSelectedTransformationSkill = "";

        int iTransformationItem = -1;

        if (m_pClient->m_iTransformationItem == -1)
        {
            iTransformationItem = 379090000;
        }
        else
        {
            iTransformationItem = m_pClient->m_iTransformationItem;
        }

        std::map<uint32_t, __TABLE_DISGUISE_RING>* mapDisguiseTable;
        if (Drawing::Bot->GetDisguiseRingTable(&mapDisguiseTable))
        {
            for (auto& [k, v] : *mapDisguiseTable)
            {
                if (m_pClient->m_PlayerMySelf.iLevel < v.iRequiredLevel)
                    continue;

                if (iTransformationItem != v.iItemID)
                    continue;

                if (m_pClient->m_iTransformationSkill == v.iSkillID)
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
                const bool bIsSelected = m_pClient->m_iTransformationSkill == e.iID;

                if (ImGui::Selectable(e.szName.c_str(), bIsSelected))
                {
                    m_pClient->m_iTransformationSkill = m_pUserConfiguration->SetInt(skCryptDec("Transformation"), skCryptDec("Skill"), e.iID);
                }

                if (bIsSelected)
                    ImGui::SetItemDefaultFocus();
            }

            ImGui::EndCombo();
        }
    }
}

void Drawing::DrawSpeedController()
{
    ImGui::BulletText(skCryptDec("Hiz Ayarlari"));
    ImGui::Separator();

    ImGui::Spacing();
    {
        if (ImGui::Checkbox(skCryptDec("##SearchTargetSpeedCheckbox"), &m_pClient->m_bSearchTargetSpeed))
            m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("SearchTargetSpeed"), m_pClient->m_bSearchTargetSpeed ? 1 : 0);

        ImGui::SameLine();

        ImGui::Text(skCryptDec("Mob Tarama Hizi (ms)"));

        ImGui::SameLine();

        ImGui::PushItemWidth(75);

        if (ImGui::DragInt(skCryptDec("##SearchTargetSpeedValue"), &m_pClient->m_iSearchTargetSpeedValue, 1, 0, 65535))
            m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("SearchTargetSpeedValue"), m_pClient->m_iSearchTargetSpeedValue);

        ImGui::PopItemWidth();

        ImGui::SameLine();

        if (ImGui::Checkbox(skCryptDec("##AttackSpeedCheckbox"), &m_pClient->m_bAttackSpeed))
            m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("AttackSpeed"), m_pClient->m_bAttackSpeed ? 1 : 0);

        ImGui::SameLine();

        ImGui::Text(skCryptDec("Saldiri Hizi (ms)"));

        ImGui::SameLine();

        ImGui::PushItemWidth(75);

        if (ImGui::DragInt(skCryptDec("##AttackSpeedValue"), &m_pClient->m_iAttackSpeedValue, 1, 0, 65535))
            m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("AttackSpeedValue"), m_pClient->m_iAttackSpeedValue);

        ImGui::PopItemWidth();

    }
}

void Drawing::DrawDistanceController()
{
    ImGui::BulletText(skCryptDec("Mesafe Ayarlari"));
    ImGui::Separator();

    ImGui::Spacing();
    {
        if (ImGui::Checkbox(skCryptDec("##RangeLimitCheckbox"), &m_pClient->m_bRangeLimit))
            m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("RangeLimit"), m_pClient->m_bRangeLimit ? 1 : 0);

        ImGui::SameLine();

        ImGui::Text(skCryptDec("Mob Tarama Mesafesi (m)"));

        ImGui::SameLine();

        ImGui::PushItemWidth(50);

        if (ImGui::DragInt(skCryptDec("##RangeLimitValue"), &m_pClient->m_iRangeLimitValue, 1, 0, 100))
            m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("RangeLimitValue"), m_pClient->m_iRangeLimitValue);

        ImGui::PopItemWidth();

        ImGui::SameLine();

        if (ImGui::Checkbox(skCryptDec("##AttackRangeLimitCheckbox"), &m_pClient->m_bAttackRangeLimit))
            m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("AttackRangeLimit"), m_pClient->m_bAttackRangeLimit ? 1 : 0);

        ImGui::SameLine();

        ImGui::Text(skCryptDec("Saldiri Mesafesi (m)"));

        ImGui::SameLine();

        ImGui::PushItemWidth(50);

        if (ImGui::DragInt(skCryptDec("##AttackRangeLimitValue"), &m_pClient->m_iAttackRangeLimitValue, 1, 0, 100))
            m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("AttackRangeLimitValue"), m_pClient->m_iAttackRangeLimitValue);

        ImGui::PopItemWidth();
    }
}

void Drawing::DrawProtectionController()
{
    ImGui::BulletText(skCryptDec("Koruma Ayarlari"));
    ImGui::Separator();

    ImGui::Spacing();
    {
        if (ImGui::Checkbox(skCryptDec("##HpPotionCheckbox"), &m_pClient->m_bHpProtectionEnable))
            m_pUserConfiguration->SetInt(skCryptDec("Protection"), skCryptDec("Hp"), m_pClient->m_bHpProtectionEnable ? 1 : 0);

        ImGui::SameLine();

        ImGui::Text(skCryptDec("Hp Potion (%%)"));

        ImGui::SameLine();

        ImGui::PushItemWidth(50);

        if (ImGui::DragInt(skCryptDec("##HpPotionValue"), &m_pClient->m_iHpProtectionValue, 1, 0, 100))
            m_pUserConfiguration->SetInt(skCryptDec("Protection"), skCryptDec("HpValue"), m_pClient->m_iHpProtectionValue);

        ImGui::PopItemWidth();

        ImGui::SameLine();

        if (!m_pClient->IsRogue())
            ImGui::BeginDisabled();

        if (ImGui::Checkbox(skCryptDec("##MinorCheckbox"), &m_pClient->m_bMinorProtection))
            m_pUserConfiguration->SetInt(skCryptDec("Protection"), skCryptDec("Minor"), m_pClient->m_bMinorProtection ? 1 : 0);

        ImGui::SameLine();

        ImGui::Text(skCryptDec("Minor (%%)"));

        ImGui::SameLine();

        ImGui::PushItemWidth(50);

        if (ImGui::DragInt(skCryptDec("##MinorValue"), &m_pClient->m_iMinorProtectionValue, 1, 0, 100))
            m_pUserConfiguration->SetInt(skCryptDec("Protection"), skCryptDec("MinorValue"), m_pClient->m_iMinorProtectionValue);

        ImGui::PopItemWidth();

        if (!m_pClient->IsRogue())
            ImGui::EndDisabled();

        if (ImGui::Checkbox(skCryptDec("##MpPotionCheckbox"), &m_pClient->m_bMpProtectionEnable))
            m_pUserConfiguration->SetInt(skCryptDec("Protection"), skCryptDec("Mp"), m_pClient->m_bMpProtectionEnable ? 1 : 0);

        ImGui::SameLine();

        ImGui::Text(skCryptDec("Mp Potion (%%)"));

        ImGui::SameLine();

        ImGui::PushItemWidth(50);

        if (ImGui::DragInt(skCryptDec("##MpPotionValue"), &m_pClient->m_iMpProtectionValue, 1, 0, 100))
            m_pUserConfiguration->SetInt(skCryptDec("Protection"), skCryptDec("MpValue"), m_pClient->m_iMpProtectionValue);

        ImGui::PopItemWidth();

        ImGui::SameLine();

        if (!m_pClient->IsPriest())
            ImGui::BeginDisabled();

        if (ImGui::Checkbox(skCryptDec("##HealCheckbox"), &m_pClient->m_bHealProtection))
            m_pUserConfiguration->SetInt(skCryptDec("Protection"), skCryptDec("Heal"), m_pClient->m_bHealProtection ? 1 : 0);

        ImGui::SameLine();

        ImGui::Text(skCryptDec("Heal (%%)"));

        ImGui::SameLine();

        ImGui::PushItemWidth(50);

        if (ImGui::DragInt(skCryptDec("##HealValue"), &m_pClient->m_iHealProtectionValue, 1, 0, 100))
            m_pUserConfiguration->SetInt(skCryptDec("Protection"), skCryptDec("HealValue"), m_pClient->m_iHealProtectionValue);

        ImGui::PopItemWidth();

        if (!m_pClient->IsPriest())
            ImGui::EndDisabled();
    }
}

void Drawing::DrawAutoLootController()
{
    ImGui::BulletText(skCryptDec("Oto Kutu Ayarlari"));
    ImGui::Separator();

    ImGui::Spacing();
    {
        if (ImGui::Checkbox(skCryptDec("##AutoLoot"), &m_pClient->m_bAutoLoot))
        {
            if (!m_pClient->m_bAutoLoot)
            {
                m_pClient->m_vecLootList.clear();
            }

            m_pUserConfiguration->SetInt(skCryptDec("AutoLoot"), skCryptDec("Enable"), m_pClient->m_bAutoLoot ? 1 : 0);
        }

        ImGui::SameLine();

        ImGui::Text(skCryptDec("Kutu Topla"));

        ImGui::SameLine();

        if (ImGui::Checkbox(skCryptDec("##MoveToLoot"), &m_pClient->m_bMoveToLoot))
        {
            if (!m_pClient->m_bMoveToLoot)
            {
                m_pClient->SetMovingToLoot(false);
            }

            m_pClient->m_bMoveToLoot = m_pUserConfiguration->SetInt(skCryptDec("AutoLoot"), skCryptDec("MoveToLoot"), m_pClient->m_bMoveToLoot ? 1 : 0);
        }

        ImGui::SameLine();

        ImGui::Text(skCryptDec("Kutuya Kos"));

        if (ImGui::Checkbox(skCryptDec("##MinPriceEnable"), &m_pClient->m_bMinPriceLootEnable))
        {
            m_pClient->m_bMinPriceLootEnable =  m_pUserConfiguration->SetInt(skCryptDec("AutoLoot"), skCryptDec("MinPriceLootEnable"), m_pClient->m_bMinPriceLootEnable ? 1 : 0);
        }

        ImGui::SameLine();

        ImGui::Text(skCryptDec("Satis Fiyatina Gore Topla"));

        ImGui::SameLine();

        ImGui::PushItemWidth(80);

        if (ImGui::DragInt(skCryptDec("##LootMinPrice"), &m_pClient->m_iLootMinPrice, 1, 0, INT_MAX))
            m_pUserConfiguration->SetInt(skCryptDec("AutoLoot"), skCryptDec("MinPrice"), m_pClient->m_iLootMinPrice);

        ImGui::PopItemWidth();

        if(ImGui::RadioButton("Hepsini Topla", &m_pClient->m_iLootType, 0))
            m_pClient->m_iLootType = m_pUserConfiguration->SetInt(skCryptDec("AutoLoot"), skCryptDec("LootType"), m_pClient->m_iLootType);

        ImGui::SameLine();

        if (ImGui::RadioButton("Sadece Coin Topla", &m_pClient->m_iLootType, 1))
            m_pClient->m_iLootType = m_pUserConfiguration->SetInt(skCryptDec("AutoLoot"), skCryptDec("LootType"), m_pClient->m_iLootType);

        ImGui::SameLine();

        if (ImGui::RadioButton("Item Listesine Gore Topla", &m_pClient->m_iLootType, 2))
            m_pClient->m_iLootType = m_pUserConfiguration->SetInt(skCryptDec("AutoLoot"), skCryptDec("LootType"), m_pClient->m_iLootType);
    }
}

void Drawing::DrawItemListController()
{
    ImGui::BulletText(skCryptDec("Item Listesi Yonetimi"));
    ImGui::Separator();

    ImGui::Spacing();
    {
        ImGui::PushItemWidth(240);
        ImGui::InputTextWithHint(skCryptDec("##ItemSearchName"), skCryptDec("Bir esyayi adiyla ara"), & m_szItemSearchName[0], 100);
        ImGui::PopItemWidth();

        ImGui::SameLine();

        ImGui::RadioButton("Item Listesi", &m_iInventoryManagementWindow, 0); 
        ImGui::SameLine();
        ImGui::RadioButton("Inventory", &m_iInventoryManagementWindow, 1);

        if (ImGui::Button(skCryptDec("Tum Itemleri Topla"), ImVec2(151.0f, 0.0f)))
        {
            std::map<uint32_t, __TABLE_ITEM>* pItemTable;

            if (Drawing::Bot->GetItemTable(&pItemTable))
            {
                m_pClient->m_vecLootItemList.clear();

                for (auto it = pItemTable->begin(); it != pItemTable->end(); it++)
                {
                    m_pClient->m_vecLootItemList.insert(it->second.iID);
                }

                m_pClient->m_vecLootItemList = m_pUserConfiguration->SetInt(skCryptDec("Automation"), skCryptDec("LootItemList"), m_pClient->m_vecLootItemList);
            }
        }

        ImGui::SameLine();

        if (ImGui::Button(skCryptDec("Tum Itemleri Sat"), ImVec2(151.0f, 0.0f)))
        {
            std::map<uint32_t, __TABLE_ITEM>* pItemTable;

            if (Drawing::Bot->GetItemTable(&pItemTable))
            {
                m_pClient->m_vecSellItemList.clear();

                for (auto it = pItemTable->begin(); it != pItemTable->end(); it++)
                {
                    m_pClient->m_vecSellItemList.insert(it->second.iID);
                }

                m_pClient->m_vecSellItemList = m_pUserConfiguration->SetInt(skCryptDec("Automation"), skCryptDec("SellItemList"), m_pClient->m_vecSellItemList);
            }
        }

        ImGui::SameLine();

        if (ImGui::Button(skCryptDec("Listeyi Sifirla"), ImVec2(151.0f, 0.0f)))
        {
            m_pClient->m_vecLootItemList.clear();
            m_pClient->m_vecLootItemList = m_pUserConfiguration->SetInt(skCryptDec("Automation"), skCryptDec("LootItemList"), m_pClient->m_vecLootItemList);

            m_pClient->m_vecSellItemList.clear();
            m_pClient->m_vecSellItemList = m_pUserConfiguration->SetInt(skCryptDec("Automation"), skCryptDec("SellItemList"), m_pClient->m_vecSellItemList);

            m_pClient->m_vecInnItemList.clear();
            m_pClient->m_vecInnItemList = m_pUserConfiguration->SetInt(skCryptDec("Automation"), skCryptDec("InnItemList"), m_pClient->m_vecInnItemList);

            m_pClient->m_vecDeleteItemList.clear();
            m_pClient->m_vecDeleteItemList = m_pUserConfiguration->SetInt(skCryptDec("Automation"), skCryptDec("DeleteItemList"), m_pClient->m_vecDeleteItemList);
        }

        ImGui::BeginChild(skCryptDec("ItemListChild"), ImVec2(472, 320), true);
        {
            if (ImGui::BeginTable(skCryptDec("ItemList.Table"), 5, ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_Borders))
            {
                ImGui::TableSetupColumn(skCryptDec("Item"), ImGuiTableColumnFlags_WidthFixed, 270);
                ImGui::TableSetupColumn(skCryptDec("Topla"), ImGuiTableColumnFlags_WidthFixed, 35);
                ImGui::TableSetupColumn(skCryptDec("Sat"), ImGuiTableColumnFlags_WidthFixed, 35);
                ImGui::TableSetupColumn(skCryptDec("Banka"), ImGuiTableColumnFlags_WidthFixed, 35);
                ImGui::TableSetupColumn(skCryptDec("Sil"), ImGuiTableColumnFlags_WidthFixed, 35);
                ImGui::TableHeadersRow();

                std::map<uint32_t, __TABLE_ITEM>* pItemTable;
                if (Drawing::Bot->GetItemTable(&pItemTable))
                {
                    if (m_iInventoryManagementWindow == 0)
                    {
                        int i = 0;
                        for (auto it = pItemTable->begin(); it != pItemTable->end(); it++)
                        {
                            bool bRenderRow = false;

                            bool bLootSelected = m_pClient->m_vecLootItemList.count(it->second.iID) > 0;
                            bool bSellSelected = m_pClient->m_vecSellItemList.count(it->second.iID) > 0;
                            bool bInnSelected = m_pClient->m_vecInnItemList.count(it->second.iID) > 0;
                            bool bDeleteSelected = m_pClient->m_vecDeleteItemList.count(it->second.iID) > 0;

                            if (strlen(m_szItemSearchName) == 0)
                            {
                                if (bLootSelected || bSellSelected || bInnSelected || bDeleteSelected)
                                {
                                    bRenderRow = true;
                                }
                            }
                            else
                            {
                                if (stristr(it->second.szName.c_str(), m_szItemSearchName) != nullptr)
                                {
                                    bRenderRow = true;
                                }
                            }

                            if (bRenderRow)
                            {
                                ImGui::PushID(i++);
                                ImGui::TableNextRow();
                                ImGui::TableNextColumn();
                                ImGui::Text(it->second.szName.c_str());
                                ImGui::TableNextColumn();

                                if (ImGui::Checkbox(skCryptDec("##LootItemList"), &bLootSelected))
                                {
                                    if (bLootSelected)
                                        m_pClient->m_vecLootItemList.insert(it->second.iID);
                                    else
                                        m_pClient->m_vecLootItemList.erase(std::find(m_pClient->m_vecLootItemList.begin(), m_pClient->m_vecLootItemList.end(), it->second.iID));

                                    m_pClient->m_vecLootItemList = m_pUserConfiguration->SetInt(skCryptDec("Automation"), skCryptDec("LootItemList"), m_pClient->m_vecLootItemList);
                                }

                                ImGui::TableNextColumn();

                                if (ImGui::Checkbox(skCryptDec("##SellItemList"), &bSellSelected))
                                {
                                    if (bSellSelected)
                                        m_pClient->m_vecSellItemList.insert(it->second.iID);
                                    else
                                        m_pClient->m_vecSellItemList.erase(std::find(m_pClient->m_vecSellItemList.begin(), m_pClient->m_vecSellItemList.end(), it->second.iID));

                                    m_pClient->m_vecSellItemList = m_pUserConfiguration->SetInt(skCryptDec("Automation"), skCryptDec("SellItemList"), m_pClient->m_vecSellItemList);
                                }

                                ImGui::TableNextColumn();

                                if (ImGui::Checkbox(skCryptDec("##InnItemList"), &bInnSelected))
                                {
                                    if (bInnSelected)
                                        m_pClient->m_vecInnItemList.insert(it->second.iID);
                                    else
                                        m_pClient->m_vecInnItemList.erase(std::find(m_pClient->m_vecInnItemList.begin(), m_pClient->m_vecInnItemList.end(), it->second.iID));

                                    m_pClient->m_vecInnItemList = m_pUserConfiguration->SetInt(skCryptDec("Automation"), skCryptDec("InnItemList"), m_pClient->m_vecInnItemList);
                                }

                                ImGui::TableNextColumn();

                                if (ImGui::Checkbox(skCryptDec("##DeleteItemList"), &bDeleteSelected))
                                {
                                    if (bDeleteSelected)
                                        m_pClient->m_vecDeleteItemList.insert(it->second.iID);
                                    else
                                        m_pClient->m_vecDeleteItemList.erase(std::find(m_pClient->m_vecDeleteItemList.begin(), m_pClient->m_vecDeleteItemList.end(), it->second.iID));

                                    m_pClient->m_vecDeleteItemList = m_pUserConfiguration->SetInt(skCryptDec("Automation"), skCryptDec("DeleteItemList"), m_pClient->m_vecDeleteItemList);
                                }

                                ImGui::PopID();
                            }
                        }
                    }
                    else
                    {
                        std::vector<TItemData> vecInventoryItemList;
                        m_pClient->GetInventoryItemList(vecInventoryItemList);

                        int i = 0;
                        for (const TItemData& pItem : vecInventoryItemList)
                        {
                            if (pItem.iItemID == 0)
                                continue;

                            __TABLE_ITEM* pItemData;
                            if (!Drawing::Bot->GetItemData(pItem.iItemID, pItemData))
                                continue;

                            if (strlen(m_szItemSearchName) > 0 
                                && stristr(pItemData->szName.c_str(), m_szItemSearchName) == nullptr)
                                continue;


                            uint32_t iItemBaseID = pItem.iItemID / 1000 * 1000;

                            bool bLootSelected = m_pClient->m_vecLootItemList.count(iItemBaseID) > 0;
                            bool bSellSelected = m_pClient->m_vecSellItemList.count(iItemBaseID) > 0;
                            bool bInnSelected = m_pClient->m_vecInnItemList.count(iItemBaseID) > 0;
                            bool bDeleteSelected = m_pClient->m_vecDeleteItemList.count(iItemBaseID) > 0;

                            ImGui::PushID(i++);
                            ImGui::TableNextRow();
                            ImGui::TableNextColumn();
                            ImGui::Text(pItemData->szName.c_str());
                            ImGui::TableNextColumn();

                            if (ImGui::Checkbox(skCryptDec("##LootItemList"), &bLootSelected))
                            {
                                if (bLootSelected)
                                    m_pClient->m_vecLootItemList.insert(iItemBaseID);
                                else
                                    m_pClient->m_vecLootItemList.erase(std::find(m_pClient->m_vecLootItemList.begin(), m_pClient->m_vecLootItemList.end(), iItemBaseID));

                                m_pClient->m_vecLootItemList = m_pUserConfiguration->SetInt(skCryptDec("Automation"), skCryptDec("LootItemList"), m_pClient->m_vecLootItemList);
                            }

                            ImGui::TableNextColumn();

                            if (ImGui::Checkbox(skCryptDec("##SellItemList"), &bSellSelected))
                            {
                                if (bSellSelected)
                                    m_pClient->m_vecSellItemList.insert(iItemBaseID);
                                else
                                    m_pClient->m_vecSellItemList.erase(std::find(m_pClient->m_vecSellItemList.begin(), m_pClient->m_vecSellItemList.end(), iItemBaseID));

                                m_pClient->m_vecSellItemList = m_pUserConfiguration->SetInt(skCryptDec("Automation"), skCryptDec("SellItemList"), m_pClient->m_vecSellItemList);
                            }

                            ImGui::TableNextColumn();

                            if (ImGui::Checkbox(skCryptDec("##InnItemList"), &bInnSelected))
                            {
                                if (bInnSelected)
                                    m_pClient->m_vecInnItemList.insert(iItemBaseID);
                                else
                                    m_pClient->m_vecInnItemList.erase(std::find(m_pClient->m_vecInnItemList.begin(), m_pClient->m_vecInnItemList.end(), iItemBaseID));

                                m_pClient->m_vecInnItemList = m_pUserConfiguration->SetInt(skCryptDec("Automation"), skCryptDec("InnItemList"), m_pClient->m_vecInnItemList);
                            }

                            ImGui::TableNextColumn();

                            if (ImGui::Checkbox(skCryptDec("##DeleteItemList"), &bDeleteSelected))
                            {
                                if (bDeleteSelected)
                                    m_pClient->m_vecDeleteItemList.insert(iItemBaseID);
                                else
                                    m_pClient->m_vecDeleteItemList.erase(std::find(m_pClient->m_vecDeleteItemList.begin(), m_pClient->m_vecDeleteItemList.end(), iItemBaseID));

                                m_pClient->m_vecDeleteItemList = m_pUserConfiguration->SetInt(skCryptDec("Automation"), skCryptDec("DeleteItemList"), m_pClient->m_vecDeleteItemList);
                            }

                            ImGui::PopID();
                        }

                    }
                    
                }

                

                ImGui::EndTable();
            }
        }
        ImGui::EndChild();
    }
}

void Drawing::DrawAttackController()
{
    ImGui::BulletText(skCryptDec("Saldiri Ayarlari"));
    ImGui::Separator();

    ImGui::Spacing();
    {
        if (ImGui::Checkbox(skCryptDec("##BasicAttackCheckbox"), &m_pClient->m_bBasicAttack))
            m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("BasicAttack"), m_pClient->m_bBasicAttack ? 1 : 0);

        ImGui::SameLine();

        ImGui::Text(skCryptDec("R Vur"));

        if (!m_pClient->m_bBasicAttack)
            ImGui::BeginDisabled();

        ImGui::SameLine();

        if (ImGui::Checkbox(skCryptDec("##BasicAttackWithPacketCheckbox"), &m_pClient->m_bBasicAttackWithPacket))
            m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("BasicAttackWithPacket"), m_pClient->m_bBasicAttackWithPacket ? 1 : 0);

        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255.0f, 0.0f, 0.0f, 1.0f));
        ImGui::Text(skCryptDec("R Illegal Vur"));
        ImGui::PopStyleColor();

        if (!m_pClient->m_bBasicAttack)
            ImGui::EndDisabled();
    }
}

void Drawing::DrawTargetListController()
{
    ImGui::BulletText(skCryptDec("Hedef Ayarlari"));
    ImGui::Separator();

    ImGui::Spacing();
    {
        if (ImGui::Checkbox(skCryptDec("##MoveToTargetCheckbox"), &m_pClient->m_bMoveToTarget))
            m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("MoveToTarget"), m_pClient->m_bMoveToTarget ? 1 : 0);

        ImGui::SameLine();

        ImGui::Text(skCryptDec("Hedefe Kos"));

        ImGui::SameLine();

        if (ImGui::Checkbox(skCryptDec("##ClosestTargetCheckbox"), &m_pClient->m_bClosestTarget))
            m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("ClosestTarget"), m_pClient->m_bClosestTarget ? 1 : 0);

        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255.0f, 0.0f, 0.0f, 1.0f));
        ImGui::Text(skCryptDec("Herzaman En Yakin Hedefe Vur"));
        ImGui::PopStyleColor();

        if (ImGui::Checkbox(skCryptDec("##PartyLeaderSelect"), &m_pClient->m_bPartyLeaderSelect))
        {
            m_pClient->m_bPartyLeaderSelect = m_pUserConfiguration->SetInt(skCryptDec("Bot"), skCryptDec("PartyLeaderSelect"), m_pClient->m_bPartyLeaderSelect ? 1 : 0);
        }

        ImGui::SameLine();

        ImGui::Text(skCryptDec("Party Baskaninin Sectigini Sec"));

        if (m_pClient->m_bAutoTarget)
            ImGui::BeginDisabled();

        if (ImGui::Button(skCryptDec("Listeye Mob ID Ekle"), ImVec2(470.0f, 0.0f)))
        {
            int32_t iTargetID = m_pClient->GetTarget();

            if (iTargetID != -1)
            {
                const auto pFindedID = std::find_if(m_pClient->m_vecSelectedNpcIDList.begin(), m_pClient->m_vecSelectedNpcIDList.end(),
                    [iTargetID](const auto& a) { return a == iTargetID; });

                if (pFindedID == m_pClient->m_vecSelectedNpcIDList.end())
                {
                    m_pClient->m_vecSelectedNpcIDList.insert(iTargetID);
                    m_pClient->m_vecSelectedNpcIDList = m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("NpcIDList"), m_pClient->m_vecSelectedNpcIDList);
                }
            }
        }

        if (m_pClient->m_bAutoTarget)
            ImGui::EndDisabled();

        ImGui::BeginChild(skCryptDec("TargetListController"), ImVec2(470.0f, 276), true);
        {
            if (m_pClient->m_bAutoTarget)
                ImGui::BeginDisabled();

            if (ImGui::TreeNodeEx(skCryptDec("Mob ID Listesi"), ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Framed))
            {
                for (const auto& x : m_pClient->m_vecSelectedNpcIDList)
                {
                    if (ImGui::Selectable(std::to_string(x).c_str(), true))
                    {
                        m_pClient->m_vecSelectedNpcIDList.erase(std::find(m_pClient->m_vecSelectedNpcIDList.begin(), m_pClient->m_vecSelectedNpcIDList.end(), x));
                        m_pClient->m_vecSelectedNpcIDList = m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("NpcIDList"), m_pClient->m_vecSelectedNpcIDList);
                    }
                }

                ImGui::TreePop();
            }

            ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Framed;
            if (ImGui::TreeNodeEx(skCryptDec("Yakinlardaki Hedefler"), flags))
            {
                std::vector<SNpcData> vecTargetList;

                for (const auto& x : m_pClient->m_vecSelectedNpcList)
                {
                    SNpcData pNpcData{};

                    pNpcData.iProtoID = x;

                    vecTargetList.push_back(pNpcData);
                }

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

                    bool bSelected = std::find(m_pClient->m_vecSelectedNpcList.begin(), m_pClient->m_vecSelectedNpcList.end(), x.iProtoID) != m_pClient->m_vecSelectedNpcList.end();

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
                            m_pClient->m_vecSelectedNpcList.insert(x.iProtoID);
                        else
                            m_pClient->m_vecSelectedNpcList.erase(std::find(m_pClient->m_vecSelectedNpcList.begin(), m_pClient->m_vecSelectedNpcList.end(), x.iProtoID));

                        m_pClient->m_vecSelectedNpcList = m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("NpcList"), m_pClient->m_vecSelectedNpcList);
                    }

                    ImGui::PopID();
                }

                ImGui::TreePop();
            }

            if (m_pClient->m_bAutoTarget)
                ImGui::EndDisabled();

        }
        ImGui::EndChild();
    }
}

void Drawing::DrawSkillController()
{
    ImGui::BulletText(skCryptDec("Skill Ayarlari"));
    ImGui::Separator();

    ImGui::Spacing();
    {
        if (ImGui::Checkbox(skCryptDec("##UseSkillWithPacket"), &m_pClient->m_bUseSkillWithPacket))
        {
            m_pUserConfiguration->SetInt(skCryptDec("Skill"), skCryptDec("UseSkillWithPacket"), m_pClient->m_bUseSkillWithPacket ? 1 : 0);
        }

        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255.0f, 0.0f, 0.0f, 1.0f));
        ImGui::Text(skCryptDec("Skilleri Illegal Vur"));
        ImGui::PopStyleColor();

        if (!m_pClient->m_bUseSkillWithPacket)
            ImGui::BeginDisabled();

        ImGui::SameLine();

        if (ImGui::Checkbox(skCryptDec("##OnlyAttackSkillUseWithPacket"), &m_pClient->m_bOnlyAttackSkillUseWithPacket))
        {
            m_pUserConfiguration->SetInt(skCryptDec("Skill"), skCryptDec("OnlyAttackSkillUseWithPacket"), m_pClient->m_bOnlyAttackSkillUseWithPacket ? 1 : 0);
        }

        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
        ImGui::Text(skCryptDec("Sadece Saldiri Skillerini Illegal Vur"));
        ImGui::PopStyleColor();

        if (!m_pClient->m_bUseSkillWithPacket)
            ImGui::EndDisabled();

        ImGui::BeginChild(skCryptDec("SkillController1"), ImVec2(232.0f, 386), true);
        {
            if (ImGui::TreeNodeEx(skCryptDec("Saldiri Skilleri"), ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Framed))
            {
                for (const auto& x : m_pClient->m_vecAvailableSkill)
                {
                    if (x.iTarget != SkillTargetType::TARGET_ENEMY_ONLY && x.iTarget != SkillTargetType::TARGET_AREA_ENEMY)
                        continue;

                    ImGui::PushID(x.iID);

                    bool bSelected = std::find(m_pClient->m_vecAttackSkillList.begin(), m_pClient->m_vecAttackSkillList.end(), x.iID) != m_pClient->m_vecAttackSkillList.end();

                    if (ImGui::Selectable(x.szName.c_str(), &bSelected))
                    {
                        if (bSelected)
                            m_pClient->m_vecAttackSkillList.insert(x.iID);
                        else
                            m_pClient->m_vecAttackSkillList.erase(std::find(m_pClient->m_vecAttackSkillList.begin(), m_pClient->m_vecAttackSkillList.end(), x.iID));

                        m_pClient->m_vecAttackSkillList = m_pUserConfiguration->SetInt(skCryptDec("Automation"), skCryptDec("AttackSkillList"), m_pClient->m_vecAttackSkillList);
                    }

                    ImGui::PopID();
                }

                ImGui::TreePop();
            }

            ImGui::EndChild();
        }

        ImGui::SameLine();

        ImGui::BeginChild(skCryptDec("SkillController2"), ImVec2(232.0f, 386), true);
        {
            if (ImGui::TreeNodeEx(skCryptDec("Karakter Skilleri"), ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Framed))
            {
                for (const auto& x : m_pClient->m_vecAvailableSkill)
                {
                    if (x.iTarget != SkillTargetType::TARGET_SELF
                        && x.iTarget != SkillTargetType::TARGET_PARTY
                        && x.iTarget != SkillTargetType::TARGET_PARTY_ALL
                        && x.iTarget != SkillTargetType::TARGET_FRIEND_WITHME
                        && x.iTarget != SkillTargetType::TARGET_FRIEND_ONLY)
                        continue;

                    ImGui::PushID(x.iID);

                    bool bSelected = std::find(m_pClient->m_vecCharacterSkillList.begin(), m_pClient->m_vecCharacterSkillList.end(), x.iID) != m_pClient->m_vecCharacterSkillList.end();

                    if (ImGui::Selectable(x.szName.c_str(), &bSelected))
                    {
                        if (bSelected)
                            m_pClient->m_vecCharacterSkillList.insert(x.iID);
                        else
                            m_pClient->m_vecCharacterSkillList.erase(std::find(m_pClient->m_vecCharacterSkillList.begin(), m_pClient->m_vecCharacterSkillList.end(), x.iID));

                        m_pClient->m_vecCharacterSkillList = m_pUserConfiguration->SetInt(skCryptDec("Automation"), skCryptDec("CharacterSkillList"), m_pClient->m_vecCharacterSkillList);
                    }

                    ImGui::PopID();
                }

                ImGui::TreePop();
            }

            ImGui::EndChild();
        }

    }
}

void Drawing::DrawSizeController()
{
    ImGui::BulletText(skCryptDec("Buyutme Ayarlari"));
    ImGui::Separator();

    ImGui::Spacing();
    {
        if (ImGui::Checkbox(skCryptDec("##TargetSizeCheckbox"), &m_pClient->m_bTargetSizeEnable))
            m_pClient->m_bTargetSizeEnable = m_pUserConfiguration->SetInt(skCryptDec("Target"), skCryptDec("SizeEnable"), m_pClient->m_bTargetSizeEnable ? 1 : 0);

        ImGui::SameLine();

        ImGui::Text(skCryptDec("Hedef"));

        ImGui::SameLine();

        ImGui::PushItemWidth(125);

        if (ImGui::SliderInt(skCryptDec("##TargetSize"), &m_pClient->m_iTargetSize, 1, 10))
            m_pClient->m_iTargetSize = m_pUserConfiguration->SetInt(skCryptDec("Target"), skCryptDec("Size"), m_pClient->m_iTargetSize);

        ImGui::PopItemWidth();

        ImGui::SameLine();

        if (ImGui::Checkbox(skCryptDec("##CharacterSizeCheckbox"), &m_pClient->m_bCharacterSizeEnable))
        {
            m_pClient->m_bCharacterSizeEnable = m_pUserConfiguration->SetInt(skCryptDec("Character"), skCryptDec("SizeEnable"), m_pClient->m_bCharacterSizeEnable ? 1 : 0);

            DWORD iMyBase = Drawing::Bot->Read4Byte(Drawing::Bot->GetAddress(skCryptDec("KO_PTR_CHR")));

            if (m_pClient->m_bCharacterSizeEnable)
                m_pClient->SetScale(iMyBase, (float)m_pClient->m_iCharacterSize, (float)m_pClient->m_iCharacterSize, (float)m_pClient->m_iCharacterSize);
            else
                m_pClient->SetScale(iMyBase, 1.0f, 1.0f, 1.0f);
        }

        ImGui::SameLine();

        ImGui::Text(skCryptDec("Karakter"));

        ImGui::SameLine();

        ImGui::PushItemWidth(125);

        if (ImGui::SliderInt(skCryptDec("##CharacterSize"), &m_pClient->m_iCharacterSize, 1, 10))
        {
            m_pClient->m_iCharacterSize = m_pUserConfiguration->SetInt(skCryptDec("Character"), skCryptDec("Size"), m_pClient->m_iCharacterSize);

            if (m_pClient->m_bCharacterSizeEnable)
            {
                DWORD iMyBase = Drawing::Bot->Read4Byte(Drawing::Bot->GetAddress(skCryptDec("KO_PTR_CHR")));
                m_pClient->SetScale(iMyBase, (float)m_pClient->m_iCharacterSize, (float)m_pClient->m_iCharacterSize, (float)m_pClient->m_iCharacterSize);
            }
        }

        ImGui::PopItemWidth();
    }
}

void Drawing::DrawSaveCPUController()
{
    ImGui::BulletText(skCryptDec("Save CPU Ayarlari"));
    ImGui::Separator();

    ImGui::Spacing();
    {
        if (ImGui::Checkbox(skCryptDec("##SaveCPUCheckbox"), &m_pClient->m_bSaveCPUEnable))
        {
            m_pUserConfiguration->SetInt(skCryptDec("Feature"), skCryptDec("SaveCPU"), m_pClient->m_bSaveCPUEnable ? 1 : 0);

            if (!m_pClient->m_bSaveCPUEnable)
            {
                m_pClient->SetSaveCPUSleepTime(0);
            }
            else
            {
                m_pClient->SetSaveCPUSleepTime(m_pClient->m_iSaveCPUValue);
            }
        }

        ImGui::SameLine();

        ImGui::Text(skCryptDec("Save CPU"));

        ImGui::SameLine();

        ImGui::PushItemWidth(160);

        if (ImGui::SliderInt(skCryptDec("##SaveCPUValue"), &m_pClient->m_iSaveCPUValue, 1, 100))
        {
            m_pUserConfiguration->SetInt(skCryptDec("Feature"), skCryptDec("SaveCPUValue"), m_pClient->m_iSaveCPUValue);

            if (m_pClient->m_bSaveCPUEnable)
            {
                m_pClient->SetSaveCPUSleepTime(m_pClient->m_iSaveCPUValue);
            }
        }

        ImGui::PopItemWidth();
    }
}

void Drawing::DrawSupplyController()
{
    ImGui::BulletText(skCryptDec("Tedarik Ayarlari"));
    ImGui::Separator();

    ImGui::Spacing();
    {
        if (ImGui::Checkbox(skCryptDec("##AutoSupply"), &m_pClient->m_bAutoSupply))
            m_pClient->m_bAutoSupply = m_pUserConfiguration->SetInt(skCryptDec("Supply"), skCryptDec("AutoSupply"), m_pClient->m_bAutoSupply ? 1 : 0);

        ImGui::SameLine();

        ImGui::Text(skCryptDec("Oto Tedarik"));

        ImGui::SameLine();

        if (ImGui::Checkbox(skCryptDec("##AutoRepair"), &m_pClient->m_bAutoRepair))
            m_pUserConfiguration->SetInt(skCryptDec("Supply"), skCryptDec("AutoRepair"), m_pClient->m_bAutoRepair ? 1 : 0);

        ImGui::SameLine();

        ImGui::Text(skCryptDec("Oto RPR (NPC)"));

        ImGui::SameLine();

        if (ImGui::Checkbox(skCryptDec("##AutoRepairMagicHammer"), &m_pClient->m_bAutoRepairMagicHammer))
        {
            m_pUserConfiguration->SetInt(skCryptDec("Supply"), skCryptDec("AutoRepairMagicHammer"), m_pClient->m_bAutoRepairMagicHammer ? 1 : 0);
        }

        ImGui::SameLine();

        ImGui::Text(skCryptDec("Oto RPR (Magic Hammer)"));

    }
}

void Drawing::DrawSupplyListController()
{
    ImGui::BulletText(skCryptDec("Tedarik Listesi"));
    ImGui::Separator();

    ImGui::Spacing();
    {
        auto jSupplyList = Drawing::Bot->GetSupplyList();

        for (size_t i = 0; i < jSupplyList.size(); i++)
        {
            ImGui::PushID(i);

            std::string szItemIdAttribute = skCryptDec("itemid");
            std::string szNameAttribute = skCryptDec("name");
            std::string szCountAttribute = skCryptDec("count");

            bool bSelected = std::find(m_pClient->m_vecSupplyList.begin(), m_pClient->m_vecSupplyList.end(), jSupplyList[i][szItemIdAttribute.c_str()].get<int>()) != m_pClient->m_vecSupplyList.end();

            if (ImGui::Checkbox(skCryptDec("##Enable"), &bSelected))
            {
                if (bSelected)
                    m_pClient->m_vecSupplyList.insert(jSupplyList[i][szItemIdAttribute.c_str()].get<int>());
                else
                    m_pClient->m_vecSupplyList.erase(std::find(m_pClient->m_vecSupplyList.begin(), m_pClient->m_vecSupplyList.end(), jSupplyList[i][szItemIdAttribute.c_str()].get<int>()));

                m_pClient->m_vecSupplyList = m_pUserConfiguration->SetInt(skCryptDec("Supply"), skCryptDec("Enable"), m_pClient->m_vecSupplyList);
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


void Drawing::DrawListenerController()
{
    ImGui::BulletText(skCryptDec("Dinleme Ayarlari"));
    ImGui::Separator();

    ImGui::Spacing();
    {
        if (ImGui::Checkbox(skCryptDec("##PartyRequest"), &m_pClient->m_bPartyRequest))
            m_pUserConfiguration->SetInt(skCryptDec("Listener"), skCryptDec("PartyRequest"), m_pClient->m_bPartyRequest ? 1 : 0);

        ImGui::SameLine();

        ImGui::Text(skCryptDec("Oto Party"));

        ImGui::SameLine();

        ImGui::PushItemWidth(125);

        if (ImGui::InputText(skCryptDec("##PartyRequestMessage"), &m_pClient->m_szPartyRequestMessage[0], 100))
        {
            m_pUserConfiguration->SetString(skCryptDec("Listener"), skCryptDec("PartyRequestMessage"), &m_pClient->m_szPartyRequestMessage[0]);
        }

        ImGui::PopItemWidth();

        ImGui::SameLine();

        if (!m_pClient->IsMage())
            ImGui::BeginDisabled();

        if (ImGui::Checkbox(skCryptDec("##TeleportRequest"), &m_pClient->m_bTeleportRequest))
            m_pUserConfiguration->SetInt(skCryptDec("Listener"), skCryptDec("TeleportRequest"), m_pClient->m_bTeleportRequest ? 1 : 0);

        ImGui::SameLine();

        ImGui::Text(skCryptDec("Oto TP"));

        ImGui::SameLine();

        ImGui::PushItemWidth(125);

        if (ImGui::InputText(skCryptDec("##TeleportRequestMessage"), &m_pClient->m_szTeleportRequestMessage[0], 100))
        {
            m_pUserConfiguration->SetString(skCryptDec("Listener"), skCryptDec("TeleportRequestMessage"), &m_pClient->m_szTeleportRequestMessage[0]);
        }

        if (!m_pClient->IsMage())
            ImGui::EndDisabled();
    }
}

void Drawing::DrawPartyController()
{
    ImGui::BulletText(skCryptDec("Party Ayarlari"));
    ImGui::Separator();

    ImGui::Spacing();
    {
        if (!m_pClient->IsRogue())
            ImGui::BeginDisabled();

        if (ImGui::Checkbox(skCryptDec("##RoguePartySwift"), &m_pClient->m_bPartySwift))
            m_pUserConfiguration->SetInt(skCryptDec("Rogue"), skCryptDec("PartySwift"), m_pClient->m_bPartySwift ? 1 : 0);

        ImGui::SameLine();

        ImGui::Text(skCryptDec("Party Swift"));

        if (!m_pClient->IsRogue())
            ImGui::EndDisabled();

        ImGui::SameLine();

        if (!m_pClient->IsPriest())
            ImGui::BeginDisabled();

        if (ImGui::Checkbox(skCryptDec("##PriestPartyHeal"), &m_pClient->m_bPriestPartyHeal))
            m_pUserConfiguration->SetInt(skCryptDec("Priest"), skCryptDec("PartyHeal"), m_pClient->m_bPriestPartyHeal ? 1 : 0);

        ImGui::SameLine();

        ImGui::Text(skCryptDec("Party Heal"));

        ImGui::SameLine();

        if (ImGui::Checkbox(skCryptDec("##PriestPartyBuff"), &m_pClient->m_bPriestPartyBuff))
            m_pUserConfiguration->SetInt(skCryptDec("Priest"), skCryptDec("PartyBuff"), m_pClient->m_bPriestPartyBuff ? 1 : 0);

        ImGui::SameLine();

        ImGui::Text(skCryptDec("Party Buff"));

        if (!m_pClient->IsPriest())
            ImGui::EndDisabled();
    }

}

void Drawing::DrawSettingsController()
{
    ImGui::BulletText(skCryptDec("Sistem & Bot Ayarlari"));
    ImGui::Separator();

    ImGui::Spacing();
    {
        if (ImGui::Checkbox(skCryptDec("##TownStopBot"), &m_pClient->m_bTownStopBot))
            m_pUserConfiguration->SetInt(skCryptDec("Bot"), skCryptDec("TownStopBot"), m_pClient->m_bTownStopBot ? 1 : 0);

        ImGui::SameLine();

        ImGui::Text(skCryptDec("Town atinca botu durdur"));

        if (ImGui::Checkbox(skCryptDec("##TownOrTeleportStopBot"), &m_pClient->m_bTownOrTeleportStopBot))
            m_pUserConfiguration->SetInt(skCryptDec("Bot"), skCryptDec("TownOrTeleportStopBot"), m_pClient->m_bTownOrTeleportStopBot ? 1 : 0);

        ImGui::SameLine();

        ImGui::Text(skCryptDec("Town veya Teleport olunca botu durdur"));

        if (ImGui::Checkbox(skCryptDec("##SyncWithGenie"), &m_pClient->m_bSyncWithGenie))
            m_pUserConfiguration->SetInt(skCryptDec("Bot"), skCryptDec("SyncWithGenie"), m_pClient->m_bSyncWithGenie ? 1 : 0);

        ImGui::SameLine();

        ImGui::Text(skCryptDec("Genie ile botu esitle"));

        if (ImGui::Checkbox(skCryptDec("##StartGenieIfUserInRegion"), &m_pClient->m_bStartGenieIfUserInRegion))
            m_pClient->m_bStartGenieIfUserInRegion = m_pUserConfiguration->SetInt(skCryptDec("Settings"), skCryptDec("StartGenieIfUserInRegion"), m_pClient->m_bStartGenieIfUserInRegion ? 1 : 0);

        ImGui::SameLine();

        ImGui::Text(skCryptDec("Yakinlarda oyuncu varsa Genie baslat"));

        if (ImGui::Checkbox(skCryptDec("##SendTownIfBanNotice"), &m_pClient->m_bSendTownIfBanNotice))
            m_pClient->m_bSendTownIfBanNotice = m_pUserConfiguration->SetInt(skCryptDec("Settings"), skCryptDec("SendTownIfBanNotice"), m_pClient->m_bSendTownIfBanNotice ? 1 : 0);

        ImGui::SameLine();

        ImGui::Text(skCryptDec("Ban Notice gecerse Town at"));

        if (ImGui::Checkbox(skCryptDec("##PlayBeepfIfBanNotice"), &m_pClient->m_bPlayBeepfIfBanNotice))
            m_pClient->m_bPlayBeepfIfBanNotice = m_pUserConfiguration->SetInt(skCryptDec("Settings"), skCryptDec("PlayBeepfIfBanNotice"), m_pClient->m_bPlayBeepfIfBanNotice ? 1 : 0);

        ImGui::SameLine();

        ImGui::Text(skCryptDec("Ban Notice gecerse BEEP sesi cal"));

        if (ImGui::Checkbox(skCryptDec("##SlotExpLimitEnable"), &m_pClient->m_bSlotExpLimitEnable))
            m_pUserConfiguration->SetInt(skCryptDec("Bot"), skCryptDec("SlotExpLimitEnable"), m_pClient->m_bSlotExpLimitEnable ? 1 : 0);

        ImGui::SameLine();

        ImGui::Text(skCryptDec("Exp miktari "));

        ImGui::SameLine();

        ImGui::PushItemWidth(75);

        if (ImGui::DragInt(skCryptDec("##SlotExpLimit"), &m_pClient->m_iSlotExpLimit, 35000, 1, 1000000))
            m_pClient->m_iSlotExpLimit = m_pUserConfiguration->SetInt(skCryptDec("Bot"), skCryptDec("SlotExpLimit"), m_pClient->m_iSlotExpLimit);

        ImGui::PopItemWidth();

        ImGui::SameLine();

        ImGui::Text(skCryptDec(" altindaysa illegal vur, aksi durumda legal vur"));


    }
}

void Drawing::SetLegalModeSettings(bool bMode)
{
    m_pClient->m_bLegalMode = m_pUserConfiguration->SetInt(skCryptDec("Automation"), skCryptDec("LegalMode"), bMode);

    if (bMode)
    {
        m_pClient->m_bAutoTransformation = m_pUserConfiguration->SetInt(skCryptDec("Transformation"), skCryptDec("Auto"), 0);

        m_pClient->m_bBasicAttackWithPacket = m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("BasicAttackWithPacket"), 0);
        m_pClient->m_bUseSkillWithPacket = m_pUserConfiguration->SetInt(skCryptDec("Skill"), skCryptDec("UseSkillWithPacket"), 0);
        m_pClient->m_bClosestTarget = m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("ClosestTarget"), 0);

        m_pClient->m_bAttackRangeLimit = m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("AttackRangeLimit"), 1);
        m_pClient->m_iAttackRangeLimitValue = m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("AttackRangeLimitValue"), 4);
    }
    else
    {
        m_pClient->m_bAutoTransformation = m_pUserConfiguration->SetInt(skCryptDec("Transformation"), skCryptDec("Auto"), 1);

        m_pClient->m_bBasicAttackWithPacket = m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("BasicAttackWithPacket"), 1);
        m_pClient->m_bUseSkillWithPacket = m_pUserConfiguration->SetInt(skCryptDec("Skill"), skCryptDec("UseSkillWithPacket"), 1);
        m_pClient->m_bClosestTarget = m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("ClosestTarget"), 1);

        m_pClient->m_bAttackRangeLimit = m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("AttackRangeLimit"), 1);
        m_pClient->m_iAttackRangeLimitValue = m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("AttackRangeLimitValue"), 50);

    }
}

void Drawing::SetSpeedModeSettings(bool bMode)
{
    m_pClient->m_bSpeedMode = m_pUserConfiguration->SetInt(skCryptDec("Automation"), skCryptDec("SpeedMode"), bMode);

    if (bMode)
    {
        m_pClient->m_bSearchTargetSpeed = m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("SearchTargetSpeed"), 1);
        m_pClient->m_iSearchTargetSpeedValue = m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("SearchTargetSpeedValue"), 0);

        m_pClient->m_bAttackSpeed = m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("AttackSpeed"), 1);
        m_pClient->m_iAttackSpeedValue = m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("AttackSpeedValue"), 900);

        m_pClient->m_bDeathEffect = m_pUserConfiguration->SetInt(skCryptDec("Feature"), skCryptDec("DeathEffect"), 1);
    }
    else
    {
        m_pClient->m_bSearchTargetSpeed = m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("SearchTargetSpeed"), 1);
        m_pClient->m_iSearchTargetSpeedValue = m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("SearchTargetSpeedValue"), 1000);

        m_pClient->m_bAttackSpeed = m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("AttackSpeed"), 1);
        m_pClient->m_iAttackSpeedValue = m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("AttackSpeedValue"), 1000);

        m_pClient->m_bDeathEffect = m_pUserConfiguration->SetInt(skCryptDec("Feature"), skCryptDec("DeathEffect"), 0);
    }
}

void Drawing::DrawModeController()
{
    ImGui::BulletText(skCryptDec("Bot Modu Kontrolleri"));
    ImGui::Separator();

    ImGui::Spacing();
    {
        if (!m_pClient->m_bLegalMode)
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.255f, 0.0f, 1.0f));
        else
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.255f, 0.0f, 0.0f, 1.0f));

        if (ImGui::Button("Illegal Mod", ImVec2(132.0f, 0.0f)))
        {
            m_pClient->m_bLegalMode = false;
            SetLegalModeSettings(m_pClient->m_bLegalMode);
        }

        ImGui::PopStyleColor(1);

        ImGui::SameLine();

        if (m_pClient->m_bLegalMode)
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.255f, 0.0f, 1.0f));
        else
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.255f, 0.0f, 0.0f, 1.0f));

        if (ImGui::Button(skCryptDec("Legal Mod"), ImVec2(132.0f, 0.0f)))
        {
            m_pClient->m_bLegalMode = true;
            SetLegalModeSettings(m_pClient->m_bLegalMode);
        }

        ImGui::PopStyleColor(1);

        if (m_pClient->m_bSpeedMode)
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.255f, 0.0f, 1.0f));
        else
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.255f, 0.0f, 0.0f, 1.0f));

        if (ImGui::Button(skCryptDec("Hizli Mod"), ImVec2(132.0f, 0.0f)))
        {
            m_pClient->m_bSpeedMode = true;
            SetSpeedModeSettings(m_pClient->m_bSpeedMode);
        }

        ImGui::PopStyleColor(1);
        ImGui::SameLine();

        if (!m_pClient->m_bSpeedMode)
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.255f, 0.0f, 1.0f));
        else
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.255f, 0.0f, 0.0f, 1.0f));

        if (ImGui::Button("Yavas Mod", ImVec2(132.0f, 0.0f)))
        {
            m_pClient->m_bSpeedMode = false;
            SetSpeedModeSettings(m_pClient->m_bSpeedMode);
        }

        ImGui::PopStyleColor(1);
    };
}

void Drawing::DrawFlashController()
{
    ImGui::BulletText(skCryptDec("Flash Ayarlari"));
    ImGui::Separator();

    ImGui::Spacing();
    {
        if (ImGui::Checkbox(skCryptDec("##AutoDCFlash"), &m_pClient->m_bAutoDCFlash))
            m_pUserConfiguration->SetInt(skCryptDec("Settings"), skCryptDec("AutoDCFlash"), m_pClient->m_bAutoDCFlash ? 1 : 0);

        ImGui::SameLine();

        ImGui::Text(skCryptDec("DC Flash"));

        ImGui::SameLine();

        ImGui::PushItemWidth(35);

        if (ImGui::DragInt(skCryptDec("##AutoDCFlashCount"), &m_pClient->m_iAutoDCFlashCount, 10, 1, 10))
            m_pUserConfiguration->SetInt(skCryptDec("Settings"), skCryptDec("AutoDCFlashCount"), m_pClient->m_iAutoDCFlashCount);

        ImGui::PopItemWidth();

        ImGui::SameLine();

        if (ImGui::Checkbox(skCryptDec("##AutoWarFlash"), &m_pClient->m_bAutoWarFlash))
            m_pUserConfiguration->SetInt(skCryptDec("Settings"), skCryptDec("AutoWarFlash"), m_pClient->m_bAutoWarFlash ? 1 : 0);

        ImGui::SameLine();

        ImGui::Text(skCryptDec("War Flash"));

        ImGui::SameLine();

        ImGui::PushItemWidth(35);

        if (ImGui::DragInt(skCryptDec("##AutoWarFlashCount"), &m_pClient->m_iAutoWarFlashCount, 10, 1, 10))
            m_pUserConfiguration->SetInt(skCryptDec("Settings"), skCryptDec("AutoWarFlashCount"), m_pClient->m_iAutoWarFlashCount);

        ImGui::PopItemWidth();

        ImGui::SameLine();

        if (ImGui::Checkbox(skCryptDec("##AutoExpFlash"), &m_pClient->m_bAutoExpFlash))
            m_pUserConfiguration->SetInt(skCryptDec("Settings"), skCryptDec("AutoExpFlash"), m_pClient->m_bAutoExpFlash ? 1 : 0);

        ImGui::SameLine();

        ImGui::Text(skCryptDec("Exp Flash"));

        ImGui::SameLine();

        ImGui::PushItemWidth(35);

        if (ImGui::DragInt(skCryptDec("##AutoExpFlashCount"), &m_pClient->m_iAutoExpFlashCount, 10, 1, 10))
            m_pUserConfiguration->SetInt(skCryptDec("Settings"), skCryptDec("AutoExpFlashCount"), m_pClient->m_iAutoExpFlashCount);

        ImGui::PopItemWidth();
    }
}

void Drawing::DrawRoutePlannerController()
{
    ImGui::BulletText(skCryptDec("Rota Planlayici"));
    ImGui::Separator();

    ImGui::Spacing();
    {
        if (m_pClient->m_bIsRoutePlanning)
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.255f, 0.0f, 0.0f, 1.0f));
        else
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.255f, 0.0f, 1.0f));

        if (ImGui::Button(m_pClient->m_bIsRoutePlanning ? skCryptDec("Rota Kaydetmeyi Bitir") : skCryptDec("Rota Kaydetmeye Basla"), ImVec2(470.0f, 0.0f)))
        {
            if (m_pClient->m_bIsRoutePlanning)
            {
                ImGui::OpenPopup(skCryptDec("Rotayi Kaydet"));
            }
            else
            {
                m_pClient->m_bIsRoutePlanning = !m_pClient->m_bIsRoutePlanning;
            }                 
        }

        ImGui::PopStyleColor();

        if (ImGui::BeginPopupModal(skCryptDec("Rotayi Kaydet")))
        {
            ImGui::Text(skCryptDec("Rotaniza bir isim verip kaydedin veya son rota planini sifirlayin"));

            ImGui::PushItemWidth(350);
            ImGui::InputText(skCryptDec("##RoutePlanName"), &m_szPlannedRouteName[0], sizeof(m_szPlannedRouteName));

            size_t routeNameSize = strlen(m_szPlannedRouteName);

            if (routeNameSize == 0)
                ImGui::BeginDisabled();

            if (ImGui::Button(skCryptDec("Kaydet")))
            {
                Drawing::Bot->GetRouteManager()->Save(m_szPlannedRouteName, m_pClient->GetZone(), m_pClient->m_vecRoutePlan);
                strcpy(m_szPlannedRouteName, "");
                m_pClient->m_vecRoutePlan.clear();
                m_pClient->m_bIsRoutePlanning = !m_pClient->m_bIsRoutePlanning;
                ImGui::CloseCurrentPopup();
            }

            if (routeNameSize == 0)
                ImGui::EndDisabled();

            ImGui::SameLine();

            if (ImGui::Button(skCryptDec("Sifirla")))
            {
                m_pClient->m_vecRoutePlan.clear();
                m_pClient->m_bIsRoutePlanning = !m_pClient->m_bIsRoutePlanning;
                ImGui::CloseCurrentPopup();
            }   

            ImGui::SameLine();

            if (ImGui::Button(skCryptDec("Kapat")))
            {
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }

        ImGui::BeginChild(skCryptDec("RoutePlannerChild"), ImVec2(470, 200), true);
        {
            if (ImGui::BeginTable(skCryptDec("RoutePlanner.Table"), 4, ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_Borders))
            {
                ImGui::TableSetupColumn(skCryptDec("Adim"), ImGuiTableColumnFlags_WidthFixed, 100);
                ImGui::TableSetupColumn(skCryptDec("X"), ImGuiTableColumnFlags_WidthFixed, 100);
                ImGui::TableSetupColumn(skCryptDec("Y"), ImGuiTableColumnFlags_WidthFixed, 100);
                ImGui::TableSetupColumn(skCryptDec("Hareket"), ImGuiTableColumnFlags_WidthFixed, 100);
                ImGui::TableHeadersRow();

                std::vector<Route> vecRoutePlan = m_pClient->m_vecRoutePlan;

                std::reverse(vecRoutePlan.begin(), vecRoutePlan.end());

                for (size_t i = 0; i < vecRoutePlan.size(); i++)
                {
                    ImGui::PushID(i);
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::Text("%d", vecRoutePlan.size() - i);
                    ImGui::TableNextColumn();
                    ImGui::Text("%d", (int)vecRoutePlan[i].fX);
                    ImGui::TableNextColumn();
                    ImGui::Text("%d", (int)vecRoutePlan[i].fY);
                    ImGui::TableNextColumn();

                    switch (vecRoutePlan[i].eStepType)
                    {
                        case STEP_MOVE:
                            ImGui::Text(skCryptDec("Yuru"));
                            break;
                        case STEP_TOWN:
                            ImGui::Text(skCryptDec("Town"));
                            break;
                        case STEP_SUNDRIES:
                            ImGui::Text(skCryptDec("Sundries"));
                            break;
                        case STEP_INN:
                            ImGui::Text(skCryptDec("Inn Hostess"));
                            break;
                        case STEP_GENIE:
                            ImGui::Text(skCryptDec("Genie Baslat"));
                            break;
                        case STEP_POTION:
                            ImGui::Text(skCryptDec("Potcu"));
                            break;
                        case STEP_BOT_START:
                            ImGui::Text(skCryptDec("Bot Baslat"));
                            break;
                        case STEP_OBJECT_EVENT:
                            ImGui::Text(skCryptDec("Obje Etkilesim"));
                            break;
                        case STEP_WARP:
                            ImGui::Text(skCryptDec("Isinlanma"));
                            break;
                        default:
                            ImGui::Text(skCryptDec("Bilinmiyor"));
                            break;
                    }
                   
                    ImGui::PopID();
                }

                ImGui::EndTable();
            }
        }
        ImGui::EndChild();

        ImGui::BulletText(skCryptDec("Bulundugun Noktaya Hareket Ekle"));
        ImGui::Separator();

        ImGui::Spacing();
        {
            if (!m_pClient->m_bIsRoutePlanning)
                ImGui::BeginDisabled();

            ImGui::BeginChild(skCryptDec("RoutePlannerStepChild"), ImVec2(470, 90), true);
            {
                if (ImGui::Button(skCryptDec("Potcu Noktasi"), ImVec2(145.0f, 0.0f)))
                {
                    Route pAutoRoute{};
                    pAutoRoute.fX = m_pClient->GetX();
                    pAutoRoute.fY = m_pClient->GetY();
                    pAutoRoute.eStepType = RouteStepType::STEP_POTION;

                    m_pClient->m_vecRoutePlan.push_back(pAutoRoute);
                }

                ImGui::SameLine();

                if (ImGui::Button(skCryptDec("Sundries Noktasi"), ImVec2(145.0f, 0.0f)))
                {
                    Route pAutoRoute{};
                    pAutoRoute.fX = m_pClient->GetX();
                    pAutoRoute.fY = m_pClient->GetY();
                    pAutoRoute.eStepType = RouteStepType::STEP_SUNDRIES;

                    m_pClient->m_vecRoutePlan.push_back(pAutoRoute);
                }

                ImGui::SameLine();

                if (ImGui::Button(skCryptDec("Inn Hostess Noktasi"), ImVec2(145.0f, 0.0f)))
                {
                    Route pAutoRoute{};
                    pAutoRoute.fX = m_pClient->GetX();
                    pAutoRoute.fY = m_pClient->GetY();
                    pAutoRoute.eStepType = RouteStepType::STEP_INN;

                    m_pClient->m_vecRoutePlan.push_back(pAutoRoute);
                }

                if (ImGui::Button(skCryptDec("Bot Baslama Noktasi"), ImVec2(145.0f, 0.0f)))
                {
                    Route pAutoRoute{};
                    pAutoRoute.fX = m_pClient->GetX();
                    pAutoRoute.fY = m_pClient->GetY();
                    pAutoRoute.eStepType = RouteStepType::STEP_BOT_START;

                    m_pClient->m_vecRoutePlan.push_back(pAutoRoute);
                }

                ImGui::SameLine();

                if (ImGui::Button(skCryptDec("Genie Baslama Noktasi"), ImVec2(145.0f, 0.0f)))
                {
                    Route pAutoRoute{};
                    pAutoRoute.fX = m_pClient->GetX();
                    pAutoRoute.fY = m_pClient->GetY();
                    pAutoRoute.eStepType = RouteStepType::STEP_GENIE;

                    m_pClient->m_vecRoutePlan.push_back(pAutoRoute);
                }
            }
            ImGui::EndChild();

            if (!m_pClient->m_bIsRoutePlanning)
                ImGui::EndDisabled();
        }
    }
}

void Drawing::DrawRouteListController()
{
    ImGui::BulletText(skCryptDec("Rota Ayarlari"));
    ImGui::Separator();

    ImGui::Spacing();
    {
        RouteManager* pRouteManager = Drawing::Bot->GetRouteManager();

        uint8_t iZoneID = m_pClient->GetRepresentZone(m_pClient->GetZone());

        if (ImGui::Checkbox(skCryptDec("##SupplyRouteCheckbox"), &m_pClient->m_bSupplyRouteStatus))
            m_pUserConfiguration->SetInt(skCryptDec("Bot"), skCryptDec("SupplyRouteStatus"), m_pClient->m_bSupplyRouteStatus ? 1 : 0);

        ImGui::SameLine();

        ImGui::Text(skCryptDec("Tedarik"));
        ImGui::SameLine();

        ImGui::SetCursorPosX(87);
        ImGui::SetNextItemWidth(332);

        if (ImGui::BeginCombo(skCryptDec("##PlannedRoute.SupplyRouteList"), m_pClient->m_szSelectedSupplyRoute.c_str()))
        {
            RouteManager::RouteList pRouteList;

            if (pRouteManager
                && pRouteManager->GetRouteList(iZoneID, pRouteList))
            {
                for (auto& e : pRouteList)
                {
                    const bool is_selected = (m_pClient->m_szSelectedSupplyRoute == e.first);

                    if (ImGui::Selectable(e.first.c_str(), is_selected))
                    {
                        m_pClient->m_szSelectedSupplyRoute = m_pUserConfiguration->SetString(skCryptDec("Bot"), skCryptDec("SelectedSupplyRoute"), e.first.c_str());
                    }

                    if (is_selected)
                        ImGui::SetItemDefaultFocus();
                }
            }

            ImGui::EndCombo();
        }

        ImGui::SameLine();

        if (ImGui::Button(m_pClient->m_vecRouteActive.size() == 0 ? skCryptDec("Test") : skCryptDec("Durdur"), ImVec2(53.0f, 0.0f)))
        {
            if (m_pClient->m_vecRouteActive.size() == 0)
            {
                RouteManager::RouteList pRouteList;

                if (pRouteManager
                    && pRouteManager->GetRouteList(iZoneID, pRouteList))
                {
                    auto pRoute = pRouteList.find(m_pClient->m_szSelectedSupplyRoute);

                    if (pRoute != pRouteList.end())
                    {
                        m_pClient->SetRoute(pRoute->second);
                    }
                }
            }
            else
            {
                m_pClient->ClearRoute();
            }
        }

        if (ImGui::Checkbox(skCryptDec("##DeathRouteCheckbox"), &m_pClient->m_bDeathRouteStatus))
            m_pUserConfiguration->SetInt(skCryptDec("Bot"), skCryptDec("DeathRouteStatus"), m_pClient->m_bDeathRouteStatus ? 1 : 0);

        ImGui::SameLine();

        ImGui::Text(skCryptDec("Death"));
        ImGui::SameLine();

        ImGui::SetCursorPosX(87);
        ImGui::SetNextItemWidth(332);

        if (ImGui::BeginCombo(skCryptDec("##PlannedRoute.DeathRouteList"), m_pClient->m_szSelectedDeathRoute.c_str()))
        {
            RouteManager::RouteList pRouteList;

            if (pRouteManager
                && pRouteManager->GetRouteList(iZoneID, pRouteList))
            {
                for (auto& e : pRouteList)
                {
                    const bool is_selected = (m_pClient->m_szSelectedDeathRoute == e.first);

                    if (ImGui::Selectable(e.first.c_str(), is_selected))
                    {
                        m_pClient->m_szSelectedDeathRoute = m_pUserConfiguration->SetString(skCryptDec("Bot"), skCryptDec("SelectedDeathRoute"), e.first.c_str());
                    }

                    if (is_selected)
                        ImGui::SetItemDefaultFocus();
                }
            }

            ImGui::EndCombo();
        }

        ImGui::SameLine();

        if (ImGui::Button(m_pClient->m_vecRouteActive.size() == 0 ? skCryptDec("Test") : skCryptDec("Durdur"), ImVec2(53.0f, 0.0f)))
        {
            if (m_pClient->m_vecRouteActive.size() == 0)
            {
                RouteManager::RouteList pRouteList;

                if (pRouteManager
                    && pRouteManager->GetRouteList(iZoneID, pRouteList))
                {
                    auto pRoute = pRouteList.find(m_pClient->m_szSelectedDeathRoute);

                    if (pRoute != pRouteList.end())
                    {
                        m_pClient->SetRoute(pRoute->second);
                    }
                }
            }
            else
            {
                m_pClient->ClearRoute();
            }
        }

        if (ImGui::Checkbox(skCryptDec("##LoginRouteCheckbox"), &m_pClient->m_bLoginRouteStatus))
            m_pUserConfiguration->SetInt(skCryptDec("Bot"), skCryptDec("LoginRouteStatus"), m_pClient->m_bLoginRouteStatus ? 1 : 0);

        ImGui::SameLine();

        ImGui::Text(skCryptDec("Login"));
        ImGui::SameLine();

        ImGui::SetCursorPosX(87);
        ImGui::SetNextItemWidth(332);

        if (ImGui::BeginCombo(skCryptDec("##PlannedRoute.LoginRouteList"), m_pClient->m_szSelectedLoginRoute.c_str()))
        {
            RouteManager::RouteList pRouteList;

            if (pRouteManager
                && pRouteManager->GetRouteList(iZoneID, pRouteList))
            {
                for (auto& e : pRouteList)
                {
                    const bool is_selected = (m_pClient->m_szSelectedLoginRoute == e.first);

                    if (ImGui::Selectable(e.first.c_str(), is_selected))
                    {
                        m_pClient->m_szSelectedLoginRoute = m_pUserConfiguration->SetString(skCryptDec("Bot"), skCryptDec("SelectedLoginRoute"), e.first.c_str());
                    }

                    if (is_selected)
                        ImGui::SetItemDefaultFocus();
                }
            }

            ImGui::EndCombo();
        }

        ImGui::SameLine();

        if (ImGui::Button(m_pClient->m_vecRouteActive.size() == 0 ? skCryptDec("Test") : skCryptDec("Durdur"), ImVec2(53.0f, 0.0f)))
        {
            if (m_pClient->m_vecRouteActive.size() == 0)
            {
                RouteManager::RouteList pRouteList;

                if (pRouteManager
                    && pRouteManager->GetRouteList(iZoneID, pRouteList))
                {
                    auto pRoute = pRouteList.find(m_pClient->m_szSelectedLoginRoute);

                    if (pRoute != pRouteList.end())
                    {
                        m_pClient->SetRoute(pRoute->second);
                    }
                }
            }
            else
            {
                m_pClient->ClearRoute();
            }
        }
    }
}

void Drawing::DrawMainSettingsArea()
{
    ImGui::BulletText(skCryptDec("Genel Ayarlar"));
    ImGui::Separator();

    ImGui::Spacing();
    {
        if (ImGui::Checkbox(skCryptDec("##WallHack"), &m_pClient->m_bWallHack))
        {
            m_pClient->SetAuthority(m_pClient->m_bWallHack ? 0 : 1);

            m_pClient->m_bWallHack = m_pUserConfiguration->SetInt(skCryptDec("Feature"), skCryptDec("WallHack"), m_pClient->m_bWallHack ? 1 : 0);
        }

        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255.0f, 0.0f, 0.0f, 1.0f));
        ImGui::Text(skCryptDec("Wall Hack (Full)"));
        ImGui::PopStyleColor();

        ImGui::SameLine();

        if (ImGui::Checkbox(skCryptDec("##LegalWallHack"), &m_pClient->m_bLegalWallHack))
        {
            m_pClient->PatchObjectCollision(m_pClient->m_bLegalWallHack);

            m_pClient->m_bLegalWallHack = m_pUserConfiguration->SetInt(skCryptDec("Feature"), skCryptDec("LegalWallHack"), m_pClient->m_bLegalWallHack ? 1 : 0);
        }

        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255.0f, 0.0f, 0.0f, 1.0f));
        ImGui::Text(skCryptDec("Wall Hack (Sadece Objeler)"));
        ImGui::PopStyleColor();

        if (ImGui::Checkbox(skCryptDec("##SpeedHack"), &m_pClient->m_bSpeedHack))
        {
            if (m_pClient->m_bSpeedHack)
            {
                m_pClient->SetCharacterSpeed(1.5);
                m_pClient->PatchSpeedHack(true);
            }
            else
            {
                m_pClient->SetCharacterSpeed(1);
                m_pClient->PatchSpeedHack(false);
            }

            m_pClient->m_bSpeedHack = m_pUserConfiguration->SetInt(skCryptDec("Feature"), skCryptDec("SpeedHack"), m_pClient->m_bSpeedHack ? 1 : 0);
        }

        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255.0f, 0.0f, 0.0f, 1.0f));
        ImGui::Text(skCryptDec("Speed Hack (Swift)"));
        ImGui::PopStyleColor();

        ImGui::SameLine();

        if (ImGui::Checkbox(skCryptDec("##DisableStunCheckbox"), &m_pClient->m_bDisableStun))
            m_pClient->m_bDisableStun = m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("DisableStun"), m_pClient->m_bDisableStun ? 1 : 0);

        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255.0f, 0.0f, 0.0f, 1.0f));
        ImGui::Text(skCryptDec("LR Kaldir"));
        ImGui::PopStyleColor();

        ImGui::SameLine();

        if (ImGui::Checkbox(skCryptDec("##DisableCasting"), &m_pClient->m_bDisableCasting))
        {
            m_pClient->UpdateSkillSuccessRate(m_pClient->m_bDisableCasting);

            m_pUserConfiguration->SetInt(skCryptDec("Feature"), skCryptDec("DisableCasting"), m_pClient->m_bDisableCasting ? 1 : 0);
        }

        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
        ImGui::Text(skCryptDec("El Dusurmeyi Kaldir"));
        ImGui::PopStyleColor();

        if (ImGui::Checkbox(skCryptDec("##DeathEffect"), &m_pClient->m_bDeathEffect))
        {
            m_pClient->PatchDeathEffect(m_pClient->m_bDeathEffect);
            m_pUserConfiguration->SetInt(skCryptDec("Feature"), skCryptDec("DeathEffect"), m_pClient->m_bDeathEffect ? 1 : 0);
        }

        ImGui::SameLine();

        ImGui::Text(skCryptDec("Death Animasyonu Kaldir"));

        ImGui::SameLine();

        if (ImGui::Checkbox(skCryptDec("##HidePlayer"), &m_pClient->m_bHidePlayer))
        {
            m_pClient->HidePlayer(m_pClient->m_bHidePlayer);
            m_pUserConfiguration->SetInt(skCryptDec("Feature"), skCryptDec("HidePlayer"), m_pClient->m_bHidePlayer ? 1 : 0);
        }

        ImGui::SameLine();

        ImGui::Text(skCryptDec("Oyunculari Gizle"));
    }
}

void Drawing::DrawExtraController()
{
    ImGui::BulletText(skCryptDec("Magic Bag"));
    ImGui::Separator();

    ImGui::Spacing();
    {
        ImGui::Text("Sol banka, 1. ve 2. slota +1 Dagger koy ve Magic Bag Tak dugmesine bas");

        if (ImGui::Button(skCryptDec("Magic Bag Tak"), ImVec2(150.0f, 0.0f)))
        {
            std::vector<TNpc> tmpVecNpc = m_pClient->m_vecNpc;

            auto findedNpc = std::find_if(tmpVecNpc.begin(), tmpVecNpc.end(),
                [&](const TNpc& a) { return a.iProtoID == 16096; });

            if (findedNpc != tmpVecNpc.end())
            {
                m_pClient->SendWarehouseGetOut(findedNpc->iID, 110110001, 0, 0, 54, 1);
                m_pClient->SendWarehouseGetOut(findedNpc->iID, 110110001, 0, 1, 68, 1);
            }
        }
    }
}

void Drawing::DrawLevelDownerController()
{
    ImGui::BulletText(skCryptDec("Level Dusurme"));
    ImGui::Separator();

    ImGui::Spacing();
    {
        if (ImGui::Checkbox(skCryptDec("##LevelDownerEnableCheckBox"), &m_pClient->m_bLevelDownerEnable))
            m_pClient->m_bLevelDownerEnable = m_pUserConfiguration->SetInt(skCryptDec("LevelDowner"), skCryptDec("Enable"), m_pClient->m_bLevelDownerEnable ? 1 : 0);

        ImGui::SameLine();

        ImGui::Text(skCryptDec("Npc ID"));

        ImGui::SameLine();

        ImGui::PushItemWidth(100);
        if (ImGui::DragInt(skCryptDec("##LevelDownerNpcId"), &m_pClient->m_iLevelDownerNpcId, 1, -1, 100000))
        {
            m_pClient->m_iLevelDownerNpcId = m_pUserConfiguration->SetInt(skCryptDec("LevelDowner"), skCryptDec("NpcId"), m_pClient->m_iLevelDownerNpcId);
        }
        ImGui::PopItemWidth();

        ImGui::SameLine();

        if (ImGui::Button(skCryptDec("Npc ID Ekle"), ImVec2(150.0f, 0.0f)))
        {
            if (m_pClient->GetTarget() != -1)
            {
                m_pClient->m_iLevelDownerNpcId = m_pUserConfiguration->SetInt(skCryptDec("LevelDowner"), skCryptDec("NpcId"), m_pClient->GetTarget());
            }
        }

        if (ImGui::Checkbox(skCryptDec("##LevelDownerLevelLimitCheckBox"), &m_pClient->m_bLevelDownerLevelLimitEnable))
            m_pClient->m_bLevelDownerLevelLimitEnable = m_pUserConfiguration->SetInt(skCryptDec("LevelDowner"), skCryptDec("LevelLimitEnable"), m_pClient->m_bLevelDownerLevelLimitEnable ? 1 : 0);

        ImGui::SameLine();

        ImGui::Text(skCryptDec("Level Limit"));

        ImGui::SameLine();

        ImGui::PushItemWidth(100);

        if (ImGui::DragInt(skCryptDec("##LevelDownerLevelLimit"), &m_pClient->m_iLevelDownerLevelLimit, 1, 1, 83))
        {
            m_pClient->m_iLevelDownerLevelLimit = m_pUserConfiguration->SetInt(skCryptDec("LevelDowner"), skCryptDec("LevelLimit"), m_pClient->m_iLevelDownerLevelLimit);
        }

        ImGui::PopItemWidth();

        if (ImGui::Checkbox(skCryptDec("##LevelDownerStopIfNearbyPlayerCheckBox"), &m_pClient->m_bLevelDownerStopNearbyPlayer))
            m_pClient->m_bLevelDownerStopNearbyPlayer = m_pUserConfiguration->SetInt(skCryptDec("LevelDowner"), skCryptDec("StopIfNearbyPlayer"), m_pClient->m_bLevelDownerStopNearbyPlayer ? 1 : 0);

        ImGui::SameLine();

        ImGui::Text(skCryptDec("Yakinlarda Oyuncu Varsa Durdur"));
    }
}
