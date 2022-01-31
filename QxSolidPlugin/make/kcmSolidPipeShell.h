//////////////////////////////////////////////////////////////////////////
//
#ifndef _KCM_SOLID_PIPE_SHELL_H_
#define _KCM_SOLID_PIPE_SHELL_H_

#include "kiCommand.h"
#include "kiOptionSet.h"

class kiInputEntityTool;
class kiInputDoubleTool;

class kcmSolidPipeShell : public kiCommand
{
public:
	kcmSolidPipeShell(void);
	virtual ~kcmSolidPipeShell(void);

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
	BOOL					BuildPipe();

protected:
	kiOptionSet				m_optionSet;
	kiInputEntityTool		*m_pInputSpline;
	kiInputDoubleTool		*m_pInputRadius;
	double					m_dRadius;

	kiOptionEnum			*m_pModelOptItem;
	kiOptionEnum			*m_pTransitionOptItem;
	bool					m_bBuildSolid;//是否创建实体
	bool					m_bContact;
	bool					m_bCorrect;
	int						m_nModel;
	int						m_nTranstion;
};

class kcmSolidSweepShell : public kiCommand
{
public:
	kcmSolidSweepShell(void);
	virtual ~kcmSolidSweepShell(void);

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
	BOOL					BuildPipe();

protected:
	kiOptionSet				m_optionSet;
	kiInputEntityTool		*m_pInputSpline;
	kiInputEntityTool		*m_pInputProfile;

	kiOptionEnum			*m_pModelOptItem;
	kiOptionEnum			*m_pTransitionOptItem;
	bool					m_bBuildSolid;//是否创建实体
	bool					m_bContact;
	bool					m_bCorrect;
	int						m_nModel;
	int						m_nTranstion;
};


#endif