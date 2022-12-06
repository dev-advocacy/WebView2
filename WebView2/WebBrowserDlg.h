
// WebBrowserDlg.h : header file
//

#pragma once

#include "pch.h"
#include "ComponentBase.h"
#include "resource.h"

#include <winrt/Windows.UI.Composition.h>
#include <winrt/Windows.UI.ViewManagement.h>
namespace winrtComp = winrt::Windows::UI::Composition;


// CWebBrowserDlg dialog
class CWebBrowserDlg : public CDialogImpl<CWebBrowserDlg>
{
public:
	enum { IDD = IDD_DIALOG_WEB_VIEW };
	CWebBrowserDlg(std::wstring browerdirectory, std::wstring userdatedirectory, std::wstring url);
	CWebBrowserDlg() {}


	void InitializeWebView();
	void CloseWebView(bool cleanupUserDataFolder = false);
	HRESULT OnCreateEnvironmentCompleted(HRESULT result, ICoreWebView2Environment* environment);
	HRESULT OnCreateCoreWebView2ControllerCompleted(HRESULT result, ICoreWebView2Controller* controller);
    void WebView2Explorer(LPDISPATCH pDisp, VARIANT * URL);
    HRESULT WebView2MessageHandler(PWSTR *s_Message);
    HRESULT WebView2MessageProcess(ICoreWebView2 * webview, ICoreWebView2WebMessageReceivedEventArgs * args);
	void RunAsync(std::function<void(void)> callback);
	void ResizeEverything();
	HRESULT DCompositionCreateDevice2(IUnknown* renderingDevice, REFIID riid, void** ppv);

	void OnSize(UINT a, int b, int c);

	ICoreWebView2Controller* GetWebViewController()
	{
		return m_controller.get();
	}
	ICoreWebView2* GetWebView()
	{
		return m_webView.get();
	}
	ICoreWebView2Environment* GetWebViewEnvironment()
	{
		return m_webViewEnvironment.get();
	}
	HWND GetMainWindow()
	{
		return this->m_hWnd;
	}

	// Implementation
protected:
	HICON m_hIcon;
	DWORD m_creationModeId = 0;
	
private:
	wil::com_ptr<ICoreWebView2Environment> m_webViewEnvironment;
	wil::com_ptr<ICoreWebView2Controller> m_controller;
	wil::com_ptr<ICoreWebView2> m_webView;
	wil::com_ptr<IDCompositionDevice> m_dcompDevice;
	std::vector<std::unique_ptr<ComponentBase>> m_components;
	winrt::Windows::UI::Composition::Compositor m_wincompCompositor{ nullptr };


	HWND m_mainWindow = nullptr;
	HINSTANCE g_hInstance;
	static constexpr size_t s_maxLoadString = 100;
	template <class ComponentType, class... Args> void NewComponent(Args&&... args);

	template <class ComponentType> ComponentType* GetComponent();

	BEGIN_MSG_MAP(CWebBrowserDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
};

