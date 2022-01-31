// Dialogs\XCAFDocDialog.cpp : 实现文件
//

#include "stdafx.h"
#include <STEPControl_Reader.hxx>
#include <STEPCAFControl_Reader.hxx>
#include <XSControl_WorkSession.hxx>
#include <TDocStd_Document.hxx>
#include <XCAFApp_Application.hxx>
#include <Transfer_TransientProcess.hxx>
#include <StepData_StepModel.hxx>
#include <XCAFDoc_ShapeTool.hxx>
#include <XCAFDoc_DocumentTool.hxx>
#include <XCAFDoc_ColorTool.hxx>
#include <XCAFDoc_Color.hxx>
#include <XCAFDoc_LayerTool.hxx>
#include <XCAFDoc_GraphNode.hxx>
#include <TDF_Tool.hxx>
#include <TDF_ChildIterator.hxx>
#include <TDF_LabelSequence.hxx>
#include <TDF_AttributeIterator.hxx>
#include <TDataStd_Name.hxx>
#include "XCAFDocDialog.h"


// XCAFDocDialog 对话框

IMPLEMENT_DYNAMIC(XCAFDocDialog, CDialog)

XCAFDocDialog::XCAFDocDialog(const Handle(TDocStd_Document) &aDoc,CWnd* pParent /*=NULL*/)
	: CDialog(XCAFDocDialog::IDD, pParent)
{
	xCAFDoc_ = aDoc;
}

XCAFDocDialog::~XCAFDocDialog()
{
}

void XCAFDocDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TREE_XCAF_DOC, xCAFDocTree_);
}


BEGIN_MESSAGE_MAP(XCAFDocDialog, CDialog)
END_MESSAGE_MAP()


// XCAFDocDialog 消息处理程序


BOOL XCAFDocDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	if(!xCAFDoc_.IsNull()){
		ShowXDocStruct();
	}

	return TRUE;  // return TRUE unless you set the focus to a control
}


void  XCAFDocDialog::OnOK()
{
	// TODO: 在此添加专用代码和/或调用基类

	CDialog::OnOK();
}

static char *ss_aShapeType[] = {
		"COMPOUND",
		"COMPSOLID",
		"SOLID",
		"SHELL",
		"FACE",
		"WIRE",
		"EDGE",
		"VERTEX",
		"SHAPE"
	};

void  XCAFDocDialog::ShowXDocStruct()
{
	TDF_Label aMainLab = xCAFDoc_->Main();
	if(!aMainLab.IsNull()){
		ShowLabel(aMainLab,TVI_ROOT);
	}
}

