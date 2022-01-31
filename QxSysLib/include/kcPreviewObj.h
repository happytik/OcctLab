//////////////////////////////////////////////////////////////////////////
// 预览对象，适用于临时绘制。不会保存，不可拾取。主要用于交互中临时对象的绘制。
// 
#ifndef _KC_PREVIEW_OBJ_H_
#define _KC_PREVIEW_OBJ_H_

#include <AIS_InteractiveContext.hxx>
#include <AIS_InteractiveObject.hxx>
#include <Aspect_TypeOfMarker.hxx>
#include <Graphic3d_AspectMarker3d.hxx>
#include <Graphic3d_AspectLine3d.hxx>
#include <Graphic3d_ArrayOfSegments.hxx>
#include <vector>
#include "kPoint.h"

#include "QxLibDef.h"

class kvCoordSystem;
class kcBasePlane;
class kcDocContext;

class QXLIB_API kcPreviewObj
{
public:
	kcPreviewObj(const Handle(AIS_InteractiveContext)& aCtx,kcDocContext *pDocCtx);
	virtual ~kcPreviewObj(void);

	//显示和隐藏
	virtual BOOL						Display(BOOL bShow);
	virtual BOOL						IsDisplayed() const;

	//清除
	virtual void						Clear();
	virtual void						Destroy();

public:
	//设置显示颜色
	virtual void						SetColor(double r,double g,double b);

protected:
	// 根据参数创建显示对象,当需要使用多组Group时，可以重载该函数
	virtual bool						DoBuildDisplay();
	// 具体的创建显示对象,创建到一个Group中
	virtual bool						BuildGroupDisplay(const Handle(Graphic3d_Group)& hGroup) = 0;

	// 更新显示对象,当数据发生改变时，可以调用该函数，重建显示数据
	virtual void						UpdateDisplay();
	
protected:
	Handle(AIS_InteractiveContext)		aAISCtx_;
	kcDocContext						*pDocContext_;//
	
	Handle(Prs3d_Presentation)			_hStru;
	double							_dColR,_dColG,_dColB;//颜色
	bool								_bBuilded;//是否构建了显示对象
};

// 临时点的显示
class QXLIB_API kcPreviewPoint : public kcPreviewObj{
public:
	kcPreviewPoint(const Handle_AIS_InteractiveContext& aCtx,kcDocContext *pDocCtx);
	virtual ~kcPreviewPoint();

	//更新
	void								Update(gp_Pnt& pnt);

	/* enum Aspect_TypeOfMarker {
	Aspect_TOM_POINT,
	Aspect_TOM_PLUS,
	Aspect_TOM_STAR,
	Aspect_TOM_O,
	Aspect_TOM_X,
	Aspect_TOM_O_POINT,
	Aspect_TOM_O_PLUS,
	Aspect_TOM_O_STAR,
	Aspect_TOM_O_X,
	Aspect_TOM_BALL,
	Aspect_TOM_RING1,
	Aspect_TOM_RING2,
	Aspect_TOM_RING3,
	Aspect_TOM_USERDEFINED
	};*/
	void								SetType(int type);	
	void								SetScale(double scale);

	Handle(Graphic3d_AspectMarker3d)&	GetMasker3d() { return m_aMaskerAttr;}

protected:
	// 具体的创建显示对象
	virtual bool						BuildGroupDisplay(const Handle(Graphic3d_Group)& hGroup);

protected:
	gp_Pnt							m_pnt;
	int								m_eType;
	double							m_dScale;

	Handle(Graphic3d_AspectMarker3d)	m_aMaskerAttr;
};

//////////////////////////////////////////////////////////////////////////
// 基类，实现一些曲线相关绘制属性
class QXLIB_API kcPreviewCurveObj : public kcPreviewObj{
public:
	kcPreviewCurveObj(const Handle_AIS_InteractiveContext& aCtx,kcDocContext *pDocCtx);
	virtual ~kcPreviewCurveObj();

	/*
	enum Aspect_TypeOfLine {
	Aspect_TOL_SOLID,
	Aspect_TOL_DASH,
	Aspect_TOL_DOT,
	Aspect_TOL_DOTDASH,
	Aspect_TOL_USERDEFINED
	};
	*/
	void								SetType(int type);
	void								SetWidth(double width);
	bool								HasInited() const {  return _bInited; }

	Handle(Graphic3d_AspectLine3d)&		GetLine3d() { return _aLineAttr;}

