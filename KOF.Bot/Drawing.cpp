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

char m_szPlannedRouteName[255] = "";

char m_szItemSearchName[255] = "";

int m_iInventoryManagementWindow = 0;

int m_iSelectedSellingGroup = 253000;
int m_iNewCredit = 1;
std::string m_szSelectedSellingGroupNpc = skCryptDec("Potcu");

Drawing::Scene Drawing::m_Scene = Scene::HIDDEN;
bool Drawing::m_bUpdateSceneSize = false;

uint32_t iSelectedDay = 0;

float m_fLastCreditRequestTime = 0.0f;
float m_fLastSubscriptionRequestTime = 0.0f;

void Drawing::SetScene(Scene scene)
{
    if (scene != Scene::UI)
    {
        SetWindowPos(UI::hWindow, nullptr, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), SWP_SHOWWINDOW);
    }

    if (scene == Scene::UI)
        bDraw = false;
    else
        bDraw = true;

    if (m_Scene != scene)
    {
        m_bUpdateSceneSize = true;
    }

    m_Scene = scene;    
}

/**
	@brief : function that check if the menu is drawed.
	@retval : true if the function is drawed else false.
**/
bool Drawing::IsActive()
{
	return bDraw == true;
}

/**
	@brief : Function that draw the ImGui menu.
**/
void Drawing::Draw()
{
    vWindowSize = { 0, 0 };

    switch (GetScene())
    {
        case Scene::LOGIN:
            vWindowSize = { 285, 170 };
            break;

        case Scene::UPDATE:
            vWindowSize = { 285, 70 };
            break;

        case Scene::LOADER:
            vWindowSize = { 285, 395 };
            break;

        case Scene::UI:
            vWindowSize = { 800, 650 };
            break;
    }

    if (Drawing::GetScene() != Drawing::Scene::UI)
    {
        float fScreenWidth = (GetSystemMetrics(SM_CXSCREEN)) / 2.0f;
        float fScreenHeight = (GetSystemMetrics(SM_CYSCREEN)) / 2.0f;

        ImVec2 vec2InitialPos = { fScreenWidth, fScreenHeight };

        vec2InitialPos.x -= vWindowSize.x / 2;
        vec2InitialPos.y -= vWindowSize.y / 2;

        ImGui::SetNextWindowPos(vec2InitialPos, m_bUpdateSceneSize ? ImGuiCond_Always : ImGuiCond_Once);
        ImGui::SetNextWindowSize(vWindowSize, m_bUpdateSceneSize ? ImGuiCond_Always : ImGuiCond_Once);
        ImGui::SetNextWindowBgAlpha(1.0f);
    }
    else
    {
        RECT rect;
        GetWindowRect(UI::hTargetWindow, &rect);

        float fScreenWidth = (rect.right - rect.left) / 2.0f;
        float fScreenHeight = (rect.bottom - rect.top) / 2.0f;

        ImVec2 vec2InitialPos = { fScreenWidth, fScreenHeight };

        vec2InitialPos.x -= vWindowSize.x / 2;
        vec2InitialPos.y -= vWindowSize.y / 2;

        ImGui::SetNextWindowPos(vec2InitialPos, m_bUpdateSceneSize ? ImGuiCond_Always : ImGuiCond_Once);
        ImGui::SetNextWindowSize(vWindowSize, m_bUpdateSceneSize ? ImGuiCond_Always : ImGuiCond_Once);
        ImGui::SetNextWindowBgAlpha(1.0f);
    }

    m_bUpdateSceneSize = false;

    ImGui::Begin(lpWindowName, &bDraw, WindowFlags);
    {
        m_iniAppConfiguration = Drawing::Bot->GetAppConfiguration();
        m_pClient = Drawing::Bot->GetClientHandler();

        if (m_pClient)
        {
            m_pUserConfiguration = m_pClient->GetUserConfiguration();
        }

        bool bIsAuthenticated = Drawing::Bot->IsAuthenticated();
        bool bIsGameStarting = Drawing::Bot->IsGameStarting();
        bool bIsGameWaitingReady = m_pClient && !m_pClient->IsReady();

        switch (GetScene())
        {
            case Scene::LOGIN:
            {
                ImGui::BeginChild(skCryptDec("##LoginBoard"), ImVec2(270, 95), true);
                {
                    ImGui::SetCursorPos(ImVec2(10, 10));
                    ImGui::Text(skCryptDec("E-Posta"));
                    ImGui::SameLine();
                    ImGui::SetCursorPos(ImVec2(88, 7));
                    ImGui::InputText(skCryptDec("##EMail"), &m_szEmail[0], 100);

                    ImGui::SetCursorPos(ImVec2(10, 36));
                    ImGui::Text(skCryptDec("Sifre"));
                    ImGui::SameLine();
                    ImGui::SetCursorPos(ImVec2(88, 35));
                    ImGui::InputText(skCryptDec("##Password"), &m_szPassword[0], 25, ImGuiInputTextFlags_Password);

                    ImGui::Spacing();

                    if (ImGui::Button(skCryptDec("Etkinlestir"), ImVec2(255.0f, 0.0f)))
                    {
                        Drawing::Bot->SendLogin(m_szEmail, m_szPassword);
                        m_iniAppConfiguration->SetString(skCryptDec("Internal"), skCryptDec("User"), m_szEmail);
                    }

                    ImGui::EndChild();
                }

                ImGui::BeginChild(skCryptDec("##LoginFooter"), ImVec2(270, 33), true);
                {
                    if (Drawing::Bot->m_isAuthenticationMessage)
                    {
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255.0f, 0.0f, 0.0f, 1.0f));
                        ImGui::Text("%s", Drawing::Bot->m_szAuthenticationMessage.c_str());
                        ImGui::PopStyleColor();
                    }
                    else
                    {
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.00f, 1.00f, 0.00f, 1.00f));
                        ImGui::Text(skCryptDec("Aktivasyon islemi bekleniyor"));
                        ImGui::PopStyleColor();
                    }

                    ImGui::EndChild();
                }
            }
            break;

            case Scene::UPDATE:
            {
                ImGui::BeginChild(skCryptDec("##UpdateInformation"), ImVec2(270, 33), true);
                {
                    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 215, 0, 255));
                    ImGui::Text(skCryptDec("Guncelleme yapiliyor, bekleyin %c"), "|/-\\"[(int)(ImGui::GetTime() / 0.05f) & 3]);
                    ImGui::PopStyleColor();

                    ImGui::EndChild();
                }
            }
            break;

            case Scene::LOADER:
            {
                auto iSubscriptionEndAt = Drawing::Bot->m_iSubscriptionEndAt;
                std::time_t iCurrentTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

                if (ImGui::BeginTabBar(skCryptDec("##Loader.TabBar"), ImGuiTabBarFlags_None))
                {
                    if (iSubscriptionEndAt > iCurrentTime)
                    {
                        if (ImGui::BeginTabItem(skCryptDec("Login")))
                        {
                            ImGui::BeginChild(skCryptDec("##AutoLoginBoard"), ImVec2(270, 170), true);
                            {
                                if (bIsGameStarting || bIsGameWaitingReady)
                                    ImGui::BeginDisabled();

                                ImGui::SetCursorPos(ImVec2(10, 10));
                                ImGui::Text(skCryptDec("ID"));
                                ImGui::SameLine();
                                ImGui::SetCursorPos(ImVec2(88, 7));

                                if (ImGui::InputText(skCryptDec("##ID"), &Drawing::Bot->m_szID[0], 100))
                                {
                                    Drawing::Bot->m_szID = m_iniAppConfiguration->SetString(skCryptDec("AutoLogin"), skCryptDec("ID"), Drawing::Bot->m_szID.c_str());
                                }

                                ImGui::SetCursorPos(ImVec2(10, 36));
                                ImGui::Text(skCryptDec("Sifre"));
                                ImGui::SameLine();
                                ImGui::SetCursorPos(ImVec2(88, 33));

                                if (ImGui::InputText(skCryptDec("##Password"), &Drawing::Bot->m_szPassword[0], 100, ImGuiInputTextFlags_Password))
                                {
                                    Drawing::Bot->m_szPassword = m_iniAppConfiguration->SetString(skCryptDec("AutoLogin"), skCryptDec("Password"), Drawing::Bot->m_szPassword.c_str());
                                }

                                std::vector<std::string> vecServerList;
                                vecServerList.push_back(skCryptDec("OREADS"));
                                vecServerList.push_back(skCryptDec("MINARK"));
                                vecServerList.push_back(skCryptDec("DESTAN"));
                                vecServerList.push_back(skCryptDec("DRYADS"));
                                vecServerList.push_back(skCryptDec("PANDORA"));
                                vecServerList.push_back(skCryptDec("FELIS"));
                                vecServerList.push_back(skCryptDec("AGARTHA"));
                                vecServerList.push_back(skCryptDec("ZERO"));

                                ImGui::SetCursorPos(ImVec2(10, 62));
                                ImGui::Text(skCryptDec("Server"));
                                ImGui::SameLine();
                                ImGui::SetCursorPos(ImVec2(88, 59));

                                if (ImGui::BeginCombo(skCryptDec("##ServerListCombo"), vecServerList[Drawing::Bot->m_iServerID].c_str()))
                                {
                                    for (size_t i = 0; i < vecServerList.size(); i++)
                                    {
                                        const bool bIsSelected = Drawing::Bot->m_iServerID == i;

                                        if (ImGui::Selectable(vecServerList[i].c_str(), bIsSelected))
                                        {
                                            Drawing::Bot->m_iServerID = m_iniAppConfiguration->SetInt(skCryptDec("AutoLogin"), skCryptDec("Server"), i);
                                        }

                                        if (bIsSelected)
                                            ImGui::SetItemDefaultFocus();
                                    }

                                    ImGui::EndCombo();
                                }

                                ImGui::SetCursorPos(ImVec2(10, 88));
                                ImGui::Text(skCryptDec("Kanal"));
                                ImGui::SameLine();
                                ImGui::SetCursorPos(ImVec2(88, 85));

                                std::string szSelectedChannelName = vecServerList[Drawing::Bot->m_iServerID].c_str() + std::to_string(Drawing::Bot->m_iChannelID + 1);

                                if (ImGui::BeginCombo(skCryptDec("##ChannelListCombo"), szSelectedChannelName.c_str()))
                                {
                                    for (size_t i = 0; i < 6; i++)
                                    {
                                        const bool bIsSelected = Drawing::Bot->m_iChannelID == i;

                                        std::string szChannelName = vecServerList[Drawing::Bot->m_iServerID].c_str() + std::to_string(i + 1);
                                        if (ImGui::Selectable(szChannelName.c_str(), bIsSelected))
                                        {
                                            Drawing::Bot->m_iChannelID = m_iniAppConfiguration->SetInt(skCryptDec("AutoLogin"), skCryptDec("Channel"), i);
                                        }

                                        if (bIsSelected)
                                            ImGui::SetItemDefaultFocus();
                                    }

                                    ImGui::EndCombo();
                                }

                                ImGui::SetCursorPos(ImVec2(10, 114));
                                ImGui::Text(skCryptDec("Slot"));
                                ImGui::SameLine();
                                ImGui::SetCursorPos(ImVec2(88, 111));

                                if (ImGui::DragInt(skCryptDec("##SlotValue"), &Drawing::Bot->m_iSlotID, 1, 1, 4))
                                    Drawing::Bot->m_iSlotID = m_iniAppConfiguration->SetInt(skCryptDec("AutoLogin"), skCryptDec("Slot"), Drawing::Bot->m_iSlotID);

                                ImGui::Spacing();
                                if (ImGui::Button(skCryptDec("AnyOTP"), ImVec2(255.0f, 0.0f)))
                                {
                                    ImGui::OpenPopup(skCryptDec("AnyOTP Ayarlari"));
                                }

                                if (ImGui::BeginPopupModal(skCryptDec("AnyOTP Ayarlari"), 0, ImGuiWindowFlags_NoResize))
                                {
                                    if (Drawing::Bot->m_hModuleAnyOTP == NULL)
                                    {
                                        ImGui::Text(skCryptDec("AnyOTP bilgisayarda kurulu degil"));

                                        ImGui::Spacing();

                                        if (ImGui::Button(skCryptDec("Kapat"), ImVec2(230, 0.0f)))
                                        {
                                            ImGui::CloseCurrentPopup();
                                        }
                                    }
                                    else
                                    {
                                        ImGui::BeginChild(skCryptDec("##AnyOtpBoard"), ImVec2(270, 100), true);
                                        {
                                            ImGui::SetCursorPos(ImVec2(10, 10));
                                            ImGui::Text(skCryptDec("OTP Sifre"));
                                            ImGui::SameLine();
                                            ImGui::SetCursorPos(ImVec2(88, 7));

                                            if (ImGui::InputText(skCryptDec("##AnyOTPPassword"), &Drawing::Bot->m_szAnyOTPPassword[0], 100))
                                            {
                                                Drawing::Bot->m_szAnyOTPPassword = m_iniAppConfiguration->SetString(skCryptDec("AnyOTP"), skCryptDec("Password"), Drawing::Bot->m_szAnyOTPPassword.c_str());
                                            }

                                            ImGui::SetCursorPos(ImVec2(10, 36));
                                            ImGui::Text(skCryptDec("OTP ID"));
                                            ImGui::SameLine();
                                            ImGui::SetCursorPos(ImVec2(88, 33));

                                            if (ImGui::InputText(skCryptDec("##AnyOTPID"), &Drawing::Bot->m_szAnyOTPID[0], 100))
                                            {
                                                Drawing::Bot->m_szAnyOTPID = m_iniAppConfiguration->SetString(skCryptDec("AnyOTP"), skCryptDec("ID"), Drawing::Bot->m_szAnyOTPID.c_str());
                                            }

                                            ImGui::Spacing();
                                            ImGui::Separator();

                                            ImGui::SetCursorPos(ImVec2(114, 73));

                                            std::string szCode = to_string(Drawing::Bot->ReadAnyOTPCode(Drawing::Bot->m_szAnyOTPPassword, Drawing::Bot->m_szAnyOTPID).c_str());

                                            ImGui::Text(skCryptDec("%s"), szCode.c_str());
                                        }

                                        ImGui::EndChild();

                                        ImGui::Spacing();

                                        if (ImGui::Button(skCryptDec("Sifirla"), ImVec2(270, 0.0f)))
                                        {
                                            Drawing::Bot->m_szAnyOTPID = m_iniAppConfiguration->SetString(skCryptDec("AnyOTP"), skCryptDec("ID"), to_string(Drawing::Bot->GetAnyOTPHardwareID().c_str()).c_str());
                                        }

                                        if (ImGui::Button(skCryptDec("Kapat"), ImVec2(270, 0.0f)))
                                        {
                                            ImGui::CloseCurrentPopup();
                                        }
                                    }

                                    ImGui::EndPopup();
                                }

                                if (bIsGameStarting || bIsGameWaitingReady)
                                    ImGui::EndDisabled();

                                ImGui::EndChild();
                            }

                            ImGui::EndTabItem();
                        }

                        if (ImGui::BeginTabItem(skCryptDec("Proxy")))
                        {
                            ImGui::BeginChild(skCryptDec("##ProxyConfigurationBoard"), ImVec2(270, 170), true);
                            {
                                if (bIsGameStarting || bIsGameWaitingReady)
                                    ImGui::BeginDisabled();

                                ImGui::SetCursorPos(ImVec2(10, 10));
                                ImGui::Text(skCryptDec("IP"));
                                ImGui::SameLine();
                                ImGui::SetCursorPos(ImVec2(88, 7));

                                if (ImGui::InputText(skCryptDec("##ProxyIP"), &Drawing::Bot->m_szProxyIP[0], 100))
                                {
                                    Drawing::Bot->m_szProxyIP = m_iniAppConfiguration->SetString(skCryptDec("Proxy"), skCryptDec("IP"), Drawing::Bot->m_szProxyIP.c_str());
                                }

                                ImGui::SetCursorPos(ImVec2(10, 36));
                                ImGui::Text(skCryptDec("Port"));
                                ImGui::SameLine();
                                ImGui::SetCursorPos(ImVec2(88, 33));

                                if (ImGui::InputInt(skCryptDec("##ProxyPort"), &Drawing::Bot->m_iProxyPort, 0))
                                {
                                    Drawing::Bot->m_iProxyPort = m_iniAppConfiguration->SetInt(skCryptDec("Proxy"), skCryptDec("Port"), Drawing::Bot->m_iProxyPort);
                                }

                                ImGui::SetCursorPos(ImVec2(10, 62));
                                ImGui::Text(skCryptDec("Protokol"));
                                ImGui::SameLine();
                                ImGui::SetCursorPos(ImVec2(88, 59));

                                ImGui::BeginDisabled();

                                ImGui::BeginCombo(skCryptDec("##ProxyTypeCombo"), "SOCKS5");

                                ImGui::EndDisabled();

                                ImGui::SetCursorPos(ImVec2(10, 88));
                                ImGui::Text(skCryptDec("Username"));
                                ImGui::SameLine();
                                ImGui::SetCursorPos(ImVec2(88, 85));

                                if (ImGui::InputText(skCryptDec("##ProxyUsername"), &Drawing::Bot->m_szProxyUsername[0], 100))
                                {
                                    Drawing::Bot->m_szProxyUsername = m_iniAppConfiguration->SetString(skCryptDec("Proxy"), skCryptDec("Username"), Drawing::Bot->m_szProxyUsername.c_str());
                                }

                                ImGui::SetCursorPos(ImVec2(10, 114));
                                ImGui::Text(skCryptDec("Password"));
                                ImGui::SameLine();
                                ImGui::SetCursorPos(ImVec2(88, 111));

                                if (ImGui::InputText(skCryptDec("##ProxyPassword"), &Drawing::Bot->m_szProxyPassword[0], 100))
                                {
                                    Drawing::Bot->m_szProxyPassword = m_iniAppConfiguration->SetString(skCryptDec("Proxy"), skCryptDec("Password"), Drawing::Bot->m_szProxyPassword.c_str());
                                }

                                ImGui::Spacing();

                                if (Drawing::Bot->m_bCheckingProxy)
                                    ImGui::BeginDisabled();

                                std::string szProxyTestButtonText = skCryptDec("Proxy Test");

                                if (Drawing::Bot->m_bCheckingProxy)
                                {
                                    char szLoadingString[100];
                                    snprintf(szLoadingString, sizeof(szLoadingString), "%c", "|/-\\"[(int)(ImGui::GetTime() / 0.05f) & 3]);

                                    szProxyTestButtonText = skCryptDec("Proxy Test Ediliyor");
                                    szProxyTestButtonText += " ";
                                    szProxyTestButtonText += szLoadingString;
                                }

                                if (ImGui::Button(szProxyTestButtonText.c_str(), ImVec2(255.0f, 0.0f)))
                                {
                                    Drawing::Bot->CheckProxy(
                                        Drawing::Bot->m_szProxyIP,
                                        (uint16_t)Drawing::Bot->m_iProxyPort,
                                        Drawing::Bot->m_szProxyUsername.c_str(),
                                        Drawing::Bot->m_szProxyPassword.c_str());
                                }

                                if (Drawing::Bot->m_bCheckingProxy)
                                    ImGui::EndDisabled();

                                if (bIsGameStarting || bIsGameWaitingReady)
                                    ImGui::EndDisabled();

                                ImGui::EndChild();
                            }
                            ImGui::EndTabItem();
                        }
                    }

                    if (ImGui::BeginTabItem(skCryptDec("Magaza")))
                    {
                        ImGui::BeginChild(skCryptDec("##ShopBoard"), ImVec2(270, 170), true);
                        {
                            if (bIsGameStarting || bIsGameWaitingReady)
                                ImGui::BeginDisabled();

                            ImGui::SetCursorPos(ImVec2(10, 10));
                            ImGui::Text(skCryptDec("Kredi"));
                            ImGui::SameLine();
                            ImGui::SetCursorPos(ImVec2(88, 7));

                            ImGui::BeginDisabled();
                            ImGui::InputInt(skCryptDec("##CurrentCredit"), &Drawing::Bot->m_iCredit, 0, 0, ImGuiInputTextFlags_ReadOnly);
                            ImGui::EndDisabled();

                            if (Drawing::Bot->m_iCredit <= -1)
                                ImGui::BeginDisabled();

                            ImGui::SetCursorPos(ImVec2(10, 36));
                            ImGui::Text(skCryptDec("Yeni Kredi"));
                            ImGui::SameLine();
                            ImGui::SetCursorPos(ImVec2(88, 33));

                            ImGui::InputInt(skCryptDec("##NewCredit"), &m_iNewCredit, 0, 100);

                            if (Drawing::Bot->m_iCredit <= -1)
                                ImGui::EndDisabled();

                            ImGui::Spacing();
                            ImGui::Separator();
                            ImGui::Spacing();

                            float fCurrentTime = TimeGet();
                            bool fDisableCreditButton = fCurrentTime < (m_fLastCreditRequestTime + (15000.0f / 1000.0f));

                            if (Drawing::Bot->m_iCredit <= -1 || m_iNewCredit <= 0 || fDisableCreditButton)
                                ImGui::BeginDisabled();

                            std::string szCreditButtonText = skCryptDec("Kredi Satin Al");

                            if (fDisableCreditButton)
                            {
                                char szLoadingString[100];
                                snprintf(szLoadingString, sizeof(szLoadingString), "%c", "|/-\\"[(int)(ImGui::GetTime() / 0.05f) & 3]);

                                szCreditButtonText += " ";
                                szCreditButtonText += szLoadingString;
                            }

                            if (ImGui::Button(szCreditButtonText.c_str(), ImVec2(255.0f, 0.0f)))
                            {
                                m_fLastCreditRequestTime = TimeGet();
                                Drawing::Bot->SendPurchase(0, m_iNewCredit, 0);
                            }

                            if (Drawing::Bot->m_iCredit <= -1 || m_iNewCredit <= 0 || fDisableCreditButton)
                                ImGui::EndDisabled();

                            ImGui::Spacing();
                            ImGui::Separator();
                            ImGui::Spacing();

                            ImGui::SetCursorPosX(10);
                            ImGui::Text(skCryptDec("Paket"));
                            ImGui::SameLine();
                            ImGui::SetCursorPos(ImVec2(88, 104));

                            std::vector<std::string> vecDayList;
                            vecDayList.push_back(skCryptDec("15 Gun"));
                            vecDayList.push_back(skCryptDec("30 Gun"));

                            if(ImGui::BeginCombo(skCryptDec("##DayListCombo"), vecDayList[iSelectedDay].c_str()))
                            {
                                for (size_t i = 0; i < vecDayList.size(); i++)
                                {
                                    const bool bIsSelected = iSelectedDay == i;

                                    if (ImGui::Selectable(vecDayList[i].c_str(), bIsSelected))
                                    {
                                        iSelectedDay = i;
                                    }

                                    if (bIsSelected)
                                        ImGui::SetItemDefaultFocus();
                                }

                                ImGui::EndCombo();
                            }

                            ImGui::Spacing();
                            ImGui::Separator();
                            ImGui::Spacing();

                            bool fDisableSubscriptionButton = fCurrentTime < (m_fLastSubscriptionRequestTime + (15000.0f / 1000.0f));

                            if (fDisableSubscriptionButton)
                                ImGui::BeginDisabled();

                            std::string szSubscriptionButtonText = skCryptDec("Abonelik Suresini Uzat");

                            if (fDisableSubscriptionButton)
                            {
                                char szLoadingString[100];
                                snprintf(szLoadingString, sizeof(szLoadingString), "%c", "|/-\\"[(int)(ImGui::GetTime() / 0.05f) & 3]);

                                szSubscriptionButtonText += " ";
                                szSubscriptionButtonText += szLoadingString;
                            }

                            if (ImGui::Button(szSubscriptionButtonText.c_str(), ImVec2(255.0f, 0.0f)))
                            {
                                m_fLastSubscriptionRequestTime = TimeGet();
                                Drawing::Bot->SendPurchase(1, 0, iSelectedDay);
                            }

                            if (fDisableSubscriptionButton)
                                ImGui::EndDisabled();

                            if (bIsGameStarting || bIsGameWaitingReady)
                                ImGui::EndDisabled();

                            ImGui::EndChild();
                        }
                        ImGui::EndTabItem();
                    }

                    ImGui::EndTabBar();
                }

                if (iSubscriptionEndAt < iCurrentTime)
                    ImGui::BeginDisabled();

                ImGui::BeginChild(skCryptDec("##LoaderButtonBoard"), ImVec2(270, 120), true);
                {
                    if (bIsGameStarting || bIsGameWaitingReady)
                        ImGui::BeginDisabled();

                    if (ImGui::Checkbox(skCryptDec("Oto login"), &Drawing::Bot->m_bAutoLogin))
                    {
                        if (Drawing::Bot->m_bAutoLogin)
                        {
                            Drawing::Bot->m_bAutoStart = m_iniAppConfiguration->SetInt(skCryptDec("Automation"), skCryptDec("Start"), true);
                        }

                        Drawing::Bot->m_bAutoLogin = m_iniAppConfiguration->SetInt(skCryptDec("Automation"), skCryptDec("Login"), Drawing::Bot->m_bAutoLogin);
                    }

                    if (ImGui::Checkbox(skCryptDec("Proxy ile oyuna baglan"), &Drawing::Bot->m_bConnectWithProxy))
                        Drawing::Bot->m_bConnectWithProxy = m_iniAppConfiguration->SetInt(skCryptDec("Proxy"), skCryptDec("Enable"), Drawing::Bot->m_bConnectWithProxy);


                    if (ImGui::Checkbox(skCryptDec("Oyunu otomatik baslat"), &Drawing::Bot->m_bAutoStart))
                        Drawing::Bot->m_bAutoStart = m_iniAppConfiguration->SetInt(skCryptDec("Automation"), skCryptDec("Start"), Drawing::Bot->m_bAutoStart);

                    ImGui::Separator();
                    ImGui::Spacing();

                    if (bIsGameStarting || bIsGameWaitingReady)
                        ImGui::EndDisabled();

                    if (bIsGameStarting || bIsGameWaitingReady)
                    {
                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.35f, 0.0f, 0.0f, 1.0f));
                        if (ImGui::Button(skCryptDec("Baslatmayi Durdur"), ImVec2(255.0f, 0.0f)))
                        {
                            Drawing::Bot->StopStartGameProcess();
                        }
                        ImGui::PopStyleColor();
                    }
                    else
                    {
                        if (ImGui::BeginPopupModal(skCryptDec("Baslatma Hatasi"), 0, ImGuiWindowFlags_NoResize))
                        {
                            ImGui::Text(skCryptDec("Knight Online bilgisayarinizda kurulu degil"));

                            ImGui::Spacing();

                            if (ImGui::Button(skCryptDec("Kapat"), ImVec2(303, 0.0f)))
                            {
                                ImGui::CloseCurrentPopup();
                            }

                            ImGui::EndPopup();
                        }

                        if (ImGui::Button(skCryptDec("Oyunu Baslat"), ImVec2(255.0f, 0.0f)))
                        {
                            if (!FileExists(skCryptDec("C:\\NTTGame\\KnightOnlineEn\\KnightOnLine.exe")))
                            {
                                ImGui::OpenPopup(skCryptDec("Baslatma Hatasi"));
                            }
                            else
                            {
                                Drawing::Bot->StartGame();
                            }
                        }
                    }

                    ImGui::EndChild();
                }

                if (iSubscriptionEndAt < iCurrentTime)
                    ImGui::EndDisabled();

                ImGui::BeginChild(skCryptDec("##LoaderFooter"), ImVec2(270, 33), true);
                {
                    if (bIsGameStarting)
                    {
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.00f, 1.00f, 0.00f, 1.00f));
                        ImGui::Text(skCryptDec("Oyun baslatiliyor, bekleyin %c"), "|/-\\"[(int)(ImGui::GetTime() / 0.05f) & 3]);
                        ImGui::PopStyleColor();
                    }
                    else if (bIsGameWaitingReady)
                    {
                        if (Drawing::Bot->IsClientProcessLost())
                        {
                            if (Drawing::Bot->m_bAutoLogin)
                            {
                                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.00f, 1.00f, 0.00f, 1.00f));
                                ImGui::Text(skCryptDec("Oto login basliyor, bekleyin (%d)"), (60 - (int)(TimeGet() - m_pClient->m_fLastDisconnectTime)));
                                ImGui::PopStyleColor();
                            }
                            else
                            {
                                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255.0f, 0.0f, 0.0f, 1.0f));
                                ImGui::Text(skCryptDec("Oyun baglantisi kesildi"));
                                ImGui::PopStyleColor();
                            }
                        }
                        else
                        {
                            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.00f, 1.00f, 0.00f, 1.00f));
                            ImGui::Text(skCryptDec("Oyun hazir! Bot yukleniyor %c"), "|/-\\"[(int)(ImGui::GetTime() / 0.05f) & 3]);
                            ImGui::PopStyleColor();
                        }
                    }
                    else
                    {
                        if (Drawing::Bot->m_szCheckingProxyResult.size() > 0)
                        {
                            if(Drawing::Bot->m_bCheckingProxyResult)
                                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.00f, 1.00f, 0.00f, 1.00f));
                            else
                                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255.0f, 0.0f, 0.0f, 1.0f));

                            ImGui::Text("%s", Drawing::Bot->m_szCheckingProxyResult.c_str());
                            ImGui::PopStyleColor();
                        }
                        else
                        {
                            auto iSubscriptionEndAt = Drawing::Bot->m_iSubscriptionEndAt;
                            std::time_t iCurrentTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

                            if (iSubscriptionEndAt > iCurrentTime)
                            {
                                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.00f, 1.00f, 0.00f, 1.00f));
                                ImGui::Text(skCryptDec("%s"), RemainingTime(iSubscriptionEndAt - iCurrentTime).c_str());
                                ImGui::PopStyleColor();
                            }
                            else
                            {
                                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255.0f, 0.0f, 0.0f, 1.0f));
                                ImGui::Text(skCryptDec("Abonelik suresi doldu, Magazadan uzatabilirsin"));
                                ImGui::PopStyleColor();
                            }
                        }                
                    }

                    ImGui::EndChild();
                }
            }
            break;

            case Scene::UI:
            {
                ImGui::BeginChild(skCryptDec("##Main.Child1"), ImVec2(288, 610), true);
                {
                    DrawMainController();
                    ImGui::Separator();

                    DrawModeController();
                    ImGui::Separator();

                    DrawCommonController();
                    ImGui::Separator();

#ifdef _DEBUG
                    ImGui::Spacing();
                    ImGui::SetCursorPosX(42);
                    ImGui::Text("Average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
#endif
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

                            DrawVIPStorageController();
                            ImGui::Separator();

                            DrawItemListController();

                            ImGui::EndTabItem();
                        }

                        if (ImGui::BeginTabItem(skCryptDec("Tedarik")))
                        {
                            DrawSupplyController();
                            ImGui::Separator();

                            DrawSupplyListController();
                            ImGui::Separator();

                            DrawWeaponListController();

                            ImGui::EndTabItem();
                        }

                        if (ImGui::BeginTabItem(skCryptDec("Sistem")))
                        {
                            DrawSettingsController();

                            ImGui::EndTabItem();
                        }

                        ImGui::EndTabBar();
                    }

                    ImGui::EndChild();
                }
            }
            break;
        }
    }

    ImGui::End();
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
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.35f, 0.0f, 0.0f, 1.0f));

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
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.35f, 0.0f, 0.0f, 1.0f));

        if (ImGui::Button(m_pClient->m_bCharacterStatus ? skCryptDec("Bot Durdur") : skCryptDec("Bot Baslat"), ImVec2(132.0f, 0.0f)))
        {
            m_pClient->m_bCharacterStatus = !m_pClient->m_bCharacterStatus;
            m_pUserConfiguration->SetInt(skCryptDec("Automation"), skCryptDec("Character"), m_pClient->m_bCharacterStatus);
        }

        ImGui::PopStyleColor(1);
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
            uint32_t m_iID;
            std::string m_szName;
            DisguiseItem(uint32_t iID, std::string szName) : m_iID(iID), m_szName(szName) {};
        };

        std::vector<DisguiseItem> vecDisguiseItems;

        vecDisguiseItems.push_back(DisguiseItem(381001000, skCryptDec("TS Scroll (60 Dakika)")));
        vecDisguiseItems.push_back(DisguiseItem(-1, skCryptDec("TS Scroll (90 Dakika)")));
        vecDisguiseItems.push_back(DisguiseItem(379090000, skCryptDec("TS Totem 1")));
        vecDisguiseItems.push_back(DisguiseItem(379093000, skCryptDec("TS Totem 2")));

        std::string szSelectedTransformationItem = skCryptDec("TS Scroll");

        const auto pFindedTransformationItem = std::find_if(vecDisguiseItems.begin(), vecDisguiseItems.end(),
            [&](const DisguiseItem& a) { return a.m_iID == m_pClient->m_iTransformationItem; });

        if (pFindedTransformationItem != vecDisguiseItems.end())
            szSelectedTransformationItem = pFindedTransformationItem->m_szName;

        if (ImGui::BeginCombo(skCryptDec("##Transformation.ItemList"), szSelectedTransformationItem.c_str()))
        {
            for (auto& e : vecDisguiseItems)
            {
                const bool bIsSelected = m_pClient->m_iTransformationItem == e.m_iID;

                if (ImGui::Selectable(e.m_szName.c_str(), bIsSelected))
                {
                    m_pClient->m_iTransformationItem = m_pUserConfiguration->SetInt(skCryptDec("Transformation"), skCryptDec("Item"), e.m_iID);
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
            uint32_t m_iID;
            std::string m_szName;
            DisguiseSkill(uint32_t iID, std::string szName) : m_iID(iID), m_szName(szName) {};
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
            for (auto it = mapDisguiseTable->begin(); it != mapDisguiseTable->end(); ++it)
            {
                const auto& kvPair = *it;
                const auto& v = kvPair.second;

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
                const bool bIsSelected = m_pClient->m_iTransformationSkill == e.m_iID;

                if (ImGui::Selectable(e.m_szName.c_str(), bIsSelected))
                {
                    m_pClient->m_iTransformationSkill = m_pUserConfiguration->SetInt(skCryptDec("Transformation"), skCryptDec("Skill"), e.m_iID);
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

        ImGui::PushItemWidth(60);

        if (ImGui::DragInt(skCryptDec("##SearchTargetSpeedValue"), &m_pClient->m_iSearchTargetSpeedValue, 1, 0, 65535))
            m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("SearchTargetSpeedValue"), m_pClient->m_iSearchTargetSpeedValue);

        ImGui::PopItemWidth();

        ImGui::SameLine();

        if (ImGui::Checkbox(skCryptDec("##AttackSpeedCheckbox"), &m_pClient->m_bAttackSpeed))
            m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("AttackSpeed"), m_pClient->m_bAttackSpeed ? 1 : 0);

        ImGui::SameLine();

        ImGui::Text(skCryptDec("Saldiri Hizi (ms)"));

        ImGui::SameLine();

        ImGui::PushItemWidth(60);

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

        ImGui::PushItemWidth(40);

        if (ImGui::DragInt(skCryptDec("##RangeLimitValue"), &m_pClient->m_iRangeLimitValue, 1, 0, 100))
            m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("RangeLimitValue"), m_pClient->m_iRangeLimitValue);

        ImGui::PopItemWidth();

        ImGui::SameLine();

        if (ImGui::Checkbox(skCryptDec("##AttackRangeLimitCheckbox"), &m_pClient->m_bAttackRangeLimit))
            m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("AttackRangeLimit"), m_pClient->m_bAttackRangeLimit ? 1 : 0);

        ImGui::SameLine();

        ImGui::Text(skCryptDec("Saldiri Mesafesi (m)"));

        ImGui::SameLine();

        ImGui::PushItemWidth(40);

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

void Drawing::DrawVIPStorageController()
{
    ImGui::BulletText(skCryptDec("VIP Storage Ayarlari"));
    ImGui::Separator();

    ImGui::Spacing();
    {
        if (ImGui::Checkbox(skCryptDec("##VIPStorageSellSupply"), &m_pClient->m_bVIPSellSupply))
        {

            m_pClient->m_bVIPSellSupply = m_pUserConfiguration->SetInt(skCryptDec("VIPStorage"), skCryptDec("SellSupply"), m_pClient->m_bVIPSellSupply ? 1 : 0);
        }

        ImGui::SameLine();

        ImGui::Text(skCryptDec("Satilacaklari VIP Storage Depola"));
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

        if(ImGui::RadioButton(skCryptDec("Hepsini Topla"), &m_pClient->m_iLootType, 0))
            m_pClient->m_iLootType = m_pUserConfiguration->SetInt(skCryptDec("AutoLoot"), skCryptDec("LootType"), m_pClient->m_iLootType);

        ImGui::SameLine();

        if (ImGui::RadioButton(skCryptDec("Sadece Coin Topla"), &m_pClient->m_iLootType, 1))
            m_pClient->m_iLootType = m_pUserConfiguration->SetInt(skCryptDec("AutoLoot"), skCryptDec("LootType"), m_pClient->m_iLootType);

        ImGui::SameLine();

        if (ImGui::RadioButton(skCryptDec("Item Listesine Gore Topla"), &m_pClient->m_iLootType, 2))
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

        ImGui::RadioButton(skCryptDec("Item Listesi"), &m_iInventoryManagementWindow, 0); 
        ImGui::SameLine();
        ImGui::RadioButton(skCryptDec("Inventory"), &m_iInventoryManagementWindow, 1);

        if (ImGui::Button(skCryptDec("Tum Itemleri Topla"), ImVec2(151.0f, 0.0f)))
        {
            std::map<uint32_t, __TABLE_ITEM>* pItemTable;

            if (Drawing::Bot->GetItemTable(&pItemTable))
            {
                m_pClient->m_vecLootItemList.clear();
                m_pClient->m_vecLootItemList = m_pUserConfiguration->SetInt(skCryptDec("Automation"), skCryptDec("LootItemList"), m_pClient->m_vecLootItemList);

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
                m_pClient->m_vecSellItemList = m_pUserConfiguration->SetInt(skCryptDec("Automation"), skCryptDec("SellItemList"), m_pClient->m_vecSellItemList);

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
                ImGui::TableSetupColumn(skCryptDec("Item"), ImGuiTableColumnFlags_WidthFixed, 200);
                ImGui::TableSetupColumn(skCryptDec("Topla"), ImGuiTableColumnFlags_WidthFixed, 40);
                ImGui::TableSetupColumn(skCryptDec("Sat"), ImGuiTableColumnFlags_WidthFixed, 40);
                ImGui::TableSetupColumn(skCryptDec("Banka"), ImGuiTableColumnFlags_WidthFixed, 40);
                ImGui::TableSetupColumn(skCryptDec("Sil"), ImGuiTableColumnFlags_WidthFixed, 40);
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
                                if (it->second.szName.find(m_szItemSearchName) != std::string::npos)
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
                                && pItemData->szName.find(m_szItemSearchName) == std::string::npos)
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

        if (ImGui::Checkbox(skCryptDec("##AutoTargetCheckbox"), &m_pClient->m_bAutoTarget))
            m_pClient->m_bAutoTarget = m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("AutoTarget"), m_pClient->m_bAutoTarget ? 1 : 0);

        ImGui::SameLine();

        ImGui::Text(skCryptDec("Tum Hedeflere Saldir"));

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
                std::vector<int> toBeRemoved; 

                for (const auto& x : m_pClient->m_vecSelectedNpcIDList)
                {
                    if (ImGui::Selectable(std::to_string(x).c_str(), true))
                    {
                        toBeRemoved.push_back(x);
                    }
                }

                for (const auto& x : toBeRemoved)
                {
                    m_pClient->m_vecSelectedNpcIDList.erase(std::find(m_pClient->m_vecSelectedNpcIDList.begin(), m_pClient->m_vecSelectedNpcIDList.end(), x));
                    m_pClient->m_vecSelectedNpcIDList = m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("NpcIDList"), m_pClient->m_vecSelectedNpcIDList);
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
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.00f, 1.00f, 0.00f, 1.00f));
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
        if (ImGui::BeginCombo(skCryptDec("##SupplyList.SellingGroupList"), m_szSelectedSellingGroupNpc.c_str()))
        {
            struct NpcSellingGroupList
            {
                uint32_t iSellingGroup;
                std::string m_szName;
                NpcSellingGroupList(uint32_t iSellingGroup, std::string szName) : iSellingGroup(iSellingGroup), m_szName(szName) {};
            };

            std::vector<NpcSellingGroupList> vecSellingGroupList;

            vecSellingGroupList.push_back(NpcSellingGroupList(253000, skCryptDec("Potcu")));
            vecSellingGroupList.push_back(NpcSellingGroupList(255000, skCryptDec("Sundries")));
            vecSellingGroupList.push_back(NpcSellingGroupList(267000, skCryptDec("DC Sundries")));

            for (auto& e : vecSellingGroupList)
            {
                const bool is_selected = (m_szSelectedSellingGroupNpc == e.m_szName);

                if (ImGui::Selectable(e.m_szName.c_str(), is_selected))
                {
                    m_szSelectedSellingGroupNpc = e.m_szName;
                    m_iSelectedSellingGroup = e.iSellingGroup;
                }

                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }

            ImGui::EndCombo();
        }

        ImGui::BeginChild(skCryptDec("SupplyListChild"), ImVec2(470, 200), true);
        {
            if (ImGui::BeginTable(skCryptDec("SupplyList.Table"), 3, ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_Borders))
            {
                ImGui::TableSetupColumn(skCryptDec(""), ImGuiTableColumnFlags_WidthFixed, 25);
                ImGui::TableSetupColumn(skCryptDec("Item"), ImGuiTableColumnFlags_WidthFixed, 250);
                ImGui::TableSetupColumn(skCryptDec("Adet"), ImGuiTableColumnFlags_WidthFixed, 100);
                ImGui::TableHeadersRow();

                std::vector<SShopItem> vecShopItemTable;
                if (Drawing::Bot->GetShopItemTable(m_iSelectedSellingGroup, vecShopItemTable))
                {
                    for (size_t i = 0; i < vecShopItemTable.size(); i++)
                    {
                        if (vecShopItemTable[i].m_iItemId == 0)
                            continue;

                        std::map<uint32_t, __TABLE_ITEM>* pItemTable;
                        if (!Drawing::Bot->GetItemTable(&pItemTable))
                            continue;

                        auto iItemData = pItemTable->find(vecShopItemTable[i].m_iItemId);

                        if (iItemData != pItemTable->end())
                        {
                            ImGui::PushID(i);
                            ImGui::TableNextRow();
                            ImGui::TableNextColumn();

                            bool bSelected = std::find(m_pClient->m_vecSupplyList.begin(), m_pClient->m_vecSupplyList.end(), vecShopItemTable[i].m_iItemId) != m_pClient->m_vecSupplyList.end();

                            if (ImGui::Checkbox(skCryptDec("##SupplyList.Table.Enable"), &bSelected)) 
                            {
                                if (bSelected)
                                    m_pClient->m_vecSupplyList.insert(vecShopItemTable[i].m_iItemId);
                                else
                                    m_pClient->m_vecSupplyList.erase(std::find(m_pClient->m_vecSupplyList.begin(), m_pClient->m_vecSupplyList.end(), vecShopItemTable[i].m_iItemId));

                                std::string szSupplyString = "Supply_" + std::to_string(m_iSelectedSellingGroup);
                                m_pClient->m_vecSupplyList = m_pUserConfiguration->SetInt(szSupplyString.c_str(), skCryptDec("Enable"), m_pClient->m_vecSupplyList);

                            }

                            ImGui::TableNextColumn();
                            ImGui::Text("%s", iItemData->second.szName.c_str());
                            ImGui::TableNextColumn();
                            ImGui::PushItemWidth(100);

                            std::string szSupplyString = "Supply_" + std::to_string(m_iSelectedSellingGroup);
                            int iCount = m_pUserConfiguration->GetInt(szSupplyString.c_str(), std::to_string(vecShopItemTable[i].m_iItemId).c_str(), 0);
                            if (ImGui::DragInt(skCryptDec("##SupplyList.Table.Count"), &iCount, 1, 0, 9998))
                            {
                                m_pUserConfiguration->SetInt(szSupplyString.c_str(), std::to_string(vecShopItemTable[i].m_iItemId).c_str(), iCount);
                            }

                            ImGui::PopItemWidth();
                            ImGui::PopID();
                        }
                    }
                }
            }

            ImGui::EndTable();
        }
        ImGui::EndChild();
    }
}


