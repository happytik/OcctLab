#include "StdAfx.h"
#include <BRepExtrema_ExtCF.hxx>
#include "QxBaseUtils.h"
#include "kcgIntersect.h"

//////////////////////////////////////////////////////////////////////////
//
kcxCurveIntpt::kcxCurveIntpt()
{
	_widx = -1;
	_t = 0.0;
	_dist = 0.0;
}

kcxCurveIntpt::kcxCurveIntpt(const kcxCurveIntpt& other)
{
	*this = other;
}

kcxCurveIntpt&	kcxCurveIntpt::operator=(const kcxCurveIntpt& other)
{
	_widx = other._widx;
	_t = other._t;
	_p = other._p;
	_dist = other._dist;

	return *this;
}

//////////////////////////////////////////////////////////////////////////
//
kcxCCIntpt::kcxCCIntpt()
{
}
kcxCCIntpt::kcxCCIntpt(const kcxCCIntpt& other)
{
	*this = other;
}

kcxCCIntpt&		kcxCCIntpt::operator=(const kcxCCIntpt& other)
{
	_ipt1 = other._ipt1;
	_ipt2 = other._ipt2;

	return *this;
}

//////////////////////////////////////////////////////////////////////////
//
kcxCSIntpt::kcxCSIntpt()
{
	_u = _v = 0.0;
}

kcxCSIntpt::kcxCSIntpt(const kcxCSIntpt& other)
{
	*this = other;
}

kcxCSIntpt&		kcxCSIntpt::operator=(const kcxCSIntpt& other)
{
	_cvipt = other._cvipt;
	_u = other._u;
	_v = other._v;
	_p = other._p;

	return *this;
}

//////////////////////////////////////////////////////////////////////////
// 点和edge或wire的交点
kcgPointCurveInter::kcgPointCurveInter()
{
}

bool	kcgPointCurveInter::Intersect(const kPoint3& p,const TopoDS_Edge& aEdge,double tol)
{
	_aIntpt.clear();
	DoIntersect(p,aEdge,-1,tol);
	if(!_aIntpt.empty())
		return true;
	return false;
}

bool	kcgPointCurveInter::Intersect(const kPoint3& p,const TopoDS_Wire& aWire,double tol)
{
	int ix = 0;
	_aIntpt.clear();

	TopoDS_Iterator ite(aWire);
	for(;ite.More();ite.Next())
	{
		TopoDS_Edge aEdge = TopoDS::Edge(ite.Value());
		DoIntersect(p,aEdge,ix,tol);
		ix ++;
	}

	if(!_aIntpt.empty())
		return true;
	return false;
}

void	kcgPointCurveInter::DoIntersect(const kPoint3& p,const TopoDS_Edge& aEdge,int widx,double tol)
{
	if(aEdge.IsNull())
		return;

	double t0,t1;
	Handle(Geom_Curve) aCurve = BRep_Tool::Curve(aEdge,t0,t1);
	if(aCurve.IsNull())
		return;

	//正常求交
	try{
		GeomAPI_ProjectPointOnCurve ppc(gp_Pnt(p[0],p[1],p[2]),aCurve,t0,t1);
		if(ppc.NbPoints() > 0)
		{
			if(ppc.LowerDistance() < tol)
			{
				kcxCurveIntpt ipt;
				ipt._widx = widx;
				ipt._t = ppc.LowerDistanceParameter();
				gp_Pnt pnt = ppc.NearestPoint();
				ipt._p.set(pnt.X(),pnt.Y(),pnt.Z());

				_aIntpt.push_back(ipt);
			}
		}
	}catch(Standard_Failure){
		return;
	}
}

int		kcgPointCurveInter::NbInterPoints() const
{
	return (int)_aIntpt.size();
}

kcxCurveIntpt	kcgPointCurveInter::InterPoint(int ix)
{
	ASSERT(ix >= 0 && ix < NbInterPoints());
	return _aIntpt[ix];
}

//////////////////////////////////////////////////////////////////////////
// 曲线和曲线的交点
kcgCurveCurveInter::kcgCurveCurveInter()
{

}

//
bool	kcgCurveCurveInter::Intersect(const TopoDS_Edge& aEdge1,const TopoDS_Edge& aEdge2,double tol)
{
	_aIntpt.clear();
	DoIntersect(-1,aEdge1,-1,aEdge2,tol);
	if(!_aIntpt.empty())
		return true;
	return false;
}

bool	kcgCurveCurveInter::Intersect(const TopoDS_Edge& aEdge,const TopoDS_Wire& aWire,double tol)
{
	if(aEdge.IsNull() || aWire.IsNull())
		return false;

	_aIntpt.clear();
	int wix = 0;
	TopoDS_Iterator ite(aWire);
	for(;ite.More();ite.Next())
	{
		TopoDS_Edge aSubEdge = TopoDS::Edge(ite.Value());
		DoIntersect(-1,aEdge,wix,aSubEdge,tol);
		wix ++;
	}

	if(!_aIntpt.empty())
		return true;
	return false;
}

