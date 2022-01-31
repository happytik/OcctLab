// Dialogs\LineStyleAndWidthDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "OcctLab.h"
#include "LineStyleAndWidthDlg.h"
#include "afxdialogex.h"


// CLineStyleAndWidthDlg 对话框

IMPLEMENT_DYNAMIC(CLineStyleAndWidthDlg, CDialog)

CLineStyleAndWidthDlg::CLineStyleAndWidthDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CLineStyleAndWidthDlg::IDD, pParent)
	, m_nLineStyle(0)
	, m_dLineWidth(1.0)
{

}

CLineStyleAndWidthDlg::~CLineStyleAndWidthDlg()
{
}

void CLineStyleAndWidthDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_CBIndex(pDX, IDC_COMBO_LINE_STYLE, m_nLineStyle);
	DDX_Text(pDX, IDC_EDIT_LINE_WIDTH, m_dLineWidth);
}


BEGIN_MESSAGE_MAP(CLineStyleAndWidthDlg, CDialog)
END_MESSAGE_MAP()


// CLineStyleAndWidthDlg 消息处理程序


BOOL CLineStyleAndWidthDlg::OnInitDialog()
{
	CDialog::OnInitDialog();


	return TRUE;  // return TRUE unless you set the focus to a control
}
