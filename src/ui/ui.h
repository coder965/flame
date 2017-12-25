#pragma once

#include <vector>

#include "../../imgui/imgui.h"
#include "../../imgui/imgui_internal.h"
#include "../../IconFontCppHeaders/IconsFontAwesome.h"

#include "../refl.h"
#include "../math/math.h"

static inline ImVec2 operator+(const ImVec2& lhs, const float& rhs) { return ImVec2(lhs.x + rhs, lhs.y + rhs); }
static inline ImVec2 operator-(const ImVec2& lhs, const float& rhs) { return ImVec2(lhs.x - rhs, lhs.y - rhs); }
static inline ImVec2 operator*(const ImVec2& lhs, const float& rhs) { return ImVec2(lhs.x * rhs, lhs.y * rhs); }
static inline ImVec2 operator/(const ImVec2& lhs, const float& rhs) { return ImVec2(lhs.x / rhs, lhs.y / rhs); }

static inline ImVec2 operator+(const ImVec2& lhs, const ImVec2& rhs) { return ImVec2(lhs.x + rhs.x, lhs.y + rhs.y); }
static inline ImVec2 operator-(const ImVec2& lhs, const ImVec2& rhs) { return ImVec2(lhs.x - rhs.x, lhs.y - rhs.y); }
static inline ImVec2 operator*(const ImVec2& lhs, const ImVec2& rhs) { return ImVec2(lhs.x * rhs.x, lhs.y * rhs.y); }
static inline ImVec2 operator/(const ImVec2& lhs, const ImVec2& rhs) { return ImVec2(lhs.x / rhs.x, lhs.y / rhs.y); }

static inline ImVec4 operator+(const ImVec4& lhs, const ImVec4& rhs) { return ImVec4(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z, lhs.w + rhs.w); }
static inline ImVec4 operator-(const ImVec4& lhs, const ImVec4& rhs) { return ImVec4(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z, lhs.w - rhs.w); }
static inline ImVec4 operator*(const ImVec4& lhs, const ImVec4& rhs) { return ImVec4(lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z, lhs.w * rhs.w); }
static inline ImVec4 operator/(const ImVec4& lhs, const ImVec4& rhs) { return ImVec4(lhs.x / rhs.x, lhs.y / rhs.y, lhs.z / rhs.z, lhs.w / rhs.w); }

static inline ImVec2& operator+=(ImVec2& lhs, const ImVec2& rhs) { lhs.x += rhs.x; lhs.y += rhs.y; return lhs; }
static inline ImVec2& operator-=(ImVec2& lhs, const ImVec2& rhs) { lhs.x -= rhs.x; lhs.y -= rhs.y; return lhs; }
static inline ImVec2& operator*=(ImVec2& lhs, const ImVec2& rhs) { lhs.x *= rhs.x; lhs.y *= rhs.y; return lhs; }
static inline ImVec2& operator/=(ImVec2& lhs, const ImVec2& rhs) { lhs.x /= rhs.x; lhs.y /= rhs.y; return lhs; }

static inline ImVec4& operator+=(ImVec4& lhs, const ImVec4& rhs) { lhs.x += rhs.x; lhs.y += rhs.y; lhs.z += rhs.z; lhs.w += rhs.w; return lhs; }
static inline ImVec4& operator-=(ImVec4& lhs, const ImVec4& rhs) { lhs.x -= rhs.x; lhs.y -= rhs.y; lhs.z -= rhs.z; lhs.w -= rhs.w; return lhs; }
static inline ImVec4& operator*=(ImVec4& lhs, const ImVec4& rhs) { lhs.x *= rhs.x; lhs.y *= rhs.y; lhs.z *= rhs.z; lhs.w *= rhs.w; return lhs; }
static inline ImVec4& operator/=(ImVec4& lhs, const ImVec4& rhs) { lhs.x /= rhs.x; lhs.y /= rhs.y; lhs.z /= rhs.z; lhs.w /= rhs.w; return lhs; }

namespace ImGui
{
	extern bool main_menu_alive;
	extern bool last_frame_main_menu_alive;

	bool BeginMenu_keepalive(const char* label, bool enabled = true);
}

namespace tke
{
	struct Image;

	IMPL() bool uiAcceptedMouse;
	IMPL() bool uiAcceptedKey;

	IMPL(glm::vec3(0.69f,0.76f,0.79f)) glm::vec3 bkColor;

	void initUi();
	void ui_onKeyDown(int);
	void ui_onKeyUp(int);
	void ui_onChar(int);
	void beginUi(bool _need_clear);
	void endUi();
	void addUiImage(Image*image);
	void removeUiImage(Image *image);
}
