// kuiPropBar.cpp : 实现文件
//

#include "stdafx.h"
#include "kuiPropBar.h"
#include ".\kuipropbar.h"


// kuiPropBar

IMPLEMENT_DYNAMIC(kuiPropBar, CPinDockBar)
kuiPropBar::kuiPropBar()
{
}

kuiPropBar::~kuiPropBar()
{
}


BEGIN_MESSAGE_MAP(kuiPropBar, CPinDockBar)
	ON_WM_CREATE()
	ON_WM_SIZE()
END_MESSAGE_MAP()



// kuiPropBar 消息处理程序

void kuiPropBar::OnUpdateCmdUI(CFrameWnd* /*pTarget*/, BOOL /*bDisableIfNoHndler*/)
{
}

int kuiPropBar::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CPinDockBar::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  在此添加您专用的创建代码

	return 0;
}

void kuiPropBar::OnSize(UINT nType, int cx, int cy)
{
	CPinDockBar::OnSize(nType, cx, cy);

	// TODO: 在此处添加消息处理程序代码
}
