// View.cpp : implementation of the CView class
//
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "resource.h"

#include "WebView2.h"

/// <summary>
/// TODO : validate parameters
/// </summary>
/// <param name="browerdirectory"></param>
/// <param name="userdatedirectory"></param>
/// <param name="url"></param>
CWebView2::CWebView2(std::wstring browerdirectory, std::wstring userdatedirectory, std::wstring url)
{
	userDataDirectory_ = userdatedirectory;
	url_ = url;
	browserDirectory_ = browerdirectory;
}
CWebView2::~CWebView2()
{

}
BOOL CWebView2::PreTranslateMessage(MSG* pMsg)
{
	if (CWebView2Impl<CWebView2>::PreTranslateMessage(pMsg))
		return TRUE;

	return 0L;
}
LRESULT CWebView2::OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CPaintDC dc(m_hWnd);
	return 0L;
}
void CWebView2::CreationCompleted()
{
	LOG_TRACE << __FUNCTION__;
}
void CWebView2::NavigationCompleted(std::wstring url)
{
	LOG_TRACE << __FUNCTION__  << " url=" << url.c_str();
}
void CWebView2::AuthenticationCompleted()
{
	LOG_TRACE << __FUNCTION__;
}
