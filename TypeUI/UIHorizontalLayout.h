
#pragma once
//#include "pch.h"
/////////////////////////////////////////////////////////////////////////////////////
//

class UILIB_API CHorizontalLayoutUI : public CContainerUI
{
public:
	CHorizontalLayoutUI();

	LPCTSTR GetClass() const;
	void SetPos(RECT rc);
};