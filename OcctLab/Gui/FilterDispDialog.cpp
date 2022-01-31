// Gui\FilterDispDialog.cpp : 实现文件
//

#include "stdafx.h"
#include "..\OcctLab.h"
#include "FilterDispDialog.h"


// CFilterDispDialog 对话框

IMPLEMENT_DYNAMIC(CFilterDispDialog, CDialog)

CFilterDispDialog::CFilterDispDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CFilterDispDialog::IDD, pParent)
	, m_bDispPoint(TRUE)
	, m_bDispCurve(TRUE)
	, m_bDispWire(TRUE)
	, m_bDispSurface(TRUE)
	, m_bDispShell(FALSE)
	, m_bDispSolid(TRUE)
	, m_bDispCompSolid(TRUE)
	, m_bDispCompound(TRUE)
{

}

CFilterDispDialog::~CFilterDispDialog()
{
}

void CFilterDispDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CHECK_POINT, m_bDispPoint);
	DDX_Check(pDX, IDC_CHECK_CURVE, m_bDispCurve);
	DDX_Check(pDX, IDC_CHECK_WIRE, m_bDispWire);
	DDX_Check(pDX, IDC_CHECK_SURFACE, m_bDispSurface);
	DDX_Check(pDX, IDC_CHECK_SHELL, m_bDispShell);
	DDX_Check(pDX, IDC_CHECK_SOLID, m_bDispSolid);
	DDX_Check(pDX, IDC_CHECK_COMPSOLID, m_bDispCompSolid);
	DDX_Check(pDX, IDC_CHECK_COMPOUND, m_bDispCompound);
}


BEGIN_MESSAGE_MAP(CFilterDispDialog, CDialog)
	ON_BN_CLICKED(IDOK, &CFilterDispDialog::OnBnClickedOk)
END_MESSAGE_MAP()


// CFilterDispDialog 消息处理程序

BOOL CFilterDispDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  在此添加额外的初始化

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CFilterDispDialog::OnOK()
{
	// TODO: 在此添加专用代码和/或调用基类

	CDialog::OnOK();
}

void CFilterDispDialog::OnBnClickedOk()
{
	
	OnOK();
}
