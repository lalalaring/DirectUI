
#include "StdAfx.h"
#include "UIContainer.h"


/////////////////////////////////////////////////////////////////////////////////////
//
//

CContainerUI::CContainerUI() : 
   m_hwndScroll(NULL), 
   m_iPadding(0),
   m_iScrollPos(0),
   m_bAutoDestroy(true),
   m_bAllowScrollbars(false)
{
   m_cxyFixed.cx = m_cxyFixed.cy = 0;
   ::ZeroMemory(&m_rcInset, sizeof(m_rcInset));
}

CContainerUI::~CContainerUI()
{
   RemoveAll();
}

LPCTSTR CContainerUI::GetClass() const
{
   return _T("ContainerUI");
}

LPVOID CContainerUI::GetInterface(LPCTSTR pstrName)
{
   if( _tcscmp(pstrName, _T("Container")) == 0 ) return static_cast<IContainerUI*>(this);
   return CControlUI::GetInterface(pstrName);
}

CControlUI* CContainerUI::GetItem(int iIndex) const
{
   if( iIndex < 0 || iIndex >= m_items.GetSize() ) return NULL;
   return static_cast<CControlUI*>(m_items[iIndex]);
}

int CContainerUI::GetCount() const
{
   return m_items.GetSize();
}

bool CContainerUI::Add(CControlUI* pControl)
{
   if( m_pManager != NULL ) m_pManager->InitControls(pControl, this);
   if( m_pManager != NULL ) m_pManager->UpdateLayout();
   return m_items.Add(pControl);
}

bool CContainerUI::Remove(CControlUI* pControl)
{
   for( int it = 0; m_bAutoDestroy && it < m_items.GetSize(); it++ ) {
      if( static_cast<CControlUI*>(m_items[it]) == pControl ) {
         if( m_pManager != NULL ) m_pManager->UpdateLayout();
         delete pControl;
         return m_items.Remove(it);
      }
   }
   return false;
}

void CContainerUI::RemoveAll()
{
   for( int it = 0; m_bAutoDestroy && it < m_items.GetSize(); it++ ) delete static_cast<CControlUI*>(m_items[it]);
   m_items.Empty();
   m_iScrollPos = 0;
   if( m_pManager != NULL ) m_pManager->UpdateLayout();
}

void CContainerUI::SetAutoDestroy(bool bAuto)
{
   m_bAutoDestroy = bAuto;
}

void CContainerUI::SetInset(SIZE szInset)
{
   m_rcInset.left = m_rcInset.right = szInset.cx;
   m_rcInset.top = m_rcInset.bottom = szInset.cy;
}

void CContainerUI::SetInset(RECT rcInset)
{
   m_rcInset = rcInset;
}

void CContainerUI::SetPadding(int iPadding)
{
   m_iPadding = iPadding;
}

void CContainerUI::SetWidth(int cx)
{
   m_cxyFixed.cx = cx;
}

void CContainerUI::SetHeight(int cy)
{
   m_cxyFixed.cy = cy;
}

void CContainerUI::SetVisible(bool bVisible)
{
   // Hide possible scrollbar control
   if( m_hwndScroll != NULL ) ::ShowScrollBar(m_hwndScroll, SB_CTL, bVisible);
   // Hide children as well
   for( int it = 0; it < m_items.GetSize(); it++ ) {
      static_cast<CControlUI*>(m_items[it])->SetVisible(bVisible);
   }
   CControlUI::SetVisible(bVisible);
}

void CContainerUI::Event(TEventUI& event)
{
   if( m_hwndScroll != NULL ) 
   {
      if( event.Type == UIEVENT_VSCROLL ) 
      {
         switch( LOWORD(event.wParam) ) {
         case SB_THUMBPOSITION:
         case SB_THUMBTRACK:
            {
               SCROLLINFO si = { 0 };
               si.cbSize = sizeof(SCROLLINFO);
               si.fMask = SIF_TRACKPOS;
               ::GetScrollInfo(m_hwndScroll, SB_CTL, &si);
               SetScrollPos(si.nTrackPos);
            }
            break;
         case SB_LINEUP:
            SetScrollPos(GetScrollPos() - 5);
            break;
         case SB_LINEDOWN:
            SetScrollPos(GetScrollPos() + 5);
            break;
         case SB_PAGEUP:
            SetScrollPos(GetScrollPos() - GetScrollPage());
            break;
         case SB_PAGEDOWN:
            SetScrollPos(GetScrollPos() + GetScrollPage());
            break;
         }
      }
      if( event.Type == UIEVENT_KEYDOWN ) 
      {
         switch( event.chKey ) {
         case VK_DOWN:
            SetScrollPos(GetScrollPos() + 5);
            return;
         case VK_UP:
            SetScrollPos(GetScrollPos() - 5);
            return;
         case VK_NEXT:
            SetScrollPos(GetScrollPos() + GetScrollPage());
            return;
         case VK_PRIOR:
            SetScrollPos(GetScrollPos() - GetScrollPage());
            return;
         case VK_HOME:
            SetScrollPos(0);
            return;
         case VK_END:
            SetScrollPos(9999);
            return;
         }
      }
   }
   CControlUI::Event(event);
}

