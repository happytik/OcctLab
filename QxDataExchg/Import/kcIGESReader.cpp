#include "StdAfx.h"
#include <IGESControl_Reader.hxx>
#include "kcEntity.h"
#include "kcModel.h"
#include "kcBaseReader.h"
#include "kcIGESReader.h"

kcIGESReader::kcIGESReader(kcModel *pModel)
:m_pModel(pModel)
{
	ASSERT(pModel);
}

kcIGESReader::~kcIGESReader(void)
{
}

//导入IGES文件
bool	kcIGESReader::DoImport()
{
	if(!m_pModel)
		return false;

	CFileDialog dlg(TRUE,
					NULL,
					NULL,
					OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
					"IGES Files (*.iges , *.igs)|*.iges; *.igs||", 
					NULL );

	if(dlg.DoModal() == IDOK)
	{
		CWaitCursor wc;

		CString szFile = dlg.GetPathName();
		Standard_CString aFileName = (Standard_CString)(LPCTSTR)szFile;
		//
		IGESControl_Reader Reader;
		char utf8Char[1024];
		
		EncodeUtf8FilePath(aFileName,utf8Char,1024);

		Standard_Integer status = Reader.ReadFile(utf8Char);

		if (status != IFSelect_RetDone) 
			return false;

		Reader.TransferRoots();
		TopoDS_Shape aShape = Reader.OneShape();     

		AddEntity(aShape);
	}
	return true;
}

// 将导入对象添加到model中。
// 这里aShape是个Compound结构。
//
bool	kcIGESReader::AddEntity(const TopoDS_Shape& aShape)
{
	if(aShape.IsNull())
		return false;

	TopAbs_ShapeEnum etyp = aShape.ShapeType();
	if(etyp == TopAbs_COMPOUND)
	{
		//分别添加子对象
		TopoDS_Iterator ite(aShape);
		while(ite.More())
		{
			TopoDS_Shape aS = ite.Value();
			if(!aS.IsNull())
				AddOneShape(aS);

			ite.Next();
		}
	}
	else
	{
		AddOneShape(aShape);
	}
	return true;
}

// 添加一个对象
bool	kcIGESReader::AddOneShape(const TopoDS_Shape& aShape)
{
	if(aShape.IsNull())
		return false;

	TopAbs_ShapeEnum etyp = aShape.ShapeType();
	switch(etyp){
		case TopAbs_VERTEX:
			AddVertex(aShape);
			break;
		case TopAbs_EDGE:
			AddEdge(aShape);
			break;
		case TopAbs_WIRE:
			AddWire(aShape);
			break;
		case TopAbs_FACE:
			AddFace(aShape);
			break;
		case TopAbs_SHELL:
			AddShell(aShape);
			break;
		case TopAbs_SOLID:
			AddSolid(aShape);
			break;
		case TopAbs_COMPOUND:
			AddCompound(aShape);
			break;
		default:
			break;
	}

	return true;
}

bool	kcIGESReader::AddVertex(const TopoDS_Shape& aShape)
{
	TopoDS_Vertex aV = TopoDS::Vertex(aShape);
	//
	gp_Pnt pnt = BRep_Tool::Pnt(aV);
	kcPointEntity *pEnt = new kcPointEntity;
	if(pEnt)
	{
		pEnt->SetPoint(pnt.X(),pnt.Y(),pnt.Z());
		m_pModel->AddEntity(pEnt);
	}

	return true;
}

bool	kcIGESReader::AddEdge(const TopoDS_Shape& aShape)
{
	TopoDS_Edge aEdge = TopoDS::Edge(aShape);
	kcEdgeEntity *pEnt = new kcEdgeEntity;
	if(pEnt)
	{
		pEnt->SetEdge(aEdge);
		m_pModel->AddEntity(pEnt);
	}
	
	return true;
}

bool	kcIGESReader::AddWire(const TopoDS_Shape& aShape)
{
	TopoDS_Wire aWire = TopoDS::Wire(aShape);
	kcWireEntity *pEnt = new kcWireEntity;
	if(pEnt)
	{
		pEnt->SetWire(aWire);
		m_pModel->AddEntity(pEnt);
	}
	return true;
}

bool	kcIGESReader::AddFace(const TopoDS_Shape& aShape)
{
	TopoDS_Face aFace = TopoDS::Face(aShape);
	kcFaceEntity *pEnt = new kcFaceEntity;
	if(pEnt)
	{
		pEnt->SetFace(aFace);
		m_pModel->AddEntity(pEnt);
	}
	return true;
}

bool	kcIGESReader::AddShell(const TopoDS_Shape& aShape)
{
	TopoDS_Shell aShell = TopoDS::Shell(aShape);
	kcShellEntity *pEnt = new kcShellEntity;
	if(pEnt)
	{
		pEnt->SetShell(aShell);
		m_pModel->AddEntity(pEnt);
	}
	return true;
}

bool	kcIGESReader::AddSolid(const TopoDS_Shape& aShape)
{
	TopoDS_Solid aSolid = TopoDS::Solid(aShape);
	kcSolidEntity *pEnt = new kcSolidEntity;
	if(pEnt)
	{
		pEnt->SetSolid(aSolid);
		m_pModel->AddEntity(pEnt);
	}
	return true;
}

bool	kcIGESReader::AddCompound(const TopoDS_Shape& aShape)
{
	TopoDS_Compound aComp = TopoDS::Compound(aShape);
	kcCompoundEntity *pEnt = new kcCompoundEntity;
	if(pEnt)
	{
		pEnt->SetCompound(aComp);
		m_pModel->AddEntity(pEnt);
	}
	return true;
}
