// Qx库的基本定义
#ifndef _QX_BASE_DEF_H_
#define _QX_BASE_DEF_H_

#ifndef ASSERT
#include <assert.h>
#define ASSERT(f) assert(f)
#endif

#include "QxPrecision.h"

#ifndef KDBL_EPSILON	
#define KDBL_EPSILON		1.0e-012 //最小精度,不必太高了。
#endif

#ifndef KDBL_MIN			
#define KDBL_MIN			1.0e-020 //小于该值认为为0
#endif
#ifndef KDBL_MAX
#define KDBL_MAX			1.0e+200 //最大双精度数，通常用不了太大。
#endif

#ifndef K_PI
#define K_PI				3.14159265358979323846
#endif

#ifndef PI1
#define PI1				3.14159265358979323846
#endif

#ifndef PI2
#define PI2				6.283185307179586
#endif

// 方便角度间的转换
#ifndef DEG_TO_RAD
#define DEG_TO_RAD(ang)		((ang) * PI1 / 180.0)
#endif

#ifndef RAD_TO_DEG
#define RAD_TO_DEG(deg)		((deg) * 180.0 / PI1)
#endif

// 浮点数的一个精度值
#define DOUBLE_EPS			(1.e-12) //足够小

#ifndef KC_SAFE_DELETE
#define KC_SAFE_DELETE(p)	{ if(p) { delete p; p = NULL;} }
#endif

#ifndef KC_SAFE_DELETE_ARRAY
#define KC_SAFE_DELETE_ARRAY(a) { if(a) {delete[] a; a = NULL;} }
#endif

typedef double PNT2D[2];
typedef double PNT3D[3];
typedef double VEC3D[3];

// 如下宏，简化点和矢量的处理
#ifndef KX_CPY_PNT

#define KX_CPY_PNT(frm,to)   memcpy(to,frm,sizeof(double)*3)
#define KX_CPY_VEC(frm,to)   memcpy(to,frm,sizeof(double)*3)
#define KX_GET_VEC(p1,p2,v)   {v[0] = p2[0] - p1[0];v[1] = p2[1] - p1[1];v[2] = p2[2] - p1[2];}
#define KX_REV_VEC(v) { v[0] = -v[0];v[1] = -v[1];v[2] = -v[2];}	
#define KX_MID_PNT(mp,p1,p2)  { mp[0] = (p1[0] + p2[0])/2;mp[1] = (p1[1] + p2[1])/2;mp[2] = (p1[2] + p2[2])/2;}

#define KX_PVT_ADD(p,p0,t,d) \
	p[0] = p0[0] + (t) * d[0];\
	p[1] = p0[1] + (t) * d[1];\
	p[2] = p0[2] + (t) * d[2];

#endif

// 简化版本，后面需要统一
#ifndef CPY_PNT

#define CPY_PNT(frm,to)   memcpy(to,frm,sizeof(double)*3)
#define CPY_VEC(frm,to)   memcpy(to,frm,sizeof(double)*3)
#define GET_VEC(p1,p2,v)   {v[0] = p2[0] - p1[0];v[1] = p2[1] - p1[1];v[2] = p2[2] - p1[2];}
#define REV_VEC(v) { v[0] = -v[0];v[1] = -v[1];v[2] = -v[2];}	
#define MID_PNT(mp,p1,p2)  { mp[0] = (p1[0] + p2[0])/2;mp[1] = (p1[1] + p2[1])/2;mp[2] = (p1[2] + p2[2])/2;}

#define PVT_ADD(p,p0,t,d) \
	p[0] = p0[0] + (t) * d[0];\
	p[1] = p0[1] + (t) * d[1];\
	p[2] = p0[2] + (t) * d[2];

#endif

#endif