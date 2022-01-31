#pragma once

#include "..\resource.h"

// CTextEditDialog 对话框

class CTextEditDialog : public CDialog
{
	DECLARE_DYNAMIC(CTextEditDialog)

public:
	CTextEditDialog(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CTextEditDialog();

// 对话框数据
	enum { IDD = IDD_DIALOG_TEXTEDIT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	LOGFONT		m_logFont;
	CString		m_szText;
	double		m_dTextHeight;
	int			m_nCharSpace;
	BOOL		m_bGroup;
	BOOL		m_bMakeFace;
	afx_msg void OnBnClickedButtonSelfont();
	virtual BOOL OnInitDialog();
protected:
	virtual void OnOK();

protected:
	BOOL		m_bSelFont;
	
};
