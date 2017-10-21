#pragma once

#include <vector>

#include "../../imgui/imgui.h"
#include "../../imgui/imgui_internal.h"
#include "../../imgui/imgui_operator.h"
//#include "../../IconFontCppHeaders/IconsFontAwesome.h"

#include "../math/math.h"
#include "../render/vulkan.h"

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
		char m_buf[260];
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
		char filename[260];
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
	struct CommandBuffer;
	struct GuiComponent
	{
		Window *window;

		ImGuiContext *context = nullptr;
		CommandBuffer *cb;
		std::vector<VkEvent> waitEvents;
		glm::vec3 bkColor = glm::vec3(0.69f, 0.76f, 0.79f);

		GuiComponent(Window *_window);
		void onKeyDown(int);
		void onKeyUp(int);
		void onChar(int);
		void begin(bool _need_clear);
		void end();
	};

	struct Image;
	void addGuiImage(Image *image);
	void removeGuiImage(Image *image);

	void saveGuiDock(const std::string &filename);
	void loadGuiDock(const std::string &filename);
}
