//////////////////////////////////////////////////////////////////////////
//
#ifndef _KCM_UTILS_H_
#define _KCM_UTILS_H_

#include "kPoint.h"
#include "QxLibDef.h"

class kvCoordSystem;
class kcBasePlane;

// 根据当前工作平面和圆心、半径创建圆对象
QXLIB_API Handle(Geom_Circle)  ulb_MakeCircle(const kPoint3& center,double radius,kcBasePlane *pWorkPlane);

QXLIB_API Handle(Geom_Circle)  ulb_MakeCircle(const kvCoordSystem& lcs,double radius);


#endif