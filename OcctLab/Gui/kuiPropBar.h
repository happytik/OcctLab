///////////////////////////////////////////////////////////////
// kuiPropBar
#ifndef _KUI_PROP_BAR_H_
#define _KUI_PROP_BAR_H_

#include "PinDockBar.h"

class kuiPropBar : public CPinDockBar
{
	DECLARE_DYNAMIC(kuiPropBar)

public:
	kuiPropBar();
	virtual ~kuiPropBar();
	virtual void OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler);

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
};

#endif
