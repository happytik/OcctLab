//////////////////////////////////////////////////////////////////////////
//
#ifndef _KCM_REVOL_SURFACE_H_
#define _KCM_REVOL_SURFACE_H_

#include "kiCommand.h"
#include "kiOptionSet.h"

class kiInputEntityTool;
class kiInputVector;

class kcmRevolSurface : public kiCommand
{
public:
	kcmRevolSurface(void);
	virtual ~kcmRevolSurface(void);

	// 表明命令可以完成,主要和Apply函数结合使用.
	virtual  BOOL			CanFinish();//命令是否可以完成

	// 当输入工具获取结果,结束时,会回调对应命令的该函数.
	// 用于继续执行命令或切换输入工具.也可能返回KSTA_DONE,表明命令执行结束.
	//
	virtual  int			OnInputFinished(kiInputTool *pTool);

	//
protected:
	// 创建必要的输入工具,在Initialize中调用，每个命令仅仅调用一次。
	virtual	BOOL			CreateInputTools();
	virtual BOOL			DestroyInputTools();
	// 在begin中调用，主要是初始化工具队列。
	virtual BOOL			InitInputToolQueue();

protected:
	//
	BOOL					MakeRevolSurface();

protected:
	int						m_nState;
	kiInputEntityTool		*m_pInputEntity;
	kiInputVector			*m_pInputVec;

protected:
	kiOptionSet				m_optionSet;
	double					m_dAngle;//角度
};

#endif