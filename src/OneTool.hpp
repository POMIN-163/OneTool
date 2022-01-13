#ifndef _ONE_TOOL_
#define _ONE_TOOL_

#include <dinput.h>
#include <tchar.h>
#include <d3d11.h>
#include <stdint.h>
#include <iostream>
#include <string>
#include <stdio.h>
#include <fstream>
#include <shlwapi.h>

#include "..\imgui\imgui.h"
#include "..\imgui\imgui_impl_win32.h"
#include "..\imgui\imgui_impl_dx11.h"
#define DIRECTINPUT_VERSION 0x0800

#include "..\res\framework.h"
#include "..\res\resource.h"
#include "..\opencode\cJSON\cJSON.h"

#pragma comment (lib,"shlwapi.lib")
#pragma comment (lib, "D3D11.lib" )

using namespace std;

#include "IconsFontAwesome5.h"
#include "IconsForkAwesome.h"

typedef struct hotkey_obj {
	bool   enable = 0;
	string key_name;
	string cmd_value;
	int    id;
	char   vk;
	char   mod;
	char   key_name_buff[1000];
	char   cmd_value_buff[1000];
} hotkey_obj;

/* 获取唯一热键 ID */
#define GET_HOTKEY_ID(obj) ((obj).vk << 3 | (obj).mod)

extern HWND glo_hwnd;
extern HWND glo_menu_hwnd;

extern ImVec2 win_size;
extern ImVec2 win_pos;

extern POINT menu_pos;

extern hotkey_obj* hotkey;
extern size_t hotkey_num;

extern bool is_all_enable;

void exit_all (void);
void main_window_init (void);
void main_window_client (void);

/* 调试输出 */
#ifdef DEBUG
	#define DEBUG_OUT(...) \
	do {  \
		char* dbg_buff = new char[500];  \
		sprintf(dbg_buff, ##__VA_ARGS__);  \
		OutputDebugStringA(dbg_buff);    \
		OutputDebugStringA(__FILE__);    \
		OutputDebugStringA(to_string(__LINE__).c_str());    \
		OutputDebugStringA ("\n"); \
		delete[] dbg_buff; \
	} while (0)
#else
	#define DEBUG_OUT(...)
#endif // DEBUG



#endif // !_ONE_TOOL_

