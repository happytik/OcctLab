// MainFrm.cpp : CMainFrame 类的实现
//

#include "stdafx.h"
#include "QxCommandItem.h"
#include "QxPluginMgr.h"
#include "kiCommandMgr.h"
#include "XInputEdit.h"
#include "OcctLab.h"
#include "OcctLabDoc.h"
#include "kcModel.h"
#include "kBase.h"
#include "QxBaseDef.h"
#include "QxWndTimerMgr.h"
#include "QxLuaScript.h"
#include "resource.h"
#include "MainFrm.h"

//#ifdef _DEBUG
//#define new DEBUG_NEW
//#endif

// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)
IMPLEMENT_PINDOCK(CMainFrame)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	ON_WM_CREATE()
	ON_PINDOCK_MESSAGES()
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	ON_COMMAND(ID_MENU_LUA_SCRIPT, &CMainFrame::OnMenuLuaScript)
	ON_COMMAND(ID_COM_TEST_ADD, &CMainFrame::OnComTestAdd)
	ON_WM_TIMER()
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // 状态行指示器
	IDS_STATUS_PROMPT,
	//ID_INDICATOR_CAPS,
	//ID_INDICATOR_NUM,
	//ID_INDICATOR_SCRL,
};


// CMainFrame 构造/析构

CMainFrame::CMainFrame()
{
	// TODO: 在此添加成员初始化代码
	m_pTimerMgr = NULL;
}

CMainFrame::~CMainFrame()
{
	KC_SAFE_DELETE(m_pTimerMgr);
}

CXInputEdit*	CMainFrame::GetInputEdit()
{
	ASSERT(m_wndCmdBar.GetSafeHwnd());
	return m_wndCmdBar.GetInputEdit();
}

//获取模型树的接口
kuiModelTreeInterface* CMainFrame::GetModelTreeInterface()
{
	ASSERT(m_wndMainBar.GetSafeHwnd());
	return m_wndMainBar.GetModelTreeInterface();
}

QxMainWndTimerMgr* CMainFrame::GetTimerMgr() const
{
	return m_pTimerMgr;
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	if(!CreateMainMenu())
	{
		return -1;
	}
	
	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("未能创建工具栏\n");
		return -1;      // 未能创建
	}

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("未能创建状态栏\n");
		return -1;      // 未能创建
	}
	m_wndStatusBar.SetPaneInfo(1,IDS_STATUS_PROMPT,SBPS_NORMAL,160);

	if(!m_wndCmdBar.Create(_T("Command Bar"),this,130))
	{
		TRACE0("Failed to create cmd bar\n");
		return -1;      // fail to create
	}

	m_wndCmdBar.SetBarStyle(m_wndCmdBar.GetBarStyle() |
		CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);

	//
	int ix;
	m_wndBarImgList.Create(IDB_BITMAP_IMGLIST, 16, 1, RGB(255,0,255));
	for(ix = 0;ix < 4; ix ++)
		m_AutoHideBar[ix].SetImageList(&this->m_wndBarImgList);
	
	//
	if(!m_wndMainBar.Create(_T("Main Bar"),this,131))
	{
		TRACE0("Failed to create main bar\n");
		return -1;		// fail to create
	}
	m_wndMainBar.SetBarStyle(m_wndMainBar.GetBarStyle() |
		CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);
	m_wndMainBar.EnableDocking(CBRS_ALIGN_LEFT | CBRS_ALIGN_RIGHT);

	// TODO: 如果不需要工具栏可停靠，则删除这三行
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	m_wndCmdBar.EnableDocking(CBRS_ALIGN_BOTTOM);

	EnablePinDocking(CBRS_ALIGN_ANY);
	//EnableDocking(CBRS_ALIGN_ANY);

	DockControlBar(&m_wndToolBar);
	DockControlBar(&m_wndCmdBar,AFX_IDW_DOCKBAR_BOTTOM);
	DockControlBar(&m_wndMainBar,AFX_IDW_DOCKBAR_LEFT);

	//
	m_pTimerMgr = new QxMainWndTimerMgr(this->GetSafeHwnd());

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: 在此处通过修改 CREATESTRUCT cs 来修改窗口类或
	// 样式

	return TRUE;
}


