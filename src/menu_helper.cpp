#include "menu_helper.hpp"
#include "file_helper.hpp"
#include <time.h>
#define WM_TO_TRAY 0xf9
#define ID_APP_EXIT 88
#define ID_APP_DISP 66
#define ID_APP_DIR  55
#define ID_APP_RUN  44

HMENU menu;
HBITMAP bmp[4];
POINT menu_pos = { 0, 0 };
HWND glo_menu_hwnd;
NOTIFYICONDATA nid;

hotkey_obj* hotkey;
size_t hotkey_num;

void ToTray (HWND hWnd);
void DeleteTray (HWND hWnd);
/**
 * @brief ��������
 *
 * @param hWnd
**/
void ToTray (HWND hWnd) {
    nid.cbSize = (DWORD)sizeof (NOTIFYICONDATA);
    nid.hWnd = hWnd;
    nid.uID = IDR_MAINFRAME;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_TO_TRAY;    // �Զ������Ϣ ��������ͼ���¼�
    nid.hIcon = LoadIcon (GetModuleHandle (0), MAKEINTRESOURCE (IDI_SMALL));
    wcscpy_s (nid.szTip, _T ("OneTool")); // ����������ͼ����ʱ��ʾ������
    Shell_NotifyIcon (NIM_ADD, &nid);     // �����������ͼ��
}
/**
 * @brief ɾ������
 *
 * @param hWnd
**/
void DeleteTray (HWND hWnd) {
    Shell_NotifyIcon (NIM_DELETE, &nid);//��������ɾ��ͼ��
}
/**
 * @brief ˢ������
 *
 * @param hWnd
**/
void RefreshTray (HWND hWnd) {
    // Shell_NotifyIcon (NIM_DELETE, &nid);  // ��������ɾ��ͼ��
    Shell_NotifyIcon (NIM_ADD, &nid);     // �����������ͼ��
}
void refresh_task (HWND hwnd, UINT message, UINT_PTR iTimerID, DWORD dwTime) {
    RefreshTray (hwnd);
}
bool is_all_enable = true;
/**
 * @brief �����˵�
 *
**/
void menu_track (void) {
    menu = CreatePopupMenu ();
    AppendMenu (menu, MF_STRING, ID_APP_DISP, L"��ʾ����");
    AppendMenu (menu, MF_STRING, ID_APP_DIR, L"����Ŀ¼");
    AppendMenu (menu, MF_STRING, ID_APP_RUN, is_all_enable ? L"��ʱ�ر�" : L"��������");
    AppendMenu (menu, MF_STRING, ID_APP_EXIT, L"�رճ���");

    int bmp_id[4] = { IDB_BITMAP1, IDB_BITMAP2, IDB_BITMAP3, IDB_BITMAP4 };
    for (size_t i = 0; i < 4; i++) {
        bmp[i] = (HBITMAP)LoadImage (GetModuleHandle (NULL), MAKEINTRESOURCE (bmp_id[i]), IMAGE_BITMAP, 20, 20, LR_DEFAULTCOLOR );
        SetMenuItemBitmaps (menu, i, MF_BYPOSITION, bmp[i], bmp[i]);
    }
    ::GetCursorPos (&menu_pos);
    SetForegroundWindow (glo_menu_hwnd); // ���κ�һ��������Ҽ��˵�������ʧ
    TrackPopupMenu (menu, TPM_LEFTALIGN | TPM_VERNEGANIMATION | TPM_RIGHTBUTTON, menu_pos.x, menu_pos.y, 0, glo_menu_hwnd, NULL);

    DestroyMenu (menu);
}
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
            /* ��ע�������ȼ� */
            if (is_all_enable) {
                for (size_t i = 0; i < hotkey_num; i++) {
                    UnregisterHotKey (glo_menu_hwnd, hotkey[i].id);
                }
            }
            /* ע���ȼ� */
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
        DEBUG_OUT ("%d  %d  %c\n", (int) wParam, (UINT) LOWORD(lParam), (UINT) HIWORD(lParam));
        for (size_t i = 0; i < hotkey_num; i++) {
            if ((int)wParam == hotkey[i].id) {
                if (hotkey[i].enable) {
                    /* ���� WinExec ����֧�ֺ���������в�������ʹ�� WinExec */
                    WinExec (hotkey[i].cmd_value.c_str(), SW_NORMAL);
                    // strcpy (buff, hotkey[i].cmd_value_buff);
                    // PathRemoveFileSpecA (buff);
                    // DEBUG_OUT("work_dir:%s", buff);
                    // ShellExecuteA (glo_menu_hwnd, "open", hotkey[i].cmd_value.c_str (), "", buff, SW_NORMAL);
                }
            }
        }
        break;

    case WM_TO_TRAY:
        if ((LOWORD (lParam) & 0x07) != 0) {
            switch (LOWORD (lParam) & 0x07) {
            case 2:
                /* ��� 1 2 */
                menu_track ();
                break;
            case 5:
                /* �Ҽ� 4 5 */
                menu_track ();
                break;
            default:
                break;
            }
        }
        break;

    case WM_DESTROY:
        KillTimer (glo_menu_hwnd, 9999);
        DeleteTray (glo_menu_hwnd);

        ::PostQuitMessage (0);
        return 0;
    }
    return ::DefWindowProc (hWnd, msg, wParam, lParam);
}
/**
 * @brief ����˵������̴���
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
    SetTimer (glo_menu_hwnd, 9999, 10000, refresh_task);
}

