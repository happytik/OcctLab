#include "StdAfx.h"
#include "kcgIntersect.h"
#include "QxCurvePicker.h"

// 曲线上点的拾取
QxCurvePointPicker::QxCurvePointPicker()
{

}

bool	QxCurvePointPicker::Init(const TopoDS_Edge& aEdge)
{
	if(aEdge.IsNull()) return false;
	_aCurveShape = aEdge;
	return true;
}

bool	QxCurvePointPicker::Init(const TopoDS_Wire& aWire)
{
	if(aWire.IsNull()) return false;
	_aCurveShape = aWire;
	return true;
}

bool	QxCurvePointPicker::Perform(const kPoint3& p,double tol,double& t,kPoint3& pnt)
{
	if(_aCurveShape.IsNull()) return false;
	TopoDS_Edge aEdge;
	return Perform(p,tol,t,pnt,aEdge);
}

bool	QxCurvePointPicker::Perform(const kPoint3& p,double tol,double& t,kPoint3& pnt,TopoDS_Edge& aPickEdge)
{
	if(_aCurveShape.IsNull()) return false;
	if(_aCurveShape.ShapeType() == TopAbs_EDGE)
	{
		TopoDS_Edge aEdge = TopoDS::Edge(_aCurveShape);
		if(DoIntersect(p,aEdge,tol,t,pnt))
		{
			aPickEdge = aEdge;
			return true;
		}
	}
	else
	{
		TopoDS_Wire aWire = TopoDS::Wire(_aCurveShape);
		TopoDS_Iterator ite(aWire);
		for(;ite.More();ite.Next())
		{
			TopoDS_Edge aEdge = TopoDS::Edge(ite.Value());
			if(DoIntersect(p,aEdge,tol,t,pnt))
			{
				aPickEdge = aEdge;
				return true;
			}
		}
	}

	return false;
}

bool	QxCurvePointPicker::DoIntersect(const kPoint3& p,const TopoDS_Edge& aEdge,double tol,
										 double& t,kPoint3& pnt)
{
	kcgPointCurveInter pcInter;
	if(!pcInter.Intersect(p,aEdge,tol))
		return false;

	//通常一个交点
	kcxCurveIntpt ipt = pcInter.InterPoint(0);
	t = ipt._t;
	pnt = ipt._p;

	return true;
}

//////////////////////////////////////////////////////////////////////////
//
kcgPicker::kcgPicker(void)
{
}

kcgPicker::~kcgPicker(void)
{
}
