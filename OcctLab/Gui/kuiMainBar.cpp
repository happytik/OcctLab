// kuiMainBar.cpp : 实现文件
//

#include "stdafx.h"
#include "Resource.h"
#include "AutoHideBar.h"
#include "kuiMainBar.h"

//////////////////////////////////////////////////////////////////////////
//
axItemData::axItemData()
{
	_eType = eUnknowItem;
	_pData = NULL;
	_bVisible = TRUE;
	_wpType = 0;
}

axItemData::axItemData(eItemType eType,void *pData)
:_eType(eType),_pData(pData)
{
	_bVisible = TRUE;
	_wpType = 0;
}

// kuiMainBar

IMPLEMENT_DYNAMIC(kuiMainBar, CPinDockBar)
kuiMainBar::kuiMainBar()
{
}

kuiMainBar::~kuiMainBar()
{
}

void kuiMainBar::AddToDrawBar(CAutoHideBar * pDrawBar)
{
	CRect rt;
	GetClientRect(&rt);
	CString szStr;
	GetWindowText(szStr);
	pDrawBar->AddButton(szStr, DRAWBTNSTYLE_BTN, &m_wndTreeCtrl, this, &rt, this, 0);
}

void  kuiMainBar::RemoveFromDrawBar(CAutoHideBar * pDrawBar)
{
	CRect rect;
	pDrawBar->RemoveButton(&m_wndTreeCtrl);
	pDrawBar->Shrink();
	pDrawBar->CalcLayout();
	pDrawBar->Invalidate();
	GetParentFrame()->ShowControlBar(this, TRUE, FALSE);

	GetClientRect(&rect);
	m_wndTreeCtrl.MoveWindow(2,2, rect.Width() - 4, rect.Height() - 4);
}

kuiModelTreeInterface*  kuiMainBar::GetModelTreeInterface() const
{
	return (kuiModelTreeInterface *)(&m_wndTreeCtrl);
}

BEGIN_MESSAGE_MAP(kuiMainBar, CPinDockBar)
	ON_WM_CREATE()
	ON_WM_SIZE()
END_MESSAGE_MAP()


// kuiMainBar 消息处理程序

void kuiMainBar::OnUpdateCmdUI(CFrameWnd* /*pTarget*/, BOOL /*bDisableIfNoHndler*/)
{
}

int kuiMainBar::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CPinDockBar::OnCreate(lpCreateStruct) == -1)
		return -1;

	//
	if(!m_wndTreeCtrl.Create(WS_CHILD | WS_VISIBLE | 
		TVS_HASBUTTONS | TVS_LINESATROOT  | TVS_HASLINES | TVS_SHOWSELALWAYS,
		CRect(0,0,0,0),this,123))
	{
		TRACE("\n create list ctrl failed.");
		return -1;
	}

	m_wndTreeCtrl.ModifyStyleEx(0,WS_EX_CLIENTEDGE);

	CreateTreeImageList();

	//执行必要的初始化
	m_wndTreeCtrl.Initialize();

	return 0;
}

void kuiMainBar::CreateTreeImageList()
{
	CBitmap bmp;
	if(!bmp.LoadBitmapA(IDB_BITMAP_MODELTREE24)){
		return;
	}

	BITMAP bmpObj;
	bmp.GetBitmap(&bmpObj);

	UINT nFlags = ILC_MASK | ILC_COLOR24;

	m_treeImageList.Create(24,bmpObj.bmHeight,nFlags,0,0);
	m_treeImageList.Add(&bmp, RGB(255, 0, 255));

	m_wndTreeCtrl.SetImageList(&m_treeImageList,TVSIL_NORMAL);
}

void kuiMainBar::OnSize(UINT nType, int cx, int cy)
{
	CPinDockBar::OnSize(nType, cx, cy);

	// TODO: 在此处添加消息处理程序代码
	if(::IsWindow(m_wndTreeCtrl.GetSafeHwnd()))
	{
		m_wndTreeCtrl.MoveWindow(5,5,cx - 10,cy - 10);
	}
}


