
// WebBrowserDlg.h : header file
//

#pragma once

#include "pch.h"
#include "ComponentBase.h"
#include "resource.h"
#include "logger.h"
#include "Utility.h"

#include <winrt/Windows.UI.Composition.h>
#include <winrt/Windows.UI.ViewManagement.h>
namespace winrtComp = winrt::Windows::UI::Composition;

namespace WebView2
{
	template <class T>
	class CWebView2Impl2
	{
	public:
		// Message map and handlers
		BEGIN_MSG_MAP(CWebView2Impl2)
			MESSAGE_HANDLER(WM_PAINT, OnPaint)
			MESSAGE_HANDLER(WM_SIZE, OnSize)
			MESSAGE_HANDLER(WM_CREATE, OnCreate)
		END_MSG_MAP()

		//CWebView2Impl() = default;
		CWebView2Impl2()
		{
			LOG_TRACE << __FUNCTION__;
			WebView2::Utility::InitCOM();
		}
		CWebView2Impl2(std::wstring brower_directory, std::wstring user_data_directory, std::wstring url)
		{
			if (!url.empty())
				url_ = url;
			if (!brower_directory.empty())
				browserDirectory_ = brower_directory;
			if (!user_data_directory.empty())
				userDataDirectory_ = user_data_directory;
		}
		virtual ~CWebView2Impl2()
		{
			LOG_TRACE << __FUNCTION__;
			CloseWebView();
		}

#pragma region windows_event
		virtual BOOL PreTranslateMessage(MSG* pMsg)
		{
			return FALSE;
		}
		virtual LRESULT OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
		{
			T* pT = static_cast<T*>(this);
			if (::IsWindow(pT->m_hWnd))
			{
				CPaintDC dc(pT->m_hWnd);
			}
			return 0L;
		}
		HRESULT OnDlgInit(bool ismodeless = false)
		{
			T* pT = static_cast<T*>(this);
			LOG_TRACE << __FUNCTION__;
			if (pT->InitWebView() == false)
			{
				RETURN_IF_FAILED(HRESULT_FROM_WIN32(GetLastError()));
			}
			m_isModal = ismodeless;
			return 0L;
		}
		LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
		{

			LOG_TRACE << __FUNCTION__;
			if (InitWebView() == false)
			{
				RETURN_IF_FAILED(HRESULT_FROM_WIN32(GetLastError()));
			}
			return 0L;
		}
		/// <summary>
		/// Windows event to Resize the webview2
		/// </summary>
		LRESULT OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
		{
			ResizeToClientArea();
			return 0;
		}
#pragma endregion windows_event


	public:
		std::wstring								url_;
		std::wstring								browserDirectory_;
		std::wstring								userDataDirectory_;
		bool										m_isModal = false;
	private:
		wil::com_ptr<ICoreWebView2Environment>		webViewEnvironment_ = nullptr;
		wil::com_ptr<ICoreWebView2>					webView_ = nullptr;
		wil::com_ptr <ICoreWebView2_10>				m_webviewEventSource3 = nullptr;
		wil::com_ptr<ICoreWebView2_2>				m_webviewEventSource2 = nullptr;
		wil::com_ptr<ICoreWebView2Controller>		webController_ = nullptr;
		wil::com_ptr<ICoreWebView2Settings>			webSettings_ = nullptr;
		wil::com_ptr<ICoreWebView2CookieManager>	cookieManager_ = nullptr;
		EventRegistrationToken						m_navigationStartingToken = {};
		EventRegistrationToken						m_navigationCompletedToken = {};
		EventRegistrationToken						m_documentTitleChangedToken = {};
		EventRegistrationToken						webresourcerequestedToken_ = {};
		EventRegistrationToken						m_webResourceResponseReceivedToken = {};
		EventRegistrationToken						m_basicAuthenticationRequestedToken = {};
		bool										m_isNavigating = false;


	private:

		HRESULT Navigate(std::wstring_view url)
		{
			LOG_TRACE << __FUNCTION__;
			RETURN_IF_NULL_ALLOC(webView_);
			//return (navigate_to(url));
			return S_OK;
		}


