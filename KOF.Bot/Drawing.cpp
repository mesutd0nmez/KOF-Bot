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
        if (m_pClient->m_bAttackStatus)
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.255f, 0.0f, 1.0f));
        else
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.255f, 0.0f, 0.0f, 1.0f));

        if (ImGui::Button(m_pClient->m_bAttackStatus ? skCryptDec("Saldiri Durdur") : skCryptDec("Saldiri Baslat"), ImVec2(117.0f, 0.0f)))
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

        if (ImGui::Button(m_pClient->m_bCharacterStatus ? skCryptDec("Bot Durdur") : skCryptDec("Bot Baslat"), ImVec2(117.0f, 0.0f)))
        {
            m_pClient->m_bCharacterStatus = !m_pClient->m_bCharacterStatus;
            m_pUserConfiguration->SetInt(skCryptDec("Automation"), skCryptDec("Character"), m_pClient->m_bCharacterStatus);
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

        if (ImGui::Checkbox(skCryptDec("##MpPotionCheckbox"), &m_pClient->m_bMpProtectionEnable))
            m_pUserConfiguration->SetInt(skCryptDec("Protection"), skCryptDec("Mp"), m_pClient->m_bMpProtectionEnable ? 1 : 0);

        ImGui::SameLine();

        ImGui::Text(skCryptDec("Mp Potion (%%)"));

        ImGui::SameLine();

        ImGui::PushItemWidth(50);

        if (ImGui::DragInt(skCryptDec("##MpPotionValue"), &m_pClient->m_iMpProtectionValue, 1, 0, 100))
            m_pUserConfiguration->SetInt(skCryptDec("Protection"), skCryptDec("MpValue"), m_pClient->m_iMpProtectionValue);

        ImGui::PopItemWidth();

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

            m_pUserConfiguration->SetInt(skCryptDec("AutoLoot"), skCryptDec("MoveToLoot"), m_pClient->m_bMoveToLoot ? 1 : 0);
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

        if (ImGui::DragInt(skCryptDec("##LootMinPrice"), &m_pClient->m_iLootMinPrice, 1, 0, INT_MAX))
            m_pUserConfiguration->SetInt(skCryptDec("AutoLoot"), skCryptDec("MinPrice"), m_pClient->m_iLootMinPrice);

        ImGui::PopItemWidth();
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

        if (ImGui::Checkbox(skCryptDec("##BasicAttackWithPacketCheckbox"), &m_pClient->m_bBasicAttackWithPacket))
            m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("BasicAttackWithPacket"), m_pClient->m_bBasicAttackWithPacket ? 1 : 0);

        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255.0f, 0.0f, 0.0f, 1.0f));
        ImGui::Text(skCryptDec("R Illegal Vur"));
        ImGui::PopStyleColor();

        if (!m_pClient->m_bBasicAttack)
            ImGui::EndDisabled();

        if (ImGui::Checkbox(skCryptDec("##DisableStunCheckbox"), &m_pClient->m_bDisableStun))
            m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("DisableStun"), m_pClient->m_bDisableStun ? 1 : 0);

        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255.0f, 0.0f, 0.0f, 1.0f));
        ImGui::Text(skCryptDec("LR Kaldir"));
        ImGui::PopStyleColor();
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

        if (ImGui::Checkbox(skCryptDec("##DeathEffect"), &m_pClient->m_bDeathEffect))
        {
            m_pClient->PatchDeathEffect(m_pClient->m_bDeathEffect);
            m_pUserConfiguration->SetInt(skCryptDec("Feature"), skCryptDec("DeathEffect"), m_pClient->m_bDeathEffect ? 1 : 0);
        }

        ImGui::SameLine();

        ImGui::Text(skCryptDec("Death Animasyonu Kaldir"));

        if (ImGui::Checkbox(skCryptDec("##ClosestTargetCheckbox"), &m_pClient->m_bClosestTarget))
            m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("ClosestTarget"), m_pClient->m_bClosestTarget ? 1 : 0);

        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255.0f, 0.0f, 0.0f, 1.0f));
        ImGui::Text(skCryptDec("Herzaman En Yakin Hedefe Vur"));
        ImGui::PopStyleColor();

        if (m_pClient->m_bAutoTarget)
            ImGui::BeginDisabled();

        if (ImGui::Button(skCryptDec("Listeye Mob ID Ekle"), ImVec2(300.0f, 0.0f)))
        {
            int32_t iTargetID = m_pClient->GetTarget();

            if (iTargetID != -1)
            {
                const auto pFindedID = std::find_if(m_pClient->m_vecSelectedNpcIDList.begin(), m_pClient->m_vecSelectedNpcIDList.end(),
                    [iTargetID](const auto& a) { return a == iTargetID; });

                if (pFindedID == m_pClient->m_vecSelectedNpcIDList.end())
                {
                    m_pClient->m_vecSelectedNpcIDList.push_back(iTargetID);
                    m_pClient->m_vecSelectedNpcIDList = m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("NpcIDList"), m_pClient->m_vecSelectedNpcIDList);
                }
            }
        }

        if (m_pClient->m_bAutoTarget)
            ImGui::EndDisabled();

        ImGui::BeginChild(skCryptDec("TargetListController"), ImVec2(300, 296), true);
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
                            m_pClient->m_vecSelectedNpcList.push_back(x.iProtoID);
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

        if (ImGui::Checkbox(skCryptDec("##DisableCasting"), &m_pClient->m_bDisableCasting))
        {
            m_pClient->UpdateSkillSuccessRate(m_pClient->m_bDisableCasting);

            m_pUserConfiguration->SetInt(skCryptDec("Feature"), skCryptDec("DisableCasting"), m_pClient->m_bDisableCasting ? 1 : 0);
        }

        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
        ImGui::Text(skCryptDec("El Dusurmeyi Kaldir"));
        ImGui::PopStyleColor();

        if (ImGui::Button(skCryptDec("Skill Listesini Sifirla"), ImVec2(300.0f, 0.0f)))
        {
            m_pClient->m_vecAttackSkillList = m_pUserConfiguration->SetInt(skCryptDec("Automation"), skCryptDec("AttackSkillList"), std::vector<int>());
            m_pClient->m_vecCharacterSkillList = m_pUserConfiguration->SetInt(skCryptDec("Automation"), skCryptDec("CharacterSkillList"), std::vector<int>());

            m_pClient->LoadSkillData();
        }

        ImGui::BeginChild(skCryptDec("SkillController"), ImVec2(300, 366), true);
        {
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

                        bool bSelected = std::find(m_pClient->m_vecAttackSkillList.begin(), m_pClient->m_vecAttackSkillList.end(), x.iID) != m_pClient->m_vecAttackSkillList.end();

                        if (ImGui::Selectable(x.szName.c_str(), &bSelected))
                        {
                            if (bSelected)
                                m_pClient->m_vecAttackSkillList.push_back(x.iID);
                            else
                                m_pClient->m_vecAttackSkillList.erase(std::find(m_pClient->m_vecAttackSkillList.begin(), m_pClient->m_vecAttackSkillList.end(), x.iID));

                            m_pClient->m_vecAttackSkillList = m_pUserConfiguration->SetInt(skCryptDec("Automation"), skCryptDec("AttackSkillList"), m_pClient->m_vecAttackSkillList);
                        }

                        ImGui::PopID();
                    }
                }

                ImGui::TreePop();
            }

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

                        bool bSelected = std::find(m_pClient->m_vecCharacterSkillList.begin(), m_pClient->m_vecCharacterSkillList.end(), x.iID) != m_pClient->m_vecCharacterSkillList.end();

                        if (ImGui::Selectable(x.szName.c_str(), &bSelected))
                        {
                            if (bSelected)
                                m_pClient->m_vecCharacterSkillList.push_back(x.iID);
                            else
                                m_pClient->m_vecCharacterSkillList.erase(std::find(m_pClient->m_vecCharacterSkillList.begin(), m_pClient->m_vecCharacterSkillList.end(), x.iID));

                            m_pClient->m_vecCharacterSkillList = m_pUserConfiguration->SetInt(skCryptDec("Automation"), skCryptDec("CharacterSkillList"), m_pClient->m_vecCharacterSkillList);
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
        if (ImGui::Checkbox(skCryptDec("##TargetSizeCheckbox"), &m_pClient->m_bTargetSizeEnable))
            m_pUserConfiguration->SetInt(skCryptDec("Target"), skCryptDec("SizeEnable"), m_pClient->m_bTargetSizeEnable ? 1 : 0);

        ImGui::SameLine();

        ImGui::Text(skCryptDec("Hedef Boyutu"));

        ImGui::SameLine();

        ImGui::PushItemWidth(139);

        if (ImGui::SliderInt(skCryptDec("##TargetSize"), &m_pClient->m_iTargetSize, 1, 10))
            m_pUserConfiguration->SetInt(skCryptDec("Target"), skCryptDec("Size"), m_pClient->m_iTargetSize);

        ImGui::PopItemWidth();

        if (ImGui::Checkbox(skCryptDec("##CharacterSizeCheckbox"), &m_pClient->m_bCharacterSizeEnable))
            m_pUserConfiguration->SetInt(skCryptDec("Character"), skCryptDec("SizeEnable"), m_pClient->m_bCharacterSizeEnable ? 1 : 0);

        ImGui::SameLine();

        ImGui::Text(skCryptDec("Karakter Boyutu"));

        ImGui::SameLine();

        ImGui::PushItemWidth(125);

        if (ImGui::SliderInt(skCryptDec("##CharacterSize"), &m_pClient->m_iCharacterSize, 1, 10))
            m_pUserConfiguration->SetInt(skCryptDec("Character"), skCryptDec("Size"), m_pClient->m_iCharacterSize);

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
        if (ImGui::Checkbox(skCryptDec("##AutoRepairMagicHammer"), &m_pClient->m_bAutoRepairMagicHammer))
        {
            m_pUserConfiguration->SetInt(skCryptDec("Supply"), skCryptDec("AutoRepairMagicHammer"), m_pClient->m_bAutoRepairMagicHammer ? 1 : 0);
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
        if (ImGui::Checkbox(skCryptDec("##PartyRequest"), &m_pClient->m_bPartyRequest))
            m_pUserConfiguration->SetInt(skCryptDec("Listener"), skCryptDec("PartyRequest"), m_pClient->m_bPartyRequest ? 1 : 0);

        ImGui::SameLine();

        ImGui::Text(skCryptDec("Oto Party"));

        ImGui::SameLine();

        ImGui::PushItemWidth(169);

        if (ImGui::InputText(skCryptDec("##PartyRequestMessage"), &m_pClient->m_szPartyRequestMessage[0], 100))
        {
            m_pUserConfiguration->SetString(skCryptDec("Listener"), skCryptDec("PartyRequestMessage"), &m_pClient->m_szPartyRequestMessage[0]);
        }

        ImGui::PopItemWidth();

        if (!m_pClient->IsMage())
            ImGui::BeginDisabled();

        if (ImGui::Checkbox(skCryptDec("##TeleportRequest"), &m_pClient->m_bTeleportRequest))
            m_pUserConfiguration->SetInt(skCryptDec("Listener"), skCryptDec("TeleportRequest"), m_pClient->m_bTeleportRequest ? 1 : 0);

        ImGui::SameLine();

        ImGui::Text(skCryptDec("Oto TP"));

        ImGui::SameLine();

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

        if (ImGui::Button(skCryptDec("Tum Ayarlari Sifirla"), ImVec2(282.0f, 0.0f)))
        {
            m_pUserConfiguration->Reset();
            m_pClient->ClearUserConfiguration();
        }
    }
}