	//清除
	virtual void						Clear();
	virtual void						Destroy();

protected:
	virtual  void						SetPrimAspect();
	void								SetInited(bool bInited) { _bInited = bInited;}

protected:
	int								_eType;//类型
	double							_dWidth;//线宽

	Handle(Graphic3d_AspectLine3d)		_aLineAttr;

private:
	bool								_bInited;//是否初始化
};

class QXLIB_API kcPreviewLine : public kcPreviewCurveObj{
public:
	kcPreviewLine(const Handle_AIS_InteractiveContext& aCtx,kcDocContext *pDocCtx);
	virtual ~kcPreviewLine();
	
	//更新直线的两点
	void								Update(gp_Pnt& pnt1,gp_Pnt& pnt2);
	void								Update(const kPoint3& p1,const kPoint3& p2);

protected:
	// 具体的创建显示对象
	virtual bool						BuildGroupDisplay(const Handle(Graphic3d_Group)& hGroup);

protected:
	gp_Pnt							m_pnt1,m_pnt2;
};

typedef std::vector<gp_Pnt> stdPntVector;

//一系列线段
class QXLIB_API kcPreviewSegments : public kcPreviewCurveObj{
public:
	kcPreviewSegments(const Handle_AIS_InteractiveContext& aCtx,kcDocContext *pDocCtx);
	~kcPreviewSegments();

	void								AddSegment(const gp_Pnt &p1,const gp_Pnt &p2);
	virtual void						Clear();

protected:
	// 具体的创建显示对象
	virtual bool						BuildGroupDisplay(const Handle(Graphic3d_Group)& hGroup);

protected:
	stdPntVector						aPntVec_;//记录一系列点
};

// 三角形
class QXLIB_API kcPreviewTriangle : public kcPreviewCurveObj{
public:
	kcPreviewTriangle(const Handle_AIS_InteractiveContext& aCtx,kcDocContext *pDocCtx);
	virtual ~kcPreviewTriangle();

	//设定三个点
	void								Set(const gp_Pnt& p1,const gp_Pnt& p2,const gp_Pnt& p3);

protected:
	// 具体的创建显示对象
	virtual bool						BuildGroupDisplay(const Handle(Graphic3d_Group)& hGroup);

protected:
	gp_Pnt							m_pnt1,m_pnt2,m_pnt3;

};

// 矩形
class QXLIB_API kcPreviewRect : public kcPreviewCurveObj{
public:
	kcPreviewRect(const Handle_AIS_InteractiveContext& aCtx,kcDocContext *pDocCtx);
	virtual ~kcPreviewRect();

	//
	void								Init(gp_Pnt& pnt1,kcBasePlane *pWP);
	void								Update(gp_Pnt& pnt2);

protected:
	// 具体的创建显示对象
	virtual bool						BuildGroupDisplay(const Handle(Graphic3d_Group)& hGroup);

protected:
	kcBasePlane						*m_pWorkPlane;//工作平面
	gp_Pnt							m_pnt1;
	gp_Pnt							m_aPnt[4];//四个交点

};

// box框架预览
class QXLIB_API kcPreviewBox : public kcPreviewCurveObj{
public:
	kcPreviewBox(const Handle_AIS_InteractiveContext& aCtx,kcDocContext *pDocCtx);
	virtual ~kcPreviewBox();

	//
	bool								Init(gp_Pnt& pnt1,gp_Pnt& pnt2,kcBasePlane *pWP);
	void								Update(gp_Pnt& pnt3);

protected:
	// 具体的创建显示对象
	virtual bool						BuildGroupDisplay(const Handle(Graphic3d_Group)& hGroup);

protected:
	kcBasePlane						*m_pWorkPlane;//工作平面
	gp_Pnt							m_pnt1,m_pnt2;
	double							m_aPnt[8][3];//点
	int								m_aEdge[12][2];//边
};

// 圆的临时显示
class QXLIB_API kcPreviewCircle : public kcPreviewCurveObj{
public:
	kcPreviewCircle(const Handle_AIS_InteractiveContext& aCtx,kcDocContext *pDocCtx);
	virtual ~kcPreviewCircle();

	//初始化
	BOOL								Init(const Handle(Geom_Circle)& hCircle);
	void								Update(double radius);

