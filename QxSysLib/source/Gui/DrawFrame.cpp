// DrawFrame.cpp : implementation file
//

#include "stdafx.h"
#include "AutoHideBar.h"
#include "DrawFrame.h"


//#ifdef _DEBUG
//#define new DEBUG_NEW
//#undef THIS_FILE
//static char THIS_FILE[] = __FILE__;
//#endif

int CALLBACK EnumFontFamProc(ENUMLOGFONT FAR *lpelf,
                             NEWTEXTMETRIC FAR *lpntm,
                             int FontType,
                             LPARAM lParam);

#ifndef WS_EX_LAYERED

#define WS_EX_LAYERED 0x80000
#define LWA_ALPHA 0x2
#define LWA_COLORKEY 0x1

#endif

#define SHOWOFFSTEP 160
/////////////////////////////////////////////////////////////////////////////
// CDrawFrame

CDrawFrame::CDrawFrame()
{
    CDC dc;
    dc.CreateCompatibleDC(NULL);

    m_sFontFace = (::EnumFontFamilies(dc.m_hDC,
        _T("Tahoma"), (FONTENUMPROC) EnumFontFamProc, 0) == 0) ?
        _T("Tahoma") : _T("Arial");

    dc.DeleteDC();
	m_Size = 200;
	m_TitleHeight = 12;
	m_pDrawBar = NULL;
	m_bShowOff = FALSE;
}

CDrawFrame::~CDrawFrame()
{
}


BEGIN_MESSAGE_MAP(CDrawFrame, CWnd)
	//{{AFX_MSG_MAP(CDrawFrame)
	ON_WM_CREATE()
	ON_WM_PAINT()
	ON_WM_NCACTIVATE()
	ON_WM_TIMER()
	ON_WM_NCHITTEST()
	ON_WM_NCCALCSIZE()
	ON_WM_SIZE()
	ON_WM_NCPAINT()
	ON_WM_NCMOUSEMOVE()
	ON_WM_NCLBUTTONDOWN()
	ON_WM_NCLBUTTONUP()
	//}}AFX_MSG_MAP
	ON_MESSAGE( WM_SHOWIT, ShowOff)
	ON_MESSAGE( WM_SHOWHIDE, ShowHide)
	ON_MESSAGE( WM_SHOWDOCKBAR, ShowDockBar)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CDrawFrame message handlers

int CDrawFrame::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// TODO: Add your specialized creation code here
	return 0;
}

void CDrawFrame::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// TODO: Add your message handler code here
	CRect rt;
	GetClientRect(rt);
	dc.FillSolidRect(rt, RGB(255,255,255));
	
	// Do not call CWnd::OnPaint() for painting messages
}


BOOL CDrawFrame::OnNcActivate(BOOL bActive) 
{
	// TODO: Add your message handler code here and/or call default

	if(!bActive)
		PostMessage(WM_SHOWHIDE);
	return CWnd::OnNcActivate(bActive);

}

void CDrawFrame::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	if(nIDEvent == 1)
	{
		m_bShowOff = FALSE;
		KillTimer(1);
		CRect rt;
		GetWindowRect(rt);
		switch(m_pDrawBar->m_Style)
		{
			int diff;
			case TOPSIDE:
				if(rt.Height() >= m_Size)
				return;
				diff = min(SHOWOFFSTEP, m_Size - rt.Height());
				rt.bottom += diff;
				break;
			case BOTTOMSIDE:
				if(rt.Height() >= m_Size)
				return;
				diff = min(SHOWOFFSTEP, m_Size - rt.Height());
				rt.top -= diff;
				break;
			case LEFTSIDE:
				if(rt.Width() >= m_Size)
				return;
				diff = min(SHOWOFFSTEP, m_Size - rt.Width());
				rt.right += diff;
				break;
			case RIGHTSIDE:
				if(rt.Width() >= m_Size)
				return;
				diff = min(SHOWOFFSTEP, m_Size - rt.Width());
				rt.left -= diff;
				break;
		}
		m_bShowOff = TRUE;
		MoveWindow(rt);
		SetTimer(1, 100, NULL);
	}
	
	CWnd::OnTimer(nIDEvent);
}

