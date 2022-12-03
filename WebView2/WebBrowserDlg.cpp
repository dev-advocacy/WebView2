
// WebBrowserDlg.cpp : implementation file
//



#include "pch.h"
#include "framework.h"
#include "WebBrowserDlg.h"
#include "ViewComponent.h"
#include <Shellapi.h>


// CWebBrowserDlg dialog

static constexpr UINT s_runAsyncWindowMessage = WM_APP;

// CEdgeBrowserAppDlg dialog



LRESULT CWebBrowserDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    CenterWindow(GetParent());
    HRESULT hresult = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

    SetWindowLongPtr(GWLP_USERDATA, (LONG_PTR)this);

    
    // TODO: Add extra initialization here
    InitializeWebView();

    return TRUE;
}
LRESULT CWebBrowserDlg::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{

    EndDialog(wID);
    return 0;
}



void CWebBrowserDlg::RunAsync(std::function<void()> callback)
{
    auto* task = new std::function<void()>(callback);
    PostMessage(s_runAsyncWindowMessage, reinterpret_cast<WPARAM>(task), 0);
}

void CWebBrowserDlg::InitializeWebView()
{

    CloseWebView();
    m_dcompDevice = nullptr;

    m_wincompCompositor = nullptr;

    HRESULT hr2 = DCompositionCreateDevice2(nullptr, IID_PPV_ARGS(&m_dcompDevice));
    if (!SUCCEEDED(hr2))
    {
        return;
    }

    LPCWSTR subFolder = nullptr;
    auto options = Microsoft::WRL::Make<CoreWebView2EnvironmentOptions>();
    options->put_AllowSingleSignOnUsingOSPrimaryAccount(FALSE);


    HRESULT hr = CreateCoreWebView2EnvironmentWithOptions(subFolder, nullptr, options.Get(), Microsoft::WRL::Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(this, &CWebBrowserDlg::OnCreateEnvironmentCompleted).Get());
    if (!SUCCEEDED(hr))
    {
        if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
        {

        }
        else
        {
            
        }
    }
}

HRESULT CWebBrowserDlg::DCompositionCreateDevice2(IUnknown* renderingDevice, REFIID riid, void** ppv)
{
    HRESULT hr = E_FAIL;
    static decltype(::DCompositionCreateDevice2)* fnCreateDCompDevice2 = nullptr;
    if (fnCreateDCompDevice2 == nullptr)
    {
        HMODULE hmod = ::LoadLibraryEx(L"dcomp.dll", nullptr, 0);
        if (hmod != nullptr)
        {
            fnCreateDCompDevice2 = reinterpret_cast<decltype(::DCompositionCreateDevice2)*>(
                ::GetProcAddress(hmod, "DCompositionCreateDevice2"));
        }
    }
    if (fnCreateDCompDevice2 != nullptr)
    {
        hr = fnCreateDCompDevice2(renderingDevice, riid, ppv);
    }
    return hr;
}

void CWebBrowserDlg::OnSize(UINT a, int b, int c)
{
    ResizeEverything();
}

void CWebBrowserDlg::CloseWebView(bool cleanupUserDataFolder)
{

    if (m_controller)
    {
        m_controller->Close();
        m_controller = nullptr;
        m_webView = nullptr;
    }
    m_webViewEnvironment = nullptr;
    if (cleanupUserDataFolder)
    {
        //Clean user data        
    }
}

HRESULT CWebBrowserDlg::OnCreateEnvironmentCompleted(HRESULT result, ICoreWebView2Environment* environment)
{
    m_webViewEnvironment = environment;
    m_webViewEnvironment->CreateCoreWebView2Controller(this->m_hWnd, Microsoft::WRL::Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(this, &CWebBrowserDlg::OnCreateCoreWebView2ControllerCompleted).Get());

    return S_OK;
}

HRESULT CWebBrowserDlg::OnCreateCoreWebView2ControllerCompleted(HRESULT result, ICoreWebView2Controller* controller)
{
    if (result == S_OK)
    {
        m_controller = controller;
        wil::com_ptr<ICoreWebView2> coreWebView2;
        m_controller->get_CoreWebView2(&coreWebView2);
        coreWebView2.query_to(&m_webView);


        HRESULT hresult = m_webView->Navigate(L"https://google.com");

        if (hresult == S_OK)
        {
            ResizeEverything();
        }
    }
    else
    {
        
    }
    return S_OK;
}

void CWebBrowserDlg::ResizeEverything()
{
    RECT availableBounds = { 0 };
    GetClientRect(&availableBounds);
    m_controller->put_Bounds(availableBounds);
}