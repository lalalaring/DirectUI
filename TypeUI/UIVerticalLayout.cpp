#include "StdAfx.h"
#include "UIVerticalLayout.h"

/////////////////////////////////////////////////////////////////////////////////////
//
//

CVerticalLayoutUI::CVerticalLayoutUI():m_cyNeeded(0)
{
}

LPCTSTR CVerticalLayoutUI::GetClass() const
{
	return _T("VertialLayoutUI");
}

void CVerticalLayoutUI::SetPos(RECT rc)
{
	m_rcItem = rc;
	// Adjust for inset
	rc.left += m_rcInset.left;
	rc.top += m_rcInset.top;
	rc.right -= m_rcInset.right;
	rc.bottom -= m_rcInset.bottom;
	if (m_hwndScroll != NULL) rc.right -= m_pManager->GetSystemMetrics().cxvscroll;
	// Determine the minimum size
	SIZE szAvailable = { rc.right - rc.left, rc.bottom - rc.top };
	int nAdjustables = 0;
	int cyFixed = 0;
	for (int it1 = 0; it1 < m_items.GetSize(); it1++) {
		CControlUI* pControl = static_cast<CControlUI*>(m_items[it1]);
		if (!pControl->IsVisible()) continue;
		SIZE sz = pControl->EstimateSize(szAvailable);
		if (sz.cy == 0) nAdjustables++;
		cyFixed += sz.cy + m_iPadding;
	}
	// Place elements
	int cyNeeded = 0;
	int cyExpand = 0;
	if (nAdjustables > 0) cyExpand = MAX(0, (szAvailable.cy - cyFixed) / nAdjustables);
	// Position the elements
	SIZE szRemaining = szAvailable;
	int iPosY = rc.top - m_iScrollPos;
	int iAdjustable = 0;
	for (int it2 = 0; it2 < m_items.GetSize(); it2++) {
		CControlUI* pControl = static_cast<CControlUI*>(m_items[it2]);
		if (!pControl->IsVisible()) continue;
		SIZE sz = pControl->EstimateSize(szRemaining);
		if (sz.cy == 0) {
			iAdjustable++;
			sz.cy = cyExpand;
			// Distribute remaining to last element (usually round-off left-overs)
			if (iAdjustable == nAdjustables) sz.cy += MAX(0, szAvailable.cy - (cyExpand * nAdjustables) - cyFixed);
		}
		RECT rcCtrl = { rc.left, iPosY, rc.right, iPosY + sz.cy };
		pControl->SetPos(rcCtrl);
		iPosY += sz.cy + m_iPadding;
		cyNeeded += sz.cy + m_iPadding;
		szRemaining.cy -= sz.cy + m_iPadding;
	}
	// Handle overflow with scrollbars
	ProcessScrollbar(rc, cyNeeded);
}