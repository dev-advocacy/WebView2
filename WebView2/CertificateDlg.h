#pragma once

#include "LogView.h"
#include "ImageListBox.h"


class CCertificateDlg : public CDialogImpl<CCertificateDlg>,
						public CWinDataExchange<CCertificateDlg>
{
public:
	enum { IDD = IDD_DIALOG_CERTIFICATE };

	BEGIN_MSG_MAP(CCertificateDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
		DEFAULT_REFLECTION_HANDLER()
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

public:
	CCertificateDlg() = delete;
	CCertificateDlg(std::vector<ClientCertificate> client_certificates, HWND hwnd_parent);
	bool AddCertificate(const std::wstring& name, const std::wstring& issuer, const std::wstring& expirationDate);
	int get_selectedItem();

private:
		std::wstring	UnixEpochToDateTime(double value);
		void			set_selectedItem();
private:
	CImageListBoxCtrl				m_List_certificate;
	CImageList						m_ImageList_certificate;
	std::vector<ClientCertificate> 	m_client_certificates;
	int								m_selectedItem = -1;								
	HWND							m_hwnd_parent = nullptr;
};