#include "pch.h"
#include "UI.h"
#include "Drawing.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <D3dx9tex.h>
#pragma comment(lib, "D3dx9")
#pragma comment(lib, "D3d9")

LPDIRECT3DDEVICE9 UI::pD3DDevice = nullptr;
LPDIRECT3D9 UI::pD3D = nullptr;
D3DPRESENT_PARAMETERS UI::D3Dpp = {};
UINT UI::g_ResizeHeight = 0;
UINT UI::g_ResizeWidth = 0;
bool UI::bInit = false;
HWND UI::hTargetWindow = nullptr;
BOOL UI::bTargetSet = FALSE;
DWORD UI::dTargetPID = 0;

HMODULE UI::hCurrentModule = nullptr;

DWORD g_iLastFrameTime = 0;
DWORD g_iFPSLimit = 30;

/**
    @brief : Function that create a D3D9 device.
    @param  hWnd : HWND of the created window.
    @retval : true if the function succeed else false.
**/
bool UI::CreateDeviceD3D(const HWND hWnd)
{
    if ((pD3D = Direct3DCreate9(D3D_SDK_VERSION)) == nullptr)
        return false;

    // Create the D3DDevice
    ZeroMemory(&D3Dpp, sizeof(D3Dpp));
    D3Dpp.Windowed = TRUE;
    D3Dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    D3Dpp.BackBufferFormat = D3DFMT_A8R8G8B8;
    D3Dpp.EnableAutoDepthStencil = TRUE;
    D3Dpp.AutoDepthStencilFormat = D3DFMT_D16;
    D3Dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
    if (pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &D3Dpp, &pD3DDevice) < 0)
        return false;

    return true;
}

#ifndef WM_DPICHANGED
#define WM_DPICHANGED 0x02E0 // From Windows SDK 8.1+ headers
#endif

/**
    @brief : Window message handler (https://learn.microsoft.com/en-us/windows/win32/api/winuser/nc-winuser-wndproc).
**/
LRESULT WINAPI UI::WndProc(const HWND hWnd, const UINT msg, const WPARAM wParam, const LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED)
            return 0;
        g_ResizeWidth = (UINT)LOWORD(lParam); // Queue resize
        g_ResizeHeight = (UINT)HIWORD(lParam);
        return 0;

    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU)
            return 0;
        break;

    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;

    default:
        break;
    }
    return ::DefWindowProc(hWnd, msg, wParam, lParam);
}

