#include "StdAfx.h"
#include <GeomAPI_ExtremaCurveCurve.hxx>
#include <ShapeFix_Wire.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <ShapeFix_Edge.hxx>
#include <BRepCheck_Wire.hxx>
#include <ShapeAnalysis_Edge.hxx>
#include <GeomAPI_Interpolate.hxx>
#include <TColgp_HArray1OfPnt.hxx>
#include <TColStd_HArray1OfReal.hxx>
#include <TColgp_Array1OfVec.hxx>
#include <TColStd_HArray1OfBoolean.hxx>
#include <GCPnts_AbscissaPoint.hxx>
#include <GCE2d_MakeSegment.hxx>
#include <GC_MakeSegment.hxx>
#include "QxBRepLib.h"
#include "QxCurveLib.h"

Handle(Geom_Curve) QxCurveLib::MakeSegment(const gp_Pnt &aP1,const gp_Pnt &aP2)
{
	Handle(Geom_Curve) aCurve;
	GC_MakeSegment mkSeg(aP1,aP2);
	if(mkSeg.IsDone()){
		aCurve = mkSeg.Value();
	}
	return aCurve;
}

Handle(Geom_Curve) QxCurveLib::MakeSegment(double p1[3],double p2[3])
{
	return MakeSegment(gp_Pnt(p1[0],p1[1],p1[2]),gp_Pnt(p2[0],p2[1],p2[2]));
}

double	QxCurveLib::CurveLength(const Handle(Geom_Curve)& aCrv,double t0,double t1)
{
	GeomAdaptor_Curve AC(aCrv);
	return GCPnts_AbscissaPoint::Length(AC,t0,t1);
}

// 计算两条直线间的最近距离及其对应点。
//
bool	QxCurveLib::CalcLineLineExtrema(const kPoint3& org1,const kVector3& dir1,
				 					  const kPoint3& org2,const kVector3& dir2,
									  kPoint3& rp1,kPoint3& rp2,double& dist)
{
	Handle(Geom_Line) aLin1 = new Geom_Line(gp_Pnt(org1[0],org1[1],org1[2]),gp_Dir(dir1[0],dir1[1],dir1[2]));
	Handle(Geom_Line) aLin2 = new Geom_Line(gp_Pnt(org2[0],org2[1],org2[2]),gp_Dir(dir2[0],dir2[1],dir2[2]));
	GeomAPI_ExtremaCurveCurve exc(aLin1,aLin2);
	if(exc.NbExtrema() <= 0)
		return false;

	gp_Pnt pnt1,pnt2;
	exc.NearestPoints(pnt1,pnt2);
	dist = exc.TotalLowerDistance();
	rp1.set(pnt1.X(),pnt1.Y(),pnt1.Z());
	rp2.set(pnt2.X(),pnt2.Y(),pnt2.Z());

	return true;
}

// 点到直线的投影。t为返回参数。
bool	QxCurveLib::ProjectPointToLine(const kPoint3& org,const kVector3& dir,const kPoint3& point,
									   kPoint3& prjpoint,double& t)
{
	Handle(Geom_Line) aLine = new Geom_Line(gp_Pnt(org[0],org[1],org[2]),gp_Dir(dir[0],dir[1],dir[2]));
	GeomAPI_ProjectPointOnCurve ppoc(gp_Pnt(point[0],point[1],point[2]),aLine);
	if(ppoc.NbPoints() != 1)
		return false;

	gp_Pnt nrsPnt = ppoc.Point(1);
	t = ppoc.Parameter(1);
	prjpoint.set(nrsPnt.X(),nrsPnt.Y(),nrsPnt.Z());

	return true;
}

bool	QxCurveLib::ProjectPointToLine(const kPoint3& org,const kVector3& dir,const kPoint3& point,
									   kPoint3& prjpoint)
{
	kVector3 v(org,point);
	double d = v * dir;
	prjpoint = org + dir * d;
	return true;
}

