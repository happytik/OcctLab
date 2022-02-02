//////////////////////////////////////////////////////////////////////////
// 标注命令。线性标注
#ifndef _KCM_RADIUS_DIMENSION_H_
#define _KCM_RADIUS_DIMENSION_H_

#include "kiCommand.h"

class kiInputEntityTool;
class kiInputLength;
class kcPreviewLine;

class kcmRadiusDim : public kiCommand
{
public:
	kcmRadiusDim(void);
	virtual ~kcmRadiusDim(void);

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
	//
	BOOL					BuildDimension();

protected:
	kiInputEntityTool		*pPickCircleTool_;
	kiOptionSet				*pOptionSet_;
	double					dFlyout_;
	double					dArrowLen_;
	double					dFontHeight_;
};

#endif