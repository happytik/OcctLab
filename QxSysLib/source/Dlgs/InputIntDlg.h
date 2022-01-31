#pragma once

//#include "afxdialogex.h"
#include "..\resource.h"

// CInputIntDlg 对话框

class CInputIntDlg : public CDialog
{
	DECLARE_DYNAMIC(CInputIntDlg)

public:
	CInputIntDlg(int value,const char *pszPrompt,CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CInputIntDlg();

// 对话框数据
	enum { IDD = IDD_DIALOG_INPUT_INT };

	void		SetSpinIncrement(int incVal);

	int		GetValue() const { return m_nValue; }

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()

protected:
	int m_nValue;
	CString m_szPrompt;
	int m_nSpinIncrement;//spin每次增加或减少的量

	virtual BOOL OnInitDialog();
	virtual void OnOK();
	
public:
	afx_msg void OnDeltaposSpinIntValue(NMHDR *pNMHDR, LRESULT *pResult);
};