/**
    @brief : Function that create the overlay window and more.
**/
void UI::Render(Bot* pBot)
{
    ImGui_ImplWin32_EnableDpiAwareness();

    // Get the main window of the process when overlay as DLL
#ifdef _WINDLL
    if (hTargetWindow == nullptr)
        GetWindow();
#endif

    WNDCLASSEX wc;

    wc.cbClsExtra = NULL;
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.cbWndExtra = NULL;
    wc.hbrBackground = (HBRUSH)CreateSolidBrush(RGB(0, 0, 0));
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wc.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);
    wc.hInstance = GetModuleHandle(nullptr);
    wc.lpfnWndProc = WndProc;
    wc.lpszClassName = Drawing::lpWindowName;
    wc.lpszMenuName = nullptr;
    wc.style = CS_VREDRAW | CS_HREDRAW;

    ::RegisterClassEx(&wc);
    const HWND hWnd = ::CreateWindowExA(WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_NOACTIVATE, wc.lpszClassName, Drawing::lpWindowName, WS_POPUP, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), nullptr, nullptr, wc.hInstance, nullptr);

    SetLayeredWindowAttributes(hWnd, RGB(0, 0, 0), 0, LWA_COLORKEY);
    const MARGINS margin = { -1, -1, -1, -1 };
    DwmExtendFrameIntoClientArea(hWnd, &margin);

    if (!CreateDeviceD3D(hWnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClass(wc.lpszClassName, wc.hInstance);
        return;
    }

    int iWindowState = SW_SHOWDEFAULT;

    //SetWindowDisplayAffinity(hWnd, WDA_EXCLUDEFROMCAPTURE);

    ::ShowWindow(hWnd, iWindowState);
    ::UpdateWindow(hWnd);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    io.Framerate = 30;
    
    //ImGui::StyleColorsDark();
    StyleColorsHazar();

    io.Fonts->AddFontFromFileTTF(".\\data\\SF-Pro-Display-Medium.otf", 16.0f);

    ImGui::GetIO().IniFilename = nullptr;

    ImGui_ImplWin32_Init(hWnd);
    ImGui_ImplDX9_Init(pD3DDevice);

    const ImVec4 clear_color = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);

    bInit = true;

    Drawing::Bot = pBot;

    bool bDone = false;

    while (!bDone)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        if (hTargetWindow != nullptr && GetAsyncKeyState(VK_INSERT) & 1)
            Drawing::bDraw = !Drawing::bDraw;

        if (Drawing::Bot->IsClosed())
            break;

        if (Drawing::Bot->GetInjectedProcessId() != 0 && Drawing::Bot->IsInjectedProcessLost())
            break;

        Drawing::Bot->Process();

        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                bDone = true;
        }

        // Check if the targeted window is still up.
        if (!IsWindowAlive() && bTargetSet)
            bDone = true;

        if (bDone)
            break;

        if (!Drawing::bDraw && hTargetWindow == nullptr)
            break;

        DWORD iCurrentTime = timeGetTime();
        if ((iCurrentTime - g_iLastFrameTime) < (1000 / g_iFPSLimit))
        {
            continue;
        }

        if (g_ResizeWidth != 0 && g_ResizeHeight != 0)
        {
            D3Dpp.BackBufferWidth = g_ResizeWidth;
            D3Dpp.BackBufferHeight = g_ResizeHeight;
            g_ResizeWidth = g_ResizeHeight = 0;
            ResetDevice();
        }

        // Move the window on top of the targeted window and handle resize.
#ifdef _WINDLL 
        if (hTargetWindow != nullptr)
            MoveWindow(hWnd);
        else
            continue;
#else
        if (hTargetWindow != nullptr && bTargetSet)
            MoveWindow(hWnd);
#endif

        // Clear overlay when the targeted window is not focus
        if (!IsWindowFocus(hWnd) && bTargetSet)
        {
            pD3DDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
            pD3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
            pD3DDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
            const D3DCOLOR clear_col_dx = D3DCOLOR_RGBA((int)(clear_color.x * clear_color.w * 255.0f), (int)(clear_color.y * clear_color.w * 255.0f), (int)(clear_color.z * clear_color.w * 255.0f), (int)(clear_color.w * 255.0f));
            pD3DDevice->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, clear_col_dx, 1.0f, 0);

            if (pD3DDevice->BeginScene() >= 0)
                pD3DDevice->EndScene();

            const HRESULT result = pD3DDevice->Present(nullptr, nullptr, nullptr, nullptr);

            // Handle loss of D3D9 device
            if (result == D3DERR_DEVICELOST && pD3DDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
                ResetDevice();
        }
        else
        {
            ImGui_ImplDX9_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();
            {
                Drawing::Draw();
            }
            ImGui::EndFrame();

            // Overlay handle inputs when menu is showed.
            if (Drawing::isActive())
                SetWindowLong(hWnd, GWL_EXSTYLE, WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TOOLWINDOW);
            else
                SetWindowLong(hWnd, GWL_EXSTYLE, WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOOLWINDOW);

            ImGui::Render();
            ImGui::EndFrame();

            pD3DDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
            pD3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
            pD3DDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
            const D3DCOLOR clear_col_dx = D3DCOLOR_RGBA((int)(clear_color.x * clear_color.w * 255.0f), (int)(clear_color.y * clear_color.w * 255.0f), (int)(clear_color.z * clear_color.w * 255.0f), (int)(clear_color.w * 255.0f));
            pD3DDevice->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, clear_col_dx, 1.0f, 0);

            // Draw figure with DirectX
            //if (IsWindowTargeted())
            //    Drawing::DXDraw(pD3DDevice);

            if (pD3DDevice->BeginScene() >= 0)
            {
                ImGui::Render();
                ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
                pD3DDevice->EndScene();
            }

            const HRESULT result = pD3DDevice->Present(nullptr, nullptr, nullptr, nullptr);

            // Handle loss of D3D9 device
            if (result == D3DERR_DEVICELOST && pD3DDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
                ResetDevice();
        }

        g_iLastFrameTime = iCurrentTime;
    }

    bInit = false;

    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hWnd);
    ::UnregisterClass(wc.lpszClassName, wc.hInstance);

