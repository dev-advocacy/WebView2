#pragma once

#include "Utility.h"
#include "CheckFailure.h"
#include "logger.h"
#include "headersprop.h"

namespace WebView2
{
	template <class T>
	class CDialogWebView2Impl
	{
	public:
		using CallbackFunc = std::function<void(void)>;
		enum class CallbackType
		{
			CreationCompleted,
			NavigationCompleted,
			TitleChanged,
			AuthenticationCompleted,
			NavigationStarting,
		};

		void CDialogWebView2Impl_Init()
		{
			T* pT = static_cast<T*>(this);
			LOG_TRACE << __FUNCTION__;
			if (pT->InitWebView() == false)
			{
				// TODO : need to handle this error properly
				THROW_WIN32(GetLastError());
			}
			pT->RegisterCallback(CDialogWebView2Impl::CallbackType::CreationCompleted, [this]() {CreationCompleted(); });
			pT->RegisterCallback(CDialogWebView2Impl::CallbackType::NavigationCompleted, [this]() {NavigationCompleted(this->url_); });
			pT->RegisterCallback(CDialogWebView2Impl::CallbackType::AuthenticationCompleted, [this]() {AuthenticationCompleted(); });
			pT->RegisterCallback(CDialogWebView2Impl::CallbackType::NavigationStarting, [this]() {NavigationStarting(); });
		}
		// Message map and handlers
		BEGIN_MSG_MAP(CDialogWebView2Impl)
			MESSAGE_HANDLER(WM_PAINT, OnPaint)
			MESSAGE_HANDLER(WM_SIZE, OnSize)
			MESSAGE_HANDLER(MSG_RUN_ASYNC_CALLBACK, OnCallBack)
		END_MSG_MAP()

