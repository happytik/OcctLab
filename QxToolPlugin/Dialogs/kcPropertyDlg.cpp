// Dialogs\kcPropertyDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "kcPropertyDlg.h"


// kcPropertyDlg 对话框

IMPLEMENT_DYNAMIC(kcPropertyDlg, CDialog)

kcPropertyDlg::kcPropertyDlg(kcPropHandler *pPropHandler,CWnd* pParent /*=NULL*/)
:m_pPropHandler(pPropHandler),CDialog(kcPropertyDlg::IDD, pParent)
{

}

kcPropertyDlg::~kcPropertyDlg()
{
	m_pPropHandler = NULL;
}

void kcPropertyDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROPERTY_TREE, m_propTree);
}

void  kcPropertyDlg::ClearAllProperty()
{
	m_propTree.DeleteAllItems();
}

BEGIN_MESSAGE_MAP(kcPropertyDlg, CDialog)
END_MESSAGE_MAP()


// kcPropertyDlg 消息处理程序


BOOL kcPropertyDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	ClearAllProperty();
	//加入新的
	if(m_pPropHandler){
		m_pPropHandler->SetProperty(m_propTree);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
}


void kcPropertyDlg::OnOK()
{
	// TODO: 在此添加专用代码和/或调用基类

	CDialog::OnOK();
}
