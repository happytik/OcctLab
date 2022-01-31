// 用于定义系统的接口
#ifndef _QX_SYS_INTERFACE_H_
#define _QX_SYS_INTERFACE_H_

// 一个定时器处理对象接口
class IQxTimerHandler{
public:
	virtual ~IQxTimerHandler() {}

	// 对应的处理函数
	virtual void			OnStarting() {} //定时器正在启动，还没启动
	virtual void			OnStarted() {} //定时器已经启动
	virtual void			OnStopped() {}  //定时器已终止
	virtual void			OnPause() {}
	virtual void			OnResume() {}

	//消息相应函数,dwPassTime:是距离上次定时器过去的时间，单位ms
	virtual int			OnTimer(UINT_PTR nIDEvent,DWORD dwPassTime) = 0;
};


// 命令定时器管理类
class IQxTimerManager{
public:
	virtual ~IQxTimerManager() {}

	// 设定一个定时器,返回ID，这个ID用于后面的
	virtual UINT_PTR		SetTimer(IQxTimerHandler *pTimerHandler,UINT uElapse) = 0;
	// 
	virtual bool			KillTimer(UINT_PTR nIDEvent) = 0;

	// 是否已经存在定时器
	virtual bool			HasTimer(UINT_PTR nIDEvent) = 0;
		
	// 暂停定时器
	virtual bool			PauseTimer(UINT_PTR nIDEvent) = 0;
	// 恢复定时器
	virtual bool			ResumeTimer(UINT_PTR nIDEvent) = 0;

	//定时处理函数
	virtual int			OnTimer(UINT_PTR nIDEvent,DWORD dwTime) = 0;
};

#endif