// CMainFrame 诊断

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG

int		CMainFrame::ShowInputString(LPCTSTR lpszInputStr)
{
	if(::IsWindow(m_wndCmdBar.m_hWnd))
		m_wndCmdBar.ShowInputString(lpszInputStr);

	return 0;
}

int	CMainFrame::ShowPrompt(LPCTSTR lpszPrompt)
{
	m_wndStatusBar.SetPaneText(1,lpszPrompt);

	return 0;
}

//根据插件创建菜单
BOOL	CMainFrame::CreateMainMenu()
{
	QxPluginMgr *pPluginMgr = ((COcctLabApp *)AfxGetApp())->GetPluginMgr();
	if(!pPluginMgr)
		return FALSE;

	QxCommandItem *pCmdItem = NULL;
	UINT ix,iCount = pPluginMgr->GetItemCount();
	if(iCount == 0)
		return FALSE;

	CMenu *pSubMenu = NULL;
	CMenu *pMainMenu = GetMenu();
	if(!pMainMenu)
		return FALSE;

	int nPos = 6;
	for(ix = 0;ix < iCount;ix ++){
		pCmdItem = pPluginMgr->GetCmdItem(ix);
		//
		const char *pszTopMenuText = DoGetTopMenuText(pCmdItem);
		ASSERT(pszTopMenuText);
		pSubMenu = FindSubMenu(pMainMenu,pszTopMenuText);
		if(!pSubMenu){
			//创建菜单
			CMenu subMenu;
			subMenu.CreatePopupMenu();
			pMainMenu->InsertMenu(nPos ++,MF_POPUP | MF_BYPOSITION,(UINT_PTR)subMenu.m_hMenu,
				pszTopMenuText);
			//添加子菜单
			if(pCmdItem->_nLevel == 2){
				subMenu.AppendMenu(MF_STRING,pCmdItem->_nCmdID,pCmdItem->_strMenu[1].c_str());
				if(pCmdItem->_nMenuFlag & KUI_MENU_AFTER_SEP)
					subMenu.AppendMenu(MF_SEPARATOR,0,(LPCTSTR)NULL);
			}else{
				UINT nIdx = subMenu.GetMenuItemCount();
				CMenu ssMenu;
				ssMenu.CreatePopupMenu();
				subMenu.InsertMenu(nIdx,MF_POPUP | MF_BYPOSITION,(UINT_PTR)ssMenu.m_hMenu,
					pCmdItem->_strMenu[1].c_str());
				//
				ssMenu.AppendMenu(MF_STRING,pCmdItem->_nCmdID,pCmdItem->_strMenu[2].c_str());

				if(pCmdItem->_nMenuFlag & KUI_MENU_AFTER_SEP)
					subMenu.AppendMenu(MF_SEPARATOR,0,(LPCTSTR)NULL);

				ssMenu.Detach();
			}
			
			subMenu.Detach();
		}else{
			int nbSubMenu = pSubMenu->GetMenuItemCount();
			if(nbSubMenu >= 2 && nbSubMenu <= 4){
				int nPos = FindMenuItem(pSubMenu, "xxxxxx");
				if(nPos == 0){
					TRACE("\n Find Dumb menu, remove it.\n");
					pSubMenu->RemoveMenu(0,MF_BYPOSITION);
				}
			}
			if(pCmdItem->_nMenuFlag & KUI_MENU_BEFORE_SEP)
				pSubMenu->AppendMenu(MF_SEPARATOR,0,(LPCTSTR)NULL);

			if(pCmdItem->_nLevel == 2){
				pSubMenu->AppendMenu(MF_STRING,pCmdItem->_nCmdID,pCmdItem->_strMenu[1].c_str());
				if(pCmdItem->_nMenuFlag & KUI_MENU_AFTER_SEP)
					pSubMenu->AppendMenu(MF_SEPARATOR,0,(LPCTSTR)NULL);
			}else{
				CMenu *pSsMenu = FindSubMenu(pSubMenu,pCmdItem->_strMenu[1].c_str());
				if(!pSsMenu){
					UINT nIdx = pSubMenu->GetMenuItemCount();
					CMenu ssMenu;
					ssMenu.CreatePopupMenu();
					pSubMenu->InsertMenu(nIdx,MF_POPUP | MF_BYPOSITION,(UINT_PTR)ssMenu.m_hMenu,
						pCmdItem->_strMenu[1].c_str());
					//
					ssMenu.AppendMenu(MF_STRING,pCmdItem->_nCmdID,pCmdItem->_strMenu[2].c_str());
					if(pCmdItem->_nMenuFlag & KUI_MENU_AFTER_SEP)
						ssMenu.AppendMenu(MF_SEPARATOR,0,(LPCTSTR)NULL);

					ssMenu.Detach();
				}else{
					pSsMenu->AppendMenu(MF_STRING,pCmdItem->_nCmdID,pCmdItem->_strMenu[2].c_str());
					if(pCmdItem->_nMenuFlag & KUI_MENU_AFTER_SEP)
						pSsMenu->AppendMenu(MF_SEPARATOR,0,(LPCTSTR)NULL);
				}
			}
		}
	}

	this->DrawMenuBar();
	
	return TRUE;
}