		CDialogWebView2Impl()
		{
			LOG_TRACE << __FUNCTION__;
			WebView2::Utility::InitCOM();
			m_callbacks[CallbackType::CreationCompleted] = nullptr;
			m_callbacks[CallbackType::NavigationCompleted] = nullptr;
			m_callbacks[CallbackType::AuthenticationCompleted] = nullptr;
			m_callbacks[CallbackType::NavigationStarting] = nullptr;
		}
		CDialogWebView2Impl(std::wstring brower_directory, std::wstring user_data_directory, std::wstring url)
		{
			if (!url.empty())
				url_ = url;
			if (!brower_directory.empty())
				browserDirectory_ = brower_directory;
			if (!user_data_directory.empty())
				userDataDirectory_ = user_data_directory;
		}
		virtual ~CDialogWebView2Impl()
		{
			LOG_TRACE << __FUNCTION__;
			CloseWebView();
		}
		LRESULT	OnCallBack(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
		{
			auto* task = reinterpret_cast<CallbackFunc*>(wParam);
			(*task)();
			delete task;
			return 0;
		}
		void RegisterCallback(CallbackType const type, CallbackFunc callback)
		{
			m_callbacks[type] = callback;
		}

		#pragma region windows_event
		virtual BOOL PreTranslateMessage(MSG* pMsg)
		{
			return FALSE;
		}
		virtual LRESULT OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
		{
			T* pT = static_cast<T*>(this);
			THROW_IF_WIN32_BOOL_FALSE(::IsWindow(pT->m_hWnd));
			CPaintDC dc(pT->m_hWnd);
			return 0;
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
		#pragma region event
		virtual void CreationCompleted()
		{
			LOG_TRACE << __FUNCTION__;
		}
		virtual void NavigationCompleted(std::wstring url)
		{
			LOG_TRACE << __FUNCTION__ << L" URI=" << url;
		}
		virtual void AuthenticationCompleted()
		{
			LOG_TRACE << __FUNCTION__;
		}
		virtual void NavigationStarting()
		{
			LOG_TRACE << __FUNCTION__;
		}
		#pragma endregion events
		#pragma region webview2_implementation
		HRESULT process_cookie_dev_tools(PCWSTR resultJson)
		{
			utility::stringstream_t jsonCookieArray;
			jsonCookieArray << resultJson;
			web::json::value jsonCookieArrayValue = web::json::value::parse(jsonCookieArray);
			if (jsonCookieArrayValue.is_object() == true)
			{
				for (auto iter = std::begin(jsonCookieArrayValue.as_object()); iter != std::end(jsonCookieArrayValue.as_object()); ++iter)
				{
					std::wstring name = iter->first;
					web::json::value cookiecoll = iter->second;

					if (cookiecoll.is_array() == true)
					{
						for (auto iter1 = std::begin(cookiecoll.as_array()); iter1 != std::end(cookiecoll.as_array()); ++iter1)
						{
							auto& cookie = *iter1;
							if (cookie.is_object() == true)
							{

								auto vdomain = cookie[DOMAIN_PROP].serialize();
								auto vexpire = cookie[EXPIRES_PROP].serialize();
								// TODO : convert the date
								auto dt = utility::datetime::from_string(vexpire, utility::datetime::RFC_1123);
								uint64_t interval = dt.to_interval();
								auto vpath = cookie[PATH_PROP].serialize();
								auto vvalue = cookie[VALUE_PROP].serialize();
								auto vname = cookie[NAME_PROP].serialize();

								std::wstring s = std::format(L"domain:{0} expire:{1} path:{2} name={3} value={4}", vdomain, dt.to_string(), vpath, vname, vvalue);
								LOG_TRACE << s;
							}
						}
					}
				}
			}
			return S_OK;
		}
		bool get_cookies_by_devtools()
		{
			LOG_TRACE << __FUNCTION__;
			THROW_IF_NULL_ALLOC(webSettings_);
			wil::com_ptr<ICoreWebView2DevToolsProtocolEventReceiver> receiver;
			THROW_IF_FAILED(webView_->GetDevToolsProtocolEventReceiver(L"Network.getAllCookies", &receiver));
			HRESULT hr = webView_->CallDevToolsProtocolMethod(L"Network.getAllCookies", L"{}", Callback<ICoreWebView2CallDevToolsProtocolMethodCompletedHandler>([this](HRESULT error, PCWSTR resultJson) -> HRESULT
				{
					if (SUCCEEDED(error))
					{
						error = process_cookie_dev_tools(resultJson);
					}
			return error;
				}
			).Get());
			THROW_IF_FAILED_MSG(hr, "function = % s, message = % s, hr = % d\n", __func__, std::system_category().message(hr).data(), hr);
			return true;
		}
		bool add_cookie(std::wstring domain, std::wstring name, std::wstring value)
		{
			LOG_TRACE << __FUNCTION__ << L" domain=" << domain << L" name=" << name << L" value=" << value;
			THROW_IF_NULL_ALLOC(cookieManager_);
			wil::com_ptr<ICoreWebView2Cookie> cookie;
			HRESULT hr = cookieManager_->CreateCookie(name.c_str(), value.c_str(), domain.c_str(), L"/", &cookie);
			THROW_IF_FAILED_MSG(hr, "function = % s, message = % s, hr = % d\n", __func__, std::system_category().message(hr).data(), hr);

			//cookie->put_IsHttpOnly(TRUE);
			cookie->put_IsSecure(TRUE);

			hr = cookieManager_->AddOrUpdateCookie(cookie.get());
			THROW_IF_FAILED_MSG(hr, "function = % s, message = % s, hr = % d\n", __func__, std::system_category().message(hr).data(), hr);

			return true;
		}
		bool get_cookies(std::wstring uri)
		{
			THROW_IF_NULL_ALLOC(cookieManager_);
			HRESULT hr = cookieManager_->GetCookies(uri.c_str(), Callback<ICoreWebView2GetCookiesCompletedHandler>([this, uri](HRESULT error_code, ICoreWebView2CookieList* list) -> HRESULT
				{
					if (SUCCEEDED(error_code))
					{
						std::wstring result;
						UINT cookie_list_size;
						THROW_IF_FAILED(list->get_Count(&cookie_list_size));
						if (cookie_list_size == 0)
						{
							result += L"No cookies found.";
						}
						else
						{
							result += std::to_wstring(cookie_list_size) + L" cookie(s) found";
							if (!uri.empty())
							{
								result += L" on " + uri;
							}
							result += L"\n[";
							for (UINT i = 0; i < cookie_list_size; ++i)
							{
								wil::com_ptr<ICoreWebView2Cookie> cookie;
								THROW_IF_FAILED(list->GetValueAtIndex(i, &cookie));

								if (cookie.get())
								{
									result += Utility::CookieToString(cookie.get());
									if (i != cookie_list_size - 1)
									{
										result += L",\n";
									}
								}
							}
							result += L"]";
						}
						LOG_TRACE << result;
					}
			return error_code;
				}
			).Get());
			THROW_IF_FAILED_MSG(hr, "function = % s, message = % s, hr = % d\n", __func__, std::system_category().message(hr).data(), hr);
			return true;
		}
		bool delete_all_cookies()
		{
			LOG_TRACE << __FUNCTION__;
			THROW_IF_NULL_ALLOC(cookieManager_);
			HRESULT hr = cookieManager_->DeleteAllCookies();
			THROW_IF_FAILED_MSG(hr, "function = % s, message = % s, hr = % d\n", __func__, std::system_category().message(hr).data(), hr);
			return true;
		}
		bool navigate_to(std::wstring_view url)
		{
			LOG_TRACE << __FUNCTION__;

			if (url.empty())
				THROW_IF_FAILED_MSG(E_INVALIDARG, "function = % s, message = % s, hr = % d\n", __func__, std::system_category().message(E_INVALIDARG).data(), E_INVALIDARG);

			std::wstring url_to_navigate(url);

			if (url_to_navigate.find(L"://") < 0)
			{
				if (url_to_navigate.length() > 1 && url_to_navigate[1] == ':')
					url_to_navigate = L"file://" + url_to_navigate;
				else
					url_to_navigate = L"http://" + url_to_navigate;
			}
			HRESULT hr = webView_->Navigate(url_to_navigate.c_str());
			THROW_IF_FAILED_MSG(hr, "function = % s, message = % s, hr = % d\n", __func__, std::system_category().message(hr).data(), hr);
			return true;
		}

		bool open_dev_tools()
		{
			THROW_IF_FAILED(webView_->OpenDevToolsWindow());
			return true;
		}

#pragma endregion webview2_implementation
	public:
		std::wstring								url_;
		std::wstring								browserDirectory_;
		std::wstring								userDataDirectory_;
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
		std::map<CallbackType, CallbackFunc>		m_callbacks;


	private:

		bool Navigate(std::wstring_view url, CallbackFunc onComplete)
		{
			LOG_TRACE << __FUNCTION__;
			THROW_IF_NULL_ALLOC(webView_);

			m_callbacks[CallbackType::NavigationCompleted] = onComplete;
			return (navigate_to(url));
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
				THROW_IF_WIN32_BOOL_FALSE(::IsWindow(pT->m_hWnd));

				LOG_TRACE << __FUNCTION__ << "m_hWnd=" << pT->m_hWnd;

				THROW_IF_FAILED(environment->QueryInterface(IID_PPV_ARGS(&webViewEnvironment_)));
				THROW_IF_FAILED(webViewEnvironment_->CreateCoreWebView2Controller(pT->m_hWnd, Microsoft::WRL::Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(this, &CDialogWebView2Impl::OnCreateWebViewControllerCompleted).Get()));
			}
			else
			{
				THROW_IF_FAILED_MSG(hr, "function = % s, message = % s, hr = % d\n", __func__, std::system_category().message(hr).data(), hr);
			}
			return S_OK;
		}
		/// <summary>
		/// Initialize the webview2
		/// </summary>
		bool InitWebView()
		{
			LOG_TRACE << __FUNCTION__ << " Using user data directory:" << userDataDirectory_.data();
			auto options = Microsoft::WRL::Make<CoreWebView2EnvironmentOptions>();
			THROW_IF_FAILED(options->put_AllowSingleSignOnUsingOSPrimaryAccount(TRUE));
			std::wstring langid(Utility::GetUserMUI());
			THROW_IF_FAILED(options->put_Language(langid.c_str()));	

			HRESULT hr = CreateCoreWebView2EnvironmentWithOptions(browserDirectory_.empty() ? nullptr : browserDirectory_.data(),
																  userDataDirectory_.data(), options.Get(),
											  Microsoft::WRL::Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(this, &CDialogWebView2Impl::OnCreateEnvironmentCompleted).Get());


			THROW_IF_FAILED_MSG(hr, "function = % s, message = % s, hr = % d\n", __func__, std::system_category().message(hr).data(), hr);
			return (true);
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
		bool ResizeToClientArea()
		{
			if (webController_)
			{
				T* pT = static_cast<T*>(this);
				THROW_IF_WIN32_BOOL_FALSE(::IsWindow(pT->m_hWnd));
				CRect bounds;
				pT->GetClientRect(&bounds);
				THROW_IF_FAILED(webController_->put_Bounds(bounds));
			}
			else
			{
				LOG_TRACE << __FUNCTION__ << "webController_ is nullptr";
			}
			return true;
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

			THROW_IF_NULL_ALLOC(controller);

			webController_ = controller;

			T* pT = static_cast<T*>(this);
			THROW_IF_WIN32_BOOL_FALSE(::IsWindow(pT->m_hWnd));

			CRect bounds;
			pT->GetClientRect(&bounds);
			webController_->put_Bounds(bounds);

			THROW_IF_FAILED(controller->get_CoreWebView2(&webView_));

			THROW_IF_FAILED(webView_->QueryInterface(&m_webviewEventSource2));
			THROW_IF_FAILED(webView_->QueryInterface(&m_webviewEventSource3));


			THROW_IF_FAILED(webView_->get_Settings(&webSettings_));
			THROW_IF_FAILED(RegisterEventHandlers());
			ResizeToClientArea();
			auto& callback = m_callbacks[CallbackType::CreationCompleted];
			THROW_IF_NULL_ALLOC(callback);
			RunAsync(callback);
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
					ATLTRACE("function=%s message=COREWEBVIEW2_WEB_ERROR_STATUS_DISCONNECTED\n", __func__);
				}
			}
			wil::unique_cotaskmem_string uri;
			webView_->get_Source(&uri);
			if (wcscmp(uri.get(), L"about:blank") == 0)
			{
				uri = wil::make_cotaskmem_string(L"");
			}

			auto& callback = m_callbacks[CallbackType::NavigationCompleted];
			if (callback != nullptr)
				RunAsync(callback);
			return hr;
		}
		HRESULT onNavigationStarting(ICoreWebView2* core_web_view2, ICoreWebView2NavigationStartingEventArgs* args)
		{
			wil::unique_cotaskmem_string uri;
			args->get_Uri(&uri);
			m_isNavigating = true;
			url_ = uri.get();

			auto& callback = m_callbacks[CallbackType::NavigationStarting];
			THROW_IF_NULL_ALLOC(callback);
			RunAsync(callback);
			return S_OK;
		}

