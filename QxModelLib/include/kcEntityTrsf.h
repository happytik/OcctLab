///////////////////////////////////////////////////////////////////////////
// 定义了对Entity的变换功能
#ifndef _KC_ENTITY_TRSF_H_
#define _KC_ENTITY_TRSF_H_

#include "kPoint.h"
#include "kVector.h"
#include "QxLibDef.h"

class kcEntity;

class QXLIB_API kcEntityTrsf
{
public:
	kcEntityTrsf(void);
	~kcEntityTrsf(void);

	// 移动对象,生成新的对象
	kcEntity*						Move(kcEntity *pEntity,const kVector3& vec);

	// 旋转
	// p:基点,dir:单位向量，ang：旋转角度，弧度。
	kcEntity*						Rotate(kcEntity *pEntity,const kPoint3& p,const kVector3& dir,double ang);

	// 缩放
	kcEntity*						Scale(kcEntity *pEntity,const kPoint3& p,double scale);
};

#endif