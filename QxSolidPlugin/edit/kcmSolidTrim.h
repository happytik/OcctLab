/////////////////////////////////////
// 实体的裁剪命令
#ifndef _KCM_SOLID_TRIM_H_
#define _KCM_SOLID_TRIM_H_

#include "kiCommand.h"
#include "kiOptionSet.h"
#include "kcLayer.h"

class kiInputEntityTool;
class kcDispSolidArrow;

class kcmSolidTrimByShell : public kiCommand
{
public:
	kcmSolidTrimByShell(void);
	virtual ~kcmSolidTrimByShell(void);

	// 表明命令可以完成,主要和Apply函数结合使用.
	virtual  BOOL				CanFinish();//命令是否可以完成

	// 当输入工具获取结果,结束时,会回调对应命令的该函数.
	// 用于继续执行命令或切换输入工具.也可能返回KSTA_DONE,表明命令执行结束.
	//
	virtual  int				OnInputFinished(kiInputTool *pTool);
	// 当输入项改变时，会调用该回调函数
	virtual  int				OnInputChanged(kiInputTool *pTool);

	virtual void				OnOptionChanged(kiOptionItem *pOptionItem);

protected:
	virtual int				OnExecute();
	virtual int				OnEnd(int nCode);

protected:
	// 创建必要的输入工具
	virtual	BOOL				CreateInputTools();
	virtual BOOL				DestroyInputTools();
	virtual BOOL				InitInputToolQueue();

protected:
	void						ShowDirArrow();
	BOOL						DoTrim();

protected:
	kiInputEntityTool			*m_pInputSolidTool;
	kiInputEntityTool			*m_pInputShellTool;

	kiOptionSet				_optionSet;
	bool						_bRevDir;
	bool						_bParallel;//是否使用并发执行

	kcDispSolidArrow			*_pDirArrow;
};

//分割裁剪
class kcmSolidTrimSplit : public kiCommand
{
public:
	kcmSolidTrimSplit(void);
	virtual ~kcmSolidTrimSplit(void);

	// 表明命令可以完成,主要和Apply函数结合使用.
	virtual  BOOL				CanFinish();//命令是否可以完成

	// 当输入工具获取结果,结束时,会回调对应命令的该函数.
	// 用于继续执行命令或切换输入工具.也可能返回KSTA_DONE,表明命令执行结束.
	//
	virtual  int				OnInputFinished(kiInputTool *pTool);

protected:
	virtual int				OnExecute();
	virtual int				OnEnd(int nCode);

protected:
	// 创建必要的输入工具
	virtual	BOOL				CreateInputTools();
	virtual BOOL				DestroyInputTools();
	virtual BOOL				InitInputToolQueue();

protected:
	BOOL						DoSplit();
	BOOL						GetInputEntList(kiInputEntityTool *pTool,kcEntityList &entList,
										  TopTools_ListOfShape& aShapeList,TopTools_ListOfShape& aCpyShapeList);

protected:
	kiInputEntityTool			*m_pInputObjTool1;
	kiInputEntityTool			*m_pInputObjTool2;

	kiOptionSet				_optionSet;
	bool						_bParallel;//是否使用并发执行
};

#endif