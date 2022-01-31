//////////////////////////////////////////////////////////////////////////
//
#ifndef _KCM_SOLID_DRAFT_H_
#define _KCM_SOLID_DRAFT_H_

#include "kiCommand.h"

class kiInputEntityTool;
class kiInputVector;
class kiInputDoubleTool;

class kcmSolidDraft : public kiCommand
{
public:
	kcmSolidDraft(void);
	virtual ~kcmSolidDraft(void);

	// 表明命令可以完成,主要和Apply函数结合使用.
	virtual  BOOL			CanFinish();//命令是否可以完成

	// 当输入工具获取结果,结束时,会回调对应命令的该函数.
	// 用于继续执行命令或切换输入工具.也可能返回KSTA_DONE,表明命令执行结束.
	//
	virtual  int			OnInputFinished(kiInputTool *pTool);

protected:
	virtual int				OnExecute();
	virtual int				OnEnd(int nCode);
	virtual int				OnApply();

protected:
	// 创建必要的输入工具
	virtual	BOOL			CreateInputTools();
	virtual BOOL			DestroyInputTools();
	virtual BOOL			InitInputToolQueue();

protected:
	//
	BOOL					DoDraft();

protected:
	kiOptionSet				m_optionSet;
	kiInputEntityTool		*m_pPickEntityTool;
	kiInputVector			*m_pInputVectorTool;
	kiInputDoubleTool		*m_pInputDoubleTool;
	kiInputEntityTool		*m_pPickStopEntityTool;
	double					m_dAngle;

	kiOptionEnum			*m_pDraftMode;
	int						m_nDraftMode;
	kiOptionEnum			*m_pTransitionMode;
	int						m_nTransitionMode;
	bool					m_bInternal;

protected:
	bool					m_bReady;
};

#endif