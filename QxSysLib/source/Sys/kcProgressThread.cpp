#include "stdafx.h"
#include "..\dlgs\kuiProgressDlg.h"
#include "kcProgressThread.h"


kcProgressThread::kcProgressThread(void)
{
	bRunning_ = false;
	bQuit_ = false;

	szProgText_ = "";
}


kcProgressThread::~kcProgressThread(void)
{
	Stop();
}

bool  kcProgressThread::Start()
{
	if(bRunning_){
		ASSERT(FALSE);
		return false;
	}

	_beginthread(kcProgressThread::ThreadProc,0,(void *)this);

	kuiProgressDlg dlg(this);
	dlg.DoModal();

	return bRunning_;
}

int  kcProgressThread::Stop()
{
	if(!bRunning_){
		return 0;
	}

	bQuit_ = true;

	return 1;
}

void  kcProgressThread::ThreadProc(void *pArg)
{
	kcProgressThread *pThis = (kcProgressThread *)pArg;

	pThis->bRunning_ = true;
	pThis->bQuit_ = false;

	pThis->Run();

	pThis->bRunning_ = false;
}

void  kcProgressThread::Run() 
{}

std::string  kcProgressThread::	GetProgText()
{
	std::string ss;
	aCSLock_.Lock();
	ss = szProgText_;
	aCSLock_.Unlock();

	return ss;
}

void  kcProgressThread::SetProgText(const std::string &szText)
{
	aCSLock_.Lock();
	szProgText_ = szText;
	aCSLock_.Unlock();
}