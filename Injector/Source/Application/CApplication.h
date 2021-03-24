#pragma once

#pragma warning(push, 0)
#include <AppCore/App.h>
#include <AppCore/Window.h>
#include <AppCore/Overlay.h>
#include <AppCore/JSHelpers.h>
#include <Ultralight/Ultralight.h>
#include <JavascriptCore/Javascript.h>

#include <spdlog/spdlog.h>

#include <fmt/format.h>
#include <fmt/color.h>
#pragma warning(pop)

#include <string>

class CApplication : public ultralight::WindowListener, public ultralight::LoadListener, public ultralight::ViewListener {
public:
	CApplication(const std::string& title = "Application", const uint32_t& width = 800, const uint32_t& height = 600);

	int64_t Initialize();
	int64_t InitializePlatform();
	int64_t InitializeWindow();
	int64_t InitializeOverlay(const std::string& url);

	inline const void Start() const { this->_app->Run(); }

	inline virtual void OnClose(ultralight::Window* window) override { this->_app->Quit(); }
	inline virtual void OnResize(ultralight::Window* window, uint32_t width, uint32_t height) override { 
		if (this->_overlay != 0)
			this->_overlay.get()->Resize(width, height);
	}
		   virtual void OnDOMReady(ultralight::View* caller, uint64_t frame_id, bool is_main_frame, const ultralight::String& url) override;

	ultralight::JSValue CPPAPI_SelectDll(const ultralight::JSObject& thisObj, const ultralight::JSArgs& args);
	ultralight::JSValue CPPAPI_SetProcess(const ultralight::JSObject& thisObj, const ultralight::JSArgs& args);
	ultralight::JSValue CPPAPI_Inject(const ultralight::JSObject& thisObj, const ultralight::JSArgs& args);

	inline void JS_Log(const std::string type, const std::string message) {
		if (this->_view != nullptr)
			this->_view->EvaluateScript(fmt::format("Log('{0}', '{1}')", type.c_str(), message.c_str()).c_str());
	}

	inline std::string ReplaceAll(std::string str, const std::string& from, const std::string& to) {
		size_t start_pos = 0;
		while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
			str.replace(start_pos, from.length(), to);
			start_pos += to.length();
		}

		return str;
	}

	virtual ~CApplication();

public:
	ultralight::RefPtr<ultralight::Window> GetWindow() { return this->_window; }

private:
	ultralight::RefPtr<ultralight::App> _app;
	ultralight::RefPtr<ultralight::Window> _window;
	ultralight::RefPtr<ultralight::Overlay> _overlay;

	ultralight::View* _view = nullptr;

private:
	std::string _url = "";
	std::string _title = "";
	uint32_t _width = 0;
	uint32_t _height = 0;

	std::string _processName = "";

	std::string _dll = "";
};