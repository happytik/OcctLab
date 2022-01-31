// AutoHideBar.cpp : implementation file
//

#include "stdafx.h"
//#include "pindock.h"
#include "AutoHideBar.h"

//#ifdef _DEBUG
//#define new DEBUG_NEW
//#undef THIS_FILE
//static char THIS_FILE[] = __FILE__;
//#endif

/////////////////////////////////////////////////////////////////////////////
// CAutoHideBar

CAutoHideBar::CAutoHideBar()
{
	m_CurrentSelect = -1;
	m_Height = 24;
	m_pImageList = NULL;
	m_Font.CreateFont(24 - 10 ,0, 0,0, FW_NORMAL, 0,0,0, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, FF_ROMAN , "Arial");
	m_VertFont.CreateFont(24 - 10 ,0, 900,0, FW_NORMAL, 0,0,0, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, FF_ROMAN , "Arial");
}

CAutoHideBar::~CAutoHideBar()
{
	int size = m_ButtonInfo.GetSize();
	for(int idx = 0; idx < size; idx++)
		delete m_ButtonInfo[idx];
	m_ButtonInfo.RemoveAll();
}


CPinDockBar * CAutoHideBar::FindCtrlBar(CWnd * pWnd)
{
	int size = m_ButtonInfo.GetSize();
	for(int idx = 0; idx < size; idx++)
	{
		CDrawButtonInfo * pBtn = m_ButtonInfo[idx];
		if(pBtn->m_pWnd != pWnd)
			continue;
		return pBtn->m_pControlBar;
	}
	return NULL;
}

void CAutoHideBar::CalcLayout()
{
	CRect rt;
	GetClientRect(&rt);
	CRect txtRt;
	CDC * pDC= GetDC();
	int size = m_ButtonInfo.GetSize();
	int x = rt.left + 10;
	int y = rt.top + 10;
	txtRt = rt;
	BOOL bHorz = (m_Style == TOPSIDE || m_Style == BOTTOMSIDE);
	if(bHorz)
		pDC->SelectObject(&m_Font);
	else
		pDC->SelectObject(&m_VertFont);
	for(int idx = 0; idx < size; idx++)
	{
		if(m_ButtonInfo[idx]->m_Style & DRAWBTNSTYLE_SEP)
		{
				x += 10;
				y += 10;
				continue;
		}
		if(bHorz)
			txtRt.left = x;
		else
			txtRt.top = y;
		if(m_ButtonInfo[idx]->m_Style & (DRAWBTNSTYLE_BTN | DRAWBTNSTYLE_SELECT))
		{
			if(bHorz)
				txtRt.right = txtRt.left + 10;
			else
				txtRt.bottom = txtRt.top + 100;
			CSize sz = pDC->GetTextExtent(m_ButtonInfo[idx]->m_Name);
			if(bHorz)
			{
				txtRt.top = rt.top + 2;
				txtRt.bottom = rt.bottom - 2;
				txtRt.right =  txtRt.left + sz.cx + 2 + 16;
				txtRt.left -=  2;
				
				m_ButtonInfo[idx]->m_Rect = txtRt;
				txtRt.top = rt.top - 2;
				txtRt.bottom = rt.bottom + 2;
				txtRt.left +=  2;
				txtRt.right -=  2;
				x = txtRt.right;
			}
			else
			{	
				txtRt.left = rt.left + 2;
				txtRt.right = rt.right - 2;
				txtRt.bottom = txtRt.top + sz.cx + 2 + 16;
				txtRt.top -= 2;
				m_ButtonInfo[idx]->m_Rect = txtRt;
				txtRt.left = rt.left - 2;
				txtRt.right = rt.right + 2;
				txtRt.top += 2;
				txtRt.bottom -= 2;
				y = txtRt.bottom;
			}
		}
		else
		if(m_ButtonInfo[idx]->m_Style == DRAWBTNSTYLE_GROUP )
		{
			if(bHorz)
			{
				txtRt.top = rt.top + 2;
				txtRt.bottom = rt.bottom - 2;
				txtRt.right = txtRt.left +  21;
				m_ButtonInfo[idx]->m_Rect = txtRt;
				txtRt.top = rt.top - 2;
				txtRt.bottom = rt.bottom + 2;
				x = txtRt.right - 1;
			}
			else
			{
				txtRt.left = rt.left + 2;
				txtRt.right = rt.right - 2;
				txtRt.bottom = txtRt.top + 21;
				m_ButtonInfo[idx]->m_Rect = txtRt;
				txtRt.left = rt.left - 2;
				txtRt.right = rt.right + 2;
				y = txtRt.bottom - 1;
			}
		}

	}
	ReleaseDC(pDC);
}

BOOL CAutoHideBar::Create(int Style, CWnd * pWnd)
{
	m_Style = Style;
	CRect rt(10, 10, 10, 10);
	if(!CWnd::Create(AfxRegisterWndClass(
      CS_VREDRAW | CS_HREDRAW,
      ::LoadCursor(NULL, IDC_ARROW),
      (HBRUSH) ::GetStockObject(WHITE_BRUSH),
      ::LoadIcon(NULL, IDI_APPLICATION)),
		"tesT", 
		WS_CHILD , rt, pWnd, 
		233 + Style))
		return FALSE;

	return TRUE;
}

