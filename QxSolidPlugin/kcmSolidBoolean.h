//////////////////////////////////////////////////////////////////////////
//
#ifndef _KCM_SOLID_BOOLEAN_H_
#define _KCM_SOLID_BOOLEAN_H_

#include <vector>
#include "kiCommand.h"
#include "kiOptionSet.h"

class kiInputEntityTool;
class kcSolidEntity;
class kcEntity;

class kcmSolidBoolean : public kiCommand
{
public:
	kcmSolidBoolean(void);
	virtual ~kcmSolidBoolean(void);

	// 表明命令可以完成,主要和Apply函数结合使用.
	virtual  BOOL				CanFinish();//命令是否可以完成

	// 当输入工具获取结果,结束时,会回调对应命令的该函数.
	// 用于继续执行命令或切换输入工具.也可能返回KSTA_DONE,表明命令执行结束.
	//
	virtual  int				OnInputFinished(kiInputTool *pTool);

protected:
	// 创建必要的输入工具
	virtual	BOOL				CreateInputTools();
	virtual BOOL				DestroyInputTools();
	virtual BOOL				InitInputToolQueue();

protected:
	virtual BOOL				DoBoolean();
	virtual TopoDS_Shape		DoBoolean(const TopoDS_Shape& aS1,const TopTools_ListOfShape& aLS) = 0;

	BOOL						GetSolidEntity(kiInputEntityTool *pTool,kcSolidEntity *&pEntity);
	BOOL						GetShapeAndEntity(kiInputEntityTool *pTool,kcEntity *&pEntity,TopoDS_Shape& aShape);
	BOOL						GetShapesAndEntities(kiInputEntityTool *pTool,std::vector<kcEntity *>& entList,TopTools_ListOfShape& aLS);

	TopoDS_Shape				MakeHalfSolid(const TopoDS_Face& aFace,bool bRev);

protected:
	kiInputEntityTool			*_pInputEntity1;
	kiInputEntityTool			*_pInputEntity2;

	kiOptionSet				_optionSet;
	bool						_bParallel;//是否使用并发执行
	bool						_bAsHalfSpace;//face和shell是否作为半空间

protected:
	bool						bKeepTools_;//保留tools，不删除
};

// 并
class kcmSolidBoolFuse : public kcmSolidBoolean{
public:
	kcmSolidBoolFuse();

protected:
	virtual TopoDS_Shape	DoBoolean(const TopoDS_Shape& aS1,const TopTools_ListOfShape& aLS);

};

// 交
class kcmSolidBoolCommon : public kcmSolidBoolean{
public:
	kcmSolidBoolCommon();

protected:
	virtual TopoDS_Shape	DoBoolean(const TopoDS_Shape& aS1,const TopTools_ListOfShape& aLS);

};

// 差
class kcmSolidBoolCut : public kcmSolidBoolean{
public:
	kcmSolidBoolCut();

protected:
	virtual TopoDS_Shape	DoBoolean(const TopoDS_Shape& aS1,const TopTools_ListOfShape& aLS);

};

//分割
class kcmSolidSplit : public kcmSolidBoolean{
public:
	kcmSolidSplit();

protected:
	virtual TopoDS_Shape	DoBoolean(const TopoDS_Shape& aS1,const TopTools_ListOfShape& aLS);
};


#endif