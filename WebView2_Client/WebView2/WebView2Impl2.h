#pragma once
#include "pch.h"
#include "resource.h"
#include "logger.h"
#include "CompositionHost.h"
#include "WebViewEvents.h"
#include "WebViewAuthentication.h"

namespace WebView2
{
	class ContextData
	{
	public:
		std::wstring m_classname = L"Chrome_WidgetWin_0";
		HWND					m_hwnd = nullptr;
	};

	template <class T>
	class CWebView2Impl2 : public CCompositionHost<T>, public IWebWiew2ImplEventCallback
	{
	public:
		std::wstring										m_url;
		std::wstring										m_browser_directory;
		std::wstring										m_user_data_directory;
		bool												m_is_modal = false;
	private:
		HWND												m_hwnd = nullptr;
		wil::com_ptr<ICoreWebView2Environment>				m_webViewEnvironment = nullptr;
		wil::com_ptr<ICoreWebView2CompositionController>	m_compositionController = nullptr;
		wil::com_ptr<ICoreWebView2Controller>				m_controller = nullptr;
		wil::com_ptr<ICoreWebView2>							m_webView = nullptr;
		std::unique_ptr<webview2_events>					m_webview2_events = nullptr;
		std::unique_ptr<webview2_authentication_events>		m_webview2_authentication_events = nullptr;
		std::list<std::future<void>>						m_asyncResults;
		std::mutex											m_asyncResultsMutex;
		HWND												m_hwnd_parent = nullptr;
	public:
		// Message map and handlers
		BEGIN_MSG_MAP(CWebView2Impl2)
			CHAIN_MSG_MAP(CCompositionHost<T>)
			MESSAGE_HANDLER(WM_CREATE, OnCreate)
			MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
			MESSAGE_HANDLER(WM_RUN_FUNCTOR, OnRunFunctor)
		END_MSG_MAP()

