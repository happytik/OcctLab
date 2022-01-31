//////////////////////////////////////////////////////////////////////////
//
#ifndef _KCM_RECT_H_
#define _KCM_RECT_H_

#include "kiCommand.h"
#include "kiOptionSet.h"

class kiInputPoint;
class kcPreviewRect;
class kiOptionSet;
class kcPreviewText;

class kcmRect : public kiCommand
{
public:
	kcmRect(void);
	virtual ~kcmRect(void);

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
	//创建曲线对象
	BOOL					MakeRectEntity();

protected:
	kiOptionSet				m_optionSet;

protected:
	kiInputPoint			*_pInputPoint1;
	kiInputPoint			*_pInputPoint2;
	kiInputPoint			*_pInputPoint3;
	kPoint3					_aPoint1,
							_aPoint2,
							_aPoint3;

protected:
	int						m_nState;//
	kcPreviewRect			*_pPreviewRect;
	kcPreviewText			*_pPreviewText;
	kcPreviewText			*_pPreviewText2;
};

#endif