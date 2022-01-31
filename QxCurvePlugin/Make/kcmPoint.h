//////////////////////////////////////////////////////////////////////////
//
#ifndef _KCM_POINT_H_
#define _KCM_POINT_H_

#include "kiCommand.h"

class kiInputPoint;
class kiInputEntityTool;

class kcmPoint : public kiCommand
{
public:
	kcmPoint(void);
	virtual ~kcmPoint(void);

	// 表明命令可以完成,主要和Apply函数结合使用.
	virtual  BOOL			CanFinish();//命令是否可以完成

	// 当输入工具获取结果,结束时,会回调对应命令的该函数.
	// 用于继续执行命令或切换输入工具.也可能返回KSTA_DONE,表明命令执行结束.
	//
	virtual  int			OnInputFinished(kiInputTool *pTool);

protected:
	virtual int				OnExecute();

	//
protected:
	// 创建必要的输入工具,在Initialize中调用，每个命令仅仅调用一次。
	virtual	BOOL			CreateInputTools();
	// 销毁创建的输入工具.每个命令仅调用一次.
	virtual BOOL			DestroyInputTools();
	// 在begin中调用，主要是初始化工具队列。
	virtual BOOL			InitInputToolQueue();

protected:
	kPoint3					m_aPoint;
	kiInputPoint			*m_pInputPoint;
};

//////////////////////////////////////////////////////////////////////////
// 曲线交点
class kcmCurveInterPoint : public kiCommand
{
public:
	kcmCurveInterPoint();
	virtual ~kcmCurveInterPoint();

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
	BOOL					CalcInterPoint();

protected:
	kiInputEntityTool		*m_pInputCurve1;
	kiInputEntityTool		*m_pInputCurve2;
};

#endif