//////////////////////////////////////////////////////////////////////////
//
#ifndef _KC_SYS_UTILS_H_
#define _KC_SYS_UTILS_H_

#include "kBase.h"
#include "kPoint.h"
#include "QxLibDef.h"

class kvCoordSystem;

QXLIB_API bool	kcg_CalcRectCorner(kvCoordSystem& cs,const kPoint3& p1,const kPoint3& p2,double c[4][3]);

// 计算当前工作平面上的box的角点和边的信息。p1和p2是工作平面上的点。
QXLIB_API bool    kcg_CalcBoxCorner(kvCoordSystem& cs,const kPoint3& p1,const kPoint3& p2,const kPoint3& p3,
						  double c[8][3],int e[12][2]);

// 根据两个点,构建当前工作平面上的平面.
QXLIB_API TopoDS_Face kcg_BuildPlane(kvCoordSystem& cs,const kPoint3& p1,const kPoint3& p2);

// 使用输入对话框，输出一个整形值
// pszDesc是描述信息，dValue是可以传入初始值，并返回输入值,incStep:是每次调整值
// 
QXLIB_API bool kcDlgInputDoubleValue(const char *pszDesc,double& dValue,double incStep=0.1);

QXLIB_API bool kcDlgInputIntValue(const char *pszDesc,int& iValue,int incStep=1);

#endif