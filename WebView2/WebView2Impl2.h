#pragma once
#include "pch.h"
#include "resource.h"
#include "logger.h"
#include "CompositionHost.h"
#include "WebViewEvents.h"
#include "WebViewAuthentication.h"


namespace WebView2
{
	template <class T>
	class CWebView2Impl2 : public CCompositionHost<T>
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
	public:
		// Message map and handlers
		BEGIN_MSG_MAP(CWebView2Impl2)
			CHAIN_MSG_MAP(CCompositionHost<T>)
			MESSAGE_HANDLER(WM_CREATE, OnCreate)
			MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		END_MSG_MAP()

		CWebView2Impl2()
		{
			m_webview2_events = std::make_unique<webview2_events>();
			m_webview2_authentication_events = std::make_unique<webview2_authentication_events>();
		};
		CWebView2Impl2(std::wstring brower_directory, std::wstring user_data_directory, std::wstring url)
		{
			if (!url.empty())
				m_url = url;
			if (!brower_directory.empty())
				m_browser_directory = brower_directory;
			if (!user_data_directory.empty())
				m_user_data_directory = user_data_directory;
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
				RETURN_IF_FAILED(InitializeWebView());
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
				InitializeWebView();
			}
			return 0L;
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

		#pragma endregion WebView2_event

	private:
		HRESULT OnCreateCoreWebView2ControllerCompleted(HRESULT result, ICoreWebView2CompositionController* compositionController)
		{
			LOG_TRACE << __FUNCTION__;
			if (result != S_OK)
				return result;
			m_compositionController = compositionController;
			RETURN_IF_FAILED(m_compositionController->QueryInterface(IID_PPV_ARGS(&m_controller)));
			RETURN_IF_FAILED(m_controller->get_CoreWebView2(&m_webView));
			RETURN_IF_FAILED(m_webview2_events->initialize(m_hwnd, m_webView, m_controller));
			RETURN_IF_FAILED(m_webview2_authentication_events->initialize(m_hwnd, m_webView, m_controller));

			RETURN_IF_FAILED((static_cast<T*>(this))->initialize(m_hwnd, m_controller, m_compositionController));
			CRect bounds;
			GetClientRect(m_hwnd , &bounds);

			RETURN_IF_FAILED(m_controller->put_IsVisible(true));
			
			RETURN_IF_FAILED(m_webView->Navigate(m_url.c_str()));
			(static_cast<T*>(this))->put_bounds(bounds);

			return S_OK;
		}

		HRESULT InitializeWebView()
		{
			auto options = Microsoft::WRL::Make<CoreWebView2EnvironmentOptions>();
			HRESULT hr = CreateCoreWebView2EnvironmentWithOptions(
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
								auto hr = webViewEnvironment3->CreateCoreWebView2CompositionController(
									m_hwnd,
									Microsoft::WRL::Callback<
									ICoreWebView2CreateCoreWebView2CompositionControllerCompletedHandler>(
										[this](HRESULT hr, ICoreWebView2CompositionController* compositionController)
										-> HRESULT
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





