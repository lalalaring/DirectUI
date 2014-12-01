#include "StdAfx.h"
#include "UITileLayout.h"



/////////////////////////////////////////////////////////////////////////////////////
//
//

CTileLayoutUI::CTileLayoutUI() : m_nColumns(2), m_cyNeeded(0)
{
	SetPadding(10);
	SetInset(CSize(10, 10));
}

LPCTSTR CTileLayoutUI::GetClass() const
{
	return _T("TileLayoutUI");
}

void CTileLayoutUI::SetColumns(int nCols)
{
	if (nCols <= 0) return;
	m_nColumns = nCols;
	UpdateLayout();
}

void CTileLayoutUI::SetPos(RECT rc)
{
	m_rcItem = rc;
	// Adjust for inset
	rc.left += m_rcInset.left;
	rc.top += m_rcInset.top;
	rc.right -= m_rcInset.right;
	rc.bottom -= m_rcInset.bottom;
	if (m_hwndScroll != NULL) rc.right -= m_pManager->GetSystemMetrics().cxvscroll;
	// Position the elements
	int cxWidth = (rc.right - rc.left) / m_nColumns;
	int cyHeight = 0;
	int iCount = 0;
	POINT ptTile = { rc.left, rc.top - m_iScrollPos };
	for (int it1 = 0; it1 < m_items.GetSize(); it1++) {
		CControlUI* pControl = static_cast<CControlUI*>(m_items[it1]);
		if (!pControl->IsVisible()) continue;
		// Determine size
		RECT rcTile = { ptTile.x, ptTile.y, ptTile.x + cxWidth, ptTile.y };
		// Adjust with element padding
		if ((iCount % m_nColumns) == 0) rcTile.right -= m_iPadding / 2;
		else if ((iCount % m_nColumns) == m_nColumns - 1) rcTile.left += m_iPadding / 2;
		else ::InflateRect(&rcTile, -(m_iPadding / 2), 0);
		// If this panel expands vertically
		if (m_cxyFixed.cy == 0) {
			SIZE szAvailable = { rcTile.right - rcTile.left, 9999 };
			int iIndex = iCount;
			for (int it2 = it1; it2 < m_items.GetSize(); it2++) {
				SIZE szTile = static_cast<CControlUI*>(m_items[it2])->EstimateSize(szAvailable);
				cyHeight = MAX(cyHeight, szTile.cy);
				if ((++iIndex % m_nColumns) == 0) break;
			}
		}
		// Set position
		rcTile.bottom = rcTile.top + cyHeight;
		pControl->SetPos(rcTile);
		// Move along...
		if ((++iCount % m_nColumns) == 0) {
			ptTile.x = rc.left;
			ptTile.y += cyHeight + m_iPadding;
			cyHeight = 0;
		}
		else {
			ptTile.x += cxWidth;
		}
		m_cyNeeded = rcTile.bottom - (rc.top - m_iScrollPos);
	}
	// Process the scrollbar
	ProcessScrollbar(rc, m_cyNeeded);
}
