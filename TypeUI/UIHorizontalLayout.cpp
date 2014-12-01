#include "StdAfx.h"
#include "UIHorizontalLayout.h"

/////////////////////////////////////////////////////////////////////////////////////
//
//

CHorizontalLayoutUI::CHorizontalLayoutUI()
{
}

LPCTSTR CHorizontalLayoutUI::GetClass() const
{
	return _T("HorizontalLayoutUI");
}

void CHorizontalLayoutUI::SetPos(RECT rc)
{
	m_rcItem = rc;
	// Adjust for inset
	rc.left += m_rcInset.left;
	rc.top += m_rcInset.top;
	rc.right -= m_rcInset.right;
	rc.bottom -= m_rcInset.bottom;
	// Determine the width of elements that are sizeable
	SIZE szAvailable = { rc.right - rc.left, rc.bottom - rc.top };
	int nAdjustables = 0;
	int cxFixed = 0;
	for (int it1 = 0; it1 < m_items.GetSize(); it1++) {
		CControlUI* pControl = static_cast<CControlUI*>(m_items[it1]);
		if (!pControl->IsVisible()) continue;
		SIZE sz = pControl->EstimateSize(szAvailable);
		if (sz.cx == 0) nAdjustables++;
		cxFixed += sz.cx + m_iPadding;
	}
	int cxExpand = 0;
	if (nAdjustables > 0) cxExpand = MAX(0, (szAvailable.cx - cxFixed) / nAdjustables);
	// Position the elements
	SIZE szRemaining = szAvailable;
	int iPosX = rc.left;
	int iAdjustable = 0;
	for (int it2 = 0; it2 < m_items.GetSize(); it2++) {
		CControlUI* pControl = static_cast<CControlUI*>(m_items[it2]);
		if (!pControl->IsVisible()) continue;
		SIZE sz = pControl->EstimateSize(szRemaining);
		if (sz.cx == 0) {
			iAdjustable++;
			sz.cx = cxExpand;
			if (iAdjustable == nAdjustables) sz.cx += MAX(0, szAvailable.cx - (cxExpand * nAdjustables) - cxFixed);
		}
		RECT rcCtrl = { iPosX, rc.top, iPosX + sz.cx, rc.bottom };
		pControl->SetPos(rcCtrl);
		iPosX += sz.cx + m_iPadding;
		szRemaining.cx -= sz.cx + m_iPadding;
	}
}
