#include "pch.h"
#include "Drawing.h"
#include "ClientHandler.h"

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
                vWindowSize = { 600, 600 };

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

                    ImGui::BeginChild(skCryptDec("##Main.Child1"), ImVec2(258, 560), true);
                    {
                        DrawMainController();
                        ImGui::Separator();
                        DrawModeController();
                    }
                    ImGui::EndChild();

                    ImGui::SameLine();

                    ImGui::BeginChild(skCryptDec("##Main.Child2"), ImVec2(318, 560), true);
                    {
                        if (ImGui::BeginTabBar(skCryptDec("##Main.TabBar"), ImGuiTabBarFlags_None))
                        {
                            if (ImGui::BeginTabItem(skCryptDec("Genel")))
                            {
                                DrawProtectionController();
                                ImGui::Separator();

                                DrawTransformationController();
                                ImGui::Separator();

                                DrawAutoLootController();

                                ImGui::Separator();
                                DrawPartyController();

                                ImGui::Separator();
                                DrawFlashController();

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

                            if (ImGui::BeginTabItem(skCryptDec("Tedarik")))
                            {
                                DrawSupplyController();

                                ImGui::EndTabItem();
                            }

                            if (ImGui::BeginTabItem(skCryptDec("Sistem")))
                            {
                                DrawSaveCPUController();
                                ImGui::Separator();
                                DrawListenerController();
                                ImGui::Separator();
                                DrawSettingsController();

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

void Drawing::DrawMainController()
{
    ImGui::BulletText(skCryptDec("Ana Kontroller"));
    ImGui::Separator();

    ImGui::Spacing();
    {
        bool bAttackStatus = m_pUserConfiguration->GetBool(skCryptDec("Automation"), skCryptDec("Attack"), false);

        if (bAttackStatus)
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.255f, 0.0f, 1.0f));
        else
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.255f, 0.0f, 0.0f, 1.0f));

        if (ImGui::Button(bAttackStatus ? skCryptDec("Saldiri Durdur") : skCryptDec("Saldiri Baslat"), ImVec2(117.0f, 0.0f)))
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

        if (ImGui::Button(bCharacterStatus ? skCryptDec("Bot Durdur") : skCryptDec("Bot Baslat"), ImVec2(117.0f, 0.0f)))
        {
            bCharacterStatus = !bCharacterStatus;
            m_pUserConfiguration->SetInt(skCryptDec("Automation"), skCryptDec("Character"), bCharacterStatus);
        }

        ImGui::PopStyleColor(1);

        if (ImGui::Button(skCryptDec("Town At"), ImVec2(117.0f, 0.0f)))
        {
            m_pClient->SendTownPacket();
        }

        ImGui::SameLine();

        if (ImGui::Button(skCryptDec("Oyunu Kapat"), ImVec2(117.0f, 0.0f)))
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

        vecDisguiseItems.push_back(DisguiseItem(381001000, "TS Scroll (60 Dakika)"));
        vecDisguiseItems.push_back(DisguiseItem(-1, "TS Scroll (90 Dakika)"));
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

        if (iTransformationItem == -1)
        {
            iTransformationItem = 379090000;
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

void Drawing::DrawSpeedController()
{
    ImGui::BulletText(skCryptDec("Hiz Ayarlari"));
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

    }
}

void Drawing::DrawDistanceController()
{
    ImGui::BulletText(skCryptDec("Mesafe Ayarlari"));
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

void Drawing::DrawProtectionController()
{
    ImGui::BulletText(skCryptDec("Koruma Ayarlari"));
    ImGui::Separator();

    ImGui::Spacing();
    {
        bool bHpProtection = m_pUserConfiguration->GetBool(skCryptDec("Protection"), skCryptDec("Hp"), false);

        if (ImGui::Checkbox(skCryptDec("##HpPotionCheckbox"), &bHpProtection))
            m_pUserConfiguration->SetInt(skCryptDec("Protection"), skCryptDec("Hp"), bHpProtection ? 1 : 0);

        ImGui::SameLine();

        ImGui::Text(skCryptDec("Hp Potion (%%)"));

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

        ImGui::Text(skCryptDec("Mp Potion (%%)"));

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

        ImGui::Text(skCryptDec("Minor (%%)"));

        ImGui::SameLine();

        ImGui::PushItemWidth(50);

        int iMinorProtectionValue = m_pUserConfiguration->GetInt(skCryptDec("Protection"), skCryptDec("MinorValue"), 30);

        if (ImGui::DragInt(skCryptDec("##MinorValue"), &iMinorProtectionValue, 1, 0, 100))
            m_pUserConfiguration->SetInt(skCryptDec("Protection"), skCryptDec("MinorValue"), iMinorProtectionValue);

        ImGui::PopItemWidth();

        if (!m_pClient->IsRogue())
            ImGui::EndDisabled();


        if (!m_pClient->IsPriest())
            ImGui::BeginDisabled();

        bool bHealProtection = m_pUserConfiguration->GetBool(skCryptDec("Protection"), skCryptDec("Heal"), false);

        if (ImGui::Checkbox(skCryptDec("##HealCheckbox"), &bHealProtection))
            m_pUserConfiguration->SetInt(skCryptDec("Protection"), skCryptDec("Heal"), bHealProtection ? 1 : 0);

        ImGui::SameLine();

        ImGui::Text(skCryptDec("Heal (%%)"));

        ImGui::SameLine();

        ImGui::PushItemWidth(50);

        int iHealProtectionValue = m_pUserConfiguration->GetInt(skCryptDec("Protection"), skCryptDec("HealValue"), 75);

        if (ImGui::DragInt(skCryptDec("##HealValue"), &iHealProtectionValue, 1, 0, 100))
            m_pUserConfiguration->SetInt(skCryptDec("Protection"), skCryptDec("HealValue"), iHealProtectionValue);

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
        bool bAutoLoot = m_pUserConfiguration->GetBool(skCryptDec("AutoLoot"), skCryptDec("Enable"), false);

        if (ImGui::Checkbox(skCryptDec("##AutoLoot"), &bAutoLoot))
        {
            if (!bAutoLoot)
            {
                m_pClient->m_vecLootList.clear();
            }

            m_pUserConfiguration->SetInt(skCryptDec("AutoLoot"), skCryptDec("Enable"), bAutoLoot ? 1 : 0);
        }

        ImGui::SameLine();

        ImGui::Text(skCryptDec("Kutu Topla"));

        ImGui::SameLine();

        bool bMoveToLoot = m_pUserConfiguration->GetBool(skCryptDec("AutoLoot"), skCryptDec("MoveToLoot"), false);

        if (ImGui::Checkbox(skCryptDec("##MoveToLoot"), &bMoveToLoot))
        {
            if (!bMoveToLoot)
            {
                m_pClient->SetMovingToLoot(false);
            }

            m_pUserConfiguration->SetInt(skCryptDec("AutoLoot"), skCryptDec("MoveToLoot"), bMoveToLoot ? 1 : 0);
        }

        ImGui::SameLine();

        ImGui::Text(skCryptDec("Kutuya Kos"));

        bool bTodo = true;
        if (ImGui::Checkbox(skCryptDec("##MinPriceEnable"), &bTodo))
        {
            //TODO: Implement here
        }

        ImGui::SameLine();

        ImGui::Text(skCryptDec("Satis Fiyatina Gore Topla"));

        ImGui::SameLine();

        ImGui::PushItemWidth(80);

        int iLootMinPrice = m_pUserConfiguration->GetInt(skCryptDec("AutoLoot"), skCryptDec("MinPrice"), 0);

        if (ImGui::DragInt(skCryptDec("##LootMinPrice"), &iLootMinPrice, 1, 0, INT_MAX))
            m_pUserConfiguration->SetInt(skCryptDec("AutoLoot"), skCryptDec("MinPrice"), iLootMinPrice);

        ImGui::PopItemWidth();
    }
}

void Drawing::DrawAttackController()
{
    ImGui::BulletText(skCryptDec("Saldiri Ayarlari"));
    ImGui::Separator();

    ImGui::Spacing();
    {
        bool bBasicAttack = m_pUserConfiguration->GetBool(skCryptDec("Attack"), skCryptDec("BasicAttack"), false);

        if (ImGui::Checkbox(skCryptDec("##BasicAttackCheckbox"), &bBasicAttack))
            m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("BasicAttack"), bBasicAttack ? 1 : 0);

        ImGui::SameLine();

        ImGui::Text(skCryptDec("R Vur"));

        if (!bBasicAttack)
            ImGui::BeginDisabled();

        bool bBasicAttackWithPacket = m_pUserConfiguration->GetBool(skCryptDec("Attack"), skCryptDec("BasicAttackWithPacket"), false);

        if (ImGui::Checkbox(skCryptDec("##BasicAttackWithPacketCheckbox"), &bBasicAttackWithPacket))
            m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("BasicAttackWithPacket"), bBasicAttackWithPacket ? 1 : 0);

        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255.0f, 0.0f, 0.0f, 1.0f));
        ImGui::Text(skCryptDec("R Illegal Vur"));
        ImGui::PopStyleColor();

        if (!bBasicAttack)
            ImGui::EndDisabled();

        bool bDisableStun = m_pUserConfiguration->GetBool(skCryptDec("Attack"), skCryptDec("DisableStun"), false);

        if (ImGui::Checkbox(skCryptDec("##DisableStunCheckbox"), &bDisableStun))
            m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("DisableStun"), bDisableStun ? 1 : 0);

        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255.0f, 0.0f, 0.0f, 1.0f));
        ImGui::Text(skCryptDec("Stun Kaldir"));
        ImGui::PopStyleColor();
    }
}

