//////////////////////////////////////////////////////////////////////////
//
#ifndef _KCM_TRSF_MOVE_H_
#define _KCM_TRSF_MOVE_H_

#include "kVector.h"
#include "kiCommand.h"
#include "kiOptionSet.h"

class kiInputDirTool;
class kiInputDoubleTool;
class kiInputVector;
class kiInputEntityTool;
class kcDispFrmShape;

class kcmTrsfMove : public kiCommand
{
public:
	kcmTrsfMove(void);
	virtual ~kcmTrsfMove(void);

	// 表明命令可以完成,主要和Apply函数结合使用.
	virtual  BOOL			CanFinish();//命令是否可以完成

	// 当输入工具获取结果,结束时,会回调对应命令的该函数.
	// 用于继续执行命令或切换输入工具.也可能返回KSTA_DONE,表明命令执行结束.
	//
	virtual  int			OnInputFinished(kiInputTool *pTool);

protected:
	virtual int				OnExecute();
	virtual int				OnEnd(int nCode);

	virtual int				OnMouseMove(kuiMouseInput& mouseInput);

	//
protected:
	// 创建必要的输入工具,在Initialize中调用，每个命令仅仅调用一次。
	virtual	BOOL			CreateInputTools();
	// 销毁创建的输入工具.每个命令仅调用一次.
	virtual BOOL			DestroyInputTools();
	// 在begin中调用，主要是初始化工具队列。
	virtual BOOL			InitInputToolQueue();

protected:
	//
	BOOL					DoMove();

protected:
	int						m_nState;
	kVector3				m_aDir;
	double					m_dDist;
	kiOptionSet				m_optionSet;
	bool					m_bCopy;
	kiInputEntityTool		*m_pSelEntity;
	kiInputDirTool			*m_pInputDir;
	kiInputDoubleTool		*m_pInputDistTool;
	//kcDispFrmShape			*m_pFrmShape;
};

#endif