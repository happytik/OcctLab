//////////////////////////////////////////////////////////////////////////
// 标注命令。线性标注
#ifndef _KCM_ANGLE_DIMENSION_H_
#define _KCM_ANGLE_DIMENSION_H_

#include "kiCommand.h"

class kiInputEntityTool;
class kiInputLength;
class kcPreviewLine;

class kcmAngleDim : public kiCommand
{
public:
	kcmAngleDim(int subCmd);
	virtual ~kcmAngleDim(void);

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
	bool					GetSelectedLine(bool bFirst);
	//
	bool					BuildDimension();

protected:
	kiInputEntityTool		*pPickLineTool_;
	kiInputEntityTool		*pPickLineTool2_;
	kiOptionSet				*pOptionSet_;
	double					dFlyout_;
	double					dArrowLen_;
	double					dFontHeight_;

protected:
	int						nSubCmd_; //子命令 0：2 line 
	int						nState_;
	TopoDS_Edge				aEdge1_, aEdge2_;
};

#endif