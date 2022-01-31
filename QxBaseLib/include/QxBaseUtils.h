//
#ifndef _QX_BASE_UTILS_H_
#define _QX_BASE_UTILS_H_

#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>
#include "QxLibDef.h"
#include "QxBaseDef.h"

//定义occ的一些辅助功能和函数
// 转换宏
#define KCG_GP_PNT(p)	gp_Pnt(p[0],p[1],p[2])
#define KCG_GP_VEC(v)	gp_Vec(v[0],v[1],v[2])
#define KCG_GP_DIR(d)	gp_Dir(d[0],d[1],d[2])

QXLIB_API const char* kxCurveType(const Handle(Geom_Curve) &aCurve);
QXLIB_API const char* kxCurveType(const TopoDS_Edge &aEdge);

QXLIB_API const char* kxSurfaceType(const Handle(Geom_Surface) &aSurf);
QXLIB_API const char* kxSurfaceType(const TopoDS_Face &aFace);

QXLIB_API const char* kxContinuity(GeomAbs_Shape cont);

//在OCC的点和数组间进行转换
inline void KxToOccPnt(double pnt[3],gp_Pnt &aPnt)
{
	aPnt.SetCoord(pnt[0],pnt[1],pnt[2]);
}

inline void KxFromOccPnt(const gp_Pnt &aPnt,double pnt[3])
{
	pnt[0] = aPnt.X();pnt[1] = aPnt.Y();pnt[2] = aPnt.Z();
}

inline void KxToOccVec(double v[3],gp_Vec &aVec)
{
	aVec.SetCoord(v[0],v[1],v[2]);
}

inline void KxFromOccVec(const gp_Vec &aVec,double v[3])
{
	v[0] = aVec.X();v[1] = aVec.Y();v[2] = aVec.Z();
}

//记录中点距离两点的弦高误差
QXLIB_API double kxCalcMidPointDeviation(double p1[3],double p2[3],double mp[3]);

QXLIB_API double kxTwoPointDist(double p1[3],double p2[3]);

QXLIB_API double kxDotProduct(double v1[3],double v2[3]);

QXLIB_API bool kxCrossProduct(double v1[3],double v2[3],double vres[3]);

QXLIB_API bool  kxTransLocalPntToWCS(const gp_Ax2 &aLF,double lpnt[3],double wpnt[3]);

QXLIB_API bool  kxTransWorldPntToLCS(const gp_Ax2 &aLF,double wpnt[3],double lpnt[3]);

QXLIB_API bool  kxTransLocalVecToWCS(const gp_Ax2 &aLF,double lvec[3],double wvec[3]);

QXLIB_API bool  kxTransWorldVecToLCS(const gp_Ax2 &aLF,double wvec[3],double lvec[3]);

QXLIB_API void  kxRotatePoint(const gp_Ax1 &axis,double pnt[3],double angle,double rp[3]);

QXLIB_API bool  kxNormalizeVec(double vec[3],double eps);
QXLIB_API bool  kxGetNormalizedVec(double p1[3],double p2[3],double vec[3]);

QXLIB_API double kxGetAngle(double vec1[3],double vec2[3],double eps);

//计算点到直线的距离
QXLIB_API double kxPointLineDist(double pnt[3],double pivot[3],double dir[3]);

//生成直线,无限长
QXLIB_API Handle(Geom_Curve) kxMakeLine(double p1[3],double p2[3]);
//生成直线段
QXLIB_API Handle(Geom_Curve) kxMakeLineSegment(double p1[3],double p2[3]);
QXLIB_API Handle(Geom_Curve) kxMakeLineSegment(const gp_Pnt &aP1,const gp_Pnt &aP2);

class CWnd;
QXLIB_API BOOL kuiBrowseForFolder(CWnd *pWndParent,LPCTSTR lpszTitle,CString &szFolder);

#endif