#include "pch.h"
#include "Drawing_Hazar.h"
#include "Bot.h"
#include "Client.h"
#include "ClientHandler.h"
#include "UI.h"
#include "RouteManager.h"

#ifdef UI_HAZAR

Bot* Drawing::Bot = nullptr;
bool Drawing::bDraw = true;
bool Drawing::Done = false;

ClientHandler* m_pClient = nullptr;
Ini* m_pUserConfiguration = nullptr;
Ini* m_pAppConfiguration = nullptr;

PDIRECT3DTEXTURE9 m_pMapTexture = nullptr;
PDIRECT3DTEXTURE9 m_pMinimapTexture = nullptr;

float m_fScreenWidth = 0.0f;
float m_fScreenHeight = 0.0f;

bool m_bInitPos = false;

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
        ImVec2 WindowSize { 928, 760 };

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
	}

    if (!bTableLoaded)
        ImGui::EndDisabled();
}

void Drawing::DrawMainController()
{
    ImGui::TextUnformatted(skCryptDec("Ana Kontroller"));
    ImGui::Separator();

    ImGui::Spacing();
    {
        bool bAttackStatus = m_pUserConfiguration->GetBool(skCryptDec("Automation"), skCryptDec("Attack"), false);

        if (bAttackStatus)
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.255f, 0.0f, 1.0f));
        else
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.255f, 0.0f, 0.0f, 1.0f));

        if (ImGui::Button(bAttackStatus ? skCryptDec("Saldiri Durdur") : skCryptDec("Saldiri Baslat"), ImVec2(137.0f, 0.0f)))
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

        if (ImGui::Button(bCharacterStatus ? skCryptDec("Bot Durdur") : skCryptDec("Bot Baslat"), ImVec2(137.0f, 0.0f)))
        {
            bCharacterStatus = !bCharacterStatus;
            m_pUserConfiguration->SetInt(skCryptDec("Automation"), skCryptDec("Character"), bCharacterStatus);
        }

        ImGui::PopStyleColor(1);

        if (ImGui::Button(skCryptDec("Town At"), ImVec2(137.0f, 0.0f)))
        {
            m_pClient->SendTownPacket();
        }

        ImGui::SameLine();

        if (ImGui::Button(skCryptDec("Skilleri Yenile"), ImVec2(137.0f, 0.0f)))
        {
            m_pUserConfiguration->SetInt(skCryptDec("Automation"), skCryptDec("AttackSkillList"), std::vector<int>());
            m_pUserConfiguration->SetInt(skCryptDec("Automation"), skCryptDec("CharacterSkillList"), std::vector<int>());

            m_pClient->LoadSkillData();
        }

        if (ImGui::Button(skCryptDec("Ayarlari Sifirla"), ImVec2(282.0f, 0.0f)))
        {
            m_pUserConfiguration->Reset();
        }
    };
}

void Drawing::DrawTransformationController()
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

void Drawing::DrawSpeedController()
{
    ImGui::TextUnformatted(skCryptDec("Hiz Ayarlari"));
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

void Drawing::DrawDistanceController()
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

void Drawing::DrawProtectionController()
{
    ImGui::TextUnformatted(skCryptDec("Koruma Ayarlari"));
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
    }
}

void Drawing::DrawAutoLootController()
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

void Drawing::DrawAttackController()
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

        bool bMoveToTarget = m_pUserConfiguration->GetBool(skCryptDec("Attack"), skCryptDec("MoveToTarget"), false);

        if (ImGui::Checkbox(skCryptDec("##MoveToTargetCheckbox"), &bMoveToTarget))
            m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("MoveToTarget"), bMoveToTarget ? 1 : 0);

        ImGui::SameLine();

        ImGui::Text(skCryptDec("Hedefe Kos"));
    }
}

