//
#ifndef _QX_CURVE_INTER_H_
#define _QX_CURVE_INTER_H_

#include <vector>
#include <Geom_Curve.hxx>
#include "QxLibDef.h"

class QxCCInterPoint;
class QxCCInterCurve;
typedef std::vector<QxCCInterPoint> KCCInterPointVector;
typedef std::vector<QxCCInterCurve> KCCInterCurveVector;

//说明：
//  曲线求交，这里借用了布尔运算中的edge和edge求交，
//  还需要优化，特别是对无限长直线和其他曲线求交目前还没有添加处理。
//  这个将是主要的求交函数

class QXLIB_API QxParamRange{
public:
	QxParamRange();
	QxParamRange(double t1,double t2);

	void			Set(double t1,double t2);

public:
	double		ts_,te_;//参数域范围值
};

//交点
class QXLIB_API QxCCInterPoint{
public:
	QxCCInterPoint();

	void			Set(double t1,double t2);

public:
	double		t1_,t2_;//交点参数
	double		pnt1_[3],pnt2_[3];//交点
};

//交线
class QXLIB_API QxCCInterCurve{
public:
	QxCCInterCurve();

public:
	QxParamRange	aRange1_,aRange2_;//两个范围
};

class QXLIB_API QxCurveInter{
public:
	QxCurveInter();
	~QxCurveInter();

	//两个曲线求交
	bool			Intersect(const Handle(Geom_Curve) &aCurve1,const Handle(Geom_Curve) &aCurve2,double tol);

	//两个曲线参数域求交
	bool			Intersect(const Handle(Geom_Curve) &aCurve1,double t11,double t12,
						const Handle(Geom_Curve) &aCurve2,double t21,double t22,double tol);

	void			Clear();

	int			NbPoints() const;
	const QxCCInterPoint& GetInterPoint(int ix) const;

	int			NbCurves() const;
	const QxCCInterCurve& GetInterCurve(int ix) const;

protected:
	KCCInterPointVector	aIntPntVec_;//交点列表
	KCCInterCurveVector	aIntCrvVec_;//交线列表
};


#endif