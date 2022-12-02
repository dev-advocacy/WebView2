#pragma once

#include "WebView2Impl.h"

class CWebViewModeless : public CDialogImpl<CWebViewModeless>, public WebView2::CWebView2Impl<CWebViewModeless>
{
public:
	enum { IDD = IDD_DIALOG_WEB_VIEW };
	CWebViewModeless(std::wstring browerdirectory, std::wstring userdatedirectory, std::wstring url);
	CWebViewModeless() {}

	BEGIN_MSG_MAP(CWebViewModeless)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
		CHAIN_MSG_MAP(WebView2::CWebView2Impl<CWebViewModeless>)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
};
