#include "pch.h"
#include "Drawing.h"
#include "Bot.h"
#include "Client.h"
#include "ClientHandler.h"
#include "UI.h"
#include "Guard.h"

Bot* Drawing::Bot = nullptr;
LPCSTR Drawing::lpWindowName = skCryptEnc("KOF.Bot");
ImVec2 Drawing::vWindowSize = { 658, 600 };
ImGuiWindowFlags Drawing::WindowFlags = ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize;
bool Drawing::bDraw = true;
bool Drawing::Done = false;

float fScreenWidth = (GetSystemMetrics(SM_CXSCREEN)) / 2.0f;
float fScreenHeight = (GetSystemMetrics(SM_CYSCREEN)) / 2.0f;

ImVec2 vec2InitialPos = { fScreenWidth, fScreenHeight };

ClientHandler* m_pClient = nullptr;
Ini* m_pConfiguration = nullptr;

std::map<uint32_t, __TABLE_NPC> m_mapNpcTable;
std::map<uint32_t, __TABLE_MOB_USKO> m_mapMobTable;

std::vector<__TABLE_UPC_SKILL> m_vecAvailableSkill;

WorldData* m_pWorldData = nullptr;

ID3D11ShaderResourceView* m_pMinimapTexture = nullptr;

void Drawing::Active()
{
	bDraw = true;
}

bool Drawing::isActive()
{
	return bDraw == true;
}