		CWebView2Impl2()
		{
			m_webview2_events = std::make_unique<webview2_events>();
			m_webview2_authentication_events = std::make_unique<webview2_authentication_events>();
		};
		void set_parent(HWND hwnd)
		{
			m_hwnd_parent = hwnd;
		}
		virtual ~CWebView2Impl2()
		{
			LOG_TRACE << __FUNCTION__;
		}
		#pragma region windows_event
		LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
		{
			LOG_TRACE << __FUNCTION__;

			T* pT = static_cast<T*>(this);
			if (::IsWindow(pT->m_hWnd))
			{
				m_hwnd = pT->m_hWnd;
				RETURN_IF_FAILED(InitializeWebView(true));
				m_is_modal = true;
			}
			return 0L;
		}
		LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
		{
			LOG_TRACE << __FUNCTION__;
			T* pT = static_cast<T*>(this);
			if (::IsWindow(pT->m_hWnd))
			{
				m_hwnd = pT->m_hWnd;
				RETURN_IF_FAILED(InitializeWebView(true));
			}
			return 0L;
		}
		LRESULT	OnRunFunctor(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
		{
			auto* functor = reinterpret_cast<UIFunctorBase*>(wParam);

			if (functor == nullptr)
				return -1; // Error while posting the message. 

			functor->Invoke();
			functor->SignalComplete();

			// Clean up future<void> instances for fire-and-forget operations that have completed.
			std::lock_guard guard(m_asyncResultsMutex);
			m_asyncResults.remove_if([](std::future<void>& value)
				{
					return value.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready;
				});

			return 0;
		}
		#pragma endregion windows_event

		#pragma region WebView2_event
		HRESULT navigate(std::wstring url)
		{
			if (m_webView)
			{
				RETURN_IF_FAILED(m_webView->Navigate(url.c_str()));
			}
			else
				return ERROR_INVALID_ADDRESS;

			return S_OK;
		}
		HWND get_hwnd()
		{
			HWND parent = nullptr;
			if (SUCCEEDED(m_controller->get_ParentWindow(&parent)))
			{
				ContextData context;

				EnumChildWindows(parent, EnumChildProc, (LPARAM)&context);

				return context.m_hwnd;
			}
			return nullptr;
		}
		void copy()
		{
			if (m_webView)
			{
				m_webView->ExecuteScript(L"document.execCommand(\"copy\")", nullptr);
			}		
		}
		void paste(HWND hwnd)
		{
			if (m_webView)
			{
				m_webView->ExecuteScript(L"document.execCommand(\"paste\")", nullptr);
			}
		}
		void cut()
		{
			if (m_webView)
			{
				m_webView->ExecuteScript(L"document.execCommand(\"cut\")", nullptr);
			}
		}
		#pragma endregion WebView2_event

		// Implement IWebWiew2ImplEventCallback
		virtual HWND GetHWnd() override
		{
			return m_hwnd;
		}


		virtual void KeepAliveAsyncResult(std::future<void>&& result) override
		{
			// Need to keep alive future<void> instance until fire-and-forget async operation completes.
			// See Herb Sutter's paper: https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2012/n3451.pdf
			std::lock_guard guard(m_asyncResultsMutex);
			m_asyncResults.push_back(std::move(result));
		}
		virtual void NavigationStartingEvent(std::wstring_view uri, unsigned long long navigationId, 
			                                 bool isRedirected, bool isUserInitiated) override
		{
			LOG_TRACE << __FUNCTION__;
			LOG_TRACE << L"  uri=" << uri << L", ID=" << navigationId 
					  << L", redirected=" << isRedirected 
					  << L", user initiated=" << isUserInitiated;
		}
		virtual void NavigationCompleteEvent(bool isSuccess, unsigned long long navigationId,
			                                 COREWEBVIEW2_WEB_ERROR_STATUS errorStatus) override
		{
			LOG_TRACE << __FUNCTION__;
			LOG_TRACE << L"  success=" << isSuccess << L", ID=" << navigationId
				      << L", error status=" << errorStatus;
		}
		virtual void ResponseReceivedEvent(std::wstring_view method, std::wstring_view uri) override
		{
			LOG_TRACE << __FUNCTION__;
			LOG_TRACE << L"  method=" << method << L", uri=" << uri;
		}
		virtual void RequestEvent(std::wstring_view method, std::wstring_view uri,
			                      COREWEBVIEW2_WEB_RESOURCE_CONTEXT resourceContext) override
		{
			LOG_TRACE << __FUNCTION__;
			LOG_TRACE << L"  method=" << method << L", uri=" << uri
				      << L", resource context=" << resourceContext;
		}
		virtual void ClientCertificateRequestedEvent(std::vector<ClientCertificate> client_certificates, wil::com_ptr<ICoreWebView2Deferral> deferral) override
		{
			LOG_TRACE << __FUNCTION__;
		}
	private:

		static BOOL CALLBACK EnumChildProc(HWND hwnd, LPARAM lParam)
		{
			ContextData* pThis = (ContextData*)lParam;

			std::array<wchar_t, 1024> dataclassName;

			::GetClassName(hwnd, dataclassName.data(), (int)dataclassName.size());
			std::wstring classname(dataclassName.data());
			if (classname == pThis->m_classname)
			{
				pThis->m_hwnd = hwnd;
			}

			return TRUE;
		}

		HRESULT OnCreateCoreWebView2ControllerCompleted(HRESULT result, ICoreWebView2CompositionController* compositionController)
		{
			LOG_TRACE << __FUNCTION__;
			if (result != S_OK)
				return result;
			m_compositionController = compositionController;
			RETURN_IF_FAILED(m_compositionController->QueryInterface(IID_PPV_ARGS(&m_controller)));
			RETURN_IF_FAILED(m_controller->get_CoreWebView2(&m_webView));


			RETURN_IF_FAILED(m_webview2_events->initialize(this, m_webView, m_controller, m_hwnd_parent));
			RETURN_IF_FAILED(m_webview2_authentication_events->initialize(m_hwnd, m_webView, m_controller));
			RETURN_IF_FAILED((static_cast<T*>(this))->initialize(m_hwnd, m_controller, m_compositionController));
			CRect bounds;
			GetClientRect(m_hwnd , &bounds);

			RETURN_IF_FAILED(m_controller->put_IsVisible(true));		
			RETURN_IF_FAILED(m_webView->Navigate(m_url.c_str()));
			(static_cast<T*>(this))->put_bounds(bounds);

			return S_OK;
		}
		HRESULT InitializeWebView(bool log=false)
		{
			LOG_TRACE << __FUNCTION__ << " Using browser directory:" << m_browser_directory.data();
			auto options = Microsoft::WRL::Make<CoreWebView2EnvironmentOptions>();
			HRESULT hr = S_OK;

			if (log == true)
			{
				fs::path unique_file;
				if (!Utility::GetUniqueLogFileName(unique_file))
				{
					LOG_DEBUG << "Create unique log file for log-net-log filename: " << unique_file;

					auto log = L"--log-net-log=" + unique_file.native();

					hr = options->put_AdditionalBrowserArguments(log.c_str()); // Network logs include the network requests, responses, and details on any errors when loading files.
				}
				else
				{
					LOG_ERROR << "Failed to create unique log file name for log-net-log";
				}
			}
			hr = CreateCoreWebView2EnvironmentWithOptions(
				 m_browser_directory.empty() ? nullptr : m_browser_directory.data(),
				 m_user_data_directory.data(),
				 options.Get(),
				 Microsoft::WRL::Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>([this](
				 HRESULT result, ICoreWebView2Environment* environment) -> HRESULT
				 {
					HRESULT hr = S_OK;
					m_webViewEnvironment = environment;
					wil::com_ptr<ICoreWebView2Environment3> webViewEnvironment3 = m_webViewEnvironment.try_query<ICoreWebView2Environment3>();
					if (webViewEnvironment3)
					{
						auto hr = webViewEnvironment3->CreateCoreWebView2CompositionController(m_hwnd,
												Microsoft::WRL::Callback<ICoreWebView2CreateCoreWebView2CompositionControllerCompletedHandler>(
												[this](HRESULT hr, ICoreWebView2CompositionController* compositionController) -> HRESULT
												{
													if (SUCCEEDED(hr)) {

														hr = OnCreateCoreWebView2ControllerCompleted(hr, compositionController);
													}
													else
														RETURN_IF_FAILED(hr);
													return hr;
										})
									.Get());
							}
							return hr;
						}).Get());
				return hr;
		}
	};
}