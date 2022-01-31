
#ifndef _KUI_MAIN_BAR_H_
#define _KUI_MAIN_BAR_H_

#include <vector>

// kuiMainBar
#include "PinDockBar.h"
#include "kuiInterface.h"
#include "ModelTreeCtrl.h"


class kuiMainBar : public CPinDockBar
{
	DECLARE_DYNAMIC(kuiMainBar)

public:
	kuiMainBar();
	virtual ~kuiMainBar();
	virtual void OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler);

	virtual void AddToDrawBar(CAutoHideBar * pDrawBar);
	virtual void RemoveFromDrawBar(CAutoHideBar * pDrawBar);

	kuiModelTreeInterface*		GetModelTreeInterface() const;

protected:
	void		CreateTreeImageList();

protected:
	CModelTreeCtrl			m_wndTreeCtrl;
	CImageList				m_treeImageList;
	
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
};

#endif

