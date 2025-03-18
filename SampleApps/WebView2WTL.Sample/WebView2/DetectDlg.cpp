#include "pch.h"
#include "resource.h"
#include "DetectDlg.h"

LRESULT CDetectDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// load ddx values
	

	auto p = GetDlgItem(IDC_COMBO_EDGE_VERSION);
	if (!p)
	{
		return 0;
	}
	CComboBox* pComPortSelection = reinterpret_cast<CComboBox*>(&p);
	
	GetVersion();

	for (auto it = m_edge_versions.begin(); it != m_edge_versions.end(); ++it)
	{
		pComPortSelection->AddString(it->m_name.c_str());
	}


	if (!m_edge_versions.empty())
	{
		pComPortSelection->SetCurSel(0);
	}
	CString str;
	pComPortSelection->GetWindowTextW(str);
	std::wstring selected_version = str.GetString();



	for (auto it = m_edge_versions.begin(); it != m_edge_versions.end(); ++it)
	{
		if (wcscmp(it->m_name.c_str(), selected_version.c_str()) == 0)
		{
			m_information.Format(L"Name: %s\r\nVersion: %s\r\nInstall Location: %s", it->m_name.c_str(), it->m_version.c_str(), it->m_install_location.c_str());
			DoDataExchange(FALSE);
			break;
		}
	}
	CenterWindow(GetParent());
	return TRUE;
}

LRESULT CDetectDlg::OnCbnSelChange(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	auto p = GetDlgItem(IDC_COMBO_EDGE_VERSION);
	if (!p)
	{
		return 0;
	}
	CComboBox* pComPortSelection = reinterpret_cast<CComboBox*>(&p);

	// Get the selected item use  CSimpleString
	CString str;
	pComPortSelection->GetWindowTextW(str);
	std::wstring selected_version = str.GetString();

	for (auto it = m_edge_versions.begin(); it != m_edge_versions.end(); ++it)
	{
		if (wcscmp(it->m_name.c_str(), selected_version.c_str()) == 0)
		{
			m_information.Format(L"Name: %s\r\nVersion: %s\r\nInstall Location: %s", it->m_name.c_str(), it->m_version.c_str(), it->m_install_location.c_str());
			DoDataExchange(FALSE);
			break;
		}
	}
	return 0;
}

LRESULT CDetectDlg::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	EndDialog(wID);
	return 0;
}

void CDetectDlg::GetVersion()
{
	std::vector<std::wstring> keys = {
		L"SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Microsoft EdgeWebView",
		L"SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Microsoft Edge Beta",
		L"SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Microsoft Edge Dev",
	};
	for (const auto& key : keys)
	{
		GetVersionFromRegistry(HKEY_LOCAL_MACHINE, key);
		GetVersionFromRegistry(HKEY_CURRENT_USER, key);
	}	
}
void CDetectDlg::GetVersionFromRegistry(HKEY key, std::wstring key_entry)
{
	CRegKey regKey;
	// Open the registry key
	if (regKey.Open(key, key_entry.c_str(), KEY_READ) == ERROR_SUCCESS)
	{
		// Read the version value
		DWORD dwSize = 0;
		regKey.QueryStringValue(L"DisplayVersion", nullptr, &dwSize);
		std::wstring version(dwSize, L'\0');
		regKey.QueryStringValue(L"DisplayVersion", &version[0], &dwSize);

		//get the display name
		dwSize = 0;
		regKey.QueryStringValue(L"DisplayName", nullptr, &dwSize);
		std::wstring display_name(dwSize, L'\0');
		regKey.QueryStringValue(L"DisplayName", &display_name[0], &dwSize);

		// Get the install location
		dwSize = 0;
		regKey.QueryStringValue(L"InstallLocation", nullptr, &dwSize);
		std::wstring install_location(dwSize, L'\0');
		regKey.QueryStringValue(L"InstallLocation", &install_location[0], &dwSize);

		EdgeInfomation edge(display_name, version, install_location);
		// Add the version to the list
		m_edge_versions.push_back(edge);
		
		
	}
}