int CContainerUI::GetScrollPos() const
{
   return m_iScrollPos;
}

int CContainerUI::GetScrollPage() const
{
   // TODO: Determine this dynamically
   return 40;
}

SIZE CContainerUI::GetScrollRange() const
{
   if( m_hwndScroll == NULL ) return CSize();
   int cx = 0, cy = 0;
   ::GetScrollRange(m_hwndScroll, SB_CTL, &cx, &cy);
   return CSize(cx, cy);
}

void CContainerUI::SetScrollPos(int iScrollPos)
{
   if( m_hwndScroll == NULL ) return;
   int iRange1 = 0, iRange2 = 0;
   ::GetScrollRange(m_hwndScroll, SB_CTL, &iRange1, &iRange2);
   iScrollPos = CLAMP(iScrollPos, iRange1, iRange2);
   ::SetScrollPos(m_hwndScroll, SB_CTL, iScrollPos, TRUE);
   m_iScrollPos = ::GetScrollPos(m_hwndScroll, SB_CTL);
   // Reposition children to the new viewport.
   SetPos(m_rcItem);
   Invalidate();
}

void CContainerUI::EnableScrollBar(bool bEnable)
{
   if( m_bAllowScrollbars == bEnable ) return;
   m_iScrollPos = 0;
   m_bAllowScrollbars = bEnable;
}

int CContainerUI::FindSelectable(int iIndex, bool bForward /*= true*/) const
{
   // NOTE: This is actually a helper-function for the list/combo/ect controls
   //       that allow them to find the next enabled/available selectable item
   if( GetCount() == 0 ) return -1;
   iIndex = CLAMP(iIndex, 0, GetCount() - 1);
   if( bForward ) {
      for( int i = iIndex; i < GetCount(); i++ ) {
         if( GetItem(i)->GetInterface(_T("ListItem")) != NULL 
             && GetItem(i)->IsVisible()
             && GetItem(i)->IsEnabled() ) return i;
      }
      return -1;
   }
   else {
      for( int i = iIndex; i >= 0; --i ) {
         if( GetItem(i)->GetInterface(_T("ListItem")) != NULL 
             && GetItem(i)->IsVisible()
             && GetItem(i)->IsEnabled() ) return i;
      }
      return FindSelectable(0, true);
   }
}

void CContainerUI::SetPos(RECT rc)
{
   CControlUI::SetPos(rc);
   if( m_items.IsEmpty() ) return;
   rc.left += m_rcInset.left;
   rc.top += m_rcInset.top;
   rc.right -= m_rcInset.right;
   rc.bottom -= m_rcInset.bottom;
   // We'll position the first child in the entire client area.
   // Do not leave a container empty.
   ASSERT(m_items.GetSize()==1);
   static_cast<CControlUI*>(m_items[0])->SetPos(rc);
}

SIZE CContainerUI::EstimateSize(SIZE /*szAvailable*/)
{
   return m_cxyFixed;
}

void CContainerUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
   if( _tcscmp(pstrName, _T("inset")) == 0 ) SetInset(CSize(_ttoi(pstrValue), _ttoi(pstrValue)));
   else if( _tcscmp(pstrName, _T("padding")) == 0 ) SetPadding(_ttoi(pstrValue));
   else if( _tcscmp(pstrName, _T("width")) == 0 ) SetWidth(_ttoi(pstrValue));
   else if( _tcscmp(pstrName, _T("height")) == 0 ) SetHeight(_ttoi(pstrValue));
   else if( _tcscmp(pstrName, _T("scrollbar")) == 0 ) EnableScrollBar(_tcscmp(pstrValue, _T("true")) == 0);
   else CControlUI::SetAttribute(pstrName, pstrValue);
}

void CContainerUI::SetManager(CPaintManagerUI* pManager, CControlUI* pParent)
{
   for( int it = 0; it < m_items.GetSize(); it++ ) {
      static_cast<CControlUI*>(m_items[it])->SetManager(pManager, this);
   }
   CControlUI::SetManager(pManager, pParent);
}

