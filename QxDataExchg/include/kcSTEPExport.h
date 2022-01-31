#pragma once

#include <IFSelect_ReturnStatus.hxx>
#include <STEPControl_StepModelType.hxx>
#include "kcBaseExport.h"

class QXLIB_API kcSTEPExport : public kcBaseExport
{
public:
	kcSTEPExport(kcModel *pModel);
	~kcSTEPExport(void);

	virtual void			DoExport();

protected:
	IFSelect_ReturnStatus	WriteSTEP(const Standard_CString& aFileName);
	STEPControl_StepModelType GetExportType(const TopoDS_Shape& aShape);
};

