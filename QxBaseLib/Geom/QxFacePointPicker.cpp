//
#include "stdafx.h"
#include <Geom_Line.hxx>
#include <BRepClass_FaceClassifier.hxx>
#include "QxFacePointPicker.h"

QxFacePointPicker::QxFacePointPicker(const TopoDS_Face &aFace,const Handle(V3d_View) &aView)
{
	aFace_ = aFace;
	aView_ = aView;
	//获取对应的曲面
	aSurf_ = BRep_Tool::Surface(aFace);
}

//根据平面，计算曲面上点,成功，返回相应的值，失败，返回false
bool QxFacePointPicker::PickPoint(int scrX,int scrY,double pnt[3],double uv[2])
{
	if(aSurf_.IsNull())
		return false;

	double eyeX,eyeY,eyeZ,dx,dy,dz;
	//
	aView_->ConvertWithProj(scrX,scrY,eyeX,eyeY,eyeZ,dx,dy,dz);
	//
	//生成射线
	gp_Ax1 ax1(gp_Pnt(eyeX,eyeY,eyeZ),gp_Dir(dx,dy,dz));
	Handle(Geom_Line) aLine = new Geom_Line(ax1);
	//线面求交
	GeomAPI_IntCS intCS;
	intCS.Perform(aLine,aSurf_);
	if(!intCS.IsDone()){
		return false;
	}

	int ix,ixMin = 0,nbPnts = intCS.NbPoints();
	BRepClass_FaceClassifier faceCls;
	double dTol = BRep_Tool::Tolerance(aFace_);
	double u,v,w,wMin = Precision::Infinite();
	TopAbs_State state;

	for(ix = 1;ix <= nbPnts;ix ++){
		intCS.Parameters(ix,u,v,w);
		//是否在面内部
		faceCls.Perform(aFace_,gp_Pnt2d(u,v),dTol);
		state = faceCls.State();
		if(state == TopAbs_ON || state == TopAbs_IN){
			if(w < wMin){
				wMin = w;
				ixMin = ix;
			}
		}
	}

	if(ixMin == 0){//没有相应点
		return false;
	}

	intCS.Parameters(ixMin,uv[0],uv[1],w);
	gp_Pnt aPnt = intCS.Point(ixMin);
	aPnt.Coord(pnt[0],pnt[1],pnt[2]);

	return true;
}