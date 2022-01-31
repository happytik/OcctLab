//////////////////////////////////////////////////
//
#pragma once

#include <Standard_Transient.hxx>
#include <Quantity_Color.hxx>
#include "QxLibDef.h"

class kcShapeAttrib;
DEFINE_STANDARD_HANDLE(kcShapeAttrib,Standard_Transient);

class kcShapeAttrib : public Standard_Transient
{
public:
	kcShapeAttrib(void);
	~kcShapeAttrib(void);

	void					SetColor(double r,double g,double b);
	void					SetColor(const Quantity_Color &aCol);
	Quantity_Color		GetColor() const;
	bool					HasColor() const { return bHasColor_; }

public:
	DEFINE_STANDARD_RTTIEXT(kcShapeAttrib,Standard_Transient)

protected:
	bool					bHasColor_;
	Quantity_Color		aColor_;
};