void  XCAFDocDialog::ShowLabel(TDF_Label aLabel,HTREEITEM hParent)
{
	CString szItem;
	HTREEITEM hItem = NULL,hSubItem = NULL,hSubItem2 = NULL;
	TCollection_AsciiString strTags;
	Handle(Standard_Type) aType;
	Handle(XCAFDoc_LayerTool) aLyTool = XCAFDoc_DocumentTool::LayerTool( xCAFDoc_->Main() );

	bool bLayer = aLyTool->IsLayer(aLabel);

	//tags
	TDF_Tool::Entry(aLabel,strTags);
	szItem.Format("[%s]",strTags);
	hItem = xCAFDocTree_.InsertItem(szItem,hParent);

	//shape
	std::string ssType;
	if(XCAFDoc_ShapeTool::IsAssembly(aLabel)){
		ssType = "Assembly";
	}else if(XCAFDoc_ShapeTool::IsComponent(aLabel)){
		ssType = "Component";
	}else if(XCAFDoc_ShapeTool::IsReference(aLabel)){
		ssType = "Reference";
	}

	TopoDS_Shape aS;
	if(XCAFDoc_ShapeTool::GetShape(aLabel,aS)){
		TopoDS_TShape *pTS = aS.TShape().get();
		if(ssType.empty()){
			szItem.Format("Shape : %s, %X",ss_aShapeType[aS.ShapeType()],pTS);
		}else{
			szItem.Format("%s : %s, %X",ssType.c_str(),ss_aShapeType[aS.ShapeType()],pTS);
		}
		xCAFDocTree_.InsertItem(szItem,hItem);
	}
	//

	//attribs
	szItem.Format("Attribs : [%d]",aLabel.NbAttributes());
	hSubItem = xCAFDocTree_.InsertItem(szItem,hItem);
	//
	TDF_AttributeIterator attIter(aLabel,false);
	for(;attIter.More();attIter.Next()){
		Handle(TDF_Attribute) aAttrib = attIter.Value();

		//
		aType = aAttrib->DynamicType();
		//
		if(aAttrib->IsKind(STANDARD_TYPE(XCAFDoc_Color))){
			Quantity_Color aCol;
			Handle(XCAFDoc_Color) aXDCol = Handle(XCAFDoc_Color)::DownCast(aAttrib);
			if(!aXDCol.IsNull()){
				aCol = aXDCol->GetColor();
				szItem.Format("%s : (%f,%f,%f)",aType->Name(),aCol.Red(),aCol.Green(),aCol.Blue());
				xCAFDocTree_.InsertItem(szItem,hSubItem);
			}else{
				xCAFDocTree_.InsertItem(aType->Name(),hSubItem);
			}
		}else if(aAttrib->IsKind(STANDARD_TYPE(TDataStd_Name))){
			Handle(TDataStd_Name) aName = Handle(TDataStd_Name)::DownCast(aAttrib);
			if(!aName.IsNull()){
				const TCollection_ExtendedString &sExtStr = aName->Get();
				TCollection_AsciiString ascStr(sExtStr);
				if(ascStr.IsEmpty()){
					szItem.Format("%s : (),%X",aType->Name(),aAttrib.get());
				}else{
					szItem.Format("%s : (%s),%X",aType->Name(),ascStr.ToCString(),aAttrib.get());
				}
			}else{
				szItem = aType->Name();
			}
			xCAFDocTree_.InsertItem(szItem,hSubItem);
		}else if(aAttrib->IsKind(STANDARD_TYPE(XCAFDoc_GraphNode))){
			szItem.Format("%s : %08X",aType->Name(),aAttrib.get());
			HTREEITEM hLyItem = xCAFDocTree_.InsertItem(szItem,hSubItem);

			if(bLayer){
				Handle(XCAFDoc_GraphNode) aGNode = Handle(XCAFDoc_GraphNode)::DownCast(aAttrib);
				int ix,nbChild = aGNode->NbChildren();
				for(ix = 1;ix <= nbChild;ix ++){
					Handle(XCAFDoc_GraphNode) aCNode = aGNode->GetChild(ix);
					//
					TDF_Label aLyLab = aCNode->Label();
					TDF_Tool::Entry(aLyLab,strTags);
					szItem.Format("[%s]",strTags);
					xCAFDocTree_.InsertItem(szItem,hLyItem);
				}
			}
		}else{
			szItem.Format("%s : %08X",aType->Name(),aAttrib.get());
			xCAFDocTree_.InsertItem(szItem,hSubItem);
		}
	}

	//child
	szItem.Format("Child : [%d]",aLabel.NbChildren());
	hSubItem = xCAFDocTree_.InsertItem(szItem,hItem);
	//
	TDF_ChildIterator cIte(aLabel,false);
	for(;cIte.More();cIte.Next()){
		TDF_Label aSubLab = cIte.Value();
		//
		ShowLabel(aSubLab,hSubItem);
	}

	//显示图层下的对象
	if(bLayer){
		TDF_LabelSequence aChildLab;
		aLyTool->GetShapesOfLayer(aLabel,aChildLab);

		if(!aChildLab.IsEmpty()){
			int kx,nbChild = aChildLab.Length();

			szItem.Format("Layer Shapes : [%d]",nbChild);
			hSubItem = xCAFDocTree_.InsertItem(szItem,hItem);

			for(kx = 1;kx <= nbChild;kx ++){
				TDF_Label aSL = aChildLab.Value(kx);
				//
				TDF_Tool::Entry(aSL,strTags);
				szItem.Format("[%s]",strTags);
				xCAFDocTree_.InsertItem(szItem,hSubItem);
			}
		}
	}
}