CMenu*	CMainFrame::FindSubMenu(CMenu *pMenu,const char* pszName)
{
	CMenu *pSubMenu = NULL;
	CString szText;
	UINT ix,iCount = pMenu->GetMenuItemCount();

	for(ix = 0;ix < iCount;ix ++){
		pMenu->GetMenuString(ix,szText,MF_BYPOSITION);
		if(szText.Compare(pszName) == 0){
			pSubMenu = pMenu->GetSubMenu(ix);
			if(pSubMenu){
				break;
			}
		}
	}

	return pSubMenu;
}

int CMainFrame::FindMenuItem(CMenu *pMenu,const char* pszName)
{
	CMenu *pSubMenu = NULL;
	CString szText;
	int ix,iCount = pMenu->GetMenuItemCount();

	for(ix = 0;ix < iCount;ix ++){
		pMenu->GetMenuString(ix,szText,MF_BYPOSITION);
		if(szText.Compare(pszName) == 0){
			return ix;
		}
	}
	return -1;
}

// CMainFrame 消息处理程序


void CMainFrame::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	KTRACE("hh\n");
	m_wndStatusBar.SetPaneText(1,"hello,world");

	CFrameWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CMainFrame::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	CFrameWnd::OnKeyUp(nChar, nRepCnt, nFlags);
}

BOOL CMainFrame::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类
	if(pMsg->message == WM_KEYDOWN || pMsg->message == WM_KEYUP ||
	   pMsg->message == WM_CHAR)
	{
		CXInputEdit *pInputEdit = GetInputEdit();
		if(pInputEdit)
		{
			CWnd *pFocusWnd = GetFocus();
			//只有在没有焦点的情况下,才直接发送.
			if(pFocusWnd && pFocusWnd->GetSafeHwnd() != pInputEdit->GetSafeHwnd())
				pInputEdit->SendMessage(pMsg->message,pMsg->wParam,pMsg->lParam);
		}
	}

	return CFrameWnd::PreTranslateMessage(pMsg);
}

void CMainFrame::OnUpdateFrameTitle(BOOL bAddToTitle)
{
	// TODO: 在此添加专用代码和/或调用基类

	CFrameWnd::OnUpdateFrameTitle(bAddToTitle);
}


void CMainFrame::OnMenuLuaScript()
{
	// TODO: 在此添加命令处理程序代码
	//m_wndStatusBar.SetPaneText(1,"execute luadlg");
	ShowLuaScriptDialog(true);
}


void CMainFrame::OnComTestAdd()
{
	ComTest_Add();	
}


void CMainFrame::OnTimer(UINT_PTR nIDEvent)
{
	if(m_pTimerMgr){
		m_pTimerMgr->OnTimer(nIDEvent,::GetTickCount());
	}

	CFrameWnd::OnTimer(nIDEvent);
}
