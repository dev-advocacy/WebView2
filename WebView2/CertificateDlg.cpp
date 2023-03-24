#include "pch.h"
#include "resource.h"
#include "ClientCertificate.h"
#include "CertificateDlg.h"



CCertificateDlg::CCertificateDlg(std::vector<ClientCertificate> client_certificates, wil::com_ptr<ICoreWebView2Deferral> deferral) : m_client_certificates(client_certificates), m_deferral(deferral)
{
}

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

	auto hIconCert = LoadIcon(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDI_ICON_CERTIFICATE));	
	m_courseImageList.AddIcon(hIconCert);

	auto hIconPin = LoadIcon(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDI_ICON_PIN));
	m_courseImageList.AddIcon(hIconPin);

	for (auto client_certificate : m_client_certificates)
	{
		auto dd = client_certificate.DisplayName.get();
		auto ii = client_certificate.Issuer.get();
		auto ee = client_certificate.CertificateKind;
		auto ee1 = client_certificate.Subject.get();
	}

	m_deferral->Complete();

	//ILBITEM item = { 0 };
	//item.mask = ILBIF_TEXT | ILBIF_IMAGE | ILBIF_SELIMAGE | ILBIF_STYLE | ILBIF_FORMAT;
	//item.iItem = 0;
	//item.iImage = 0;
	//item.iSelImage = 0;
	//item.pszText = const_cast<LPTSTR>(L"Gilles Guimard\nWindows Hello - MSIT2\n2/26/2023");
	//item.style = ILBS_IMGLEFT | ILBS_SELROUND;
	//item.format = DT_LEFT;
	//m_courseList.InsertItem(&item);

	return TRUE;
}

bool CCertificateDlg::AddCertificate(const std::wstring& name, const std::wstring& issuer, const std::wstring& expirationDate)
{
	
	return true;
}

LRESULT CCertificateDlg::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	EndDialog(wID);
	return 0;
}