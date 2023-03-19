#include "pch.h"
#include "resource.h"
#include "ListViewCtrlMultiline.h"
#include "CertificateDlg.h"


LRESULT CCertificateDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	CenterWindow(GetParent());


	//auto rect = RECT();
	//GetClientRect(&rect);
	//rect.bottom = 540; // 40x 13 = 520, + 20 for column header

	//m_logview.Create(*this, rect);

	//m_logview.InsertColumn(0, _T("Scoobies2"), LVCFMT_LEFT, -1, -1);
	//m_logview.InsertItem(0, _T("01 Willow is een test regel met een hoop tekens er achter"));
	//m_logview.InsertItem(1, _T("02 Buffy is een test regel met een hoop tekens er achter"));
	//m_logview.InsertItem(2, _T("03 Giles is een test regel met een hoop tekens er achter"));
	//m_logview.InsertItem(3, _T("04 Willow is een test regel met een hoop tekens er achter"));
	

	//DoDataExchange();

	auto rect = RECT();
	GetClientRect(&rect);
	rect.bottom = 540; // 40x 13 = 520, + 20 for column header

	m_listCertificate.Create(*this, rect);

	m_listCertificate.InsertColumn(0, _T("Scoobies2"), LVCFMT_LEFT, 150, -1);
	m_listCertificate.InsertItem(0, _T("01 Willow is een test regel met een hoop tekens er achter"));
	m_listCertificate.InsertItem(1, _T("02 Buffy is een test regel met een hoop tekens er achter"));
	m_listCertificate.InsertItem(2, _T("03 Giles is een test regel met een hoop tekens er achter"));
	m_listCertificate.InsertItem(3, _T("04 Willow is een test regel met een hoop tekens er achter"));

	//m_listCertificate.AddItem(0, 0, L"Subject");

	
	//DoDataExchange(FALSE);



	return TRUE;
}

LRESULT CCertificateDlg::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	EndDialog(wID);
	return 0;
}