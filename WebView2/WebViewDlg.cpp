#include "pch.h"
#include "resource.h"
#include "WebViewDlg.h"


CDlgWebView2::CDlgWebView2(std::wstring browerdirectory, std::wstring userdatedirectory, std::wstring url)
{
	userDataDirectory_ = userdatedirectory;
	url_ = url;
	browserDirectory_ = browerdirectory;
}


LRESULT CDlgWebView2::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	CenterWindow(GetParent());
	CDialogWebView2Impl_Init();
	return TRUE;
}
LRESULT CDlgWebView2::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	m_bModal == TRUE ? EndDialog(wID) : DestroyWindow(); 
	return 0;
}

