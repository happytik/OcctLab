//////////////////////////////////////////////////////////////////////////
//
#ifndef _KCM_INTER_CURVE_H_
#define _KCM_INTER_CURVE_H_

#include "kiCommand.h"

class kiInputEntityTool;

class kcmInterCurve : public kiCommand
{
public:
	kcmInterCurve(void);
	virtual ~kcmInterCurve(void);

	// 表明命令可以完成,主要和Apply函数结合使用.
	virtual  BOOL			CanFinish();//命令是否可以完成

	// 当输入工具获取结果,结束时,会回调对应命令的该函数.
	// 用于继续执行命令或切换输入工具.也可能返回KSTA_DONE,表明命令执行结束.
	//
	virtual  int			OnInputFinished(kiInputTool *pTool);

protected:
	virtual int				OnExecute();
	virtual int				OnEnd(int nCode);

protected:
	// 创建必要的输入工具
	virtual	BOOL			CreateInputTools();
	virtual BOOL			DestroyInputTools();
	virtual BOOL			InitInputToolQueue();

protected:
	BOOL					DoIntersect();

protected:
	kiInputEntityTool		*m_pInputEntity1;
	kiInputEntityTool		*m_pInputEntity2;
};

#endif