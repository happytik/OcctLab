#include "StdAfx.h"
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <AIS_Shape.hxx>
#include <Graphic3d_AspectLine3d.hxx>
#include <GCPnts_TangentialDeflection.hxx>
#include <GeomAdaptor_Curve.hxx>
#include <Graphic3d_ArrayOfPolylines.hxx>
#include <Graphic3d_StructureManager.hxx>
#include <Graphic3d_SequenceOfGroup.hxx>
#include <Aspect_TypeOfMarker.hxx>
#include <Graphic3d_AspectMarker3d.hxx>
#include <Graphic3d_ArrayOfSegments.hxx>
#include <Graphic3d_ArrayOfPolylines.hxx>
#include <OpenGl_Group.hxx>
#include "kvCoordSystem.h"
#include "kcBasePlane.h"
#include "kPoint.h"
#include "kcDocContext.h"
#include "kcSysUtils.h"
#include "kcPreviewObj.h"

#pragma warning(disable : 4996)

kcPreviewObj::kcPreviewObj(const Handle(AIS_InteractiveContext)& aCtx,kcDocContext *pDocCtx)
	:aAISCtx_(aCtx),pDocContext_(pDocCtx)
{
	_hStru.Nullify();
	_dColR = _dColG = _dColB = 0.0;
	_bBuilded = false;

	//构建显示对象
	if(!aCtx.IsNull() && !aCtx->CurrentViewer().IsNull()){
		Handle(V3d_Viewer) aViewer = aCtx->CurrentViewer();
		if(!aViewer.IsNull()){
			_hStru = new Prs3d_Presentation(aViewer->StructureManager());	
		}
	}
		
}

kcPreviewObj::~kcPreviewObj(void)
{
	Destroy();
}

//显示和隐藏
BOOL  kcPreviewObj::Display(BOOL bShow)
{
	if(_hStru.IsNull())
		return FALSE;

	BOOL bDisp = IsDisplayed();
	if((bDisp && bShow) || (!bDisp && !bShow)){//不需要改变
		return TRUE;
	}
	
	if(bShow){
		if(!_bBuilded){
			//创建显示对象
			DoBuildDisplay();
			_bBuilded = true;
		}

		_hStru->Display();
	}else{
		_hStru->Erase();
	}

	// 这里，调用该接口更新
	if(pDocContext_){
		pDocContext_->RedrawCurView();
	}
	
	return TRUE;
}

BOOL  kcPreviewObj::IsDisplayed() const
{
	if(_hStru.IsNull())
		return FALSE;
	return _hStru->IsDisplayed();
}

void  kcPreviewObj::Clear()
{
	if(!_hStru.IsNull()){
		_hStru->Erase();//隐藏
		_hStru->Clear();//清除
		//
	}
	_bBuilded = false;
}

void  kcPreviewObj::Destroy()
{
	if(!_hStru.IsNull()){
		_hStru->Erase();//隐藏
		_hStru->Clear();
		
		_hStru.Nullify();
	}
}

void	kcPreviewObj::SetColor(double r,double g,double b)
{
	_dColR = r;_dColG = g;_dColB = b;
}

// 根据参数创建显示对象,这里只使用一个Group
//
bool  kcPreviewObj::DoBuildDisplay()
{
	//清除数据,主要是group的数据。
	Handle(Graphic3d_Group) hGroup;
	int nbGrp = _hStru->NumberOfGroups();
	if(nbGrp <= 0){
		//
		hGroup = _hStru->NewGroup();
		if(hGroup.IsNull())
			return false;
	}else{
		//清除旧的
		hGroup = _hStru->Groups().First();
		hGroup->Clear();
	}

	// 重新创建显示对象
	return BuildGroupDisplay(hGroup);
}

