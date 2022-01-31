//
#include "StdAfx.h"
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <AIS_Shape.hxx>
#include <Graphic3d_AspectLine3d.hxx>
#include <GCPnts_TangentialDeflection.hxx>
#include <GeomAdaptor_Curve.hxx>
#include <Graphic3d_ArrayOfPolylines.hxx>
#include <Graphic3d_ArrayOfPoints.hxx>
#include <Graphic3d_StructureManager.hxx>
#include <Graphic3d_SequenceOfGroup.hxx>
#include <Aspect_TypeOfMarker.hxx>
#include <Graphic3d_AspectMarker3d.hxx>
#include <Graphic3d_ArrayOfSegments.hxx>
#include <Graphic3d_ArrayOfPolylines.hxx>
#include <OpenGl_Group.hxx>
#include <TColgp_Array1OfPnt.hxx>
#include "kvCoordSystem.h"
#include "kcBasePlane.h"
#include "kPoint.h"
#include "QxBaseUtils.h"
#include "kcDocContext.h"
#include "kcPreviewObj.h"

kcPreviewNurbsPoles::kcPreviewNurbsPoles(const Handle_AIS_InteractiveContext& aCtx,kcDocContext *pDocCtx)
	:kcPreviewObj(aCtx,pDocCtx)
{
	dLineColorR_ = 0.0;
	dLineColorG_ = 1.0;
	dLineColorB_ = 0.0;

	bShowUIsoLine_ = true;
	bShowVIsoLine_ = false;
}

kcPreviewNurbsPoles::~kcPreviewNurbsPoles()
{
}

bool  kcPreviewNurbsPoles::Initialize(const Handle(Geom_Curve) &aSplCrv)
{
	if(aSplCrv.IsNull())
		return false;
	//
	Handle(Geom_Curve) aBasCrv = aSplCrv;
	if(aSplCrv->IsKind(STANDARD_TYPE(Geom_TrimmedCurve))){
		Handle(Geom_TrimmedCurve) aTrmCrv = Handle(Geom_TrimmedCurve)::DownCast(aSplCrv);
		if(!aTrmCrv.IsNull()){
			aBasCrv = aTrmCrv->BasisCurve();
			if(aBasCrv.IsNull()){
				return false;
			}
		}
	}

	if(aBasCrv->IsKind(STANDARD_TYPE(Geom_BSplineCurve))){
		aSplCurve_ = Handle(Geom_BSplineCurve)::DownCast(aBasCrv);
		return true;
	}
	return true;
}

bool  kcPreviewNurbsPoles::Initialize(const Handle(Geom_Surface) &aSplSurf)
{
	if(aSplSurf.IsNull())
		return false;

	Handle(Geom_Surface) aBasSurf = aSplSurf;
	if(aSplSurf->IsKind(STANDARD_TYPE(Geom_RectangularTrimmedSurface))){
		Handle(Geom_RectangularTrimmedSurface) aTrmSurf = Handle(Geom_RectangularTrimmedSurface)::DownCast(aSplSurf);
		aBasSurf = aTrmSurf->BasisSurface();
		if(aBasSurf.IsNull()){
			return false;
		}
	}

	if(aBasSurf->IsKind(STANDARD_TYPE(Geom_BSplineSurface))){
		aSplSurf_ = Handle(Geom_BSplineSurface)::DownCast(aBasSurf);
		return true;
	}

	return false;
}

void  kcPreviewNurbsPoles::SetLineColor(double r,double g,double b)
{
	dLineColorR_ = r;
	dLineColorG_ = g;
	dLineColorB_ = b;
}

//是否显示U向（等V方向）连接线
void  kcPreviewNurbsPoles::ShowUIsoLine(bool bShow)
{
	bShowUIsoLine_ = bShow;
}

//是否显示V向（等U方向）连接线
void  kcPreviewNurbsPoles::ShowVIsoLine(bool bShow)
{
	bShowVIsoLine_ = bShow;
}

