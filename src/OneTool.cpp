/*********************************************************************
 * \file   OneTool.cpp
 * \brief  一个小工具 含有全局热键......
 *
 * \author POMIN
 * \date   January 2022
 *********************************************************************/
#include "OneTool.hpp"
#include "file_helper.hpp"

#pragma region D3D_var

ID3D11Device* g_pd3dDevice = NULL;
IDXGISwapChain* g_pSwapChain = NULL;
ID3D11DeviceContext* g_pd3dDeviceContext = NULL;
ID3D11RenderTargetView* g_mainRenderTargetView = NULL;

#pragma endregion

#pragma region D3D_Function

bool CreateDeviceD3D (HWND hWnd);
void CleanupDeviceD3D ();
void CreateRenderTarget ();
void CleanupRenderTarget ();

bool CreateDeviceD3D (HWND hWnd) {
    RECT rcWnd;
    GetWindowRect (hWnd, &rcWnd);
    // Setup swap chain
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory (&sd, sizeof (sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.Width = rcWnd.right - rcWnd.left;
    sd.BufferDesc.Height = rcWnd.bottom - rcWnd.top;
    sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH | DXGI_SWAP_CHAIN_FLAG_GDI_COMPATIBLE;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    //createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    if (D3D11CreateDeviceAndSwapChain (NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext) != S_OK)
        return false;

    CreateRenderTarget ();
    return true;
}
void CleanupDeviceD3D () {
    CleanupRenderTarget ();
    if (g_pSwapChain) { g_pSwapChain->Release (); g_pSwapChain = NULL; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release (); g_pd3dDeviceContext = NULL; }
    if (g_pd3dDevice) { g_pd3dDevice->Release (); g_pd3dDevice = NULL; }
}
void CreateRenderTarget () {
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer (0, IID_PPV_ARGS (&pBackBuffer));
    if (pBackBuffer != 0)
        g_pd3dDevice->CreateRenderTargetView (pBackBuffer, NULL, &g_mainRenderTargetView);
    pBackBuffer->Release ();
}
void CleanupRenderTarget () {
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release (); g_mainRenderTargetView = NULL; }
}
#pragma endregion

#pragma region Win_Fun

/* 窗口回调 */
LRESULT WINAPI WndProc (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WINAPI WndProc (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler (hWnd, msg, wParam, lParam))
        return true;

    switch (msg) {
    case WM_GETMINMAXINFO:
        /* 最小宽高 */
        MINMAXINFO* mminfo;
        mminfo = (PMINMAXINFO)lParam;

        mminfo->ptMinTrackSize.x = 800;
        mminfo->ptMinTrackSize.y = 500;

        mminfo->ptMaxPosition.x = 0;
        mminfo->ptMaxPosition.y = 0;

        return 0;
    case WM_SIZE:
        if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED) {
            CleanupRenderTarget ();
            g_pSwapChain->ResizeBuffers (0, (UINT)LOWORD (lParam), (UINT)HIWORD (lParam), DXGI_FORMAT_UNKNOWN, 0);
            CreateRenderTarget ();
        }
        win_size.x = (float)LOWORD (lParam);
        win_size.y = (float)HIWORD (lParam);
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU)
            return 0;
        break;
    case WM_CLOSE:
        /* 关闭窗口时仅是隐藏 */
        ShowWindow (glo_hwnd, HIDE_WINDOW);
        return 0;
    case WM_DESTROY:
        ::PostQuitMessage (0);
        return 0;
    }
    return ::DefWindowProc (hWnd, msg, wParam, lParam);
}
#pragma endregion

/**
 * \brief 程序进入函数
 *
 * \param hInstance
 * \param hPrevInstance
 * \param lpCmdLine
 * \param nCmdShow
 * \return
 */
