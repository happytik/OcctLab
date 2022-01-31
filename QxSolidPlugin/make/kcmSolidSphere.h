//////////////////////////////////////////////////////////////////////////
//
#ifndef _KCM_SOLID_SPHERE_H_
#define _KCM_SOLID_SPHERE_H_

#include "kiCommand.h"
#include "kiOptionSet.h"

class kiInputPoint;
class kiOptionSet;
class kcPreviewCircle;
class kiInputDoubleTool;

class kcmSolidSphere : public kiCommand
{
public:
	kcmSolidSphere(void);
	virtual ~kcmSolidSphere(void);

	// 执行一次初始化和释放的地方。
	virtual  int			Destroy();

	//执行和结束接口
	virtual	 int			Execute();
	virtual  int			End(int nCode);
	virtual  BOOL			CanFinish();//命令是否可以完成

	//输入工具结束时
	virtual  int			OnInputFinished(kiInputTool *pTool);

protected:
	// 创建必要的输入工具
	virtual	BOOL			CreateInputTools();
	virtual BOOL			InitInputToolQueue();
	virtual int				OnMouseMove(kuiMouseInput& mouseInput);
	virtual int				OnLButtonUp(kuiMouseInput& mouseInput);

protected:
	//
	BOOL					MakeSphere();

protected:
	int						m_nState;
	kPoint3					m_dCenter;//圆心
	double					m_dRadius;//半径

protected:
	kiInputPoint			*m_pInputPoint;//输入点
	kiInputDoubleTool		*m_pInputRadius;//输入半径
	kcPreviewCircle			*m_pPreviewCircle;
};

#endif