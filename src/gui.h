#pragma once

#include <vector>

#include <imgui.h>
#include <imgui_internal.h>

inline ImVec2 operator+(const ImVec2 &a, const ImVec2 &b)
{
	return ImVec2(a.x + b.x, a.y + b.y);
}

inline ImVec2 operator-(const ImVec2 &a, const ImVec2 &b)
{
	return ImVec2(a.x - b.x, a.y - b.y);
}

inline ImVec2 operator*(const ImVec2 &a, float b)
{
	return ImVec2(a.x * b, a.y * b);
}

inline ImVec4 operator+(const ImVec4& lhs, const ImVec4& rhs) { return ImVec4(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z, lhs.w + rhs.w); }
inline ImVec4 operator*(const ImVec4& lhs, const ImVec4& rhs) { return ImVec4(lhs.x*rhs.x, lhs.y*rhs.y, lhs.z*rhs.z, lhs.w*rhs.w); }
inline ImVec4 operator/(const ImVec4& lhs, const ImVec4& rhs) { return ImVec4(lhs.x / rhs.x, lhs.y / rhs.y, lhs.z / rhs.z, lhs.w / rhs.w); }

inline ImVec4& operator+=(ImVec4& lhs, const ImVec4& rhs) { lhs.x += rhs.x; lhs.y += rhs.y; lhs.z += rhs.z; lhs.w += rhs.w; return lhs; }
inline ImVec4& operator-=(ImVec4& lhs, const ImVec4& rhs) { lhs.x -= rhs.x; lhs.y -= rhs.y; lhs.z -= rhs.z; lhs.w -= rhs.w; return lhs; }
inline ImVec4& operator*=(ImVec4& lhs, const ImVec4& rhs) { lhs.x *= rhs.x; lhs.y *= rhs.y; lhs.z *= rhs.z; lhs.w *= rhs.w; return lhs; }
inline ImVec4& operator/=(ImVec4& lhs, const ImVec4& rhs) { lhs.x /= rhs.x; lhs.y /= rhs.y; lhs.z /= rhs.z; lhs.w /= rhs.w; return lhs; }

inline ImVec2& operator+=(ImVec2& lhs, const ImVec2& rhs) { lhs.x += rhs.x; lhs.y += rhs.y; return lhs; }
inline ImVec2& operator-=(ImVec2& lhs, const ImVec2& rhs) { lhs.x -= rhs.x; lhs.y -= rhs.y; return lhs; }
inline ImVec2& operator*=(ImVec2& lhs, const ImVec2& rhs) { lhs.x *= rhs.x; lhs.y *= rhs.y; return lhs; }
inline ImVec2& operator/=(ImVec2& lhs, const ImVec2& rhs) { lhs.x /= rhs.x; lhs.y /= rhs.y; return lhs; }

#include "math.h"
#include "render.h"

namespace tke
{
	struct DialogT
	{
		enum class State
		{
			eClosed,
			eNeedOpen,
			eOpened
		};
		State state;
		std::string name;

		void begin();
		void end();
	};

	struct YesNoDialog : DialogT
	{
		std::string m_text;
		void(*m_callback)(bool);

		YesNoDialog();
		void start(const std::string &text, void(*callback)(bool));
		void show();
	};

	struct MessageDialog : DialogT
	{
		std::vector<std::string> texts;

		MessageDialog();
		void add(const std::string &text);
		void show();
	};

	struct InputDialog : DialogT
	{
		char m_buf[MAX_PATH];
		void(*m_callback)(const std::string &);

		InputDialog();
		void start(void(*callback)(const std::string &));
		void show();
	};

	struct FileDialogT : DialogT
	{
		std::string m_path;
		std::vector<std::string> m_dirs;
		std::vector<std::string> m_files;

		void search();
	};

	struct DirectoryDialog : DialogT
	{
		std::string m_path;
		std::vector<std::string> m_dirs;
		std::string m_currentPath;
		void(*m_callback)(const std::string &);

		DirectoryDialog();
		void start(void(*callback)(const std::string &));
		void search();
		void show();
	};

	struct OpenFileDialog : FileDialogT
	{
		void(*m_callback)(const std::string &);

		OpenFileDialog();
		void start(void(*callback)(const std::string &));
		void show();
	};

	struct SaveFileDialog : FileDialogT
	{
		char filename[MAX_PATH];
		void(*m_callback)(const std::string &);

		SaveFileDialog();
		void start(void(*callback)(const std::string &));
		void show();
	};

	extern YesNoDialog yesNoDialog;
	extern MessageDialog messageDialog;
	extern InputDialog inputDialog;
	extern DirectoryDialog directoryDialog;
	extern OpenFileDialog openFileDialog;
	extern SaveFileDialog saveFileDialog;

	void showDialogs();

	extern bool uiAcceptedMouse;
	extern bool uiAcceptedKey;

	struct Window;
	struct GuiComponent
	{
		Window *window;

		ImGuiContext *context = nullptr;
		CommandBuffer *cb;
		glm::vec3 bkColor = glm::vec3(0.69f, 0.76f, 0.79f);

		GuiComponent(Window *_window);
		void onKeyDown(int);
		void onKeyUp(int);
		void onChar(int);
		void begin(bool _need_clear = false);
		void end();
	};

	void addGuiImage(Image *image);
	void removeGuiImage(Image *image);
}