// 更新显示对象
void  kcPreviewObj::UpdateDisplay()
{
	if(_hStru.IsNull())
		return;

	BOOL bDisp = IsDisplayed();
	if(bDisp){
		Display(FALSE);
	}

	_bBuilded = false;
	if(!DoBuildDisplay())
		return;
	_bBuilded = true;

	if(bDisp){
		Display(TRUE);
	}
}

//////////////////////////////////////////////////////////////////////////
//
// 临时点的显示
kcPreviewPoint::kcPreviewPoint(const Handle_AIS_InteractiveContext& aCtx,kcDocContext *pDocCtx)
	:kcPreviewObj(aCtx,pDocCtx)
{
	m_eType = Aspect_TOM_O_PLUS;
	m_dScale = 1.0;

	m_aMaskerAttr = new Graphic3d_AspectMarker3d;
}

kcPreviewPoint::~kcPreviewPoint()
{
	m_aMaskerAttr.Nullify();
}

//更新
void	kcPreviewPoint::Update(gp_Pnt& pnt)
{
	m_pnt = pnt;
	//
	UpdateDisplay();
}

// 具体的创建显示对象
bool	kcPreviewPoint::BuildGroupDisplay(const Handle(Graphic3d_Group)& hGroup)
{
	Handle(Graphic3d_AspectMarker3d) mct = GetMasker3d();
	mct->SetColor(Quantity_Color(_dColR,_dColG,_dColB,Quantity_TOC_RGB));
	mct->SetType((Aspect_TypeOfMarker)m_eType);
	mct->SetScale(m_dScale);
	hGroup->SetGroupPrimitivesAspect(mct);

	Graphic3d_Vertex vp1(m_pnt.X(),m_pnt.Y(),m_pnt.Z());
	//hGroup->Marker(vp1);

	return true;
}

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
void	kcPreviewPoint::SetType(int type)
{
	m_eType = type;
	if(type < Aspect_TOM_POINT) m_eType = Aspect_TOM_POINT;
	if(type >=  Aspect_TOM_USERDEFINED) m_eType = Aspect_TOM_O_PLUS;
}

void	kcPreviewPoint::SetScale(double scale)
{
	m_dScale = scale;
}

//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
// 基类，实现一些曲线相关绘制属性
kcPreviewCurveObj::kcPreviewCurveObj(const Handle_AIS_InteractiveContext& aCtx,kcDocContext *pDocCtx)
	:kcPreviewObj(aCtx,pDocCtx)
{
	_eType = Aspect_TOL_SOLID;
	_dWidth = 1.0;

	_aLineAttr = new Graphic3d_AspectLine3d;
	_bInited = false;
}

kcPreviewCurveObj::~kcPreviewCurveObj()
{
	Destroy();
}

/*
enum Aspect_TypeOfLine {
Aspect_TOL_SOLID,
Aspect_TOL_DASH,
Aspect_TOL_DOT,
Aspect_TOL_DOTDASH,
Aspect_TOL_USERDEFINED
};
*/
void	kcPreviewCurveObj::SetType(int type)
{
	_eType = type;
	if(_eType < Aspect_TOL_SOLID) _eType = Aspect_TOL_SOLID;
	if(_eType >= Aspect_TOL_USERDEFINED) _eType = Aspect_TOL_SOLID;
}

void	kcPreviewCurveObj::SetWidth(double width)
{
	_dWidth = width;
	if(_dWidth < 0.0)
		_dWidth = 1.0;
}

// 设定属性信息
void  kcPreviewCurveObj::SetPrimAspect()
{
	Handle(Graphic3d_AspectLine3d) hAspLin = GetLine3d();
	hAspLin->SetColor(Quantity_Color(_dColR,_dColG,_dColB,Quantity_TOC_RGB));
	hAspLin->SetType((Aspect_TypeOfLine)_eType);
	hAspLin->SetWidth(_dWidth);

	const Graphic3d_SequenceOfGroup& aGroups = _hStru->Groups();
	Graphic3d_SequenceOfGroup::Iterator aGroupIter(aGroups);
	for(;aGroupIter.More();aGroupIter.Next()){
		const Handle(Graphic3d_Group)& aGroup = aGroupIter.Value();
		aGroup->SetGroupPrimitivesAspect(hAspLin);
	}
}

