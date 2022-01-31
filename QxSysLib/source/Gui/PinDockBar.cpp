// PinDockBar.cpp : implementation file
//

#include "stdafx.h"
//#include "pindock.h"
#include "PinDockBar.h"

//#ifdef _DEBUG
//#define new DEBUG_NEW
//#undef THIS_FILE
//static char THIS_FILE[] = __FILE__;
//#endif

void CSCBPushPinButton::Paint(CDC* pDC)
{
    CRect rc = GetRect();

    if (bPushed)
        pDC->Draw3dRect(rc, ::GetSysColor(COLOR_BTNSHADOW),
            ::GetSysColor(COLOR_BTNHIGHLIGHT));
    else
        if (bRaised)
            pDC->Draw3dRect(rc, ::GetSysColor(COLOR_BTNHIGHLIGHT),
                ::GetSysColor(COLOR_BTNSHADOW));

    COLORREF clrOldTextColor = pDC->GetTextColor();
    pDC->SetTextColor(::GetSysColor(COLOR_BTNTEXT));
    int nPrevBkMode = pDC->SetBkMode(TRANSPARENT);
    CFont font;
    int ppi = pDC->GetDeviceCaps(LOGPIXELSX);
    int pointsize = MulDiv(60, 96, ppi); // 6 points at 96 ppi
    font.CreatePointFont(pointsize, _T("Marlett"));
    CFont* oldfont = pDC->SelectObject(&font);

    pDC->TextOut(ptOrg.x + 2, ptOrg.y + 2, CString(_T("v"))); // < >-like

    pDC->SelectObject(oldfont);
    pDC->SetBkMode(nPrevBkMode);
    pDC->SetTextColor(clrOldTextColor);
}

/////////////////////////////////////////////////////////////////////////////
// CPinDockBar
#ifndef COLOR_GRADIENTACTIVECAPTION
#define COLOR_GRADIENTACTIVECAPTION     27
#define COLOR_GRADIENTINACTIVECAPTION   28
#define SPI_GETGRADIENTCAPTIONS         0x1008
#endif

CPinDockBar::CPinDockBar()
{
}

CPinDockBar::~CPinDockBar()
{
}

void CPinDockBar::NcCalcClient(LPRECT pRc, UINT nDockBarID)
{
    CRect rcBar(pRc); // save the bar rect

    // subtract edges
    baseCSizingControlBarG::NcCalcClient(pRc, nDockBarID);

    if (!HasGripper())
        return;

    CRect rc(pRc); // the client rect as calculated by the base class

    BOOL bHorz = (nDockBarID == AFX_IDW_DOCKBAR_TOP) ||
                 (nDockBarID == AFX_IDW_DOCKBAR_BOTTOM);

    if (bHorz)
        rc.DeflateRect(m_cyGripper, 0, 0, 0);
    else
        rc.DeflateRect(0, m_cyGripper, 0, 0);

    // set position for the "x" (hide bar) button
    CPoint ptOrgBtn;
    if (bHorz)
        ptOrgBtn = CPoint(rc.left - 13, rc.top);
    else
        ptOrgBtn = CPoint(rc.right - 12, rc.top - 13);

    m_biHide.Move(ptOrgBtn - rcBar.TopLeft());
	if(!IsFloating())
	{
	    if (bHorz)
		    m_PushPin.Move(CPoint(rc.left - 12, rc.bottom - 13) - rcBar.TopLeft());
	    else
			m_PushPin.Move(CPoint(rc.left , rc.top - 13) - rcBar.TopLeft());
	}
    *pRc = rc;
}