void Drawing::DrawTargetListController()
{
    ImGui::BulletText(skCryptDec("Hedef Ayarlari"));
    ImGui::Separator();

    bool bAutoTarget = m_pUserConfiguration->GetInt(skCryptDec("Attack"), skCryptDec("AutoTarget"), false);

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

        ImGui::Text(skCryptDec("Death Animasyonu Kaldir"));

        bool bClosestTarget = m_pUserConfiguration->GetInt(skCryptDec("Attack"), skCryptDec("ClosestTarget"), false);

        if (ImGui::Checkbox(skCryptDec("##ClosestTargetCheckbox"), &bClosestTarget))
            m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("ClosestTarget"), bClosestTarget ? 1 : 0);

        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255.0f, 0.0f, 0.0f, 1.0f));
        ImGui::Text(skCryptDec("Herzaman En Yakin Hedefe Vur"));
        ImGui::PopStyleColor();

        if (bAutoTarget)
            ImGui::BeginDisabled();

        if (ImGui::Button(skCryptDec("Listeye Mob ID Ekle"), ImVec2(300.0f, 0.0f)))
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

        if (bAutoTarget)
            ImGui::EndDisabled();

        ImGui::BeginChild(skCryptDec("TargetListController"), ImVec2(300, 296), true);
        {
            bool bRangeLimit = m_pUserConfiguration->GetBool(skCryptDec("Attack"), skCryptDec("RangeLimit"), false);
            int iRangeLimitValue = m_pUserConfiguration->GetInt(skCryptDec("Attack"), skCryptDec("RangeLimitValue"), 100);

            if (bAutoTarget)
                ImGui::BeginDisabled();

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



            if (bAutoTarget)
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
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
        ImGui::Text(skCryptDec("Sadece Saldiri Skillerini Illegal Vur"));
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
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
        ImGui::Text(skCryptDec("El Dusurmeyi Kaldir"));
        ImGui::PopStyleColor();

        if (ImGui::Button(skCryptDec("Skill Listesini Sifirla"), ImVec2(300.0f, 0.0f)))
        {
            m_pUserConfiguration->SetInt(skCryptDec("Automation"), skCryptDec("AttackSkillList"), std::vector<int>());
            m_pUserConfiguration->SetInt(skCryptDec("Automation"), skCryptDec("CharacterSkillList"), std::vector<int>());

            m_pClient->LoadSkillData();
        }

        ImGui::BeginChild(skCryptDec("SkillController"), ImVec2(300, 366), true);
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

void Drawing::DrawSizeController()
{
    ImGui::BulletText(skCryptDec("Boyut Ayarlari"));
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

void Drawing::DrawSaveCPUController()
{
    ImGui::BulletText(skCryptDec("Save CPU Ayarlari"));
    ImGui::Separator();

    ImGui::Spacing();
    {
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

void Drawing::DrawSupplyController()
{
    ImGui::BulletText(skCryptDec("Tedarik Ayarlari"));
    ImGui::Separator();

    ImGui::Spacing();
    {
        bool bAutoRepairMagicHammer = m_pUserConfiguration->GetBool(skCryptDec("Supply"), skCryptDec("AutoRepairMagicHammer"), false);

        if (ImGui::Checkbox(skCryptDec("##AutoRepairMagicHammer"), &bAutoRepairMagicHammer))
        {
            m_pUserConfiguration->SetInt(skCryptDec("Supply"), skCryptDec("AutoRepairMagicHammer"), bAutoRepairMagicHammer ? 1 : 0);
        }

        ImGui::SameLine();

        ImGui::Text(skCryptDec("Oto RPR (Magic Hammer)"));
    }
}

void Drawing::DrawListenerController()
{
    ImGui::BulletText(skCryptDec("Dinleyici Ayarlari"));
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

void Drawing::DrawPartyController()
{
    ImGui::BulletText(skCryptDec("Party Ayarlari"));
    ImGui::Separator();

    ImGui::Spacing();
    {
        if (!m_pClient->IsRogue())
            ImGui::BeginDisabled();

        bool bPartySwift = m_pUserConfiguration->GetBool(skCryptDec("Rogue"), skCryptDec("PartySwift"), false);

        if (ImGui::Checkbox(skCryptDec("##RoguePartySwift"), &bPartySwift))
            m_pUserConfiguration->SetInt(skCryptDec("Rogue"), skCryptDec("PartySwift"), bPartySwift ? 1 : 0);

        ImGui::SameLine();

        ImGui::Text(skCryptDec("Party Swift"));

        if (!m_pClient->IsRogue())
            ImGui::EndDisabled();

        ImGui::SameLine();

        if (!m_pClient->IsPriest())
            ImGui::BeginDisabled();

        bool bPriestPartyHeal = m_pUserConfiguration->GetBool(skCryptDec("Priest"), skCryptDec("PartyHeal"), false);

        if (ImGui::Checkbox(skCryptDec("##PriestPartyHeal"), &bPriestPartyHeal))
            m_pUserConfiguration->SetInt(skCryptDec("Priest"), skCryptDec("PartyHeal"), bPriestPartyHeal ? 1 : 0);

        ImGui::SameLine();

        ImGui::Text(skCryptDec("Party Heal"));

        bool bPriestPartyBuff = m_pUserConfiguration->GetBool(skCryptDec("Priest"), skCryptDec("PartyBuff"), false);

        if (ImGui::Checkbox(skCryptDec("##PriestPartyBuff"), &bPriestPartyBuff))
            m_pUserConfiguration->SetInt(skCryptDec("Priest"), skCryptDec("PartyBuff"), bPriestPartyBuff ? 1 : 0);

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

        if (ImGui::Button(skCryptDec("Tum Ayarlari Sifirla"), ImVec2(282.0f, 0.0f)))
        {
            m_pUserConfiguration->Reset();
        }
    }
}

void Drawing::SetLegalModeSettings(bool bMode)
{
    m_pUserConfiguration->SetInt(skCryptDec("Automation"), skCryptDec("LegalMode"), bMode);

    if (bMode)
    {
        m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("BasicAttackWithPacket"), 0);
        m_pUserConfiguration->SetInt(skCryptDec("Skill"), skCryptDec("UseSkillWithPacket"), 0);
        m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("ClosestTarget"), 0);

        m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("AttackRangeLimit"), 1);
        m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("AttackRangeLimitValue"), 9);
    }
    else
    {
        m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("BasicAttackWithPacket"), 1);
        m_pUserConfiguration->SetInt(skCryptDec("Skill"), skCryptDec("UseSkillWithPacket"), 1);
        m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("ClosestTarget"), 1);

        m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("AttackRangeLimit"), 1);
        m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("AttackRangeLimitValue"), 50);

    }
}

