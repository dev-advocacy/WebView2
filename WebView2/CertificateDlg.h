#pragma once

#include "ListViewCtrlMultiline.h"
#include "LogView.h"

class CCertificateDlg : public CDialogImpl<CCertificateDlg>,
						public CWinDataExchange<CCertificateDlg>
{
public:
	enum { IDD = IDD_DIALOG_CERTIFICATE };

	BEGIN_DDX_MAP(CCertificateDlg)
		//DDX_CONTROL_HANDLE(IDC_LIST_CERT, m_listCertificate)
	END_DDX_MAP()

	BEGIN_MSG_MAP(CCertificateDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
		DEFAULT_REFLECTION_HANDLER()
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

	// Handler prototypes (uncomment arguments if needed):
	//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

private:
	ListViewCtrlMultiline m_listCertificate;
	CLogView m_logview;
};


