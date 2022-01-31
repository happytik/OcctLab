#include "StdAfx.h"
#include <BRepAdaptor_Surface.hxx>
#include <BRepAlgo_FaceRestrictor.hxx>
#include <BRepCheck_Wire.hxx>
#include "kBase.h"
#include "QxSurfLib.h"

/////////////////////////////////////////////////////////////
//
static const char st_aSurfName[11][64]={
	"Plane",
	"ConicalSurface",
	"CylindricalSurface",
	"SphericalSurface",
	"ToroidalSurface",
	"BezierSurface",
	"BSplineSurface",
	"RectangularTrimmedSurface",
	"OffsetSurface",
	"SurfaceOfLinearExtrusion",
	"SurfaceOfRevolution"
};

int  kcgSurfNameAndIndex::GeomSurfIndex(const Handle(Geom_Surface) aSurf)
{
	int nIdx = -1;
	if(aSurf->IsKind(STANDARD_TYPE(Geom_Plane))){
		nIdx = 0;
	}else if(aSurf->IsKind(STANDARD_TYPE(Geom_ConicalSurface))){
		nIdx = 1;
	}else if(aSurf->IsKind(STANDARD_TYPE(Geom_CylindricalSurface))){
		nIdx = 2;
	}else if(aSurf->IsKind(STANDARD_TYPE(Geom_SphericalSurface))){
		nIdx = 3;
	}else if(aSurf->IsKind(STANDARD_TYPE(Geom_ToroidalSurface))){
		nIdx = 4;
	}else if(aSurf->IsKind(STANDARD_TYPE(Geom_BezierSurface))){
		nIdx = 5;
	}else if(aSurf->IsKind(STANDARD_TYPE(Geom_BSplineSurface))){
		nIdx = 6;
	}else if(aSurf->IsKind(STANDARD_TYPE(Geom_RectangularTrimmedSurface))){
		nIdx = 7;
	}else if(aSurf->IsKind(STANDARD_TYPE(Geom_OffsetSurface))){
		nIdx = 8;
	}else if(aSurf->IsKind(STANDARD_TYPE(Geom_SurfaceOfLinearExtrusion))){
		nIdx = 9;
	}else if(aSurf->IsKind(STANDARD_TYPE(Geom_SurfaceOfRevolution))){
		nIdx = 10;
	}else{
		ASSERT(FALSE);
	}
	return nIdx;
}
const char*  kcgSurfNameAndIndex::GeomSurfName(const Handle(Geom_Surface) aSurf)
{
	int idx = GeomSurfIndex(aSurf);
	return st_aSurfName[idx];
}

const char*  kcgSurfNameAndIndex::GeomSurfName(int idx)
{
	if(idx < 0 || idx > 10)
		return NULL;
	return st_aSurfName[idx];
}

//////////////////////////////////////////////////////////////////
//
QxSurfLib::QxSurfLib(void)
{
}

QxSurfLib::~QxSurfLib(void)
{
}

//投影点到平面
bool	QxSurfLib::ProjectPointToPlane(const kPoint3& p,
										const kPoint3& pivot,const kVector3& dir,
										kPoint3& prjpnt)
{
	try{
		Handle(Geom_Plane) aPln = new Geom_Plane(gp_Pnt(pivot[0],pivot[1],pivot[2]),gp_Dir(dir[0],dir[1],dir[2]));
		if(aPln.IsNull())
			return false;
		GeomAPI_ProjectPointOnSurf pps(gp_Pnt(p[0],p[1],p[2]),aPln);
		if(!pps.IsDone() || pps.NbPoints() == 0)
			return false;
		gp_Pnt pnt = pps.Point(1);
		prjpnt.set(pnt.X(),pnt.Y(),pnt.Z());
	}catch(Standard_Failure){
		return false;
	}
	return true;
}

bool	QxSurfLib::IsPointOnPlane(const kPoint3& p,
								   const kPoint3& pivot,const kVector3& dir)
{
	kVector3 v = p - pivot,vdir = dir;
	v.normalize();
	vdir.normalize();
	double dist = v * dir;
	if(fabs(dist) < KDBL_MIN)
		return true;
	return false;
}

//计算face的一个点和normal
bool	QxSurfLib::CalcFaceNormal(const TopoDS_Face& aFace,double u,double v,bool bTopo,kPoint3& p,kVector3& normal)
{
	if(aFace.IsNull()) return false;
	BRepAdaptor_Surface aSurf;
	TopoDS_Face aF = TopoDS::Face(aFace.Oriented(TopAbs_FORWARD));
	bool bReverse = (aFace.Orientation() == TopAbs_REVERSED) ? true : false;
	aSurf.Initialize(aF);

	gp_Pnt pnt;
	gp_Vec du,dv,dn;
	aSurf.D1(u,v,pnt,du,dv);
	dn = du.Crossed(dv);
	if(dn.Magnitude() < KDBL_MIN)
		return false;

	//考虑上拓扑方向，计算face的normal
	if(bTopo){
		if(bReverse)
			dn.Reverse();
	}

	p.set(pnt.X(),pnt.Y(),pnt.Z());
	normal.set(dn.X(),dn.Y(),dn.Z());

	return true;
}

