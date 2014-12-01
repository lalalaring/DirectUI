#pragma once
#include "StdAfx.h"


/////////////////////////////////////////////////////////////////////////////////////
//

class UILIB_API CDialogLayoutUI : public CContainerUI
{
public:
	CDialogLayoutUI();

	LPCTSTR GetClass() const;
	LPVOID GetInterface(LPCTSTR pstrName);

	void SetStretchMode(CControlUI* pControl, UINT uMode);

	void SetPos(RECT rc);
	SIZE EstimateSize(SIZE szAvailable);

protected:
	void RecalcArea();

protected:
	typedef struct
	{
		CControlUI* pControl;
		UINT uMode;
		RECT rcItem;
	} STRETCHMODE;

	RECT m_rcDialog;
	RECT m_rcOriginal;
	bool m_bFirstResize;
	CStdValArray m_aModes;
};
