// ShapeStructDialog.cpp : 实现文件
//

#include "stdafx.h"
#include <string>
#include <map>
#include <strstream>
#include <sstream>
#include <omp.h>
#include <ShapeAnalysis.hxx>
#include <ShapeExtend_WireData.hxx>
#include <TopTools_DataMapIteratorOfDataMapOfShapeInteger.hxx>
#include <Poly_Triangulation.hxx>
#include <BRep_TEdge.hxx>
#include <BRep_Tool.hxx>
#include <BRep_Curve3D.hxx>
#include <BRep_GCurve.hxx>
#include <BRep_CurveOnClosedSurface.hxx>
#include <BRep_CurveOn2Surfaces.hxx>
#include <BRep_ListIteratorOfListOfCurveRepresentation.hxx>
#include <BRepGProp.hxx>
#include <GProp_GProps.hxx>
#include <BRepAdaptor_Curve.hxx>

#include "kBase.h"
#include "ShapeStructDialog.h"

#ifndef DeclareAndCast
#define DeclareAndCast(atype,result,aSurf) \
	Handle(atype) result = Handle(atype)::DownCast(aSurf)
#endif

//常用宏描述
char *s_shapeType[] = {
	" COMPOUND ",
		" COMPSOLID ",
		" SOLID ",
		" SHELL ",
		" FACE ",
		" WIRE ",
		" EDGE ",
		" VERTEX ",
		" SHAPE "
};

char *s_oritDesc[] = {
	" FORWARD ",
		" REVERSED ",
		" INTERNAL ",
		" EXTERNAL "
};

// CShapeStructDialog 对话框

IMPLEMENT_DYNAMIC(CShapeStructDialog, CDialog)
CShapeStructDialog::CShapeStructDialog(const TopoDS_Shape& aShape,CWnd* pParent /*=NULL*/)
	: CDialog(CShapeStructDialog::IDD, pParent)
{
	m_aShape = aShape;
	bUseParentOri_ = true;//默认考虑
}

CShapeStructDialog::~CShapeStructDialog()
{		
}

void CShapeStructDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TREE_SHAPE_STRUCT, m_ssTree);
}

BEGIN_MESSAGE_MAP(CShapeStructDialog, CDialog)
END_MESSAGE_MAP()

//测试代码

// CShapeStructDialog 消息处理程序

