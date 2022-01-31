//////////////////////////////////////////////////////////////////////////
// 获取曲面上的曲线，包括：边界曲线、环、流线等等。
#ifndef _KCM_FACE_CURVE_H_
#define _KCM_FACE_CURVE_H_

#include <string>
#include <vector>
#include "kiCommand.h"
#include "kiOptionSet.h"

class kiInputEntityTool;
class kcEntity;
class kcPreviewObj;
class kcPreviewCurve;

//曲面的边界
class kiFaceBoundCurve : public kiCommand{
public:
	kiFaceBoundCurve();
	virtual ~kiFaceBoundCurve();

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
	//
	virtual BOOL			DoBuildBoundCurve() = 0;

protected:
	kiInputEntityTool		*m_pPickEntityTool;
	kiInputEntityTool		*m_pPickLocalShapeTool;

protected:
	std::string				m_strPrompt;
	int						m_nSelModel;
};

//边界曲线
class kcmFaceBoundEdge : public kiFaceBoundCurve{
public:
	kcmFaceBoundEdge();

protected:
	//
	virtual BOOL			DoBuildBoundCurve();
};

//边界曲线方向
class kcmFaceBoundEdgeDire : public kiFaceBoundCurve{
public:
	kcmFaceBoundEdgeDire();
	virtual ~kcmFaceBoundEdgeDire();

	// 当输入工具获取结果,结束时,会回调对应命令的该函数.
	// 用于继续执行命令或切换输入工具.也可能返回KSTA_DONE,表明命令执行结束.
	//
	virtual  int			OnInputFinished(kiInputTool *pTool);

protected:
	virtual int				OnEnd(int nCode);
	// 创建必要的输入工具
	virtual	BOOL			CreateInputTools();

protected:
	//
	virtual BOOL			DoBuildBoundCurve();
	bool					DoShowWireDire();
	bool					DoGenEdgeDireDisp(const TopoDS_Edge& aEdge,const TopoDS_Face& aFace);

	void					ClearTempDispObj();

protected:
	bool					m_bTopolDire;//是否拓扑层面的方向
	kiOptionSet				m_optionSet;

	std::vector<kcPreviewObj *>	m_prevObjArray;
};

class kcmFaceBoundWire : public kiFaceBoundCurve{
public:
	kcmFaceBoundWire();

protected:
	//
	virtual BOOL			DoBuildBoundCurve();

	virtual BOOL			DoBuildEntity(const TopoDS_Wire& aWire,const TopoDS_Face& aFace,
										  std::vector<kcEntity *>& aEnt);
};

// 获取Face的wire的原始顺序曲线
class kcmFaceRawWire : public kcmFaceBoundWire{
public:
	kcmFaceRawWire();

protected:
	//
	virtual BOOL			DoBuildEntity(const TopoDS_Wire& aWire,const TopoDS_Face& aFace,
										  std::vector<kcEntity *>& aEnt);
};

//曲面流线
class kcmFaceIsoCurve : public kiCommand{
public:
	kcmFaceIsoCurve();

	// 当输入工具获取结果,结束时,会回调对应命令的该函数.
	// 用于继续执行命令或切换输入工具.也可能返回KSTA_DONE,表明命令执行结束.
	//
	virtual  int			OnInputFinished(kiInputTool *pTool);

public:
	virtual int			OnLButtonUp(kuiMouseInput& mouseInput);
	virtual int			OnMouseMove(kuiMouseInput& mouseInput);

protected:
	virtual int			OnExecute();
	virtual int			OnEnd(int nCode);

protected:
	// 创建必要的输入工具
	virtual	BOOL			CreateInputTools();
	virtual BOOL			DestroyInputTools();
	virtual BOOL			InitInputToolQueue();

protected:

	bool					DoGetSelFace();
	bool					DoBuildIsoCurve();
	Handle(Geom_Curve)	DoGetIsoCurve(double uv);

protected:
	kiInputEntityTool		*pPickEntityTool_;
	kiOptionSet			aOptionSet_;
	kcPreviewCurve		*pPrvwCurve_;

	TopoDS_Face			aSelFace_;
	int					nIsoDir_; //0 U向 1: V向
	double				dParam_;//参数值
};

#endif