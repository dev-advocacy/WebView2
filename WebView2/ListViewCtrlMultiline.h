#pragma once

//https://github.com/gix/foo_scrobble/blob/bc9e53db898f3293240a2477732ef909cb9477eb/sdk/libPPUI/DarkMode.cpp#L702

class ListViewCtrlMultiline : public CWindowImpl<ListViewCtrlMultiline, CListViewCtrl,
	CWinTraitsOR<
	LVS_OWNERDRAWFIXED | LVS_REPORT | LVS_OWNERDATA | LVS_NOSORTHEADER | LVS_SHOWSELALWAYS,
	LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_HEADERDRAGDROP>>,
	public WTL::COwnerDraw<ListViewCtrlMultiline>

{
	public:	

		DECLARE_WND_SUPERCLASS(nullptr, CListViewCtrl::GetWndClassName())

	BEGIN_MSG_MAP_EX(ListViewCtrlMultiline)
			CHAIN_MSG_MAP_ALT(COwnerDraw<ListViewCtrlMultiline>, 1)
			DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()


        ListViewCtrlMultiline()
        {
        }

        void DeleteItem(DELETEITEMSTRUCT* lParam)
        {
            COwnerDraw<ListViewCtrlMultiline>::DeleteItem(lParam);
        }

        void SetFont(HFONT hFont)
        {
            CListViewCtrl::SetFont(hFont);
            GetHeader().Invalidate();

            // Trigger WM_MEASUREPOS
            // See: http://www.codeproject.com/Articles/1401/Changing-Row-Height-in-an-owner-drawn-Control
            CRect rect;
            GetWindowRect(&rect);
            WINDOWPOS wp;
            wp.hwnd = *this;
            wp.cx = rect.Width();
            wp.cy = rect.Height();
            wp.flags = SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER;
            SendMessage(WM_WINDOWPOSCHANGED, 0, reinterpret_cast<LPARAM>(&wp));
        }

        void Create(HWND hWndParent, ATL::_U_RECT rect)
        {
            __super::Create(hWndParent, rect, CListViewCtrl::GetWndClassName(), WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL, WS_EX_CLIENTEDGE);
        }

        void MeasureItem(MEASUREITEMSTRUCT* pMeasureItemStruct)
        {
            OutputDebugStringA("CLogView::MeasureItem - debug");
            CClientDC dc(*this);

            //Win32::GdiObjectSelection font(dc, GetFont());
            TEXTMETRIC metric;
            dc.GetTextMetrics(&metric);
            pMeasureItemStruct->itemHeight = metric.tmHeight;
        }

        void DrawItem(DRAWITEMSTRUCT* pDrawItemStruct)
        {
            OutputDebugStringA("CLogView::DrawItem - debug");
            DrawItem(pDrawItemStruct->hDC, pDrawItemStruct->itemID, pDrawItemStruct->itemState);
        }

        RECT GetItemRect(int iItem, unsigned code) const
        {
            RECT rect;
            CListViewCtrl::GetItemRect(iItem, &rect, code);
            return rect;
        }

        void DrawItem(CDCHandle dc, int iItem, unsigned /*iItemState*/)
        {
            auto rect = GetItemRect(iItem, LVIR_BOUNDS);

            bool selected = GetItemState(iItem, LVIS_SELECTED) == LVIS_SELECTED;
            bool focused = GetItemState(iItem, LVIS_FOCUSED) == LVIS_FOCUSED;
            auto bkColor = selected ? RGB(255, 200, 200) : RGB(255, 255, 255);
            auto txColor = RGB(0, 0, 0);

            dc.FillSolidRect(rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, bkColor);

            //Win32::ScopedBkColor bcol(dc, bkColor);
            //Win32::ScopedTextColor tcol(dc, txColor);

            std::wstring text = L" If the logview is rendered correctly then exactly 40 lines will fit";
            dc.DrawText(text.c_str(), text.size(), &rect, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

            if (focused)
                dc.DrawFocusRect(&rect);
        }
};