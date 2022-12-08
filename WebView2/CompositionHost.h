#pragma once

namespace WebView2
{
    class CompositionHost
    {
    public:
        CompositionHost() = default;
        ~CompositionHost();
        void OnMouseMessage(UINT message, WPARAM wParam, LPARAM lParam);
        void SetBounds(CRect bounds);
        void Initialize(HWND hwnd, wil::com_ptr<ICoreWebView2Controller> controller, wil::com_ptr<ICoreWebView2CompositionController> compositionController);

    private:
        void EnsureDispatcherQueue();
        void CreateDesktopWindowTarget(HWND window);
        void CreateCompositionRoot();
        HRESULT CreateWebViewVisual();
        void DestroyWinCompVisualTree();
        void AddElement();
        HRESULT UpdateVisual(POINT point, UINT message, WPARAM wParam);
        winrt::Windows::UI::Composition::ContainerVisual FindVisual(POINT point);
        void CreateVisuals();
        winrt::Windows::UI::Color RandomBlue();
        void SetWebViewVisualBounds();
        void ResizeAllVisuals();


        wil::com_ptr<ICoreWebView2Controller> m_controller;
        wil::com_ptr<ICoreWebView2CompositionController> m_compositionController;

        winrt::Windows::UI::Composition::Compositor m_compositor{ nullptr };
        winrt::Windows::System::DispatcherQueueController m_dispatcherQueueController{ nullptr };
        winrt::Windows::UI::Composition::Desktop::DesktopWindowTarget m_target{ nullptr };
        winrt::Windows::UI::Composition::ContainerVisual m_rootVisual{ nullptr };
        winrt::Windows::UI::Composition::ContainerVisual m_webViewVisual{ nullptr };
        CRect m_appBounds = {};
    };
}