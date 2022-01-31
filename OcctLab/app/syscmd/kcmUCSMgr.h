//////////////////////////////////////////////////////////////////////////
//
#ifndef _KCM_UCSMGR_H_
#define _KCM_UCSMGR_H_

#include "kiCommand.h"

class kiInputPoint;
class kcPreviewLine;

// 三点创建坐标系
class kcmCreateUCSByThreePoint : public kiCommand{
public:
	kcmCreateUCSByThreePoint();
	virtual ~kcmCreateUCSByThreePoint();

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

	virtual int				OnMouseMove(kuiMouseInput& mouseInput);

protected:
	BOOL					DoCreateUCS();

protected:
	int						m_nState;
	kiInputPoint			*m_pInputPointOrg;
	kiInputPoint			*m_pInputPointX;
	kiInputPoint			*m_pInputPointY;
	kPoint3					m_pointOrg;
	kPoint3					m_pointX;
	kPoint3					m_pointY;

	kcPreviewLine			*m_pPreviewXLine;
	kcPreviewLine			*m_pPreviewYLine;
};

// 平移当前坐标系
class kcmCreateUCSByMove : public kiCommand{
public:
	kcmCreateUCSByMove();
	virtual	~kcmCreateUCSByMove();
};

#endif