CControlUI* CContainerUI::FindControl(FINDCONTROLPROC Proc, LPVOID pData, UINT uFlags)
{
   // Check if this guy is valid
   if( (uFlags & UIFIND_VISIBLE) != 0 && !IsVisible() ) return NULL;
   if( (uFlags & UIFIND_ENABLED) != 0 && !IsEnabled() ) return NULL;
   if( (uFlags & UIFIND_HITTEST) != 0 && !::PtInRect(&m_rcItem, *(static_cast<LPPOINT>(pData))) ) return NULL;
   if( (uFlags & UIFIND_ME_FIRST) != 0 ) {
      CControlUI* pControl = CControlUI::FindControl(Proc, pData, uFlags);
      if( pControl != NULL ) return pControl;
   }
   for( int it = 0; it != m_items.GetSize(); it++ ) {
      CControlUI* pControl = static_cast<CControlUI*>(m_items[it])->FindControl(Proc, pData, uFlags);
      if( pControl != NULL ) return pControl;
   }
   return CControlUI::FindControl(Proc, pData, uFlags);
}

void CContainerUI::DoPaint(HDC hDC, const RECT& rcPaint)
{
   RECT rcTemp = { 0 };
   if( !::IntersectRect(&rcTemp, &rcPaint, &m_rcItem) ) return;

   CRenderClip clip;
   CBlueRenderEngineUI::GenerateClip(hDC, m_rcItem, clip);

   for( int it = 0; it < m_items.GetSize(); it++ ) {
      CControlUI* pControl = static_cast<CControlUI*>(m_items[it]);
      if( !pControl->IsVisible() ) continue;
      if( !::IntersectRect(&rcTemp, &rcPaint, &pControl->GetPos()) ) continue;
      if( !::IntersectRect(&rcTemp, &m_rcItem, &pControl->GetPos()) ) continue;
      pControl->DoPaint(hDC, rcPaint);
   }
}

void CContainerUI::ProcessScrollbar(RECT rc, int cyRequired)
{
   // Need the scrollbar control, but it's been created already?
   if( cyRequired > rc.bottom - rc.top && m_hwndScroll == NULL && m_bAllowScrollbars ) {
      m_hwndScroll = ::CreateWindowEx(0, WC_SCROLLBAR, NULL, WS_CHILD | SBS_VERT, 0, 0, 0, 0, m_pManager->GetPaintWindow(), NULL, m_pManager->GetResourceInstance(), NULL);
      ASSERT(::IsWindow(m_hwndScroll));
      ::SetProp(m_hwndScroll, "WndX", static_cast<HANDLE>(this));
      ::SetScrollPos(m_hwndScroll, SB_CTL, 0, TRUE);
      ::ShowWindow(m_hwndScroll, SW_SHOWNOACTIVATE);
      SetPos(m_rcItem);
      return;
   }
   // No scrollbar required
   if( m_hwndScroll == NULL ) return;
   // Move it into place
   int cxScroll = m_pManager->GetSystemMetrics().cxvscroll;
   ::MoveWindow(m_hwndScroll, rc.right, rc.top, cxScroll, rc.bottom - rc.top, TRUE);
   // Scroll not needed anymore?
   int cyScroll = cyRequired - (rc.bottom - rc.top);
   if( cyScroll < 0 ) {
      if( m_iScrollPos != 0 ) SetScrollPos(0);
      cyScroll = 0;
   }
   // Scroll range changed?
   int cyOld1, cyOld2;
   ::GetScrollRange(m_hwndScroll, SB_CTL, &cyOld1, &cyOld2);
   if( cyOld2 != cyScroll ) {
      ::SetScrollRange(m_hwndScroll, SB_CTL, 0, cyScroll, FALSE);
      ::EnableScrollBar(m_hwndScroll, SB_CTL, cyScroll == 0 ? ESB_DISABLE_BOTH : ESB_ENABLE_BOTH);
   }
}


/////////////////////////////////////////////////////////////////////////////////////
//
//

CCanvasUI::CCanvasUI() : m_hBitmap(NULL), m_iOrientation(HTBOTTOMRIGHT)
{
}

CCanvasUI::~CCanvasUI()
{
   if( m_hBitmap != NULL ) ::DeleteObject(m_hBitmap);
}

LPCTSTR CCanvasUI::GetClass() const
{
   return _T("CanvasUI");
}

bool CCanvasUI::SetWatermark(UINT iBitmapRes, int iOrientation)
{
   return SetWatermark(MAKEINTRESOURCE(iBitmapRes), iOrientation);
}

