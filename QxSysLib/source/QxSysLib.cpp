//
#include "stdafx.h"
#include "Dlgs\ShapeStructDialog.h"
#include "QxSysLib.h"


// 显示shape的信息，bCumOri:子对象是否考虑父对象的方向
void ShowShapeStructDialog(const TopoDS_Shape &aShape,bool bCumOri)
{
	if(aShape.IsNull())
		return;

	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CShapeStructDialog dlg(aShape);
	dlg.UseParentOri(bCumOri);
	dlg.DoModal();
}
