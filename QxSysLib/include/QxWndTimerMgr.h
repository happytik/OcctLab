//
#ifndef _QX_WND_TIMER_MGR_H_
#define _QX_WND_TIMER_MGR_H_

#include <map>
#include "QxLibDef.h"
#include "QxSysInterface.h"

class QxTimerState;
typedef std::map<UINT_PTR,QxTimerState *> QxTimerStateMap;

///////////////////////////////////////////////////////////
// 基于MainFrame的定时器

class QXLIB_API QxMainWndTimerMgr : public IQxTimerManager{
public:
	QxMainWndTimerMgr(HWND hMainWnd);
	~QxMainWndTimerMgr();

	// 设定一个定时器,返回ID，这个ID用于后面的
	virtual UINT_PTR		SetTimer(IQxTimerHandler *pTimerHandler,UINT uElapse);
	// 
	virtual bool			KillTimer(UINT_PTR nIDEvent);

	// 是否已经存在定时器
	virtual bool			HasTimer(UINT_PTR nIDEvent);
		
	// 暂停定时器
	virtual bool			PauseTimer(UINT_PTR nIDEvent);
	// 恢复定时器
	virtual bool			ResumeTimer(UINT_PTR nIDEvent);

	//定时处理函数
	virtual int			OnTimer(UINT_PTR nIDEvent,DWORD dwTime);

protected:
	void					RemoveTimerState(UINT_PTR nIDEvent);
	QxTimerState*			GetTimerState(UINT_PTR nIDEvent);

protected:
	HWND					hMainWnd_; //定时器依附的窗口
	UINT_PTR				uNextID_;  //下一个可用的ID值
	QxTimerStateMap		aTimerStateMap_;
};

#endif