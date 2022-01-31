//////////////////////////////////////////////////////////////////////////
//
#ifndef _KCM_PLANE_H_
#define _KCM_PLANE_H_

#include "kiCommand.h"
#include "kiOptionSet.h"

class kiInputPoint;
class kcPreviewRect;

class kcmPlane : public kiCommand
{
public:
	kcmPlane(void);
	virtual ~kcmPlane(void);

	// 执行一次初始化和释放的地方。
	virtual  BOOL			CanFinish();//命令是否可以完成

	//输入工具结束时
	virtual  int			OnInputFinished(kiInputTool *pTool);

protected:
	//具体的初始化和释放代码,每个子命令可以做各自的初始化和销毁操作。
	virtual int				OnInitialize();
	virtual int				OnDestroy();

	// 执行函数
	virtual int				OnExecute();
	virtual int				OnEnd(int nCode);
	virtual int				OnApply();

protected:
	// 创建必要的输入工具
	virtual	BOOL			CreateInputTools();
	virtual BOOL			InitInputToolQueue();
	virtual int				OnMouseMove(kuiMouseInput& mouseInput);
	virtual int				OnLButtonUp(kuiMouseInput& mouseInput);

protected:
	int						m_nState;
	kPoint3					m_aPnt1;
	kPoint3					m_aPnt2;
	kiInputPoint			*m_pInputPoint;
	kiInputPoint			*m_pInputPoint2;
	kcPreviewRect			*m_pPreviewRect;
};

#endif