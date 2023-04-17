#include "pch.h"
#include "resource.h"
#include "ClientCertificate.h"
#include "CertificateDlg.h"


// https://certtestdemo.azurewebsites.net/

CCertificateDlg::CCertificateDlg(std::vector<ClientCertificate> client_certificates, HWND hwnd_parent) : m_client_certificates(client_certificates), m_hwnd_parent(hwnd_parent)
{
}


LRESULT CCertificateDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	CenterWindow(GetParent());

	m_ImageList_certificate.Create(80, 64, TRUE | ILC_COLOR32, 1, 1);
	m_List_certificate.SubclassWindow(GetDlgItem(IDC_LIST_CERTIFICATE));
	m_List_certificate.SetItemHeight(0, 70);
	m_List_certificate.SetImageList(m_ImageList_certificate.m_hImageList, ILSIL_NORMAL);
	m_List_certificate.SetImageList(m_ImageList_certificate.m_hImageList, ILSIL_SELECTED);

	CSize size(16,16);
	m_ImageList_certificate.SetIconSize(size);

	auto hIconCert = LoadIcon(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDI_ICON_CERTIFICATE));	
	m_ImageList_certificate.AddIcon(hIconCert);

	auto hIconPin = LoadIcon(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDI_ICON_PIN));
	m_ImageList_certificate.AddIcon(hIconPin);

	for (auto client_certificate : m_client_certificates)
	{
		ILBITEM item = { 0 };
		item.mask = ILBIF_TEXT | ILBIF_IMAGE | ILBIF_SELIMAGE | ILBIF_STYLE | ILBIF_FORMAT;
		item.iItem = 0;
		item.iImage = 0;
		item.iSelImage = 0;
		item.iSelImage = 0;

		auto dd = client_certificate.DisplayName.get();
		auto ii = client_certificate.Issuer.get();
		auto ee = client_certificate.CertificateKind;
		auto ee1 = client_certificate.Subject.get();

		auto ee3 = client_certificate.ValidTo;

		std::wstring wstr = client_certificate.DisplayName.get();
		wstr += L"\n";
		wstr += client_certificate.Issuer.get();
		wstr += L"\n";
		wstr += UnixEpochToDateTime(client_certificate.ValidTo);

		

		item.pszText = const_cast<LPTSTR>(wstr.c_str());

		item.style = ILBS_IMGLEFT | ILBS_SELROUND;
		m_List_certificate.InsertItem(&item);
	
		
	}	
	SetWindowPos(this->m_hwnd_parent, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	return TRUE;
}


std::wstring CCertificateDlg::UnixEpochToDateTime(double value) 
{
	WCHAR rawResult[32] = {};
	std::time_t rawTime = std::time_t(value);
	struct tm timeStruct = {};
	gmtime_s(&timeStruct, &rawTime);
	_wasctime_s(rawResult, 32, &timeStruct);
	std::wstring result(rawResult);
	return result;
}

bool CCertificateDlg::AddCertificate(const std::wstring& name, const std::wstring& issuer, const std::wstring& expirationDate)
{
	return true;
}

LRESULT CCertificateDlg::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	set_selectedItem();
	EndDialog(wID);
	return wID;
}


void CCertificateDlg::set_selectedItem()
{
	m_selectedItem = m_List_certificate.GetCurSel();
}

int CCertificateDlg::get_selectedItem()
{
	return m_selectedItem;
}