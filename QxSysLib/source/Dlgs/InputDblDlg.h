#pragma once

#include "afxdialogex.h"
#include "..\resource.h"

// CInputDblDlg 对话框

class CInputDblDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CInputDblDlg)

public:
	CInputDblDlg(double value,const char *pszPrompt,CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CInputDblDlg();

	void			SetSpinIncrement(double incVal);

	double		GetValue() const { return m_dValue; }

// 对话框数据
	enum { IDD = IDD_DIALOG_INPUT_DOUBLE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()

protected:
	double m_dValue;
	CString m_szPrompt;
	double m_dSpinIncrement;//spin每次增加或减少的数量

	virtual BOOL OnInitDialog();
	virtual void OnOK();
	
public:
	afx_msg void OnDeltaposSpinDblValue(NMHDR *pNMHDR, LRESULT *pResult);
};