void Drawing::Draw()
{
    m_pConfiguration = Drawing::Bot->GetConfiguration();
    m_pClient = Drawing::Bot->GetClientHandler();

    if (m_mapNpcTable.size() == 0 && Drawing::Bot->IsTableLoaded())
    {
        m_mapNpcTable = Drawing::Bot->GetNpcTable()->GetData();
    }

    if (m_mapMobTable.size() == 0 && Drawing::Bot->IsTableLoaded())
    {
        m_mapMobTable = Drawing::Bot->GetMobTable()->GetData();
    }

    if (m_vecAvailableSkill.size() == 0 && Drawing::Bot->IsTableLoaded())
    {
        m_vecAvailableSkill = m_pClient->GetAvailableSkill();
    }

    if (m_pWorldData == nullptr || (m_pWorldData != nullptr && m_pWorldData->iId != m_pClient->GetZone()))
    {
        m_pWorldData = m_pClient->GetWorld()->GetWorldData(m_pClient->GetZone());

        if(m_pWorldData != nullptr)
            UI::LoadTextureFromMemory(m_pWorldData->pMiniMapImageData, &m_pMinimapTexture, m_pWorldData->iMiniMapImageWidth, m_pWorldData->iMiniMapImageHeight);
    }

	if (isActive())
	{
		ImGui::SetNextWindowPos(vec2InitialPos, ImGuiCond_Once);
		ImGui::SetNextWindowSize(vWindowSize);
		ImGui::SetNextWindowBgAlpha(1.0f);
		ImGui::Begin(Drawing::lpWindowName, &bDraw, WindowFlags);
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
    ImGui::BeginChild(1, ImVec2(283, 563), true);
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
                    auto pNpcList = m_pClient->GetNpcList();

                    if (pNpcList.size() > 0)
                    {
                        for (const TNpc& pNpc : pNpcList)
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

                    //ImVec2 pPlayerPosition = ImVec2(
                    //    pOffsetPosition.x + std::ceil(pPlayer.fX / (float)(m_pWorldData->fMapLength / m_pWorldData->iMiniMapImageWidth)),
                    //    pOffsetPosition.y + std::ceil(m_pWorldData->iMiniMapImageHeight - (pPlayer.fY / (float)(m_pWorldData->fMapLength / m_pWorldData->iMiniMapImageHeight))));

                    //ImGui::GetWindowDrawList()->AddCircle(pPlayerPosition, 1.0f, IM_COL32(0, 191, 255, 255), 0, 3.0f);

                    //TEST

                    Guard playerListLock(m_pClient->m_vecPlayerLock);
                    auto pPlayerList = m_pClient->GetPlayerList();

                    if (pPlayerList.size() > 0)
                    {
                        for (const TPlayer& pPlayer : pPlayerList)
                        {
                            if (m_pClient->GetDistance(pPlayer.fX, pPlayer.fY) > MAX_VIEW_RANGE)
                                continue;

                            ImVec2 pPlayerPosition = ImVec2(
                                pOffsetPosition.x + std::ceil(pPlayer.fX / (float)(m_pWorldData->fMapLength / m_pWorldData->iMiniMapImageWidth)),
                                pOffsetPosition.y + std::ceil(m_pWorldData->iMiniMapImageHeight - (pPlayer.fY / (float)(m_pWorldData->fMapLength / m_pWorldData->iMiniMapImageHeight))));

                            ImGui::GetWindowDrawList()->AddCircle(pPlayerPosition, 1.0f, IM_COL32(0, 191, 255, 255), 0, 3.0f);
                        }
                    }

                    ImVec2 currentPosition = ImVec2(
                        pOffsetPosition.x + std::ceil(m_pClient->GetPosition().m_fX / (float)(m_pWorldData->fMapLength / m_pWorldData->iMiniMapImageWidth)),
                        pOffsetPosition.y + std::ceil(m_pWorldData->iMiniMapImageHeight - (m_pClient->GetPosition().m_fY / (float)(m_pWorldData->fMapLength / m_pWorldData->iMiniMapImageHeight))));

                    ImGui::GetWindowDrawList()->AddCircle(currentPosition, 1.0f, IM_COL32(0, 255, 0, 255), 0, 3.0f);

                    if (m_pClient->GetGoX() > 0.0f && m_pClient->GetGoY() > 0.0f)
                    {
                        ImVec2 movePosition = ImVec2(
                            pOffsetPosition.x + std::ceil(m_pClient->GetGoX() / (float)(m_pWorldData->fMapLength / m_pWorldData->iMiniMapImageWidth)),
                            pOffsetPosition.y + std::ceil(m_pWorldData->iMiniMapImageHeight - (m_pClient->GetGoY() / (float)(m_pWorldData->fMapLength / m_pWorldData->iMiniMapImageHeight))));

                        ImGui::GetWindowDrawList()->AddLine(currentPosition, movePosition, IM_COL32(0, 255, 0, 255), 3.0f);
                    }

                    ImGui::EndChild();
                }
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
            auto fHeight = 60.0f;
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

            ImGui::SameLine();

            bool bProtectionStatus = m_pConfiguration->GetBool(skCryptDec("Automation"), skCryptDec("Protection"), false);

            if (bProtectionStatus)
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.255f, 0.0f, 1.0f));
            else
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.255f, 0.0f, 0.0f, 1.0f));

            if (ImGui::Button(skCryptDec("Protection"), ImVec2(129.0f, 0.0f)))
            {
                bProtectionStatus = !bProtectionStatus;
                m_pConfiguration->SetInt(skCryptDec("Automation"), skCryptDec("Protection"), bProtectionStatus);
            }

            ImGui::PopStyleColor(1);
        }
    }

    ImGui::EndChild();

    ImGui::SameLine();

    ImGui::BeginChild(2, ImVec2(350, 563), true);
    {
        if (ImGui::BeginTabBar(skCryptDec("##KOF.Toolbar"), ImGuiTabBarFlags_None))
        {
            if (ImGui::BeginTabItem(skCryptDec("Main")))
            {
                DrawMainProtectionArea();
                DrawMainFeaturesArea();
                DrawMainAutoLootArea();

                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem(skCryptDec("Skill")))
            {
                if (m_vecAvailableSkill.size() == 0)
                    ImGui::BeginDisabled();

                DrawAutomatedAttackSkillTree();
                DrawAutomatedCharacterSkillTree();

                if (m_vecAvailableSkill.size() == 0)
                    ImGui::EndDisabled();

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

                    auto iItem = m_pClient->GetInventoryItem(110110001); // +1 Dagger 

                    if (iItem)
                    {
                        m_pClient->SendItemMovePacket(1, ITEM_INVEN_INVEN, iItem->iItemID, iItem->iPos - 14, 35);
                        m_pClient->SendShoppingMall(ShoppingMallType::STORE_CLOSE);
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

        for (const auto& x : m_vecAvailableSkill)
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
        for (const auto& x : m_vecAvailableSkill)
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
        std::vector<SNpcData> vecNpcList;

        std::vector<int> vecSelectedNpcList = m_pConfiguration->GetInt(skCryptDec("Attack"), skCryptDec("NpcList"), std::vector<int>());

        Guard lock(m_pClient->m_vecNpcLock);
        auto mapNpcList = m_pClient->GetNpcList();

        if (mapNpcList.size() > 0)
        {
            for (const auto& x : vecSelectedNpcList)
            {
                SNpcData pNpcData;

                pNpcData.iProtoID = x;
                pNpcData.fDistance = 0.0f;

                const auto pFindedNpc = std::find_if(mapNpcList.begin(), mapNpcList.end(),
                    [x](const TNpc& a) { return a.iProtoID == x; });

                if (pFindedNpc != mapNpcList.end())
                    pNpcData.fDistance = m_pClient->GetDistance(pFindedNpc->fX, pFindedNpc->fY);

                vecNpcList.push_back(pNpcData);
            }

            for (const auto& x : mapNpcList)
            {
                const auto pFindedNpc = std::find_if(vecNpcList.begin(), vecNpcList.end(),
                    [x](const SNpcData& a) { return a.iProtoID == x.iProtoID; });

                if ((x.iMonsterOrNpc == 1
                    || (x.iProtoID >= 19067 && x.iProtoID <= 19069)
                    || (x.iProtoID >= 19070 && x.iProtoID <= 19072))
                    && x.iProtoID != 9009
                    && m_pClient->GetDistance(x.fX, x.fY) <= MAX_VIEW_RANGE
                    && pFindedNpc == vecNpcList.end())
                {
                    SNpcData pNpcData;

                    pNpcData.iProtoID = x.iProtoID;
                    pNpcData.fDistance = m_pClient->GetDistance(x.fX, x.fY);

                    vecNpcList.push_back(pNpcData);
                }
            }

            for (const auto& x : vecNpcList)
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

                auto pNpcInfo = m_mapNpcTable.find(x.iProtoID);
                auto pMobInfo = m_mapMobTable.find(x.iProtoID);

                std::string szNpcName = skCryptDec("~Unknown~");

                if (pNpcInfo != m_mapNpcTable.end())
                    szNpcName = pNpcInfo->second.szText;

                if (pMobInfo != m_mapMobTable.end())
                    szNpcName = pMobInfo->second.szText;

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