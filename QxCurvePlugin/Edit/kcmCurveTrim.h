//////////////////////////////////////////////////////////////////////////
//
#ifndef _KCM_CURVE_TRIM_H_
#define _KCM_CURVE_TRIM_H_

#include <AIS_ListOfInteractive.hxx>
#include "kiCommand.h"

class kiInputEntityTool;

// 两线裁剪
class kcmCurveTrim : public kiCommand
{
public:
	kcmCurveTrim(void);
	virtual ~kcmCurveTrim(void);

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

	virtual int				OnLButtonUp(kuiMouseInput& mouseInput);
	virtual int				OnMouseMove(kuiMouseInput& mouseInput);

protected:
	// 进行分割
	BOOL					DoSplit();
	// 裁剪
	BOOL					DoTrim(int x,int y);

protected:
	kiInputEntityTool		*m_pInputCurve;
	kiInputEntityTool		*m_pInputCurveToTrim;

	AIS_ListOfInteractive	m_listOfObj;
	int						m_nTrimCount;
	BOOL					m_bCanTrim;
};

//快速裁剪
//
class kcmCurveQuickTrim : public kiCommand
{
public:
	kcmCurveQuickTrim(void);
	virtual ~kcmCurveQuickTrim(void);

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

	virtual int				OnLButtonUp(kuiMouseInput& mouseInput);
	virtual int				OnMouseMove(kuiMouseInput& mouseInput);

protected:
	// 进行分割
	BOOL					DoSplit();
	// 裁剪
	BOOL					DoTrim(int x,int y);
	// 从ctx中清除临时显示对象
	BOOL					RemoveTmpObjList(AIS_ListOfInteractive& objList);

protected:
	kiInputEntityTool		*m_pInputCurve1;
	kiInputEntityTool		*m_pInputCurve2;

	AIS_ListOfInteractive	m_aObjList1;
	AIS_ListOfInteractive	m_aObjList2;//分别记录两个曲线的临时显示对象
	int						m_nTrimCount1;
	int						m_nTrimCount2;
	BOOL					m_bCanTrim;
};

// 两线裁剪
class kcmSplitAllEdges : public kiCommand
{
public:
	kcmSplitAllEdges(void);
	virtual ~kcmSplitAllEdges(void);

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

	virtual int				OnLButtonUp(kuiMouseInput& mouseInput);
	virtual int				OnMouseMove(kuiMouseInput& mouseInput);

protected:
	// 进行分割
	BOOL					DoSplit();

protected:
	kiInputEntityTool		*m_pInputEdges;

	AIS_ListOfInteractive	m_listOfObj;
	int					m_nTrimCount;
	BOOL					m_bCanTrim;
};

#endif