bool CCanvasUI::SetWatermark(LPCTSTR pstrBitmap, int iOrientation)
{
   if( m_hBitmap != NULL ) ::DeleteObject(m_hBitmap);
   m_hBitmap = (HBITMAP) ::LoadImage(m_pManager->GetResourceInstance(), pstrBitmap, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
   ASSERT(m_hBitmap!=NULL);
   if( m_hBitmap == NULL ) return false;
   ::GetObject(m_hBitmap, sizeof(BITMAP), &m_BitmapInfo);
   m_iOrientation = iOrientation;
   Invalidate();
   return true;
}

void CCanvasUI::DoPaint(HDC hDC, const RECT& rcPaint)
{
   // Fill background
   RECT rcFill = { 0 };
   if( ::IntersectRect(&rcFill, &rcPaint, &m_rcItem) ) {
      CBlueRenderEngineUI::DoFillRect(hDC, m_pManager, rcFill, m_clrBack);
   }
   // Paint watermark bitmap
   if( m_hBitmap != NULL ) {
      RECT rcBitmap = { 0 };
      switch( m_iOrientation ) {
      case HTTOPRIGHT:
         ::SetRect(&rcBitmap, m_rcItem.right - m_BitmapInfo.bmWidth, m_rcItem.top, m_rcItem.right, m_rcItem.top + m_BitmapInfo.bmHeight);
         break;
      case HTBOTTOMRIGHT:
         ::SetRect(&rcBitmap, m_rcItem.right - m_BitmapInfo.bmWidth, m_rcItem.bottom - m_BitmapInfo.bmHeight, m_rcItem.right, m_rcItem.bottom);
         break;
      default:
         ::SetRect(&rcBitmap, m_rcItem.right - m_BitmapInfo.bmWidth, m_rcItem.bottom - m_BitmapInfo.bmHeight, m_rcItem.right, m_rcItem.bottom);
         break;
      }
      RECT rcTemp = { 0 };
      if( ::IntersectRect(&rcTemp, &rcPaint, &rcBitmap) ) {
         CRenderClip clip;
         CBlueRenderEngineUI::GenerateClip(hDC, m_rcItem, clip);
         CBlueRenderEngineUI::DoPaintBitmap(hDC, m_pManager, m_hBitmap, rcBitmap);
      }
   }
   CContainerUI::DoPaint(hDC, rcPaint);
}

void CCanvasUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
	//ˮӡ
   //if( _tcscmp(pstrName, _T("watermark")) == 0 ) SetWatermark(pstrValue);
   //else 
	   CContainerUI::SetAttribute(pstrName, pstrValue);
}


/////////////////////////////////////////////////////////////////////////////////////
//
//

CWindowCanvasUI::CWindowCanvasUI()
{
   SetInset(CSize(10, 10));
   m_clrBack = m_pManager->GetThemeColor(UICOLOR_WINDOW_BACKGROUND);
}

LPCTSTR CWindowCanvasUI::GetClass() const
{
   return _T("WindowCanvasUI");
}


/////////////////////////////////////////////////////////////////////////////////////
//
//

CControlCanvasUI::CControlCanvasUI()
{
   SetInset(CSize(0, 0));
   m_clrBack = m_pManager->GetThemeColor(UICOLOR_CONTROL_BACKGROUND_NORMAL);
}

LPCTSTR CControlCanvasUI::GetClass() const
{
   return _T("ControlCanvasUI");
}


/////////////////////////////////////////////////////////////////////////////////////
//
//

CWhiteCanvasUI::CWhiteCanvasUI()
{
   SetInset(CSize(0, 0));
   m_clrBack = m_pManager->GetThemeColor(UICOLOR_STANDARD_WHITE);
}

LPCTSTR CWhiteCanvasUI::GetClass() const
{
   return _T("WhiteCanvasUI");
}


/////////////////////////////////////////////////////////////////////////////////////
//
//

CDialogCanvasUI::CDialogCanvasUI()
{
   SetInset(CSize(10, 10));
   m_clrBack = m_pManager->GetThemeColor(UICOLOR_DIALOG_BACKGROUND);
}

LPCTSTR CDialogCanvasUI::GetClass() const
{
   return _T("DialogCanvasUI");
}


/////////////////////////////////////////////////////////////////////////////////////
//
//

CTabFolderCanvasUI::CTabFolderCanvasUI()
{
   SetInset(CSize(0, 0));
   COLORREF clrColor1;
   m_pManager->GetThemeColorPair(UICOLOR_TAB_FOLDER_NORMAL, clrColor1, m_clrBack);
}

LPCTSTR CTabFolderCanvasUI::GetClass() const
{
   return _T("TabFolderCanvasUI");
}





