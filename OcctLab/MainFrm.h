// MainFrm.h : CMainFrame 类的接口
//
#ifndef _MAIN_FRAME_H_
#define _MAIN_FRAME_H_

#include "autoHideBar.h"
#include "kuiCmdBar.h"
#include "kuiMainBar.h"

class CXInputEdit;
class kuiModelTreeInterface;
class QxCommandItem;
class QxMainWndTimerMgr;

class CMainFrame : public CFrameWnd
{
	
protected: // 仅从序列化创建
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

// 属性
public:

// 操作
public:
	int						ShowInputString(LPCTSTR lpszInputStr);
	int						ShowPrompt(LPCTSTR lpszPrompt);

	CXInputEdit*				GetInputEdit();
	kuiModelTreeInterface		*GetModelTreeInterface();
	QxMainWndTimerMgr*		GetTimerMgr() const;
	
// 重写
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// 实现
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // 控件条嵌入成员
	CStatusBar		m_wndStatusBar;
	CToolBar		m_wndToolBar;
	kuiCmdBar		m_wndCmdBar;
	kuiMainBar		m_wndMainBar;

	CImageList		m_wndBarImgList;

	QxMainWndTimerMgr	*m_pTimerMgr;

protected:
	// 根据插件创建菜单
	BOOL			CreateMainMenu();

	CMenu*			FindSubMenu(CMenu *pMenu,const char* pszName);
	int				FindMenuItem(CMenu *pMenu,const char* pszName);
	const char*		DoGetTopMenuText(QxCommandItem *pItem);

// 生成的消息映射函数
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	DECLARE_MESSAGE_MAP()

	DECLARE_PINDOCK()
public:
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
protected:
	virtual void OnUpdateFrameTitle(BOOL bAddToTitle);
public:
	afx_msg void OnMenuLuaScript();
	afx_msg void OnComTestAdd();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};

void ComTest_Add();

#endif
