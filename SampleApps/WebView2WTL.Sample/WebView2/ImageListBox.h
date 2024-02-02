#pragma once

#include "osutility.h"

constexpr int       FONT_SIZE = 9;
constexpr wchar_t   FONT_NAME[] = TEXT("Segoe UI");


// ImageListBox mask flags
#define ILBIF_TEXT               0x0001
#define ILBIF_SUBTEXT            0x0002
#define ILBIF_IMAGE              0x0004
#define ILBIF_SELIMAGE           0x0008
#define ILBIF_STYLE              0x0010
#define ILBIF_FORMAT             0x0020
#define ILBIF_PARAM              0x0040

// ImageListBox styles
#define ILBS_IMGLEFT             0x0000
#define ILBS_IMGRIGHT            0x0001
#define ILBS_IMGTOP              0x0002
#define ILBS_SELNORMAL           0x0000
#define ILBS_SELROUND            0x0010


// ImageListBox item structure
typedef struct
{
   int iItem;           // index
   UINT mask;           // mask (ILBIF_xxx flags)
   UINT style;          // item styles (ILBS_xxx flags)
   UINT format;         // text format (DT_xxx flags)
   LPTSTR pszText;      // title text
   int cchMaxText;
   LPTSTR pszSubText;   // subtext
   int cchMaxSubText;
   int iImage;          // image
   int iSelImage;       // selected image
   LPARAM lParam;       // user-defined parameter
} ILBITEM, *PILBITEM;

// ImageListBox ImageList constants
#define ILSIL_NORMAL    0
#define ILSIL_SELECTED  1

// ImageListBox settings
typedef struct
{
   COLORREF clrText;
   COLORREF clrBackground;
   COLORREF clrHighlite;
   COLORREF clrHighliteText;
   COLORREF clrHighliteBorder;
   SIZE sizeMargin;
   SIZE sizeIndent;
   SIZE sizeSubIndent;
   POINT ptArc;
} ILBSETTINGS, *PILBSETTINGS;

