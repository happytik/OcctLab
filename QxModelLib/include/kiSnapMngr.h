//////////////////////////////////////////////////////////////////////////
//
#ifndef _KI_SNAP_MGR_H_
#define _KI_SNAP_MGR_H_

#include <vector>
#include <Graphic3d_Structure.hxx>
#include <Graphic3d_Group.hxx>
#include "kPoint.h"
#include "QxLibDef.h"

#define KSNAP_NIL				0 //不捕捉
#define KSNAP_EXIST_POINT		0x01 //存在点
#define KSNAP_TERM_POINT		0x02 //端点
#define KSNAP_MID_POINT			0x04 //中点
#define KSNAP_CEN_POINT			0x08 //中心点。
#define KSNAP_TMP_POINT			0x10 //临时点
#define KSNAP_QUA_POINT			0x20 //圆的四分点
#define KSNAP_GRID_POINT		0x80 //网格点
#define KSNAP_ALL				(KSNAP_EXIST_POINT | KSNAP_TERM_POINT | KSNAP_MID_POINT | KSNAP_CEN_POINT | KSNAP_TMP_POINT | KSNAP_GRID_POINT)

class kvWorkPlane;
class CRect;

//记录一个捕捉记录点
class QXLIB_API kiSnapPoint{
public:
	kiSnapPoint();
	kiSnapPoint(double x,double y,double z);
	kiSnapPoint(const kiSnapPoint& other);

	kiSnapPoint&	operator=(const kiSnapPoint& other);

public:
	kPoint3			_point;
	int				_scx,_scy;//屏幕坐标.
};


//提炼一个接口，用于将接口和实现分离
class QXLIB_API kiSnapMngr{
public:
	virtual bool			AddSnapPoint(int snapType,const kiSnapPoint& sp) = 0;
	virtual void			Clear() = 0;

	virtual bool			CalcSnapPoint(const TopoDS_Edge& aEdge) = 0;
	virtual bool			CalcSnapPoint(const TopoDS_Face& aFace) = 0;
	virtual bool			CalcSnapPoint(const TopoDS_Shape& aShape) = 0;

	// 网格点捕捉相关函数
	virtual bool			AddGridSnapPoint(const kiSnapPoint& sp) = 0;
	virtual void			ClearGridSnapPoint() = 0;
};

#endif