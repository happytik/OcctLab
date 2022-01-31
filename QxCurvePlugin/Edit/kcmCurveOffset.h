//////////////////////////////////////////////////////////////////////////
//
#ifndef _KCM_CURVE_OFFSET_H_
#define _KCM_CURVE_OFFSET_H_

#include "kiCommand.h"

class kiInputEntityTool;
class kiInputDoubleTool;

class kcmCurveOffset : public kiCommand
{
public:
	kcmCurveOffset(void);
	~kcmCurveOffset(void);

	// 执行一次初始化和释放的地方。
	virtual  int			Destroy();

	//执行和结束接口
	virtual	 int			Execute();
	virtual  int			End(int nCode);
	virtual  int			Apply();//实际应用
	virtual  BOOL			CanFinish();//命令是否可以完成

	//输入工具结束时
	virtual  int			OnInputFinished(kiInputTool *pTool);

protected:
	// 创建必要的输入工具
	virtual	BOOL			CreateInputTools();
	virtual BOOL			InitInputToolQueue();

protected:
	//
	bool					MakeOffset();

protected:
	kiInputEntityTool		*m_pInputEntityTool;
	kiInputDoubleTool		*m_pInputOffset;
	double					m_dOffset;
};

#endif