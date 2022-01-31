#pragma once

#include <vector>
#include <stack>
#include "kiOptionSet.h"
#include "kiInputEdit.h"
#include "QxLibDef.h"

//记录一个输入请求信息
class QXLIB_API XInputRequest{
public:
	XInputRequest();
	XInputRequest(const char *pszPrompt,kiInputEditCallBack *pCallBack,kiOptionSet *pOptSet);
	XInputRequest(const XInputRequest& other);

	XInputRequest&			operator=(const XInputRequest& other);

public:
	std::string				_ssPromptText;
	kiInputEditCallBack		*_pInputEditCallBack;
	kiOptionSet				*_pOptionSet;
};

typedef std::stack<XInputRequest>	stdInputRequestStack;

//记录一个option的位置信息
class XOptionPos{
public:
	XOptionPos(){
		_rect.left = _rect.top = _rect.right = _rect.bottom = 0;
		_bHilight = false;
	}

public:
	RECT				_rect;//option在edit中占据的区域范围
	bool				_bHilight;//是否高亮显示，由于选中、点击等
};

// 记录一个char的位置信息，为计算caret方便。
class XCharPos{
public:
	XCharPos(){
		_nStart = _nEnd = 0;
	}

public:
	int				_nStart,_nEnd;//在Edit内的起始和终止位置，单位是：像素
};

// CXInputEdit

class QXLIB_API CXInputEdit : public CWnd,public kiInputEdit
{
	DECLARE_DYNAMIC(CXInputEdit)

public:
	CXInputEdit();
	virtual ~CXInputEdit();

	// 必要的初始化
	BOOL						Initialize();

	//主要接口，为输入工具显示一条提示信息，可以带选项列表
	//InputTool指针不能为NULL.
	virtual int				RequestInput(const char *pPromptText,
										 kiInputEditCallBack *pEditCallBack,
										 kiOptionSet *pOptionSet = NULL);
	//显示一条提示信息.
	virtual void				PromptText(const char *lpszText);

	//更新所有
	virtual void				Update();

	//获取输入文本
	virtual std::string		GetInputText() const;		

	//用于保存当前输入请求和恢复输入请求
	virtual int				PushRequest();
	virtual int				PopRequest();

protected:
	// 计算提示文本的信息
	BOOL						CalcPromptTextInfo();
	//
	void						CalcCaretPos(CPoint point);

	void						UpdateCaret();

	// 接受一个字符
	int						AcceptChar(char c);
	int						DeleteChar(BOOL bBack);

protected:
	//提示文本信息
	CString					m_szPromptText;//传入提示文本
	CString					m_szFullPromptText;//包含了选项信息的完整的提示文本，不可更改。
	CRect					m_promptRect;//提示文本的范围
	
	//输入文本信息
	CString					m_szInputText;//用户输入输入文本
	std::vector<XCharPos>		m_chrPosArray;//记录了输入文字的每个字符的位置信息
	int						m_nCaretIdx;//Caret对应的chrPosArray的下标。0-size,size表示最后。

	CString					m_szFullText;//上面的整个文本。包括: prompt : input

protected:
	std::vector<XOptionPos>	m_optPosArray;//记录option的位置信息

	int						HitOptionTest(CPoint point);

protected:
	CFont					m_textFont; //文本字体
	COLORREF					m_selOptionColor;//选中的option的颜色。
	int						m_nCaretHeight;//光标的高度
	int						m_nMargin;//空隙,距离左边和上边.
	BOOL						m_bHasFocus;//是否有焦点

protected:
	stdInputRequestStack		m_inputReqStack;//输入请求栈
	
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg UINT OnGetDlgCode();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
};


