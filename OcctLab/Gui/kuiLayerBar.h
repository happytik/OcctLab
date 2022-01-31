////////////////////////////////////////////////////////////
// kuiLayerBar
#ifndef _KUI_LAYER_BAR_H_
#define _KUI_LAYER_BAR_H_

#include "PinDockBar.h"

class kuiLayerBar : public CPinDockBar
{
	DECLARE_DYNAMIC(kuiLayerBar)

public:
	kuiLayerBar();
	virtual ~kuiLayerBar();
	virtual void OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler);

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
};

#endif

