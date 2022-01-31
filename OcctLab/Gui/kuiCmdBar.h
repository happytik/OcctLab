////////////////////////////////////////////////////////////////
//
// kuiCmdBar

#ifndef _KUI_CMD_BAR_H_
#define _KUI_CMD_BAR_H_

#include "scbarg.h"
#include "XInputEdit.h"

class kuiCmdBar : public CSizingControlBarG
{
	DECLARE_DYNAMIC(kuiCmdBar)

public:
	kuiCmdBar();
	virtual ~kuiCmdBar();
	virtual void OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler);

	//显示一个提示信息
	int					ShowInputString(LPCTSTR lpszInputStr);
	int					ShowPrompt(LPCTSTR lpszPrompt);

	CXInputEdit*		GetInputEdit();

protected:
	CXInputEdit			m_inputEdit;
	int					m_nEditHeight;
	CFont				m_font;

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
};

#endif
