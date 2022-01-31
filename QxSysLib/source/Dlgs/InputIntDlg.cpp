// Dialogs\InputIntDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "InputIntDlg.h"


// CInputIntDlg 对话框

IMPLEMENT_DYNAMIC(CInputIntDlg, CDialog)

CInputIntDlg::CInputIntDlg(int value,const char *pszPrompt,CWnd* pParent /*=NULL*/)
	: CDialog(CInputIntDlg::IDD, pParent)
	, m_nValue(value)
	, m_szPrompt(pszPrompt)
{
	m_nSpinIncrement = 1;//默认值
}

CInputIntDlg::~CInputIntDlg()
{
}

void CInputIntDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_INT_VAL, m_nValue);
	DDX_Text(pDX, IDC_STATIC_IV_PROMPT, m_szPrompt);
}

void CInputIntDlg::SetSpinIncrement(int incVal)
{
	ASSERT(incVal > 0);
	m_nSpinIncrement = incVal;
}

BEGIN_MESSAGE_MAP(CInputIntDlg, CDialog)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_INT_VALUE, &CInputIntDlg::OnDeltaposSpinIntValue)
END_MESSAGE_MAP()


// CInputIntDlg 消息处理程序


BOOL CInputIntDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
}


void CInputIntDlg::OnOK()
{
	// TODO: 在此添加专用代码和/或调用基类
	UpdateData(TRUE);

	CDialog::OnOK();
}

void CInputIntDlg::OnDeltaposSpinIntValue(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	
	if(pNMUpDown != NULL){
		if(pNMUpDown->iDelta < 0){
			m_nValue += m_nSpinIncrement;
		}else{
			m_nValue -= m_nSpinIncrement;
		}

		UpdateData(FALSE);
	}

	*pResult = 0;
}