void CAutoHideBar::Draw(CDC * pDC)
{
	BOOL bHorz = (m_Style == TOPSIDE || m_Style == BOTTOMSIDE);
	if(bHorz)
		pDC->SelectObject(&m_Font);
	else
		pDC->SelectObject(&m_VertFont);
	CRect rt;
	GetClientRect(&rt);
	CRect txtRt;
	pDC->FillSolidRect(rt, RGB(232,232,232));
	int x = 10;
	int y = 5;
	pDC->SetTextColor(RGB(0,0,0));
	pDC->SetBkMode(TRANSPARENT);
	int size = m_ButtonInfo.GetSize();
	x = rt.left + 10;
	txtRt = rt;
	CBrush br (RGB(0,0,0));
	for(int idx = 0; idx < size; idx++)
	{
		if(m_ButtonInfo[idx]->m_Style & DRAWBTNSTYLE_SEP)
		{
				continue;
		}

		pDC->FillSolidRect(&m_ButtonInfo[idx]->m_Rect,RGB(210,210,210));
		pDC->FrameRect(&m_ButtonInfo[idx]->m_Rect, &br);
		if(m_ButtonInfo[idx]->m_Style & (DRAWBTNSTYLE_BTN | DRAWBTNSTYLE_GROUP))
		{
			if(m_pImageList && m_ButtonInfo[idx]->m_ImgIndex >= 0)
			{
		        IMAGEINFO Info;
				if (m_pImageList->GetImageInfo(m_ButtonInfo[idx]->m_ImgIndex, &Info))
				{
					POINT pt;
					pt.x = m_ButtonInfo[idx]->m_Rect.left + 2;
					if(bHorz)
						pt.y = m_ButtonInfo[idx]->m_Rect.top + 2;
					else
						pt.y = m_ButtonInfo[idx]->m_Rect.bottom - 2 - 16;
			        m_pImageList->Draw(pDC, m_ButtonInfo[idx]->m_ImgIndex, pt, ILD_NORMAL);
				}

			}
			if(!(m_ButtonInfo[idx]->m_Style & (DRAWBTNSTYLE_BTN |DRAWBTNSTYLE_SELECT)))
				continue;
			if(bHorz)
				pDC->ExtTextOut(m_ButtonInfo[idx]->m_Rect.left + 2 + 16,
					m_ButtonInfo[idx]->m_Rect.top+ 1,
					ETO_CLIPPED,
					NULL, m_ButtonInfo[idx]->m_Name, NULL);
			else
				pDC->ExtTextOut(m_ButtonInfo[idx]->m_Rect.left,
					m_ButtonInfo[idx]->m_Rect.bottom - 2 - 16,
					ETO_CLIPPED,
					NULL, m_ButtonInfo[idx]->m_Name, NULL);

		}
	}

}

void CAutoHideBar::Shrink()
{
	int size = m_ButtonInfo.GetSize();
	BOOL bFlag = TRUE;
	for(int idx = size - 1; idx >= 0; idx--)
	{
		CDrawButtonInfo * pBtn = m_ButtonInfo[idx];
		if(pBtn->m_Style & DRAWBTNSTYLE_SEP)
		{
			if(bFlag || (idx == 0))
			{
				m_ButtonInfo.RemoveAt(idx);
				delete pBtn;
			}
			else
				bFlag = TRUE;
		}
		else
			bFlag = FALSE;
	}
}

void CAutoHideBar::RemoveButton(CWnd * pWnd)
{
	int size = m_ButtonInfo.GetSize();
	for(int idx = 0; idx < size; idx++)
	{
		CDrawButtonInfo * pBtn = m_ButtonInfo[idx];
		if(pBtn->m_pWnd != pWnd)
			continue;
		m_ButtonInfo.RemoveAt(idx);
		pBtn->m_pWnd->SetParent(pBtn->m_pOrginalParent);
		if(pBtn->m_Frame)
			pBtn->m_Frame.DestroyWindow();
		delete pBtn;
		return;
	}
}

