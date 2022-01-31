#if !defined(AFX_PINDOCKBAR_H__A84CB99F_86F9_4915_A34A_0AC066382F4E__INCLUDED_)
#define AFX_PINDOCKBAR_H__A84CB99F_86F9_4915_A34A_0AC066382F4E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PinDockBar.h : header file
//

#include "scbarcf.h"
#include "scbarg.h"

#define BARPUSHPIN_ON WM_USER + 20
#define BARPUSHPIN_OFF WM_USER + 30

class CAutoHideBar;
/////////////////////////////////////////////////////////////////////////////
// CPinDockBar window
class CSCBPushPinButton: public CSCBButton
{
public:
	void Paint(CDC* pDC);
};

class QXLIB_API CPinDockBar : public CSizingControlBarCF
{
// Construction
public:
	CPinDockBar();

// Attributes
public:
	CSCBPushPinButton m_PushPin;
// Operations
public:
	virtual void OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler);
	UINT  GetDockBarID(){ return  m_nDockBarID;};
protected:
	virtual void NcPaintGripper(CDC* pDC, CRect rcClient);
	virtual void NcCalcClient(LPRECT pRc, UINT nDockBarID);
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPinDockBar)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CPinDockBar();

	virtual void AddToDrawBar(CAutoHideBar * pDrawBar) = 0;
	virtual void RemoveFromDrawBar(CAutoHideBar * pDrawBar) = 0;
	// Generated message map functions
protected:
	//{{AFX_MSG(CPinDockBar)
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp);
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg void OnNcLButtonUp(UINT nHitTest, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PINDOCKBAR_H__A84CB99F_86F9_4915_A34A_0AC066382F4E__INCLUDED_)
