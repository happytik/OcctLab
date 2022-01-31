//
#include "stdafx.h"
#include <GC_MakeLine.hxx>
#include <GC_MakeSegment.hxx>
#include <GeomAPI_ProjectPointOnCurve.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <BRepAdaptor_Surface.hxx>
#include "QxBaseDef.h"
#include "QxBaseUtils.h"

///////////////////////////////////////////////////////////////////////////////
//
//曲线类型，和宏GeomAbs_CurveType对应
static const char  *s_pCurveType[] = 
	{
		"Line",
		"Circle",
		"Ellipse",
		"Hyperbola",
		"Parabola",
		"Bezier Curve",
		"BSpline Curve",
		"Offset Curve",
		"Other Curve"
	};

//曲面类型，和宏GeomAbs_SurfaceType对应
static const char  *s_pSurfType[] = 
	{
		"Plane",
		"Cylinder",
		"Cone",
		"Sphere",
		"Torus",
		"Bezier Surface",
		"BSpline Surface",
		"SurfaceOfRevolution",
		"SurfaceOfExtrusion",
		"Offset Surface",
		"OtherSurface"
	};

const char* kxCurveType(const Handle(Geom_Curve) &aCurve)
{
	if(aCurve.IsNull())
		return NULL;

	GeomAdaptor_Curve adCrv(aCurve);
	return s_pCurveType[(int)adCrv.GetType()];
}

const char* kxCurveType(const TopoDS_Edge &aEdge)
{
	if(aEdge.IsNull())
		return NULL;

	BRepAdaptor_Curve adCrv(aEdge);
	return s_pCurveType[(int)adCrv.GetType()];
}

const char* kxSurfaceType(const Handle(Geom_Surface) &aSurf)
{
	if(aSurf.IsNull())
		return NULL;

	GeomAdaptor_Surface adSurf(aSurf);
	return s_pSurfType[(int)adSurf.GetType()];
}

const char* kxSurfaceType(const TopoDS_Face &aFace)
{
	if(aFace.IsNull())
		return NULL;

	BRepAdaptor_Surface adSurf(aFace);
	return s_pSurfType[(int)adSurf.GetType()];
}

///////////////////////////////////////////////////
//

static const char *s_pContName[] = {
	"GeomAbs_C0",
	"GeomAbs_G1",
	"GeomAbs_C1",
	"GeomAbs_G2",
	"GeomAbs_C2",
	"GeomAbs_C3",
	"GeomAbs_CN"
};

const char* kxContinuity(GeomAbs_Shape cont)
{
	return s_pContName[(int)cont];
}

///////////////////////////////////////////////////////////////////////////////
//


double kxCalcMidPointDeviation(double p1[3],double p2[3],double mp[3])
{
	double dval = 10000.0;

	gp_Dir aDir(p2[0]-p1[0],p2[1]-p1[1],p2[2]-p1[2]);
	GC_MakeLine mkLin(gp_Pnt(p1[0],p1[1],p1[2]),aDir);
	if(mkLin.IsDone()){
		Handle(Geom_Line) aLin = mkLin.Value();
		//点向其投影
		GeomAPI_ProjectPointOnCurve ppOnC(gp_Pnt(mp[0],mp[1],mp[2]),aLin);
		if(ppOnC.NbPoints() > 0){
			dval = ppOnC.LowerDistance();
		}
	}
	return dval;
}

double kxTwoPointDist(double p1[3],double p2[3])
{
	gp_Vec aVec(p2[0]-p1[0],p2[1]-p1[1],p2[2]-p1[2]);
	return aVec.Magnitude();
}

double kxDotProduct(double v1[3],double v2[3])
{
	return (v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2]);
}

bool kxCrossProduct(double v1[3],double v2[3],double vres[3])
{
	gp_Vec vec(v1[0],v1[1],v1[2]);
	vec.Cross(gp_Vec(v2[0],v2[1],v2[2]));

	double len = vec.Magnitude();
	if(len < 1.0e-20){
		return false;
	}
	vec.Coord(vres[0],vres[1],vres[2]);
	return true;
}

bool  kxTransLocalPntToWCS(const gp_Ax2 &aLF,double lpnt[3],double wpnt[3])
{
	gp_Ax3 wcs,lcs(aLF);
	gp_Trsf aTrsf;
	gp_Pnt pnt(lpnt[0],lpnt[1],lpnt[2]);

	aTrsf.SetTransformation(lcs,wcs);
	gp_Pnt wp = pnt.Transformed(aTrsf);
	wp.Coord(wpnt[0],wpnt[1],wpnt[2]);

	return true;
}

bool  kxTransWorldPntToLCS(const gp_Ax2 &aLF,double wpnt[3],double lpnt[3])
{
	gp_Ax3 wcs,lcs(aLF);
	gp_Trsf aTrsf;
	gp_Pnt pnt(wpnt[0],wpnt[1],wpnt[2]);

	aTrsf.SetTransformation(wcs,lcs);
	gp_Pnt pt = pnt.Transformed(aTrsf);
	pt.Coord(lpnt[0],lpnt[1],lpnt[2]);

	return true;
}

bool  kxTransLocalVecToWCS(const gp_Ax2 &aLF,double lvec[3],double wvec[3])
{
	gp_Ax3 wcs,lcs(aLF);
	gp_Trsf aTrsf;
	gp_Vec vec(lvec[0],lvec[1],lvec[2]);

	aTrsf.SetTransformation(lcs,wcs);
	gp_Vec v = vec.Transformed(aTrsf);
	v.Coord(wvec[0],wvec[1],wvec[2]);

	return true;
}