void CPinDockBar::NcPaintGripper(CDC* pDC, CRect rcClient)
{
    if (!HasGripper())
        return;

    // compute the caption rectangle
    BOOL bHorz = IsHorzDocked();
    CRect rcGrip = rcClient;
    CRect rcBtn = m_biHide.GetRect();
    if (bHorz)
    {   // right side gripper
        rcGrip.left -= m_cyGripper + 1;
        rcGrip.right = rcGrip.left + 11;
        rcGrip.top = rcBtn.bottom + 3;
// add pushpin
		if(!IsFloating())
			rcGrip.bottom -= 15;
    }
    else
    {   // gripper at top
        rcGrip.top -= m_cyGripper + 1;
        rcGrip.bottom = rcGrip.top + 11;
        rcGrip.right = rcBtn.left - 3;
// add pushpin
		if(!IsFloating())
			rcGrip.left += 16;
    }
    rcGrip.InflateRect(bHorz ? 1 : 0, bHorz ? 0 : 1);

    // draw the caption background
    //CBrush br;
    COLORREF clrCptn = m_bActive ?
        ::GetSysColor(COLOR_ACTIVECAPTION) :
        ::GetSysColor(COLOR_INACTIVECAPTION);

    // query gradient info (usually TRUE for Win98/Win2k)
    BOOL bGradient = FALSE;
    ::SystemParametersInfo(SPI_GETGRADIENTCAPTIONS, 0, &bGradient, 0);
    
    if (!bGradient)
        pDC->FillSolidRect(&rcGrip, clrCptn); // solid color
    else
    {
        // gradient from left to right or from bottom to top
        // get second gradient color (the right end)
        COLORREF clrCptnRight = m_bActive ?
            ::GetSysColor(COLOR_GRADIENTACTIVECAPTION) :
            ::GetSysColor(COLOR_GRADIENTINACTIVECAPTION);

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
            CRect r2 = rcGrip;
            if (bHorz)
            {
                r2.bottom = rcGrip.bottom - 
                    ((i * rcGrip.Height()) >> nShift);
                r2.top = rcGrip.bottom - 
                    (((i + 1) * rcGrip.Height()) >> nShift);
                if (r2.Height() > 0)
                    pDC->FillSolidRect(r2, cr);
            }
            else
            {
                r2.left = rcGrip.left + 
                    ((i * rcGrip.Width()) >> nShift);
                r2.right = rcGrip.left + 
                    (((i + 1) * rcGrip.Width()) >> nShift);
                if (r2.Width() > 0)
                    pDC->FillSolidRect(r2, cr);
            }
        }
    }

    // draw the caption text - first select a font
    CFont font;
    int ppi = pDC->GetDeviceCaps(LOGPIXELSX);
    int pointsize = MulDiv(85, 96, ppi); // 8.5 points at 96 ppi

    LOGFONT lf;
    BOOL bFont = font.CreatePointFont(pointsize, m_sFontFace);
    if (bFont)
    {
        // get the text color
        COLORREF clrCptnText = m_bActive ?
            ::GetSysColor(COLOR_CAPTIONTEXT) :
            ::GetSysColor(COLOR_INACTIVECAPTIONTEXT);

        int nOldBkMode = pDC->SetBkMode(TRANSPARENT);
        COLORREF clrOldText = pDC->SetTextColor(clrCptnText);

        if (bHorz)
        {
            // rotate text 90 degrees CCW if horizontally docked
            font.GetLogFont(&lf);
            font.DeleteObject();
            lf.lfEscapement = 900;
            font.CreateFontIndirect(&lf);
        }
        
        CFont* pOldFont = pDC->SelectObject(&font);
        CString sTitle;
        GetWindowText(sTitle);

        CPoint ptOrg = bHorz ?
            CPoint(rcGrip.left - 1, rcGrip.bottom - 3) :
            CPoint(rcGrip.left + 3, rcGrip.top - 1);

        pDC->ExtTextOut(ptOrg.x, ptOrg.y,
            ETO_CLIPPED, rcGrip, sTitle, NULL);

        pDC->SelectObject(pOldFont);
        pDC->SetBkMode(nOldBkMode);
        pDC->SetTextColor(clrOldText);
    }

    // draw the button
    m_biHide.Paint(pDC);
	if(!IsFloating())
		m_PushPin.Paint(pDC);
}

BEGIN_MESSAGE_MAP(CPinDockBar, CSizingControlBarCF)
	//{{AFX_MSG_MAP(CPinDockBar)
	ON_WM_NCCALCSIZE()
	ON_WM_NCHITTEST()
	ON_WM_NCLBUTTONUP()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CPinDockBar message handlers
void CPinDockBar::OnNcCalcSize(BOOL bCalcValidRects,
                                     NCCALCSIZE_PARAMS FAR* lpncsp)
{
	UNUSED_ALWAYS(bCalcValidRects);
	CSizingControlBar::OnNcCalcSize(bCalcValidRects, lpncsp);
}

LRESULT CPinDockBar::OnNcHitTest(CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
    CRect rcBar;
    GetWindowRect(rcBar);

    LRESULT nRet = baseCSizingControlBarG::OnNcHitTest(point);
    if (nRet != HTCLIENT)
        return nRet;

    CRect rc = m_biHide.GetRect();
    rc.OffsetRect(rcBar.TopLeft());
    if (rc.PtInRect(point))
        return HTCLOSE;
	rc = m_PushPin.GetRect();
    rc.OffsetRect(rcBar.TopLeft());
    if (rc.PtInRect(point))
        return HTOBJECT;
    return HTCLIENT;
}

void CPinDockBar::OnNcLButtonUp(UINT nHitTest, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	if(!IsFloating())
	{
		if (nHitTest == HTOBJECT)
		{
			GetParentFrame()->PostMessage(BARPUSHPIN_ON, 0, (LPARAM)this);
		}

	}
	
	CSizingControlBarCF::OnNcLButtonUp(nHitTest, point);
}

void CPinDockBar::OnUpdateCmdUI(CFrameWnd* pTarget,
                                      BOOL bDisableIfNoHndler)
{
	CSizingControlBarCF::OnUpdateCmdUI(pTarget,bDisableIfNoHndler);

    if (!HasGripper())
        return;

    BOOL bNeedPaint = FALSE;

    CPoint pt;
    ::GetCursorPos(&pt);
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

}