//清除
void	kcPreviewCurveObj::Clear()
{
	kcPreviewObj::Clear();

	_bInited = false;
}

void	kcPreviewCurveObj::Destroy()
{
	_aLineAttr.Nullify();

	kcPreviewObj::Destroy();
}
//////////////////////////////////////////////////////////////////////////
//

kcPreviewLine::kcPreviewLine(const Handle_AIS_InteractiveContext& aCtx,kcDocContext *pDocCtx)
	:kcPreviewCurveObj(aCtx,pDocCtx)
{
}

kcPreviewLine::~kcPreviewLine()
{
	Destroy();
}

//更新直线的两点
void	kcPreviewLine::Update(gp_Pnt& pnt1,gp_Pnt& pnt2)
{
	if(pnt1.Distance(pnt2) < 1e-6)
		return;

	m_pnt1 = pnt1;
	m_pnt2 = pnt2;

	UpdateDisplay();

	SetInited(true);
}

void	kcPreviewLine::Update(const kPoint3& p1,const kPoint3& p2)
{
	if(p1.distance(p2) < 1e-6)
		return;

	m_pnt1.SetCoord(p1[0],p1[1],p1[2]);
	m_pnt2.SetCoord(p2[0],p2[1],p2[2]);

	UpdateDisplay();

	SetInited(true);
}
// 具体的创建显示对象
bool	kcPreviewLine::BuildGroupDisplay(const Handle(Graphic3d_Group)& hGroup)
{
	SetPrimAspect();

	Handle(Graphic3d_ArrayOfSegments) aSegs = new Graphic3d_ArrayOfSegments(2);
	//
	aSegs->AddVertex(m_pnt1.X(),m_pnt1.Y(),m_pnt1.Z());
	aSegs->AddVertex(m_pnt2.X(),m_pnt2.Y(),m_pnt2.Z());

	hGroup->AddPrimitiveArray(aSegs);

	return true;
}

///////////////////////////////////////////////////////////////////////////
//
//一系列线段

kcPreviewSegments::kcPreviewSegments(const Handle_AIS_InteractiveContext& aCtx,kcDocContext *pDocCtx)
	:kcPreviewCurveObj(aCtx,pDocCtx)
{
}

kcPreviewSegments::~kcPreviewSegments()
{
}

void  kcPreviewSegments::AddSegment(const gp_Pnt &p1,const gp_Pnt &p2)
{
	double dist = p1.Distance(p2);
	if(dist < Precision::Confusion() * 10)
		return;

	aPntVec_.push_back(p1);
	aPntVec_.push_back(p2);
}

void  kcPreviewSegments::Clear()
{
	kcPreviewCurveObj::Clear();

	aPntVec_.clear();	
}

// 具体的创建显示对象
bool  kcPreviewSegments::BuildGroupDisplay(const Handle(Graphic3d_Group)& hGroup)
{
	if(aPntVec_.empty())
		return false;

	Quantity_Color aCol(_dColR,_dColG,_dColB,Quantity_TOC_RGB);
	int ix,num = (int)aPntVec_.size();
	Handle(Graphic3d_ArrayOfSegments) aSegs = new Graphic3d_ArrayOfSegments(num,num,true);
	for(ix = 0;ix < num;ix ++){
		const gp_Pnt &aPnt = aPntVec_.at(ix);
		aSegs->AddVertex(aPnt,aCol);
	}
	//添加边
	for(ix = 0;ix < num;ix += 2){
		aSegs->AddEdge(ix+1);
		aSegs->AddEdge(ix+2);
	}

	hGroup->AddPrimitiveArray(aSegs);

	return true;
}

