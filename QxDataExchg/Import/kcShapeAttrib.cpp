#include "stdafx.h"
#include "kcShapeAttrib.h"

IMPLEMENT_STANDARD_RTTIEXT(kcShapeAttrib,Standard_Transient)

kcShapeAttrib::kcShapeAttrib(void)
{
	bHasColor_ = false;
}


kcShapeAttrib::~kcShapeAttrib(void)
{
}

void  kcShapeAttrib::SetColor(double r,double g,double b)
{
	bHasColor_ = true;
	aColor_.SetValues(r,g,b,Quantity_TOC_RGB);
}

void  kcShapeAttrib::SetColor(const Quantity_Color &aCol)
{
	aColor_ = aCol;
	bHasColor_ = true;
}

Quantity_Color  kcShapeAttrib::GetColor() const
{
	ASSERT(HasColor());
	return aColor_;
}