	//清除
	virtual void						Clear();
	virtual void						Destroy();

protected:
	// 具体的创建显示对象
	virtual bool						BuildGroupDisplay(const Handle(Graphic3d_Group)& hGroup);

protected:
	Handle(Geom_Circle)				m_hCircle;
};

// 圆弧的临时显示
class QXLIB_API kcPreviewArc : public kcPreviewCurveObj{
public:
	kcPreviewArc(const Handle_AIS_InteractiveContext& aCtx,kcDocContext *pDocCtx);
	virtual ~kcPreviewArc();

	//初始化
	BOOL								Init(const Handle(Geom_Circle)& aCircle);
	void								Update(double ang1,double ang2);

	//清除
	virtual void						Clear();
	virtual void						Destroy();

protected:
	// 具体的创建显示对象
	virtual bool						BuildGroupDisplay(const Handle(Graphic3d_Group)& hGroup);

protected:
	Handle(Geom_Circle)				m_aCircle;
	Handle(Geom_TrimmedCurve)			m_aArc;
};

// 曲线的临时显示
class QXLIB_API kcPreviewCurve : public kcPreviewCurveObj{
public:
	kcPreviewCurve(const Handle_AIS_InteractiveContext& aCtx,kcDocContext *pDocCtx);
	virtual ~kcPreviewCurve();

	void								Update(const Handle(Geom_Curve)& aCurve);
	void								Update(const TopoDS_Edge& aEdge);

	//清除
	virtual void						Clear();
	virtual void						Destroy();

protected:
	// 具体的创建显示对象
	virtual bool						BuildGroupDisplay(const Handle(Graphic3d_Group)& hGroup);

protected:
	Handle(Geom_Curve)				m_aCurve;

};

// 多义线的临时显示
class QXLIB_API kcPreviewPolyline : public kcPreviewCurveObj{
public:
	kcPreviewPolyline(const Handle_AIS_InteractiveContext& aCtx,kcDocContext *pDocCtx);
	virtual ~kcPreviewPolyline();

	void								AddPoint(const kPoint3& p);

	//清除
	virtual void						Clear();
	virtual void						Destroy();

protected:
	// 具体的创建显示对象
	virtual bool						BuildGroupDisplay(const Handle(Graphic3d_Group)& hGroup);

protected:
	std::vector<kPoint3>				m_pointArray;

};

//显示控制点数据
class QXLIB_API kcPreviewNurbsPoles : public kcPreviewObj{
public:
	kcPreviewNurbsPoles(const Handle_AIS_InteractiveContext& aCtx,kcDocContext *pDocCtx);
	~kcPreviewNurbsPoles();

	bool					Initialize(const Handle(Geom_Curve) &aSplCrv);
	bool					Initialize(const Handle(Geom_Surface) &aSplSurf);

	void					SetLineColor(double r,double g,double b);

	//
	void					ShowUIsoLine(bool bShow);//是否显示U向（等V方向）连接线
	void					ShowVIsoLine(bool bShow);//是否显示V向（等U方向）连接线

protected:
	// 根据参数创建显示对象,当需要使用多组Group时，可以重载该函数
	virtual bool						DoBuildDisplay();
	// 具体的创建显示对象,创建到一个Group中
	virtual bool						BuildGroupDisplay(const Handle(Graphic3d_Group)& hGroup);

protected:
	Handle(Geom_BSplineCurve)		aSplCurve_;//要预览的样条曲线
	Handle(Geom_BSplineSurface)	aSplSurf_;//要预览的样条曲面

	double				dLineColorR_,dLineColorG_,dLineColorB_;
	bool					bShowUIsoLine_,bShowVIsoLine_;
};

// 显示网格面
class QXLIB_API kcPreviewMesh : public kcPreviewObj{
public:
	kcPreviewMesh(const Handle_AIS_InteractiveContext& aCtx,kcDocContext *pDocCtx);
	~kcPreviewMesh();

	bool					Initialize(const TopoDS_Face &aFace);
	bool					Initialize(const TopoDS_Shape &aS);

protected:
	// 根据参数创建显示对象,当需要使用多组Group时，可以重载该函数
	virtual bool			DoBuildDisplay();

	// 具体的创建显示对象
	virtual bool			BuildGroupDisplay(const Handle(Graphic3d_Group)& hGroup);

protected:
	void					DoBuildFaceDisplay(const TopoDS_Face &aF);

protected:
	TopoDS_Face			aFace_;
	TopoDS_Shape			aShape_;

};

#endif