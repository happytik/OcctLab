#ifndef _KCM_ONE_FACE_BASE_H_
#define _KCM_ONE_FACE_BASE_H_

#include <vector>
#include <TopoDS_Face.hxx>
#include "kiCommand.h"
#include "kiOptionSet.h"

class kiInputEntityTool;
class kiInputIntTool;

class kcmOneFaceBase : public kiCommand
{
public:
	kcmOneFaceBase(void);
	virtual ~kcmOneFaceBase(void);

	// 表明命令可以完成,主要和Apply函数结合使用.
	virtual  BOOL			CanFinish();//命令是否可以完成

	// 当输入工具获取结果,结束时,会回调对应命令的该函数.
	// 用于继续执行命令或切换输入工具.也可能返回KSTA_DONE,表明命令执行结束.
	//
	virtual  int			OnInputFinished(kiInputTool *pTool);

protected:
	virtual int				OnExecute();
	virtual int				OnEnd(int nCode);

	virtual int				OnSelOneFace() = 0;//虚函数，需要实现

protected:
	// 创建必要的输入工具
	virtual	BOOL			CreateInputTools();
	virtual BOOL			DestroyInputTools();
	virtual BOOL			InitInputToolQueue();

protected:
	TopoDS_Face				GetFaceFromGlobalSelSet();
	int						AddShapeToModel(const TopoDS_Shape& aShape);

protected:
	kiInputEntityTool		*m_pSelFaceTool;

	TopoDS_Face				m_aFace;//选中的face
};

class kcmOneFaceCopy : public kcmOneFaceBase
{
public:
	kcmOneFaceCopy();
	virtual ~kcmOneFaceCopy();

protected:
	virtual int				OnSelOneFace();
};

// 获取一个face的裁剪环的参数曲线
class kcmOneFaceParLoop : public kcmOneFaceBase
{
public:
	kcmOneFaceParLoop();
	virtual ~kcmOneFaceParLoop();

protected:
	virtual int				OnSelOneFace();
};

//对一个曲面进行插值，获取插值后的面
class kcmInterpOneFace : public kcmOneFaceBase
{
public:
	kcmInterpOneFace();
	virtual ~kcmInterpOneFace();

protected:
	virtual int				OnSelOneFace();
};

//导出一个选中的面
class kcmExpOneFaceToStep : public kcmOneFaceBase
{
public:
	kcmExpOneFaceToStep();
	virtual ~kcmExpOneFaceToStep();

protected:
	virtual int				OnSelOneFace();
};

//周期样条曲面的一些测试
class kcmPeriodicBSpSurfTry : public kcmOneFaceBase
{
public:
	kcmPeriodicBSpSurfTry(int subcmd);
	virtual ~kcmPeriodicBSpSurfTry();

protected:
	virtual int				OnSelOneFace();

private:
	void						ToNoPeriodicSurf(const Handle(Geom_BSplineSurface) &aBspSurf);
	void						ChangeUVOrigin(const Handle(Geom_BSplineSurface) &aBspSurf,int uvflag);

protected:
	int						subCmd_;//子命令
};

//提前第n个面
class kcmExtractNFace : public kiCommand{
public:
	kcmExtractNFace();
	virtual ~kcmExtractNFace();

	// 表明命令可以完成,主要和Apply函数结合使用.
	virtual  BOOL				CanFinish();//命令是否可以完成

	// 当输入工具获取结果,结束时,会回调对应命令的该函数.
	// 用于继续执行命令或切换输入工具.也可能返回KSTA_DONE,表明命令执行结束.
	//
	virtual  int				OnInputFinished(kiInputTool *pTool);

protected:
	// 创建必要的输入工具
	virtual BOOL				CreateInputTools();
	virtual BOOL				DestroyInputTools();
	virtual BOOL				InitInputToolQueue();

protected:
	BOOL						DoExtractFace();

protected:
	kiInputEntityTool			*_pInputEntityTool;
	kiInputIntTool			*_pInputNumTool;
	int						_nFaceNo;//face的序号
};


#endif