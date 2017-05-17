#ifndef __TKE_GUI__
#define __TKE_GUI__

#include <vector>
#include "..\..\..\imgui-master\imgui.h"

#include "render.h"
#include "window.h"

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

	struct GuiWindow : Window
	{
		void *uiContext = nullptr;
		VkRenderPass uiRenderPass = VK_NULL_HANDLE;
		uint32_t uiSubpassIndex = -1;
		VkPipeline uiPipeline;
		VkFramebuffer uiFramebuffer = VK_NULL_HANDLE;
		VkCommandBuffer uiCommandBuffer = VK_NULL_HANDLE;
		bool uiAcceptedMouse = false;
		bool uiAcceptedKey = false;

		virtual void keyDownEvent(int) override;
		virtual void keyUpEvent(int) override;
		virtual void charEvent(int) override;

		GuiWindow(int cx, int cy, const char *title, unsigned int windowStyle = 0, unsigned int windowStyleEx = 0, bool hasFrame = true);
		void initUi(VkRenderPass uiRenderPass, uint32_t uiSubpassIndex);
		void lockUi();
		void unlockUi();
	};

	extern GuiWindow *guiCurrentWindow;

	void guiPushIcon(Image *image);
	void guiSetupIcons(VkSampler sampler);
	void initGui();
}

#endif