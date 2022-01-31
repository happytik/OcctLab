//////////////////////////////////////////////////////////////////////////
//
#ifndef _KCM_LINE_H_
#define _KCM_LINE_H_

#include <vector>
#include "kiCommand.h"
#include "kiOptionSet.h"

class kiInputPointTool;
class kcPreviewLine;
class kiOptionSet;

class kcmLine : public kiCommand
{
public:
	kcmLine(void);
	virtual ~kcmLine(void);

	// 表明命令可以完成,主要和Apply函数结合使用.
	virtual  BOOL				CanFinish();//命令是否可以完成

	// 当输入工具获取结果,结束时,会回调对应命令的该函数.
	// 用于继续执行命令或切换输入工具.也可能返回KSTA_DONE,表明命令执行结束.
	//
	virtual  int				OnInputFinished(kiInputTool *pTool);

	// 当输入项改变时，会调用该回调函数
	virtual  int				OnInputChanged(kiInputTool *pTool);

protected:
	virtual int				OnExecute();
	virtual int				OnEnd(int nCode);

protected:
	// 创建必要的输入工具
	virtual	BOOL				CreateInputTools();
	virtual BOOL				DestroyInputTools();
	virtual BOOL				InitInputToolQueue();

	virtual int				OnMouseMove(kuiMouseInput& mouseInput);
	virtual int				OnLButtonUp(kuiMouseInput& mouseInput);
	virtual int				OnRButtonUp(kuiMouseInput& mouseInput);

protected:
	//创建曲线对象
	BOOL						MakeLineEntities();

protected:
	kiOptionSet				m_optionSet;
	bool						_bMultiLine;//多条直线
	bool						_bMakeWire;//多线组成wire
	double					m_dLen;//长度，double更合适。

protected:
	kiInputPointTool			*_pInputStartPoint;
	kiInputPointTool			*_pInputEndPoint;
    kPoint3					_startPoint,
							_endPoint;
	std::vector<kPoint3>		_pointList;

protected:
	int						m_nState;//
	kcPreviewLine				*_pPreviewLine;
	std::vector<kcPreviewLine*>	_prevLineList;
};

#endif
