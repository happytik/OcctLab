//////////////////////////////////////////////////////////////////////////
//
#ifndef _KCM_ANIMATION_DEMO_H_
#define _KCM_ANIMATION_DEMO_H_

#include "kiCommand.h"

class kiInputEntityTool;
class QxAnimationShape;

class kcmAnimationDemo : public kiCommand
{
public:
	kcmAnimationDemo(void);
	virtual ~kcmAnimationDemo(void);

	// 表明命令可以完成,主要和Apply函数结合使用.
	virtual  BOOL			CanFinish();//命令是否可以完成

	// 当输入工具获取结果,结束时,会回调对应命令的该函数.
	// 用于继续执行命令或切换输入工具.也可能返回KSTA_DONE,表明命令执行结束.
	//
	virtual  int			OnInputFinished(kiInputTool *pTool);

protected:
	virtual int				OnExecute();
	virtual int				OnEnd(int nCode);

	// 定时器处理函数
	virtual int				OnTimer(int nId,DWORD dwTime);

protected:
	// 创建必要的输入工具
	virtual	BOOL			CreateInputTools();
	virtual BOOL			DestroyInputTools();
	virtual BOOL			InitInputToolQueue();

protected:
	QxAnimationShape		*m_pAnimationShape;
	kiInputEntityTool		*m_pInputEntityTool;
	UINT					m_nTimer;
	double					m_dAngle;
};

#endif