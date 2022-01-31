//////////////////////////////////////////////////////////////////////////
// 导出的基类
#ifndef _KC_BASE_EXPORT_H_
#define _KC_BASE_EXPORT_H_

#include <TopTools_ListOfShape.hxx>
#include "QxLibDef.h"

class kcModel;

class QXLIB_API kcBaseExport
{
public:
	kcBaseExport(kcModel *pModel);
	virtual ~kcBaseExport(void);

	void					EnableSelectExport(bool bEnable) { _bExportSelShape = bEnable;}
	void					ExportAsCompound(bool bEnable) { _bExportAsComp = bEnable; }
	void					SetAISContext(const Handle(AIS_InteractiveContext) &aCtx);

	void					AddShape(const TopoDS_Shape &aShape);
	virtual void			DoExport() = 0;

protected:
	//
	int					GetAllShapeFromModel();
	int					GetAllSelectedShape();

protected:
	kcModel				*_pModel;
	Handle(AIS_InteractiveContext) hAISCtx_;

	TopTools_ListOfShape	_aShapeList;
	bool					_bExportSelShape;
	bool					_bExportAsComp;//作为组合输出

	bool					_bHasAddedShape;
};

#endif