LRESULT CDrawFrame::OnNcHitTest(CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
    LRESULT hit =  CWnd::OnNcHitTest(point);
	CRect rcBar;
    GetWindowRect(rcBar);

	CRect rc = m_PushPin.GetRect();
    rc.OffsetRect(rcBar.TopLeft());
    if (rc.PtInRect(point))
        return HTOBJECT;

	switch(m_pDrawBar->m_Style)
	{
		case TOPSIDE:
			return HTBOTTOM == hit?hit: HTNOWHERE;
			break;
		case BOTTOMSIDE:
			return HTTOP == hit?hit: HTNOWHERE;
			break;
		case LEFTSIDE:
			return HTRIGHT == hit?hit: HTNOWHERE;
			break;
		case RIGHTSIDE:
			return HTLEFT == hit?hit: HTNOWHERE;
			break;
	}

	return HTNOWHERE;

}

void CDrawFrame::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp) 
{
	// TODO: Add your message handler code here and/or call default
	CWnd::OnNcCalcSize(bCalcValidRects, lpncsp);
	CRect rc(&lpncsp->rgrc[0]);
	rc.DeflateRect(0, m_TitleHeight, 0, 0);
	lpncsp->rgrc[0] = rc;
	

	m_PushPin.Move(CPoint(rc.left+ ::GetSystemMetrics(SM_CYFRAME) , rc.top + ::GetSystemMetrics(SM_CXFRAME))
		- rc.TopLeft());
}

void CDrawFrame::OnSize(UINT nType, int cx, int cy) 
{
	CWnd::OnSize(nType, cx, cy);
	CRect rt;
	GetWindowRect(rt);
	
	// TODO: Add your message handler code here
	CWnd* pWnd = GetWindow(GW_CHILD);
	if (pWnd != NULL)
	{
		pWnd->MoveWindow(0, 0, cx, cy);
	}
	if(m_pDrawBar == NULL || m_bShowOff)
		return;
	switch(m_pDrawBar->m_Style)
	{
		case TOPSIDE:
		case BOTTOMSIDE:
			m_Size = cy;
			break;
		case LEFTSIDE:
		case RIGHTSIDE:
			m_Size = cx;
			break;
	}
	
}

#ifndef COLOR_GRADIENTACTIVECAPTION
#define COLOR_GRADIENTACTIVECAPTION     27
#define COLOR_GRADIENTINACTIVECAPTION   28
#define SPI_GETGRADIENTCAPTIONS         0x1008
#endif

void CDrawFrame::OnNcPaint() 
{
	CWnd::OnNcPaint();
	// TODO: Add your message handler code here
	CWindowDC dc(this);
	CRect rc, rcClient;
	GetClientRect(&rcClient);
	ClientToScreen(rcClient);
    GetWindowRect(rc);
    rc.OffsetRect(-rc.TopLeft());

	rc.top += ::GetSystemMetrics(SM_CXFRAME);
	rc.bottom = rc.top + m_TitleHeight;
	rc.left +=::GetSystemMetrics(SM_CYFRAME);
	rc.right -=::GetSystemMetrics(SM_CYFRAME);

    COLORREF clrCptn = GetActiveWindow( ) == this ?
        ::GetSysColor(COLOR_ACTIVECAPTION) :
        ::GetSysColor(COLOR_INACTIVECAPTION);
	COLORREF clrCptnRight = GetActiveWindow( ) == this ?
            ::GetSysColor(COLOR_GRADIENTACTIVECAPTION) :
            ::GetSysColor(COLOR_GRADIENTINACTIVECAPTION);
	dc.FillSolidRect(&rc, ::GetSysColor(COLOR_BTNFACE));
	rc.left += 15;
        // this will make 2^6 = 64 fountain steps
        int nShift = 6;
        int nSteps = 1 << nShift;

        for (int i = 0; i < nSteps; i++)
        {
            // do a little alpha blending
            int nR = (GetRValue(clrCptn) * (nSteps - i) +
                      GetRValue(clrCptnRight) * i) >> nShift;
            int nG = (GetGValue(clrCptn) * (nSteps - i) +
                      GetGValue(clrCptnRight) * i) >> nShift;
            int nB = (GetBValue(clrCptn) * (nSteps - i) +
                      GetBValue(clrCptnRight) * i) >> nShift;

            COLORREF cr = RGB(nR, nG, nB);

            // then paint with the resulting color
            CRect r2 = rc;
			r2.left = rc.left + 
				((i * rc.Width()) >> nShift);
			r2.right = rc.left + 
				(((i + 1) * rc.Width()) >> nShift);
			if (r2.Width() > 0)
				dc.FillSolidRect(r2, cr);

        }
    CFont font;
    int ppi = dc.GetDeviceCaps(LOGPIXELSX);
    int pointsize = MulDiv(85, 96, ppi); // 8.5 points at 96 ppi

//    LOGFONT lf;
    BOOL bFont = font.CreatePointFont(pointsize, m_sFontFace);
    if (bFont)
    {
        // get the text color
        COLORREF clrCptnText = ::GetSysColor(COLOR_CAPTIONTEXT);

        int nOldBkMode = dc.SetBkMode(TRANSPARENT);
        COLORREF clrOldText = dc.SetTextColor(clrCptnText);

        CFont* pOldFont = dc.SelectObject(&font);
        CString sTitle = m_Title;
        //GetWindowText(sTitle);

        dc.ExtTextOut(rc.left + 2, rc.top,
            ETO_CLIPPED, rc, sTitle, NULL);

        dc.SelectObject(pOldFont);
        dc.SetBkMode(nOldBkMode);
        dc.SetTextColor(clrOldText);
    }

	m_PushPin.Paint(&dc);
	ReleaseDC(&dc);
	
	// Do not call CWnd::OnNcPaint() for painting messages
}

