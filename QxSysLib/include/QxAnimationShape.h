//////////////////////////////////////////////////////////////////////////
// ∂Øª≠∂‘œÛ
#ifndef _KCG_ANIMATION_SHAPE_H_
#define _KCG_ANIMATION_SHAPE_H_

#include "QxLibDef.h"

class QXLIB_API QxAnimationShape
{
public:
	QxAnimationShape(const Handle(AIS_InteractiveContext)& aCtx,const TopoDS_Shape& aShape,bool bCopy = true);
	~QxAnimationShape(void);

	//
	BOOL						Reset(BOOL bUpdateView);
	//
	BOOL						SetTrsf(gp_Trsf& trsf,BOOL bUpdateView);

	void						SetColor(double r,double g,double b);
	//
	BOOL						Display(BOOL bUpdateView);
	BOOL						Hide(BOOL bUpdateView);

protected:
	Handle(AIS_InteractiveContext)	m_aCtx;
	Handle(AIS_Shape)			m_aAIShape;
};


#endif