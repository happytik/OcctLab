//////////////////////////////////////////////////////////////////////////
//
#ifndef _KCM_SOLID_CONE_H_
#define _KCM_SOLID_CONE_H_

#include "kiCommand.h"
#include "kiOptionSet.h"

class kiInputPoint;
class kiInputLength;
class kcPreviewCircle;
class kcPreviewLine;
class kiInputVector;


class kcmSolidCone : public kiCommand
{
public:
	kcmSolidCone(void);
	virtual ~kcmSolidCone(void);

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
	BOOL					DoBuildCone();

protected:
	int						m_nState;//状态
	kiOptionSet				m_optionSet;
	kiOptionSet				m_optionSet2;
	bool					m_bTCone;//是否平顶锥体
	kiInputPoint			*m_pInputCenter;
	kPoint3					m_aCenter;
	kiInputLength			*m_pInputRadius;
	kiInputLength			*m_pInputRadius2;
	double					m_dRadius;
	kiInputLength			*m_pInputHeight;
	double					m_dAngle;

	kcPreviewCircle			*m_pPreviewCircle;
	kcPreviewCircle			*m_pPreviewCircle2;
	kcPreviewLine			*m_pPreviewLine;

protected:
	kPoint3					m_aXPoint;//X轴起始点的确定
};

#endif