		static void DumpHeaders(ICoreWebView2WebResourceRequestedEventArgs* args)
		{
			wil::com_ptr <ICoreWebView2WebResourceRequest>			 request = nullptr;
			wil::com_ptr<ICoreWebView2HttpHeadersCollectionIterator> it_headers = nullptr;
			wil::com_ptr <ICoreWebView2HttpRequestHeaders>			 headers = nullptr;

			auto hr = args->get_Request(&request);
			THROW_IF_FAILED_MSG(hr, "function=%s, message=%s, hr=%d\n", __func__, std::system_category().message(hr).data(), hr);
			hr = request->get_Headers(&headers);
			THROW_IF_FAILED_MSG(hr, "function=%s, message=%s, hr=%d\n", __func__, std::system_category().message(hr).data(), hr);
			hr = headers->GetIterator(&it_headers);
			THROW_IF_FAILED_MSG(hr, "function=%s, message=%s, hr=%d\n", __func__, std::system_category().message(hr).data(), hr);

			LPWSTR uri = nullptr;
			request->get_Uri(&uri);

			BOOL hasCurrent = FALSE;
			std::wstring result = L"[";

			while (SUCCEEDED(it_headers->get_HasCurrentHeader(&hasCurrent)) && hasCurrent)
			{
				wil::unique_cotaskmem_string name;
				wil::unique_cotaskmem_string value;

				CHECK_FAILURE(it_headers->GetCurrentHeader(&name, &value));
				result += L"{\"name\": " + Utility::EncodeQuote(name.get()) + L", \"value\": " + Utility::EncodeQuote(value.get()) + L"}";
				BOOL hasNext = FALSE;
				CHECK_FAILURE(it_headers->MoveNext(&hasNext));
				if (hasNext)
				{
					result += L", ";
				}
			}
			result += L"]";
			LOG_TRACE << "uri:" << uri << " headers:" << result;
		}