Handle(Geom_Circle)	QxCurveLib::MakeCircle(const kPoint3& aCenter,const kPoint3& aXPoint,
										   const kVector3& Z,double radius)
{
	Handle(Geom_Circle) aCirc;
	if(radius < 1e-6)
		return aCirc;

	kVector3 X = aXPoint - aCenter;
	if(!X.normalize())
		return aCirc;

	kVector3 Y = Z ^ X;
	if(!Y.normalize())
		return aCirc;

	gp_Ax2 axis(gp_Pnt(aCenter.x(),aCenter.y(),aCenter.z()),
		gp_Dir(gp_Vec(Z[0],Z[1],Z[2])),
		gp_Dir(gp_Vec(X[0],X[1],X[2])));
	aCirc = new Geom_Circle(axis,radius);

	return aCirc;
}

bool	QxCurveLib::CalcTangent(const TopoDS_Edge& aE,const TopoDS_Face& aFace,double t,bool bTopo,kPoint3& p,kVector3& tang)
{
	if(aE.IsNull() || aFace.IsNull())
		return false;

	BRepAdaptor_Curve aCur;
	aCur.Initialize(aE,aFace);
	gp_Pnt pnt;
	gp_Vec du;
	aCur.D1(t,pnt,du);

	if(bTopo){
		if(aE.Orientation() == TopAbs_REVERSED)
			du.Reverse();
	}

	p.set(pnt.X(),pnt.Y(),pnt.Z());
	tang.set(du.X(),du.Y(),du.Z());

	return true;
}

bool	QxCurveLib::CalcTangent(const TopoDS_Edge& aE,const TopoDS_Face& aFace,bool bTopo,kPoint3& p,kVector3& tang)
{
	if(aE.IsNull() || aFace.IsNull())
		return false;

	BRepAdaptor_Curve aCur;
	aCur.Initialize(aE,aFace);
	double t = (aCur.FirstParameter() + aCur.LastParameter()) / 2.0;
	return QxCurveLib::CalcTangent(aE,aFace,t,bTopo,p,tang);
}

// 从face的边界edge创建新的edge
TopoDS_Edge		QxCurveLib::BuildBoundEdge(const TopoDS_Edge& aEdge,const TopoDS_Face& aFace)
{
	TopoDS_Edge aNE;
	if(aEdge.IsNull() || aFace.IsNull())
		return aNE;

	double df,dl;
	Handle(Geom_Curve) aCrv = BRep_Tool::Curve(aEdge,df,dl);
	if(aCrv.IsNull())
	{
		//没有3d曲线，生成
		ShapeFix_Edge fixEdge;
		fixEdge.FixAddCurve3d(aEdge);

		aCrv = BRep_Tool::Curve(aEdge,df,dl);
	}
	if(aCrv.IsNull())
		return aNE;

	Handle(Geom_Curve) aCCrv = Handle(Geom_Curve)::DownCast(aCrv->Copy());
	
	try
	{
		BRepBuilderAPI_MakeEdge me(aCCrv,df,dl);
		if(me.IsDone()){
			aNE = me.Edge();
			//考虑上edge的方向
			aNE.Orientation(aEdge.Orientation());
		}
	}
	catch (Standard_Failure){}

	return aNE;
}

//创建edge的方向标记
TopoDS_Edge	QxCurveLib::BuildEdgeDirFlag(const TopoDS_Edge& aEdge,const TopoDS_Face& aFace,double radius)
{
	TopoDS_Edge aNE;
	if(aEdge.IsNull() || aFace.IsNull())
		return aNE;

	Handle(Geom_Surface) aSurf = BRep_Tool::Surface(aFace);
	if(aSurf.IsNull())
		return aNE;

	double df,dl,t,rat = 0.2;
	Handle(Geom_Curve) aCrv = BRep_Tool::Curve(aEdge,df,dl);
	if(aCrv.IsNull())
		return aNE;

	double pf,pl,pt;
	Handle(Geom2d_Curve) aCrv2d = BRep_Tool::CurveOnSurface(aEdge,aFace,pf,pl);
	if(aCrv2d.IsNull())
		return aNE;

	if(aEdge.Orientation() == TopAbs_REVERSED){
		rat = 0.8;
	}
	t = df + rat * (dl - df);
	pt = pf + rat * (pl - pf);

	gp_Pnt pnt,p2;
	gp_Pnt2d pnt2d;
	gp_Vec Dt,D1U,D1V,DN;
	aCrv->D1(t,pnt,Dt);
	aCrv2d->D0(pt,pnt2d);
	aSurf->D1(pnt2d.X(),pnt2d.Y(),p2,D1U,D1V);
	DN = D1U.Crossed(D1V);

	if(radius < 0.0){
		double len = QxCurveLib::CurveLength(aCrv,df,dl);
		radius = len * 0.02;
	}

	gp_Ax2 ax2(pnt,gp_Dir(DN),gp_Dir(Dt));
	Handle(Geom_Circle) aCirc = new Geom_Circle(ax2,radius);
	
	try
	{
		BRepBuilderAPI_MakeEdge me(aCirc);
		if(me.IsDone()){
			aNE = me.Edge();
		}
	}
	catch (Standard_Failure){}

	return aNE;
}

