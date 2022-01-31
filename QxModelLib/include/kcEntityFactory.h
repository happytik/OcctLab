//////////////////////////////////////////////////////////////////////////
//
#ifndef _KC_ENTITY_FACTORY_H_
#define _KC_ENTITY_FACTORY_H_

#include <vector>
#include "QxLibDef.h"

class kcEntity;
class TopoDS_Shape;

class QXLIB_API kcEntityFactory
{
public:
	kcEntityFactory(void);
	~kcEntityFactory(void);

	//根据类型，创建对应的entity对象
	static kcEntity*				CreateByType(int type);

	kcEntity*					Create(const TopoDS_Shape& aShape);
	static kcEntity*				CreateEntity(const TopoDS_Shape& aShape);

	//创建一个或一系列entity,如果shape是compound，则是否分解为子对象
	bool							Create(const TopoDS_Shape& aShape,
									   std::vector<kcEntity *>& aEntList,
									   bool bDeCompound = true);

	//从曲线创建
	kcEntity*					Create(const Handle(Geom_Curve) &aCurve);
	// 带名称、颜色的创建一个对象
	static kcEntity*				Create(const Handle(Geom_Curve) &aCurve,int color,const char *pDesc);

	//从曲面创建
	kcEntity*					Create(const Handle(Geom_Surface) &aSurf);
};

#endif