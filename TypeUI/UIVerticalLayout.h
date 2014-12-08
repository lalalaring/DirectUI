
#pragma  once
//#include "pch.h"
/////////////////////////////////////////////////////////////////////////////////////
//


class UILIB_API CVerticalLayoutUI:public CContainerUI
{
public:
	CVerticalLayoutUI();

	LPCTSTR GetClass() const;
	void SetPos(RECT rc);

protected:
	int m_cyNeeded;
	int m_nPrevItems;
};