#ifdef _WINDLL
    CreateThread(nullptr, NULL, (LPTHREAD_START_ROUTINE)FreeLibrary, hCurrentModule, NULL, nullptr);
#else
    TerminateProcess(GetModuleHandleA(nullptr), 0);
#endif
}

/**
    @brief : Reset the current D3D9 device.
**/
void UI::ResetDevice()
{
    ImGui_ImplDX9_InvalidateDeviceObjects();
    const HRESULT hr = pD3DDevice->Reset(&D3Dpp);
    if (hr == D3DERR_INVALIDCALL)
        IM_ASSERT(0);
    ImGui_ImplDX9_CreateDeviceObjects();
}

/**
    @brief : Release the D3D9 device and object.
**/
void UI::CleanupDeviceD3D()
{
    if (pD3DDevice) { pD3DDevice->Release(); pD3DDevice = nullptr; }
    if (pD3D) { pD3D->Release(); pD3D = nullptr; }
}


/**
    @brief : Function that retrieve the main window of the process.
             This function is only called when the overlay is build as DLL.
**/
void UI::GetWindow()
{
    EnumWindows((WNDENUMPROC)EnumWind, NULL);
}

/**
    @brief : Callback function that retrive the main window of the process.
             This function is only called when the overlay is build as DLL.
             (https://learn.microsoft.com/fr-fr/windows/win32/api/winuser/nf-winuser-enumwindows)

**/
BOOL CALLBACK UI::EnumWind(const HWND hWindow, const LPARAM lPrams)
{
    DWORD procID;
    GetWindowThreadProcessId(hWindow, &procID);
    if (GetCurrentProcessId() != procID)
        return TRUE;

    if (!IsWindowValid(hWindow))
        return TRUE;

    SetTargetWindow(hWindow);
    return FALSE;
}

/**
    @brief : Function that move the overlay on top of the targeted window.
    @param hCurrentProcessWindow : Window of the overlay.
**/
void UI::MoveWindow(const HWND hCurrentProcessWindow)
{
    RECT rect;
    if (hTargetWindow == nullptr)
        return;

    GetWindowRect(hTargetWindow, &rect);

    int lWindowWidth = rect.right - rect.left;
    int lWindowHeight = rect.bottom - rect.top;

    lWindowWidth -= 5;
    lWindowHeight -= 1;

    SetWindowPos(hCurrentProcessWindow, nullptr, rect.left, rect.top, lWindowWidth, lWindowHeight, SWP_SHOWWINDOW);
}

/**
    @brief : Function that check if the overlay window or the targeted window is focus.
    @param  hCurrentProcessWindow : Window of the overlay.
    @retval : TRUE if one of the window is focus else FALSE.
**/
BOOL UI::IsWindowFocus(const HWND hCurrentProcessWindow)
{
    char lpCurrentWindowUsedClass[125];
    char lpCurrentWindowClass[125];
    char lpOverlayWindowClass[125];

    const HWND hCurrentWindowUsed = GetForegroundWindow();
    if (GetClassNameA(hCurrentWindowUsed, lpCurrentWindowUsedClass, sizeof(lpCurrentWindowUsedClass)) == 0)
        return FALSE;

    if (GetClassNameA(hTargetWindow, lpCurrentWindowClass, sizeof(lpCurrentWindowClass)) == 0)
        return FALSE;

    if (GetClassNameA(hCurrentProcessWindow, lpOverlayWindowClass, sizeof(lpOverlayWindowClass)) == 0)
        return FALSE;

    if (strcmp(lpCurrentWindowUsedClass, lpCurrentWindowClass) != 0 && strcmp(lpCurrentWindowUsedClass, lpOverlayWindowClass) != 0)
    {
        SetWindowLong(hCurrentProcessWindow, GWL_EXSTYLE, WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOOLWINDOW);
        return FALSE;
    }

    return TRUE;
}

