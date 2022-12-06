#include "pch.h"
#include "ProgressDlg.h"

//static int CALLBACK;

class CDCallback : public IBindStatusCallback
{
public:
	CProgressDlg* pDialog;

	STDMETHOD(OnStartBinding) (DWORD dwReserved, IBinding __RPC_FAR* pib) { return E_NOTIMPL; }
	STDMETHOD(GetPriority) (LONG __RPC_FAR* pnPriority) { return E_NOTIMPL; }
	STDMETHOD(OnLowResource) (DWORD reserved) { return E_NOTIMPL; }
	STDMETHOD(OnProgress) (ULONG ulProgress, ULONG ulProgressMax, ULONG ulStatusCode, LPCWSTR wszStatusText)
	{
		if (pDialog->m_hWnd != nullptr)
		{
			if (::IsWindow(pDialog->m_hWnd))
			{
				if (::IsWindow(pDialog->m_progress.m_hWnd))
				{
					pDialog->m_progress.SetRange(0, (static_cast<int>(ulProgressMax) / 1024));
					pDialog->m_progress.SetPos((static_cast<int>(ulProgress) / 1024));
				}
			}
		}
		return S_OK;
	}

	STDMETHOD(OnStopBinding) (HRESULT hresult, LPCWSTR szError) { return E_NOTIMPL; }
	STDMETHOD(GetBindInfo)(DWORD __RPC_FAR* grfBINDF, BINDINFO __RPC_FAR* pbindinfo) { return E_NOTIMPL; }
	STDMETHOD(OnDataAvailable)(DWORD grfBSCF, DWORD dwSize, FORMATETC __RPC_FAR* pformatetc, STGMEDIUM __RPC_FAR* pstgmed) { return E_NOTIMPL; }
	STDMETHOD(OnObjectAvailable)(REFIID riid, IUnknown __RPC_FAR* punk) { return E_NOTIMPL; }
	STDMETHOD_(ULONG, AddRef) () { return 0; }
	STDMETHOD_(ULONG, Release) () { return 0; }
	STDMETHOD(QueryInterface) (REFIID riid, void __RPC_FAR* __RPC_FAR* ppvObject) { return E_NOTIMPL; }
};

void installEdgeSilent(CProgressDlg* pDialog)
{
	SHELLEXECUTEINFO shExInfo = { 0 };
	shExInfo.cbSize = sizeof(shExInfo);
	shExInfo.fMask = SEE_MASK_ASYNCOK;
	shExInfo.hwnd = 0;
	shExInfo.lpVerb = L"runas";
	shExInfo.lpFile = pDialog->m_LocalFile.c_str();
	shExInfo.lpParameters = L" /install";
	shExInfo.lpDirectory = 0;
	shExInfo.nShow = 0;
	shExInfo.hInstApp = 0;

	BOOL success = ShellExecuteEx(&shExInfo);

	if (success)
	{
		if (shExInfo.hProcess != nullptr)
		{
			WaitForSingleObject(shExInfo.hProcess, INFINITE);
		}
	}
}

void DownloadThread(void* pData)
{
	CProgressDlg  *pDialog = (CProgressDlg*)pData;
	CDCallback Callback;
	Callback.pDialog = pDialog;

	HRESULT Result = URLDownloadToFile(NULL, pDialog->m_RemoteURL.c_str(), pDialog->m_LocalFile.c_str(), 0, &Callback);
	if (SUCCEEDED(Result))
	{
		installEdgeSilent(pDialog);
		pDialog->EndDialog(0);
	}	
}



CProgressDlg::CProgressDlg(std::wstring remote_url, std::wstring local_file) : m_RemoteURL(remote_url), m_LocalFile(local_file)
{

}




LRESULT CProgressDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	DoDataExchange();
	CenterWindow(GetParent());
	OnStart();
	return TRUE;
}

LRESULT CProgressDlg::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	EndDialog(wID);
	return 0;
}

// CExchangeDlg message handlers

void CProgressDlg::OnStart()
{

	_beginthread(DownloadThread, 0, (void*)this);
}

