
#pragma once
#include "StdAfx.h"
/////////////////////////////////////////////////////////////////////////////////////
//

class UILIB_API CTileLayoutUI : public CContainerUI
{
public:
	CTileLayoutUI();

	LPCTSTR GetClass() const;

	void SetPos(RECT rc);
	void SetColumns(int nCols);

protected:
	int m_nColumns;
	int m_cyNeeded;
};
