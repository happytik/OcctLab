//
#include "stdafx.h"
#include <assert.h>
#include <IntTools_EdgeEdge.hxx>
#include "QxBaseUtils.h"
#include "QxCurveInter.h"

////////////////////////////////////////////////////////////////////////////////////////
//
QxParamRange::QxParamRange()
{
	ts_ = 0.0;te_ = -1.0;//无效范围
}

QxParamRange::QxParamRange(double t1,double t2)
	:ts_(t1),te_(t2)
{
	assert(t1 <= t2);
}

void QxParamRange::Set(double t1,double t2)
{
	assert(t1 <= t2);
	ts_ = t1;te_ = t2;
}

////////////////////////////////////////////////////////////////////////////////////////
//交点
QxCCInterPoint::QxCCInterPoint()
{
	t1_ = t2_ = -1.0;
}

void  QxCCInterPoint::Set(double t1,double t2)
{
	t1_ = t1;t2_ = t2;
}

////////////////////////////////////////////////////////////////////////////////////////
//交线
QxCCInterCurve::QxCCInterCurve()
{
}


////////////////////////////////////////////////////////////////////////////////////////
//
QxCurveInter::QxCurveInter()
{
}

QxCurveInter::~QxCurveInter()
{
}

//两个曲线求交
bool  QxCurveInter::Intersect(const Handle(Geom_Curve) &aCurve1,const Handle(Geom_Curve) &aCurve2,double tol)
{
	if(aCurve1.IsNull() || aCurve2.IsNull()){
		assert(0);
		return false;
	}

	return Intersect(aCurve1,aCurve1->FirstParameter(),aCurve1->LastParameter(),
		aCurve2,aCurve2->FirstParameter(),aCurve2->LastParameter(),tol);
}

//两个曲线参数域求交
bool  QxCurveInter::Intersect(const Handle(Geom_Curve) &aCurve1,double t11,double t12,
						       const Handle(Geom_Curve) &aCurve2,double t21,double t22,double tol)
{
	if(aCurve1.IsNull() || (t11 >= t12) || aCurve2.IsNull() || (t21 >= t22)){
		assert(0);
		return false;
	}

	//生成Edge
	TopoDS_Edge aEdge1,aEdge2;
	try{
		BRepBuilderAPI_MakeEdge mkEdge1(aCurve1,t11,t12),mkEdge2(aCurve2,t21,t22);
		if(!mkEdge1.IsDone() || !mkEdge2.IsDone()){
			return false;
		}
		aEdge1 = mkEdge1.Edge();
		aEdge2 = mkEdge2.Edge();
	}catch(Standard_Failure){
		return false;
	}

	//求交
	IntTools_EdgeEdge aEEInt(aEdge1,aEdge2);
	QxCCInterPoint intpt;
	QxCCInterCurve intCrv;
	gp_Pnt aPnt;
	double t1 = 0.0,t2 = 0.0;

	try{
		aEEInt.Perform();
		if(aEEInt.IsDone()){
			const IntTools_SequenceOfCommonPrts &aCommPrts = aEEInt.CommonParts();
			//
			Standard_Integer ix = 0,nbCP = aCommPrts.Length();
			for(ix = 1;ix <= nbCP;ix ++){
				const IntTools_CommonPrt &aCommPart = aCommPrts(ix);
				//
				if(aCommPart.Type() == TopAbs_VERTEX){//交点
					//参数
					t1 = aCommPart.VertexParameter1();
					t2 = aCommPart.VertexParameter2();
					//
					intpt.Set(t1,t2);
					aCurve1->D0(t1,aPnt);
					KxFromOccPnt(aPnt,intpt.pnt1_);
					aCurve2->D0(t2,aPnt);
					KxFromOccPnt(aPnt,intpt.pnt2_);
					//
					aIntPntVec_.push_back(intpt);
				}
				if(aCommPart.Type() == TopAbs_EDGE){//交线
					//
					aCommPart.Range1(t1,t2);
					intCrv.aRange1_.Set(t1,t2);
					//
					const IntTools_SequenceOfRanges& aRanges = aCommPart.Ranges2();
					if(aRanges.Length() > 0){
						aRanges(1).Range(t1,t2);
						intCrv.aRange2_.Set(t1,t2);
					}
					//
					aIntCrvVec_.push_back(intCrv);
				}
			}
		}
	}catch(Standard_Failure){
		return false;
	}

	if(aIntPntVec_.empty() && aIntCrvVec_.empty())
		return false;

	return true;
}

void  QxCurveInter::Clear()
{
	aIntPntVec_.clear();
	aIntCrvVec_.clear();
}

int  QxCurveInter::NbPoints() const
{
	return (int)aIntPntVec_.size();
}

const QxCCInterPoint&  QxCurveInter::GetInterPoint(int ix) const
{
	assert(ix >= 0 && ix < (int)aIntPntVec_.size());
	return aIntPntVec_[ix];
}

int  QxCurveInter::NbCurves() const
{
	return (int)aIntCrvVec_.size();
}

const QxCCInterCurve&  QxCurveInter::GetInterCurve(int ix) const
{
	assert(ix >= 0 && ix < (int)aIntCrvVec_.size());
	return aIntCrvVec_[ix];
}