void Drawing::SetSpeedModeSettings(bool bMode)
{
    m_pUserConfiguration->SetInt(skCryptDec("Automation"), skCryptDec("SpeedMode"), bMode);

    if (bMode)
    {
        m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("SearchTargetSpeed"), 1);
        m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("SearchTargetSpeedValue"), 0);

        m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("AttackSpeed"), 1);
        m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("AttackSpeedValue"), 800);

        m_pUserConfiguration->SetInt(skCryptDec("Feature"), skCryptDec("DeathEffect"), 1);
    }
    else
    {
        m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("SearchTargetSpeed"), 1);
        m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("SearchTargetSpeedValue"), 1000);

        m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("AttackSpeed"), 1);
        m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("AttackSpeedValue"), 1000);

        m_pUserConfiguration->SetInt(skCryptDec("Feature"), skCryptDec("DeathEffect"), 0);
    }
}

void Drawing::DrawModeController()
{
    ImGui::BulletText(skCryptDec("Bot Modu Kontrolleri"));
    ImGui::Separator();

    ImGui::Spacing();
    {
        bool bLegalMode = m_pUserConfiguration->GetBool(skCryptDec("Automation"), skCryptDec("LegalMode"), true);

        if (!bLegalMode)
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.255f, 0.0f, 1.0f));
        else
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.255f, 0.0f, 0.0f, 1.0f));

        if (ImGui::Button("Illegal Mod", ImVec2(117.0f, 0.0f)))
        {
            bLegalMode = false;
            SetLegalModeSettings(bLegalMode);
        }

        ImGui::PopStyleColor(1);

        ImGui::SameLine();

        if (bLegalMode)
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.255f, 0.0f, 1.0f));
        else
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.255f, 0.0f, 0.0f, 1.0f));

        if (ImGui::Button(skCryptDec("Legal Mod"), ImVec2(117.0f, 0.0f)))
        {
            bLegalMode = true;
            SetLegalModeSettings(bLegalMode);
        }

        ImGui::PopStyleColor(1);

        bool bSpeedMode = m_pUserConfiguration->GetBool(skCryptDec("Automation"), skCryptDec("SpeedMode"), true);

        if (bSpeedMode)
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.255f, 0.0f, 1.0f));
        else
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.255f, 0.0f, 0.0f, 1.0f));

        if (ImGui::Button(skCryptDec("Hizli Mod"), ImVec2(117.0f, 0.0f)))
        {
            bSpeedMode = true;
            SetSpeedModeSettings(bSpeedMode);
        }

        ImGui::PopStyleColor(1);
        ImGui::SameLine();

        if (!bSpeedMode)
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.255f, 0.0f, 1.0f));
        else
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.255f, 0.0f, 0.0f, 1.0f));

        if (ImGui::Button("Yavas Mod", ImVec2(117.0f, 0.0f)))
        {
            bSpeedMode = false;
            SetSpeedModeSettings(bSpeedMode);
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