/**
    @brief : function that check if a target window is set.
    @retval : TRUE if a target window has been setted else FALSE.
**/
BOOL UI::IsWindowTargeted()
{
    return bTargetSet;
}

/**
    @brief : Function that clear the current window list and enumerate all windows.
    @param vWindowList : pointer of the WindowItem vector.
**/
void UI::GetAllWindow(std::vector<WindowItem>* vWindowList)
{
    vWindowList->clear();
    EnumWindows((WNDENUMPROC)EnumAllWind, (LPARAM)vWindowList);
}

/**
    @brief : Callback function that retrive all the valid window and get processname, pid and window title.
             (https://learn.microsoft.com/fr-fr/windows/win32/api/winuser/nf-winuser-enumwindows)
**/
BOOL CALLBACK UI::EnumAllWind(const HWND hWindow, const LPARAM lPrams)
{
    if (!IsWindowValid(hWindow))
        return TRUE;

    WindowItem CurrentWindowItem = { hWindow, 0, 0 };
    DWORD procID;

    GetWindowTextA(hWindow, CurrentWindowItem.CurrentWindowTitle, sizeof(CurrentWindowItem.CurrentWindowTitle));

    if (strlen(CurrentWindowItem.CurrentWindowTitle) == 0)
        return TRUE;

    GetWindowThreadProcessId(hWindow, &procID);
    GetProcessName(CurrentWindowItem.CurrentProcessName, procID);

    const auto vWindowList = (std::vector<WindowItem>*)lPrams;

    vWindowList->push_back(CurrentWindowItem);

    return TRUE;
}


/**
    @brief : Function that retrieve the process name from the PID.
    @param lpProcessName : pointer to the string that store the process name.
    @param dPID : PID of the process.
**/
void UI::GetProcessName(LPSTR lpProcessName, DWORD dPID)
{
    char lpCurrentProcessName[MAX_PATH]; // Assuming MAX_PATH is sufficient for the process name

    PROCESSENTRY32 ProcList{};
    ProcList.dwSize = sizeof(ProcList);

    const HANDLE hProcList = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcList == INVALID_HANDLE_VALUE)
        return;

    if (!Process32First(hProcList, &ProcList))
    {
        CloseHandle(hProcList);
        return;
    }

    do
    {
        if (ProcList.th32ProcessID == dPID)
        {
            strcpy_s(lpCurrentProcessName, sizeof(lpCurrentProcessName), ProcList.szExeFile);
            strcpy_s(lpProcessName, strlen(lpCurrentProcessName) + 1, lpCurrentProcessName);
            CloseHandle(hProcList);
            return;
        }

    } while (Process32Next(hProcList, &ProcList));

    CloseHandle(hProcList);
}

/**
    @brief : Function that check if a window is valid.
    @param  hCurrentWindow : window to be tested.
    @retval : TRUE if the window is valid else FALSE.
**/
BOOL UI::IsWindowValid(const HWND hCurrentWindow)
{
    DWORD styles, ex_styles;
    RECT rect;

    if (!IsWindowVisible(hCurrentWindow) ||
        (IsIconic(hCurrentWindow) || IsWindowCloaked(hCurrentWindow)))
        return FALSE;

    GetClientRect(hCurrentWindow, &rect);
    styles = (DWORD)GetWindowLongPtr(hCurrentWindow, GWL_STYLE);
    ex_styles = (DWORD)GetWindowLongPtr(hCurrentWindow, GWL_EXSTYLE);

    if (ex_styles & WS_EX_TOOLWINDOW)
        return FALSE;
    if (styles & WS_CHILD)
        return FALSE;
    if (rect.bottom == 0 || rect.right == 0)
        return FALSE;

    return TRUE;
}

