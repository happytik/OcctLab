#pragma once
#include "afxwin.h"

#include "..\resource.h"

// CLuaScriptDlg 对话框

class CLuaScriptDlg : public CDialog
{
	DECLARE_DYNAMIC(CLuaScriptDlg)

public:
	CLuaScriptDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CLuaScriptDlg();

// 对话框数据
	enum { IDD = IDD_DIALOG_LUA_SCRIPT };

public:
	//输出一行
	void				OutputLine(const char *pszLine);
	//输出数据
	void				Output(const char *pszText);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonCheckScript();
	afx_msg void OnBnClickedButtonRunScript();
	afx_msg void OnBnClickedButtonClearScript();
	afx_msg void OnBnClickedButtonClearOutput();

protected:
	CString m_szScript;
	CString m_szOutput;
public:
	CEdit m_outputEdit;
};
