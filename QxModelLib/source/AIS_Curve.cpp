// ScInputEdit.cpp : 实现文件
//

#include "stdafx.h"
#include "AIS_Curve.h"

#include <Prs3d_Drawer.hxx>
#include <StdSelect_BRepSelectionTool.hxx>
#include <Select3D_SensitiveBox.hxx>
#include <StdSelect_BRepOwner.hxx>
#include <BRepBndLib.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <Bnd_Box.hxx>
#include <SelectMgr_Selection.hxx>
#include <StdPrs_DeflectionCurve.hxx>

IMPLEMENT_STANDARD_RTTIEXT(AIS_Curve,AIS_InteractiveObject)

AIS_Curve::AIS_Curve(const Handle(Geom_Curve)& hCurve)
:m_hCurve(hCurve)
{

}

//! Returns index 20 by default. <br>
Standard_Integer AIS_Curve::Signature() const
{
	return 20;
}

const Handle(Geom_Curve)&	AIS_Curve::Curve() const
{
	return m_hCurve;
}

void	AIS_Curve::Compute(const Handle(PrsMgr_PresentationManager)& aPresentationManager,
							const Handle(Prs3d_Presentation)& aPresentation,
							const Standard_Integer aMode)
{
	GeomAdaptor_Curve curv(m_hCurve);
	StdPrs_DeflectionCurve::Add(aPresentation,curv,myDrawer);
}

void	AIS_Curve::ComputeSelection(const Handle(SelectMgr_Selection)& aSelection,
									const Standard_Integer aMode) 
{
	if(m_hCurve.IsNull())
		return;

	Handle(Geom_Curve) hCopyCur = Handle(Geom_Curve)::DownCast(m_hCurve->Copy());
	if(hCopyCur.IsNull())
		return;

	BRepBuilderAPI_MakeEdge me(hCopyCur);
	if(me.IsDone() == Standard_False)
		return;

	const TopoDS_Edge& sEdge = me.Edge();

	//从AIS_Shape中借鉴的代码
	Standard_Real aDeflection = myDrawer->MaximalChordialDeviation();
	if (myDrawer->TypeOfDeflection() == Aspect_TOD_RELATIVE)
	{
		// Vector is calculated depending on global min max of the part:
		Bnd_Box aBndBox; //= BoundingBox(); ?
		BRepBndLib::Add (sEdge, aBndBox);
		if (!aBndBox.IsVoid())
		{
			Standard_Real aXmin, aYmin, aZmin, aXmax, aYmax, aZmax;
			aBndBox.Get (aXmin, aYmin, aZmin, aXmax, aYmax, aZmax);
			aDeflection = Max (aXmax - aXmin, Max (aYmax - aYmin, aZmax - aZmin)) * myDrawer->DeviationCoefficient();
		}
	}

	Standard_Boolean autoTriangulation = Standard_False;
	try {  
		//OCC_CATCH_SIGNALS
			StdSelect_BRepSelectionTool::Load(aSelection,
			this,
			sEdge,
			TopAbs_EDGE,
			aDeflection,
			myDrawer->DeviationAngle(),
			autoTriangulation); 
	} catch ( Standard_Failure ) {
		//    cout << "a Shape should be incorrect : A Selection on the Bnd  is activated   "<<endl;
		if ( aMode == 0 ) {
			Bnd_Box aBndBox; //= BoundingBox(); ?
			BRepBndLib::Add (sEdge, aBndBox);
			Handle(StdSelect_BRepOwner) aOwner = new StdSelect_BRepOwner(sEdge,this);
			Handle(Select3D_SensitiveBox) aSensitiveBox = new Select3D_SensitiveBox(aOwner,aBndBox);
			aSelection->Add(aSensitiveBox);
		}
	}
}