// tab
namespace ImGui
{
	/// User Colors & Style Extensions
	enum ImGuiUserCol_ {
		ImGuiUserCol_TabBorder = 0,
		ImGuiUserCol_TabBorderShadow,
		ImGuiUserCol_TabNormal,
		ImGuiUserCol_TabHover,
		ImGuiUserCol_TabTitleTextNormal,
		ImGuiUserCol_TabTitleTextSelected,
		ImGuiUserCol_COUNT
	};

	/// Defines our user style attributes that don't fit within the standard ImGui stack
	IMGUI_API struct ImGuiUserStyle
	{
		ImVec4      Colors[ImGuiUserCol_COUNT];
		ImGuiUserStyle();
	};

	IMGUI_API static ImGuiUserStyle UserStyle;

	/// Additional prototype to retrieve user-defined colors in this header (to prevent foward compatilibity conflicts)
	IMGUI_API const ImU32 GetColorU32(ImGuiUserCol_ idx, float alpha_mul = 0.0f);

	///Bitmask flags for telling _drawPartialRect() what edges to draw
	enum _EdgeType : char {
		EDGE_NONE = 0,
		EDGE_LEFT = 1 << 0,
		EDGE_TOP = 1 << 1,
		EDGE_RIGHT = 1 << 2,
		EDGE_BOTTOM = 1 << 3
	};

	/// Used internally to draw a rounded rect with the different borders disabled
	static void _drawPartialRect(const ImVec2 a, const ImVec2 b, const float rounding, const int rounding_corners,
		ImDrawList* dl, const _EdgeType edges, const ImU32 color, const bool shadow = false,
		const _EdgeType shadow_edges = EDGE_NONE, const float shadow_offset = 4.0f,
		const float shadow_alpha = 0.075f);

	/// Struct containing TabBar data
	IMGUI_API struct TabBar {

		/// Constructor
		TabBar(const char* label, const ImVec2 tab_bar_size);

		/// Stores the titles of the tabs
		std::vector<const char*> tabTitles;

		/// Stores hash IDs of the tabs
		std::vector<ImGuiID> tabHashes;

		/// Keeps a tally of how many tabs have been added
		int tabCount = 0; // Set to 0 in StartTab

						  /// Index defining the active tab
		int activeTab = 0;

		/// Title of the bar. Use #<ID> to omit the title but pass in an ID
		std::string barTitle;

		/// Size
		ImVec2 size = ImVec2(0, 0);

		/// Hash of the TabBar
		ImU32 hash = 0x0;

		/// Index counter, cleared every call to "BeginTabBar"
		int idxCounter = 0;

		/// This holds the upper left corner for the final draw sequence
		ImVec2 upperLeft = ImVec2(0, 0);

		/// Set to true once the tab bar has been initialized
		bool hasBeenInitialized = false;

		/// Holds the rounding value for padding reasons
		float corner_rounding = 0;

		/// Holds the value of the new selected tab
		int newSelected = -1;

		/// Sets the active tab
		void setActiveTab(const unsigned idx);

		/// Returns the currently active tab
		const int getActiveTab();

		/// Useful enums
		enum _TabType : char { LEFTMOST_TAB, MIDDLE_TAB, RIGHTMOST_TAB };

		/// Used internally
		void _drawTabBarTop(const char* label);

		void _drawTabBarBottom();

		/// Used internally
		void _setTabCount();
	};

	/// Struct containing global TabBar status
	IMGUI_API struct TabBarStack {
		/// Vector containing all of our tab bars
		std::vector <TabBar> TabBars;

		/// The running count of our tab bar. Useful as it's only written to when a new tab bar is generated. Helps prevent useless calls to std::vector<>::size()
		unsigned TabBarCount = 0;

		/// Index pointing to the current tab
		unsigned CurrentTabBar = 0;

		/// Returns a pointer to the current tab bar
		TabBar* getCurrentTabBar();

		/// Clears the counting index for the current tab bar
		void clearIdxCountCurrentTabBar();

		/// Returns true if the tab bar already exists, pass idx if you want a return index value
		const bool doesTabBarExist(const ImU32 hash, unsigned* const idx = NULL);

		/// Returns true if the tab bar already exists (computes hash), pass idx if you want a return index value
		const bool doesTabBarExist(const char* id, unsigned* const idx = NULL);
	};

	/// Static instantiation of the TabBarStack
	IMGUI_API static TabBarStack TabStack;

	/// Outwardly accessible way to set the active tab of the current stack.
	IMGUI_API static void SetActiveTabOfCurrentTabBar(const unsigned idx);

	/// Star the Tab Bar with this function. It creates a tab bar based on the computed CRC32 hash and pushes it into the internal stack if so.
	IMGUI_API void BeginTabBar(const char *label, const ImVec2 size = ImVec2(0, 0));

	/// Returns true when the tab is active. To use, implement with if(ImGui::AddTab(...)){}
	IMGUI_API const bool AddTab(const char* title);

	/// Call this after you are done adding tabs
	IMGUI_API void  EndTabBar();

}

// dock
namespace ImGui
{
	void ShutdownDock();
	void RootDock(const ImVec2& pos, const ImVec2& size);
	bool BeginDock(const char* label, bool* opened = nullptr, ImGuiWindowFlags extra_flags = 0, const ImVec2& default_size = ImVec2(-1, -1));
	void EndDock();
	void SetDockActive();
	void SaveDock(const std::string &filename);
	void LoadDock(const std::string &filename);
}
