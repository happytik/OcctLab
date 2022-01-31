#pragma once


// CLineStyleAndWidthDlg 对话框

class CLineStyleAndWidthDlg : public CDialog
{
	DECLARE_DYNAMIC(CLineStyleAndWidthDlg)

public:
	CLineStyleAndWidthDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CLineStyleAndWidthDlg();

// 对话框数据
	enum { IDD = IDD_DIALOG_LINE_DISPATTR };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();

	
public:
	int m_nLineStyle;
	double m_dLineWidth;
};
