/// 对象属性
#ifndef _KCM_SHAPE_PROPERTY_H_
#define _KCM_SHAPE_PROPERTY_H_

#include "kiCommand.h"

class kiInputEntityTool;

//一个shape对象的属性
//
class kcmShapeProperty : public kiCommand
{
public:
	kcmShapeProperty(void);
	virtual ~kcmShapeProperty(void);

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
	virtual BOOL			DoShowProperty();

protected:
	kiInputEntityTool		*m_pInputEntityTool;
};

//一条边的属性
class kcmEdgeProperty : public kiCommand
{
public:
	kcmEdgeProperty(void);
	virtual ~kcmEdgeProperty(void);

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
	virtual BOOL			DoShowProperty();

protected:
	kiInputEntityTool		*pInputEntityTool_;

	Handle(AIS_InteractiveObject)  aCtxObject_;
	TopoDS_Shape			aShape_;
};

//一个面的属性
class kcmFaceProperty : public kiCommand
{
public:
	kcmFaceProperty(void);
	virtual ~kcmFaceProperty(void);

	// 表明命令可以完成,主要和Apply函数结合使用.
	virtual  BOOL			CanFinish();//命令是否可以完成

	// 当输入工具获取结果,结束时,会回调对应命令的该函数.
	// 用于继续执行命令或切换输入工具.也可能返回KSTA_DONE,表明命令执行结束.
	//
	virtual  int			OnInputFinished(kiInputTool *pTool);

protected:
	virtual int			OnExecute();

protected:
	// 创建必要的输入工具
	virtual	BOOL			CreateInputTools();
	virtual BOOL			DestroyInputTools();
	virtual BOOL			InitInputToolQueue();

protected:
	virtual BOOL			DoShowProperty();

protected:
	kiInputEntityTool		*pInputEntityTool_;

	Handle(AIS_InteractiveObject)  aCtxObject_;
	TopoDS_Face			aFace_;//
	bool					bLocalFace_;//是否局部面
};

#endif