///////////////////////////////////////////////////////////////////////////
//
// 三角形
kcPreviewTriangle::kcPreviewTriangle(const Handle_AIS_InteractiveContext& aCtx,kcDocContext *pDocCtx)
	:kcPreviewCurveObj(aCtx,pDocCtx)
{
}

kcPreviewTriangle::~kcPreviewTriangle()
{
	Destroy();
}

//设定三个点
void	kcPreviewTriangle::Set(const gp_Pnt& p1,const gp_Pnt& p2,const gp_Pnt& p3)
{
	m_pnt1 = p1;m_pnt2 = p2;m_pnt3 = p3;
	SetInited(true);
}

// 具体的创建显示对象
bool	kcPreviewTriangle::BuildGroupDisplay(const Handle(Graphic3d_Group)& hGroup)
{
	if(!HasInited())
		return false;

	SetPrimAspect();

	Handle(Graphic3d_ArrayOfSegments) aSegs = new Graphic3d_ArrayOfSegments(3,6);
	aSegs->AddVertex(m_pnt1.X(),m_pnt1.Y(),m_pnt1.Z());
	aSegs->AddVertex(m_pnt2.X(),m_pnt2.Y(),m_pnt2.Z());
	aSegs->AddVertex(m_pnt3.X(),m_pnt3.Y(),m_pnt3.Z());
	aSegs->AddEdge(1);aSegs->AddEdge(2);
	aSegs->AddEdge(2);aSegs->AddEdge(3);
	aSegs->AddEdge(3);aSegs->AddEdge(1);
	
	hGroup->AddPrimitiveArray(aSegs);

	return true;
}

//////////////////////////////////////////////////////////////////////////
// 矩形
kcPreviewRect::kcPreviewRect(const Handle_AIS_InteractiveContext& aCtx,kcDocContext *pDocCtx)
	:kcPreviewCurveObj(aCtx,pDocCtx)
{
	m_pWorkPlane = NULL;
}

kcPreviewRect::~kcPreviewRect()
{
	Destroy();
}

//
void	kcPreviewRect::Init(gp_Pnt& pnt1,kcBasePlane *pWP)
{
	ASSERT(pWP);
	if(!pWP) return;

	m_pWorkPlane = pWP;
	m_pnt1 = pnt1;

	SetInited(true);
}

void	kcPreviewRect::Update(gp_Pnt& pnt2)
{
	if(!m_pWorkPlane || !HasInited()) return;
	kPoint3 p1(m_pnt1.X(),m_pnt1.Y(),m_pnt1.Z()),p2(pnt2.X(),pnt2.Y(),pnt2.Z());
	double c[4][3];
	kvCoordSystem cs;
	if(!m_pWorkPlane->GetCoordSystem(cs))
		return;

	if(!kcg_CalcRectCorner(cs,p1,p2,c))
		return;

	int ix;
	for(ix = 0;ix < 4;ix ++)
		m_aPnt[ix].SetCoord(c[ix][0],c[ix][1],c[ix][2]);

	UpdateDisplay();
}

// 具体的创建显示对象
bool	kcPreviewRect::BuildGroupDisplay(const Handle(Graphic3d_Group)& hGroup)
{
	SetPrimAspect();

	Handle(Graphic3d_ArrayOfSegments) aSegs = new Graphic3d_ArrayOfSegments(4,8);
	int ix;
	for (ix = 1; ix <= 4; ix++) {
		aSegs->AddVertex(m_aPnt[ix - 1].X(), m_aPnt[ix - 1].Y(), m_aPnt[ix - 1].Z());
	}
	for(ix = 1;ix <= 4; ix ++){
		if (ix < 4)
			aSegs->AddEdges(ix, ix + 1);
		else
			aSegs->AddEdges(ix, 1);
	}

	hGroup->AddPrimitiveArray(aSegs);

	return true;
}

//////////////////////////////////////////////////////////////////////////
//
// box框架预览
kcPreviewBox::kcPreviewBox(const Handle_AIS_InteractiveContext& aCtx,kcDocContext *pDocCtx)
	:kcPreviewCurveObj(aCtx,pDocCtx)
{
	m_pWorkPlane = NULL;
}

