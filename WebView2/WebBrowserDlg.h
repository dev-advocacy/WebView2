#pragma once
#include "pch.h"
#include "resource.h"
#include "logger.h"
#include "Utility.h"


namespace WebView2
{
	template <class T>
	class CompositionHost1
	{
	private:
		wil::com_ptr<ICoreWebView2Controller>				m_controller = nullptr;
		wil::com_ptr<ICoreWebView2CompositionController>	m_compositionController = nullptr;
		wil::com_ptr<IDCompositionDevice>					m_dcompDevice = nullptr;
		wil::com_ptr<IDCompositionTarget>					m_dcompHwndTarget = nullptr;
		wil::com_ptr<IDCompositionVisual>					m_dcompRootVisual = nullptr;
		wil::com_ptr<IDCompositionVisual>					m_dcompWebViewVisual = nullptr;
		CRect m_webViewBounds = {};
		float m_webViewRatio = 1.0f;
		float m_webViewScale = 1.0f;
		bool m_isTrackingMouse = false;
		bool m_isCapturingMouse = false;
		EventRegistrationToken m_cursorChangedToken = {};

	public:
		// Message map and handlers
		BEGIN_MSG_MAP(CompositionHost1)
			MESSAGE_HANDLER(WM_SIZE, OnSize)
			MESSAGE_RANGE_HANDLER(WM_MOUSEFIRST, WM_MOUSELAST, onMouseEvent)
		END_MSG_MAP()

