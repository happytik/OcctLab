//////////////////////////////////////////////////////////////////////////
//
#ifndef _KCM_SURFACE_PIPE_H_
#define _KCM_SURFACE_PIPE_H_

#include "kiCommand.h"

class kiInputEntityTool;
class kiInputDoubleTool;

class kcmSurfacePipe : public kiCommand
{
public:
	kcmSurfacePipe(void);
	virtual ~kcmSurfacePipe(void);

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
	//
	BOOL					BuildPipe();

protected:
	kiInputEntityTool		*m_pPickPathTool;
	kiInputDoubleTool		*m_pInputRadiusTool;
	double					m_dRadius;
};

// 单轨扫掠
class kcmSurfaceSweep1 : public kiCommand
{
public:
	kcmSurfaceSweep1(void);
	virtual ~kcmSurfaceSweep1(void);

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
	//
	BOOL					BuildSweep();

protected:
	kiOptionSet				m_optionSet;
	kiInputEntityTool		*m_pPickPathTool;
	kiInputEntityTool		*m_pPickSectTool;
	int						m_nTrihedronType;
	kiOptionEnum			*m_pTrihedronOpt;
};

// 多截面线扫掠
class kcmSurfaceSweepN : public kiCommand
{
public:
	kcmSurfaceSweepN(void);
	virtual ~kcmSurfaceSweepN(void);

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
	//
	BOOL					BuildSweep();

protected:
	kiInputEntityTool		*m_pPickPathTool;
	kiInputEntityTool		*m_pPickSectTool;
};

#endif