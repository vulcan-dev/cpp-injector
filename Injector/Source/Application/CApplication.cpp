#include "CApplication.h"
#include "../Utilities.h"

#include <AppCore/Platform.h>

#include <windows.h>
#include <Commdlg.h>
#include <iostream>

using namespace ultralight;

CApplication::CApplication(const std::string& title, const uint32_t& width, const uint32_t& height) : _title(title), _width(width), _height(height), _view(nullptr) {}

int64_t CApplication::Initialize() {
	try {
		this->_app = App::Create();
	} catch (std::exception e) {
		spdlog::error("Exception: ", e.what());
	}

	this->InitializeWindow();

	return 0;
}

int64_t CApplication::InitializePlatform() {
	try {
		Platform::instance().set_font_loader(GetPlatformFontLoader());
		Platform::instance().set_logger(GetDefaultLogger("Logs/Ultralight.log"));
		Platform::instance().set_file_system(GetPlatformFileSystem("./Assets"));
	} catch (std::exception e) {
		spdlog::error("Exception: ", e.what());
		return 1;
	}

	return 0;
}

int64_t CApplication::InitializeWindow() {
	try {
		if (this->_title.empty()) {
			this->_title = "Application";
			spdlog::warn("Title \"{}\" is invalid. Reverting to defaults \"Application\"", this->_title);
		}

		this->_window = Window::Create(this->_app->main_monitor(), this->_width, this->_height, false, ultralight::kWindowFlags_Titled | ultralight::kWindowFlags_Resizable);
		this->_window->is_accelerated();

		this->_window->SetTitle(this->_title.c_str());
		this->_window->set_listener(this);
	} catch (std::exception e) {
		spdlog::error("Exception: ", e.what());
	}

	return 0;
}

int64_t CApplication::InitializeOverlay(const std::string& url) {
	try {
		this->_overlay = Overlay::Create(*this->_window, this->_width, this->_height, 0, 0);
		this->_overlay->view()->set_load_listener(this);
		this->_overlay->view()->LoadURL(url.c_str());
	} catch (std::exception e) {
		spdlog::error("Exception: ", e.what());
	}

	return 0;
}

void CApplication::OnDOMReady(ultralight::View* caller, uint64_t frame_id, bool is_main_frame, const ultralight::String& url) {
	Ref< JSContext > context = caller->LockJSContext();
	SetJSContext(context.get());
	JSObject global = JSGlobalObject();

	global["CPPAPI_SelectDll"] = BindJSCallbackWithRetval(&CApplication::CPPAPI_SelectDll);
	global["CPPAPI_SetProcess"] = BindJSCallbackWithRetval(&CApplication::CPPAPI_SetProcess);
	global["CPPAPI_Inject"] = BindJSCallbackWithRetval(&CApplication::CPPAPI_Inject);

	this->_view = caller;
}

ultralight::JSValue CApplication::CPPAPI_SelectDll(const ultralight::JSObject& thisObj, const ultralight::JSArgs& args) {
	OPENFILENAME ofn;
	char szFile[100] = {};

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL;
	ofn.lpstrFile = szFile;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = "Dynamic Link Library\0*.dll*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	if (GetOpenFileName(&ofn)) {
		this->_dll = ofn.lpstrFile;
		spdlog::info("Selected Dll: {}", this->_dll);
		this->JS_Log("I", fmt::format("Selected file: {}", this->ReplaceAll(this->_dll, "\\", "\\\\")));
	} else {
		this->JS_Log("I", "User canceled action");
	}

	return 0;
}

ultralight::JSValue CApplication::CPPAPI_SetProcess(const ultralight::JSObject& thisObj, const ultralight::JSArgs& args) {
	JSString str = args[0];
	char buf[128];
	JSStringGetUTF8CString(str, buf, 128);
	if (buf == 0) {
		this->JS_Log("E", "Invalid application");
		return 1;
	} else {
		this->_processName = buf;
	};

	return 0;
}

ultralight::JSValue CApplication::CPPAPI_Inject(const ultralight::JSObject& thisObj, const ultralight::JSArgs& args) {
	DWORD processId = Utilities::fetchPID(this->_processName.c_str());
	spdlog::info("PID {}", processId);

	if (Utilities::Inject_RemoteThread(this->_dll, processId) != 0) {
		this->JS_Log("E", fmt::format("Failed CreateRemoteThread Injection: {}", GetLastError()));
		spdlog::error("Failed injecting into process");
		return 1;
	} else {
		this->JS_Log("I", fmt::format("Injected {0} into Process: {1}", this->ReplaceAll(this->_dll, "\\", "\\\\"), this->_processName));
	}

	return 0;
}

CApplication::~CApplication() {
	this->_view = nullptr;
	this->_overlay = nullptr;
	this->_window = nullptr;
	this->_app = nullptr;
}