kcPreviewBox::~kcPreviewBox()
{

}

//
bool	kcPreviewBox::Init(gp_Pnt& pnt1,gp_Pnt& pnt2,kcBasePlane *pWP)
{
	ASSERT(pWP);
	if(!pWP) return false;

	if(pnt1.Distance(pnt2) < KDBL_MIN)
		return false;

	m_pWorkPlane = pWP;
	m_pnt1 = pnt1;
	m_pnt2 = pnt2;

	SetInited(true);

	return true;
}


void	kcPreviewBox::Update(gp_Pnt& pnt3)
{
	if(!m_pWorkPlane || !HasInited()) return;

	kvCoordSystem cs;
	if(!m_pWorkPlane->GetCoordSystem(cs))
		return;

	kPoint3 p1(m_pnt1.X(),m_pnt1.Y(),m_pnt1.Z());
	kPoint3 p2(m_pnt2.X(),m_pnt2.Y(),m_pnt2.Z());
	kPoint3 p3(pnt3.X(),pnt3.Y(),pnt3.Z());

	if(!kcg_CalcBoxCorner(cs,p1,p2,p3,m_aPnt,m_aEdge))
		return;

	UpdateDisplay();
}

// 具体的创建显示对象
bool	kcPreviewBox::BuildGroupDisplay(const Handle(Graphic3d_Group)& hGroup)
{
	SetPrimAspect();

	Handle(Graphic3d_ArrayOfSegments) aArrSeg = new Graphic3d_ArrayOfSegments(8,24);
	int ix;
	for(ix = 0;ix < 8;ix ++)
		aArrSeg->AddVertex(m_aPnt[ix][0],m_aPnt[ix][1],m_aPnt[ix][2]);

	for(ix = 0;ix < 12;ix ++)
	{
		aArrSeg->AddEdge(m_aEdge[ix][0] + 1);
		aArrSeg->AddEdge(m_aEdge[ix][1] + 1);//下标都是从1开始。
	}

	//hGroup->BeginPrimitives();
	hGroup->AddPrimitiveArray(aArrSeg);
	//hGroup->EndPrimitives();

	return true;
}

//////////////////////////////////////////////////////////////////////////
//
// 圆的临时显示
kcPreviewCircle::kcPreviewCircle(const Handle_AIS_InteractiveContext& aCtx,kcDocContext *pDocCtx)
	:kcPreviewCurveObj(aCtx,pDocCtx)
{
	m_hCircle = NULL;
}

kcPreviewCircle::~kcPreviewCircle()
{
	Destroy();
}

//初始化
BOOL	kcPreviewCircle::Init(const Handle(Geom_Circle)& hCircle)
{
	if(_hStru.IsNull() || hCircle.IsNull())
		return FALSE;

	m_hCircle = hCircle;

	UpdateDisplay();

	SetInited(true);

	return TRUE;
}

void	kcPreviewCircle::Update(double radius)
{
	if(_hStru.IsNull() || m_hCircle.IsNull() || radius <= 0.0)
		return;

	m_hCircle->SetRadius(radius);

	UpdateDisplay();
}

// 具体的创建显示对象
bool	kcPreviewCircle::BuildGroupDisplay(const Handle(Graphic3d_Group)& hGroup)
{
	GCPnts_TangentialDeflection tdcur(GeomAdaptor_Curve(m_hCircle),0.01,0.01);
	Standard_Integer nbPoint = tdcur.NbPoints();

	Handle(Graphic3d_ArrayOfPolylines) aPoly = new Graphic3d_ArrayOfPolylines(nbPoint);
	int ix;
	for(ix = 1;ix <= nbPoint;ix ++)
	{
		gp_Pnt P = tdcur.Value(ix);
		aPoly->AddVertex(P.X(),P.Y(),P.Z());
	}

	SetPrimAspect();

	hGroup->AddPrimitiveArray(aPoly);

	return true;
}

