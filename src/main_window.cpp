#include "OneTool.hpp"
#include "file_helper.hpp"
#include "menu_helper.hpp"
#include "image_helper.hpp"
#include "json_helper.hpp"

HWND glo_hwnd = NULL;
ImVec2 win_size = {
    0.0f, 0.0f
};
ImVec2 win_pos = {
    0.0f, 0.0f
};

/* 获得带有 webfont 前缀的中文 */
#define GET_ICON_TEXT(ico, text) (string (ICON_FA_##ico) + string (text)).c_str ()

/* json配置文件目录 */
#define JSON_PATH (GET_FILE_PATH ("/data/one_tool.json"))

static void HelpMarker (const char* desc) {
    ImGui::TextDisabled ("(?)");
    if (ImGui::IsItemHovered ()) {
        ImGui::BeginTooltip ();
        ImGui::PushTextWrapPos (ImGui::GetFontSize () * 35.0f);
        ImGui::TextUnformatted (desc);
        ImGui::PopTextWrapPos ();
        ImGui::EndTooltip ();
    }
}
/**
 * @brief 关闭所有窗口
 *
**/
void exit_all (void) {
    SendMessage (glo_hwnd, WM_DESTROY, 0, 0);
    SendMessage (glo_menu_hwnd, WM_DESTROY, 0, 0);
    ImGui::SaveIniSettingsToDisk ("imgui.ini");
    Sleep (100);
    exit (0);
}
/**
 * @brief 初始化主窗口
 *
**/
void main_window_init (void) {
    ImGuiIO& io = ImGui::GetIO ();
    ImGuiStyle& style = ImGui::GetStyle ();
    // 设置字体
    ImFontAtlas* atlas = io.Fonts;
    for (int i = 0; i < atlas->Fonts.Size; i++) {
        ImFont* font = atlas->Fonts[i];
        ImGui::PushID (font);
        ImGui::PushFont (font);
        ImGui::PopFont ();
        font->Scale = 0.6;
        ImGui::PopID ();
    }
    style.FramePadding.x = 15;
    style.FramePadding.y = 10;

    style.FrameRounding = 10;
    style.GrabRounding = 10;

    //style.TabBorderSize = 5;
    //style.WindowBorderSize = 0;
    style.WindowRounding = 3;
    style.WindowPadding.x = 15;
    style.WindowPadding.y = 15;

    style.ItemSpacing.x = 10;
    style.ItemSpacing.y = 5;

    // ImGui::StyleColorsLight ();
    menu_init ();
}
/**
 * @brief 窗口循环
 *
**/
void main_window_client (void) {

    ImGuiIO& io = ImGui::GetIO ();
    ImGuiStyle& style = ImGui::GetStyle ();

    static bool first_load = true;
    static int my_image_width = 0;
    static int my_image_height = 0;
    static ImVec2 main_size = ImVec2 (0, 0);
    static int menu_h = 0;
    static float main_x = 0;
    static float main_y = 0;
    static ID3D11ShaderResourceView* my_texture = NULL;

    /* 初始化、准备资源 (仅一次) */
    if (first_load) {
        main_window_init ();
        /* 载入图片 */
        bool ret = LoadTextureFromFile (GET_FILE_PATH ("/pic/cat.png"), &my_texture, &my_image_width, &my_image_height);
        IM_ASSERT (ret);

        load_from_json (JSON_PATH);
        first_load = false;
    }
    if (!::IsWindowVisible (glo_hwnd)) {
        return;
    }
    /* imgui 接入 */
    ImGui::Begin (
        u8"OneTool - by pomin",
        NULL,
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_MenuBar |
        ImGuiWindowFlags_NoScrollbar
    );
    /* 固定位置 */
    ImGui::SetWindowSize (win_size);
    ImGui::SetWindowPos (win_pos);
    /* 置顶窗口 */
    // SetWindowPos (glo_hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    /* 设置菜单 */
    if (ImGui::BeginMenuBar ()) {

        if (ImGui::BeginMenu (GET_ICON_TEXT (FILE, u8"  文件"))) {
            if (ImGui::MenuItem (GET_ICON_TEXT (TRUCK_LOADING, u8"  重载"))) {
                load_from_json (JSON_PATH);
            }
            if (ImGui::MenuItem (GET_ICON_TEXT (SAVE, u8"   保存"))) {
                save_to_json (JSON_PATH);
            }
            ImGui::EndMenu ();
        }
        if (ImGui::BeginMenu (GET_ICON_TEXT (ADJUST, u8"  主题"))) {
            if (ImGui::MenuItem (GET_ICON_TEXT (MOON, u8"  暗色"))) {
                ImGui::StyleColorsDark ();
            }
            if (ImGui::MenuItem (GET_ICON_TEXT (SUN, u8"  亮色"))) {
                ImGui::StyleColorsLight ();
                style.Colors[0].w = 0.7;
            }
            ImGui::EndMenu ();
        }
        if (ImGui::BeginMenu (GET_ICON_TEXT (COGS, u8"  设置"))) {
            if (ImGui::MenuItem (GET_ICON_TEXT (POWER_OFF, u8"  退出"))) {
                exit_all ();
            }
            ImGui::EndMenu ();
        }
        menu_h = ImGui::GetFrameHeightWithSpacing ();
        ImGui::EndMenuBar ();
    }

    /* 计算宽高 */
    main_size = ImGui::GetWindowSize ();
    main_size.x -= style.ItemSpacing.x * 2 + style.WindowPadding.x * 2;
    main_size.y -= style.ItemSpacing.y * 2 + style.WindowPadding.y * 2 + menu_h;

    main_x = main_size.x;
    main_y = main_size.y;
    /* 绘制窗口 */
    if (ImGui::BeginChild ("CHILD-1", ImVec2 (130, main_y), true)) {
        ImGui::Image ((void*)my_texture, ImVec2 (my_image_width * 0.5, my_image_height * 0.5));
    }
    ImGui::EndChild ();
    ImGui::SameLine ();

    if (ImGui::BeginChild ("CHILD-2", ImVec2 (0, main_y), false)) {
        if (ImGui::BeginTabBar ("TAB-1")) {
            ImGui::PushStyleVar (ImGuiStyleVar_ChildRounding, 5.0f);
            /* 热键窗口绘制 */
            if (ImGui::BeginTabItem (GET_ICON_TEXT (KEY, u8"  热键"))) {
                ImGui::BeginChild ("HOTKEYS", ImVec2(0, main_y - 120));
                for (int i = hotkey_num - 1; i > -1; i--) {
                    /* 单个选择框、输入框 * 2 组合 */
                    if (ImGui::BeginChild ((string("item_child-") + to_string(i)).c_str(), ImVec2(0, 50))) {
                        int xxx = (ImGui::GetWindowWidth()) / 2;
                        if (ImGui::Checkbox (" ", &hotkey[i].enable)) {
                            if (hotkey[i].enable) {
                                // UnregisterHotKey (glo_menu_hwnd, hotkey[i].id);
                                if (RegisterHotKey (glo_menu_hwnd, hotkey[i].id, hotkey[i].mod, hotkey[i].vk)) {

                                }
                                else {
                                    hotkey[i].enable = false;
                                }
                            }
                            else {
                                UnregisterHotKey (glo_menu_hwnd, hotkey[i].id);
                            }
                            save_to_json (JSON_PATH);
                            // ::MessageBoxA (glo_hwnd, (string("按键被按下, 当前值: ") + to_string(hotkey[i].enable)).c_str(), "", 0);
                        }
                        ImGui::SameLine ();
                        if (ImGui::BeginChild ("INPUT_NAME", ImVec2 (xxx, 50))) {
                            if (ImGui::InputText ("", hotkey[i].key_name_buff, 1000, ImGuiInputTextFlags_AutoSelectAll)) {
                                hotkey[i].key_name.clear ();
                                hotkey[i].key_name = string (hotkey[i].key_name_buff);
                                hotkey[i].enable = false;
                                UnregisterHotKey (glo_menu_hwnd, hotkey[i].id);
                            }
                        }
                        ImGui::EndChild ();
                        ImGui::SameLine ();
                        ImGui::SetCursorPosX (ImGui::GetCursorPosX () - main_x * 0.15);
                        if (ImGui::BeginChild ("INPUT_CMD", ImVec2 (xxx, 50))) {
                            if (ImGui::InputText ("  ", hotkey[i].cmd_value_buff, 1000, ImGuiInputTextFlags_AutoSelectAll)) {
                                hotkey[i].cmd_value.clear ();
                                hotkey[i].cmd_value = string (hotkey[i].cmd_value_buff);
                                hotkey[i].enable = false;
                                UnregisterHotKey (glo_menu_hwnd, hotkey[i].id);
                            }
                        }
                        ImGui::EndChild ();
                        ImGui::SameLine ();
                        ImGui::SetCursorPosX (ImGui::GetCursorPosX () - main_x * 0.15);
                        if (ImGui::BeginChild ("DEL", ImVec2 (xxx, 50))) {
                            if (ImGui::Button (GET_ICON_TEXT (UNDO, u8"  删除"))) {
                                del_to_json (JSON_PATH, i);
                            }
                        }
                        ImGui::EndChild ();
                    }
                    ImGui::EndChild ();
                }
                ImGui::EndChild ();
                ImGui::SetCursorPosY (main_y - 55);
                /* 增加热键窗口 */
                ImGui::BeginChild ("CHILD_ADD", ImVec2 (0, 0));
                {
                    const char* items[] = {
                        "Alt+", "Ctrl+", "Ctrl+Alt+", "Shift+",
                        "Shift+Alt+", "Ctrl+Shift+", "Ctrl+Shift+Alt+"
                    };
                    const char* items_asc[] = {
                        "0", "1", "2", "3", "4", "5", "6", "7", "8", "9",
                        "A", "B", "C", "D", "E", "F", "G", "H", "I", "J",
                        "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T",
                        "U", "V", "W", "X", "Y", "Z",
                    };
                    static int item_current = 0;
                    static int item_current_asc = 0;
                    static char add_buff[1000];
                    ImGui::BeginChild ("ADD-1", ImVec2(main_x / 4, 0));
                    {
                        ImGui::Combo ("", &item_current, items, IM_ARRAYSIZE (items));
                        ImGui::EndChild ();
                    }
                    ImGui::SameLine ();
                    ImGui::SetCursorPosX (ImGui::GetCursorPosX() - main_x * 0.08);
                    ImGui::BeginChild ("ADD-2", ImVec2 (main_x / 4, 0));
                    {
                        ImGui::Combo (" ", &item_current_asc, items_asc, IM_ARRAYSIZE (items_asc));
                        ImGui::EndChild ();
                    }
                    ImGui::SameLine ();
                    ImGui::SetCursorPosX (ImGui::GetCursorPosX () - main_x * 0.08);
                    ImGui::BeginChild ("ADD-3", ImVec2 (main_x / 4, 0));
                    {
                        ImGui::InputText (" ", add_buff, 1000);
                        ImGui::EndChild ();
                    }
                    ImGui::SetCursorPosX (ImGui::GetCursorPosX () - main_x * 0.35);
                    ImGui::SameLine ();
                    ImGui::BeginChild ("ADD-4", ImVec2 (0, 0));
                    {
                        if (ImGui::Button (GET_ICON_TEXT (REGISTERED, u8"  注册"))) {
                            hotkey_obj* new_hotkey = new hotkey_obj;
                            bool id_is_used = false;
                            new_hotkey->enable = false;
                            new_hotkey->cmd_value = string(add_buff);
                            new_hotkey->key_name = (string(items[item_current]) + string(items_asc[item_current_asc]).c_str());
                            new_hotkey->mod = item_current + 1;
                            new_hotkey->vk = items_asc[item_current_asc][0];
                            new_hotkey->id = GET_HOTKEY_ID (*new_hotkey);
                            strcpy (new_hotkey->cmd_value_buff, new_hotkey->cmd_value.c_str ());
                            strcpy (new_hotkey->key_name_buff, new_hotkey->key_name.c_str ());
                            for (size_t i = 0; i < hotkey_num; i++) {
                                if (hotkey[i].id == new_hotkey->id) {
                                    id_is_used = true;
                                    break;
                                }
                            }
                            if (!id_is_used) {
                                add_to_json (JSON_PATH, *new_hotkey);
                            }
                            delete new_hotkey;
                        }
                        ImGui::EndChild ();
                    }
                }
                ImGui::EndChild ();
                ImGui::EndTabItem ();
            }
            /* 其他窗口绘制 */
            if (ImGui::BeginTabItem (GET_ICON_TEXT (ARCHIVE, u8"  其他"))) {

                ImGui::EndTabItem ();
            }
            ImGui::PopStyleVar ();
            ImGui::EndTabBar ();
        }
    }
    ImGui::EndChild ();

    ImGui::End ();

}
