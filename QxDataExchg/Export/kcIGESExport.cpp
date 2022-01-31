//
#include "stdafx.h"
#include <string>
#include <TopTools_ListIteratorOfListOfShape.hxx>
#include <IFSelect_ReturnStatus.hxx>
#include <IGESControl_Writer.hxx>
#include <IGESControl_Controller.hxx>
#include <XSControl_WorkSession.hxx>
#include <Transfer_TransientProcess.hxx>
#include "kcLayer.h"
#include "kcModel.h"
#include "kcEntity.h"
#include "kcEntityFactory.h"
#include "kcBaseReader.h"
#include "kcIGESExport.h"

kcIGESExport::kcIGESExport(kcModel *pModel)
	:kcBaseExport(pModel)
{
}


kcIGESExport::~kcIGESExport(void)
{
}

void	kcIGESExport::DoExport()
{
	//如果有添加的导出shape，就不进行下面的处理，用于导出
	//添加的对象
	if(!_bHasAddedShape){
		if(_bExportSelShape){
			GetAllSelectedShape();
		}else{
			GetAllShapeFromModel();
		}
	}
	if(_aShapeList.IsEmpty()){
		AfxMessageBox("没有对象可以导出.");
		return;
	}
	
	CFileDialog dlg(FALSE,
		NULL,
		NULL,
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		"IGES Files (*.iges , *.igs)|*.iges; *.igs||", 
		NULL );

	if(dlg.DoModal() == IDOK){
		CString szFile = dlg.GetPathName();

		CWaitCursor wc;
		char utf8Char[1024];
		
		EncodeUtf8FilePath(szFile,utf8Char,1024);

		bool bRet = WriteIGES(utf8Char);
		if(!bRet){
			AfxMessageBox("导出失败!");
		}
	}
}

bool kcIGESExport::WriteIGES(const Standard_CString& aFileName)
{
	IGESControl_Controller::Init();
	IGESControl_Writer aWriter("MM",0);//单位，曲面模数，非BRep
	
	TopTools_ListIteratorOfListOfShape lls;
	for(lls.Initialize(_aShapeList);lls.More();lls.Next()){
		const TopoDS_Shape &aS = lls.Value();
		//
		aWriter.AddShape(aS);
	}
	return aWriter.Write(aFileName);
}