		/// <summary>
		/// Create the WebView2 Environment
		/// </summary>
		/// <param name="result"></param>
		/// <param name="environment"></param>
		/// <returns></returns>
		HRESULT OnCreateEnvironmentCompleted(HRESULT hr, ICoreWebView2Environment* environment)
		{
			LOG_TRACE << __FUNCTION__;
			if (hr == S_OK)
			{
				T* pT = static_cast<T*>(this);
				RETURN_IF_WIN32_BOOL_FALSE(::IsWindow(pT->m_hWnd));


				wchar_t t[255];
				pT->GetWindowText((LPTSTR)t, 255);

				LOG_TRACE << "Hwnd=" << pT->m_hWnd << " caption=" << std::wstring(t);

				RETURN_IF_FAILED(environment->QueryInterface(IID_PPV_ARGS(&webViewEnvironment_)));
				RETURN_IF_FAILED(webViewEnvironment_->CreateCoreWebView2Controller(pT->m_hWnd, Microsoft::WRL::Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(this, &CWebView2Impl2::OnCreateWebViewControllerCompleted).Get()));
			}
			else
			{
				RETURN_IF_FAILED_MSG(hr, "function = % s, message = % s, hr = % d\n", __func__, std::system_category().message(hr).data(), hr);
			}
			return S_OK;
		}
		/// <summary>
		/// Initialize the webview2
		/// </summary>
		HRESULT InitWebView()
		{
			LOG_TRACE << __FUNCTION__ << " Using user data directory:" << userDataDirectory_.data();

			T* pT = static_cast<T*>(this);
			RETURN_IF_WIN32_BOOL_FALSE(::IsWindow(pT->m_hWnd));

			auto options = Microsoft::WRL::Make<CoreWebView2EnvironmentOptions>();
			RETURN_IF_FAILED(options->put_AllowSingleSignOnUsingOSPrimaryAccount(TRUE));
			std::wstring langid(Utility::GetUserMUI());
			RETURN_IF_FAILED(options->put_Language(langid.c_str()));

			HRESULT hr = CreateCoreWebView2EnvironmentWithOptions(browserDirectory_.empty() ? nullptr : browserDirectory_.data(),
				userDataDirectory_.data(), options.Get(),
				Microsoft::WRL::Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
					this, &CWebView2Impl2::OnCreateEnvironmentCompleted).Get());

			RETURN_IF_FAILED_MSG(hr, "function = % s, message = % s, hr = % d\n", __func__, std::system_category().message(hr).data(), hr);
			return (S_OK);
		}
		void CloseWebView()
		{
			LOG_TRACE << __FUNCTION__;
			if (this->webView_ && this->webController_ && this->webViewEnvironment_ && this->webSettings_)
			{
				webView_->remove_NavigationCompleted(m_navigationCompletedToken);
				webView_->remove_NavigationStarting(m_navigationStartingToken);
				webView_->remove_DocumentTitleChanged(m_documentTitleChangedToken);
				webView_ = nullptr;
				webController_->Close();
				webController_ = nullptr;
				webSettings_ = nullptr;
				webViewEnvironment_ = nullptr;
			}
			else
			{
				LOG_TRACE << __FUNCTION__ << " Unable to release webview2";
			}
		}
		/// <summary>
		/// Resize the webview2
		/// </summary>
		/// <returns></returns>	
		HRESULT ResizeToClientArea()
		{
			if (webController_)
			{
				T* pT = static_cast<T*>(this);
				RETURN_IF_WIN32_BOOL_FALSE(::IsWindow(pT->m_hWnd));

				CRect bounds;
				pT->GetClientRect(&bounds);


				HRESULT hr = webController_->put_Bounds(bounds);
				if (SUCCEEDED(hr))
				{
					BOOL isVisible = FALSE;
					hr = webController_->get_IsVisible(&isVisible);
					if (SUCCEEDED(hr) && isVisible == FALSE)
					{
						webController_->put_IsVisible(TRUE);
						CRect rc;
						webController_->get_Bounds(&rc);
						LOG_TRACE << __FUNCTION__ << " width=" << rc.Width() << " height=" << rc.Height() << " visibility=" << isVisible;
					}
				}
				else
				{
					LOG_TRACE << __FUNCTION__ << " hr=" << hr;
				}
			}
			return S_OK;
		}
		/// <summary>
		/// Create the webview2 controller
		/// </summary>
		/// <param name="result"></param>
		/// <param name="controller"></param>
		/// <returns></returns>
		HRESULT OnCreateWebViewControllerCompleted(HRESULT result, ICoreWebView2Controller* controller)
		{
			LOG_TRACE << __FUNCTION__;
			HRESULT hr = S_OK;

			RETURN_IF_NULL_ALLOC(controller);

			webController_ = controller;
			RETURN_IF_FAILED(controller->get_CoreWebView2(&webView_));

			RETURN_IF_FAILED(webView_->QueryInterface(&m_webviewEventSource2));
			RETURN_IF_FAILED(webView_->QueryInterface(&m_webviewEventSource3));

			T* pT = static_cast<T*>(this);
			RETURN_IF_WIN32_BOOL_FALSE(::IsWindow(pT->m_hWnd));

			CRect bounds;
			pT->GetClientRect(&bounds);
			webController_->put_Bounds(bounds);

			BOOL isVisible = TRUE;
			webController_->put_IsVisible(isVisible);

			RETURN_IF_FAILED(webView_->get_Settings(&webSettings_));
			RETURN_IF_FAILED(RegisterEventHandlers());
			ResizeToClientArea();
			return S_OK;
		}
		HRESULT onNavigationCompleted(ICoreWebView2* core_web_view2, ICoreWebView2NavigationCompletedEventArgs* args)
		{
			m_isNavigating = false;

			BOOL success;
			HRESULT hr = args->get_IsSuccess(&success);
			if (!success)
			{
				COREWEBVIEW2_WEB_ERROR_STATUS webErrorStatus{};
				hr = args->get_WebErrorStatus(&webErrorStatus);
				if (webErrorStatus == COREWEBVIEW2_WEB_ERROR_STATUS_DISCONNECTED)
				{
					LOG_TRACE << "function=" << __func__ << "COREWEBVIEW2_WEB_ERROR_STATUS_DISCONNECTED";
					return webErrorStatus;
				}
			}
			wil::unique_cotaskmem_string uri;
			webView_->get_Source(&uri);
			if (wcscmp(uri.get(), L"about:blank") == 0)
			{
				uri = wil::make_cotaskmem_string(L"");
			}
			return hr;
		}
		HRESULT onNavigationStarting(ICoreWebView2* core_web_view2, ICoreWebView2NavigationStartingEventArgs* args)
		{
			wil::unique_cotaskmem_string uri;
			args->get_Uri(&uri);
			m_isNavigating = true;
			url_ = uri.get();
			return S_OK;
		}


