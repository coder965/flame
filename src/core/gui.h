#ifndef __TKE_GUI__
#define __TKE_GUI__

#include <vector>
#include "..\..\..\imgui-master\imgui.h"

#include "render.h"
#include "window.h"

namespace tke
{
	namespace UI
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
			void start(const std::string &text, void(callback)(bool));
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

		struct Dialogs
		{
			YesNoDialog yesNoDialog;
			MessageDialog messageDialog;
			InputDialog inputDialog;
			DirectoryDialog directoryDialog;
			OpenFileDialog openFileDialog;
			SaveFileDialog saveFileDialog;

			void show();
		};

		struct EngineGuiWindow : Window
		{
			void *m_uiContext = nullptr;
			VkRenderPass m_uiRenderPass = VK_NULL_HANDLE;
			uint32_t m_uiSubpassIndex = -1;
			VkPipeline m_uiPipeline;
			VkFramebuffer m_uiFramebuffer = VK_NULL_HANDLE;
			VkCommandBuffer m_uiCommandBuffer = VK_NULL_HANDLE;
			bool m_uiAcceptedMouse = false;
			bool m_uiAcceptedKey = false;
			Dialogs *m_uiDialogs = nullptr;

			virtual void keyDownEvent(int) override;
			virtual void keyUpEvent(int) override;
			virtual void charEvent(int) override;

			EngineGuiWindow(int cx, int cy, const char *title, unsigned int windowStyle = 0, unsigned int windowStyleEx = 0, bool hasFrame = true);
			void initUi(VkRenderPass uiRenderPass, uint32_t uiSubpassIndex);
		};

		void keyCallback(int key, bool DownUp);
		void charCallback(unsigned int c);
		void lock(EngineGuiWindow *pWindow);
		void unlock();
		EngineGuiWindow *getCurrentWindow();
		void pushIcon(Image *image);
		void setupIcons(VkSampler sampler);
		void initWindow(EngineGuiWindow *pWindow);
		void init();
	}
}

#endif