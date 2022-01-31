// GUI\kuiLayerBar.cpp : 实现文件
//

#include "stdafx.h"
#include "kuiLayerBar.h"
#include ".\kuilayerbar.h"


// kuiLayerBar

IMPLEMENT_DYNAMIC(kuiLayerBar, CPinDockBar)
kuiLayerBar::kuiLayerBar()
{
}

kuiLayerBar::~kuiLayerBar()
{
}


BEGIN_MESSAGE_MAP(kuiLayerBar, CPinDockBar)
	ON_WM_CREATE()
	ON_WM_SIZE()
END_MESSAGE_MAP()



// kuiLayerBar 消息处理程序

void kuiLayerBar::OnUpdateCmdUI(CFrameWnd* /*pTarget*/, BOOL /*bDisableIfNoHndler*/)
{
}

int kuiLayerBar::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CPinDockBar::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  在此添加您专用的创建代码

	return 0;
}

void kuiLayerBar::OnSize(UINT nType, int cx, int cy)
{
	CPinDockBar::OnSize(nType, cx, cy);

	// TODO: 在此处添加消息处理程序代码
}
