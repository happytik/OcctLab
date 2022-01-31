//////////////////////////////////////////////////////////////////////////
//
#ifndef _KCM_SURF_POLYGON_H_
#define _KCM_SURF_POLYGON_H_

#include <vector>
#include "kiCommand.h"
#include "kiOptionSet.h"

class kiInputPoint;
class kcPreviewPolyline;
class kcPreviewLine;

class kcmSurfPolygon : public kiCommand
{
public:
	kcmSurfPolygon(void);
	virtual ~kcmSurfPolygon(void);

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

	virtual int				OnMouseMove(kuiMouseInput& mouseInput);

protected:
	BOOL					MakePolyline();

protected:
	std::vector<kPoint3>	m_pointArray;
	kiInputPoint			*m_pInputPoint;
	kcPreviewPolyline		*m_pPreviewPolyline;
	kcPreviewLine			*m_pPreviewLine;
	kcPreviewLine			*m_pPreviewLine2;
	kPoint3					m_aPoint;
};

#endif