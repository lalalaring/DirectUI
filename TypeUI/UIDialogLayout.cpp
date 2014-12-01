#include "StdAfx.h"
#include "UIDialogLayout.h"




/////////////////////////////////////////////////////////////////////////////////////
//
//

CDialogLayoutUI::CDialogLayoutUI() : m_bFirstResize(true), m_aModes(sizeof(STRETCHMODE))
{
	::ZeroMemory(&m_rcDialog, sizeof(m_rcDialog));
}

LPCTSTR CDialogLayoutUI::GetClass() const
{
	return _T("DialogLayoutUI");
}

LPVOID CDialogLayoutUI::GetInterface(LPCTSTR pstrName)
{
	if (_tcscmp(pstrName, _T("DialogLayout")) == 0) return this;
	return CContainerUI::GetInterface(pstrName);
}

void CDialogLayoutUI::SetStretchMode(CControlUI* pControl, UINT uMode)
{
	STRETCHMODE mode;
	mode.pControl = pControl;
	mode.uMode = uMode;
	mode.rcItem = pControl->GetPos();
	m_aModes.Add(&mode);
}

SIZE CDialogLayoutUI::EstimateSize(SIZE szAvailable)
{
	RecalcArea();
	return CSize(m_rcDialog.right - m_rcDialog.left, m_rcDialog.bottom - m_rcDialog.top);
}

void CDialogLayoutUI::SetPos(RECT rc)
{
	m_rcItem = rc;
	RecalcArea();
	// Do Scrollbar
	ProcessScrollbar(rc, m_rcDialog.bottom - m_rcDialog.top);
	if (m_hwndScroll != NULL) rc.right -= m_pManager->GetSystemMetrics().cxvscroll;
	// Determine how "scaled" the dialog is compared to the original size
	int cxDiff = (rc.right - rc.left) - (m_rcDialog.right - m_rcDialog.left);
	int cyDiff = (rc.bottom - rc.top) - (m_rcDialog.bottom - m_rcDialog.top);
	if (cxDiff < 0) cxDiff = 0;
	if (cyDiff < 0) cyDiff = 0;
	// Stretch each control
	// Controls are coupled in "groups", which determine a scaling factor.
	// A "line" indicator is used to apply the same scaling to a new group of controls.
	int nCount, cxStretch, cyStretch, cxMove, cyMove;
	for (int i = 0; i < m_aModes.GetSize(); i++) {
		STRETCHMODE* pItem = static_cast<STRETCHMODE*>(m_aModes[i]);
		if (i == 0 || (pItem->uMode & UISTRETCH_NEWGROUP) != 0) {
			nCount = 0;
			for (int j = i + 1; j < m_aModes.GetSize(); j++) {
				STRETCHMODE* pNext = static_cast<STRETCHMODE*>(m_aModes[j]);
				if ((pNext->uMode & (UISTRETCH_NEWGROUP | UISTRETCH_NEWLINE)) != 0) break;
				if ((pNext->uMode & (UISTRETCH_SIZE_X | UISTRETCH_SIZE_Y)) != 0) nCount++;
			}
			if (nCount == 0) nCount = 1;
			cxStretch = cxDiff / nCount;
			cyStretch = cyDiff / nCount;
			cxMove = 0;
			cyMove = 0;
		}
		if ((pItem->uMode & UISTRETCH_NEWLINE) != 0) {
			cxMove = 0;
			cyMove = 0;
		}
		RECT rcPos = pItem->rcItem;
		::OffsetRect(&rcPos, rc.left, rc.top - m_iScrollPos);
		if ((pItem->uMode & UISTRETCH_MOVE_X) != 0) ::OffsetRect(&rcPos, cxMove, 0);
		if ((pItem->uMode & UISTRETCH_MOVE_Y) != 0) ::OffsetRect(&rcPos, 0, cyMove);
		if ((pItem->uMode & UISTRETCH_SIZE_X) != 0) rcPos.right += cxStretch;
		if ((pItem->uMode & UISTRETCH_SIZE_Y) != 0) rcPos.bottom += cyStretch;
		if ((pItem->uMode & (UISTRETCH_SIZE_X | UISTRETCH_SIZE_Y)) != 0) {
			cxMove += cxStretch;
			cyMove += cyStretch;
		}
		pItem->pControl->SetPos(rcPos);
	}
}

void CDialogLayoutUI::RecalcArea()
{
	if (!m_bFirstResize) return;
	// Add the remaining control to the list
	// Controls that have specific stretching needs will define them in the XML resource
	// and by calling SetStretchMode(). Other controls needs to be added as well now...
	for (int it = 0; it < m_items.GetSize(); it++) {
		CControlUI* pControl = static_cast<CControlUI*>(m_items[it]);
		bool bFound = false;
		for (int i = 0; !bFound && i < m_aModes.GetSize(); i++) {
			if (static_cast<STRETCHMODE*>(m_aModes[i])->pControl == pControl) bFound = true;
		}
		if (!bFound) {
			STRETCHMODE mode;
			mode.pControl = pControl;
			mode.uMode = UISTRETCH_NEWGROUP;
			mode.rcItem = pControl->GetPos();
			m_aModes.Add(&mode);
		}
	}
	// Figure out the actual size of the dialog so we can add proper scrollbars later
	CRect rcDialog(9999, 9999, 0, 0);
	for (int i = 0; i < m_items.GetSize(); i++) {
		const RECT& rcPos = static_cast<CControlUI*>(m_items[i])->GetPos();
		rcDialog.Join(rcPos);
	}
	for (int j = 0; j < m_aModes.GetSize(); j++) {
		RECT& rcPos = static_cast<STRETCHMODE*>(m_aModes[j])->rcItem;
		::OffsetRect(&rcPos, -rcDialog.left, -rcDialog.top);
	}
	m_rcDialog = rcDialog;
	// We're done with initialization
	m_bFirstResize = false;
}

