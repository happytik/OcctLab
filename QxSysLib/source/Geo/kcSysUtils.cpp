#include "StdAfx.h"
#include <gce_MakePln.hxx>
#include "kvCoordSystem.h"
#include "..\Dlgs\InputIntDlg.h"
#include "..\Dlgs\InputDblDlg.h"
#include "kcSysUtils.h"

bool	kcg_CalcRectCorner(kvCoordSystem& cs,const kPoint3& p1,const kPoint3& p2,double c[4][3])
{
	if(p1.distance(p2) < KC_MIN_DIST)
		return false;

	kPoint3 plb,prt,lp1,lp2,plt,prb;
	lp1 = p1;lp2 = p2;
	cs.WCSToUCS(lp1);
	cs.WCSToUCS(lp2);

	plb[0] = (lp1[0] < lp2[0]) ? lp1[0] : lp2[0];
	prt[0] = (lp1[0] < lp2[0]) ? lp2[0] : lp1[0];
	plb[1] = (lp1[1] < lp2[1]) ? lp1[1] : lp2[1];
	prt[1] = (lp1[1] < lp2[1]) ? lp2[1] : lp1[1];
	plb[2] = prt[2] = 0.0;
	prb[0] = prt[0];prb[1] = plb[1];
	plt[0] = plb[0];plt[1] = prt[1];
	prb[2] = plt[2] = 0.0;

	cs.UCSToWCS(plb);
	cs.UCSToWCS(prb);
	cs.UCSToWCS(prt);
	cs.UCSToWCS(plt);

	plb.get(c[0][0],c[0][1],c[0][2]);
	prb.get(c[1][0],c[1][1],c[1][2]);
	prt.get(c[2][0],c[2][1],c[2][2]);
	plt.get(c[3][0],c[3][1],c[3][2]);

	return true;
}

// 根据两个点,构建当前工作平面上的平面.
TopoDS_Face		kcg_BuildPlane(kvCoordSystem& cs,const kPoint3& p1,const kPoint3& p2)
{
	TopoDS_Face aFace;
	if(p1.distance(p2) < KC_MIN_DIST)
		return aFace;

	//取中心
	kPoint3 cp = p1 + p2;
	cp *= 0.5;

	gp_Ax2 axis(gp_Pnt(cp[0],cp[1],cp[2]),
		gp_Dir(gp_Vec(cs.Z().x(),cs.Z().y(),cs.Z().z())),
		gp_Dir(gp_Vec(cs.X().x(),cs.X().y(),cs.X().z())));
	gce_MakePln mp(axis);
	if(!mp.IsDone())
		return aFace;

	gp_Pln pln = mp.Value();
	gp_Ax3 ax3 = pln.Position();
	double u1,u2,v1,v2,u,v;
	ElSLib::PlaneParameters(ax3,gp_Pnt(p1[0],p1[1],p1[2]),u1,v1);
	ElSLib::PlaneParameters(ax3,gp_Pnt(p2[0],p2[1],p2[2]),u2,v2);
	if(u1 > u2) { u = u1;u1 = u2;u2 = u;}
	if(v1 > v2) { v = v1;v1 = v2;v2 = v;}

	try{
		BRepBuilderAPI_MakeFace mf(pln,u1,u2,v1,v2);
		if(!mf.IsDone())
			return aFace;
		aFace = mf.Face();
	}
	catch(Standard_Failure){
		return aFace;
	}
	
	return aFace;
}

// 计算当前工作平面上的box的角点和边的信息。
bool    kcg_CalcBoxCorner(kvCoordSystem& cs,const kPoint3& p1,const kPoint3& p2,const kPoint3& p3,
						  double c[8][3],int e[12][2])
{
	if(p1.distance(p2) < KC_MIN_DIST || p1.distance(p3) < KC_MIN_DIST ||
	   p2.distance(p3) < KC_MIN_DIST)
		return false;

	kPoint3 lp1 = p1,lp2 = p2,lp3 = p3;
	cs.WCSToUCS(lp1);
	cs.WCSToUCS(lp2);
	cs.WCSToUCS(lp3);

	double x1,x2,y1,y2,z1,z2;
	x1 = (lp1[0] < lp2[0]) ? lp1[0] : lp2[0];
	x2 = (lp1[0] > lp2[0]) ? lp1[0] : lp2[0];
	y1 = (lp1[1] < lp2[1]) ? lp1[1] : lp2[1];
	y2 = (lp1[1] > lp2[1]) ? lp1[1] : lp2[1];
	z1 = (lp1[2] < lp3[2]) ? lp1[2] : lp3[2];
	z2 = (lp1[2] > lp3[2]) ? lp1[2] : lp3[2];

	// 3 2
	// 0 1
	kPoint3 ap[8];
	ap[0][0] = ap[3][0] = x1;
	ap[1][0] = ap[2][0] = x2;
	ap[0][1] = ap[1][1] = y1;
	ap[2][1] = ap[3][1] = y2;
	ap[0][2] = ap[1][2] = ap[2][2] = ap[3][2] = z1;

	int ix;
	for(ix = 0;ix < 4;ix ++)
	{
		ap[ix + 4][0] = ap[ix][0];
		ap[ix + 4][1] = ap[ix][1];
		ap[ix + 4][2] = z2;
	}

	//变回全局坐标系
	for(ix = 0;ix < 8;ix ++)
	{
		cs.UCSToWCS(ap[ix]);
		ap[ix].get(c[ix]);
	}

	//边
	int aaE[][2] = { {0,1},{1,2},{2,3},{3,0},{4,5},{5,6},{6,7},{7,4},{0,4},{1,5},{2,6},{3,7}};
	for(ix = 0;ix < 12;ix ++)
	{
		e[ix][0] = aaE[ix][0];
		e[ix][1] = aaE[ix][1];
	}

	return true;
}

bool kcDlgInputDoubleValue(const char *pszDesc,double& dValue,double incStep)
{
	ASSERT(pszDesc);
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CInputDblDlg dlg(dValue,pszDesc);
	dlg.SetSpinIncrement(incStep);
	if(dlg.DoModal() == IDOK){
		dValue = dlg.GetValue();
		return true;
	}
	return false;
}

bool kcDlgInputIntValue(const char *pszDesc,int& iValue,int incStep)
{
	ASSERT(pszDesc);
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CInputIntDlg dlg(iValue,pszDesc);
	dlg.SetSpinIncrement(incStep);
	if(dlg.DoModal() == IDOK){
		iValue = dlg.GetValue();
		return true;
	}
	return false;
}