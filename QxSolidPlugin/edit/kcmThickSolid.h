//////////////////////////////////////////////////////////////////////////
//
#ifndef _KCM_THICK_SOLID_H_
#define _KCM_THICK_SOLID_H_

#include "kiCommand.h"
#include "kiOptionSet.h"

class kiInputEntityTool;
class kiInputDoubleTool;

//抽壳命令
class kcmSheetSolid : public kiCommand
{
public:
	kcmSheetSolid(void);
	virtual ~kcmSheetSolid(void);

	// 表明命令可以完成,主要和Apply函数结合使用.
	virtual  BOOL			CanFinish();//命令是否可以完成

	// 当输入工具获取结果,结束时,会回调对应命令的该函数.
	// 用于继续执行命令或切换输入工具.也可能返回KSTA_DONE,表明命令执行结束.
	//
	virtual  int			OnInputFinished(kiInputTool *pTool);

protected:
	virtual int			OnExecute();
	virtual int			OnEnd(int nCode);

protected:
	// 创建必要的输入工具
	virtual	BOOL			CreateInputTools();
	virtual BOOL			DestroyInputTools();
	virtual BOOL			InitInputToolQueue();

protected:
	BOOL					BuildSheetSolid();

protected:
	kiOptionSet			m_optionSet;
	bool					m_bOutSide;//向外还是向内
	kiInputEntityTool		*m_pInputEntity;
	kiInputEntityTool		*m_pInputFace;
	kiInputDoubleTool		*m_pInputThickness;
	double				m_dThick;//偏移值
};

//等厚命令
//抽壳命令
class kcmThickSolid : public kiCommand
{
public:
	kcmThickSolid(void);
	virtual ~kcmThickSolid(void);

	// 表明命令可以完成,主要和Apply函数结合使用.
	virtual  BOOL			CanFinish();//命令是否可以完成

	// 当输入工具获取结果,结束时,会回调对应命令的该函数.
	// 用于继续执行命令或切换输入工具.也可能返回KSTA_DONE,表明命令执行结束.
	//
	virtual  int			OnInputFinished(kiInputTool *pTool);

protected:
	virtual int			OnExecute();
	virtual int			OnEnd(int nCode);

protected:
	// 创建必要的输入工具
	virtual	BOOL			CreateInputTools();
	virtual BOOL			DestroyInputTools();
	virtual BOOL			InitInputToolQueue();

protected:
	BOOL					BuildThickSolid();

protected:
	kiOptionSet			m_optionSet;
	kiInputEntityTool		*m_pInputEntity;
	kiInputDoubleTool		*m_pInputThickness;
	bool					m_bRevDir;//是否反向
	double				m_dThick;//偏移值
};

#endif