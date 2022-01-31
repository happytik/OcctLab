//
#ifndef _QX_SYS_LIB_H_
#define _QX_SYS_LIB_H_

#include "QxLibDef.h"

// 显示shape的信息，bCumOri:子对象是否考虑父对象的方向
QXLIB_API void ShowShapeStructDialog(const TopoDS_Shape &aShape,bool bCumOri=true);

#endif