void CDrawFrame::OnNcMouseMove(UINT nHitTest, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
    BOOL bNeedPaint = FALSE;

    CPoint pt;
    ::GetCursorPos(&pt);
	BOOL bHit = (OnNcHitTest(pt) == HTOBJECT);
    BOOL bLButtonDown = (::GetKeyState(VK_LBUTTON) < 0);
	BOOL bPushPin = (OnNcHitTest(pt) == HTOBJECT);

	BOOL bWasPushed = m_PushPin.bPushed;
    m_PushPin.bPushed = bPushPin && bLButtonDown;

    BOOL bWasRaised = m_PushPin.bRaised;
    m_PushPin.bRaised = bPushPin && !bLButtonDown;

    bNeedPaint |= (m_PushPin.bPushed ^ bWasPushed) ||
                  (m_PushPin.bRaised ^ bWasRaised);


    if (bNeedPaint)
        SendMessage(WM_NCPAINT);
	
	CWnd::OnNcMouseMove(nHitTest, point);
}


void CDrawFrame::OnNcLButtonDown(UINT nHitTest, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	CWnd::OnNcLButtonDown(nHitTest, point);
}

LRESULT CDrawFrame::ShowHide(WPARAM, LPARAM)
{
	ShowWindow(SW_HIDE);
	return 0;
}

LRESULT CDrawFrame::ShowDockBar(WPARAM, LPARAM)
{
	CPinDockBar * pControlBar = (CPinDockBar*)m_pDrawBar->FindCtrlBar(GetWindow(GW_CHILD));
	if(pControlBar)
	{
		pControlBar->RemoveFromDrawBar(m_pDrawBar);
		
	}
	return 0;
}

LRESULT CDrawFrame::ShowOff(WPARAM, LPARAM)
{
	if(IsWindowVisible())
		return 0;
	CRect rt;
	m_pDrawBar->GetWindowRect(rt);
	switch(m_pDrawBar->m_Style)
	{
		case TOPSIDE:
			rt.top = rt.bottom;
			rt.bottom += min(SHOWOFFSTEP, m_Size);
			break;
		case BOTTOMSIDE:
			rt.bottom = rt.top;
			rt.top -=  min(SHOWOFFSTEP, m_Size);
			break;
		case LEFTSIDE:
			rt.left = rt.right;
			rt.right +=  SHOWOFFSTEP;
			break;
		case RIGHTSIDE:
			rt.right = rt.left;
			rt.left -= SHOWOFFSTEP;
			break;
	}
	m_bShowOff = TRUE;
	SetTimer(1, 100, NULL);
	MoveWindow(rt);
	ShowWindow(SW_SHOW);
	SetActiveWindow();
	SetFocus();
	return 0;
}


void CDrawFrame::OnNcLButtonUp(UINT nHitTest, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	
	CWnd::OnNcLButtonUp(nHitTest, point);
	if (nHitTest == HTOBJECT)
		PostMessage(WM_SHOWDOCKBAR);
}