BOOL CShapeStructDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	if(m_aShape.IsNull())
		return FALSE;

	CWaitCursor wc;

	//生成Face和Edge的map
	m_aFaceIdxMap.Clear();
	m_aEdgeIdxMap.Clear();
	m_aVertexIdxMap.Clear();
	TopExp::MapShapes(m_aShape,TopAbs_FACE,m_aFaceIdxMap);
	TopExp::MapShapes(m_aShape,TopAbs_EDGE,m_aEdgeIdxMap);
	TopExp::MapShapes(m_aShape,TopAbs_VERTEX,m_aVertexIdxMap);

	//显示对象
	ShowShape(m_aShape,TVI_ROOT);
	//
	ShowShareInfo(m_aShape,TVI_ROOT);

	ShowShapeStatics(m_aShape,TVI_ROOT);

	ShowAllEdge(m_aShape,TVI_ROOT);

	//显示裁剪环信息
	if(m_aShape.ShapeType() == TopAbs_FACE){
		ShowFaceAllLoop(m_aShape,TVI_ROOT);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

// 显示一个对象的信息
//
HTREEITEM  CShapeStructDialog::ShowShape(const TopoDS_Shape& aShape,HTREEITEM hParent)
{
	HTREEITEM hItem = NULL;
	TopAbs_ShapeEnum eType = aShape.ShapeType();
	switch(eType){
		case TopAbs_COMPOUND:
		case TopAbs_COMPSOLID:
		case TopAbs_SOLID:
		case TopAbs_SHELL:
			{
				HTREEITEM hRoot = ShowShapeBaseProp(aShape,hParent);
				if(hRoot)
				{
					TopoDS_Iterator ite;
					for(ite.Initialize(aShape);ite.More();ite.Next())
					{
						ShowShape(ite.Value(),hRoot);
					}
				}
			}
			break;
		case TopAbs_FACE:
			hItem = ShowFace(aShape,hParent);
			break;
		case TopAbs_WIRE:
			hItem = ShowWire(TopoDS::Wire(aShape),hParent);
			break;
		case TopAbs_EDGE:
			hItem = ShowEdge(aShape,hParent);
			break;
		case TopAbs_VERTEX:
			hItem = ShowVertex(aShape,hParent);
			break;
		case TopAbs_SHAPE:
			break;
		default:
			break;
	}

	return hItem;
}


//
// 显示Compound。内部可能包含各种类型的shape。
//
HTREEITEM  CShapeStructDialog::ShowCompound(const TopoDS_Shape& aShape,
										  HTREEITEM hParent)
{
	if(aShape.IsNull() ||
		(aShape.ShapeType() != TopAbs_COMPOUND))
		return NULL;

	//显示基本属性
	HTREEITEM hRoot = ShowShapeBaseProp(aShape,hParent);
	if(!hRoot)
		return NULL;

	TopoDS_Iterator ite;
	ite.Initialize(aShape);
	for(;ite.More();ite.Next())
	{
		ShowShape(ite.Value(),hRoot);
	}

	return hRoot;
}

//
// 显示CompSolid，即solid的组合。包含一系列的solid。
//
HTREEITEM	CShapeStructDialog::ShowCompSolid(const TopoDS_Shape& aShape,
										   HTREEITEM hParent)
{
	if(aShape.IsNull() ||
		(aShape.ShapeType() != TopAbs_COMPSOLID))
		return NULL;

	//显示基本属性
	HTREEITEM hRoot = ShowShapeBaseProp(aShape,hParent);
	if(!hRoot)
		return NULL;

	TopExp_Explorer Ex;

	//遍历子对象
	Ex.Init(aShape,TopAbs_SOLID);
	for(;Ex.More();Ex.Next())
	{
		TopoDS_Shape aS = Ex.Current();
		ShowSolid(aS,hRoot);
	}

	return hRoot;
}

//
// 显示solid。
//
HTREEITEM	CShapeStructDialog::ShowSolid(const TopoDS_Shape& aShape,
									   HTREEITEM hParent)
{
	if(aShape.IsNull() ||
		(aShape.ShapeType() != TopAbs_SOLID))
		return NULL;

	//显示基本属性
	HTREEITEM hRoot = ShowShapeBaseProp(aShape,hParent);
	if(!hRoot)
		return NULL;

	TopExp_Explorer Ex;

	//遍历子对象
	Ex.Init(aShape,TopAbs_SHELL);
	for(;Ex.More();Ex.Next())
	{
		TopoDS_Shape aS = Ex.Current();
		ShowShell(aS,hRoot);
	}

	return hRoot;
}

//
// 显示shell。
//
HTREEITEM	CShapeStructDialog::ShowShell(const TopoDS_Shape& aShape,
									   HTREEITEM hParent)
{
	if(aShape.IsNull() ||
		(aShape.ShapeType() != TopAbs_SHELL))
		return NULL;

	//显示基本属性
	HTREEITEM hRoot = ShowShapeBaseProp(aShape,hParent);
	if(!hRoot)
		return NULL;

	TopExp_Explorer Ex;

	//遍历子对象
	Ex.Init(aShape,TopAbs_FACE);
	for(;Ex.More();Ex.Next())
	{
		TopoDS_Shape aS = Ex.Current();
		ShowFace(aS,hRoot);
	}

	return hRoot;
}

//
// 需要显示的内容比较多。包括：
//    Face基本信息
//    surface信息
//    裁剪环信息
//
HTREEITEM	CShapeStructDialog::ShowFace(const TopoDS_Shape& aShape,
									  HTREEITEM hParent)
{
	if(aShape.IsNull() ||
		(aShape.ShapeType() != TopAbs_FACE))
		return NULL;

	//显示基本属性
	HTREEITEM hRoot = ShowShapeBaseProp(aShape,hParent);
	if(!hRoot)
		return NULL;

	//显示Geom属性
	ShowFaceGeomProp(aShape,hRoot);
	//显示裁剪环属性
	ShowFaceLoopProp(aShape,hRoot);
	//
	ShowFaceMeshProp(aShape,hRoot);

	return hRoot;
}

// 显示剖分网格信息
HTREEITEM	CShapeStructDialog::ShowFaceMeshProp(const TopoDS_Shape& aShape,HTREEITEM hParent)
{
	HTREEITEM hItem = NULL;
	TopoDS_Face aFace = TopoDS::Face(aShape);
	TopLoc_Location L;

	hItem = m_ssTree.InsertItem("Mesh info:",hParent);
	Handle(Poly_Triangulation) aPoly = BRep_Tool::Triangulation(aFace,L);
	if(aPoly.IsNull())
		m_ssTree.InsertItem("no mesh triangulation",hItem);
	else
	{
		CString szStr;
		szStr.Format("Node : %d,Triangle : %d.",aPoly->NbNodes(),
			aPoly->NbTriangles());
		m_ssTree.InsertItem(szStr,hItem);
	}

	return hItem;
}

//
// 显示单独的环。非裁剪环。
//
HTREEITEM	CShapeStructDialog::ShowWire(const TopoDS_Wire& aWire,
									  HTREEITEM hParent)
{
	if(aWire.IsNull()){
		ASSERT(FALSE);
		return NULL;
	}
	//显示基本属性
	HTREEITEM hRoot = ShowShapeBaseProp(aWire,hParent);
	if(!hRoot)
		return NULL;

	//这里采用遍历子对象的方式显示
	//结合上父方向才是最终的edge的方向,这里显示最终真实的方向
	TopoDS_Iterator wite(aWire,bUseParentOri_);
	for(;wite.More();wite.Next()){
		TopoDS_Shape aS = wite.Value();
		ShowEdge(aS,hRoot);
	}

	//TopExp_Explorer Ex;
	//Ex.Init(aShape,TopAbs_EDGE);
	//for(;Ex.More();Ex.Next())
	//{
	//	TopoDS_Shape aS = Ex.Current();
	//	ShowEdge(aS,hRoot);
	//}

	return hRoot;
}



//
// 显示单独的曲线对应的edge。
//
HTREEITEM	CShapeStructDialog::ShowEdge(const TopoDS_Shape& aShape,
									  HTREEITEM hParent)
{
	//显示基本属性
	HTREEITEM hRoot = ShowShapeBaseProp(aShape,hParent);
	if(!hRoot)
		return NULL;

	HTREEITEM hItem = ShowEdgeGeomProp(aShape,hRoot);

	//显示结点信息
	TopExp_Explorer Ex;
	Ex.Init(aShape,TopAbs_VERTEX);
	for(;Ex.More();Ex.Next())
	{
		TopoDS_Shape aS = Ex.Current();
		ShowVertex(aS,hRoot);
	}

	return hRoot;
}


//
// 显示结点信息。
//
HTREEITEM  CShapeStructDialog::ShowVertex(const TopoDS_Shape& aShape,
										HTREEITEM hParent)
{
	if(aShape.IsNull()) return NULL;

	TopoDS_Vertex aV = TopoDS::Vertex(aShape);
	if(aV.IsNull())
		return NULL;

	//显示基本属性
	HTREEITEM hRoot = ShowShapeBaseProp(aShape,hParent);
	if(!hRoot)
		return NULL;

	//获取结点信息
	gp_Pnt pnt = BRep_Tool::Pnt(aV);
	CString szItem;
	szItem.Format("point [%.4f,%.4f,%.4f]",pnt.X(),pnt.Y(),pnt.Z());
	m_ssTree.InsertItem(szItem,hRoot);

	//容差
	double vtol = BRep_Tool::Tolerance(aV);
	szItem.Format("tolerance : %f",vtol);
	m_ssTree.InsertItem(szItem,hRoot);

	const Handle(BRep_TVertex) &aTV = Handle(BRep_TVertex)::DownCast(aShape.TShape());
	const BRep_ListOfPointRepresentation& pntRepList = aTV->Points();
	szItem.Format("pnt rep num : %d",pntRepList.Extent());
	m_ssTree.InsertItem(szItem,hRoot);

	return hRoot;
}

//
// 显示shape的基本信息，包括：方向、位置、内部指针等。
// 尽量在一行显示足够多的信息。
//
HTREEITEM	CShapeStructDialog::ShowShapeBaseProp(const TopoDS_Shape& aShape,
											   HTREEITEM hParent)
{
	HTREEITEM hItem = NULL;
	int ts = (int)aShape.ShapeType();
	CString szItem = s_shapeType[ts];

	//给face和edge编号，由于是
	if(aShape.ShapeType() == TopAbs_FACE){
		int idx = m_aFaceIdxMap.FindIndex(aShape);
		szItem.Format("FACE %d",idx);
	}else if(aShape.ShapeType() == TopAbs_EDGE){
		int idx = m_aEdgeIdxMap.FindIndex(aShape);
		szItem.Format("EDGE %d",idx);
	}else if(aShape.ShapeType() == TopAbs_VERTEX){
		int idx = m_aVertexIdxMap.FindIndex(aShape);
		szItem.Format("VERTEX %d",idx);
	}
	hItem = m_ssTree.InsertItem(szItem,hParent);
    
	CString szStr;
	//低层指针
	Handle(TopoDS_TShape) aTS = aShape.TShape();
	szStr.Format("TShape : 0x%08X",(DWORD)aTS.get());
	m_ssTree.InsertItem(szStr,hItem);

	//方向	
	szStr.Format("Orient : %s",s_oritDesc[(int)aShape.Orientation()]);
	m_ssTree.InsertItem(szStr,hItem);

	//位置
	if(aShape.Location().IsIdentity())
		szStr = "Location : Identity";
	else
		szStr = "Location : NotIdentity";
	m_ssTree.InsertItem(szStr,hItem);

	//面积或长度属性
	if(aShape.ShapeType() == TopAbs_FACE){
		GProp_GProps sProps;
		BRepGProp::SurfaceProperties(aShape,sProps);
		double area = sProps.Mass();
		szStr.Format("face area : %f",area);
		m_ssTree.InsertItem(szStr,hItem);
	}else if(aShape.ShapeType() == TopAbs_EDGE){
		double edgeTol = BRep_Tool::Tolerance(TopoDS::Edge(aShape));
		szStr.Format("tolerance : %f",edgeTol);
		m_ssTree.InsertItem(szStr,hItem);
	}else{
	}

	if(aShape.ShapeType() == TopAbs_FACE){
		double umin,umax,vmin,vmax;
		TopoDS_Face aFace = TopoDS::Face(aShape);
		
		BRepTools::UVBounds(aFace,umin,umax,vmin,vmax);
		szStr.Format("BRepTol_UVBounds : U[%f - %f], V[%f - %f]",umin,umax,vmin,vmax);
		m_ssTree.InsertItem(szStr,hItem);

		ShapeAnalysis::GetFaceUVBounds(aFace,umin,umax,vmin,vmax);
		szStr.Format("ShpAlys_UVBounds : U[%f - %f], V[%f - %f]",umin,umax,vmin,vmax);
		m_ssTree.InsertItem(szStr,hItem);
	}

	return hItem;
}

//
// 显示face的geom属性。主要是surface的属性。
//
HTREEITEM	CShapeStructDialog::ShowFaceGeomProp(const TopoDS_Shape& aShape,
											  HTREEITEM hParent)
{
	Handle(Geom_Surface) aSurf = BRep_Tool::Surface(TopoDS::Face(aShape));
	if(aSurf.IsNull())
		return NULL;

	//显示surface的属性
	ShowSurface(aSurf,hParent);

	return hParent;
}

//
// 显示surface的信息
//
HTREEITEM	CShapeStructDialog::ShowSurface(const Handle(Geom_Surface)& aSurf,HTREEITEM hParent)
{
	if(aSurf.IsNull())
		return NULL;

	HTREEITEM hRoot = NULL;
	CString szItem;

	//名称
	Handle(Standard_Type) aType = aSurf->DynamicType();
	CString szText = aType->Name();
	hRoot = m_ssTree.InsertItem(szText,hParent);

	//基本信息
	ShowSurfaceBaseProp(aSurf,hRoot);

	if(aSurf->IsKind(STANDARD_TYPE(Geom_BoundedSurface))){
		if(aSurf->IsKind(STANDARD_TYPE(Geom_BSplineSurface))){
			//
			DeclareAndCast(Geom_BSplineSurface,aBspSurf,aSurf);
			this->ShowBSplineSurface(aBspSurf,hRoot);
		}else if(aSurf->IsKind(STANDARD_TYPE(Geom_BezierSurface))){
			//
//			this->ShowBezierSurface(aSurf,hRoot);
		}else if(aSurf->IsKind(STANDARD_TYPE(Geom_RectangularTrimmedSurface))){
			
			DeclareAndCast(Geom_RectangularTrimmedSurface,aTrimSurf,aSurf);
			ShowRectTrimmedSurface(aTrimSurf,hRoot);
		}
	}
	else if(aSurf->IsKind(STANDARD_TYPE(Geom_ElementarySurface)))
	{
		DeclareAndCast(Geom_ElementarySurface,aEleSurf,aSurf);
		ShowElementCommonProp(aEleSurf,hParent);
		if(aSurf->IsKind(STANDARD_TYPE(Geom_Plane)))
		{
			DeclareAndCast(Geom_Plane,aPln,aSurf);
			ShowPlane(aPln,hParent);
		}
		else if(aSurf->IsKind(STANDARD_TYPE(Geom_CylindricalSurface)))
		{
			DeclareAndCast(Geom_CylindricalSurface,aCylSurf,aSurf);
			ShowCylinder(aCylSurf,hParent);
		}
		else if(aSurf->IsKind(STANDARD_TYPE(Geom_ConicalSurface)))
		{
		}
		else if(aSurf->IsKind(STANDARD_TYPE(Geom_SphericalSurface)))
		{
		}
		else if(aSurf->IsKind(STANDARD_TYPE(Geom_ToroidalSurface)))
		{
		}
	}
	else if(aSurf->IsKind(STANDARD_TYPE(Geom_SweptSurface)))
	{
		if(aSurf->IsKind(STANDARD_TYPE(Geom_SurfaceOfLinearExtrusion)))
		{
			DeclareAndCast(Geom_SurfaceOfLinearExtrusion,aLinExtSurf,aSurf);
			ShowLinearExtSurf(aLinExtSurf,hRoot);
		}
		else if(aSurf->IsKind(STANDARD_TYPE(Geom_SurfaceOfRevolution)))
		{
			DeclareAndCast(Geom_SurfaceOfRevolution,aRevSurf,aSurf);
			ShowRevolutionSurf(aRevSurf,hRoot);
		}
	}
	else if(aSurf->IsKind(STANDARD_TYPE(Geom_OffsetSurface)))
	{
	}


	return hRoot;
}

// 显示曲面的基本信息
void  CShapeStructDialog::ShowSurfaceBaseProp(const Handle(Geom_Surface)& aSurf,HTREEITEM hParent)
{
	CString str;
	double U1,U2,V1,V2;

	//参数域范围
	aSurf->Bounds(U1,U2,V1,V2);
	str.Format("U1 : %f,U2 : %f,V1 : %f,V2 : %f",U1,U2,V1,V2);
	m_ssTree.InsertItem(str,hParent);

	//闭合和周期性
	int uClose = aSurf->IsUClosed() ? 1 : 0;
	int vClose = aSurf->IsVClosed() ? 1 : 0;
	int uPeriodic = aSurf->IsUPeriodic() ? 1 : 0;
	int vPeriodic = aSurf->IsVPeriodic() ? 1 : 0;
	double uPeriod = 0.0,vPeriod = 0.0;
	if(uPeriodic){
		uPeriod = aSurf->UPeriod();
	}
	if(vPeriodic){
		vPeriod = aSurf->VPeriod();
	}

	if(uPeriodic || vPeriodic){
		str.Format("uClose : %d,vClose : %d,uPeriodic : %d,vPeriodic : %d,uPeriod : %f,vPeriod : %f",
			uClose,vClose,uPeriodic,vPeriodic,uPeriod,vPeriod);
	}else{
		str.Format("uClose : %d,vClose : %d,uPeriodic : %d,vPeriodic : %d",
			uClose,vClose,uPeriodic,vPeriodic);
	}
	m_ssTree.InsertItem(str,hParent);
}

HTREEITEM  CShapeStructDialog::ShowBSplineSurface(const Handle(Geom_BSplineSurface)& aBSpSurf,HTREEITEM hParent)
{
	if(aBSpSurf.IsNull())
		return NULL;

	HTREEITEM hItem = NULL;
	int udeg,vdeg,upoles,vpoles,uperiodic = 0,vperiodic = 0,urational = 0,vrational = 0;
	int nuknots,nvknots;

	//基本信息
	udeg = aBSpSurf->UDegree();
	vdeg = aBSpSurf->VDegree();
	upoles = aBSpSurf->NbUPoles();
	vpoles = aBSpSurf->NbVPoles();
	nuknots = aBSpSurf->NbUKnots();
	nvknots = aBSpSurf->NbVKnots();
	uperiodic = aBSpSurf->IsUPeriodic() ? 1 : 0;
	vperiodic = aBSpSurf->IsVPeriodic() ? 1 : 0;
	urational = aBSpSurf->IsURational() ? 1 : 0;
	vrational = aBSpSurf->IsVRational() ? 1 : 0;

	CString szText;
	szText.Format("udeg=%d,upoles=%d,vdeg=%d,vpoles=%d",udeg,upoles,vdeg,vpoles);
	m_ssTree.InsertItem(szText,hParent);
	szText.Format("uper=%d,urat=%d,vper=%d,vrat=%d",uperiodic,urational,vperiodic,vrational);
	m_ssTree.InsertItem(szText,hParent);
	szText.Format("NbUKnots=%d, NbVKnots=%d",nuknots,nvknots);
	m_ssTree.InsertItem(szText,hParent);



	const TColStd_Array1OfReal& uks = aBSpSurf->UKnotSequence();
	const TColStd_Array1OfReal& vks = aBSpSurf->VKnotSequence();

	szText.Format("U Flat Knots (%d):",uks.Length());
	hItem = m_ssTree.InsertItem(szText,hParent);
	InsertKnots(uks,4,hItem);

	szText.Format("V Flat Knots (%d):",vks.Length());
	hItem = m_ssTree.InsertItem(szText,hParent);
	InsertKnots(vks,4,hItem);

	//
	hItem = m_ssTree.InsertItem("U Knots :",hParent);
	const TColStd_Array1OfReal& aUks = aBSpSurf->UKnots();
	InsertKnots(aUks,4,hItem);

	const TColStd_Array1OfInteger& ums = aBSpSurf->UMultiplicities();
	hItem = m_ssTree.InsertItem("U multis:",hParent);
	InsertMults(ums,4,hItem);

	hItem = m_ssTree.InsertItem("V Knots :",hParent);
	const TColStd_Array1OfReal& aVks = aBSpSurf->VKnots();
	InsertKnots(aVks,4,hItem);

	const TColStd_Array1OfInteger& vms = aBSpSurf->VMultiplicities();
	hItem = m_ssTree.InsertItem("V multis:",hParent);
	InsertMults(vms,4,hItem);

	//控制点
	HTREEITEM hPoleItem = NULL;
	CString ss;
	int iux,ivx;
	double weight = 0.0;
	gp_Pnt cpt;

	hPoleItem = m_ssTree.InsertItem("Poles :",hParent);
	for(iux = 1;iux <= upoles;iux ++){
		szText.Format("U %d:",iux);
		hItem = m_ssTree.InsertItem(szText,hPoleItem);
		
		for(ivx = 1;ivx <= vpoles;ivx ++){
			cpt = aBSpSurf->Pole(iux,ivx);
			if(aBSpSurf->IsURational() || aBSpSurf->IsVRational()){
				weight = aBSpSurf->Weight(iux,ivx);
				szText.Format("[%f, %f, %f, %f]",cpt.X(),cpt.Y(),cpt.Z(),weight);
			}else{
				szText.Format("[%f, %f, %f]",cpt.X(),cpt.Y(),cpt.Z());
			}
			m_ssTree.InsertItem(szText,hItem);
		}
	}

	return NULL;
}

//辅助函数
void  CShapeStructDialog::InsertKnots(const TColStd_Array1OfReal& aKnots,int npl,HTREEITEM hParent)
{
	CString szText,ss;
	int ix,n = aKnots.Length();

	szText = "";
	for(ix = 1;ix <= n;ix ++){//下标从1开始
		ss.Format(" %f",aKnots(ix));
		szText += ss;
		if(ix % npl == 0){
			m_ssTree.InsertItem(szText,hParent);
			szText = "";
		}
	}
	if(!szText.IsEmpty()){
		m_ssTree.InsertItem(szText,hParent);
	}
}

//辅助函数
void  CShapeStructDialog::InsertMults(const TColStd_Array1OfInteger& aMults,int npl,HTREEITEM hParent)
{
	CString szText,ss;
	int ix,n = aMults.Length();

	szText = "";
	for(ix = 1;ix <= n;ix ++){//下标从1开始
		ss.Format(" %d",aMults(ix));
		szText += ss;
		if(ix % npl == 0){
			m_ssTree.InsertItem(szText,hParent);
			szText = "";
		}
	}
	if(!szText.IsEmpty()){
		m_ssTree.InsertItem(szText,hParent);
	}
}

///
HTREEITEM  CShapeStructDialog::ShowRectTrimmedSurface(const Handle(Geom_RectangularTrimmedSurface) &aSurf,
													  HTREEITEM hParent)
{
	if(aSurf.IsNull())
		return NULL;

	Handle(Geom_Surface) aBasSurf = aSurf->BasisSurface();
	if(!aBasSurf.IsNull()){
		//
		ShowSurface(aBasSurf,hParent);
	}
	
	return NULL;
}

HTREEITEM  CShapeStructDialog::ShowElementCommonProp(const Handle(Geom_ElementarySurface)& aSurf,HTREEITEM hParent)
{
	CString str;
	gp_Ax3 loc = aSurf->Position();
	gp_Pnt pnt;
	gp_Dir dir;
	pnt = loc.Location();
	str.Format("org : %f,%f,%f",pnt.X(),pnt.Y(),pnt.Z());
	m_ssTree.InsertItem(str,hParent);
	dir = loc.XDirection();
	str.Format("X : %f,%f,%f",dir.X(),dir.Y(),dir.Z());
	m_ssTree.InsertItem(str,hParent);
	dir = loc.YDirection();
	str.Format("Y : %f,%f,%f",dir.X(),dir.Y(),dir.Z());
	m_ssTree.InsertItem(str,hParent);
	dir = loc.Direction();
	str.Format("Z : %f,%f,%f",dir.X(),dir.Y(),dir.Z());
	m_ssTree.InsertItem(str,hParent);

	return NULL;
}

HTREEITEM	CShapeStructDialog::ShowPlane(const Handle(Geom_Plane)& aSurf,HTREEITEM hParent)
{
	return NULL;
}

HTREEITEM	CShapeStructDialog::ShowCylinder(const Handle(Geom_CylindricalSurface)& aSurf,HTREEITEM hParent)
{
	CString str;
	str.Format("radius : %f",aSurf->Radius());
	m_ssTree.InsertItem(str,hParent);

	return NULL;
}

HTREEITEM  CShapeStructDialog::ShowLinearExtSurf(const Handle(Geom_SurfaceOfLinearExtrusion)& aSurf,HTREEITEM hParent)
{
	CString str;
	Handle(Geom_Curve) aCurve = aSurf->BasisCurve();
	ShowCurve(aCurve,hParent);

	gp_Dir dire;
	dire = aSurf->Direction();
	str.Format("direction : %.4f,%.4f,%.4f",dire.X(),dire.Y(),dire.Z());
	m_ssTree.InsertItem(str,hParent);

	return NULL;
}


HTREEITEM  CShapeStructDialog::ShowRevolutionSurf(const Handle(Geom_SurfaceOfRevolution)& aSurf,HTREEITEM hParent)
{
	Handle(Geom_Curve) aBasCurve = aSurf->BasisCurve();
	ShowCurve(aBasCurve,hParent);

	CString str;
	gp_Dir dire;
	dire = aSurf->Direction();
	str.Format("direction : %.4f,%.4f,%.4f",dire.X(),dire.Y(),dire.Z());
	m_ssTree.InsertItem(str,hParent);

	return NULL;
}

//
// 显示face的裁剪环属性。
//
HTREEITEM	CShapeStructDialog::ShowFaceLoopProp(const TopoDS_Shape& aShape,HTREEITEM hParent)
{
	if(aShape.IsNull())
		return NULL;

	TopoDS_Face aFace = TopoDS::Face(aShape);
	if(aFace.IsNull())
		return NULL;

	CString szStr;
	

	///这里，false : 显示的是wire的原始方向，不综合face的方向。
	/// true : wire的方向需综合上face的方向，才是最终的方向
	TopoDS_Iterator ite(aFace,bUseParentOri_);
	for(;ite.More();ite.Next()){
		TopoDS_Wire aW = TopoDS::Wire(ite.Value());
		ShowWire(aW,aFace,hParent);
	}

	//TopExp_Explorer Ex;
	//Ex.Init(aShape,TopAbs_WIRE);
	//for(;Ex.More();Ex.Next())
	//{
	//	TopoDS_Wire aW = TopoDS::Wire(Ex.Current());
	//	ShowWire(aW,aFace,hParent);
	//}

	return hParent;
}

//
// 显示裁剪环的wire
//
HTREEITEM	CShapeStructDialog::ShowWire(const TopoDS_Wire& aWire,
										 const TopoDS_Face& aFace,
										 HTREEITEM hParent)
{
	//显示基本属性
	HTREEITEM hRoot = ShowShapeBaseProp(aWire,hParent);
	if(!hRoot)
		return NULL;

	TopoDS_Iterator ite(aWire,bUseParentOri_);
	for(;ite.More();ite.Next()){
		TopoDS_Shape aS = ite.Value();
		ShowEdge(aS,aFace,hRoot);
	}

	//显示face的参数域的信息
	if(!aFace.IsNull()){
		Handle(ShapeExtend_WireData) aWireData = new ShapeExtend_WireData();
		aWireData->Init(aWire);
		aWireData->ComputeSeams();

		CString szItem = "Wire 3D:";
		HTREEITEM hItem = m_ssTree.InsertItem(szItem,hRoot);

		CString ss;
		int ix,nbEdge = aWireData->NbEdges();
		for(ix = 1;ix <= nbEdge;ix ++){
			TopoDS_Edge &aE = aWireData->Edge(ix);
			//
			ss = (aE.Orientation() == TopAbs_FORWARD) ? "Edge(F)" : "Edge(R)";
			//3D
			gp_Pnt pb,pe,tp;
			BRepAdaptor_Curve BAC(aE,aFace);
			BAC.D0(BAC.FirstParameter(),pb);
			BAC.D0(BAC.LastParameter(),pe);
			if(aE.Orientation() == TopAbs_REVERSED){
				tp = pb;pb = pe;pe = tp;
			}
			szItem.Format("%s : [%f, %f, %f] - [%f, %f, %f]",ss,
				pb.X(),pb.Y(),pb.Z(),pe.X(),pe.Y(),pe.Z());
			//
			if(aWireData->IsSeam(ix)){
				szItem += ", Seam Edge";
			}
			m_ssTree.InsertItem(szItem,hItem);

		}

		//2d部分
		szItem = "Wire 2D:";
		hItem = m_ssTree.InsertItem(szItem,hRoot);

		for(ix = 1;ix <= nbEdge;ix ++){
			TopoDS_Edge &aE = aWireData->Edge(ix);
						
			//2d curve
			double dFirst,dLast;
			Handle(Geom2d_Curve) aCur2d = BRep_Tool::CurveOnSurface(aE,aFace,dFirst,dLast);
			if(!aCur2d.IsNull()){
				//显示首末点
				gp_Pnt2d pb2d,pe2d,tp2d;
				aCur2d->D0(dFirst,pb2d);
				aCur2d->D0(dLast,pe2d);
				if(aE.Orientation() == TopAbs_REVERSED){
					tp2d = pb2d;pb2d = pe2d;pe2d = tp2d;
				}
				szItem.Format("Edge(%2d) : [%f, %f] - [%f, %f]",ix,pb2d.X(),pb2d.Y(),pe2d.X(),pe2d.Y());
				m_ssTree.InsertItem(szItem,hItem);
			}

		}
	}
	
	return hRoot;
}

//
//显示裁剪环中的edge,显示内容：
//    基本属性
//    3d curve
//    2d curve
//
HTREEITEM	CShapeStructDialog::ShowEdge(const TopoDS_Shape& aShape,
										 const TopoDS_Face& aFace,
										 HTREEITEM hParent)
{
	TopoDS_Edge aEdge = TopoDS::Edge(aShape);
	if(aEdge.IsNull())
		return NULL;

	//显示基本属性
	HTREEITEM hRoot = ShowShapeBaseProp(aShape,hParent);
	if(!hRoot)
		return NULL;

	CString szItem;
	double dFirst,dLast;
	Handle(Standard_Type) aType;
	Standard_Transient *pTrans;
	//3d curve
	HTREEITEM hItem = NULL;
	Handle(Geom_Curve) aCurve = BRep_Tool::Curve(aEdge,dFirst,dLast);
	if(!aCurve.IsNull())
	{
		pTrans = aCurve.get();
		aType = aCurve->DynamicType();
		szItem.Format("3D : %s [0x%08X],[%.4f - %.4f]",aType->Name(),(DWORD)pTrans,dFirst,dLast);
		hItem = m_ssTree.InsertItem(szItem,hRoot);
		//显示首末点
		gp_Pnt pb,pe;
		aCurve->D0(dFirst,pb);
		aCurve->D0(dLast,pe);
		szItem.Format("3D Start:[%f, %f, %f], End:[%f, %f, %f]",
			pb.X(),pb.Y(),pb.Z(),pe.X(),pe.Y(),pe.Z());
		m_ssTree.InsertItem(szItem,hRoot);
		//通过curve on surface获取
		BRepAdaptor_Curve BAC(aEdge,aFace);
		BAC.D0(dFirst,pb);
		BAC.D0(dLast,pe);
		szItem.Format("CS Start:[%f, %f, %f], End:[%f, %f, %f]",
			pb.X(),pb.Y(),pb.Z(),pe.X(),pe.Y(),pe.Z());
		m_ssTree.InsertItem(szItem,hRoot);

		//显示曲线
		ShowCurve(aCurve,hItem);
	}

	//2d curve
	Handle(Geom2d_Curve) aCur2d = BRep_Tool::CurveOnSurface(aEdge,aFace,dFirst,dLast);
	if(!aCur2d.IsNull())
	{
		pTrans = aCur2d.get();
		aType = aCur2d->DynamicType();
		szItem.Format("2D : %s[0x%08X],[%.4f - %.4f]",aType->Name(),(DWORD)pTrans,dFirst,dLast);
		hItem = m_ssTree.InsertItem(szItem,hRoot);
		//显示首末点
		gp_Pnt2d pb2d,pe2d;
		aCur2d->D0(dFirst,pb2d);
		aCur2d->D0(dLast,pe2d);
		szItem.Format("2D Start:[%f, %f], End:[%f, %f]",pb2d.X(),pb2d.Y(),pe2d.X(),pe2d.Y());
		m_ssTree.InsertItem(szItem,hRoot);
		//显示曲线
		Show2DCurve(aCur2d,hItem);
	}

	TopExp_Explorer Ex;
	Ex.Init(aShape,TopAbs_VERTEX);
	for(;Ex.More();Ex.Next())
	{
		TopoDS_Shape aS = Ex.Current();
		ShowVertex(aS,hRoot);
	}

	return hRoot;
}

//
// 显示curve的属性
//
HTREEITEM	CShapeStructDialog::ShowCurve(const Handle(Geom_Curve)& aCur,HTREEITEM hParent)
{
	if(aCur.IsNull()) return NULL;

	HTREEITEM hItem = NULL,hCrvItem = NULL;
	CString szItem;
	Handle(Standard_Type) aType;

	aType = aCur->DynamicType();
	szItem.Format("%s",aType->Name());
	hCrvItem = m_ssTree.InsertItem(szItem,hParent);
	
	//通用属性
	//参数域范围:
	double dFirst = aCur->FirstParameter();
	double dLast = aCur->LastParameter();
	if(Precision::IsInfinite(dFirst) && Precision::IsInfinite(dLast)){
		szItem = "param range : [inf,inf]";
	}else if(Precision::IsInfinite(dFirst)){
		szItem.Format("param range : [inf,%f]",dLast);
	}else if(Precision::IsInfinite(dLast)){
		szItem.Format("param range : [%f,inf]",dFirst);
	}else{
		szItem.Format("param range : [%f,%f]",dFirst,dLast);
	}
	m_ssTree.InsertItem(szItem,hCrvItem);

	int bClosed = aCur->IsClosed() ? 1 : 0;
	int bPeriodic = aCur->IsPeriodic() ? 1 : 0;
	double period = 0.0;
	if(bPeriodic){
		period = aCur->Period();
	}
	szItem.Format("closed : %d, periodic : %d, period : %f",bClosed,bPeriodic,period);
	m_ssTree.InsertItem(szItem,hCrvItem);
	
	Standard_Transient *pTrans;
	gp_Pnt pos;
	gp_Dir dir;
	if (aCur->IsKind(STANDARD_TYPE(Geom_BoundedCurve))) 
	{
		if (aCur->IsKind(STANDARD_TYPE(Geom_BSplineCurve))) 
		{
			DeclareAndCast(Geom_BSplineCurve,aSpline,aCur);
			ShowSplineCurve(aSpline,hCrvItem);
		}
		else if (aCur->IsKind(STANDARD_TYPE(Geom_BezierCurve))) 
		{
		}
		else if ( aCur->IsKind(STANDARD_TYPE(Geom_TrimmedCurve))) 
		{
			DeclareAndCast(Geom_TrimmedCurve,aTrimCrv,aCur);
			szItem.Format("U0 : %f, U1 : %f",aTrimCrv->FirstParameter(),aTrimCrv->LastParameter());
			m_ssTree.InsertItem(szItem,hCrvItem);
			//
			Handle(Geom_Curve) aBasCrv = aTrimCrv->BasisCurve();
			pTrans = aBasCrv.get();
			aType = aBasCrv->DynamicType();
			szItem.Format("3D : %s [0x%08X],[%.4f - %.4f]",aType->Name(),(DWORD)pTrans,
				aBasCrv->FirstParameter(),aBasCrv->LastParameter());
			hItem = m_ssTree.InsertItem(szItem,hCrvItem);
			//显示曲线
			ShowCurve(aBasCrv,hItem);
		}
	}
	else if (aCur->IsKind(STANDARD_TYPE(Geom_Conic))) 
	{
		DeclareAndCast(Geom_Conic,aConic,aCur);
		gp_Ax2 ax2 = aConic->Position();
		pos = ax2.Location();
		szItem.Format("pos : %f,%f,%f",pos.X(),pos.Y(),pos.Z());
		m_ssTree.InsertItem(szItem,hCrvItem);
		dir = ax2.XDirection();
		szItem.Format("X : %f,%f,%f",dir.X(),dir.Y(),dir.Z());
		m_ssTree.InsertItem(szItem,hCrvItem);
		dir = ax2.YDirection();
		szItem.Format("Y : %f,%f,%f",dir.X(),dir.Y(),dir.Z());
		m_ssTree.InsertItem(szItem,hCrvItem);
		dir = ax2.Direction();
		szItem.Format("Z : %f,%f,%f",dir.X(),dir.Y(),dir.Z());
		m_ssTree.InsertItem(szItem,hCrvItem);

		if (aCur->IsKind(STANDARD_TYPE(Geom_Circle))) 
		{
			DeclareAndCast(Geom_Circle,aCirc,aCur);
			szItem.Format("Radius : %f",aCirc->Radius());
			m_ssTree.InsertItem(szItem,hCrvItem);
		}
		else if (aCur->IsKind(STANDARD_TYPE(Geom_Ellipse))) 
		{
			DeclareAndCast(Geom_Ellipse,aEll,aCur);
			szItem.Format("Major Radius : %f, minor Radius : %f",
				aEll->MajorRadius(),aEll->MinorRadius());
			m_ssTree.InsertItem(szItem,hCrvItem);
		}
		else if ( aCur->IsKind(STANDARD_TYPE(Geom_Hyperbola))) 
		{
		}
		else if ( aCur->IsKind(STANDARD_TYPE(Geom_Parabola))) 
		{
		}
	}
	else if ( aCur->IsKind(STANDARD_TYPE(Geom_OffsetCurve))) 
	{
		Handle(Geom_OffsetCurve) aOffCur = Handle(Geom_OffsetCurve)::DownCast(aCur);
		if(!aOffCur.IsNull())
		{
			const Handle(Geom_Curve)& aBasCur = aOffCur->BasisCurve();
			//			ShowCurve(aBasCur,hItem);
		}
	}
	else if ( aCur->IsKind(STANDARD_TYPE(Geom_Line))) 
	{
		DeclareAndCast(Geom_Line,aLin,aCur);
		gp_Ax1 ax = aLin->Position();
		pos = ax.Location();
		szItem.Format("pos : %f,%f,%f",pos.X(),pos.Y(),pos.Z());
		m_ssTree.InsertItem(szItem,hCrvItem);
		dir = ax.Direction();
		szItem.Format("dir : %f,%f,%f",dir.X(),dir.Y(),dir.Z());
		m_ssTree.InsertItem(szItem,hCrvItem);
	}  

	return hItem;
}

HTREEITEM	CShapeStructDialog::ShowSplineCurve(const Handle(Geom_BSplineCurve)& aCrv,HTREEITEM hParent)
{
	HTREEITEM hRoot = NULL;
	CString str;
	int ix;
	gp_Pnt pnt;

	const TColStd_Array1OfReal &flatknots = aCrv->KnotSequence();

	str.Format("degree : %d",aCrv->Degree());
	m_ssTree.InsertItem(str,hParent);
	str.Format("ncpts : %d",aCrv->NbPoles());
	m_ssTree.InsertItem(str,hParent);
	str.Format("nknots : %d",aCrv->NbKnots());
	m_ssTree.InsertItem(str,hParent);
	str.Format("flat knots num : %d",flatknots.Length());
	m_ssTree.InsertItem(str,hParent);
	//
	hRoot = m_ssTree.InsertItem("knots",hParent);
	for(ix = 1;ix <= aCrv->NbKnots();ix ++){
		str.Format("%d : %f, %d",ix,aCrv->Knot(ix),aCrv->Multiplicity(ix));
		m_ssTree.InsertItem(str,hRoot);
	}
	//
	hRoot = m_ssTree.InsertItem("control points",hParent);
	for(ix = 1;ix <= aCrv->NbPoles();ix ++){
		pnt = aCrv->Pole(ix);
		str.Format("%d : %f,%f,%f,%f",ix,pnt.X(),pnt.Y(),pnt.Z(),aCrv->Weight(ix));
		m_ssTree.InsertItem(str,hRoot);
	}

	//显示flat knots
	CString ss;
	int ic = 0;
	str.Empty();
	hRoot = m_ssTree.InsertItem("flat knots",hParent);
	for(ix = flatknots.Lower();ix <= flatknots.Upper();ix ++){
		ss.Format("%f",flatknots.Value(ix));
		str += ss;
		ic ++;
		if(ic % 4 == 0){
			m_ssTree.InsertItem(str,hRoot);
			str.Empty();
		}else{
			str += " , ";
		}
	}
	if(!str.IsEmpty()){
		m_ssTree.InsertItem(str,hRoot);
	}
	return NULL;
}

// 显示参数曲线信息
HTREEITEM  CShapeStructDialog::Show2DCurve(const Handle(Geom2d_Curve)& aCur,HTREEITEM hParent)
{
	Handle(Standard_Type) aType;
	Standard_Transient *pTrans;
	HTREEITEM hItem = NULL;
	CString str;
	gp_Pnt2d pnt;
	gp_Dir2d dir;

	if(aCur->IsKind(STANDARD_TYPE(Geom2d_Line))){
		DeclareAndCast(Geom2d_Line,aLin,aCur);
		//
		pnt = aLin->Location();
		str.Format("pos : %f,%f",pnt.X(),pnt.Y());
		m_ssTree.InsertItem(str,hParent);
		dir = aLin->Direction();
		str.Format("dir : %f,%f",dir.X(),dir.Y());
		m_ssTree.InsertItem(str,hParent);
	}else if(aCur->IsKind(STANDARD_TYPE(Geom2d_Conic))){
		DeclareAndCast(Geom2d_Conic,aConic,aCur);
		//
		gp_Ax22d ax2d = aConic->Position();
		pnt = ax2d.Location();
		str.Format("pos : %f,%f",pnt.X(),pnt.Y());
		m_ssTree.InsertItem(str,hParent);
		dir = ax2d.XDirection();
		str.Format("X : %f,%f",dir.X(),dir.Y());
		m_ssTree.InsertItem(str,hParent);
		dir = ax2d.YDirection();
		str.Format("X : %f,%f",dir.X(),dir.Y());
		m_ssTree.InsertItem(str,hParent);

		if(aCur->IsKind(STANDARD_TYPE(Geom2d_Circle))){
			DeclareAndCast(Geom2d_Circle,aCirc,aCur);

			str.Format("radius : %f",aCirc->Radius());
			m_ssTree.InsertItem(str,hParent);
		}

	}else if(aCur->IsKind(STANDARD_TYPE(Geom2d_BoundedCurve))){
		if(aCur->IsKind(STANDARD_TYPE(Geom2d_TrimmedCurve))){
			DeclareAndCast(Geom2d_TrimmedCurve,aTrimCrv,aCur);
			str.Format("U0 : %f, U1 : %f",aTrimCrv->FirstParameter(),aTrimCrv->LastParameter());
			m_ssTree.InsertItem(str,hParent);
			//
			Handle(Geom2d_Curve) aBasCrv = aTrimCrv->BasisCurve();
			pTrans = aBasCrv.get();
			aType = aBasCrv->DynamicType();
			str.Format("3D : %s [0x%08X],[%.4f - %.4f]",aType->Name(),(DWORD)pTrans,
				aBasCrv->FirstParameter(),aBasCrv->LastParameter());
			hItem = m_ssTree.InsertItem(str,hParent);
			//显示曲线
			Show2DCurve(aBasCrv,hItem);
		}
		else if(aCur->IsKind(STANDARD_TYPE(Geom2d_BSplineCurve))){
			DeclareAndCast(Geom2d_BSplineCurve,aSplCrv2d,aCur);
			ShowSpline2DCurve(aSplCrv2d,hParent);
		}
		else if(aCur->IsKind(STANDARD_TYPE(Geom2d_BezierCurve))){

		}
	}else if(aCur->IsKind(STANDARD_TYPE(Geom2d_OffsetCurve))){

	}else{

	}

	return NULL;
}

HTREEITEM  CShapeStructDialog::ShowSpline2DCurve(const Handle(Geom2d_BSplineCurve)& aCrv,HTREEITEM hParent)
{
	HTREEITEM hRoot = NULL;
	CString str;
	int ix;
	gp_Pnt2d pnt;

	str.Format("degree : %d",aCrv->Degree());
	m_ssTree.InsertItem(str,hParent);
	str.Format("ncpts : %d",aCrv->NbPoles());
	m_ssTree.InsertItem(str,hParent);
	str.Format("nknots : %d",aCrv->NbKnots());
	m_ssTree.InsertItem(str,hParent);
	//
	hRoot = m_ssTree.InsertItem("knots",hParent);
	for(ix = 1;ix <= aCrv->NbKnots();ix ++){
		str.Format("%d : %f, %d",ix,aCrv->Knot(ix),aCrv->Multiplicity(ix));
		m_ssTree.InsertItem(str,hRoot);
	}
	//
	hRoot = m_ssTree.InsertItem("control points",hParent);
	for(ix = 1;ix <= aCrv->NbPoles();ix ++){
		pnt = aCrv->Pole(ix);
		str.Format("%d : %f,%f,%f",ix,pnt.X(),pnt.Y(),aCrv->Weight(ix));
		m_ssTree.InsertItem(str,hRoot);
	}

	return NULL;
}

//
// 显示Edge的geom属性。
//
HTREEITEM  CShapeStructDialog::ShowEdgeGeomProp(const TopoDS_Shape& aShape,
											  HTREEITEM hParent)
{
	CString szItem;
	double dFirst,dLast;
	Handle(Standard_Type) aType;
	Standard_Transient *pTrans;
	TopoDS_Edge aEdge = TopoDS::Edge(aShape);
	//3d curve
	HTREEITEM hItem = NULL;
	Handle(Geom_Curve) aCurve = BRep_Tool::Curve(aEdge,dFirst,dLast);
	if(!aCurve.IsNull())
	{
		pTrans = aCurve.get();
		aType = aCurve->DynamicType();
		szItem.Format("3D : %s [0x%08X],[%.4f - %.4f]",aType->Name(),(DWORD)pTrans,dFirst,dLast);
		hItem = m_ssTree.InsertItem(szItem,hParent);
		//显示曲线
		ShowCurve(aCurve,hItem);
	}else{
		m_ssTree.InsertItem("Null Curve",hParent);
	}

	//2d curve
	Handle(Geom2d_Curve) aCur2d;
	Handle(Geom_Surface) aSurf;
	TopLoc_Location loc;
	BRep_Tool::CurveOnSurface(aEdge,aCur2d,aSurf,loc,dFirst,dLast);
	if(!aCur2d.IsNull())
	{
		pTrans = aCur2d.get();
		aType = aCur2d->DynamicType();
		szItem.Format("2D : %s[0x%08X],[%.4f - %.4f]",aType->Name(),(DWORD)pTrans,dFirst,dLast);
		hItem = m_ssTree.InsertItem(szItem,hParent);
		//显示曲线
		Show2DCurve(aCur2d,hItem);
	}

	//return hItem;

	const Handle(BRep_TEdge)& TE = *((Handle(BRep_TEdge)*) &aShape.TShape());
	const BRep_ListOfCurveRepresentation& lcr = TE->Curves();

	HTREEITEM hRoot = m_ssTree.InsertItem("Edge curve repr",hParent);
	//个数
	szItem.Format("curve repr num : %d",lcr.Extent());
	m_ssTree.InsertItem(szItem,hRoot);

	//CString szItem;
	Handle(BRep_CurveRepresentation) cr;
	Handle(BRep_GCurve) GC;
	Standard_Real f,l;
	HTREEITEM hCRItem = NULL;
	int ic = 0;
	BRep_ListIteratorOfListOfCurveRepresentation itcr(lcr);
	while (itcr.More()) 
	{
		Handle(BRep_CurveRepresentation)& cr = itcr.Value();
		Handle(Standard_Type) aType = itcr.Value()->DynamicType();
		szItem.Format("%s",aType->Name());
		hCRItem = m_ssTree.InsertItem(szItem,hRoot);

		GC = Handle(BRep_GCurve)::DownCast(itcr.Value());
		if(!GC.IsNull())
		{
			GC->Range(f,l);
			if(GC->IsCurve3D())
			{
				szItem.Format("3D Curve ,range[%.4f,%.4f]",f,l);
				HTREEITEM hItem = m_ssTree.InsertItem(szItem,hCRItem);
				//3d曲线
				//const Handle(Geom_Curve)& hcr = GC->Curve3D();
				//ShowCurve(hcr,hItem);
			}
			else if(GC->IsCurveOnClosedSurface())
			{
				szItem.Format("Curve On Closed Surface,range[%.4f,%.4f]",f,l);
				HTREEITEM hItem = m_ssTree.InsertItem(szItem,hCRItem);

				//const Handle(Geom_Surface)& hsr = GC->Surface();
				//ShowSurface(hsr,hItem);
				//const Handle(Geom2d_Curve)& h2cr = GC->PCurve();
				//Show2dCurve(h2cr,hItem);
				//const Handle(Geom2d_Curve)& h2cr2 = GC->PCurve();
				//Show2dCurve(h2cr2,hItem);
			}
			else if(GC->IsCurveOnSurface())
			{
				szItem.Format("Curve On Surface,range[%.4f,%.4f]",f,l);
				HTREEITEM hItem = m_ssTree.InsertItem(szItem,hCRItem);

				//const Handle(Geom_Surface)& hsr = GC->Surface();
				//ShowSurface(hsr,hItem);
				//const Handle(Geom2d_Curve)& h2cr = GC->PCurve();
				//Show2dCurve(h2cr,hItem);
			}
			else
			{
				//szItem.Format("other type curve");
				//m_ssTree.InsertItem(szItem,hRoot);
			}
		}else{
			//其他类型的曲线
			Handle(BRep_CurveOn2Surfaces) aCO2S = Handle(BRep_CurveOn2Surfaces)::DownCast(itcr.Value());
			if(!aCO2S.IsNull()){
				szItem.Format("Curve On Two Surface,continuty is %d",(int)aCO2S->Continuity());
				m_ssTree.InsertItem(szItem,hCRItem);
			}
		}
		itcr.Next();
	}

	//return hRoot;
	return NULL;
}

/// 遍历类型，将对象记录到map中，并统计出现的次数
///
void	CShapeStructDialog::MapShape(const TopoDS_Shape& aShape,TopTools_DataMapOfShapeInteger& aMap,TopAbs_ShapeEnum stype)
{
	aMap.Clear();

	TopoDS_Shape aSS;
	TopExp_Explorer ex;
	for(ex.Init(aShape,stype);ex.More();ex.Next()){
		aSS = ex.Current();
		if(aMap.IsBound(aSS)){
			int& nCount = aMap.ChangeFind(aSS);
			nCount ++;
		}else{
			aMap.Bind(aSS,1);
		}
	}

	//TopTools_DataMapIteratorOfDataMapOfShapeInteger ite;
	//for(ite.Initialize(aMap);ite.More();ite.Next()){
	//	KTRACE("\n value is %d.",ite.Value());
	//}
}

// 显示共享信息
HTREEITEM  CShapeStructDialog::ShowShareInfo(const TopoDS_Shape& aShape,HTREEITEM hParent)
{
	HTREEITEM hRoot = NULL,hVRoot = NULL,hERoot = NULL;

	m_aVertexCntMap.Clear();
	m_aEdgeCntMap.Clear();
	m_aVEMap.Clear();
	m_aEFMap.Clear();

	MapShape(m_aShape,m_aVertexCntMap,TopAbs_VERTEX);
	MapShape(m_aShape,m_aEdgeCntMap,TopAbs_EDGE);
	TopExp::MapShapesAndAncestors(aShape,TopAbs_VERTEX,TopAbs_EDGE,m_aVEMap);
	TopExp::MapShapesAndAncestors(aShape,TopAbs_EDGE,TopAbs_FACE,m_aEFMap);
	
	hRoot = m_ssTree.InsertItem("共享信息",hParent);
	hVRoot = m_ssTree.InsertItem("Vertex共享信息",hRoot);
	hERoot = m_ssTree.InsertItem("Edge共享信息",hRoot);

	//显示vertex的共享信息
	ShowVertexShareInfo(hVRoot);
	ShowEdgeShareInfo(hERoot);

	return hRoot;
}

// 显示所有vertex的共享信息
//
void  CShapeStructDialog::ShowVertexShareInfo(HTREEITEM hParent)
{
	HTREEITEM hItem = NULL,hSubItem = NULL;
	TopoDS_Shape aVS;
	CString szVal;
	TopTools_DataMapIteratorOfDataMapOfShapeInteger ite;
	
	for(ite.Initialize(m_aVertexCntMap);ite.More();ite.Next()){
		aVS = ite.Key();
		//获取邻接的edge列表
		const TopTools_ListOfShape& shapeList = m_aVEMap.FindFromKey(aVS);
		if(!shapeList.IsEmpty()){
			hItem = m_ssTree.InsertItem("Vertex:",hParent);
			szVal.Format("Vertex share count : %d.",ite.Value());
			m_ssTree.InsertItem(szVal,hItem);

			ShowVertex(aVS,hItem);

			hSubItem = m_ssTree.InsertItem("Shared by Edge:",hItem);
			TopTools_ListIteratorOfListOfShape lls;
			for(lls.Initialize(shapeList);lls.More();lls.Next()){
				TopoDS_Shape aSS = lls.Value();
				ShowEdge(aSS,hSubItem);
			}
		}
	}
}

// 显示edge的共享face信息
//
void  CShapeStructDialog::ShowEdgeShareInfo(HTREEITEM hParent)
{
	HTREEITEM hItem = NULL,hSubItem = NULL;
	TopoDS_Shape aES;
	CString szVal;
	TopTools_DataMapIteratorOfDataMapOfShapeInteger ite;

	for(ite.Initialize(m_aEdgeCntMap);ite.More();ite.Next()){
		aES = ite.Key();
		//获取邻接的face列表
		const TopTools_ListOfShape& shapeList = m_aEFMap.FindFromKey(aES);
		if(!shapeList.IsEmpty()){
			hItem = m_ssTree.InsertItem("Edge:",hParent);
			szVal.Format("Edge share count : %d.",ite.Value());
			m_ssTree.InsertItem(szVal,hItem);

			//显示edge信息
			ShowEdge(aES,hItem);

			hSubItem = m_ssTree.InsertItem("Shared by Face:",hItem);
			TopTools_ListIteratorOfListOfShape lls;
			for(lls.Initialize(shapeList);lls.More();lls.Next()){
				TopoDS_Shape aSS = lls.Value();
				ShowFace(aSS,hSubItem);
			}
		}
	}
}

// 显示统计信息
//
HTREEITEM  CShapeStructDialog::ShowShapeStatics(const TopoDS_Shape& aShape,HTREEITEM hParent)
{
	std::map<std::string,int> surfCntMap;
	std::map<std::string,int>::iterator ite;
	TopoDS_Face aFace;
	TopLoc_Location loc;
	std::string szType = "";
	TopExp_Explorer ex;

	for(ex.Init(aShape,TopAbs_FACE);ex.More();ex.Next()){
		aFace = TopoDS::Face(ex.Current());
		//
		Handle(Geom_Surface) aSurf = BRep_Tool::Surface(aFace,loc);
		if(aSurf.IsNull())
			continue;

		Handle(Standard_Type) aType = aSurf->DynamicType();
		szType = aType->Name();
		//
		ite = surfCntMap.find(szType);
		if(ite == surfCntMap.end()){
			surfCntMap.insert(std::map<std::string,int>::value_type(szType,1));
		}else{
			(*ite).second ++;
		}
	}

	//插入到树中
	HTREEITEM hItem = NULL;
	CString str;
	hItem = m_ssTree.InsertItem("Surface Status",hParent);
	//
	for(ite = surfCntMap.begin();ite != surfCntMap.end();ite ++){
		str.Format("%s : %d",(*ite).first.c_str(),(*ite).second);
		m_ssTree.InsertItem(str,hItem);
	}

	return NULL;
}

// 显示shape中的所有edge信息
//
HTREEITEM  CShapeStructDialog::ShowAllEdge(const TopoDS_Shape& aShape,HTREEITEM hParent)
{
	HTREEITEM hItem = NULL;
	TopExp_Explorer ex;

	hItem = m_ssTree.InsertItem("All Edges",hParent);
	for(ex.Init(aShape,TopAbs_EDGE);ex.More();ex.Next()){
		TopoDS_Edge aEdge = TopoDS::Edge(ex.Current());
		ShowEdge(aEdge,hItem);
	}
	
	return NULL;
}

//显示Face的裁剪环信息，如果是反向，则同时显示正向和反向信息
HTREEITEM  CShapeStructDialog::ShowFaceAllLoop(const TopoDS_Shape& aShape,HTREEITEM hParent)
{
	HTREEITEM hItem = NULL,hLpItem = NULL;
	TopoDS_Iterator aIte;
	TopoDS_Face aFace = TopoDS::Face(aShape);
	int ic = 0;
	CString szText;

	hItem = m_ssTree.InsertItem("Face loops",hParent);
	//
	aIte.Initialize(aShape,true);
	for(;aIte.More();aIte.Next()){
		const TopoDS_Wire &aW = TopoDS::Wire(aIte.Value());
		szText.Format("Wire %d (%s)",ic++,(aW.Orientation() == TopAbs_REVERSED)? "R" : "F");
		hLpItem = m_ssTree.InsertItem(szText,hItem);
		//显示Wire
		ShowWireOrderedVtx(aW,hLpItem);
	}

	//是否反向
	if(aShape.Orientation() == TopAbs_REVERSED){
		TopoDS_Face aFF = aFace;
		aFF.Orientation(TopAbs_FORWARD);

		HTREEITEM hSubItem = m_ssTree.InsertItem("Forward loops",hItem);

		ic = 0;
		aIte.Initialize(aFF,true);
		for(;aIte.More();aIte.Next()){
			const TopoDS_Wire &aW = TopoDS::Wire(aIte.Value());
			szText.Format("Wire %d (%s)",ic++,(aW.Orientation() == TopAbs_REVERSED)? "R" : "F");
			hLpItem = m_ssTree.InsertItem(szText,hSubItem);
			//显示Wire
			ShowWireOrderedVtx(aW,hLpItem);
		}
	}

	return hItem;
}

HTREEITEM  CShapeStructDialog::ShowWireOrderedVtx(const TopoDS_Wire &aW,HTREEITEM hParent)
{
	TopoDS_Iterator aIte;
	TopoDS_Vertex aVF,aVL;
	gp_Pnt pb,pe;
	CString szText;
	int ic = 0;

	aIte.Initialize(aW,true);
	for(;aIte.More();aIte.Next()){
		const TopoDS_Edge &aE = TopoDS::Edge(aIte.Value());
		//
		TopExp::Vertices(aE,aVF,aVL,true);
		pb = BRep_Tool::Pnt(aVF);
		pe = BRep_Tool::Pnt(aVL);
		szText.Format("E %d (%s): (%.3f,%.3f,%.3f) -> (%.3f,%.3f,%.3f)",ic++,
			(aE.Orientation() == TopAbs_REVERSED)? "R" : "F",
			pb.X(),pb.Y(),pb.Z(),pe.X(),pe.Y(),pe.Z());
		m_ssTree.InsertItem(szText,hParent);
	}

	return NULL;
}

#if 0
///////////////////////////////////////////////////////////////////////////
//
#include "BsHandle.h"

class SA : public BsSharedObject{
public:
	SA() { a_ = 1;b_ = 2;}

protected:
	int a_,b_;
};

class SB : public SA{
public:
	SB() { c_ = 3; }

	void Add(int n)
	{
		c_ += n;
	}

protected:
	int c_;
};

class SC : public SA{
public:
	SC() { d_ = 4;}

	void Print()
	{
		d_ *= 2;
	}

protected:
	int d_;
};

void test1()
{
	BsHandle<SA> ah,ah2;
	BsHandle<SB> bh = new SB();

	ah = bh;
	ah2 = bh;
	ah2 = new SC();
}

void other_test()
{
	if(std::is_base_of<SB,SB>::value){
		TRACE("\n true.");
	}
	if(std::is_same<SA,SB>::value){
	}

	////std::enable_if<std::is_base_of<SB,SC>::value>::type a;
	//std::enable_if<std::is_base_of<SB,SB>::value,int>::type a = 10;

	//std::enable_if <std::is_same <SA,SB>::value>::type *b = 0;

 //    std::enable_if <is_base_but_not_same <SB,SB>::value, void*>::type aTypeCheckHelperVar;


	// is_base_but_not_same <SB,SB>::value 

}

void test2()
{
	BsHandle<SA> ah = new SB();
	SA *pC = new SC();

	BsHandle<SB> bh = BsHandle<SB>::DownCast(ah);
	bh->Add(5);

	BsHandle<SC> ch = BsHandle<SC>::DownCast(pC);
	ch->Print();

	BsHandle<SA> h2(bh);
}

void test3()
{
	std::map<BsHandle<SA>,int> aMap;
	std::map<BsHandle<SA>,int>::iterator ite;
	std::vector<BsHandle<SA> > hVec;
	BsHandle<SA> ah;
	BsHandle<SB> bh,bh2;
	BsHandle<SC> ch,ch2;
	int ix;

	for(ix = 0;ix < 5;ix ++){
		bh = new SB();
		hVec.push_back(bh);
		bh2 = bh;
		hVec.push_back(bh2);
	
		ch = new SC();
		hVec.push_back(ch);
		ch2 = ch;
		hVec.push_back(ch2);
	}

	std::vector<BsHandle<SA> >::size_type idx,isz = hVec.size();
	for(idx = 0;idx < isz;idx ++){
		ah = hVec[idx];
		
		ite = aMap.find(ah);
		if(ite == aMap.end()){
			aMap.insert(std::map<BsHandle<SA>,int>::value_type(ah,1));
		}else{
			(*ite).second ++;
		}
	}

	for(ite = aMap.begin();ite != aMap.end();ite ++){
		TRACE("\n %x cnt %d",(*ite).first.get(),(*ite).second);
	}

}

class Shape{
public:
	Shape() { flag_ = 0;}

	void SetSA(const BsHandle<SA>& sa)
	{
		sa_ = sa;
	}

	void SetFlag(int flag)
	{
		flag_ = flag;
	}

protected:
	BsHandle<SA> sa_;
	int flag_;
};

void test4()
{
	BsHandle<SA> sa = new SA();
	Shape S1;
	S1.SetSA(sa);

	{
		Shape S2 = S1;
		S2.SetFlag(1);

	}
}

#include "BopColIndexedMap.h"

void test5()
{
	BopColIndexedMap<int> aMap;
	aMap.Add(12);
	aMap.Add(15);
	aMap.Add(2);
	aMap.Add(8);
	int ix,sz = aMap.Extent();
	for(ix = 0;ix < sz;ix ++){
		TRACE("\n index %d,value %d .",ix,aMap.FindKey(ix));
	}
}

void test6()
{
	std::vector<int> vv;
	std::vector<int>::size_type ix,isz;

	vv.push_back(2);
	vv.push_back(3);
	TRACE("\n size %d,cap %d.\n",vv.size(),vv.capacity());
	vv.resize(8);
	vv[2] = 10;
	vv[3] = 12;
	TRACE("\n size %d,cap %d.\n",vv.size(),vv.capacity());

	isz = vv.size();
	for(ix = 0;ix < isz;ix ++){
		TRACE("\n %d value %d",ix,vv[ix]);
	}
}

void test7()
{
	std::vector<int> vv1,vv2;
	std::vector<int>::size_type ix,isz;

	vv1.push_back(10);
	vv1.push_back(20);

	vv2 = vv1;
	isz = vv2.size();
	for(ix = 0;ix < isz;ix ++){
		TRACE("\n %d value vv1 %d,vv2 %d",ix,vv1[ix],vv2[ix]);
	}

	vv1.clear();
	vv1.push_back(10);
	vv1.push_back(20);

	std::vector<int> vv3(vv1);
	isz = vv3.size();
	for(ix = 0;ix < isz;ix ++){
		TRACE("\n %d value vv1 %d,vv3 %d",ix,vv1[ix],vv3[ix]);
	}
}

class SObj{
public:
	SObj() { n_ = 0;}

	int  Val() const { return n_; }
	void Inc() { n_ ++; }

protected:
	int n_;
};

void test8()
{
	std::map<int,SObj> aMap;
	std::map<int,SObj>::iterator ite;
	std::map<int,SObj>::const_iterator cite;
	SObj sa;

	aMap.insert(std::make_pair(10,sa));
	aMap.insert(std::make_pair(20,sa));
	aMap.insert(std::make_pair(30,sa));

	cite = aMap.find(10);
	//SObj &o = (*cite).second;
	//o.Inc();

	ite = aMap.find(20);
	SObj &obj = (*ite).second;
	obj.Inc();

	ite = aMap.find(30);
	SObj &obj2 = (*ite).second;
	obj2.Inc();
	obj2.Inc();

	for(cite = aMap.begin();cite != aMap.end();cite ++){
		TRACE("\n value %d.",(*cite).second.Val());
	}

}

class MObj{
public:

	void Get(int key,int& val) const{
		//std::map<int,int>::iterator ite = aMap_.find(key);
	}

protected:
	std::map<int,int> aMap_;
};

#include <list>

void test9()
{
	std::list<int> aList;
	std::list<int>::iterator ite;
	int ix;
	for(ix = 0;ix < 8;ix ++){
		aList.push_back(ix);
	}

	ite = aList.begin();
	while(ite != aList.end()){
		if((*ite) % 2 == 0){
			ite = aList.erase(ite);
			continue;
		}
		++ite;
	}

	for(ite = aList.begin();ite != aList.end();++ite){
		TRACE("\n %d",*ite);
	}
}

#include <unordered_map>

void test10()
{
	std::vector<int> ary;
	int ix;

	for(ix = 0;ix < 10;ix ++){
		ary.push_back(ix*2);
	}

	ary.erase(ary.begin());
	TRACE("\n");
	for(ix = 0;ix < (int)ary.size();ix ++){
		TRACE("<%d> : %d\t",ix,ary[ix]);
	}
	TRACE("\n");
	ary.erase(ary.begin() + 5);
	for(ix = 0;ix < (int)ary.size();ix ++){
		TRACE("<%d> : %d\t",ix,ary[ix]);
	}
	ary.insert(ary.begin() + 4,100);
	for(ix = 0;ix < (int)ary.size();ix ++){
		TRACE("<%d> : %d\t",ix,ary[ix]);
	}
	TRACE("\n");
}

void test11()
{
	CFileFind aFind,aFind2;
	CString ss("d:\\temp"),ss2("d:\\temp");
	ss += _T("\\*.*");
	ss2 += _T("/*.*");
	BOOL bOk = aFind.FindFile(ss);
	if(bOk){
		bOk = aFind.FindNextFile();
	}
	bOk = aFind2.FindFile(ss2);
	if(bOk){
		int i = 1;
		i++;
	}
}

void try_test()
{
	//test11();
	void *p = malloc(20);
	if(p){
		int i = 100;
	}
}

#endif