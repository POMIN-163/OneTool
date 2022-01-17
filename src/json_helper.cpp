#include "json_helper.hpp"

cJSON* root = 0;
/**
 * @brief 文本转键值对
 *
 * @param name 文本
 * @param mod  功能键
 * @param vk   按键
 * @return true
 * @return false
**/
bool string2key (string name, char& mod, char& vk) {
    int mod_pos[3] = { -1, -1, -1, };
    int vk_pos = -1;
    mod = 0;
    vk = 0;
    /* 转大写 */
    for (size_t i = 0; i < name.length (); i++) {
        char* p_char = (char*)(name.c_str () + i);
        *p_char = toupper (*p_char);
    }
    /* 功能键值 */
    if (name.find ("ALT") != -1) { mod |= 0x01; mod_pos[0] = name.find ("ALT") + 3; }
    if (name.find ("CTRL") != -1) { mod |= 0x02; mod_pos[1] = name.find ("CTRL") + 4; }
    if (name.find ("SHIFT") != -1) { mod |= 0x04; mod_pos[2] = name.find ("SHIFT") + 5; }
    /* 键值 */
    for (size_t i = 0; i < 3; i++) {
        vk_pos = vk_pos < mod_pos[i] ? mod_pos[i] : vk_pos;
    }
    vk_pos++;
    vk = name.c_str ()[vk_pos];
    if (vk == 0 || mod == 0) {
        return false;
    }
    else {
        return true;
    }
}
/**
 * @brief 键值对转文本
 *
**/
bool key2string (char mod, char vk, string& str) {
    str = "";

    if (mod & 0x02) { str += "Ctrl+"; }
    if (mod & 0x01) { str += "Alt+"; }
    if (mod & 0x04) { str += "Shift+"; }

    str += vk;
    if (str.empty ()) {
        return false;
    }
    else {
        return true;
    }

}
/**
 * @brief 删除热键项并更新 json 文件
 *
 * @param json_path
 * @param index
**/
void del_to_json (const char* json_path, int index) {
    cJSON* hotkey_arr = NULL;
    if (root && cJSON_GetObjectItem (root, "hotkeys")) {
        hotkey_arr = cJSON_GetObjectItem (root, "hotkeys");
    }
    else {
        return;
    }
    if (hotkey_arr && cJSON_IsArray (hotkey_arr)) {
        cJSON_DeleteItemFromArray (hotkey_arr, index);
    }
    hotkey_num = cJSON_GetArraySize (hotkey_arr);
    ofstream out (json_path);
    if (out.is_open ()) {
        out << cJSON_Print (root);
        out.close ();
    }
    load_from_json (json_path);
}
/**
 * @brief 增加热键项并更新 json 文件
 *
 * @param json_path
 * @param obj
**/
void add_to_json (const char* json_path, hotkey_obj& obj) {
    cJSON* hotkey_arr = NULL;
    if (root && cJSON_GetObjectItem (root, "hotkeys")) {
        hotkey_arr = cJSON_GetObjectItem (root, "hotkeys");
    }
    else {
        root = cJSON_CreateObject ();
        hotkey_arr = cJSON_AddArrayToObject (root, "hotkeys");
    }
    if (hotkey_arr && cJSON_IsArray (hotkey_arr)) {
        cJSON* one_hotkey = cJSON_CreateObject ();
        cJSON_AddBoolToObject (one_hotkey, "enable", obj.enable);
        cJSON_AddStringToObject (one_hotkey, "cmd_value", obj.cmd_value.c_str ());
        cJSON_AddStringToObject (one_hotkey, "key_name", obj.key_name.c_str ());
        cJSON_AddItemToArray (hotkey_arr, one_hotkey);
    }
    hotkey_num = cJSON_GetArraySize (hotkey_arr);
    ofstream out (json_path);
    if (out.is_open ()) {
        out << cJSON_Print (root);
        out.close ();
    }
    load_from_json (json_path);
}
void save_to_json (const char* json_path) {
    ofstream out (json_path);

    //root = cJSON_CreateObject ();
    //cJSON* hotkey_arr = cJSON_AddArrayToObject (root, "hotkeys");

    //for (size_t i = 0; i < hotkey_num; i++) {
    //    cJSON* one_hotkey = cJSON_CreateObject ();
    //    cJSON_AddBoolToObject (one_hotkey, "enable", hotkey[i].enable);
    //    cJSON_AddStringToObject (one_hotkey, "cmd_value", hotkey[i].cmd_value.c_str ());
    //    cJSON_AddStringToObject (one_hotkey, "key_name", hotkey[i].key_name.c_str ());
    //    cJSON_AddItemToArray (hotkey_arr, one_hotkey);
    //}
    if (hotkey_num != 0) {
        cJSON* hotkeys = cJSON_GetObjectItem (root, "hotkeys");
        for (size_t i = 0; i < hotkey_num; i++) {
            cJSON* one_hotkey = cJSON_GetArrayItem (hotkeys, i);
            if (one_hotkey) {

                cJSON* _key_name =  cJSON_CreateString(hotkey[i].key_name.c_str ());
                cJSON* _cmd_value = cJSON_CreateString (hotkey[i].cmd_value.c_str());
                cJSON* _enable = cJSON_CreateBool (hotkey[i].enable);

                cJSON_DeleteItemFromObject (one_hotkey, "key_name");
                cJSON_DeleteItemFromObject (one_hotkey, "cmd_value");
                cJSON_DeleteItemFromObject (one_hotkey, "enable");

                cJSON_AddItemToObject (one_hotkey, "key_name", _key_name);
                cJSON_AddItemToObject (one_hotkey, "cmd_value", _cmd_value);
                cJSON_AddItemToObject (one_hotkey, "enable", _enable);

                DEBUG_OUT ("%s  %s  %d",
                    hotkey[i].cmd_value.c_str (),
                    hotkey[i].key_name.c_str (),
                    hotkey[i].enable
                );
            }
        }
    }
    if (out.is_open ()) {
        out << cJSON_Print (root);
        out.close ();
    }
}
/**
 * @brief 载入 json 文件
 *
 * @param json_path
**/
void load_from_json (const char* json_path) {
    ifstream in (json_path);
    string one_line = "";
    string context = "";
    while (getline (in, one_line)) // 逐行读取
        context += one_line;
    if (root) {
        cJSON_Delete (root);
    }
    root = cJSON_Parse (context.c_str ());
    if (root) {
        cJSON* hotkeys = cJSON_GetObjectItem (root, "hotkeys");
        if (hotkeys && cJSON_IsArray (hotkeys)) {
            hotkey_num = cJSON_GetArraySize (hotkeys);
            if (hotkey_num != 0) {
                hotkey = new hotkey_obj[hotkey_num];
                for (size_t i = 0; i < hotkey_num; i++) {
                    cJSON* one_hotkey = cJSON_GetArrayItem (hotkeys, i);
                    if (one_hotkey) {

                        cJSON* _key_name = cJSON_GetObjectItem (one_hotkey, "key_name");
                        cJSON* _cmd_value = cJSON_GetObjectItem (one_hotkey, "cmd_value");
                        cJSON* _enable = cJSON_GetObjectItem (one_hotkey, "enable");

                        hotkey[i].cmd_value = cJSON_GetStringValue (_cmd_value);
                        hotkey[i].key_name = cJSON_GetStringValue (_key_name);
                        hotkey[i].enable = cJSON_IsTrue (_enable);

                        strcpy_s (hotkey[i].key_name_buff, hotkey[i].key_name.c_str ());
                        strcpy_s (hotkey[i].cmd_value_buff, hotkey[i].cmd_value.c_str ());

                        if (string2key (hotkey[i].key_name, hotkey[i].mod, hotkey[i].vk)) {
                            hotkey[i].id = GET_HOTKEY_ID (hotkey[i]);
                            if (hotkey[i].enable) {
                                UnregisterHotKey (glo_menu_hwnd, hotkey[i].id);
                                if (RegisterHotKey (glo_menu_hwnd, hotkey[i].id, hotkey[i].mod, hotkey[i].vk)) {
                                    hotkey[i].enable = true;
                                }
                                else {
                                    hotkey[i].enable = false;
                                }
                            }
                            else {
                                UnregisterHotKey (glo_menu_hwnd, hotkey[i].id);
                            }
                        }
                        DEBUG_OUT ("%s  %s  %d  %d  ",
                            hotkey[i].cmd_value.c_str (),
                            hotkey[i].key_name.c_str (),
                            hotkey[i].enable,
                            hotkey[i].id
                        );
                    }
                }
            }
        }
    }
    //cJSON_Delete (root);
}
