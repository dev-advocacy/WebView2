#include "pch.h"
#include "resource.h"
#include "CertificateDlg.h"


LRESULT CCertificateDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	CenterWindow(GetParent());

	m_courseImageList.Create(80, 64, TRUE | ILC_COLOR32, 1, 1);


	m_courseList.SubclassWindow(GetDlgItem(IDC_LIST_CERTIFICATE));
	m_courseList.SetItemHeight(0, 70);
	m_courseList.SetImageList(m_courseImageList.m_hImageList, ILSIL_NORMAL);
	m_courseList.SetImageList(m_courseImageList.m_hImageList, ILSIL_SELECTED);

	CSize size(16,16);

	m_courseImageList.SetIconSize(size);

	auto hIconUp = LoadIcon(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME));	
	m_courseImageList.AddIcon(hIconUp);


	ILBITEM item = { 0 };
	item.mask = ILBIF_TEXT | ILBIF_IMAGE | ILBIF_SELIMAGE | ILBIF_STYLE | ILBIF_FORMAT;
	item.iItem = 0;
	item.iImage = 0;
	item.iSelImage = 0;
	item.pszText = const_cast<LPTSTR>(L"Gilles Guimard\nWindows Hello - MSIT2\n2/26/2023");
	item.style = ILBS_IMGLEFT | ILBS_SELROUND;
	item.format = DT_LEFT;
	m_courseList.InsertItem(&item);




	return TRUE;
}

LRESULT CCertificateDlg::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	EndDialog(wID);
	return 0;
}