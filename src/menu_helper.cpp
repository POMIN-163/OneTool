#include "menu_helper.hpp"
#include "file_helper.hpp"

#define WM_TO_TRAY 0xf9
#define ID_APP_EXIT 88
#define ID_APP_DISP 66
#define ID_APP_DIR  55
#define ID_APP_RUN  44

HMENU menu;
POINT menu_pos = { 0, 0 };
HWND glo_menu_hwnd;
NOTIFYICONDATA nid;

hotkey_obj* hotkey;
size_t hotkey_num;

void ToTray (HWND hWnd);
void DeleteTray (HWND hWnd);
/**
 * @brief 加载托盘
 *
 * @param hWnd
**/
void ToTray (HWND hWnd) {
    nid.cbSize = (DWORD)sizeof (NOTIFYICONDATA);
    nid.hWnd = hWnd;
    nid.uID = IDR_MAINFRAME;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_TO_TRAY;    // 自定义的消息 处理托盘图标事件
    nid.hIcon = LoadIcon (GetModuleHandle (0), MAKEINTRESOURCE (IDI_SMALL));
    wcscpy_s (nid.szTip, _T ("OneTool")); // 鼠标放在托盘图标上时显示的文字
    Shell_NotifyIcon (NIM_ADD, &nid);     // 在托盘区添加图标
}
/**
 * @brief 删除托盘
 *
 * @param hWnd
**/
void DeleteTray (HWND hWnd) {
    Shell_NotifyIcon (NIM_DELETE, &nid);//在托盘中删除图标
}

bool is_all_enable = true;

LRESULT WINAPI menu_callback (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_COMMAND:
        if (LOWORD (wParam) == ID_APP_EXIT) {
            exit_all ();
        }
        else if (LOWORD (wParam) == ID_APP_DISP) {
            ShowWindow (glo_hwnd, SW_NORMAL);
        }
        else if (LOWORD (wParam) == ID_APP_DIR) {
            string dir = "\"" + exe_path + "\"";
            ShellExecuteA (glo_menu_hwnd, "open", "explorer.exe ", dir.c_str(), NULL, SW_NORMAL);
        }
        else if (LOWORD (wParam) == ID_APP_RUN) {
            /* 反注册所有热键 */
            if (is_all_enable) {
                for (size_t i = 0; i < hotkey_num; i++) {
                    UnregisterHotKey (glo_menu_hwnd, hotkey[i].id);
                }
            }
            /* 注册热键 */
            else {
                for (size_t i = 0; i < hotkey_num; i++) {
                    if (hotkey[i].enable) {
                        RegisterHotKey (glo_menu_hwnd, hotkey[i].id, hotkey[i].mod, hotkey[i].vk);
                    }
                }
            }
            is_all_enable = !is_all_enable;
        }
        break;
    case WM_HOTKEY:
        char buff[100];
        DEBUG_OUT ("%d  %d  %c\n", (int)wParam, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam));
        for (size_t i = 0; i < hotkey_num; i++) {
            if ((int)wParam == hotkey[i].id) {
                if (hotkey[i].enable) {
                    WinExec (hotkey[i].cmd_value.c_str (), SW_NORMAL);
                }
            }
        }
        break;

    case WM_TO_TRAY:
        if ((LOWORD (lParam) & 0x07) != 0) {
            switch (LOWORD (lParam) & 0x07) {
            case 2:
                /* 左键 1 2 */

                break;
            case 5:
                /* 右键 4 5 */

                /* 一个弹出式菜单 */
                menu = CreatePopupMenu ();
                AppendMenu (menu, MF_STRING, ID_APP_DISP, L"显示窗口");
                AppendMenu (menu, MF_STRING, ID_APP_DIR , L"所在目录");
                AppendMenu (menu, MF_STRING, ID_APP_RUN , is_all_enable ? L"暂时关闭" : L"重新启动");
                AppendMenu (menu, MF_STRING, ID_APP_EXIT, L"关闭程序");

                ::GetCursorPos (&menu_pos);
                TrackPopupMenu (menu, TPM_LEFTALIGN, menu_pos.x, menu_pos.y, 0, glo_menu_hwnd, NULL);

                DestroyMenu (menu);

                break;

            default:
                break;
            }
            // sprintf_s (buff, "%d", (UINT)LOWORD (lParam) & 0x07);
            // OutputDebugStringA (buff);
            // OutputDebugStringA ("\n");
        }
        break;

    case WM_DESTROY:
        DeleteTray (glo_menu_hwnd);

        ::PostQuitMessage (0);
        return 0;
    }
    return ::DefWindowProc (hWnd, msg, wParam, lParam);
}
/**
 * @brief 载入菜单和托盘窗口
 *
**/
void menu_init (void) {
    HINSTANCE hInstance = (HINSTANCE)GetModuleHandle (NULL);
    WNDCLASSEX wc = {
        sizeof (WNDCLASSEX),
        CS_CLASSDC,
        menu_callback,
        0L, 0L,
        hInstance,
        LoadIcon (hInstance, MAKEINTRESOURCE (IDI_ICON1)),
        LoadCursor (nullptr, IDC_ARROW),
        (HBRUSH)(COLOR_WINDOW + 1),
        nullptr,
        _T ("OneTool menu - by pomin"),
        LoadIcon (wc.hInstance, MAKEINTRESOURCE (IDI_APPLICATION))
    };

    ::RegisterClassEx (&wc);

    HWND hwnd = ::CreateWindow (
        wc.lpszClassName,
        _T ("OneTool menu - by pomin"),
        WS_SYSMENU,
        0, 0, 0, 0,
        NULL, NULL, wc.hInstance, NULL
    );
    glo_menu_hwnd = hwnd;
    ToTray (glo_menu_hwnd);
}

