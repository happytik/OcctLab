//////////////////////////////////////////////////////////////////////////
//
#ifndef _KCM_CURVE_BREAK_H_
#define _KCM_CURVE_BREAK_H_

#include "kiCommand.h"

class kiInputEntityTool;
class kiInputPointOnCurve;

//////////////////////////////////////////////////////////////////////////
// 点打断
class kcmCurveBreakByPoint : public kiCommand
{
public:
	kcmCurveBreakByPoint(void);
	virtual ~kcmCurveBreakByPoint(void);

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
	BOOL					DoBreak();

protected:
	kiInputEntityTool		*m_pInputCurve;
	kiInputPointOnCurve		*m_pPickPointOnCurve;
};

//////////////////////////////////////////////////////////////////////////
// 曲线打断
class kcmCurveBreakByCurve : public kiCommand{
public:
	kcmCurveBreakByCurve();
	virtual ~kcmCurveBreakByCurve();

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
	BOOL					DoBreak();

protected:
	kiInputEntityTool		*m_pInputCurve;
	kiInputEntityTool		*m_pInputCurve2;
};

#endif