		#pragma region windows_event
		LRESULT onMouseEvent(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
		{
			T* pT = static_cast<T*>(this);

			// Manually relay mouse messages to the WebView
			if (m_dcompDevice)
			{
				POINT point;
				POINTSTOPOINT(point, lParam);
				if (message == WM_MOUSEWHEEL || message == WM_MOUSEHWHEEL)
				{
					LOG_TRACE << __FUNCTION__ << " WM_MOUSEWHEEL " << "WM_MOUSEHWHEEL";

					// Mouse wheel messages are delivered in screen coordinates.
					// SendMouseInput expects client coordinates for the WebView, so convert
					// the point from screen to client.
					::ScreenToClient(pT->m_hWnd, &point);
				}
				// Send the message to the WebView if the mouse location is inside the
				// bounds of the WebView, if the message is telling the WebView the
				// mouse has left the client area, or if we are currently capturing
				// mouse events.
				bool isMouseInWebView = PtInRect(&m_webViewBounds, point);
				if (isMouseInWebView || message == WM_MOUSELEAVE || m_isCapturingMouse)
				{
					DWORD mouseData = 0;

					switch (message)
					{
					case WM_MOUSEWHEEL:
					case WM_MOUSEHWHEEL:
						mouseData = GET_WHEEL_DELTA_WPARAM(wParam);
						break;
					case WM_XBUTTONDBLCLK:
					case WM_XBUTTONDOWN:
					case WM_XBUTTONUP:
						mouseData = GET_XBUTTON_WPARAM(wParam);
						break;
					case WM_MOUSEMOVE:
						if (!m_isTrackingMouse)
						{
							// WebView needs to know when the mouse leaves the client area
							// so that it can dismiss hover popups. TrackMouseEvent will
							// provide a notification when the mouse leaves the client area.
							TrackMouseEvents(TME_LEAVE);
							m_isTrackingMouse = true;
						}
						break;
					case WM_MOUSELEAVE:
						m_isTrackingMouse = false;
						break;
					}

					// We need to capture the mouse in case the user drags the
					// mouse outside of the window bounds and we still need to send
					// mouse messages to the WebView process. This is useful for
					// scenarios like dragging the scroll bar or panning a map.
					// This is very similar to the Pointer Message case where a
					// press started inside of the WebView.
					if (message == WM_LBUTTONDOWN || message == WM_MBUTTONDOWN ||
						message == WM_RBUTTONDOWN || message == WM_XBUTTONDOWN)
					{
						if (isMouseInWebView && ::GetCapture() != pT->m_hWnd)
						{
							m_isCapturingMouse = true;
							::SetCapture(pT->m_hWnd);
						}
					}
					else if (message == WM_LBUTTONUP || message == WM_MBUTTONUP ||
						message == WM_RBUTTONUP || message == WM_XBUTTONUP)
					{
						if (::GetCapture() == pT->m_hWnd)
						{
							m_isCapturingMouse = false;
							::ReleaseCapture();
						}
					}

					// Adjust the point from app client coordinates to webview client coordinates.
					// WM_MOUSELEAVE messages don't have a point, so don't adjust the point.
					if (message != WM_MOUSELEAVE)
					{
						point.x -= m_webViewBounds.left;
						point.y -= m_webViewBounds.top;
					}

					m_compositionController->SendMouseInput(
						static_cast<COREWEBVIEW2_MOUSE_EVENT_KIND>(message),
						static_cast<COREWEBVIEW2_MOUSE_EVENT_VIRTUAL_KEYS>(GET_KEYSTATE_WPARAM(wParam)),
						mouseData, point);
					return true;
				}
				else if (message == WM_MOUSEMOVE && m_isTrackingMouse)
				{
					// When the mouse moves outside of the WebView, but still inside the app
					// turn off mouse tracking and send the WebView a leave event.
					m_isTrackingMouse = false;
					TrackMouseEvents(TME_LEAVE | TME_CANCEL);
					pT->onMouseEvent(WM_MOUSELEAVE, 0, 0, bHandled);
				}
			}
			return 0L;
		}
		/// <summary>
		/// Windows event to Resize the webview2
		/// </summary>
		LRESULT OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
		{
			//LOG_TRACE << __FUNCTION__;
			T* pT = static_cast<T*>(this);
			if (::IsWindow(pT->m_hWnd))
			{
				RECT availableBounds = { 0 };
				GetClientRect(pT->m_hWnd, &availableBounds);
				put_bounds(availableBounds);

			}
			return 0;
		}
#pragma endregion windows_event

	public:
		CompositionHost1() = default;
		~CompositionHost1() {}
		/**
		 * \brief : Initialize the class
		 * \param hwnd 
		 * \param controller 
		 * \param compositionController 
		 * \return S_OK or a failure
		 */
		HRESULT initialize(const HWND hwnd, wil::com_ptr<ICoreWebView2Controller> controller, wil::com_ptr<ICoreWebView2CompositionController> compositionController)
		{
			LOG_TRACE << __FUNCTION__;
			if (controller && compositionController && IsWindow(hwnd))
			{
				m_controller = controller;
				m_compositionController = compositionController;
				RETURN_IF_FAILED(InitializeComposition(hwnd));
				RETURN_IF_FAILED(InitializeCursorCapture(hwnd));
			}
			else
			{
				RETURN_IF_FAILED_MSG(ERROR_INVALID_PARAMETER, "function = % s, message = % s, hr = % d", __func__, std::system_category().message(ERROR_INVALID_PARAMETER).data(), ERROR_INVALID_PARAMETER);
			}
			return S_OK;
		}
		void put_bounds(RECT bounds)
		{
			m_webViewBounds = bounds;
			resize_web_view();
		}
		CRect get_bounds() const
		{
			LOG_TRACE << __FUNCTION__;
			CRect bounds={0};
			HRESULT result = m_controller->get_Bounds(&bounds);
			if (SUCCEEDED(result))
			{
				LOG_TRACE << "left=" << bounds.left << " top=" << bounds.top << " width="  << bounds.Width() << " height=" <<  bounds.Height();
			}
			return bounds;
		}
	private:
		/**
		 * \brief : Initialize Microsoft DirectComposition
		 * \param hwnd
		 * \return S_OK or the failure
		 */
		HRESULT InitializeComposition(HWND hwnd)
		{
			RETURN_IF_FAILED(DCompositionCreateDevice2(nullptr, IID_PPV_ARGS(&m_dcompDevice)));
			RETURN_IF_FAILED(m_dcompDevice->CreateTargetForHwnd(hwnd, TRUE, &m_dcompHwndTarget));
			RETURN_IF_FAILED(m_dcompDevice->CreateVisual(&m_dcompRootVisual));
			RETURN_IF_FAILED(m_dcompHwndTarget->SetRoot(m_dcompRootVisual.get()));
			RETURN_IF_FAILED(m_dcompDevice->CreateVisual(&m_dcompWebViewVisual));
			RETURN_IF_FAILED(m_dcompRootVisual->AddVisual(m_dcompWebViewVisual.get(), TRUE, nullptr));
			RETURN_IF_FAILED(m_compositionController->put_RootVisualTarget(m_dcompWebViewVisual.get()));
			RETURN_IF_FAILED(m_dcompDevice->Commit());
			return S_OK;
		}
		/**
		 * \brief resize the WebView2 control
		 */
		void resize_web_view() const
		{
			//LOG_TRACE << __FUNCTION__;
			if (m_controller)
			{
				const SIZE webViewSize = { static_cast<LONG>((m_webViewBounds.right - m_webViewBounds.left) * m_webViewRatio * m_webViewScale),static_cast<LONG>((m_webViewBounds.bottom - m_webViewBounds.top) * m_webViewRatio * m_webViewScale) };
				CRect desiredBounds = m_webViewBounds;
				desiredBounds.bottom = static_cast<LONG>(webViewSize.cy + m_webViewBounds.top);
				desiredBounds.right = static_cast<LONG>(webViewSize.cx + m_webViewBounds.left);
				m_controller->put_Bounds(desiredBounds);
			}
		}
		/**
		* \brief : forward the mouse change to the WebView2
		* \param hwnd
		* \return S_OK or the failure
		*/
		HRESULT InitializeCursorCapture(HWND hwnd)
		{
			LOG_DEBUG << __FUNCTION__;
			RETURN_IF_FAILED(
				m_compositionController->add_CursorChanged(Microsoft::WRL::Callback<
					ICoreWebView2CursorChangedEventHandler>([this, hwnd](ICoreWebView2CompositionController* sender,
						IUnknown* args)-> HRESULT
						{
							HRESULT hr = S_OK;
			UINT32 cursor_id;
			m_compositionController->get_SystemCursorId(&cursor_id);
			HCURSOR cursor = ::LoadCursor(nullptr, MAKEINTRESOURCE(cursor_id));
			if (cursor == nullptr)
			{
				hr = HRESULT_FROM_WIN32(GetLastError());
			}

			if (SUCCEEDED(hr))
			{
				SetClassLongPtr(hwnd, GCLP_HCURSOR, (LONG_PTR)cursor);
			}
			return hr;
						}
			).Get(), &m_cursorChangedToken));
			return S_OK;
		}
		void TrackMouseEvents(DWORD mouseTrackingFlags)
		{
			T* pT = static_cast<T*>(this);

			TRACKMOUSEEVENT tme;
			tme.cbSize = sizeof(tme);
			tme.dwFlags = mouseTrackingFlags;
			tme.hwndTrack = pT->m_hWnd;
			tme.dwHoverTime = 0;
			::TrackMouseEvent(&tme);
		}
	};


