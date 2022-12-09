#pragma once
#include "Utility.h"
#include "logger.h"

namespace WebView2
{
	class webview2_events
	{
	private:
		wil::com_ptr<ICoreWebView2>				m_webviewEventSource = nullptr;
		wil::com_ptr<ICoreWebView2_2>           m_webviewEventSource2 = nullptr;
		wil::com_ptr<ICoreWebView2_10>          m_webviewEventSource3 = nullptr;
		wil::com_ptr<ICoreWebView2Controller>	m_controllerEventSource;
		HWND						            m_hwnd = nullptr;
		EventRegistrationToken                  m_webResourceResponseReceivedToken = {};
		EventRegistrationToken                  m_webResourceRequestedToken = {};	
		EventRegistrationToken                  m_navigationStartingToken = {};
		EventRegistrationToken                  m_navigationCompletedToken = {};
	public:
		webview2_events()
		{
		}
		~webview2_events()
		{
			if (m_webResourceResponseReceivedToken.value != 0)
				m_webviewEventSource2->remove_WebResourceResponseReceived(m_webResourceResponseReceivedToken);		
			if (m_navigationCompletedToken.value != 0)
				m_webviewEventSource->remove_NavigationCompleted(m_navigationCompletedToken);
			if (m_navigationStartingToken.value != 0)
				m_webviewEventSource->remove_NavigationStarting(m_navigationStartingToken);
			if (m_webResourceRequestedToken.value != 0)
				m_webviewEventSource->remove_WebResourceRequested(m_webResourceRequestedToken);			
		}
		HRESULT initialize(HWND hwnd, wil::com_ptr<ICoreWebView2> webviewEventSource, wil::com_ptr<ICoreWebView2Controller> controllerEventSource)
		{
			if (!::IsWindow(hwnd) || webviewEventSource == nullptr || controllerEventSource == nullptr)
			{
				RETURN_IF_FAILED_MSG(ERROR_INVALID_PARAMETER, "function = % s, message = % s, hr = % d", __func__, std::system_category().message(ERROR_INVALID_PARAMETER).data(), ERROR_INVALID_PARAMETER);
			}
			m_webviewEventSource = webviewEventSource;
			m_controllerEventSource = controllerEventSource;
			m_hwnd = hwnd;
			if (m_webviewEventSource.try_query_to<ICoreWebView2_2>(&m_webviewEventSource2) != true)
				return(ERROR_INVALID_DATA);
			if (m_webviewEventSource.try_query_to<ICoreWebView2_10>(&m_webviewEventSource3) != true)
				return(ERROR_INVALID_DATA);
			RETURN_IF_FAILED(enable_webresource_navigation_starting_event());
			RETURN_IF_FAILED(enable_webresource_navigation_complete_event());
			RETURN_IF_FAILED(enable_webresource_response_received_event());
			RETURN_IF_FAILED(enable_webresource_request_event());
			return S_OK;
		}
	private:
		void trace_webresource_response_received_event(wil::com_ptr<ICoreWebView2WebResourceRequest> webResourceRequest)
		{
			//LOG_TRACE << __FUNCTION__;
		}
		void raise_webresource_response_received_event(wil::com_ptr<ICoreWebView2WebResourceRequest> webResourceRequest)
		{
			//LOG_TRACE << __FUNCTION__;
		}
		void trace_webresource_navigation_complete_event(wil::com_ptr <ICoreWebView2NavigationCompletedEventArgs> args)
		{
			BOOL is_success = FALSE;
			UINT64 inavigationid = 0;

			args->get_IsSuccess(&is_success);
			args->get_NavigationId(&inavigationid);

			LOG_TRACE << __FUNCTION__ << " IsSuccess:" << is_success << " NavigationId:" << inavigationid;
		}		
		void raise_webresource_navigation_complete_event(wil::com_ptr <ICoreWebView2NavigationCompletedEventArgs> args)
		{
			LOG_TRACE << __FUNCTION__;
		}
		void trace_webresource_navigation_starting_event(wil::com_ptr <ICoreWebView2NavigationStartingEventArgs> args)
		{
			BOOL is_redirected = FALSE;
			BOOL is_userInitiated = FALSE;
			UINT64 inavigationid = 0;
			args->get_IsRedirected(&is_redirected);
			args->get_IsUserInitiated(&is_userInitiated);
			args->get_NavigationId(&inavigationid);

			wil::unique_cotaskmem_string navigationTargetUri;
			if (SUCCEEDED(args->get_Uri(&navigationTargetUri)))
			{
				LOG_TRACE << __FUNCTION__ << " uri:" << navigationTargetUri.get() << " redirected:" << is_redirected
						  << " is user initiated:" << is_userInitiated << "NavigationId" << inavigationid;
			}
		}
		void raise_webresource_navigation_starting_event(wil::com_ptr <ICoreWebView2NavigationStartingEventArgs> args)
		{
			LOG_TRACE << __FUNCTION__;
		}
		void trace_webresource_request_event()
		{
			//LOG_TRACE << __FUNCTION__;
		}
		void raise_webresource_request_event()
		{
			//LOG_TRACE << __FUNCTION__;
		}	
		HRESULT enable_webresource_response_received_event()
		{
			LOG_TRACE << __FUNCTION__;
			m_webviewEventSource2->add_WebResourceResponseReceived(
					Microsoft::WRL::Callback<ICoreWebView2WebResourceResponseReceivedEventHandler>(
					[this](ICoreWebView2* webview, ICoreWebView2WebResourceResponseReceivedEventArgs* args)
					-> HRESULT {
						wil::com_ptr<ICoreWebView2WebResourceRequest> webResourceRequest;
						RETURN_IF_FAILED(args->get_Request(&webResourceRequest));
						wil::com_ptr<ICoreWebView2WebResourceResponseView> webResourceResponse;
						RETURN_IF_FAILED(args->get_Response(&webResourceResponse));
						webResourceResponse->GetContent(Microsoft::WRL::Callback<ICoreWebView2WebResourceResponseViewGetContentCompletedHandler>(
														[this, webResourceRequest,
														webResourceResponse](HRESULT result, IStream* content)
						{
							trace_webresource_response_received_event(webResourceRequest);
							raise_webresource_response_received_event(webResourceRequest);
							return S_OK;
					})
				.Get());
				return S_OK;})
				.Get(), &m_webResourceResponseReceivedToken);
			return S_OK;
		}
		HRESULT enable_webresource_navigation_complete_event()
		{
			LOG_TRACE << __FUNCTION__;
			RETURN_IF_FAILED(m_webviewEventSource->add_NavigationCompleted(Microsoft::WRL::Callback<ICoreWebView2NavigationCompletedEventHandler>([this](
				ICoreWebView2* core_web_view2,
				ICoreWebView2NavigationCompletedEventArgs* args) -> HRESULT
				{
					trace_webresource_navigation_complete_event(args);
					raise_webresource_navigation_complete_event(args);
					return S_OK;
				}).Get(), &m_navigationCompletedToken));

			return S_OK;
		}
		HRESULT enable_webresource_navigation_starting_event()
		{
			LOG_TRACE << __FUNCTION__;
			RETURN_IF_FAILED(m_webviewEventSource->add_NavigationStarting(Microsoft::WRL::Callback<ICoreWebView2NavigationStartingEventHandler>([this](
				ICoreWebView2* core_web_view2,
				ICoreWebView2NavigationStartingEventArgs* args) -> HRESULT
				{			
					trace_webresource_navigation_starting_event(args);
					raise_webresource_navigation_starting_event(args);
					return S_OK;
				}).Get(), &m_navigationStartingToken));
			return S_OK;
		}
		HRESULT enable_webresource_request_event()
		{
			LOG_TRACE << __FUNCTION__;
			RETURN_IF_FAILED(m_webviewEventSource->AddWebResourceRequestedFilter(L"*", COREWEBVIEW2_WEB_RESOURCE_CONTEXT_ALL));
			RETURN_IF_FAILED(m_webviewEventSource->add_WebResourceRequested(Microsoft::WRL::Callback<ICoreWebView2WebResourceRequestedEventHandler>([this](
				ICoreWebView2* core_web_view2,
				ICoreWebView2WebResourceRequestedEventArgs* args) -> HRESULT
				{
					trace_webresource_request_event();
					raise_webresource_request_event();
					return S_OK;
				}).Get(), &m_webResourceRequestedToken));
			return S_OK;
		}				
	};
}