		static void DumpCookieHeaders(ICoreWebView2WebResourceRequestedEventArgs* args)
		{
			wil::com_ptr <ICoreWebView2WebResourceRequest>			 request = nullptr;
			wil::com_ptr <ICoreWebView2HttpRequestHeaders>			 headers = nullptr;

			auto hr = args->get_Request(&request);
			THROW_IF_FAILED_MSG(hr, "function=%s, message=%s, hr=%d\n", __func__, std::system_category().message(hr).data(), hr);
			hr = request->get_Headers(&headers);
			THROW_IF_FAILED_MSG(hr, "function=%s, message=%s, hr=%d\n", __func__, std::system_category().message(hr).data(), hr);

			LPWSTR uri = nullptr;
			request->get_Uri(&uri);

			BOOL cookies_header = FALSE;
			hr = headers->Contains(L"cookie", &cookies_header);
			if (cookies_header == TRUE && hr == S_OK)
			{
				wil::com_ptr <ICoreWebView2HttpHeadersCollectionIterator> it = nullptr;
				std::wstring result = L"[";
				hr = headers->GetHeaders(L"cookie", &it);
				if (hr == S_OK)
				{
					BOOL hasCurrent = FALSE;
					while (SUCCEEDED(it->get_HasCurrentHeader(&hasCurrent)) && hasCurrent)
					{
						wil::unique_cotaskmem_string name;
						wil::unique_cotaskmem_string value;




						CHECK_FAILURE(it->GetCurrentHeader(&name, &value));
						result += L"{\"name\": " + Utility::EncodeQuote(name.get())
							+ L", \"value\": " + Utility::EncodeQuote(value.get()) + L"}";

						std::wstring dd = name.get();

						std::wstring dd1 = value.get();




						BOOL hasNext = FALSE;
						CHECK_FAILURE(it->MoveNext(&hasNext));
						if (hasNext)
						{
							result += L", ";
						}
					}
				}
				result += L"]";
				LOG_TRACE << "uri:" << uri << " cookies:" << result;
			}
		}
		void hanle_authorization(ICoreWebView2WebResourceRequestedEventArgs* args)
		{
			wil::com_ptr <ICoreWebView2WebResourceRequest>			 request = nullptr;
			wil::com_ptr <ICoreWebView2HttpRequestHeaders>			 headers = nullptr;

			auto hr = args->get_Request(&request);
			THROW_IF_FAILED_MSG(hr, "function=%s, message=%s, hr=%d\n", __func__, std::system_category().message(hr).data(), hr);
			hr = request->get_Headers(&headers);
			THROW_IF_FAILED_MSG(hr, "function=%s, message=%s, hr=%d\n", __func__, std::system_category().message(hr).data(), hr);

			LPWSTR uri = nullptr;
			request->get_Uri(&uri);

			BOOL auth_header = FALSE;

			hr = headers->Contains(L"Authorization", &auth_header);
			if (auth_header == TRUE && hr == S_OK)
			{
				auto authV = new TCHAR[1000];
				hr = headers->GetHeader(L"Authorization", &authV);
				THROW_IF_FAILED_MSG(hr, "function=%s, message=%s, hr=%d\n", __func__, std::system_category().message(hr).data(), hr);
				LOG_TRACE << __FUNCTION__ << " name=Authorization" << " value=" << authV;
				auto& callback = m_callbacks[CallbackType::AuthenticationCompleted];
				THROW_IF_NULL_ALLOC_MSG(callback, "function=%s message=unable to create callback", __func__);
				RunAsync(callback);
			}
		}