bool	kcgCurveCurveInter::Intersect(const TopoDS_Wire& aWire1,const TopoDS_Wire& aWire2,double tol)
{
	if(aWire1.IsNull() || aWire2.IsNull())
		return false;

	_aIntpt.clear();

	int wix1 = 0,wix2 = 0;
	TopoDS_Iterator ite(aWire1);
	for(;ite.More();ite.Next())
	{
		TopoDS_Edge aSubEdge1 = TopoDS::Edge(ite.Value());
		TopoDS_Iterator ite2(aWire2);
		wix2 = 0;
		for(;ite2.More();ite2.Next())
		{
			TopoDS_Edge aSubEdge2 = TopoDS::Edge(ite2.Value());
			DoIntersect(wix1,aSubEdge1,wix2,aSubEdge2,tol);
			wix2 ++;
		}
		wix1 ++;
	}

	if(!_aIntpt.empty())
		return true;
	return false;
}

void	kcgCurveCurveInter::DoIntersect(int widx1,const TopoDS_Edge& aEdge1,int widx2,const TopoDS_Edge& aEdge2,double tol)
{
	if(aEdge1.IsNull() || aEdge2.IsNull())
		return;

	double t00,t01,t10,t11;
	Handle(Geom_Curve) aCurve1 = BRep_Tool::Curve(aEdge1,t00,t01);
	Handle(Geom_Curve) aCurve2 = BRep_Tool::Curve(aEdge2,t10,t11);
	if(aCurve1.IsNull() || aCurve2.IsNull())
		return;

	try{
		GeomAPI_ExtremaCurveCurve ecc;
		ecc.Init(aCurve1,aCurve2,t00,t01,t10,t11);
		int ix;
		double dist;
		for(ix = 1;ix <= ecc.NbExtrema();ix ++)
		{
			dist = ecc.Distance(ix);
			if(dist < tol)
			{//one interpnt
				kcxCCIntpt ipt;
				double t1,t2;
				gp_Pnt p1,p2;
				ecc.Parameters(ix,t1,t2);
				ecc.Points(ix,p1,p2);
				ipt._ipt1._widx = widx1;
				ipt._ipt1._t = t1;
				ipt._ipt1._p.set(p1.X(),p1.Y(),p1.Z());
				ipt._ipt1._dist = dist;
				ipt._ipt2._widx = widx2;
				ipt._ipt2._t = t2;
				ipt._ipt2._p.set(p2.X(),p2.Y(),p2.Z());
				ipt._ipt2._dist = dist;

				_aIntpt.push_back(ipt);
			}
		}

	}catch (Standard_Failure){
		
	}
}

int		kcgCurveCurveInter::NbInterPoints() const
{
	return (int)_aIntpt.size();
}

kcxCCIntpt	kcgCurveCurveInter::InterPoint(int ix)
{
	ASSERT(ix >= 0 && ix < NbInterPoints());
	return _aIntpt[ix];
}

//////////////////////////////////////////////////////////////////////////
// 曲线和曲面的交点
kcgCurveSurfaceInter::kcgCurveSurfaceInter()
{

}

bool	kcgCurveSurfaceInter::Intersect(const TopoDS_Edge& aEdge,const TopoDS_Face& aFace,double tol)
{
	if(aEdge.IsNull() || aFace.IsNull())
		return false;

	_aIntpt.clear();
	DoIntersect(-1,aEdge,aFace,tol);
	if(_aIntpt.empty())
		return false;

	return true;
}

bool	kcgCurveSurfaceInter::Intersect(const TopoDS_Wire& aWire,const TopoDS_Face& aFace,double tol)
{
	if(aWire.IsNull() || aFace.IsNull())
		return false;

	_aIntpt.clear();
	int widx = 0;
	TopoDS_Iterator ite(aWire);
	for(;ite.More();ite.Next())
	{
		TopoDS_Edge aEdge = TopoDS::Edge(ite.Value());
		DoIntersect(widx,aEdge,aFace,tol);
		widx ++;
	}
	if(_aIntpt.empty())
		return false;

	return true;
}

void	kcgCurveSurfaceInter::DoIntersect(int widx,const TopoDS_Edge& aEdge,const TopoDS_Face& aFace,double tol)
{
	try{
		BRepExtrema_ExtCF ecf(aEdge,aFace);
		if(ecf.IsDone())
		{
			double dist;
			int ix,nbp = ecf.NbExt();
			for(ix = 1;ix <= nbp;ix ++)
			{
				dist = ecf.SquareDistance(ix);
				if(dist < tol)
				{
					kcxCSIntpt ipt;
					gp_Pnt pnt;
					ipt._cvipt._widx = widx;
					ipt._cvipt._t = ecf.ParameterOnEdge(ix);
					ecf.ParameterOnFace(ix,ipt._u,ipt._v);
					pnt = ecf.PointOnEdge(ix);
					ipt._cvipt._p.set(pnt.X(),pnt.Y(),pnt.Z());
					pnt = ecf.PointOnFace(ix);
					ipt._p.set(pnt.X(),pnt.Y(),pnt.Z());

					_aIntpt.push_back(ipt);
				}
			}
		}
	}catch (Standard_Failure){
		
	}
}

int		kcgCurveSurfaceInter::NbInterPoints() const
{
	return (int)_aIntpt.size();
}

kcxCSIntpt	kcgCurveSurfaceInter::InterPoint(int ix)
{
	ASSERT(ix >= 0 && ix < NbInterPoints());
	return _aIntpt[ix];
}