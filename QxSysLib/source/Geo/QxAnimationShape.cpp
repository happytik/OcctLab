#include "StdAfx.h"
#include "QxBRepLib.h"
#include "QxAnimationShape.h"

QxAnimationShape::QxAnimationShape(const Handle(AIS_InteractiveContext)& aCtx,
									 const TopoDS_Shape& aShape,bool bCopy)
{
	ASSERT(!aShape.IsNull());
	m_aCtx = aCtx;
	//拷贝一个对象
	if(bCopy){
		TopoDS_Shape acShape = QxBRepLib::CopyShape(aShape);
		if(!acShape.IsNull()){
			m_aAIShape = new AIS_Shape(acShape);
		}
	}else{
		m_aAIShape = new AIS_Shape(aShape);
	}
}

QxAnimationShape::~QxAnimationShape(void)
{
	if(!m_aAIShape.IsNull()){
		Hide(FALSE);
		m_aAIShape.Nullify();
	}
}

//
BOOL	QxAnimationShape::Reset(BOOL bUpdateView)
{
	if(m_aAIShape.IsNull())
		return FALSE;

	m_aCtx->ResetLocation(m_aAIShape);
	m_aCtx->Update(m_aAIShape,bUpdateView ? true:false);

	return TRUE;
}

//
BOOL	QxAnimationShape::SetTrsf(gp_Trsf& trsf,BOOL bUpdateView)
{
	if(m_aAIShape.IsNull())
		return FALSE;

	m_aCtx->SetLocation(m_aAIShape,TopLoc_Location(trsf));
	m_aCtx->Update(m_aAIShape,bUpdateView ? true:false);

	return TRUE;
}

void QxAnimationShape::SetColor(double r,double g,double b)
{
	if(m_aAIShape.IsNull())
		return;

	Quantity_Color aCol(r,g,b,Quantity_TOC_RGB);
	m_aAIShape->SetColor(aCol);
}

//
BOOL QxAnimationShape::Display(BOOL bUpdateView)
{
	if(m_aAIShape.IsNull())
		return FALSE;

	if(!m_aCtx->IsDisplayed(m_aAIShape))
	{
		m_aCtx->Display(m_aAIShape,bUpdateView ? true:false);
	}

	return TRUE;
}

BOOL	QxAnimationShape::Hide(BOOL bUpdateView)
{
	if(m_aAIShape.IsNull())
		return FALSE;

	if(m_aCtx->IsDisplayed(m_aAIShape))
	{
		m_aCtx->Remove(m_aAIShape,bUpdateView ? true:false);
	}

	return TRUE;
}
