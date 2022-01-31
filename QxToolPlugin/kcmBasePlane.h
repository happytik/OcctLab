/////////////////////////////////////////////////////
// 基准面相关命令实现
//
#pragma once

#include "kiCommand.h"

class kiInputDirTool;
class kiInputDoubleTool;
class kiInputPointTool;
class kiInputEntityTool;

//偏移生成基准面
class kcmOffsetBasePlane : public kiCommand{
public:
	kcmOffsetBasePlane();
	virtual ~kcmOffsetBasePlane();

	// 表明命令可以完成,主要和Apply函数结合使用.
	virtual  BOOL			CanFinish();//命令是否可以完成

	// 当输入工具获取结果,结束时,会回调对应命令的该函数.
	// 用于继续执行命令或切换输入工具.也可能返回KSTA_DONE,表明命令执行结束.
	//
	virtual  int			OnInputFinished(kiInputTool *pTool);

protected:
	// 创建必要的输入工具,在Initialize中调用，每个命令仅仅调用一次。
	virtual	BOOL			CreateInputTools();
	// 销毁创建的输入工具.每个命令仅调用一次.
	virtual BOOL			DestroyInputTools();
	// 在begin中调用，主要是初始化工具队列。
	virtual BOOL			InitInputToolQueue();

protected:
	BOOL					CreateBasePlane();

protected:
	kiInputDirTool		*_pInputDirTool;
	kiInputDoubleTool		*_pInputOffsetTool;

	kVector3				_offDire;
	double				_dOffset;
};

///三点生成基准面
class kcmThreePntBasePlane : public kiCommand{
public:
	kcmThreePntBasePlane();
	virtual ~kcmThreePntBasePlane();

	// 表明命令可以完成,主要和Apply函数结合使用.
	virtual  BOOL			CanFinish();//命令是否可以完成

	// 当输入工具获取结果,结束时,会回调对应命令的该函数.
	// 用于继续执行命令或切换输入工具.也可能返回KSTA_DONE,表明命令执行结束.
	//
	virtual  int			OnInputFinished(kiInputTool *pTool);

protected:
	// 创建必要的输入工具,在Initialize中调用，每个命令仅仅调用一次。
	virtual	BOOL			CreateInputTools();
	// 销毁创建的输入工具.每个命令仅调用一次.
	virtual BOOL			DestroyInputTools();
	// 在begin中调用，主要是初始化工具队列。
	virtual BOOL			InitInputToolQueue();

protected:
	BOOL					CreateBasePlane();

protected:
	kiInputPointTool		*pInputPointTool1_;
	kiInputPointTool		*pInputPointTool2_;
	kiInputPointTool		*pInputPointTool3_;

	kPoint3				pnt1_,pnt2_,pnt3_;
};


// 根据一个Face及其上一点，生成基准面
class kcmBasePlaneByFace : public kiCommand{
public:
	kcmBasePlaneByFace();
	virtual ~kcmBasePlaneByFace();

	// 表明命令可以完成,主要和Apply函数结合使用.
	virtual  BOOL			CanFinish();//命令是否可以完成

	// 当输入工具获取结果,结束时,会回调对应命令的该函数.
	// 用于继续执行命令或切换输入工具.也可能返回KSTA_DONE,表明命令执行结束.
	//
	virtual  int			OnInputFinished(kiInputTool *pTool);

protected:
	// 执行函数
	virtual int			OnExecute();

	// 创建必要的输入工具,在Initialize中调用，每个命令仅仅调用一次。
	virtual	BOOL			CreateInputTools();
	// 销毁创建的输入工具.每个命令仅调用一次.
	virtual BOOL			DestroyInputTools();
	// 在begin中调用，主要是初始化工具队列。
	virtual BOOL			InitInputToolQueue();

protected:
	BOOL					CreateBasePlane();

protected:
	kiInputEntityTool		*pFacePickTool_; //拾取平面的工具
};
