// GUI\TextEditDialog.cpp : 实现文件
//

#include "stdafx.h"
#include "TextEditDialog.h"


// CTextEditDialog 对话框

IMPLEMENT_DYNAMIC(CTextEditDialog, CDialog)
CTextEditDialog::CTextEditDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CTextEditDialog::IDD, pParent)
	, m_szText(_T(""))
	, m_nCharSpace(2)
	, m_bGroup(FALSE)
	, m_bMakeFace(FALSE)
	, m_dTextHeight(16.0)
{
}

CTextEditDialog::~CTextEditDialog()
{
}

void CTextEditDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_TEXTLINE, m_szText);
	DDX_Text(pDX, IDC_EDIT_CHAR_SPACE, m_nCharSpace);
	//DDX_Check(pDX, IDC_CHECK_GROUP, m_bGroup);
	DDX_Check(pDX, IDC_CHECK_BUILD_FACE, m_bMakeFace);
	DDX_Text(pDX, IDC_EDIT_TEXT_HEIGHT, m_dTextHeight);
	DDV_MinMaxDouble(pDX, m_dTextHeight, 0.0, 10000.0);
}


BEGIN_MESSAGE_MAP(CTextEditDialog, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_SELFONT, OnBnClickedButtonSelfont)
END_MESSAGE_MAP()


// CTextEditDialog 消息处理程序

void CTextEditDialog::OnBnClickedButtonSelfont()
{
	CFontDialog fontDlg;
	if(IDOK == fontDlg.DoModal())
	{
		m_bSelFont = TRUE;
		fontDlg.GetCurrentFont(&m_logFont);
	}
}

BOOL CTextEditDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_bSelFont = FALSE;
	memset(&m_logFont,0,sizeof(LOGFONT));

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CTextEditDialog::OnOK()
{
	UpdateData();

	if(m_szText.IsEmpty() || !m_bSelFont)
	{
		return;
	}

	CDialog::OnOK();
}
