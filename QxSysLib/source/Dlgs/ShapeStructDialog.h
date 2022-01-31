#pragma once
#include "afxcmn.h"

#include <vector>
#include <TopTools_DataMapOfShapeInteger.hxx>
#include <TopTools_IndexedDataMapOfShapeListOfShape.hxx>

#include "resource.h"
// CShapeStructDialog 对话框

class CShapeStructDialog : public CDialog
{
	DECLARE_DYNAMIC(CShapeStructDialog)

public:
	CShapeStructDialog(const TopoDS_Shape& aShape,CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CShapeStructDialog();

// 对话框数据
	enum { IDD = IDD_DIALOG_SHAPE_STRUCT };

public:
	//是否使用父对象的方向
	void							UseParentOri(bool bEnable) { bUseParentOri_ = bEnable;}

protected:
	bool							bUseParentOri_;//遍历子对象时，是否使用父对象的方向

protected:
	// 显示一个对象的信息
	HTREEITEM					ShowShape(const TopoDS_Shape& aShape,HTREEITEM hParent);

	//具体的显示函数
	HTREEITEM					ShowCompound(const TopoDS_Shape& aShape,HTREEITEM hParent);
	HTREEITEM					ShowCompSolid(const TopoDS_Shape& aShape,HTREEITEM hParent);
	HTREEITEM					ShowSolid(const TopoDS_Shape& aShape,HTREEITEM hParent);
	HTREEITEM					ShowShell(const TopoDS_Shape& aShape,HTREEITEM hParent);
	HTREEITEM					ShowFace(const TopoDS_Shape& aShape,HTREEITEM hParent);
	HTREEITEM					ShowWire(const TopoDS_Wire& aWire,HTREEITEM hParent);
	//显示裁剪环的wire
	HTREEITEM					ShowWire(const TopoDS_Wire& aWire,const TopoDS_Face& aFace,HTREEITEM hParent);
	HTREEITEM					ShowEdge(const TopoDS_Shape& aShape,HTREEITEM hParent);
	//显示裁剪环中的edge
	HTREEITEM					ShowEdge(const TopoDS_Shape& aShape,const TopoDS_Face& aFace,HTREEITEM hParent);
	HTREEITEM					ShowVertex(const TopoDS_Shape& aShape,HTREEITEM hParent);

	//显示shape的基本信息，包括：方向、位置、内部指针等。
	HTREEITEM					ShowShapeBaseProp(const TopoDS_Shape& aShape,HTREEITEM hParent);

	//显示face的geom属性。主要是surface的属性。
	HTREEITEM					ShowFaceGeomProp(const TopoDS_Shape& aShape,HTREEITEM hParent);

	//显示surface的属性
	HTREEITEM					ShowSurface(const Handle(Geom_Surface)& aSurf,HTREEITEM hParent);
	HTREEITEM					ShowBSplineSurface(const Handle(Geom_BSplineSurface)& aSurf,HTREEITEM hParent);
	HTREEITEM					ShowRectTrimmedSurface(const Handle(Geom_RectangularTrimmedSurface) &aSurf,HTREEITEM hParent);
	HTREEITEM					ShowElementCommonProp(const Handle(Geom_ElementarySurface)& aSurf,HTREEITEM hParent);
	HTREEITEM					ShowPlane(const Handle(Geom_Plane)& aSurf,HTREEITEM hParent);
	HTREEITEM					ShowCylinder(const Handle(Geom_CylindricalSurface)& aSurf,HTREEITEM hParent);
	HTREEITEM					ShowLinearExtSurf(const Handle(Geom_SurfaceOfLinearExtrusion)& aSurf,HTREEITEM hParent);
	HTREEITEM					ShowRevolutionSurf(const Handle(Geom_SurfaceOfRevolution)& aSurf,HTREEITEM hParent);

	void							ShowSurfaceBaseProp(const Handle(Geom_Surface)& aSurf,HTREEITEM hParent);
	
	//显示face的裁剪环属性。
	HTREEITEM					ShowFaceLoopProp(const TopoDS_Shape& aShape,HTREEITEM hParent);

	// 显示剖分网格信息
	HTREEITEM					ShowFaceMeshProp(const TopoDS_Shape& aShape,HTREEITEM hParent);


	//显示Edge的geom属性。
	HTREEITEM					ShowEdgeGeomProp(const TopoDS_Shape& aShape,HTREEITEM hParent);


	//显示curve的属性
	HTREEITEM					ShowCurve(const Handle(Geom_Curve)& aCur,HTREEITEM hParent);
	HTREEITEM					ShowSplineCurve(const Handle(Geom_BSplineCurve)& aCrv,HTREEITEM hParent);
	HTREEITEM					ShowSpline2DCurve(const Handle(Geom2d_BSplineCurve)& aCrv,HTREEITEM hParent);

	HTREEITEM					Show2DCurve(const Handle(Geom2d_Curve)& aCur,HTREEITEM hParent);

protected:
	// 显示共享信息
	HTREEITEM					ShowShareInfo(const TopoDS_Shape& aShape,HTREEITEM hParent);

	// 
	void							ShowVertexShareInfo(HTREEITEM hParent);
	void							ShowEdgeShareInfo(HTREEITEM hParent);

	void							InsertKnots(const TColStd_Array1OfReal& aKnots,int npl,HTREEITEM hParent);
	void							InsertMults(const TColStd_Array1OfInteger& aMults,int npl,HTREEITEM hParent);

	//
	void							MapShape(const TopoDS_Shape& aShape,TopTools_DataMapOfShapeInteger& aMap,TopAbs_ShapeEnum stype);

protected:
	// 显示统计信息
	HTREEITEM					ShowShapeStatics(const TopoDS_Shape& aShape,HTREEITEM hParent);

	HTREEITEM					ShowAllEdge(const TopoDS_Shape& aShape,HTREEITEM hParent);

	HTREEITEM					ShowFaceAllLoop(const TopoDS_Shape& aShape,HTREEITEM hParent);

	HTREEITEM					ShowWireOrderedVtx(const TopoDS_Wire &aW,HTREEITEM hParent);

protected:
	TopoDS_Shape				m_aShape;//要显示的对象

	TopTools_IndexedMapOfShape	m_aFaceIdxMap;
	TopTools_IndexedMapOfShape	m_aEdgeIdxMap;
	TopTools_IndexedMapOfShape m_aVertexIdxMap;//记录所有共享对象，获取其唯一编号

	TopTools_DataMapOfShapeInteger	m_aVertexCntMap;
	TopTools_DataMapOfShapeInteger	m_aEdgeCntMap;//key：shape，value：int记录shape被共享的次数

	TopTools_IndexedDataMapOfShapeListOfShape  m_aVEMap;
	TopTools_IndexedDataMapOfShapeListOfShape  m_aEFMap;	

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()

public:
	CTreeCtrl m_ssTree;
	virtual BOOL OnInitDialog();
};