class CImageListBoxCtrl : public CWindowImpl<CImageListBoxCtrl, CListBox>,
    public COwnerDraw<CImageListBoxCtrl>
{
public:
   DECLARE_WND_SUPERCLASS(NULL, CListViewCtrl::GetWndClassName())

   CImageListBoxCtrl() : m_hSubFont(NULL)
   { 
      ::ZeroMemory(&m_st, sizeof(m_st));      
      m_st.ptArc.x = m_st.ptArc.y = 8;
   }

   ILBSETTINGS m_st;
   CImageList m_imgNormal;
   CImageList m_imgSelected;
   HFONT m_hSubFont;
   CFont m_font_list = nullptr;
   // Message map
   
   BEGIN_MSG_MAP(CImageListBoxCtrl)
      MESSAGE_HANDLER(WM_CREATE, OnCreate)
      MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
      MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
      MESSAGE_HANDLER(WM_SIZE, OnSize)
      CHAIN_MSG_MAP_ALT( COwnerDraw< CImageListBoxCtrl >, 1 )
      DEFAULT_REFLECTION_HANDLER()
   END_MSG_MAP()

   // Operations

   

   bool Add_Certificate(std::wstring certifcate_name)
   {
       ILBITEM item = { 0 };
       item.mask = ILBIF_TEXT | ILBIF_IMAGE | ILBIF_SELIMAGE | ILBIF_STYLE | ILBIF_FORMAT;
       item.iItem = 0;
       item.iImage = 0;
       item.iSelImage = 0;
       item.pszText = const_cast<LPTSTR>(certifcate_name.c_str());
       item.style = ILBS_IMGLEFT | ILBS_SELROUND;
       item.format = DT_LEFT;
       InsertItem(&item);
   }

   BOOL SubclassWindow(HWND hWnd)
   {
      ATLASSERT(m_hWnd == NULL);
      ATLASSERT(::IsWindow(hWnd));
#ifdef _DEBUG
      TCHAR szBuffer[16] = { 0 };
      if( ::GetClassName(hWnd, szBuffer, 15) ) {
         ATLASSERT(::lstrcmpi(szBuffer, CListBox::GetWndClassName())==0);
      }
#endif // _DEBUG


      BOOL bRet = CWindowImpl<CImageListBoxCtrl, CListBox >::SubclassWindow(hWnd);
      if( bRet ) _Init();
      return bRet;
   }

   void SetItemData(int nIndex, LPARAM lParam)
   {
      ATLASSERT(false);
   }

   int InsertItem(const ILBITEM* pItem)
   {
      ATLASSERT(::IsWindow(m_hWnd));
      // Create a copy of the ITEM structure
      PILBITEM pNewItem {};
      ATLTRY(pNewItem = new ILBITEM);
      ATLASSERT(pNewItem);
      ::ZeroMemory(pNewItem, sizeof(ILBITEM));
      UINT mask = pItem->mask;
      pNewItem->mask = mask;
      if( mask & ILBIF_TEXT ) {
         ATLTRY(pNewItem->pszText = new TCHAR[::lstrlen(pItem->pszText)+1]);
         ::lstrcpy( pNewItem->pszText, pItem->pszText );
      }
      if( mask & ILBIF_SUBTEXT ) {
         ATLTRY(pNewItem->pszSubText = new TCHAR[::lstrlen(pItem->pszSubText )+1]);
         ::lstrcpy( pNewItem->pszSubText, pItem->pszSubText );
      }
      if( mask & ILBIF_STYLE ) pNewItem->style = pItem->style;
      if( mask & ILBIF_FORMAT ) pNewItem->format = pItem->format;
      if( mask & ILBIF_IMAGE ) pNewItem->iImage = pItem->iImage;
      if( mask & ILBIF_SELIMAGE ) pNewItem->iSelImage = pItem->iSelImage;
      if( mask & ILBIF_PARAM ) pNewItem->lParam = pItem->lParam;
      // Add item to listbox
      int iItem = CListBox::InsertString(pItem->iItem, pNewItem->pszText);
      if( iItem>=0 ) CListBox::SetItemDataPtr(iItem, pNewItem);
      return iItem;
   }
   BOOL GetItem(ILBITEM *pItem) const
   {
      ATLASSERT(::IsWindow(m_hWnd));
      ATLASSERT(pItem);
      PILBITEM pOrgItem = (PILBITEM) CListBox::GetItemData(pItem->iItem);
      if( pOrgItem==NULL ) return FALSE;
      // Copy attributes
      UINT mask = pItem->mask;
      if( mask & ILBIF_TEXT ) ::lstrcpyn( pItem->pszText, pOrgItem->pszText, pItem->cchMaxText );
      if( mask & ILBIF_SUBTEXT ) ::lstrcpyn( pItem->pszSubText, pOrgItem->pszSubText, pItem->cchMaxSubText );
      if( mask & ILBIF_STYLE ) pItem->style = pOrgItem->style;
      if( mask & ILBIF_FORMAT ) pItem->format = pOrgItem->format;
      if( mask & ILBIF_IMAGE ) pItem->iImage = pOrgItem->iImage;
      if( mask & ILBIF_SELIMAGE ) pItem->iSelImage = pOrgItem->iSelImage;
      if( mask & ILBIF_PARAM ) pItem->lParam = pOrgItem->lParam;
      return TRUE;
   }
   BOOL SetItem(const ILBITEM *pItem)
   {
      ATLASSERT(::IsWindow(m_hWnd));
      ATLASSERT(pItem);     
      // Get original item data and set attributes
      PILBITEM pOrgItem = (PILBITEM) CListBox::GetItemData(pItem->iItem);
      if( pOrgItem==NULL ) return FALSE;
      UINT mask = pItem->mask;
      if( mask & ILBIF_TEXT ) {
         if( pOrgItem->mask & ILBIF_TEXT ) delete [] pOrgItem->pszText;
         ATLTRY(pOrgItem->pszText = new TCHAR[::lstrlen(pItem->pszText)+1]);
         ::lstrcpy( pOrgItem->pszText, pItem->pszText );
      }
      if( mask & ILBIF_SUBTEXT ) {
         if( pOrgItem->mask & ILBIF_SUBTEXT ) delete [] pOrgItem->pszSubText;
         ATLTRY(pOrgItem->pszSubText = new TCHAR[::lstrlen(pItem->pszSubText)+1]);
         ::lstrcpy( pOrgItem->pszSubText, pItem->pszSubText );
      }
      if( mask & ILBIF_STYLE ) pOrgItem->style = pItem->style;
      if( mask & ILBIF_FORMAT ) pOrgItem->format = pItem->format;
      if( mask & ILBIF_IMAGE ) pOrgItem->iImage = pItem->iImage;
      if( mask & ILBIF_SELIMAGE ) pOrgItem->iSelImage = pItem->iSelImage;
      if( mask & ILBIF_PARAM ) pOrgItem->lParam = pItem->lParam;
      pOrgItem->mask |= pItem->mask;
      // Repaint item
      RECT rc;
      GetItemRect(pItem->iItem, &rc);
      InvalidateRect(&rc, TRUE);
      return TRUE;
   }
   CImageList SetImageList(HIMAGELIST hImageList, int nImageList)
   {
      HIMAGELIST hOldList = nullptr;
      switch( nImageList ) {
      case ILSIL_NORMAL:
         hOldList = m_imgNormal;
         m_imgNormal = hImageList;
         break;
      case ILSIL_SELECTED:
         hOldList = m_imgSelected;
         m_imgSelected = hImageList;
         break;
      default:
         ATLASSERT(false);
      }    
      return hOldList;
   }

   void SetListFont()
   {
       LOGFONT lf;
       ::ZeroMemory(&lf, sizeof(LOGFONT));
       if (this->GetDC() != nullptr)
       {
           int nHeight = -MulDiv(9, GetDeviceCaps(this->GetDC(), LOGPIXELSY), 72);
           wcscpy_s(lf.lfFaceName, FONT_NAME);
           lf.lfHeight = nHeight;
           lf.lfWeight = FW_NORMAL;
           lf.lfCharSet = ANSI_CHARSET;
           lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
           lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
           lf.lfQuality = DEFAULT_QUALITY;
           m_font_list.CreateFontIndirect(&lf);
       }
   }

   HFONT SetSmallFont(HFONT hFont)
   {
      ATLASSERT(::IsWindow(m_hWnd));
      HFONT hOldFont = m_hSubFont;
      m_hSubFont = hFont;
      Invalidate();
      return hOldFont;
   }
   void GetPreferences(PILBSETTINGS pPrefs) const
   {
      ATLASSERT(pPrefs);
      *pPrefs = m_st;
   }
   void SetPreferences(ILBSETTINGS Prefs)
   {
      m_st = Prefs;
   }

   // Implementation

   void _Init()
   {
      ATLASSERT(::IsWindow(m_hWnd));

      ModifyStyle(LBS_SORT | LBS_HASSTRINGS | LBS_MULTIPLESEL, LBS_NOTIFY);

      ATLASSERT(GetStyle() & LBS_OWNERDRAWFIXED); // Needs "OWNER DRAW" to be "FIXED"
    
      _InitSettings();
   }
   void _InitSettings()
   {
      ColorMode mode = os::theme::get_instance()->get_color_mode();

      if (mode == ColorMode::Dark)
      {
          m_st.clrText = WHITE_COLOR;
          m_st.clrBackground = DIALOG_BACKGROUD_DARK_COLOR;
          m_st.clrHighliteText = WHITE_COLOR;
          m_st.clrHighlite = DIALOG_BACKGROUD_DARK_COLOR;
          m_st.clrHighliteBorder = BLUE_COLOR;
      }
      else
      {
              m_st.clrText = ::GetSysColor(COLOR_WINDOWTEXT);
              m_st.clrBackground = ::GetSysColor(COLOR_WINDOW);
              m_st.clrHighliteText = ::GetSysColor(COLOR_HIGHLIGHTTEXT);
              m_st.clrHighlite =
              m_st.clrHighliteBorder = ::GetSysColor(COLOR_HIGHLIGHT);
      }
      SetListFont();
   }

   // Message handlers
   
   LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
   {
      LRESULT lRes = DefWindowProc(uMsg, wParam, lParam);
      _Init();      
      return lRes;
   }
   LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
   {
      ResetContent(); // Make sure to delete item-data memory
      bHandled = FALSE;
      return 0;
   }
   LRESULT OnEraseBackground(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
   {
      CDCHandle dc((HDC)wParam);
      RECT rc;
      GetClientRect(&rc);
      dc.FillSolidRect(&rc, m_st.clrBackground);
      return 1;   // no background needed
   }
   LRESULT OnSize(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
   {
      Invalidate();
      bHandled = FALSE;
      return 0;
   }

   // Owner draw methods

   void DeleteItem(LPDELETEITEMSTRUCT lpdis)
   {
      if( lpdis->itemData ) {
         PILBITEM pItem = reinterpret_cast<PILBITEM>(lpdis->itemData);
         if( pItem->mask & ILBIF_TEXT ) delete [] pItem->pszText;
         if( pItem->mask & ILBIF_SUBTEXT ) delete [] pItem->pszSubText;
         delete pItem;
      }
   }
   void DrawItem(LPDRAWITEMSTRUCT lpdis)
   {
      if( lpdis->itemID==-1 ) return; // If there are no list box items, skip this message. 

      CDCHandle dc(lpdis->hDC);
      PILBITEM pItem = (PILBITEM)lpdis->itemData;
      ATLASSERT(pItem);
      RECT rcItem = lpdis->rcItem;
      bool bSelected = lpdis->itemState & ODS_SELECTED;
      
      UINT style = (pItem->mask & ILBIF_STYLE) != 0 ? pItem->style : ILBS_IMGLEFT|ILBS_SELNORMAL;

      COLORREF clrFront = bSelected ? m_st.clrHighliteText : m_st.clrText;
      COLORREF clrBack = bSelected ? m_st.clrHighlite : m_st.clrBackground;

      if( bSelected && (style & ILBS_SELROUND) ) {
         // Draw round-rect selection
         dc.FillSolidRect(&rcItem, m_st.clrBackground);
         ::InflateRect(&rcItem, -m_st.sizeMargin.cx, -m_st.sizeMargin.cy);
         CPen pen;
         pen.CreatePen(PS_SOLID, 1, m_st.clrHighliteBorder);
         CBrush brush;
         brush.CreateSolidBrush(m_st.clrHighlite);
         HPEN hOldPen = dc.SelectPen(pen);
         HBRUSH hOldBrush = dc.SelectBrush(brush);
         dc.RoundRect(&rcItem, m_st.ptArc);
         dc.SelectBrush(hOldBrush);
         dc.SelectPen(hOldPen);
      }
      else {
         // Fill background
         dc.FillSolidRect(&rcItem, clrBack);
         ::InflateRect(&rcItem, -m_st.sizeMargin.cx, -m_st.sizeMargin.cy);
      }
      if( style & ILBS_SELROUND ) ::InflateRect(&rcItem, -m_st.ptArc.x, -m_st.ptArc.y);

      // Get image information
      int iImageWidth = 0, iImageHeight = 0;
      if( pItem->mask & ILBIF_IMAGE ) m_imgNormal.GetIconSize(iImageWidth, iImageHeight);

      // Prepare draw
      HFONT hOldFont = nullptr;
      if (m_font_list == nullptr)
          hOldFont = dc.SelectFont(GetFont());
      else
          hOldFont = dc.SelectFont(m_font_list.m_hFont);

      dc.SetBkMode(TRANSPARENT);
      dc.SetBkColor(clrBack);
      dc.SetTextColor(clrFront);

      // Draw image (may be aligned left/right/top)
      if( pItem->mask & ILBIF_IMAGE ) {
         if( (style & ILBS_IMGRIGHT) == 0 ) {
            // Left- or top-aligned image
            POINT pt;
            if( style & ILBS_IMGTOP ) {
               pt.x = rcItem.left + ((rcItem.right - rcItem.left) / 2) - (iImageWidth / 2);
               pt.y = rcItem.top;
            }
            else {
               pt.x = rcItem.left;
               pt.y = rcItem.top + ((rcItem.bottom - rcItem.top) / 2) - (iImageHeight / 2);
            }
            if( bSelected && (pItem->mask & ILBIF_SELIMAGE) ) {
               m_imgSelected.Draw(dc, pItem->iSelImage, pt, ILD_NORMAL);
            }
            else {
               m_imgNormal.Draw(dc, pItem->iImage, pt, ILD_NORMAL);
            }
            if( style & ILBS_IMGTOP ) {
               rcItem.top += iImageHeight;
            }
            else {
               rcItem.left += iImageWidth;
            }
         }
         else {
            // Right-aligned image
            int x = rcItem.right - iImageWidth;
            int y = rcItem.top + ((rcItem.bottom - rcItem.top) / 2) - (iImageHeight / 2);
            if( bSelected && (pItem->mask & ILBIF_SELIMAGE) ) {
               m_imgSelected.Draw(dc, pItem->iSelImage, x,y, ILD_NORMAL);
            }
            else {
               m_imgNormal.Draw(dc, pItem->iImage, x,y, ILD_NORMAL);
            }
            rcItem.right -= iImageWidth;
         }
      }

      // Prepare text drawing
      UINT format = DT_LEFT | DT_NOPREFIX;
      if( pItem->mask & ILBIF_FORMAT ) format = pItem->format;

      // Draw text
      if( pItem->mask & ILBIF_TEXT ) {
         ::InflateRect(&rcItem, -m_st.sizeIndent.cx, -m_st.sizeIndent.cy);
         RECT rcText = { 0 };

         rcItem.left += 10;

         dc.DrawText(pItem->pszText, -1, &rcText, format | DT_CALCRECT);
         dc.DrawText(pItem->pszText, -1, &rcItem, format);

         rcItem.top += rcText.bottom-rcText.top;
      }
      // Draw subtext
      if( pItem->mask & ILBIF_SUBTEXT ) {
         if( m_hSubFont != NULL ) dc.SelectFont(m_hSubFont);
         ::InflateRect(&rcItem, -m_st.sizeSubIndent.cx, 0);
         rcItem.top += m_st.sizeSubIndent.cy;
         RECT rcText = { 0 };
         dc.DrawText(pItem->pszSubText, -1, &rcText, format | DT_CALCRECT);
         dc.DrawText(pItem->pszSubText, -1, &rcItem, format);
         rcItem.top += rcText.bottom-rcText.top;
      }

      dc.SelectFont(hOldFont);
   }
};