void Drawing::DrawWeaponListController()
{
    ImGui::BulletText(skCryptDec("Silah Listesi"));
    ImGui::Separator();

    ImGui::Spacing();
    {
        if (ImGui::Checkbox(skCryptDec("##AutoRPRChangeWeapon"), &m_pClient->m_bAutoRPRChangeWeapon))
            m_pClient->m_bAutoRPRChangeWeapon = m_pUserConfiguration->SetInt(skCryptDec("Supply"), skCryptDec("AutoRPRChangeWeapon"), m_pClient->m_bAutoRPRChangeWeapon ? 1 : 0);

        ImGui::SameLine();

        ImGui::Text(skCryptDec("RPR geldiginde secili silahlar arasinda degisiklik yap"));

        ImGui::BeginChild(skCryptDec("WeaponListChild"), ImVec2(470, 200), true);
        {
            if (ImGui::BeginTable(skCryptDec("WeaponList.Table"), 3, ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_Borders))
            {
                ImGui::TableSetupColumn(skCryptDec("Item"), ImGuiTableColumnFlags_WidthFixed, 350);
                ImGui::TableSetupColumn(skCryptDec("Sol"), ImGuiTableColumnFlags_WidthFixed, 30);
                ImGui::TableSetupColumn(skCryptDec("Sag"), ImGuiTableColumnFlags_WidthFixed, 30);
                ImGui::TableHeadersRow();

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

                    if (pItemData->byKind != ITEM_CLASS_DAGGER
                        && pItemData->byKind != ITEM_CLASS_SWORD
                        && pItemData->byKind != ITEM_CLASS_SWORD_2H
                        && pItemData->byKind != ITEM_CLASS_AXE
                        && pItemData->byKind != ITEM_CLASS_AXE_2H
                        && pItemData->byKind != ITEM_CLASS_MACE
                        && pItemData->byKind != ITEM_CLASS_MACE_2H
                        && pItemData->byKind != ITEM_CLASS_SPEAR
                        && pItemData->byKind != ITEM_CLASS_POLEARM
                        && pItemData->byKind != ITEM_CLASS_SHIELD
                        && pItemData->byKind != ITEM_CLASS_BOW
                        && pItemData->byKind != ITEM_CLASS_BOW_CROSS
                        && pItemData->byKind != ITEM_CLASS_BOW_LONG
                        && pItemData->byKind != ITEM_CLASS_STAFF
                        && pItemData->byKind != ITEM_CLASS_ARROW
                        && pItemData->byKind != ITEM_CLASS_JAVELIN
                        && pItemData->byKind != ITEM_CLASS_NO_NAME_1)
                        continue;

                    uint32_t iItemBaseID = pItem.iItemID / 1000 * 1000;

                    ImGui::PushID(i++);
                    ImGui::TableNextRow();

                    ImGui::TableNextColumn();
                    ImGui::Text("%s", pItemData->szName.c_str());

                    ImGui::TableNextColumn();

                    bool bSelectedLeft = std::find(m_pClient->m_vecAutoRPRChangeWeaponLeft.begin(), m_pClient->m_vecAutoRPRChangeWeaponLeft.end(), pItem.iPos) != m_pClient->m_vecAutoRPRChangeWeaponLeft.end();
                    bool bSelectedRight = std::find(m_pClient->m_vecAutoRPRChangeWeaponRight.begin(), m_pClient->m_vecAutoRPRChangeWeaponRight.end(), pItem.iPos) != m_pClient->m_vecAutoRPRChangeWeaponRight.end();

                    if (bSelectedRight)
                        ImGui::BeginDisabled();

                    if (ImGui::Checkbox(skCryptDec("##RPRChangeWeaponList.Table.Left"), &bSelectedLeft))
                    {
                        if (bSelectedLeft)
                            m_pClient->m_vecAutoRPRChangeWeaponLeft.insert(pItem.iPos);
                        else
                            m_pClient->m_vecAutoRPRChangeWeaponLeft.erase(std::find(m_pClient->m_vecAutoRPRChangeWeaponLeft.begin(), m_pClient->m_vecAutoRPRChangeWeaponLeft.end(), pItem.iPos));

                        m_pClient->m_vecAutoRPRChangeWeaponLeft = m_pUserConfiguration->SetInt(skCryptDec("Supply"), skCryptDec("AutoRPRChangeWeaponLeft"), m_pClient->m_vecAutoRPRChangeWeaponLeft);
                    }

                    if (bSelectedRight)
                        ImGui::EndDisabled();

                    ImGui::TableNextColumn();

                    if (bSelectedLeft)
                        ImGui::BeginDisabled();

                    if (ImGui::Checkbox(skCryptDec("##RPRChangeWeaponList.Table.Right"), &bSelectedRight))
                    {
                        if (bSelectedRight)
                            m_pClient->m_vecAutoRPRChangeWeaponRight.insert(pItem.iPos);
                        else
                            m_pClient->m_vecAutoRPRChangeWeaponRight.erase(std::find(m_pClient->m_vecAutoRPRChangeWeaponRight.begin(), m_pClient->m_vecAutoRPRChangeWeaponRight.end(), pItem.iPos));

                        m_pClient->m_vecAutoRPRChangeWeaponRight = m_pUserConfiguration->SetInt(skCryptDec("Supply"), skCryptDec("AutoRPRChangeWeaponRight"), m_pClient->m_vecAutoRPRChangeWeaponRight);
                    }

                    if (bSelectedLeft)
                        ImGui::EndDisabled();

                    ImGui::PopID();
                }

                ImGui::EndTable();
            }
            ImGui::EndChild();
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

        if (m_pClient->m_bStartGenieIfUserInRegion)
            ImGui::BeginDisabled();

        if (ImGui::Checkbox(skCryptDec("##SyncWithGenie"), &m_pClient->m_bSyncWithGenie))
            m_pUserConfiguration->SetInt(skCryptDec("Bot"), skCryptDec("SyncWithGenie"), m_pClient->m_bSyncWithGenie ? 1 : 0);

        ImGui::SameLine();

        ImGui::Text(skCryptDec("Genie ile botu esitle"));

        if (m_pClient->m_bStartGenieIfUserInRegion)
            ImGui::EndDisabled();

        if (ImGui::Checkbox(skCryptDec("##StartGenieIfUserInRegion"), &m_pClient->m_bStartGenieIfUserInRegion))
            m_pClient->m_bStartGenieIfUserInRegion = m_pUserConfiguration->SetInt(skCryptDec("Settings"), skCryptDec("StartGenieIfUserInRegion"), m_pClient->m_bStartGenieIfUserInRegion ? 1 : 0);

        ImGui::SameLine();

        ImGui::PushItemWidth(75);

        if (ImGui::DragInt(skCryptDec("##GenieRangeLimit"), &m_pClient->m_iStartGenieIfUserInRegionMeter, 5, 1, 1000000))
            m_pClient->m_iStartGenieIfUserInRegionMeter = m_pUserConfiguration->SetInt(skCryptDec("Settings"), skCryptDec("StartGenieIfUserInRegionMeter"), m_pClient->m_iStartGenieIfUserInRegionMeter);

        ImGui::PopItemWidth();

        ImGui::SameLine();

        ImGui::Text(skCryptDec("birim yakinda oyuncu varsa Genie baslat"));

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

        ImGui::Text(skCryptDec("Exp"));

        ImGui::SameLine();

        ImGui::PushItemWidth(75);

        if (ImGui::DragInt(skCryptDec("##SlotExpLimit"), &m_pClient->m_iSlotExpLimit, 35000, 1, 1000000))
            m_pClient->m_iSlotExpLimit = m_pUserConfiguration->SetInt(skCryptDec("Bot"), skCryptDec("SlotExpLimit"), m_pClient->m_iSlotExpLimit);

        ImGui::PopItemWidth();

        ImGui::SameLine();

        ImGui::Text(skCryptDec("altindaysa illegal vur, aksi durumda legal vur"));
    }
}

