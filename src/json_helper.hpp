#ifndef _JSON_HELPER_
#define _JSON_HELPER_
#include "OneTool.hpp"

void save_to_json (const char* json_path);
void load_from_json (const char* json_path);
void add_to_json (const char* json_path, hotkey_obj& obj);
void del_to_json (const char* json_path, int index);

#endif // !_JSON_HELPER_
