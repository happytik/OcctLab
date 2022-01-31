//////////////////////////////////////////////////////////////////////////
//
#ifndef _KCG_PICKER_H_
#define _KCG_PICKER_H_

#include "QxLibDef.h"
#include "kPoint.h"

// 曲线上点的拾取
class QXLIB_API QxCurvePointPicker{
public:
	QxCurvePointPicker();

	bool						Init(const TopoDS_Edge& aEdge);
	bool						Init(const TopoDS_Wire& aWire);
	bool						Perform(const kPoint3& p,double tol,double& t,kPoint3& pnt);
	bool						Perform(const kPoint3& p,double tol,double& t,kPoint3& pnt,TopoDS_Edge& aPickEdge);

protected:
	bool						DoIntersect(const kPoint3& p,const TopoDS_Edge& aEdge,double tol,double& t,kPoint3& pnt);

protected:
	TopoDS_Shape				_aCurveShape;
};



class kcgPicker
{
public:
	kcgPicker(void);
	~kcgPicker(void);
};

#endif