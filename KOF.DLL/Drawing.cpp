#include "pch.h"
#include "Drawing.h"
#include "Memory.h"
#include "Ini.h"
#include "Client.h"
#include "Bootstrap.h"
#include "UI.h"

LPCSTR Drawing::lpWindowName = APP_TITLE;
ImVec2 Drawing::vWindowSize = { 658, 600 };
ImGuiWindowFlags Drawing::WindowFlags = ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize;
bool Drawing::m_bDraw = true;
ImVec2 initial_pos(1600, 0);
Ini* m_UserConfig = NULL;


void Drawing::Draw()
{
    std::this_thread::sleep_for(std::chrono::milliseconds(1));

    m_UserConfig = Client::GetUserConfig(Client::GetName());

    bool bRender = m_UserConfig;

	if (IsDrawable() && bRender)
	{
		ImGui::SetNextWindowPos(initial_pos, ImGuiCond_Once);
		ImGui::SetNextWindowSize(vWindowSize);
		ImGui::SetNextWindowBgAlpha(1.0f);
		ImGui::Begin(lpWindowName, &m_bDraw, WindowFlags);
		{
			DrawGameController();
		}

		ImGui::End();
	}

#ifdef _WINDLL
	if (GetAsyncKeyState(VK_INSERT) & 1)
		m_bDraw = !m_bDraw;
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
        ImGui::TextUnformatted(Client::GetName().c_str());
        ImGui::SameLine();
        ImGui::NextColumn();
        RightText("Lv " + std::to_string(Client::GetLevel()));
        ImGui::Separator();

        ImGui::Spacing();
        {
            ImGui::PushItemWidth(265);
            {
                CenteredText(std::to_string(Client::GetHp()) + " / " + std::to_string(Client::GetMaxHp()));

                float fHpProgress = (((float)Client::GetHp() / (float)Client::GetMaxHp()) * 100.f) / 100.0f;

                ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(255.0f, 0.0f, 0.0f, 1.0f));
                ImGui::ProgressBar(fHpProgress, ImVec2(0.0f, 0.0f));
                ImGui::PopStyleColor(1);

                CenteredText(std::to_string(Client::GetMp()) + " / " + std::to_string(Client::GetMaxMp()));

                float fMpProgress = (((float)Client::GetMp() / (float)Client::GetMaxMp()) * 100.f) / 100.0f;

                ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.0f, 0.0f, 255.0f, 1.0f));
                ImGui::ProgressBar(fMpProgress, ImVec2(0.0f, 0.0f));
                ImGui::PopStyleColor(1);
            }

            ImGui::PopItemWidth();
        }

        ImGui::Spacing();
        {
            int iMinimapWidth = 0;
            int iMinimapHeight = 0;
            ID3D11ShaderResourceView* pMinimapTexture = NULL;
            bool ret = UI::LoadTextureFromFile("C:\\Users\\Administrator\\Documents\\GitHub\\koef\\KOF.UI\\data\\image\\moradon_xmas.jpg", &pMinimapTexture, &iMinimapWidth, &iMinimapHeight);
            IM_ASSERT(ret);

            ImGui::BeginChild("Minimap", ImVec2((float)(iMinimapWidth + 17.0f), (float)iMinimapHeight + 17.0f), true);
            {
                ImVec2 pOffsetPosition = ImGui::GetCursorScreenPos();

                ImGui::Image((void*)pMinimapTexture, ImVec2((float)iMinimapWidth, (float)iMinimapHeight));

                if (ImGui::IsItemHovered() && ImGui::IsMouseDown(ImGuiMouseButton_Left))
                {
                    ImVec2 mousePositionAbsolute = ImGui::GetMousePos();
                    ImVec2 screenPositionAbsolute = ImGui::GetItemRectMin();
                    ImVec2 mousePositionRelative = ImVec2(mousePositionAbsolute.x - screenPositionAbsolute.x, mousePositionAbsolute.y - screenPositionAbsolute.y);

                    Client::SetMovePosition(
                        Vector3(
                            std::ceil(mousePositionRelative.x * (float)(1024 / iMinimapWidth)), 0.0f, 
                            std::ceil((iMinimapHeight - mousePositionRelative.y) * (float)(1024 / iMinimapHeight))
                        )
                    );
                }

                auto pNpcList = Client::GetNpcList();

                if (pNpcList.size() > 0)
                {
                    for (const TNpc& pNpc : pNpcList)
                    {
                        if (Client::GetDistance(pNpc.fX, pNpc.fY) > MAX_VIEW_RANGE)
                            continue;

                        ImVec2 pNpcPosition = ImVec2(
                            pOffsetPosition.x + std::ceil(pNpc.fX / (float)(1024 / iMinimapWidth)),
                            pOffsetPosition.y + std::ceil(iMinimapHeight - (pNpc.fY / (float)(1024 / iMinimapHeight))));

                        if (pNpc.iMonsterOrNpc == 1)
                            ImGui::GetWindowDrawList()->AddCircle(pNpcPosition, 1.0f, IM_COL32(255, 0, 0, 255), 0, 3.0f);
                        else
                            ImGui::GetWindowDrawList()->AddCircle(pNpcPosition, 1.0f, IM_COL32(0, 0, 255, 255), 0, 3.0f);
                    }
                }

                auto pPlayerList = Client::GetPlayerList();

                if (pPlayerList.size() > 0)
                {
                    for (const TPlayer& pPlayer : pPlayerList)
                    {
                        if (Client::GetDistance(pPlayer.fX, pPlayer.fY) > MAX_VIEW_RANGE)
                            continue;

                        ImVec2 pPlayerPosition = ImVec2(
                            pOffsetPosition.x + std::ceil(pPlayer.fX / (float)(1024 / iMinimapWidth)),
                            pOffsetPosition.y + std::ceil(iMinimapHeight - (pPlayer.fY / (float)(1024 / iMinimapHeight))));

                        ImGui::GetWindowDrawList()->AddCircle(pPlayerPosition, 1.0f, IM_COL32(0, 191, 255, 255), 0, 3.0f);
                    }
                }

                ImVec2 currentPosition = ImVec2(
                    pOffsetPosition.x + std::ceil(Client::GetPosition().m_fX / (float)(1024 / iMinimapWidth)),
                    pOffsetPosition.y + std::ceil(iMinimapHeight - (Client::GetPosition().m_fY / (float)(1024 / iMinimapHeight))));

                ImGui::GetWindowDrawList()->AddCircle(currentPosition, 1.0f, IM_COL32(0, 255, 0, 255), 0, 3.0f);

                if (Client::GetGoX() > 0.0f && Client::GetGoY() > 0.0f)
                {
                    ImVec2 movePosition = ImVec2(
                        pOffsetPosition.x + std::ceil(Client::GetGoX() / (float)(1024 / iMinimapWidth)),
                        pOffsetPosition.y + std::ceil(iMinimapHeight - (Client::GetGoY() / (float)(1024 / iMinimapHeight))));

                    ImGui::GetWindowDrawList()->AddLine(currentPosition, movePosition, IM_COL32(0, 255, 0, 255), 3.0f);
                }

                ImGui::EndChild();
            }
        }

        ImGui::Spacing();
        {
            ImGui::TextUnformatted("Quick Action");
            ImGui::Separator();

            bool bLegalStatus = m_UserConfig->GetBool("Bot", "Legal", false);

            if (bLegalStatus)
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.255f, 0.0f, 1.0f));
            else
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.255f, 0.0f, 0.0f, 1.0f));

            if (ImGui::Button("Legal", ImVec2(129.0f, 0.0f)))
            {
                bLegalStatus = !bLegalStatus;
                m_UserConfig->SetInt("Bot", "Legal", bLegalStatus);
            }

            ImGui::PopStyleColor(1);

            ImGui::SameLine();

            if (ImGui::Button("Town", ImVec2(129.0f, 0.0f)))
            {
                Client::SendTownPacket();
            }
        }

        ImGui::Spacing();
        {
            auto windowHeight = ImGui::GetWindowSize().y;
            auto fHeight = 60.0f;
            ImGui::SetCursorPosY((windowHeight - fHeight) * 0.97f);

            ImGui::TextUnformatted("Automation");
            ImGui::Separator();

            bool bAttackStatus = m_UserConfig->GetBool("Automation", "Attack", false);

            if (bAttackStatus)
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.255f, 0.0f, 1.0f));
            else
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.255f, 0.0f, 0.0f, 1.0f));

            if (ImGui::Button("Attack", ImVec2(129.0f, 0.0f)))
            {
                bAttackStatus = !bAttackStatus;
                m_UserConfig->SetInt("Automation", "Attack", bAttackStatus);
            }

            ImGui::PopStyleColor(1);

            bool bCharacterStatus = m_UserConfig->GetBool("Automation", "Character", false);

            if (bCharacterStatus)
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.255f, 0.0f, 1.0f));
            else
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.255f, 0.0f, 0.0f, 1.0f));

            if (ImGui::Button("Character", ImVec2(129.0f, 0.0f)))
            {
                bCharacterStatus = !bCharacterStatus;
                m_UserConfig->SetInt("Automation", "Character", bCharacterStatus);
            }

            ImGui::PopStyleColor(1);

            ImGui::SameLine();

            bool bProtectionStatus = m_UserConfig->GetBool("Automation", "Protection", false);

            if (bProtectionStatus)
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.255f, 0.0f, 1.0f));
            else
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.255f, 0.0f, 0.0f, 1.0f));

            if (ImGui::Button("Protection", ImVec2(129.0f, 0.0f)))
            {
                bProtectionStatus = !bProtectionStatus;
                m_UserConfig->SetInt("Automation", "Protection", bProtectionStatus);
            }

            ImGui::PopStyleColor(1);
        }
    }

    ImGui::EndChild();

    ImGui::SameLine();

    ImGui::BeginChild(2, ImVec2(350, 563), true);
    {
        if (ImGui::BeginTabBar("##KOF.Toolbar", ImGuiTabBarFlags_None))
        {
            if (ImGui::BeginTabItem("Main"))
            {
                DrawMainProtectionArea();
                DrawMainFeaturesArea();
                DrawMainAutoLootArea();

                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Skill"))
            {
                if (Client::GetAvailableSkill().size() == 0)
                    ImGui::BeginDisabled();

                DrawAutomatedAttackSkillTree();
                DrawAutomatedCharacterSkillTree();

                if (Client::GetAvailableSkill().size() == 0)
                    ImGui::EndDisabled();

                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Attack"))
            {
                ImGui::TextUnformatted("Configure Automated Attack");
                ImGui::Separator();

                ImGui::Spacing();
                {
                    bool bAutoTarget = m_UserConfig->GetInt("Attack", "AutoTarget", true);

                    if (ImGui::Checkbox("##AutoTargetCheckbox", &bAutoTarget))
                        m_UserConfig->SetInt("Attack", "AutoTarget", bAutoTarget ? 1 : 0);

                    ImGui::SameLine();

                    ImGui::Text("Auto Target");

                    ImGui::SameLine();

                    bool bRangeLimit = m_UserConfig->GetBool("Attack", "RangeLimit", false);

                    if (ImGui::Checkbox("##RangeLimitCheckbox", &bRangeLimit))
                        m_UserConfig->SetInt("Attack", "RangeLimit", bRangeLimit ? 1 : 0);

                    ImGui::SameLine();

                    ImGui::Text("Range Limit");

                    ImGui::SameLine();

                    ImGui::PushItemWidth(50);

                    int iRangeLimitValue = m_UserConfig->GetInt("Attack", "RangeLimitValue", (int)MAX_ATTACK_RANGE);

                    if (ImGui::DragInt("##RangeLimitValue", &iRangeLimitValue, 1, 0, 100))
                        m_UserConfig->SetInt("Attack", "RangeLimitValue", iRangeLimitValue);

                    ImGui::PopItemWidth();
                }

                ImGui::Spacing();
                {
                    bool bAttackSpeed = m_UserConfig->GetBool("Attack", "AttackSpeed", false);

                    if (ImGui::Checkbox("##AttackSpeedCheckbox", &bAttackSpeed))
                        m_UserConfig->SetInt("Attack", "AttackSpeed", bAttackSpeed ? 1 : 0);

                    ImGui::SameLine();

                    ImGui::Text("Attack Speed");

                    ImGui::SameLine();

                    ImGui::PushItemWidth(75);

                    int iAttackSpeedValue = m_UserConfig->GetInt("Attack", "AttackSpeedValue", 1000);

                    if (ImGui::DragInt("##AttackSpeedValue", &iAttackSpeedValue, 1, 0, 65535))
                        m_UserConfig->SetInt("Attack", "AttackSpeedValue", iAttackSpeedValue);

                    ImGui::PopItemWidth();
                }

                ImGui::Spacing();
                {
                    bool bBasicAttack = m_UserConfig->GetBool("Attack", "BasicAttack", true);

                    if (ImGui::Checkbox("##BasicAttackCheckbox", &bBasicAttack))
                        m_UserConfig->SetInt("Attack", "BasicAttack", bBasicAttack ? 1 : 0);

                    ImGui::SameLine();

                    ImGui::Text("Basic Attack (R)");

                    ImGui::SameLine();

                    bool bMoveToTarget = m_UserConfig->GetBool("Attack", "MoveToTarget", false);

                    if (ImGui::Checkbox("##MoveToTargetCheckbox", &bMoveToTarget))
                        m_UserConfig->SetInt("Attack", "MoveToTarget", bMoveToTarget ? 1 : 0);

                    ImGui::SameLine();

                    ImGui::Text("Move To Target");
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
    ImGui::TextUnformatted("Protection");
    ImGui::Separator();

    ImGui::Spacing();
    {
        bool bHpProtection = m_UserConfig->GetBool("Protection", "Hp", false);

        if (ImGui::Checkbox("##HpPotionCheckbox", &bHpProtection))
            m_UserConfig->SetInt("Protection", "Hp", bHpProtection ? 1 : 0);

        ImGui::SameLine();

        ImGui::Text("Hp Potion");

        ImGui::SameLine();

        ImGui::PushItemWidth(50);

        int iHpProtectionValue = m_UserConfig->GetInt("Protection", "HpValue", 50);

        if (ImGui::DragInt("##HpPotionValue", &iHpProtectionValue, 1, 0, 100))
            m_UserConfig->SetInt("Protection", "HpValue", iHpProtectionValue);

        ImGui::PopItemWidth();

    }

    ImGui::Spacing();
    {
        bool bMpProtection = m_UserConfig->GetBool("Protection", "Mp", false);

        if (ImGui::Checkbox("##MpPotionCheckbox", &bMpProtection))
            m_UserConfig->SetInt("Protection", "Mp", bMpProtection ? 1 : 0);

        ImGui::SameLine();

        ImGui::Text("Mp Potion");

        ImGui::SameLine();

        ImGui::PushItemWidth(50);

        int iMpProtectionValue = m_UserConfig->GetInt("Protection", "MpValue", 25);

        if (ImGui::DragInt("##MpPotionValue", &iMpProtectionValue, 1, 0, 100))
            m_UserConfig->SetInt("Protection", "MpValue", iMpProtectionValue);

        ImGui::PopItemWidth();
    }

    ImGui::Spacing();
    {
        bool bMinorProtection = m_UserConfig->GetBool("Protection", "Minor", false);

        if (ImGui::Checkbox("##MinorCheckbox", &bMinorProtection))
            m_UserConfig->SetInt("Protection", "Minor", bMinorProtection ? 1 : 0);

        ImGui::SameLine();

        ImGui::Text("Minor");

        ImGui::SameLine();

        ImGui::PushItemWidth(50);

        int iMinorProtectionValue = m_UserConfig->GetInt("Protection", "MinorValue", 30);

        if (ImGui::DragInt("##MinorValue", &iMinorProtectionValue, 1, 0, 100))
            m_UserConfig->SetInt("Protection", "MinorValue", iMinorProtectionValue);

        ImGui::PopItemWidth();
    }

}

void Drawing::DrawMainFeaturesArea()
{
    ImGui::Spacing();
    {
        ImGui::TextUnformatted("Features");
        ImGui::Separator();

        ImGui::Spacing();
        {
            bool bGodMode = m_UserConfig->GetBool("Protection", "GodMode", false);

            if (ImGui::Checkbox("##GodMode", &bGodMode))
                m_UserConfig->SetInt("Protection", "GodMode", bGodMode ? 1 : 0);

            ImGui::SameLine();

            ImGui::Text("God Mode");

            ImGui::SameLine();

            bool bWallHack = m_UserConfig->GetBool("Feature", "WallHack", false);

            if (ImGui::Checkbox("##WallHack", &bWallHack))
            {
                Client::SetAuthority(bWallHack ? 0 : 1);

                m_UserConfig->SetInt("Feature", "WallHack", bWallHack ? 1 : 0);
            }

            ImGui::SameLine();

            ImGui::Text("Wall Hack");

            ImGui::SameLine();

            bool bHyperNoah = m_UserConfig->GetBool("Feature", "HyperNoah", false);

            if (ImGui::Checkbox("##HyperNoah", &bHyperNoah))
                m_UserConfig->SetInt("Feature", "HyperNoah", bHyperNoah ? 1 : 0);

            ImGui::SameLine();

            ImGui::Text("Hyper Noah");
        }

        ImGui::Spacing();
        {
            bool bOreads = m_UserConfig->GetBool("Feature", "Oreads", false);

            if (ImGui::Checkbox("##Oreads", &bOreads))
            {
                Client::SetOreads(bOreads);

                if (bOreads)
                {
                    Client::EquipOreads(700039000);

                    //TODO: Need For Class

                    auto iItem = Client::GetInventoryItem(110110001); // +1 Dagger 

                    if (iItem)
                    {
                        Client::SendItemMovePacket(1, ITEM_INVEN_INVEN, iItem->iItemID, iItem->iPos - 14, 35);
                        Client::SendShoppingMall(ShoppingMallType::STORE_CLOSE);
                    }
                }

                m_UserConfig->SetInt("Feature", "Oreads", bOreads ? 1 : 0);
            }
               

            ImGui::SameLine();

            ImGui::Text("Oreads");
        }
    }
}

void Drawing::DrawMainAutoLootArea()
{
    ImGui::Spacing();
    {
        ImGui::TextUnformatted("Auto Loot");
        ImGui::Separator();

        ImGui::Spacing();
        {
            bool bAutoLoot = m_UserConfig->GetBool("AutoLoot", "Enable", false);

            if (ImGui::Checkbox("##AutoLoot", &bAutoLoot))
                m_UserConfig->SetInt("AutoLoot", "Enable", bAutoLoot ? 1 : 0);

            ImGui::SameLine();

            ImGui::Text("Enable");

            ImGui::SameLine();

            bool bMoveToLoot = m_UserConfig->GetBool("AutoLoot", "MoveToLoot", false);

            if (ImGui::Checkbox("##MoveToLoot", &bMoveToLoot))
            {
                Client::SetAuthority(bMoveToLoot ? 0 : 1);
                m_UserConfig->SetInt("AutoLoot", "MoveToLoot", bMoveToLoot ? 1 : 0);
            }

            ImGui::SameLine();

            ImGui::Text("Move To Loot");
        }
    }

    ImGui::Spacing();
    {
        ImGui::Text("Loot Min Price");

        ImGui::SameLine();

        ImGui::PushItemWidth(100);

        int iLootMinPrice = m_UserConfig->GetInt("AutoLoot", "MinPrice", 0);

        if (ImGui::DragInt("##LootMinPrice", &iLootMinPrice, 1, 0, INT_MAX))
            m_UserConfig->SetInt("AutoLoot", "MinPrice", iLootMinPrice);

        ImGui::PopItemWidth();
    }
}

void Drawing::DrawAutomatedAttackSkillTree()
{
    ImGui::TextUnformatted("Select automated attack or character skill");
    ImGui::Separator();

    std::vector<int> vecAttackList = m_UserConfig->GetInt("Automation", "AttackSkillList", std::vector<int>());

    std::stringstream strTreeText;

    if (Bootstrap::GetSkillTable().GetDataSize() == 0)
        strTreeText << "|/-\\"[(int)(ImGui::GetTime() / 0.05f) & 3] << " ";

    strTreeText << "Automated attack skills";

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Bullet;
    if (ImGui::TreeNodeEx(strTreeText.str().c_str(), flags))
    {
        auto vecAvailableSkill = Client::GetAvailableSkill();

        for (const auto& x : vecAvailableSkill)
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

                m_UserConfig->SetInt("Automation", "AttackSkillList", vecAttackList);
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
    std::vector<int> vecCharacterSkillList = m_UserConfig->GetInt("Automation", "CharacterSkillList", std::vector<int>());

    std::stringstream strTreeText;

    if (Bootstrap::GetSkillTable().GetDataSize() == 0)
        strTreeText << "|/-\\"[(int)(ImGui::GetTime() / 0.05f) & 3] << " ";

    strTreeText << "Automated character skills";

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Bullet;
    if (ImGui::TreeNodeEx(strTreeText.str().c_str(), flags))
    {
        auto vecAvailableSkill = Client::GetAvailableSkill();

        for (const auto& x : vecAvailableSkill)
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

                m_UserConfig->SetInt("Automation", "CharacterSkillList", vecCharacterSkillList);
            }

            if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                ImGui::SetTooltip(x.szDesc.c_str());

            ImGui::PopID();
        }

        ImGui::TreePop();
    }
}

struct SNpcData
{
    uint16_t iProtoID;
    float fDistance;
};

void Drawing::DrawMonsterListTree()
{
    bool bRangeLimit = m_UserConfig->GetBool("Attack", "RangeLimit", false);
    int iRangeLimitValue = m_UserConfig->GetInt("Attack", "RangeLimitValue", (int)MAX_ATTACK_RANGE);
    bool bAutoTarget = m_UserConfig->GetInt("Attack", "AutoTarget", true);

    ImGui::TextUnformatted("Select attackable target");
    ImGui::Separator();

    if(bAutoTarget)
        ImGui::BeginDisabled();

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Bullet;
    if (ImGui::TreeNodeEx("Target Monster List", flags))
    {
        std::vector<SNpcData> vecNpcList;
     
        std::vector<int> vecSelectedNpcList = m_UserConfig->GetInt("Attack", "NpcList", std::vector<int>());

        auto mapNpcList = Client::GetNpcList();

        for (const auto& x : vecSelectedNpcList)
        {
            SNpcData pNpcData;

            pNpcData.iProtoID = x;
            pNpcData.fDistance = 0.0f;

            const auto pFindedNpc = std::find_if(mapNpcList.begin(), mapNpcList.end(),
                [x](const TNpc& a) { return a.iProtoID == x; });

            if(pFindedNpc != mapNpcList.end())
                pNpcData.fDistance = Client::GetDistance(pFindedNpc->fX, pFindedNpc->fY);

            vecNpcList.push_back(pNpcData);
        } 

        auto pSort = [](TNpc const& a, TNpc const& b)
        {
            return Client::GetDistance(a.fX, a.fY) < Client::GetDistance(b.fX, b.fY);
        };

        std::sort(mapNpcList.begin(), mapNpcList.end(), pSort);

        for (const auto& x : mapNpcList)
        {
            const auto pFindedNpc = std::find_if(vecNpcList.begin(), vecNpcList.end(),
                [x](const SNpcData& a) { return a.iProtoID == x.iProtoID; });

            if ((x.iMonsterOrNpc == 1
                || (x.iProtoID >= 19067 && x.iProtoID <= 19069)
                || (x.iProtoID >= 19070 && x.iProtoID <= 19072))
                && x.iProtoID != 9009
                && Client::GetDistance(x.fX, x.fY) <= MAX_VIEW_RANGE
                && pFindedNpc == vecNpcList.end())
            {
                SNpcData pNpcData;

                pNpcData.iProtoID = x.iProtoID;
                pNpcData.fDistance = Client::GetDistance(x.fX, x.fY);

                vecNpcList.push_back(pNpcData);
            }
        }

        auto pNpcData = Bootstrap::GetNpcTable().GetData();
        auto pMobData = Bootstrap::GetMobTable().GetData();

        for (const auto& x : vecNpcList)
        {
            bool bIsAttackable = false;

            if (bRangeLimit)
            {
                if(x.fDistance != 0.0f && x.fDistance <= (float)MAX_ATTACK_RANGE)
                    bIsAttackable = true;
            } 
            else
                bIsAttackable = (x.fDistance != 0.0f && x.fDistance <= (float)MAX_ATTACK_RANGE);

            if(bIsAttackable)
                ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
            else
                ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 0, 0, 255));

            ImGui::PushID(x.iProtoID);

            bool bSelected = std::find(vecSelectedNpcList.begin(), vecSelectedNpcList.end(), x.iProtoID) != vecSelectedNpcList.end();

            auto pNpcInfo = pNpcData.find(x.iProtoID);
            auto pMobInfo = pMobData.find(x.iProtoID);

            std::string szNpcName = "~Unknown~";

            if (pNpcInfo != pNpcData.end())
                szNpcName = pNpcInfo->second.szText;

            if (pMobInfo != pMobData.end())
                szNpcName = pMobInfo->second.szText;

            if (ImGui::Selectable(szNpcName.c_str(), &bSelected))
            {
                if (bSelected)
                    vecSelectedNpcList.push_back(x.iProtoID);
                else
                    vecSelectedNpcList.erase(std::find(vecSelectedNpcList.begin(), vecSelectedNpcList.end(), x.iProtoID));

                m_UserConfig->SetInt("Attack", "NpcList", vecSelectedNpcList);
            }

            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 165, 0, 255));
            RightText(std::to_string((int)x.fDistance) + "m");
            ImGui::PopStyleColor();
            ImGui::PopID();

            ImGui::PopStyleColor();
        }

        ImGui::TreePop();
    }

    if (bAutoTarget)
        ImGui::EndDisabled();
}