void CAutoHideBar::AddButton(LPCSTR pName, int Style, CWnd * pWnd, CWnd * pOrginalParent, CRect* pOrginalRect, CPinDockBar * pControlBar, int ImgIndex)
{
	CRect rt(0,0, 190, 140);
	CDrawButtonInfo * pBtn = new CDrawButtonInfo();
	pBtn->m_Name = pName;
	pBtn->m_pWnd = pWnd;
	pBtn->m_Style = Style;
	pBtn->m_pControlBar = pControlBar;
	pBtn->m_pOrginalParent = pOrginalParent;
	pBtn->m_ImgIndex = ImgIndex;
	if(!(Style & DRAWBTNSTYLE_SEP))
	{
		if(pOrginalRect)
			pBtn->m_OrginalRect = *pOrginalRect;
		rt.left = 5 * m_ButtonInfo.GetSize();
		if(!pBtn->m_Frame.CreateEx(WS_EX_LEFT,
			AfxRegisterWndClass(CS_DBLCLKS | CS_VREDRAW | CS_HREDRAW),
			NULL, (WS_POPUP | WS_BORDER | WS_THICKFRAME), rt, this, 0))
			AfxThrowResourceException();
	}
	pBtn->m_Frame.m_pDrawBar = this;
	pBtn->m_Frame.m_Title = pName;
	if (pWnd != NULL)
	{
		pWnd->SetParent(&(pBtn->m_Frame));
		if(pBtn->m_OrginalRect)
		{
			if(m_Style == TOPSIDE || m_Style == BOTTOMSIDE)
				pBtn->m_Frame.m_Size = pBtn->m_OrginalRect.Height() + GetSystemMetrics(SM_CYFRAME) * 2;
			else
				pBtn->m_Frame.m_Size = pBtn->m_OrginalRect.Width() + GetSystemMetrics(SM_CXFRAME) * 2;
		}
	}

	m_ButtonInfo.Add(pBtn);
	this->RedrawWindow();
}

BEGIN_MESSAGE_MAP(CAutoHideBar, CWnd)
	//{{AFX_MSG_MAP(CAutoHideBar)
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_MOUSEMOVE()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_SIZEPARENT, OnSizeParent)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CAutoHideBar message handlers
LRESULT CAutoHideBar::OnSizeParent(WPARAM, LPARAM lp)
{
	if(m_ButtonInfo.GetSize() == 0)
	{
		ShowWindow(SW_HIDE);
		return 0;
	}
	AFX_SIZEPARENTPARAMS* pParams = reinterpret_cast<AFX_SIZEPARENTPARAMS*>(lp);

	const int height = m_Height;

	int rt_height = pParams->rect.bottom - pParams->rect.top;
	int rt_width  = pParams->rect.right - pParams->rect.left;

	switch(m_Style)
    {
		case LEFTSIDE:
			pParams->rect.left += height;
			MoveWindow(pParams->rect.left - height, pParams->rect.top, height, rt_height, true);
			pParams->sizeTotal.cx -= height;
			break;

		case RIGHTSIDE:
			pParams->rect.right -= height;
			MoveWindow(pParams->rect.right , pParams->rect.top, height, rt_height, true);
			pParams->sizeTotal.cx -= height;
			break;

		case TOPSIDE:
			pParams->rect.top += height;
			MoveWindow(pParams->rect.left, pParams->rect.top - height, rt_width, height, true);
			pParams->sizeTotal.cy -= height;
			break;

		case BOTTOMSIDE:
			pParams->rect.bottom -= height;
			MoveWindow(pParams->rect.left, pParams->rect.bottom, rt_width, height, true);
			pParams->sizeTotal.cy -= height;
			break;

    }
	
	ShowWindow(SW_NORMAL);
	return 0;
}

void CAutoHideBar::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// TODO: Add your message handler code here
	Draw(&dc);
	// Do not call CWnd::OnPaint() for painting messages
}

void CAutoHideBar::OnSize(UINT nType, int cx, int cy) 
{
	CWnd::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here
	CalcLayout();	
}

void CAutoHideBar::OnMouseMove(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	int size = m_ButtonInfo.GetSize();
	for(int idx = 0; idx < size; idx++)
	{
		if(m_ButtonInfo[idx]->m_Style & DRAWBTNSTYLE_SEP)
		{
			continue;
		}

		if(!m_ButtonInfo[idx]->m_Rect.PtInRect(point))
			continue;
		if(m_CurrentSelect == idx)
		{
			if(!m_ButtonInfo[idx]->m_Frame.IsWindowVisible())
			{
				m_ButtonInfo[idx]->m_Frame.SendMessage(WM_SHOWIT);
			}
			break;
		}
		m_CurrentSelect = idx;
		if(m_ButtonInfo[idx]->m_Style & DRAWBTNSTYLE_GROUP)
		{
			m_ButtonInfo[idx]->m_Style |= DRAWBTNSTYLE_SELECT;
			int i;
			for(i = idx + 1; i < size; i ++)
			{
				if(m_ButtonInfo[i]->m_Style & DRAWBTNSTYLE_SEP)
					break;
				m_ButtonInfo[i]->m_Style &= ~DRAWBTNSTYLE_SELECT;
			}
			for(i = idx - 1; i >= 0; i --)
			{
				if(m_ButtonInfo[i]->m_Style & DRAWBTNSTYLE_SEP)
					break;
				m_ButtonInfo[i]->m_Style &= ~DRAWBTNSTYLE_SELECT;
			}

		}
		CalcLayout();
		Invalidate();
		m_ButtonInfo[idx]->m_Frame.SendMessage(WM_SHOWIT);

		break;
	}
		

	CWnd::OnMouseMove(nFlags, point);
}
