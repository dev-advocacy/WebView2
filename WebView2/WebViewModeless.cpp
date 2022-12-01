#include "pch.h"
#include "resource.h"
#include "WebViewModeless.h"

CWebViewModeless::CWebViewModeless(std::wstring browerdirectory, std::wstring userdatedirectory, std::wstring url)
{
	userDataDirectory_ = userdatedirectory;
	url_ = url;
	browserDirectory_ = browerdirectory;
}


LRESULT CWebViewModeless::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	CenterWindow(GetParent());
	CDialogWebView2Impl_Init();
	return TRUE;
}
LRESULT CWebViewModeless::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	m_bModal == TRUE ? EndDialog(wID) : DestroyWindow();
	return 0;
}