/**
    @brief : Function that check if a window is cloacked.
    @param  hCurrentWindow : window to be tested.
    @retval : TRUE if the window is cloacked else FALSE.
**/
BOOL UI::IsWindowCloaked(const HWND hCurrentWindow)
{
    DWORD cloaked;
    const HRESULT hr = DwmGetWindowAttribute(hCurrentWindow, DWMWA_CLOAKED, &cloaked,
        sizeof(cloaked));
    return SUCCEEDED(hr) && cloaked;
}

/**
    @brief : Setter function used to define the target window from the window picker.
             This is used only when the overlay is build as an EXE.
    @param hWindow : target window.
**/
void UI::SetTargetWindow(const HWND hWindow)
{
    hTargetWindow = hWindow;
    SetForegroundWindow(hTargetWindow);
    GetWindowThreadProcessId(hTargetWindow, &dTargetPID);
    bTargetSet = TRUE;
}

/**
    @brief : Function that look if the targeted window has been closed.
    @retval : TRUE if the function is still up else FALSE.
**/
BOOL UI::IsWindowAlive()
{
    DWORD dCurrentPID;

    if (hTargetWindow == nullptr)
        return FALSE;

    if (!IsWindow(hTargetWindow))
        return FALSE;

    GetWindowThreadProcessId(hTargetWindow, &dCurrentPID);

    if (dCurrentPID != dTargetPID)
        return FALSE;

    return TRUE;
}

HWND UI::GetProcessWindowHandle(DWORD targetProcessId) 
{
    HWND result = nullptr;

    EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL 
    {
        DWORD processId;
        GetWindowThreadProcessId(hwnd, &processId);

        if (processId == static_cast<DWORD>(lParam)) 
        {
            *reinterpret_cast<HWND*>(lParam) = hwnd;
            return FALSE;
        }

        return TRUE;
    }, reinterpret_cast<LPARAM>(&result));

    return result;
}

void UI::StyleColorsHazar()
{
    ImGuiStyle& style = ImGui::GetStyle();

    style.Colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.13f, 0.14f, 0.15f, 1.00f);
    style.Colors[ImGuiCol_ChildBg] = ImVec4(0.13f, 0.14f, 0.15f, 1.00f);
    style.Colors[ImGuiCol_PopupBg] = ImVec4(0.13f, 0.14f, 0.15f, 1.00f);
    style.Colors[ImGuiCol_Border] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
    style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.38f, 0.38f, 0.38f, 1.00f);
    style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.67f, 0.67f, 0.67f, 0.39f);
    style.Colors[ImGuiCol_TitleBg] = ImVec4(0.08f, 0.08f, 0.09f, 1.00f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.08f, 0.08f, 0.09f, 1.00f);
    style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
    style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
    style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
    style.Colors[ImGuiCol_CheckMark] = ImVec4(0.11f, 0.64f, 0.92f, 1.00f);
    style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.11f, 0.64f, 0.92f, 1.00f);
    style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.08f, 0.50f, 0.72f, 1.00f);
    style.Colors[ImGuiCol_Button] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.38f, 0.38f, 0.38f, 1.00f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.67f, 0.67f, 0.67f, 0.39f);
    style.Colors[ImGuiCol_Header] = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
    style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.67f, 0.67f, 0.67f, 0.39f);
    style.Colors[ImGuiCol_Separator] = style.Colors[ImGuiCol_Border];
    style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.41f, 0.42f, 0.44f, 1.00f);
    style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
    style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.29f, 0.30f, 0.31f, 0.67f);
    style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
    style.Colors[ImGuiCol_Tab] = ImVec4(0.08f, 0.08f, 0.09f, 0.83f);
    style.Colors[ImGuiCol_TabHovered] = ImVec4(0.33f, 0.34f, 0.36f, 0.83f);
    style.Colors[ImGuiCol_TabActive] = ImVec4(0.23f, 0.23f, 0.24f, 1.00f);
    style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.08f, 0.08f, 0.09f, 1.00f);
    style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.13f, 0.14f, 0.15f, 1.00f);
    style.Colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
    style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
    style.Colors[ImGuiCol_DragDropTarget] = ImVec4(0.11f, 0.64f, 0.92f, 1.00f);
    style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);

    style.GrabRounding = style.FrameRounding = 2.3f;
}