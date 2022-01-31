
#ifndef _AIS_CURVE_H_
#define _AIS_CURVE_H_

#include <Geom_Curve.hxx>
#include <AIS_InteractiveObject.hxx>

// Handle definition
//
DEFINE_STANDARD_HANDLE(AIS_Curve,AIS_InteractiveObject)

class AIS_Curve : public AIS_InteractiveObject{
public:
	AIS_Curve(const Handle(Geom_Curve)& hCurve);

	//! Returns index 20 by default. <br>
	virtual  Standard_Integer Signature() const;

	const Handle(Geom_Curve)&	Curve() const;

	DEFINE_STANDARD_RTTIEXT(AIS_Curve,AIS_InteractiveObject)

private:
	virtual void		Compute(const Handle(PrsMgr_PresentationManager)& aPresentationManager,
								const Handle(Prs3d_Presentation)& aPresentation,
								const Standard_Integer aMode = 0) ;

	virtual void		ComputeSelection(const Handle(SelectMgr_Selection)& aSelection,
										 const Standard_Integer aMode) ;

private:
	Handle(Geom_Curve)		m_hCurve;
};

#endif