		HRESULT handle_authorization(ICoreWebView2WebResourceRequestedEventArgs* args)
		{
			wil::com_ptr <ICoreWebView2WebResourceRequest>			 request = nullptr;
			wil::com_ptr <ICoreWebView2HttpRequestHeaders>			 headers = nullptr;

			auto hr = args->get_Request(&request);
			RETURN_IF_FAILED_MSG(hr, "function=%s, message=%s, hr=%d\n", __func__, std::system_category().message(hr).data(), hr);
			hr = request->get_Headers(&headers);
			RETURN_IF_FAILED_MSG(hr, "function=%s, message=%s, hr=%d\n", __func__, std::system_category().message(hr).data(), hr);

			LPWSTR uri = nullptr;
			request->get_Uri(&uri);

			BOOL auth_header = FALSE;

			hr = headers->Contains(L"Authorization", &auth_header);
			if (auth_header == TRUE && hr == S_OK)
			{
				auto authV = new TCHAR[1000];
				hr = headers->GetHeader(L"Authorization", &authV);
				RETURN_IF_FAILED_MSG(hr, "function=%s, message=%s, hr=%d\n", __func__, std::system_category().message(hr).data(), hr);
				LOG_TRACE << __FUNCTION__ << " name=Authorization" << " value=" << authV;
			}
			return S_OK;
		}

		HRESULT onWebResourceRequested(ICoreWebView2* core_web_view2, ICoreWebView2WebResourceRequestedEventArgs* args)
		{
			handle_authorization(args);
			return S_OK;
		}
		HRESULT onResponseReceived(ICoreWebView2* core_web_view2, ICoreWebView2WebResourceResponseReceivedEventArgs* args)
		{
			int statusCode;

			wil::com_ptr<ICoreWebView2WebResourceRequest> webResourceRequest;
			RETURN_IF_FAILED(args->get_Request(&webResourceRequest));
			wil::com_ptr<ICoreWebView2WebResourceResponseView>webResourceResponse;
			RETURN_IF_FAILED(args->get_Response(&webResourceResponse));

			wil::com_ptr <ICoreWebView2HttpResponseHeaders> http_request_header;
			wil::com_ptr <ICoreWebView2HttpHeadersCollectionIterator> it_headers;
			wil::unique_cotaskmem_string					reasonPhrase;

			RETURN_IF_FAILED(webResourceResponse->get_Headers(&http_request_header));
			RETURN_IF_FAILED(webResourceResponse->get_StatusCode(&statusCode));
			RETURN_IF_FAILED(webResourceResponse->get_ReasonPhrase(&reasonPhrase));


			HRESULT hr = http_request_header->GetIterator(&it_headers);
			RETURN_IF_FAILED_MSG(hr, "function=%s, message=%s, hr=%d", __func__, std::system_category().message(hr).data(), hr);

			BOOL hasCurrent = FALSE;
			std::wstring result = L"[";

			while (SUCCEEDED(it_headers->get_HasCurrentHeader(&hasCurrent)) && hasCurrent)
			{
				wil::unique_cotaskmem_string name;
				wil::unique_cotaskmem_string value;

				RETURN_IF_FAILED(it_headers->GetCurrentHeader(&name, &value));
				result += L"{\"name\": " + Utility::EncodeQuote(name.get()) + L", \"value\": " + Utility::EncodeQuote(value.get()) + L"}";
				BOOL hasNext = FALSE;
				RETURN_IF_FAILED(it_headers->MoveNext(&hasNext));
				if (hasNext)
				{
					result += L", ";
				}
			}
			result += L"]";

			//LOG_DEBUG << "response headers:" << result;

			return S_OK;
		}


