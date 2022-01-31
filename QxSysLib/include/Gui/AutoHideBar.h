#if !defined(AFX_AUTOHIDEBAR_H__8F310D89_B221_4BB1_A43A_A2E50CD1C260__INCLUDED_)
#define AFX_AUTOHIDEBAR_H__8F310D89_B221_4BB1_A43A_A2E50CD1C260__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AutoHideBar.h : header file
//
#include "DrawFrame.h"

#define TOPSIDE 0
#define LEFTSIDE 1
#define RIGHTSIDE 2
#define BOTTOMSIDE 3

#define DRAWBTNSTYLE_BTN 1
#define DRAWBTNSTYLE_GROUP 2
#define DRAWBTNSTYLE_SELECT 4
#define DRAWBTNSTYLE_SEP 8

typedef 
struct 
{
	CString m_Name;
	int m_Style;
	CWnd * m_pWnd;
	CDrawFrame m_Frame;
	CRect m_Rect;
	CWnd * m_pOrginalParent;
	CRect m_OrginalRect;
	CPinDockBar * m_pControlBar;
	int m_ImgIndex;
} CDrawButtonInfo;

#define DECLARE_PINDOCK()                                       \
    protected:                                                  \
	CAutoHideBar m_AutoHideBar[4];								\
	void EnablePinDocking(DWORD dwDockStyle);					\
	afx_msg LRESULT OnPushPinOn(WPARAM, LPARAM);

#define ON_PINDOCK_MESSAGES()									\
	ON_MESSAGE( BARPUSHPIN_ON, OnPushPinOn)

#define IMPLEMENT_PINDOCK(theClass)                             \
LRESULT theClass::OnPushPinOn(WPARAM, LPARAM lParam)			\
{																\
	CPinDockBar * pWnd = (CPinDockBar *)lParam;					\
	CAutoHideBar * pAutoHideBar;								\
	switch(pWnd->GetDockBarID())								\
	{															\
		case AFX_IDW_DOCKBAR_TOP:								\
			pAutoHideBar = &m_AutoHideBar[0];					\
			break;												\
		case AFX_IDW_DOCKBAR_LEFT:								\
			pAutoHideBar = &m_AutoHideBar[1];					\
			break;												\
		case AFX_IDW_DOCKBAR_RIGHT:								\
			pAutoHideBar = &m_AutoHideBar[2];					\
			break;												\
		case AFX_IDW_DOCKBAR_BOTTOM:							\
			pAutoHideBar = &m_AutoHideBar[3];					\
			break;												\
	};															\
	pAutoHideBar->AddButton(NULL, DRAWBTNSTYLE_SEP);			\
	pWnd->AddToDrawBar(pAutoHideBar);							\
	pAutoHideBar->AddButton(NULL, DRAWBTNSTYLE_SEP);			\
	pAutoHideBar->Shrink();										\
	pAutoHideBar->CalcLayout();									\
	pAutoHideBar->Invalidate();									\
	ShowControlBar(pWnd, FALSE, FALSE);							\
	return 0;													\
}																\
																\
void theClass::EnablePinDocking(DWORD dwDockStyle)				\
{																\
	if(dwDockStyle & CBRS_ALIGN_TOP & CBRS_ALIGN_ANY)			\
		m_AutoHideBar[0].Create(0, this);						\
	if(dwDockStyle & CBRS_ALIGN_LEFT & CBRS_ALIGN_ANY)			\
		m_AutoHideBar[1].Create(1, this);						\
	if(dwDockStyle & CBRS_ALIGN_RIGHT & CBRS_ALIGN_ANY)			\
		m_AutoHideBar[2].Create(2, this);						\
	if(dwDockStyle & CBRS_ALIGN_BOTTOM & CBRS_ALIGN_ANY)		\
		m_AutoHideBar[3].Create(3, this);						\
	EnableDocking(dwDockStyle);									\
	if(dwDockStyle & CBRS_ALIGN_TOP & CBRS_ALIGN_ANY)			\
	{															\
		if(::IsWindow(m_AutoHideBar[1].GetSafeHwnd()))			\
			m_AutoHideBar[1].SetWindowPos(GetControlBar(AFX_IDW_DOCKBAR_TOP),0,0,0,0, SWP_NOSIZE | SWP_NOMOVE);		\
		if(::IsWindow(m_AutoHideBar[2].GetSafeHwnd()))																\
			m_AutoHideBar[2].SetWindowPos(GetControlBar(AFX_IDW_DOCKBAR_TOP),0,0,0,0, SWP_NOSIZE | SWP_NOMOVE);		\
	}																	\
	m_pFloatingFrameClass = RUNTIME_CLASS(CSCBMiniDockFrameWnd);		\
}		


/////////////////////////////////////////////////////////////////////////////
// CAutoHideBar window

class QXLIB_API CAutoHideBar : public CWnd
{
// Construction
public:
	CAutoHideBar();

// Attributes
public:
	int m_Style;
	int m_Height;
	CArray<CDrawButtonInfo*, CDrawButtonInfo *> m_ButtonInfo;
	CFont m_Font;
	CFont m_VertFont;
	int m_CurrentSelect;
	CImageList* m_pImageList;
// Operations
public:
	void Draw(CDC * pDC);
	void Shrink();
	BOOL Create(int Style, CWnd * pWnd);
	void AddButton(LPCSTR pName, int Style, CWnd * pWnd = NULL, 
		CWnd * pOrginalParent = NULL, CRect* pOrginalRect = NULL,
		CPinDockBar * pControlBar = NULL, int ImgIndex = -1);
	void RemoveButton(CWnd * pWnd);
	CPinDockBar * FindCtrlBar(CWnd * pWnd);
	void CalcLayout();
	void SetImageList(CImageList* pList)
	{
		m_pImageList = pList;
	};
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAutoHideBar)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CAutoHideBar();

	// Generated message map functions
protected:
	//{{AFX_MSG(CAutoHideBar)
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	//}}AFX_MSG
	LRESULT OnSizeParent(WPARAM, LPARAM);
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_AUTOHIDEBAR_H__8F310D89_B221_4BB1_A43A_A2E50CD1C260__INCLUDED_)
