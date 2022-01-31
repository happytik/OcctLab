//////////////////////////////////////////////////////////////////////////
//
#ifndef _KCM_SOLIDBOX_H_
#define _KCM_SOLIDBOX_H_

#include "kiCommand.h"
#include "kiOptionSet.h"

class kiInputPoint;
class kiInputDoubleTool;
class kiOptionSet;
class kcPreviewRect;
class kcPreviewBox;

class kcmSolidBox : public kiCommand
{
public:
	kcmSolidBox(void);
	virtual ~kcmSolidBox(void);

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
	//
	bool					BuildBox();

protected:
	int						m_nState;
	
protected:
	kPoint3					m_aPnt1;
	kPoint3					m_aPnt2;
	double					m_dHeight;
	kiInputPoint			*m_pInputPoint1;
	kiInputPoint			*m_pInputPoint2;
	kiInputDoubleTool		*m_pInputHeight;

protected:
	kcPreviewRect			*m_pPreviewRect;
	kcPreviewBox			*m_pPreviewBox;
};

#endif