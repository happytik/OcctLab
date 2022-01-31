//////////////////////////////////////////////////////////////////////////
//
#ifndef _KCM_ARC_H_
#define _KCM_ARC_H_

#include "kiCommand.h"
#include "kiOptionSet.h"

class kiInputPoint;
class kcPreviewCircle;
class kcPreviewLine;
class kiOptionSet;
class kcPreviewText;
class kcPreviewArc;
class kiInputAngle;
class kiInputLength;
class kvCoordSystem;

class kcmArc : public kiCommand
{
public:
	kcmArc(void);
	virtual ~kcmArc(void);

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
	BOOL					BuildArcEntity();

protected:
	int						m_nState;//状态
	kiInputPoint			*m_pInputCenter;
	kiInputLength			*m_pInputRadius;
	kiInputPoint			*m_pInputStartPnt;
	kiInputAngle			*m_pInputAngle;
	kPoint3					m_aCenter;
	kPoint3					m_aCircPnt;
	kPoint3					m_aStartPnt,m_aEndPnt;
	double					m_dRadius;
	double					m_dAngle; //角度值,非弧度值
	kvCoordSystem			*m_pLocalCS;
	bool					m_bPreviewInit;//预览对象是否初始化.
	kcPreviewLine			*m_pPreviewLine;
	kcPreviewCircle			*m_pPreviewCircle;
	kcPreviewText			*m_pPreviewText;
	kcPreviewArc			*m_pPreviewArc;
	gp_Ax2					m_arcAxis;
};

#endif