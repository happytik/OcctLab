//////////////////////////////////////////////////////////////////////////
// 组合曲线功能。
#ifndef _KCM_SEWCURVE_H_
#define _KCM_SEWCURVE_H_

#include "kiCommand.h"

class kiInputEntityTool;

class kcmSewCurve : public kiCommand
{
public:
	kcmSewCurve(void);
	virtual ~kcmSewCurve(void);

	// 执行一次初始化和释放的地方。
	virtual  BOOL			CanFinish();//命令是否可以完成

	//输入工具结束时
	virtual  int			OnInputFinished(kiInputTool *pTool);

protected:
	//具体的初始化和释放代码,每个子命令可以做各自的初始化和销毁操作。
	virtual int				OnInitialize();
	virtual int				OnDestroy();

	// 执行函数
	virtual int				OnExecute();
	virtual int				OnEnd(int nCode);
	virtual int				OnApply();

protected:
	// 创建必要的输入工具
	virtual	BOOL			CreateInputTools();
	virtual BOOL			InitInputToolQueue();

protected:
	bool					SewCurve();
	bool					DoSewingCurve();

protected:
	kiOptionSet				m_optionSet;
	bool					m_bCopy;//是否拷贝对象
	kiInputEntityTool		*m_pInputEntity;
};

#endif