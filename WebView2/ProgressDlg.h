#pragma once

#include "resource.h"
#include "Thread.h"

class CProgressDlg : public CDialogImpl<CProgressDlg>, public CWinDataExchange<CProgressDlg>
{

public:
	enum { IDD = IDD_DIALOG_PROGRESS };

	BEGIN_MSG_MAP(CProgressDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
	END_MSG_MAP()

	BEGIN_DDX_MAP(CProgressDlg)
		DDX_CONTROL_HANDLE(IDC_PROGRESS_CONTROL, m_progress)
	END_DDX_MAP()

	bool onclicklisttimeractivated;
	long m_curSel;

	CProgressBarCtrl m_progress;

	CProgressDlg() {};

	CProgressDlg(std::wstring remote_url, std::wstring local_file);

public:
	std::wstring m_RemoteURL;
	std::wstring m_LocalFile;

protected:
	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	void OnStart();


};