	template <class T>
	class CWebView2Impl2 : public CompositionHost1<T>
	{
	public:
		std::wstring										m_url;
		std::wstring										browserDirectory_;
		std::wstring										userDataDirectory_;
		bool												m_isModal = false;
	private:
		HWND												m_hwnd = nullptr;
		wil::com_ptr<ICoreWebView2Environment>				m_webViewEnvironment = nullptr;
		wil::com_ptr<ICoreWebView2CompositionController>	m_compositionController = nullptr;
		wil::com_ptr<ICoreWebView2Controller>				m_controller = nullptr;
		wil::com_ptr<ICoreWebView2>							m_webView = nullptr;
	public:
		// Message map and handlers
		BEGIN_MSG_MAP(CWebView2Impl2)
			CHAIN_MSG_MAP(CompositionHost1<T>)
			MESSAGE_HANDLER(WM_CREATE, OnCreate)
			MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		END_MSG_MAP()

		CWebView2Impl2() = default;
		CWebView2Impl2(std::wstring brower_directory, std::wstring user_data_directory, std::wstring url)
		{
			if (!url.empty())
				m_url = url;
			if (!brower_directory.empty())
				browserDirectory_ = brower_directory;
			if (!user_data_directory.empty())
				userDataDirectory_ = user_data_directory;
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
				HRESULT hr = InitializeWebView();
				if (FAILED(hr))
				{
					RETURN_IF_FAILED_MSG(hr, "function = % s, message = % s, hr = % d", __func__, std::system_category().message(hr).data(), hr);
				}
				m_isModal = true;
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
	private:

		HRESULT OnCreateCoreWebView2ControllerCompleted(HRESULT result, ICoreWebView2CompositionController* compositionController)
		{
			LOG_TRACE << __FUNCTION__;
			if (result != S_OK)
				return result;
			m_compositionController = compositionController;
			RETURN_IF_FAILED(m_compositionController->QueryInterface(IID_PPV_ARGS(&m_controller)));
			RETURN_IF_FAILED(m_controller->get_CoreWebView2(&m_webView));
			RETURN_IF_FAILED(m_controller->put_IsVisible(true));
			RETURN_IF_FAILED(m_webView->Navigate(m_url.c_str()));
			RETURN_IF_FAILED((static_cast<T*>(this))->initialize(m_hwnd, m_controller, m_compositionController));
			CRect bounds;
			GetClientRect(m_hwnd , &bounds);
			(static_cast<T*>(this))->put_bounds(bounds);
			return S_OK;
		}

		HRESULT InitializeWebView()
		{
			auto options = Microsoft::WRL::Make<CoreWebView2EnvironmentOptions>();
			HRESULT hr = CreateCoreWebView2EnvironmentWithOptions(
				 browserDirectory_.empty() ? nullptr : browserDirectory_.data(),
									userDataDirectory_.data(),
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





