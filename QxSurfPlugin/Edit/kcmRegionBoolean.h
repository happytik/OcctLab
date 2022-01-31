//////////////////////////////////////////////////////////////////////////
//
#ifndef _KCM_REGION_BOOLEAN_H_
#define _KCM_REGION_BOOLEAN_H_

#include "kiCommand.h"

class kiInputEntityTool;
class kcSolidEntity;
class kcEntity;

class kcmRegionBoolean : public kiCommand
{
public:
	kcmRegionBoolean(void);
	virtual ~kcmRegionBoolean(void);

	// 表明命令可以完成,主要和Apply函数结合使用.
	virtual  BOOL			CanFinish();//命令是否可以完成

	// 当输入工具获取结果,结束时,会回调对应命令的该函数.
	// 用于继续执行命令或切换输入工具.也可能返回KSTA_DONE,表明命令执行结束.
	//
	virtual  int			OnInputFinished(kiInputTool *pTool);

protected:
	// 创建必要的输入工具
	virtual	BOOL			CreateInputTools();
	virtual BOOL			DestroyInputTools();
	virtual BOOL			InitInputToolQueue();

protected:
	virtual BOOL			DoBoolean();
	virtual TopoDS_Shape	DoBoolean(const TopoDS_Shape& aS1,const TopoDS_Shape& aS2) = 0;

	BOOL					CheckInputShape(kiInputEntityTool *pTool);
	BOOL					GetSolidEntity(kiInputEntityTool *pTool,kcSolidEntity *&pEntity);
	BOOL					GetShapeAndEntity(kiInputEntityTool *pTool,kcEntity *&pEntity,TopoDS_Shape& aShape);

protected:
	kiInputEntityTool		*m_pInputEntity1;
	kiInputEntityTool		*m_pInputEntity2;
};

// 并
class kcmRegionBoolFuse : public kcmRegionBoolean{
public:
	kcmRegionBoolFuse();

protected:
	virtual TopoDS_Shape	DoBoolean(const TopoDS_Shape& aS1,const TopoDS_Shape& aS2);

};

// 交
class kcmRegionBoolCommon : public kcmRegionBoolean{
public:
	kcmRegionBoolCommon();

protected:
	virtual TopoDS_Shape	DoBoolean(const TopoDS_Shape& aS1,const TopoDS_Shape& aS2);

};

// 差
class kcmRegionBoolCut : public kcmRegionBoolean{
public:
	kcmRegionBoolCut();

protected:
	virtual TopoDS_Shape	DoBoolean(const TopoDS_Shape& aS1,const TopoDS_Shape& aS2);

};

#endif