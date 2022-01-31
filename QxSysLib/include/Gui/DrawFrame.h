#if !defined(AFX_DRAWFRAME_H__EF0EC395_D6D3_42D2_B68C_6FAB10C90B58__INCLUDED_)
#define AFX_DRAWFRAME_H__EF0EC395_D6D3_42D2_B68C_6FAB10C90B58__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DrawFrame.h : header file
//

#include "PinDockBar.h"

#define WM_SHOWIT WM_USER + 1
#define WM_SHOWHIDE WM_USER + 2
#define WM_SHOWDOCKBAR WM_USER + 3
/////////////////////////////////////////////////////////////////////////////
// CDrawFrame window
class CAutoHideBar;

class QXLIB_API CDrawFrame : public CWnd
{
// Construction
public:
	CDrawFrame();

// Attributes
public:
	int	m_Size;
	CAutoHideBar * m_pDrawBar;
	int m_TitleHeight;
	CString m_Title;
	CString m_sFontFace;
	CSCBPushPinButton m_PushPin;
	BOOL m_bShowOff;
	BOOL m_bActive;
// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDrawFrame)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CDrawFrame();

	// Generated message map functions
protected:
	//{{AFX_MSG(CDrawFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnPaint();
	afx_msg BOOL OnNcActivate(BOOL bActive);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnNcPaint();
	afx_msg void OnNcMouseMove(UINT nHitTest, CPoint point);
	afx_msg void OnNcLButtonDown(UINT nHitTest, CPoint point);
	afx_msg void OnNcLButtonUp(UINT nHitTest, CPoint point);
	//}}AFX_MSG
	afx_msg LRESULT ShowOff(WPARAM, LPARAM);
	afx_msg LRESULT ShowHide(WPARAM, LPARAM);
	LRESULT CDrawFrame::ShowDockBar(WPARAM, LPARAM);
	void OnUpdateCmdUI(CFrameWnd* pTarget,
                                      BOOL bDisableIfNoHndler);

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DRAWFRAME_H__EF0EC395_D6D3_42D2_B68C_6FAB10C90B58__INCLUDED_)