//清除
void	kcPreviewCircle::Clear()
{
	kcPreviewObj::Clear();
	if(!m_hCircle.IsNull())
	{
		m_hCircle.Nullify();
	}
}

void	kcPreviewCircle::Destroy()
{
	kcPreviewObj::Destroy();
	if(!m_hCircle.IsNull())
	{
		m_hCircle.Nullify();
	}
}

//////////////////////////////////////////////////////////////////////////
//
// 圆弧的临时显示
kcPreviewArc::kcPreviewArc(const Handle_AIS_InteractiveContext& aCtx,kcDocContext *pDocCtx)
	:kcPreviewCurveObj(aCtx,pDocCtx)
{
	m_aCircle = NULL;
}

kcPreviewArc::~kcPreviewArc()
{
	Destroy();
}

//初始化
BOOL	kcPreviewArc::Init(const Handle(Geom_Circle)& aCircle)
{
	if(_hStru.IsNull() || aCircle.IsNull())
		return FALSE;

	m_aCircle = aCircle;
	SetInited(true);

	return TRUE;
}

void	kcPreviewArc::Update(double ang1,double ang2)
{
	if(_hStru.IsNull() || m_aCircle.IsNull() || !HasInited())
		return;

	//double pi2 = K_PI * 2,tmp;
	//while(ang1 < 0.0) ang1 += pi2;
	//while(ang1 >= pi2) ang1 -= pi2;
	//while(ang2 < 0.0) ang2 += pi2;
	//while(ang2 >= pi2) ang2 -= pi2;
	//if(ang1 > ang2)
	//{
	//	tmp = ang1;ang1 = ang2;ang2 = tmp;
	//}
	if(fabs(ang1 - ang2) < KDBL_MIN)
		return;

	Handle(Geom_Circle) aCpyCirc = Handle(Geom_Circle)::DownCast(m_aCircle->Copy());
	if(aCpyCirc.IsNull())
		return;

	try{
		m_aArc = new Geom_TrimmedCurve(aCpyCirc,ang1,ang2);
	}catch(Standard_Failure){
		return;
	}
	if(m_aArc.IsNull())
		return;

	UpdateDisplay();
}

//清除
void	kcPreviewArc::Clear()
{
	kcPreviewObj::Clear();
	if(!m_aCircle.IsNull())
	{
		m_aCircle.Nullify();
	}
	if(!m_aArc.IsNull())
		m_aArc.Nullify();
}

void	kcPreviewArc::Destroy()
{
	kcPreviewCurveObj::Destroy();
	if(!m_aCircle.IsNull())
	{
		m_aCircle.Nullify();
	}
	if(!m_aArc.IsNull())
		m_aArc.Nullify();
}

// 具体的创建显示对象
bool	kcPreviewArc::BuildGroupDisplay(const Handle(Graphic3d_Group)& hGroup)
{
	if(m_aArc.IsNull())
		return false;

	GCPnts_TangentialDeflection tdcur(GeomAdaptor_Curve(m_aArc),0.01,0.01);
	Standard_Integer nbPoint = tdcur.NbPoints();

	Handle(Graphic3d_ArrayOfPolylines) aPolyLine = new Graphic3d_ArrayOfPolylines(nbPoint);
	int ix;
	for(ix = 1;ix <= nbPoint;ix ++)
	{
		gp_Pnt P = tdcur.Value(ix);
		aPolyLine->AddVertex(P.X(),P.Y(),P.Z());
	}

	SetPrimAspect();

	hGroup->AddPrimitiveArray(aPolyLine);

	return true;
}

//////////////////////////////////////////////////////////////////////////

// 曲线的临时显示
kcPreviewCurve::kcPreviewCurve(const Handle_AIS_InteractiveContext& aCtx,kcDocContext *pDocCtx)
	:kcPreviewCurveObj(aCtx,pDocCtx)
{

}

