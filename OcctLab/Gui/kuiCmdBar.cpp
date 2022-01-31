// GUI\kuiCmdBar.cpp : 实现文件
//

#include "stdafx.h"
#include "kuiCmdBar.h"
#include ".\kuicmdbar.h"


// kuiCmdBar

IMPLEMENT_DYNAMIC(kuiCmdBar, CSizingControlBarG)
kuiCmdBar::kuiCmdBar()
{
	m_nEditHeight = -1;
}

kuiCmdBar::~kuiCmdBar()
{
}

CXInputEdit*	kuiCmdBar::GetInputEdit()
{
	ASSERT(m_inputEdit.GetSafeHwnd());
	return &m_inputEdit;
}

BEGIN_MESSAGE_MAP(kuiCmdBar, CSizingControlBarG)
	ON_WM_CREATE()
	ON_WM_SIZE()
END_MESSAGE_MAP()



// kuiCmdBar 消息处理程序

void kuiCmdBar::OnUpdateCmdUI(CFrameWnd* /*pTarget*/, BOOL /*bDisableIfNoHndler*/)
{
}

//显示一个提示信息
int		kuiCmdBar::ShowInputString(LPCTSTR lpszInputStr)
{
	return 0;
}

int		kuiCmdBar::ShowPrompt(LPCTSTR lpszPrompt)
{
	return 0;
}

int kuiCmdBar::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CSizingControlBarG::OnCreate(lpCreateStruct) == -1)
		return -1;

	//设置类型
	SetSCBStyle(GetSCBStyle() | SCBS_SHOWEDGES);

	if(!m_inputEdit.Create(NULL,_T(""),WS_CHILD | WS_VISIBLE,CRect(0,0,0,0),this,123))
	{
		TRACE("\n Create Input edit failed.");
		return -1;
	}
	//m_inputEdit.SetPromptStr("测试:");

	LOGFONT lf;
	memset(&lf,0,sizeof(LOGFONT));
	lf.lfHeight = 13;
	strcpy(lf.lfFaceName,"宋体");
	m_font.CreateFontIndirect(&lf);

	m_szHorz.cy = 40;

	return 0;
}

void kuiCmdBar::OnSize(UINT nType, int cx, int cy)
{
	CSizingControlBarG::OnSize(nType, cx, cy);

	if(cx > 0 && cy > 0)
	{
		if(m_nEditHeight < 0)
		{
			TEXTMETRIC tm;
			CClientDC dc(this);
			CFont *pOldFont = dc.SelectObject(&m_font);
			GetTextMetrics(dc.GetSafeHdc(),&tm);
			dc.SelectObject(pOldFont);

			m_nEditHeight = tm.tmHeight + 6;
		}
		m_inputEdit.MoveWindow(4,4,cx - 8,m_nEditHeight);
	}
}