// 根据参数创建显示对象,当需要使用多组Group时，可以重载该函数
bool  kcPreviewNurbsPoles::DoBuildDisplay()
{
	if(aSplCurve_.IsNull() && aSplSurf_.IsNull()){
		return false;
	}

	Handle(Graphic3d_Group) hGroup;
	Quantity_Color aCol(_dColR,_dColG,_dColB,Quantity_TOC_RGB);
	Quantity_Color aLinCol(dLineColorR_,dLineColorG_,dLineColorB_,Quantity_TOC_RGB);

	//清除旧的
	_hStru->Clear();
	
	//生成新的
	if(!aSplCurve_.IsNull()){
		int ix,nbPoles = aSplCurve_->NbPoles();
		TColgp_Array1OfPnt aPoles(1,nbPoles);
		Handle(Graphic3d_ArrayOfPoints) aAryPnts = new Graphic3d_ArrayOfPoints(nbPoles,true);
		Handle(Graphic3d_ArrayOfPolylines) aAryPoly = new Graphic3d_ArrayOfPolylines(nbPoles);
				
		aSplCurve_->Poles(aPoles);
		for(ix = 1;ix <= nbPoles;ix ++){
			const gp_Pnt &aPnt = aPoles.Value(ix);
			//
			aAryPnts->AddVertex(aPnt,aCol);
			//
			aAryPoly->AddVertex(aPnt);
		}

		//添加两个Group
		hGroup = _hStru->NewGroup();
		hGroup->AddPrimitiveArray(aAryPnts);

		hGroup = _hStru->NewGroup();
		hGroup->AddPrimitiveArray(aAryPoly);
	}else{
		int uix,vix;
		int nbUP = aSplSurf_->NbUPoles(),nbVP = aSplSurf_->NbVPoles();

		Handle(Graphic3d_ArrayOfPoints) aAryPnts = new Graphic3d_ArrayOfPoints(nbUP*nbVP,true);
		for(uix = 1;uix <= nbUP;uix ++){
			for(vix = 1;vix <= nbVP;vix ++){
				const gp_Pnt &aP = aSplSurf_->Pole(uix,vix);
				//
				aAryPnts->AddVertex(aP,aCol);
			}
		}

		hGroup = _hStru->NewGroup();
		hGroup->AddPrimitiveArray(aAryPnts);

		//显示流线

		Handle(Graphic3d_ArrayOfPolylines) aAryPoly;
		
		if(bShowUIsoLine_){

			aAryPoly = new Graphic3d_ArrayOfPolylines(nbUP * nbVP,
				nbVP,0,false,true);
			//U向
			for(vix = 1;vix <= nbVP;vix ++){

				aAryPoly->AddBound(nbUP,aLinCol);
				for(uix = 1;uix <= nbUP;uix ++){
					const gp_Pnt &aPnt = aSplSurf_->Pole(uix,vix);
					aAryPoly->AddVertex(aPnt);
				}
			}

			hGroup = _hStru->NewGroup();
			hGroup->AddPrimitiveArray(aAryPoly);
		}

		if(bShowVIsoLine_){

			aAryPoly = new Graphic3d_ArrayOfPolylines(nbUP * nbVP,
				nbUP,0,false,true);
			//U向
			for(uix = 1;uix <= nbUP;uix ++){

				aAryPoly->AddBound(nbVP,aLinCol);
				for(vix = 1;vix <= nbVP;vix ++){
					const gp_Pnt &aPnt = aSplSurf_->Pole(uix,vix);
					aAryPoly->AddVertex(aPnt);
				}
			}

			hGroup = _hStru->NewGroup();
			hGroup->AddPrimitiveArray(aAryPoly);
		}
	}

	return true;
}

// 具体的创建显示对象,创建到一个Group中
bool  kcPreviewNurbsPoles::BuildGroupDisplay(const Handle(Graphic3d_Group)& hGroup)
{
	hGroup;
	return true;
}

///////////////////////////////////////////////////////////////////////////////
//
// 显示网格面
kcPreviewMesh::kcPreviewMesh(const Handle_AIS_InteractiveContext& aCtx,kcDocContext *pDocCtx)
	: kcPreviewObj(aCtx,pDocCtx)
{
}

kcPreviewMesh::~kcPreviewMesh()
{
}

bool kcPreviewMesh::Initialize(const TopoDS_Face &aFace)
{
	if(aFace.IsNull())
		return false;

	aFace_ = aFace;
	return true;
}

bool kcPreviewMesh::Initialize(const TopoDS_Shape &aS)
{
	if(aS.IsNull())
		return false;

	TopExp_Explorer aExp;
	aExp.Init(aS,TopAbs_FACE);
	if(!aExp.More())
		return false;

	aShape_ = aS;

	return true;
}

// 根据参数创建显示对象,当需要使用多组Group时，可以重载该函数
bool kcPreviewMesh::DoBuildDisplay()
{
	if(!aFace_.IsNull()){
		DoBuildFaceDisplay(aFace_);
	}

	if(!aShape_.IsNull()){
		TopExp_Explorer aExp;
		aExp.Init(aShape_,TopAbs_FACE);
		for(;aExp.More();aExp.Next()){
			const TopoDS_Face &aF = TopoDS::Face(aExp.Current());
			DoBuildFaceDisplay(aF);
		}
	}

	return true;
}

void kcPreviewMesh::DoBuildFaceDisplay(const TopoDS_Face &aF)
{
	Quantity_Color aCol(_dColR,_dColG,_dColB,Quantity_TOC_RGB);
	TopLoc_Location L;
	const Handle(Poly_Triangulation) &aTriangulation = BRep_Tool::Triangulation(aF,L);
	if(aTriangulation.IsNull()){
		return;
	}

	int ix,N1,N2,N3,nbTris = aTriangulation->NbTriangles();
	//const TColgp_Array1OfPnt &aNodes = aTriangulation->Nodes();
	Handle(Graphic3d_ArrayOfPolylines) aAryPoly = new Graphic3d_ArrayOfPolylines(nbTris * 4,nbTris,0,false,true);
	//
	for(ix = 1;ix <= nbTris;ix ++){
		const Poly_Triangle &aTri = aTriangulation->Triangle(ix);
		aTri.Get(N1,N2,N3);

		aAryPoly->AddBound(4,aCol);
		aAryPoly->AddVertex(aTriangulation->Node(N1));
		aAryPoly->AddVertex(aTriangulation->Node(N2));
		aAryPoly->AddVertex(aTriangulation->Node(N3));
		aAryPoly->AddVertex(aTriangulation->Node(N1));
	}

	Handle(Graphic3d_Group) hGroup = _hStru->NewGroup();
	hGroup->AddPrimitiveArray(aAryPoly);

}

// 具体的创建显示对象,创建到一个Group中
bool  kcPreviewMesh::BuildGroupDisplay(const Handle(Graphic3d_Group)& hGroup)
{
	hGroup;
	return true;
}