kcPreviewCurve::~kcPreviewCurve()
{
	Destroy();
}

void  kcPreviewCurve::Update(const Handle(Geom_Curve)& aCurve)
{
	if(_hStru.IsNull() || aCurve.IsNull())
		return;

	m_aCurve = aCurve;

	UpdateDisplay();

	SetInited(true);
}

void  kcPreviewCurve::Update(const TopoDS_Edge& aEdge)
{
	if(_hStru.IsNull() || aEdge.IsNull())
		return;

	double df,dl;
	Handle(Geom_Curve) aCrv = BRep_Tool::Curve(aEdge,df,dl);
	if(aCrv.IsNull())
		return;

	Handle(Geom_Curve) aCCrv = Handle(Geom_Curve)::DownCast(aCrv->Copy());
	if(aCCrv.IsNull())
		return;

	Handle(Geom_TrimmedCurve) aTrmCrv = new Geom_TrimmedCurve(aCCrv,df,dl);
	if(aTrmCrv.IsNull())
		return;

	m_aCurve = aTrmCrv;
	UpdateDisplay();

	SetInited(true);
}

//清除
void	kcPreviewCurve::Clear()
{
	kcPreviewObj::Clear();
	if(!m_aCurve.IsNull())
	{
		m_aCurve.Nullify();
	}
}

void	kcPreviewCurve::Destroy()
{
	if(!m_aCurve.IsNull()){
		m_aCurve.Nullify();
	}

	kcPreviewObj::Destroy();
}

// 具体的创建显示对象
bool  kcPreviewCurve::BuildGroupDisplay(const Handle(Graphic3d_Group)& hGroup)
{
	GCPnts_TangentialDeflection tdcur(GeomAdaptor_Curve(m_aCurve),0.01,0.01);
	Standard_Integer nbPoint = tdcur.NbPoints();

	Handle(Graphic3d_ArrayOfPolylines) aPolyLine = new Graphic3d_ArrayOfPolylines(nbPoint);
	int ix;
	for(ix = 1;ix <= nbPoint;ix ++){
		gp_Pnt P = tdcur.Value(ix);
		aPolyLine->AddVertex(P.X(),P.Y(),P.Z());
	}

	SetPrimAspect();
	hGroup->AddPrimitiveArray(aPolyLine);

	return true;
}

//////////////////////////////////////////////////////////////////////////
//
// 多义线的临时显示
kcPreviewPolyline::kcPreviewPolyline(const Handle_AIS_InteractiveContext& aCtx,kcDocContext *pDocCtx)
	:kcPreviewCurveObj(aCtx,pDocCtx)
{

}

kcPreviewPolyline::~kcPreviewPolyline()
{

}

void  kcPreviewPolyline::AddPoint(const kPoint3& p)
{
	m_pointArray.push_back(p);
	UpdateDisplay();
}

//清除
void  kcPreviewPolyline::Clear()
{
	m_pointArray.clear();
	kcPreviewCurveObj::Clear();
}

void  kcPreviewPolyline::Destroy()
{
	kcPreviewCurveObj::Destroy();
}

// 具体的创建显示对象
bool  kcPreviewPolyline::BuildGroupDisplay(const Handle(Graphic3d_Group)& hGroup)
{
	if(m_pointArray.size() < 2)
		return false;

	kPoint3 pnt;
	int ix,nbPoint = (int)m_pointArray.size();
	Handle(Graphic3d_ArrayOfPolylines) aPolyLine = new Graphic3d_ArrayOfPolylines(nbPoint);
	for(ix = 1;ix <= nbPoint;ix ++){
		pnt = m_pointArray[ix - 1];
		aPolyLine->AddVertex(pnt[0],pnt[1],pnt[2]);
	}

	SetPrimAspect();
	hGroup->AddPrimitiveArray(aPolyLine);

	return true;
}