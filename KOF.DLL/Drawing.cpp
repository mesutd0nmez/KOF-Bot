#include "pch.h"
#include "Drawing.h"
#include "Memory.h"
#include "Bot.h"
#include "ConfigHandler.h"
#include "Client.h"
#include "Define.h"

LPCSTR Drawing::lpWindowName = APP_TITLE;
ImVec2 Drawing::vWindowSize = { 600, 600 };
ImGuiWindowFlags Drawing::WindowFlags = ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize;
bool Drawing::m_bDraw = true;
ImVec2 initial_pos(1600, 0);
Ini* m_UserConfig = NULL;
std::vector<__TABLE_UPC_SKILL> m_vecAvailableSkill;

void Drawing::Draw()
{
    m_UserConfig = ConfigHandler::GetUserConfig(Client::GetName());

    bool bRender = Client::IsCharacterLoaded() && m_UserConfig;

	if (IsDrawable() && bRender)
	{
        LoadSkillData();

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
    ImGui::SetCursorPosX((windowWidth - textWidth) * 0.97f);
    ImGui::TextUnformatted(strValue.c_str());
}

void Drawing::DrawGameController()
{
    ImGui::BeginChild(1, ImVec2(223, 563), true);
    {
        ImGui::TextUnformatted(Client::GetName().c_str());
        ImGui::SameLine();
        ImGui::NextColumn();

        std::stringstream strLevel;
        strLevel << "Lv " << Client::GetLevel();

        RightText(strLevel.str());
        ImGui::Separator();

        ImGui::Spacing();
        {
            ImGui::PushItemWidth(207);
            {
                std::stringstream strHpValue;
                strHpValue << Client::GetHp() << " / " << Client::GetMaxHp();

                CenteredText(strHpValue.str());

                float fHpProgress = (((float)Client::GetHp() / (float)Client::GetMaxHp()) * 100.f) / 100.0f;

                ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(255.0f, 0.0f, 0.0f, 1.0f));
                ImGui::ProgressBar(fHpProgress, ImVec2(0.0f, 0.0f));
                ImGui::PopStyleColor(1);

                std::stringstream strMpValue;
                strMpValue << Client::GetMp() << " / " << Client::GetMaxMp();

                CenteredText(strMpValue.str());

                float fMpProgress = (((float)Client::GetMp() / (float)Client::GetMaxMp()) * 100.f) / 100.0f;

                ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.0f, 0.0f, 255.0f, 1.0f));
                ImGui::ProgressBar(fMpProgress, ImVec2(0.0f, 0.0f));
                ImGui::PopStyleColor(1);
            }

            ImGui::PopItemWidth();
        }

        ImGui::Spacing();
        {
            ImGui::TextUnformatted("Quick Action");
            ImGui::Separator();

            if (ImGui::Button("Town", ImVec2(100.0f, 0.0f)))
            {
                Client::Town();
            }

            if (ImGui::Button("Test", ImVec2(100.0f, 0.0f)))
            {
                Client::RouteStart(1621, 438);
            }
        }

        ImGui::Spacing();
        {
            auto windowHeight = ImGui::GetWindowSize().y;
            auto fHeight = 35.0f;
            ImGui::SetCursorPosY((windowHeight - fHeight) * 0.97f);

            ImGui::TextUnformatted("Automation");
            ImGui::Separator();

            bool bAttackStatus = m_UserConfig->GetBool("Automation", "Attack", false);

            if (bAttackStatus)
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.255f, 0.0f, 1.0f));
            else
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.255f, 0.0f, 0.0f, 1.0f));

            if (ImGui::Button("Attack", ImVec2(100.0f, 0.0f)))
            {
                bAttackStatus = !bAttackStatus;
                m_UserConfig->SetInt("Automation", "Attack", bAttackStatus);
            }

            ImGui::PopStyleColor(1);

            ImGui::SameLine();

            bool bProtectionStatus = m_UserConfig->GetBool("Automation", "Protection", false);

            if (bProtectionStatus)
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.255f, 0.0f, 1.0f));
            else
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.255f, 0.0f, 0.0f, 1.0f));

            if (ImGui::Button("Protection", ImVec2(100.0f, 0.0f)))
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

                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Skill"))
            {
                if (m_vecAvailableSkill.size() == 0)
                    ImGui::BeginDisabled();

                DrawAutomatedAttackSkillTree();
                DrawAutomatedCharacterSkillTree();

                if (m_vecAvailableSkill.size() == 0)
                    ImGui::EndDisabled();

                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Attack"))
            {
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Party"))
            {
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Action"))
            {
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Tool"))
            {
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

        ImGui::SameLine();

        ImGui::PushItemWidth(100);

        int iHpProtectionItemIndex = m_UserConfig->GetInt("Protection", "HpItemIndex", 0);
        const char* nHpPotionItems[] = { "1920", "720", "340", "250", "128" };
        ImGui::Combo("##HpPotionItem", &iHpProtectionItemIndex, nHpPotionItems, IM_ARRAYSIZE(nHpPotionItems));

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

        ImGui::SameLine();

        ImGui::PushItemWidth(100);

        int iMpProtectionItemIndex = m_UserConfig->GetInt("Protection", "MpItemIndex", 0);
        const char* nMpPotionItems[] = { "1920", "720", "340", "250", "128" };
        ImGui::Combo("##MpPotionItem", &iMpProtectionItemIndex, nMpPotionItems, IM_ARRAYSIZE(nMpPotionItems));

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
            bool bGodMode = m_UserConfig->GetBool("Feature", "GodMode", false);

            if (ImGui::Checkbox("##GodMode", &bGodMode))
                m_UserConfig->SetInt("Feature", "GodMode", bGodMode ? 1 : 0);

            ImGui::SameLine();

            ImGui::Text("God Mode");

            ImGui::SameLine();

            bool bHyperNoah = m_UserConfig->GetBool("Feature", "HyperNoah", false);

            if (ImGui::Checkbox("##HyperNoah", &bHyperNoah))
                m_UserConfig->SetInt("Feature", "HyperNoah", bHyperNoah ? 1 : 0);

            ImGui::SameLine();

            ImGui::Text("Hyper Noah");
        }
    }
}

void Drawing::LoadSkillData()
{
    m_vecAvailableSkill.clear();

    auto pSkillList = TableHandler::GetSkillTable().GetData();

    for (const auto& [key, value] : pSkillList)
    {
        if (0 != std::to_string(value.iNeedSkill).substr(0, 3).compare(std::to_string(Client::GetClass())))
            continue;

        if (value.iTarget != SkillTargetType::TARGET_SELF && value.iTarget != SkillTargetType::TARGET_PARTY_ALL && value.iTarget != SkillTargetType::TARGET_FRIEND_WITHME &&
            value.iTarget != SkillTargetType::TARGET_ENEMY_ONLY && value.iTarget != SkillTargetType::TARGET_AREA_ENEMY)
            continue;

        if ((value.iSelfAnimID1 == 153 || value.iSelfAnimID1 == 154) || (value.iSelfFX1 == 32038 || value.iSelfFX1 == 32039))
            continue;

        switch (value.iNeedSkill % 10)
        {
            case 0:
                if (value.iNeedLevel > Client::GetLevel())
                    continue;
                break;
            case 5:
                if (value.iNeedLevel > Client::GetSkillPoint(5))
                    continue;
                break;
            case 6:
                if (value.iNeedLevel > Client::GetSkillPoint(6))
                    continue;
                break;
            case 7:
                if (value.iNeedLevel > Client::GetSkillPoint(7))
                    continue;
                break;
            case 8:
                if (value.iNeedLevel > Client::GetSkillPoint(8))
                    continue;
                break;
        }

        m_vecAvailableSkill.push_back(value);
    }
}

void Drawing::DrawAutomatedAttackSkillTree()
{
    std::vector<int> vecAttackList = m_UserConfig->GetInt("Automation", "AttackSkillList", std::vector<int>());

    std::stringstream strTreeText;

    if (TableHandler::GetSkillTable().GetDataSize() == 0)
        strTreeText << "|/-\\"[(int)(ImGui::GetTime() / 0.05f) & 3] << " ";

    strTreeText << "Automated attack skills";

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Bullet;
    if (ImGui::TreeNodeEx(strTreeText.str().c_str(), flags))
    {
        for (const auto& x : m_vecAvailableSkill)
        {
            if (x.iTarget != SkillTargetType::TARGET_ENEMY_ONLY && x.iTarget != SkillTargetType::TARGET_AREA_ENEMY)
                continue;

            ImGui::PushID(x.dwID);

            bool bSelected = std::find(vecAttackList.begin(), vecAttackList.end(), x.dwID) != vecAttackList.end();

            if (ImGui::Selectable(x.szName.c_str(), &bSelected))
            {
                if (bSelected)
                    vecAttackList.push_back(x.dwID);
                else
                    vecAttackList.erase(std::find(vecAttackList.begin(), vecAttackList.end(), x.dwID));

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

    if (TableHandler::GetSkillTable().GetDataSize() == 0)
        strTreeText << "|/-\\"[(int)(ImGui::GetTime() / 0.05f) & 3] << " ";

    strTreeText << "Automated character skills";

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Bullet;
    if (ImGui::TreeNodeEx(strTreeText.str().c_str(), flags))
    {
        for (const auto& x : m_vecAvailableSkill)
        {
            if (x.iTarget != SkillTargetType::TARGET_SELF && x.iTarget != SkillTargetType::TARGET_PARTY_ALL && x.iTarget != SkillTargetType::TARGET_FRIEND_WITHME)
                continue;

            ImGui::PushID(x.dwID);

            bool bSelected = std::find(vecCharacterSkillList.begin(), vecCharacterSkillList.end(), x.dwID) != vecCharacterSkillList.end();

            if (ImGui::Selectable(x.szName.c_str(), &bSelected))
            {
                if (bSelected)
                    vecCharacterSkillList.push_back(x.dwID);
                else
                    vecCharacterSkillList.erase(std::find(vecCharacterSkillList.begin(), vecCharacterSkillList.end(), x.dwID));

                m_UserConfig->SetInt("Automation", "CharacterSkillList", vecCharacterSkillList);
            }

            if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                ImGui::SetTooltip(x.szDesc.c_str());

            ImGui::PopID();
        }

        ImGui::TreePop();
    }
}