TopoDS_Wire	QxCurveLib::BuildBoundWire(const TopoDS_Wire& aWire,const TopoDS_Face& aFace)
{
	TopoDS_Wire aNW;
	if(aWire.IsNull() || aFace.IsNull())
		return aNW;

	try{
		BRepBuilderAPI_MakeWire mw;
		TopoDS_Edge aEdge,aNE;
		BRepTools_WireExplorer wex;
		for(wex.Init(aWire,aFace);wex.More();wex.Next())
		{
			aEdge = wex.Current();
			if(BRep_Tool::Degenerated(aEdge))//退化edge，需要特别注意
				continue;

			aNE = QxCurveLib::BuildBoundEdge(aEdge,aFace);
			if(aNE.IsNull())
				return aNW;

			mw.Add(aNE);
		}
		mw.Build();
		if(mw.IsDone())
			aNW = mw.Wire();
	}catch(Standard_Failure){

	}
	
	return aNW;
}

BOOL	QxCurveLib::IsWireClosed(const TopoDS_Wire& aWire)
{
	ASSERT(!aWire.IsNull());
	BOOL bClosed = TRUE;
	if(aWire.IsNull())
		return FALSE;

	try{
		BRepCheck_Wire chkWire(aWire);
		if(BRepCheck_NoError != chkWire.Closed())
			bClosed = FALSE;//not close
	}catch(Standard_Failure){}
	
	return bClosed;
}

BOOL	QxCurveLib::IsEdgeClosed(const TopoDS_Edge& aEdge)
{
	if(aEdge.IsNull())
		return FALSE;

	BOOL bClosed = FALSE;
	try
	{
		ShapeAnalysis_Edge sae;
		if(sae.IsClosed3d(aEdge))
			bClosed = TRUE;
	}catch (Standard_Failure){}
	
	return bClosed;
}

// 是否是直线
bool QxCurveLib::IsLine(const Handle(Geom_Curve) &aCurve)
{
	if(!aCurve.IsNull()){
		Handle(Geom_Curve) aBasCrv = aCurve;
		if(aCurve->IsKind(STANDARD_TYPE(Geom_TrimmedCurve))){
			const Handle(Geom_TrimmedCurve) &aTrmCrv = Handle(Geom_TrimmedCurve)::DownCast(aCurve);
			aBasCrv = aTrmCrv->BasisCurve();
		}else if(aCurve->IsKind(STANDARD_TYPE(Geom_OffsetCurve))){
			const Handle(Geom_OffsetCurve) &aOffCrv = Handle(Geom_OffsetCurve)::DownCast(aCurve);
			aBasCrv = aOffCrv->BasisCurve();
		}
		if(aBasCrv->IsKind(STANDARD_TYPE(Geom_Line))){
			return true;
		}
	}
	return false;
}

Handle(Geom_Curve) QxCurveLib::GetCurveCopy(const TopoDS_Edge& aEdge)
{
	Handle(Geom_Curve) aNCrv,aCCrv;
	if(aEdge.IsNull())
		return aNCrv;

	try{
		double df,dl;
		Handle(Geom_Curve) aCrv = BRep_Tool::Curve(aEdge,df,dl);
		if(aCrv.IsNull())
			return aNCrv;

		aCCrv = Handle(Geom_Curve)::DownCast(aCrv->Copy());
		if(aCCrv.IsNull())
			return aNCrv;

		aNCrv = new Geom_TrimmedCurve(aCCrv,df,dl);
	}catch(Standard_Failure){}

	return aNCrv;
}