void Drawing::SetLegalModeSettings(bool bMode)
{
    m_pClient->m_bLegalMode = m_pUserConfiguration->SetInt(skCryptDec("Automation"), skCryptDec("LegalMode"), bMode);

    if (bMode)
    {
        m_pClient->m_bBasicAttackWithPacket = m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("BasicAttackWithPacket"), 0);
        m_pClient->m_bUseSkillWithPacket = m_pUserConfiguration->SetInt(skCryptDec("Skill"), skCryptDec("UseSkillWithPacket"), 0);
        m_pClient->m_bClosestTarget = m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("ClosestTarget"), 0);

        m_pClient->m_bAttackRangeLimit = m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("AttackRangeLimit"), 1);
        m_pClient->m_iAttackRangeLimitValue = m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("AttackRangeLimitValue"), 9);
    }
    else
    {
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
        m_pClient->m_iAttackSpeedValue = m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("AttackSpeedValue"), 800);

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

        if (ImGui::Button("Illegal Mod", ImVec2(117.0f, 0.0f)))
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

        if (ImGui::Button(skCryptDec("Legal Mod"), ImVec2(117.0f, 0.0f)))
        {
            m_pClient->m_bLegalMode = true;
            SetLegalModeSettings(m_pClient->m_bLegalMode);
        }

        ImGui::PopStyleColor(1);

        if (m_pClient->m_bSpeedMode)
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.255f, 0.0f, 1.0f));
        else
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.255f, 0.0f, 0.0f, 1.0f));

        if (ImGui::Button(skCryptDec("Hizli Mod"), ImVec2(117.0f, 0.0f)))
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

        if (ImGui::Button("Yavas Mod", ImVec2(117.0f, 0.0f)))
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

        ImGui::PushItemWidth(100);

        if (ImGui::DragInt(skCryptDec("##AutoDCFlashCount"), &m_pClient->m_iAutoDCFlashCount, 10, 1, 10))
            m_pUserConfiguration->SetInt(skCryptDec("Settings"), skCryptDec("AutoDCFlashCount"), m_pClient->m_iAutoDCFlashCount);

        ImGui::PopItemWidth();

        if (ImGui::Checkbox(skCryptDec("##AutoWarFlash"), &m_pClient->m_bAutoWarFlash))
            m_pUserConfiguration->SetInt(skCryptDec("Settings"), skCryptDec("AutoWarFlash"), m_pClient->m_bAutoWarFlash ? 1 : 0);

        ImGui::SameLine();

        ImGui::Text(skCryptDec("War Flash"));

        ImGui::SameLine();

        ImGui::PushItemWidth(100);

        if (ImGui::DragInt(skCryptDec("##AutoWarFlashCount"), &m_pClient->m_iAutoWarFlashCount, 10, 1, 10))
            m_pUserConfiguration->SetInt(skCryptDec("Settings"), skCryptDec("AutoWarFlashCount"), m_pClient->m_iAutoWarFlashCount);

        ImGui::PopItemWidth();

        if (ImGui::Checkbox(skCryptDec("##AutoExpFlash"), &m_pClient->m_bAutoExpFlash))
            m_pUserConfiguration->SetInt(skCryptDec("Settings"), skCryptDec("AutoExpFlash"), m_pClient->m_bAutoExpFlash ? 1 : 0);

        ImGui::SameLine();

        ImGui::Text(skCryptDec("Exp Flash"));

        ImGui::SameLine();

        ImGui::PushItemWidth(100);

        if (ImGui::DragInt(skCryptDec("##AutoExpFlashCount"), &m_pClient->m_iAutoExpFlashCount, 10, 1, 10))
            m_pUserConfiguration->SetInt(skCryptDec("Settings"), skCryptDec("AutoExpFlashCount"), m_pClient->m_iAutoExpFlashCount);

        ImGui::PopItemWidth();
    }
}