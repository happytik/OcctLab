//
#include "stdafx.h"
#include "QxWndTimerMgr.h"

/////////////////////////////////////////////////////////////////
//
class QxTimerState{
public:
	QxTimerState(UINT_PTR nID,IQxTimerHandler *pHandler);

	int					OnTimer(DWORD dwTime);

	void					PauseTimer();
	void					ResumeTimer();

	void					OnStopped();

public:
	UINT_PTR				nIDEvent_;
	IQxTimerHandler		*pHandler_;
	bool					bPause_;//是否暂停
	DWORD				dwLastTime_; //上次的时间
};

QxTimerState::QxTimerState(UINT_PTR nID,IQxTimerHandler *pHandler)
	:nIDEvent_(nID),pHandler_(pHandler)
{
	bPause_ = false;
	dwLastTime_ = ::GetTickCount();
}

int QxTimerState::OnTimer(DWORD dwTime)
{
	if(!bPause_){
		DWORD dwPassTime = dwTime - dwLastTime_;
		pHandler_->OnTimer(nIDEvent_,dwPassTime);
		dwLastTime_ = dwTime;
		return 1;
	}
	return 0;
}

void QxTimerState::PauseTimer()
{
	bPause_ = true;
	pHandler_->OnPause();
}

void QxTimerState::ResumeTimer()
{
	bPause_ = false;
	pHandler_->OnResume();
}

void QxTimerState::OnStopped()
{
	pHandler_->OnStopped();
}

///////////////////////////////////////////////////////////////////
//
QxMainWndTimerMgr::QxMainWndTimerMgr(HWND hMainWnd)
	:hMainWnd_(hMainWnd)
{
	uNextID_ = 100;//默认从100开始，避免和已经存在的冲突
}

QxMainWndTimerMgr::~QxMainWndTimerMgr()
{
}

// 设定一个定时器,返回ID，这个ID用于后面的
UINT_PTR QxMainWndTimerMgr::SetTimer(IQxTimerHandler *pTimerHandler,UINT uElapse)
{
	ASSERT(pTimerHandler);
	UINT_PTR nIDEvent = 0, cnt = 0;

	pTimerHandler->OnStarting();

	//
	nIDEvent = ::SetTimer(hMainWnd_,uNextID_,uElapse,NULL);
	while(nIDEvent == 0 && (cnt < 10)){//失败了，多尝试几次
		uNextID_ ++;
		nIDEvent = ::SetTimer(hMainWnd_,uNextID_,uElapse,NULL);
		cnt ++;
	}
	if(cnt >= 10) //失败了
		return 0;

	pTimerHandler->OnStarted();

	//记录
	QxTimerState *pState = new QxTimerState(nIDEvent,pTimerHandler);
	aTimerStateMap_.insert(QxTimerStateMap::value_type(nIDEvent,pState));
	
	return nIDEvent;
}

// 
bool QxMainWndTimerMgr::KillTimer(UINT_PTR nIDEvent)
{
	if(!HasTimer(nIDEvent)){
		return false;
	}

	::KillTimer(hMainWnd_,nIDEvent);
	//从表中删除
	RemoveTimerState(nIDEvent);
	
	return true;
}

// 是否已经存在定时器
bool QxMainWndTimerMgr::HasTimer(UINT_PTR nIDEvent)
{
	QxTimerStateMap::const_iterator ite = aTimerStateMap_.find(nIDEvent);
	return (ite != aTimerStateMap_.end());
}
		
// 暂停定时器
bool QxMainWndTimerMgr::PauseTimer(UINT_PTR nIDEvent)
{
	QxTimerState *pTimerState = GetTimerState(nIDEvent);
	if(pTimerState){
		pTimerState->bPause_ = true;
		return true;
	}
	return false;
}

// 恢复定时器
bool QxMainWndTimerMgr::ResumeTimer(UINT_PTR nIDEvent)
{
	QxTimerState *pTimerState = GetTimerState(nIDEvent);
	if(pTimerState){
		pTimerState->bPause_ = false;
		return true;
	}
	return false;
}

//定时处理函数
int QxMainWndTimerMgr::OnTimer(UINT_PTR nIDEvent,DWORD dwTime)
{
	int rc = 0;
	QxTimerState *pTimerState = GetTimerState(nIDEvent);
	if(pTimerState){
		rc = pTimerState->OnTimer(dwTime);
	}
	return rc;
}

void QxMainWndTimerMgr::RemoveTimerState(UINT_PTR nIDEvent)
{
	QxTimerStateMap::iterator ite = aTimerStateMap_.find(nIDEvent);
	if(ite != aTimerStateMap_.end()){
		QxTimerState *pState = (*ite).second;
		pState->pHandler_->OnStopped();
		delete pState;
		aTimerStateMap_.erase(ite);
	}
}

QxTimerState* QxMainWndTimerMgr::GetTimerState(UINT_PTR nIDEvent)
{
	QxTimerStateMap::iterator ite = aTimerStateMap_.find(nIDEvent);
	if(ite != aTimerStateMap_.end()){
		return (*ite).second;
	}
	return NULL;
}