void Drawing::DrawIllegalController()
{
    ImGui::TextUnformatted(skCryptDec("Illegal Ayarlari"));
    ImGui::Separator();

    ImGui::Spacing();
    {
        bool bBasicAttackWithPacket = m_pUserConfiguration->GetBool(skCryptDec("Attack"), skCryptDec("BasicAttackWithPacket"), false);

        if (ImGui::Checkbox(skCryptDec("##BasicAttackWithPacketCheckbox"), &bBasicAttackWithPacket))
            m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("BasicAttackWithPacket"), bBasicAttackWithPacket ? 1 : 0);

        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255.0f, 0.0f, 0.0f, 1.0f));
        ImGui::Text(skCryptDec("R Illegal Vur"));
        ImGui::PopStyleColor();

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

        bool bDeathEffect = m_pUserConfiguration->GetBool(skCryptDec("Feature"), skCryptDec("DeathEffect"), false);

        if (ImGui::Checkbox(skCryptDec("##DeathEffect"), &bDeathEffect))
        {
            m_pClient->PatchDeathEffect(bDeathEffect);
            m_pUserConfiguration->SetInt(skCryptDec("Feature"), skCryptDec("DeathEffect"), bDeathEffect ? 1 : 0);
        }

        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255.0f, 0.0f, 0.0f, 1.0f));
        ImGui::Text(skCryptDec("Death Efektini Kaldir"));
        ImGui::PopStyleColor();
    }
}

void Drawing::DrawTargetListController()
{
    ImGui::TextUnformatted(skCryptDec("Hedef Ayarlari"));
    ImGui::Separator();

    ImGui::Spacing();
    {
        if (ImGui::Button(skCryptDec("Listeye Mob ID Ekle"), ImVec2(283.0f, 0.0f)))
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

        ImGui::BeginChild(skCryptDec("TargetListController"), ImVec2(282, 238), true);
        {

            bool bRangeLimit = m_pUserConfiguration->GetBool(skCryptDec("Attack"), skCryptDec("RangeLimit"), false);
            int iRangeLimitValue = m_pUserConfiguration->GetInt(skCryptDec("Attack"), skCryptDec("RangeLimitValue"), 100);
            bool bAutoTarget = m_pUserConfiguration->GetInt(skCryptDec("Attack"), skCryptDec("AutoTarget"), false);

            if (bAutoTarget)
                ImGui::BeginDisabled();

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

        }
        ImGui::EndChild();
    }
}

void Drawing::DrawSkillController()
{
    ImGui::TextUnformatted(skCryptDec("Skill Ayarlari"));
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
        ImGui::Text(skCryptDec("El Dusurmeyi Iptal Et"));
        ImGui::PopStyleColor();

        ImGui::BeginChild(skCryptDec("SkillController"), ImVec2(282, 241), true);
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

void Drawing::DrawSaveCPUController()
{
    ImGui::TextUnformatted(skCryptDec("Save CPU Ayarlari"));
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
    ImGui::TextUnformatted(skCryptDec("Tedarik Ayarlari"));
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

void Drawing::DrawPartyController()
{
    ImGui::TextUnformatted(skCryptDec("Party Ayarlari"));
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

void Drawing::DrawSettingsController()
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
    }
}

void Drawing::DrawScene()
{
    ImGui::BeginChild(skCryptDec("MainChildArea1"), ImVec2(298, 720), true);
    {
        DrawMainController();
        ImGui::Separator();
        DrawAutoLootController();
        ImGui::Separator();
        DrawSpeedController();
        ImGui::Separator();
        DrawAttackController();
        ImGui::Separator();
        DrawTargetListController();
    }
    ImGui::EndChild();

    ImGui::SameLine();

    ImGui::BeginChild(skCryptDec("MainChildArea2"), ImVec2(298, 720), true);
    {
        DrawProtectionController();
        ImGui::Separator();
        DrawTransformationController();
        ImGui::Separator();
        DrawDistanceController();
        ImGui::Separator();
        DrawIllegalController();
        ImGui::Separator();
        DrawSkillController();
    }
    ImGui::EndChild();

    ImGui::SameLine();

    ImGui::BeginChild(skCryptDec("MainChildArea3"), ImVec2(298, 720), true);
    {
        DrawSizeController();
        ImGui::Separator();
        DrawSaveCPUController();
        ImGui::Separator();
        DrawSupplyController();
        ImGui::Separator();
        DrawListenerController();
        ImGui::Separator();
        DrawPartyController();
        ImGui::Separator();
        DrawSettingsController();
    }
    ImGui::EndChild();

}

#endif