///////////////////////////////////////////////////////////////////////////
//
Handle(Geom2d_Curve) kcgCurve2DLib::MakeSegLine(const gp_Pnt2d &p1,const gp_Pnt2d &p2)
{
	Handle(Geom2d_Curve) aSeg;
	try{
		GCE2d_MakeSegment mkSeg(p1,p2);
		if(mkSeg.IsDone()){
			aSeg = mkSeg.Value();
		}
	}catch(Standard_Failure){
	}
	return aSeg;
}

//////////////////////////////////////////////////////////////////////////
//
// 将多个曲线组成一个wire
kcgSewCurve::kcgSewCurve()
{
	m_bDone = FALSE;
}

kcgSewCurve::~kcgSewCurve()
{
	if(!m_wireData.IsNull())
	{
		m_wireData.Nullify();
	}
}

bool	kcgSewCurve::Init()
{
	m_bDone = FALSE;

	if(m_wireData.IsNull())
		m_wireData = new ShapeExtend_WireData;

	if(m_wireData.IsNull())
		return false;

	m_wireData->Clear();

	return true;
}

bool	kcgSewCurve::AddCurve(const Handle(Geom_Curve)& aCurve)
{
	if(m_wireData.IsNull()) return false;
	if(aCurve.IsNull()) return false;

	//是否有限的曲线
	double t0,t1;
	t0 = aCurve->FirstParameter();
	t1 = aCurve->LastParameter();
	if(Precision::IsInfinite(t0) || Precision::IsInfinite(t1))
		return false;
	
	//创建Edge
	TopoDS_Edge aEdge;
	try
	{
		//拷贝一份，避免修改旧的曲线
		Handle(Geom_Curve) aCpyCur = Handle(Geom_Curve)::DownCast(aCurve->Copy());
		BRepBuilderAPI_MakeEdge me(aCpyCur);
		if(!me.IsDone())
			return false;

		aEdge = me.Edge();
		if(aEdge.IsNull())
			return false;
	}
	catch (Standard_Failure){
		return false;
	}

	m_wireData->Add(aEdge);

	return true;
}

bool	kcgSewCurve::AddEdge(const TopoDS_Edge& aEdge)
{
	if(m_wireData.IsNull()) return false;
	if(aEdge.IsNull()) return false;

	try{
		TopoDS_Edge aCopyEdge = QxBRepLib::CopyEdge(aEdge);
		if(aCopyEdge.IsNull())
			return false;

		m_wireData->Add(aCopyEdge);
	}catch(Standard_Failure){
		return false;
	}

	return true;
}

bool	kcgSewCurve::AddWire(const TopoDS_Wire& aWire)
{
	if(m_wireData.IsNull()) return false;
	if(aWire.IsNull()) return false;

	//要都拷贝一份
	TopoDS_Iterator ite(aWire);
	for(;ite.More();ite.Next())
	{
		const TopoDS_Edge& aE = TopoDS::Edge(ite.Value());
		AddEdge(aE);
	}

	return true;
}

bool	kcgSewCurve::Sew(bool bClosed)
{
	m_bDone = false;
	if(m_wireData.IsNull()) return false;
	
	//
	try
	{
		ShapeFix_Wire wireFix;
		wireFix.Load(m_wireData);

		if(bClosed)
			wireFix.ClosedWireMode() = TRUE;
		else
			wireFix.ClosedWireMode() = FALSE;

		if(!wireFix.Perform())
			return false;

		TopoDS_Wire aWire = wireFix.WireAPIMake();
		if(aWire.IsNull())
			return false;

		m_aWire = aWire;
		m_bDone = true;
	}
	catch (Standard_Failure){
		return false;
	}

	return true;
}

//清除,重新初始化。
void	kcgSewCurve::Clear()
{
	m_bDone = false;
	if(!m_wireData.IsNull())
		m_wireData->Clear();
}

TopoDS_Wire		kcgSewCurve::Wire() const//返回结果
{
	return m_aWire;
}

