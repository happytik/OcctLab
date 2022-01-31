///
#ifndef _KC_FACE_POINT_PICKER_H_
#define _KC_FACE_POINT_PICKER_H_

#include "QxLibDef.h"
#include <V3d_View.hxx>

//根据平面坐标，获取面上一点
class QXLIB_API QxFacePointPicker{
public:
	QxFacePointPicker(const TopoDS_Face &aFace,const Handle(V3d_View) &aView);

	//根据平面，计算曲面上点,成功，返回相应的值，失败，返回false
	bool					PickPoint(int scrX,int scrY,double pnt[3],double uv[2]);

protected:
	TopoDS_Face			aFace_;
	Handle(V3d_View)		aView_;
	Handle(Geom_Surface)	aSurf_;
};

#endif