void Drawing::SetLegalModeSettings(bool bMode)
{
    m_pClient->m_bLegalMode = m_pUserConfiguration->SetInt(skCryptDec("Automation"), skCryptDec("LegalMode"), bMode);

    if (bMode)
    {
        m_pClient->m_bBasicAttack =  m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("BasicAttack"), 1);
        m_pClient->m_bBasicAttackWithPacket = m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("BasicAttackWithPacket"), 0);
        m_pClient->m_bUseSkillWithPacket = m_pUserConfiguration->SetInt(skCryptDec("Skill"), skCryptDec("UseSkillWithPacket"), 0);
        m_pClient->m_bClosestTarget = m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("ClosestTarget"), 0);
    }
    else
    {
        m_pClient->m_bBasicAttack = m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("BasicAttack"), 1);
        m_pClient->m_bBasicAttackWithPacket = m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("BasicAttackWithPacket"), 1);
        m_pClient->m_bUseSkillWithPacket = m_pUserConfiguration->SetInt(skCryptDec("Skill"), skCryptDec("UseSkillWithPacket"), 1);
        m_pClient->m_bClosestTarget = m_pUserConfiguration->SetInt(skCryptDec("Attack"), skCryptDec("ClosestTarget"), 1);
    }
}

void Drawing::DrawCommonController()
{
    ImGui::BulletText(skCryptDec("Genel Kontroller"));
    ImGui::Separator();

    ImGui::Spacing();
    {
        if (ImGui::Button(skCryptDec("Town At"), ImVec2(132.0f, 0.0f)))
        {
            m_pClient->SendTownPacket();
        }

        ImGui::SameLine();

        if (ImGui::Button(skCryptDec("Oyunu Kapat"), ImVec2(132.0f, 0.0f)))
        {
            TerminateProcess(Drawing::Bot->GetClientProcessHandle(), 0);
        }
    };

    ImGui::Spacing();
    {
        if (ImGui::Button(skCryptDec("Ayarlari Sifirla"), ImVec2(132.0f, 0.0f)))
        {
            m_pClient->ClearUserConfiguration();
        }

        ImGui::SameLine();

        if (ImGui::Button(skCryptDec("Skill Listesini Sifirla"), ImVec2(132.0f, 0.0f)))
        {
            m_pClient->m_vecAttackSkillList = m_pUserConfiguration->SetInt(skCryptDec("Automation"), skCryptDec("AttackSkillList"), std::unordered_set<int>());
            m_pClient->m_vecCharacterSkillList = m_pUserConfiguration->SetInt(skCryptDec("Automation"), skCryptDec("CharacterSkillList"), std::unordered_set<int>());

            m_pClient->LoadSkillData();
        }
    };
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
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.35f, 0.0f, 0.0f, 1.0f));

        if (ImGui::Button(skCryptDec("Illegal Mod"), ImVec2(132.0f, 0.0f)))
        {
            m_pClient->m_bLegalMode = false;
            SetLegalModeSettings(m_pClient->m_bLegalMode);
        }

        ImGui::PopStyleColor(1);

        ImGui::SameLine();

        if (m_pClient->m_bLegalMode)
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.255f, 0.0f, 1.0f));
        else
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.35f, 0.0f, 0.0f, 1.0f));

        if (ImGui::Button(skCryptDec("Legal Mod"), ImVec2(132.0f, 0.0f)))
        {
            m_pClient->m_bLegalMode = true;
            SetLegalModeSettings(m_pClient->m_bLegalMode);
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
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.35f, 0.0f, 0.0f, 1.0f));
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
                uint8_t iZoneID = m_pClient->GetRepresentZone(m_pClient->GetZone());
                m_pClient->SaveRoute(m_szPlannedRouteName, iZoneID, m_pClient->m_vecRoutePlan);
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

        ImGui::BeginChild(skCryptDec("RoutePlannerChild"), ImVec2(470, 180), true);
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

            ImGui::BeginChild(skCryptDec("RoutePlannerStepChild"), ImVec2(470, 70), true);
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

                ImGui::SameLine();

                if (ImGui::Button(skCryptDec("DC Sundries Noktasi"), ImVec2(145.0f, 0.0f)))
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

    ImGui::BeginChild(skCryptDec("RouteTable"), ImVec2(470, 200), true);
    {
        if (ImGui::BeginTable(skCryptDec("RouteList.Table"), 4, ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_Borders))
        {
            ImGui::TableSetupColumn(skCryptDec("Rota"), ImGuiTableColumnFlags_WidthFixed, 250);
            ImGui::TableSetupColumn(skCryptDec("Tedarik"), ImGuiTableColumnFlags_WidthFixed, 60);
            ImGui::TableSetupColumn(skCryptDec("Death"), ImGuiTableColumnFlags_WidthFixed, 60);
            ImGui::TableSetupColumn(skCryptDec("Login"), ImGuiTableColumnFlags_WidthFixed, 60);
            ImGui::TableHeadersRow();

            uint8_t iZoneID = m_pClient->GetRepresentZone(m_pClient->GetZone());

            ClientHandler::RouteList pRouteList;

            if (m_pClient->GetRouteList(iZoneID, pRouteList))
            {
                int i = 0;
                for (auto& e : pRouteList)
                {
                    ImGui::PushID(i++);
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::Text("%s", e.first.c_str());
                    ImGui::TableNextColumn();

                    bool bSupplySelected = std::find(m_pClient->m_setSelectedSupplyRouteList.begin(), m_pClient->m_setSelectedSupplyRouteList.end(), e.first) != m_pClient->m_setSelectedSupplyRouteList.end();

                    if (ImGui::Checkbox(skCryptDec("##SupplyRouteCheckbox"), &bSupplySelected))
                    {
                        if (bSupplySelected)
                            m_pClient->m_setSelectedSupplyRouteList.insert(e.first.c_str());
                        else
                            m_pClient->m_setSelectedSupplyRouteList.erase(std::find(m_pClient->m_setSelectedSupplyRouteList.begin(), m_pClient->m_setSelectedSupplyRouteList.end(), e.first));

                        m_pClient->m_setSelectedSupplyRouteList = m_pUserConfiguration->SetString(skCryptDec("Bot"), skCryptDec("SelectedSupplyRouteList"), m_pClient->m_setSelectedSupplyRouteList);
                    }

                    ImGui::TableNextColumn();

                    bool bDeathSelected = std::find(m_pClient->m_setSelectedDeathRouteList.begin(), m_pClient->m_setSelectedDeathRouteList.end(), e.first) != m_pClient->m_setSelectedDeathRouteList.end();

                    if (ImGui::Checkbox(skCryptDec("##DeathRouteCheckbox"), &bDeathSelected))
                    {
                        if (bDeathSelected)
                            m_pClient->m_setSelectedDeathRouteList.insert(e.first.c_str());
                        else
                            m_pClient->m_setSelectedDeathRouteList.erase(std::find(m_pClient->m_setSelectedDeathRouteList.begin(), m_pClient->m_setSelectedDeathRouteList.end(), e.first));

                        m_pClient->m_setSelectedDeathRouteList = m_pUserConfiguration->SetString(skCryptDec("Bot"), skCryptDec("SelectedDeathRouteList"), m_pClient->m_setSelectedDeathRouteList);
                    }

                    ImGui::TableNextColumn();

                    bool bLoginSelected = std::find(m_pClient->m_setSelectedLoginRouteList.begin(), m_pClient->m_setSelectedLoginRouteList.end(), e.first) != m_pClient->m_setSelectedLoginRouteList.end();

                    if (ImGui::Checkbox(skCryptDec("##LoginRouteCheckbox"), &bLoginSelected))
                    {
                        if (bLoginSelected)
                            m_pClient->m_setSelectedLoginRouteList.insert(e.first.c_str());
                        else
                            m_pClient->m_setSelectedLoginRouteList.erase(std::find(m_pClient->m_setSelectedLoginRouteList.begin(), m_pClient->m_setSelectedLoginRouteList.end(), e.first));

                        m_pClient->m_setSelectedLoginRouteList = m_pUserConfiguration->SetString(skCryptDec("Bot"), skCryptDec("SelectedLoginRouteList"), m_pClient->m_setSelectedLoginRouteList);
                    }

                    ImGui::PopID();
                }
            }

            ImGui::EndTable();
        }
    }

    ImGui::EndChild();
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
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.00f, 1.00f, 0.00f, 1.00f));
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