bool  kxTransWorldVecToLCS(const gp_Ax2 &aLF,double wvec[3],double lvec[3])
{
	gp_Ax3 wcs,lcs(aLF);
	gp_Trsf aTrsf;
	gp_Vec vec(wvec[0],wvec[1],wvec[2]);

	aTrsf.SetTransformation(wcs,lcs);
	gp_Vec v = vec.Transformed(aTrsf);
	v.Coord(lvec[0],lvec[1],lvec[2]);

	return true;
}

void  kxRotatePoint(const gp_Ax1 &axis,double pnt[3],double angle,double rp[3])
{
	gp_Pnt aP(pnt[0],pnt[1],pnt[2]);
	aP.Rotate(axis,angle);
	aP.Coord(rp[0],rp[1],rp[2]);
}

bool  kxNormalizeVec(double vec[3],double eps)
{
	gp_Vec aV(vec[0],vec[1],vec[2]);
	double m = aV.Magnitude();
	if(m < eps){
		vec[0] = vec[1] = vec[2] = 0.0;
		return false;
	}

	vec[0] /= m;
	vec[1] /= m;
	vec[2] /= m;

	return true;
}

bool  kxGetNormalizedVec(double p1[3],double p2[3],double vec[3])
{
	gp_Vec aV(p2[0]-p1[0],p2[1]-p1[1],p2[2]-p1[2]);
	double m = aV.Magnitude();
	if(m < 1.0e-20){
		vec[0] = vec[1] = vec[2] = 0.0;
		return false;
	}
	aV.Normalize();
	aV.Coord(vec[0],vec[1],vec[2]);

	return true;
}

double kxGetAngle(double vec1[3],double vec2[3],double eps)
{
	gp_Vec aV(vec1[0],vec1[1],vec1[2]);
	gp_Vec aV2(vec2[0],vec2[1],vec2[2]);
	double m1 = aV.Magnitude(),m2 = aV2.Magnitude();
	if(m1 < eps || m2 < eps){
		ASSERT(FALSE);
		return 0.0;
	}
	return aV.Angle(aV2);
}

//计算点到直线的距离
double kxPointLineDist(double pnt[3],double pivot[3],double dir[3])
{
	double dist = -1.0;
	Handle(Geom_Line) aLine = new Geom_Line(gp_Pnt(pivot[0],pivot[1],pivot[2]),gp_Dir(dir[0],dir[1],dir[2]));
	GeomAPI_ProjectPointOnCurve projector(gp_Pnt(pnt[0],pnt[1],pnt[2]),aLine);
	if(projector.NbPoints() > 0){
		dist = projector.LowerDistance();
	}else{
		ASSERT(FALSE);
	}
	return dist;
}

//生成直线段
Handle(Geom_Curve) kxMakeLine(double p1[3],double p2[3])
{
	Handle(Geom_Curve) aLine;
	double dist = kxTwoPointDist(p1,p2);
	if(dist < K_LINEAR_RES){
		return aLine;
	}

	GC_MakeLine mkLin(gp_Pnt(p1[0],p1[1],p1[2]),gp_Pnt(p2[0],p2[1],p2[2]));
	if(mkLin.IsDone()){
		aLine = mkLin.Value();
	}
	return aLine;
}

//生成直线段
Handle(Geom_Curve) kxMakeLineSegment(double p1[3],double p2[3])
{
	Handle(Geom_Curve) aSeg;
	double dist = kxTwoPointDist(p1,p2);
	if(dist < K_LINEAR_RES){
		return aSeg;
	}

	GC_MakeSegment mkSeg(gp_Pnt(p1[0],p1[1],p1[2]),gp_Pnt(p2[0],p2[1],p2[2]));
	if(mkSeg.IsDone()){
		aSeg = mkSeg.Value();
	}
	return aSeg;
}

Handle(Geom_Curve) kxMakeLineSegment(const gp_Pnt &aP1,const gp_Pnt &aP2)
{
	Handle(Geom_Curve) aSeg;
	double dist = aP1.Distance(aP2);
	if(dist < K_LINEAR_RES){
		return aSeg;
	}

	GC_MakeSegment mkSeg(aP1,aP2);
	if(mkSeg.IsDone()){
		aSeg = mkSeg.Value();
	}
	return aSeg;
}

BOOL kuiBrowseForFolder(CWnd *pWndParent,LPCTSTR lpszTitle,CString &szFolder)
{
	LPMALLOC lpMalloc = NULL;
	BROWSEINFO bi;
	BOOL bRes = FALSE;

	ZeroMemory(&bi, sizeof(bi));
	bi.lpszTitle = lpszTitle != NULL ? lpszTitle : _T("");
	bi.pszDisplayName = NULL;
	bi.hwndOwner = pWndParent->GetSafeHwnd();
	bi.lpfn = NULL;
	bi.lParam = 0;
	bi.pidlRoot = NULL;
	bi.ulFlags = BIF_RETURNONLYFSDIRS ;
	bi.iImage = -1;
	
	LPITEMIDLIST pidlRes = SHBrowseForFolder(&bi);
	if (pidlRes != NULL){
		TCHAR szPath [MAX_PATH];
		if (SHGetPathFromIDList(pidlRes, szPath)){
			szFolder = szPath;
			bRes = TRUE;
		}

		if(SUCCEEDED(SHGetMalloc(&lpMalloc))){
			if(lpMalloc != NULL){
				lpMalloc->Free(pidlRes);
				lpMalloc->Release();
			}
		}
	}

	return bRes;
}