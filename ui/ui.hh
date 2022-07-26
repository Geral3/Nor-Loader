#pragma once
#include <d3d9.h>
#include "../imgui/imgui.h"
#include "../globals.hh"
#include "../PH.hpp"
using namespace ph;
namespace ui {
	void init(LPDIRECT3DDEVICE9);
	void render();
}

inline static std::vector<module_t> modules = {};
inline char* user_name = NULL;
static std::vector<const char *> moduleItems;

namespace ui {
	inline LPDIRECT3DDEVICE9 dev;
	inline const char* window_title = ("nor_client");
}

namespace ui {
	inline ImVec2 screen_res{ 000, 000 };
	inline ImVec2 window_pos{ 0, 0 };
	inline ImVec2 window_size{ 300, 200 };
	inline DWORD  window_flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar;
}

enum sResponse
{
	LOGIN_ERROR = -1,
	SUCCESSFUL_LOGIN = 1
};