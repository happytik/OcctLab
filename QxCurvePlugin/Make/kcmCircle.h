//////////////////////////////////////////////////////////////////////////
//
#ifndef _KCM_CIRCLE_H_
#define _KCM_CIRCLE_H_

#include "kiCommand.h"
#include "kiOptionSet.h"

class kiInputPoint;
class kiInputLength;
class kcPreviewCircle;
class kcPreviewLine;
class kiOptionSet;
class kcPreviewText;

class kcmCircle : public kiCommand
{
public:
	kcmCircle(void);
	virtual ~kcmCircle(void);

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
	virtual int				OnLButtonUp(kuiMouseInput& mouseInput);

protected:
	//
	bool					MakeCircleEntity();

protected:
	int						m_nState;//状态
	kiInputPoint			*m_pInputCenter;
	kiInputLength			*m_pInputRadius;
	kPoint3					m_aCenter;
	double					m_aCircPnt[3];
	double					m_dRadius;
	bool					m_bPreviewInit;//预览对象是否初始化.
	kcPreviewCircle			*m_pPreviewCircle;
};

#endif