		HRESULT onWebResourceRequested(ICoreWebView2* core_web_view2, ICoreWebView2WebResourceRequestedEventArgs* args)
		{
			DumpHeaders(args);
			DumpCookieHeaders(args);
			hanle_authorization(args);
			return S_OK;
		}
		HRESULT onResponseReceived(ICoreWebView2* core_web_view2, ICoreWebView2WebResourceResponseReceivedEventArgs* args)
		{
			int statusCode;

			wil::com_ptr<ICoreWebView2WebResourceRequest> webResourceRequest;
			CHECK_FAILURE(args->get_Request(&webResourceRequest));
			wil::com_ptr<ICoreWebView2WebResourceResponseView>webResourceResponse;
			CHECK_FAILURE(args->get_Response(&webResourceResponse));

			wil::com_ptr <ICoreWebView2HttpResponseHeaders> http_request_header;
			wil::com_ptr <ICoreWebView2HttpHeadersCollectionIterator> it_headers;
			wil::unique_cotaskmem_string					reasonPhrase;

			CHECK_FAILURE(webResourceResponse->get_Headers(&http_request_header));
			CHECK_FAILURE(webResourceResponse->get_StatusCode(&statusCode));
			CHECK_FAILURE(webResourceResponse->get_ReasonPhrase(&reasonPhrase));


			HRESULT hr = http_request_header->GetIterator(&it_headers);
			THROW_IF_FAILED_MSG(hr, "function=%s, message=%s, hr=%d\n", __func__, std::system_category().message(hr).data(), hr);

			BOOL hasCurrent = FALSE;
			std::wstring result = L"[";

			while (SUCCEEDED(it_headers->get_HasCurrentHeader(&hasCurrent)) && hasCurrent)
			{
				wil::unique_cotaskmem_string name;
				wil::unique_cotaskmem_string value;

				CHECK_FAILURE(it_headers->GetCurrentHeader(&name, &value));
				result += L"{\"name\": " + Utility::EncodeQuote(name.get()) + L", \"value\": " + Utility::EncodeQuote(value.get()) + L"}";
				BOOL hasNext = FALSE;
				CHECK_FAILURE(it_headers->MoveNext(&hasNext));
				if (hasNext)
				{
					result += L", ";
				}
			}
			result += L"]";
			LOG_TRACE << "response headers:" << result;

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

			THROW_IF_FAILED_MSG(hr, "function=%s, message=%s, hr=%d\n", __func__, std::system_category().message(hr).data(), hr);

			// NavigationCompleted handler
			hr = webView_->add_NavigationCompleted(Microsoft::WRL::Callback<ICoreWebView2NavigationCompletedEventHandler>([this](
				ICoreWebView2* core_web_view2,
				ICoreWebView2NavigationCompletedEventArgs* args) -> HRESULT
				{
					return (onNavigationCompleted(core_web_view2, args));
				}).Get(), &m_navigationCompletedToken);
			THROW_IF_FAILED_MSG(hr, "function=%s, message=%s, hr=%d\n", __func__, std::system_category().message(hr).data(), hr);


			// NavigationStarting handler
			hr = webView_->add_NavigationStarting(Microsoft::WRL::Callback<ICoreWebView2NavigationStartingEventHandler>([this](
				ICoreWebView2* core_web_view2,
				ICoreWebView2NavigationStartingEventArgs* args) -> HRESULT
				{
					return (onNavigationStarting(core_web_view2, args));
				}).Get(), &m_navigationStartingToken);
			THROW_IF_FAILED_MSG(hr, "function=%s, message=%s, hr=%d\n", __func__, std::system_category().message(hr).data(), hr);
			// Add request filter
			hr = webView_->AddWebResourceRequestedFilter(L"*", COREWEBVIEW2_WEB_RESOURCE_CONTEXT_ALL);
			THROW_IF_FAILED_MSG(hr, "function=%s, message=%s, hr=%d\n", __func__, std::system_category().message(hr).data(), hr);
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
			THROW_IF_FAILED_MSG(hr, "function=%s, message=%s, hr=%d\n", __func__, std::system_category().message(hr).data(), hr);

			if (!url_.empty())
				hr = webView_->Navigate(url_.c_str());
			else
				hr = webView_->Navigate(L"about:blank");
			return (hr);
		}

		void RunAsync(CallbackFunc callback)
		{
			T* pT = static_cast<T*>(this);
			THROW_IF_WIN32_BOOL_FALSE(::IsWindow(pT->m_hWnd));
			auto* task = new CallbackFunc(callback);
			pT->PostMessage(MSG_RUN_ASYNC_CALLBACK, reinterpret_cast<WPARAM>(task), 0);
		}
	};

};

