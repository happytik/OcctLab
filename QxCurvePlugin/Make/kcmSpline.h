///////////////////////////////////////////////////////////////////////////
// 
#ifndef _KCM_SPLINE_H_
#define _KCM_SPLINE_H_

#include <vector>
#include "kPoint.h"

#include "kiCommand.h"
#include "kiOptionSet.h"

class kiInputEntityTool;
class kiInputPoint;
class kiOptionSet;
class kcPreviewCurve;

class kcmSpline : public kiCommand
{
public:
	kcmSpline(void);
	virtual ~kcmSpline(void);

	virtual  BOOL				CanFinish();//命令是否可以完成

	//输入工具结束时
	virtual  int				OnInputFinished(kiInputTool *pTool);

protected:
	// 执行函数
	virtual int				OnExecute();
	virtual int				OnEnd(int nCode);
	virtual int				OnApply();

protected:
	// 创建必要的输入工具
	virtual	BOOL				CreateInputTools();
	virtual BOOL				DestroyInputTools();
	virtual BOOL				InitInputToolQueue();
	virtual int				OnMouseMove(kuiMouseInput& mouseInput);
	virtual int				OnLButtonUp(kuiMouseInput& mouseInput);

protected:
	kiOptionSet				m_optionSet;
	bool						m_bPeriodic;//是否周期闭合
	kiInputPoint				*m_pInputPoint;
	std::vector<kPoint3>		m_aPoint;//输入点列
	kcPreviewCurve			*m_pPreviewCurve;
	kPoint3					m_dPoint;
};

class kcmSplineUnperiodize : public kiCommand{
public:
	kcmSplineUnperiodize();
	virtual ~kcmSplineUnperiodize();

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
	BOOL					DoUnperiodize();

protected:
	kiInputEntityTool		*m_pInputCurve;
};

#endif