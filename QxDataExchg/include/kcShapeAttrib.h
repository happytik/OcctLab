//////////////////////////////////////////////////
// 
#pragma once

#include <Standard_Type.hxx>
#include <Standard_Transient.hxx>
#include <Quantity_Color.hxx>

class kcShapeAttrib;
DEFINE_STANDARD_HANDLE(kcShapeAttrib,Standard_Transient);

// 记录一个STEP对象转换成的OCC的shape的属性信息：颜色、图层等。
// 目前只支持颜色
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
	bool					bHasColor_;//是否已经设定了颜色
	Quantity_Color		aColor_;//颜色属性，RGB值
};

