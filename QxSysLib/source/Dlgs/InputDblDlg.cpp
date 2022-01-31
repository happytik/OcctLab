// Dialogs\InputDblDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "InputDblDlg.h"


// CInputDblDlg 对话框

IMPLEMENT_DYNAMIC(CInputDblDlg, CDialogEx)

CInputDblDlg::CInputDblDlg(double value,const char *pszPrompt,CWnd* pParent /*=NULL*/)
	: CDialogEx(CInputDblDlg::IDD, pParent)
	, m_dValue(value)
	, m_szPrompt(pszPrompt)
{
	m_dSpinIncrement = 1.0;//默认值
}

CInputDblDlg::~CInputDblDlg()
{
}

void CInputDblDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_DBL_VALUE, m_dValue);
	DDX_Text(pDX, IDC_STATIC_DV_PROMPT, m_szPrompt);
}

void CInputDblDlg::SetSpinIncrement(double incVal)
{
	ASSERT(incVal > 0.0);
	m_dSpinIncrement = incVal;
}

BEGIN_MESSAGE_MAP(CInputDblDlg, CDialogEx)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_DBL_VALUE, &CInputDblDlg::OnDeltaposSpinDblValue)
END_MESSAGE_MAP()


// CInputDblDlg 消息处理程序


BOOL CInputDblDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
}


void CInputDblDlg::OnOK()
{
	// TODO: 在此添加专用代码和/或调用基类
	UpdateData(TRUE);

	CDialogEx::OnOK();
}


void CInputDblDlg::OnDeltaposSpinDblValue(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);

	if(pNMUpDown != NULL){
		TRACE("ipos = %d,idelta = %d.\n",pNMUpDown->iPos,pNMUpDown->iDelta);
		if(pNMUpDown->iDelta < 0){
			m_dValue += m_dSpinIncrement;
		}else{
			m_dValue -= m_dSpinIncrement;
		}

		UpdateData(FALSE);
	}
	
	*pResult = 0;
}