		HRESULT RegisterEventHandlers()
		{
			LOG_TRACE << __FUNCTION__;


			HRESULT hr = m_webviewEventSource3->add_BasicAuthenticationRequested(Microsoft::WRL::Callback<ICoreWebView2BasicAuthenticationRequestedEventHandler>([this](
				ICoreWebView2* sender,
				ICoreWebView2BasicAuthenticationRequestedEventArgs* args) 	-> HRESULT
				{

					return S_OK;

				}).Get(), &m_basicAuthenticationRequestedToken);


			// response handler
			hr = m_webviewEventSource2->add_WebResourceResponseReceived(Microsoft::WRL::Callback<ICoreWebView2WebResourceResponseReceivedEventHandler>([this](
				ICoreWebView2* core_web_view2,
				ICoreWebView2WebResourceResponseReceivedEventArgs* args)	-> HRESULT
				{
					return (onResponseReceived(core_web_view2, args));

				}).Get(), &m_webResourceResponseReceivedToken);

			RETURN_IF_FAILED_MSG(hr, "function=%s, message=%s, hr=%d\n", __func__, std::system_category().message(hr).data(), hr);

			// NavigationCompleted handler
			hr = webView_->add_NavigationCompleted(Microsoft::WRL::Callback<ICoreWebView2NavigationCompletedEventHandler>([this](
				ICoreWebView2* core_web_view2,
				ICoreWebView2NavigationCompletedEventArgs* args) -> HRESULT
				{
					return (onNavigationCompleted(core_web_view2, args));
				}).Get(), &m_navigationCompletedToken);
			RETURN_IF_FAILED_MSG(hr, "function=%s, message=%s, hr=%d\n", __func__, std::system_category().message(hr).data(), hr);


			// NavigationStarting handler
			hr = webView_->add_NavigationStarting(Microsoft::WRL::Callback<ICoreWebView2NavigationStartingEventHandler>([this](
				ICoreWebView2* core_web_view2,
				ICoreWebView2NavigationStartingEventArgs* args) -> HRESULT
				{
					return (onNavigationStarting(core_web_view2, args));
				}).Get(), &m_navigationStartingToken);
			RETURN_IF_FAILED_MSG(hr, "function=%s, message=%s, hr=%d\n", __func__, std::system_category().message(hr).data(), hr);
			// Add request filter
			hr = webView_->AddWebResourceRequestedFilter(L"*", COREWEBVIEW2_WEB_RESOURCE_CONTEXT_ALL);
			RETURN_IF_FAILED_MSG(hr, "function=%s, message=%s, hr=%d\n", __func__, std::system_category().message(hr).data(), hr);
			hr = webView_->add_WebResourceRequested(Microsoft::WRL::Callback<ICoreWebView2WebResourceRequestedEventHandler>([this](
				ICoreWebView2* core_web_view2,
				ICoreWebView2WebResourceRequestedEventArgs* args) -> HRESULT
				{
					return (onWebResourceRequested(core_web_view2, args));
				}).Get(), &webresourcerequestedToken_);
			if (webView_ != nullptr)
			{
				wil::com_ptr<ICoreWebView2_2> WebView2;
				webView_->QueryInterface(IID_PPV_ARGS(&WebView2));
				hr = WebView2->get_CookieManager(&cookieManager_);
			}
			RETURN_IF_FAILED_MSG(hr, "function=%s, message=%s, hr=%d\n", __func__, std::system_category().message(hr).data(), hr);

			if (!url_.empty())
				hr = webView_->Navigate(url_.c_str());
			else
				hr = webView_->Navigate(L"about:blank");
			return (hr);
		}

	};

	template <class T, class TBase = ATL::CWindow, class TWinTraits = ATL::CControlWinTraits>
	class ATL_NO_VTABLE CCWebView2Impl2 : public ATL::CWindowImpl< T, TBase, TWinTraits >, public CWebView2Impl2< T >
	{
	public:
		BEGIN_MSG_MAP(CCWebView2Impl2)
			CHAIN_MSG_MAP(CWebView2Impl2< T >)
		END_MSG_MAP()
	};

}





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

