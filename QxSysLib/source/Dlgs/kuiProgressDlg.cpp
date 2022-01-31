// Dialogs\kuiProgressDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "kcProgressThread.h"
#include "kuiProgressDlg.h"


// kuiProgressDlg 对话框

IMPLEMENT_DYNAMIC(kuiProgressDlg, CDialog)

kuiProgressDlg::kuiProgressDlg(kcProgressThread *pProgThread,CWnd* pParent /*=NULL*/)
: CDialog(kuiProgressDlg::IDD, pParent),pProgThread_(pProgThread)
, szProgText_(_T(""))
{
	nIDTimer_ = 0;
}

kuiProgressDlg::~kuiProgressDlg()
{
}

void kuiProgressDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_STATIC_PROGTEXT, szProgText_);
}


BEGIN_MESSAGE_MAP(kuiProgressDlg, CDialog)
	ON_WM_TIMER()
	ON_BN_CLICKED(ID_BUTTON_TERMINATE, &kuiProgressDlg::OnClickedButtonTerminate)
END_MESSAGE_MAP()


// kuiProgressDlg 消息处理程序


BOOL kuiProgressDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	nIDTimer_ = SetTimer(1,20,NULL);

	return TRUE;  // return TRUE unless you set the focus to a control
}


void kuiProgressDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if(!pProgThread_->bRunning_){
		KillTimer(nIDTimer_);

		CDialog::OnOK();
	}else{
		std::string szText = pProgThread_->GetProgText();
		szProgText_ = szText.c_str();
		UpdateData(FALSE);
	}

	CDialog::OnTimer(nIDEvent);
}


void kuiProgressDlg::OnClickedButtonTerminate()
{
	// TODO: 在此添加控件通知处理程序代码
	pProgThread_->Stop();

	if(nIDTimer_){
		KillTimer(nIDTimer_);
	}

	CDialog::OnOK();
}