bool	QxSurfLib::CalcFaceNormal(const TopoDS_Face& aFace,bool bTopo,kPoint3& p,kVector3& normal)
{
	double u0,u1,v0,v1,u,v;
	BRepTools::UVBounds(aFace,u0,u1,v0,v1);
	u = (u0 + u1) / 2;
	v = (v0 + v1) / 2;

	return QxSurfLib::CalcFaceNormal(aFace,u,v,bTopo,p,normal);
}

//////////////////////////////////////////////////////////////////////////
//平面多个环构造face
kcgBuildFaceFromPlnWire::kcgBuildFaceFromPlnWire()
{

}

BOOL	kcgBuildFaceFromPlnWire::Init()
{
	m_aShapeList.Clear();
	m_aFaceList.Clear();
	return TRUE;
}

//需要检查闭合性
BOOL	kcgBuildFaceFromPlnWire::Add(const TopoDS_Edge& aEdge)
{
	if(aEdge.IsNull())
		return FALSE;

	TopoDS_Wire aWire;
	try{
		BRepBuilderAPI_MakeWire mw(aEdge);
		if(!mw.IsDone())
			return FALSE;

		aWire = mw.Wire();
		//检查是否闭合
		BRepCheck_Wire chkWire(aWire);
		if(BRepCheck_NoError != chkWire.Closed())
			return FALSE;//not close
		m_aShapeList.Append(aWire);
	}catch(Standard_Failure){
		return FALSE;
	}
	return TRUE;
}

//需要检查闭合性
BOOL	kcgBuildFaceFromPlnWire::Add(const TopoDS_Wire& aWire)
{
	if(aWire.IsNull())
		return FALSE;

	//检查闭合性
	try{
		BRepCheck_Wire chkWire(aWire);
		if(BRepCheck_NoError != chkWire.Closed())
			return FALSE;//not close
		m_aShapeList.Append(aWire);
	}catch(Standard_Failure){
		return FALSE;
	}
	return TRUE;
}

BOOL	kcgBuildFaceFromPlnWire::Build()
{
	m_aFaceList.Clear();
	if(m_aShapeList.IsEmpty())
		return FALSE;

	//检查是否共面，并构造初始平面
	BRep_Builder BB;
	TopoDS_Wire aWire;
	TopoDS_Compound aComp;
	BB.MakeCompound(aComp);
	TopTools_ListIteratorOfListOfShape ite;
	for(ite.Initialize(m_aShapeList);ite.More();ite.Next())
	{
		aWire = TopoDS::Wire(ite.Value());
		BB.Add(aComp,aWire);
	}

	TopoDS_Face aPlnFace;
	try{
		BRepBuilderAPI_FindPlane findPln(aComp);
		if(!findPln.Found())
			return FALSE;

		BRepBuilderAPI_MakeFace mf(findPln.Plane(),Precision::Confusion());
		if(!mf.IsDone())
			return FALSE;

		aPlnFace = mf.Face();
		if(aPlnFace.IsNull())
			return FALSE;
	}catch(Standard_Failure){
		return FALSE;
	}

	//构造相应的face
	try{
		BRepAlgo_FaceRestrictor fr;
		fr.Init(aPlnFace,Standard_True,Standard_True);//计算参数曲线，调整方向
		for(ite.Initialize(m_aShapeList);ite.More();ite.Next())
		{
			aWire = TopoDS::Wire(ite.Value());
			fr.Add(aWire);
		}
		fr.Perform();
		if(!fr.IsDone())
			return FALSE;

		TopoDS_Face aFace;
		for(;fr.More();fr.Next())
			m_aFaceList.Append(fr.Current());
	}catch(Standard_Failure){
		return FALSE;
	}
	return TRUE;
}

int		kcgBuildFaceFromPlnWire::NbFaces() const
{
	return m_aFaceList.Length();
}

TopoDS_Face		kcgBuildFaceFromPlnWire::GetFace(int ix)
{
	TopoDS_Face aFace;
	if(ix < 0 || ix >= m_aFaceList.Length())
	{
		ASSERT(FALSE);
		return aFace;
	}
	aFace = TopoDS::Face(m_aFaceList.Value(ix + 1));
	return aFace;
}

