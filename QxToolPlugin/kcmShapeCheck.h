//////////////////////////////////////////////////////////////////////////
//
#ifndef _KCM_SHAPE_CHECK_H_
#define _KCM_SHAPE_CHECK_H_

#include "kiCommand.h"

class kiInputEntityTool;

class kcmShapeCheck : public kiCommand
{
public:
	kcmShapeCheck(void);
	virtual ~kcmShapeCheck(void);

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

protected:
	virtual BOOL			DoCheck() = 0;

protected:
	kiInputEntityTool		*m_pInputEntityTool;
};

// 检查对象的有效性
class kcmShapeCheckValid : public kcmShapeCheck{
public:
	kcmShapeCheckValid();
	virtual ~kcmShapeCheckValid();

protected:
	// 创建必要的输入工具
	virtual	BOOL			CreateInputTools();

protected:
	virtual BOOL			DoCheck();
};

// 检查对象的拓扑有效性
class kcmShapeCheckTopoValid : public kcmShapeCheck{
public:
	kcmShapeCheckTopoValid();
	virtual ~kcmShapeCheckTopoValid();

protected:
	// 创建必要的输入工具
	virtual	BOOL			CreateInputTools();

protected:
	virtual BOOL			DoCheck();
};

// 检查环或壳的闭合性
class kcmShapeCheckClose : public kcmShapeCheck{
public:
	kcmShapeCheckClose();
	virtual ~kcmShapeCheckClose();

protected:
	// 创建必要的输入工具
	virtual	BOOL			CreateInputTools();

protected:
	virtual BOOL			DoCheck();
};

// 检查环的自交
class kcmShapeCheckSelfIntersect : public kcmShapeCheck{
public:
	kcmShapeCheckSelfIntersect();
	virtual ~kcmShapeCheckSelfIntersect();

protected:
	// 创建必要的输入工具
	virtual	BOOL			CreateInputTools();

protected:
	virtual BOOL			DoCheck();
};

#endif