//////////////////////////////////////////////////////////////////////////
//
// 混接两条曲线
kcgBlendCurve::kcgBlendCurve()
{

}

kcgBlendCurve::~kcgBlendCurve()
{

}

// 传入两条edge，点击参数t，和连续性约束。
// 返回生成的edge。
TopoDS_Edge	kcgBlendCurve::Blend(const TopoDS_Edge& aFEdge,double t1,int nCont1,const TopoDS_Edge& aLEdge,double t2,int nCont2)
{
	ASSERT(!aFEdge.IsNull() && !aLEdge.IsNull());
	ASSERT(nCont1 >= 0 && nCont1 <= 2);
	ASSERT(nCont2 >= 0 && nCont2 <= 2);
	TopoDS_Edge aEdge;
	if(aFEdge.IsNull() || aLEdge.IsNull() || 
	   nCont1 < 0 || nCont1 > 2 || nCont2 < 0 || nCont2 > 2)
	   return aEdge;

	double df1,dl1,df2,dl2,u1,u2;
	Handle(Geom_Curve) aCrv1 = BRep_Tool::Curve(aFEdge,df1,dl1);
	Handle(Geom_Curve) aCrv2 = BRep_Tool::Curve(aLEdge,df2,dl2);
	if(aCrv1.IsNull() || aCrv2.IsNull())
		return aEdge;

	//保证第一条曲线尾对第二条曲线的首，这样方便计算
	bool bStart = true;
	if(t1 > (df1 + dl1) / 2)
		bStart = false;
	if(bStart)
	{
		u1 = aCrv1->ReversedParameter(df1);//
		aCrv1 = aCrv1->Reversed();
	}
	else
		u1 = dl1;

	bStart = true;
	if(t2 > (df2 + dl2) / 2)
		bStart = false;

	if(!bStart)
	{
		u2 = aCrv2->ReversedParameter(dl2);
		aCrv2 = aCrv2->Reversed();
	}
	else
		u2 = df2;

	//计算首尾的相关信息
	gp_Pnt p1,p2;
	gp_Vec du1,du2,duu1,duu2;
	if(nCont1 == 0)
		aCrv1->D0(u1,p1);
	else if(nCont1 == 1)
		aCrv1->D1(u1,p1,du1);
	else
		aCrv1->D2(u1,p1,du1,duu1);

	if(nCont2 == 0)
		aCrv2->D0(u2,p2);
	else if(nCont2 == 1)
		aCrv2->D1(u2,p2,du2);
	else
		aCrv2->D2(u2,p2,du2,duu2);

	// 进行插值
	if(nCont1 < 2 && nCont2 < 2)
	{
		try{
			Handle(TColgp_HArray1OfPnt) aPntArray = new TColgp_HArray1OfPnt(1,2);
			aPntArray->SetValue(1,p1);
			aPntArray->SetValue(2,p2);

			Handle(TColStd_HArray1OfReal) aParArray = new TColStd_HArray1OfReal(1,2);
			aParArray->SetValue(1,0.0);
			aParArray->SetValue(2,1.0);

			TColgp_Array1OfVec aTanArray(1,2);
			Handle(TColStd_HArray1OfBoolean) aFlg = new TColStd_HArray1OfBoolean(1,2,Standard_False);
			if(nCont1 == 1)
			{
				aTanArray.SetValue(1,du1);
				aFlg->SetValue(1,Standard_True);
			}
			if(nCont2 == 1)
			{
				aTanArray.SetValue(2,du2);
				aFlg->SetValue(2,Standard_True);
			}

			GeomAPI_Interpolate interp(aPntArray,aParArray,Standard_False,1e-6);
			interp.Load(aTanArray,aFlg);
			interp.Perform();
			if(interp.IsDone())
			{
				Handle(Geom_BSplineCurve) aNCrv = interp.Curve();
				if(!aNCrv.IsNull())
				{
					BRepBuilderAPI_MakeEdge me(aNCrv);
					if(me.IsDone())
						aEdge = me.Edge();
				}
			}
		}catch(Standard_Failure){}
	}
	else
	{

	}

	return aEdge;
}