int APIENTRY wWinMain (_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow) {

    if (FindWindow (L"OneTool - by pomin", L"OneTool - by pomin")) {
        DEBUG_OUT ("已经打开过一个进程");
        MessageBox (glo_hwnd, L"已经打开过一个进程", L"警告", MB_ICONERROR);
        exit (1);
    }

    /* 注册窗口类 */
    WNDCLASSEX wc = {
        sizeof (WNDCLASSEX),
        CS_CLASSDC,
        WndProc,
        0L, 0L,
        hInstance,
        LoadIcon (hInstance, MAKEINTRESOURCE (IDI_ICON1)),
        LoadCursor (nullptr, IDC_ARROW),
        (HBRUSH)(COLOR_WINDOW + 1),
        nullptr,
        _T ("OneTool - by pomin"),
        LoadIcon (wc.hInstance, MAKEINTRESOURCE (IDI_APPLICATION))
    };
    ::RegisterClassEx (&wc);

    /* 窗口创建 */
    int win_w = GetSystemMetrics (SM_CXFULLSCREEN);
    int win_h = GetSystemMetrics (SM_CYFULLSCREEN);
    HWND hwnd = ::CreateWindow (
        wc.lpszClassName,
        _T ("OneTool - by pomin"),
        WS_EX_TOPMOST | WS_OVERLAPPEDWINDOW,
        win_w * 0.2f, win_h * 0.15f, win_w * 0.6f, win_h * 0.7f,
        NULL, NULL, wc.hInstance, NULL
    );
    /* 注释部分为无 Windows 边框样式 */
    //HWND hwnd = ::CreateWindowEx (
    //    WS_EX_LAYERED,
    //    wc.lpszClassName,
    //    _T ("OneTool - by pomin"),
    //    WS_POPUP,
    //    0, 0, GetSystemMetrics (SM_CXSCREEN), GetSystemMetrics (SM_CYSCREEN) - 2,
    //    NULL, NULL, wc.hInstance, NULL
    //);
    glo_hwnd = hwnd;

    if (!CreateDeviceD3D (hwnd)) {
        CleanupDeviceD3D ();
        ::UnregisterClass (wc.lpszClassName, wc.hInstance);
        return 1;
    }
    ::ShowWindow (hwnd, HIDE_WINDOW);
    ::UpdateWindow (hwnd);

    /* imgui 开始载入 */
    ImGui::CreateContext ();
    ImGuiIO& io = ImGui::GetIO ();
    (void)io;
    ImGui::StyleColorsDark ();
    ImGui_ImplWin32_Init (hwnd);
    ImGui_ImplDX11_Init (g_pd3dDevice, g_pd3dDeviceContext);

    /* 获取程序路径 ( 绝对路径 ) */
    char* buff = new char [1024];
    GetModuleFileNameA (NULL, buff, 1024);
    PathRemoveFileSpecA (buff);
    exe_path += buff;
    DEBUG_OUT (exe_path.c_str ());
    delete [] buff;

    /* 加载必须资源 */
    string need_res_path[] = {
        "/font/msyh.ttc",
        FONT_ICON_FILE_NAME_FAS,
        FONT_ICON_FILE_NAME_FK,
        "/pic/cat.png",
    };

    for (size_t i = 0; i < sizeof(need_res_path) / sizeof (string); i++) {
        if (!PathFileExistsA (GET_FILE_PATH (need_res_path[i]))) {
            MessageBoxA (glo_hwnd, ("资源缺失: " + need_res_path[i]).c_str (), "", MB_ICONERROR);
            exit (1);
        }
    }

    /* 一般字体 微软雅黑 */
    io.Fonts->AddFontFromFileTTF (GET_FILE_PATH("/font/msyh.ttc"), 32.0f, NULL, io.Fonts->GetGlyphRangesChineseSimplifiedCommon ());

    /* 图标字体 */
    static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
    static const ImWchar icons_ranges_fk[] = { ICON_MIN_FK, ICON_MAX_FK, 0 };
    ImFontConfig icons_config;
    icons_config.MergeMode = true;
    icons_config.PixelSnapH = true;
    io.Fonts->AddFontFromFileTTF (GET_FILE_PATH (FONT_ICON_FILE_NAME_FAS), 28.0f, &icons_config, icons_ranges);
    io.Fonts->AddFontFromFileTTF (GET_FILE_PATH (FONT_ICON_FILE_NAME_FK), 28.0f, &icons_config, icons_ranges_fk);

    MSG msg;
    ZeroMemory (&msg, sizeof (msg));

    while (msg.message != WM_QUIT) {
        if (::PeekMessage (&msg, NULL, 0U, 0U, PM_REMOVE)) {
            ::TranslateMessage (&msg);
            ::DispatchMessage (&msg);
            continue;
        }

#pragma region ImGui_Init

        ImGui_ImplDX11_NewFrame ();
        ImGui_ImplWin32_NewFrame ();
        ImGui::NewFrame ();

#pragma endregion

        /* 窗口循环 */
        main_window_client ();

        /*bool a = true;
        ImGui::ShowDemoWindow (&a);*/

#pragma region ImGui_Refresh

        float clear_color[] = { 0, 0, 0, 0 };
        ImGui::Render ();
        g_pd3dDeviceContext->OMSetRenderTargets (1, &g_mainRenderTargetView, NULL);
        g_pd3dDeviceContext->ClearRenderTargetView (g_mainRenderTargetView, (float*)&clear_color);
        ImGui_ImplDX11_RenderDrawData (ImGui::GetDrawData ());
        g_pSwapChain->Present (1, 0);

        /* 注释部分为无 Windows 边框样式 */
        //ImGui::Render ();
        //g_pd3dDeviceContext->OMSetRenderTargets (1, &g_mainRenderTargetView, NULL);
        //g_pd3dDeviceContext->ClearRenderTargetView (g_mainRenderTargetView, clear_color);
        //ImGui_ImplDX11_RenderDrawData (ImGui::GetDrawData ());
        ////g_pSwapChain->Present (1, 0);

        //BLENDFUNCTION blend = { AC_SRC_OVER, 0, 0, AC_SRC_ALPHA };
        //POINT pt = { 0, 0 };
        //SIZE sz = { 0, 0 };
        //IDXGISurface1* pSurface = NULL;
        //HDC hDC = NULL;
        //g_pSwapChain->Present (1, 0); // Present with vsync
        ////g_pSwapChain->Present(0, 0); // Present without vsync
        //g_pSwapChain->GetBuffer (0, IID_PPV_ARGS (&pSurface));

        //DXGI_SURFACE_DESC desc;
        //pSurface->GetDesc (&desc);
        //sz.cx = desc.Width;
        //sz.cy = desc.Height;

        //pSurface->GetDC (FALSE, &hDC);
        //::UpdateLayeredWindow (hwnd, nullptr, nullptr, &sz, hDC, &pt, 0, &blend, ULW_COLORKEY);
        //pSurface->ReleaseDC (nullptr);
        //pSurface->Release ();
#pragma endregion
    }
#pragma region D3D_End

    ImGui_ImplDX11_Shutdown ();
    ImGui_ImplWin32_Shutdown ();
    ImGui::DestroyContext ();
    CleanupDeviceD3D ();
    ::DestroyWindow (hwnd);
    ::UnregisterClass (wc.lpszClassName, wc.hInstance);

#pragma endregion
    return 0;
}
