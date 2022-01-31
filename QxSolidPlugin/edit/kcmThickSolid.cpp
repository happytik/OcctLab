#include "StdAfx.h"
#include <BRepOffsetAPI_MakeThickSolid.hxx>
#include "kiInputEntityTool.h"
#include "kiInputValueTool.h"
#include "kcEntity.h"
#include "kcBasePlane.h"
#include "kcModel.h"
#include "kvCoordSystem.h"
#include "kcEntityFactory.h"
#include "QxBRepLib.h"
#include "kcmThickSolid.h"

//////////////////////////////////////////////////////////////////
//
kcmSheetSolid::kcmSheetSolid(void)
{
	m_strName = "抽壳";
	m_strAlias = "SheetSolid";
	m_strDesc = "抽壳";

	m_bOutSide = false;
	m_pInputEntity = NULL;
	m_pInputFace = NULL;
	m_pInputThickness = NULL;
	m_dThick = 2.0;
}

kcmSheetSolid::~kcmSheetSolid(void)
{
}


// 表明命令可以完成,主要和Apply函数结合使用.
BOOL	kcmSheetSolid::CanFinish()//命令是否可以完成
{
	return FALSE;
}

// 当输入工具获取结果,结束时,会回调对应命令的该函数.
// 用于继续执行命令或切换输入工具.也可能返回KSTA_DONE,表明命令执行结束.
//
int	kcmSheetSolid::OnInputFinished(kiInputTool *pTool)
{
	if(pTool == m_pInputEntity)
	{
		if(m_pInputEntity->IsInputDone())
		{
			//kiSelEntity se = m_pInputEntity->GetFirstSelect();
			//m_pInputFace->Set(se.AISObject(),eInputLocalFace);

			NavToNextTool();
		}
	}
	else if(pTool == m_pInputFace)
	{
		if(m_pInputFace->IsInputDone())
		{
			NavToNextTool();
		}
	}
	else if(pTool == m_pInputThickness)
	{
		if(m_pInputThickness->IsInputDone())
		{
			if(BuildSheetSolid())
			{
				Done();
				Redraw();

				return KSTA_DONE;
			}
			else
			{
				EndCommand(KSTA_FAILED);
				return KSTA_FAILED;
			}
		}
	}
	return KSTA_CONTINUE;
}

int		kcmSheetSolid::OnExecute()
{
	return KSTA_CONTINUE;
}

int		kcmSheetSolid::OnEnd(int nCode)
{
	m_pInputEntity->ClearSel();

	return nCode;
}

// 创建必要的输入工具
BOOL	kcmSheetSolid::CreateInputTools()
{
	m_optionSet.AddBoolOption("向外偏移",'O',m_bOutSide);

	m_pInputEntity = new kiInputEntityTool(this,"选择实体",&m_optionSet);
	m_pInputEntity->SetOption(KCT_ENT_SOLID,1);
	m_pInputEntity->SetNaturalMode(true);

	m_pInputFace = new kiInputEntityTool(this,"选择要去除的面");
	m_pInputFace->SetOption(KCT_ENT_FACE,-1);
	m_pInputFace->SetNaturalMode(false);

	m_pInputThickness = new kiInputDoubleTool(this,"输入厚度");
	m_pInputThickness->Init(&m_dThick,1e-6,1000);

	return TRUE;
}

BOOL	kcmSheetSolid::DestroyInputTools()
{
	KC_SAFE_DELETE(m_pInputEntity);
	KC_SAFE_DELETE(m_pInputFace);
	KC_SAFE_DELETE(m_pInputThickness);

	return TRUE;
}

BOOL	kcmSheetSolid::InitInputToolQueue()
{
	AddInputTool(m_pInputEntity);
	AddInputTool(m_pInputFace);
	AddInputTool(m_pInputThickness);

	return TRUE;
}

BOOL	kcmSheetSolid::BuildSheetSolid()
{
	if(m_pInputEntity->GetSelCount() <= 0 ||
		m_pInputFace->GetSelCount() <= 0)
		return FALSE;

	kiSelEntity se = m_pInputEntity->GetFirstSelect();
	TopoDS_Shape aShape = se._pEntity->GetShape();
	kcEntity *pSelEnt = se._pEntity;
	double dThick = m_dThick;
	if(!m_bOutSide)
		dThick = -dThick;

	TopoDS_Shape aFace;
	TopTools_ListOfShape aFaceList;
	kiSelSet *pSelSet = m_pInputFace->GetSelSet();
	for(pSelSet->InitSelected();pSelSet->MoreSelected();pSelSet->NextSelected()){
		se = pSelSet->CurSelected();
		aFace = se.SelectShape();
		if(!aFace.IsNull()){
			aFaceList.Append(aFace);
		}
	}
	

	TopoDS_Shape aNS;
	try{
		BRepOffsetAPI_MakeThickSolid  aMakeThickSolid;
		aMakeThickSolid.MakeThickSolidByJoin(aShape,aFaceList,dThick,0.0001);
		//msolid.Build();
		if(aMakeThickSolid.IsDone())
			aNS = aMakeThickSolid.Shape();
		if(aNS.IsNull())
			return FALSE;
	}catch(Standard_Failure){
		return FALSE;
	}

	kcEntityFactory efac;
	kcEntity *pEntity = efac.Create(aNS);
	if(pEntity)
	{
		kcModel *pModel = GetModel();
		pModel->BeginUndo(GetName());
		pModel->AddEntity(pEntity);
		pModel->DelEntity(pSelEnt);
		pModel->EndUndo();

		return TRUE;
	}

	return FALSE;
}

//////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////
//
kcmThickSolid::kcmThickSolid(void)
{
	m_strName = "等厚";
	m_strAlias = "ThickSolid";
	m_strDesc = "等厚生成实体";

	m_pInputEntity = NULL;
	m_pInputThickness = NULL;
	m_bRevDir = false;
	m_dThick = 2.0;
}

kcmThickSolid::~kcmThickSolid(void)
{
}


// 表明命令可以完成,主要和Apply函数结合使用.
BOOL  kcmThickSolid::CanFinish()//命令是否可以完成
{
	return FALSE;
}

// 当输入工具获取结果,结束时,会回调对应命令的该函数.
// 用于继续执行命令或切换输入工具.也可能返回KSTA_DONE,表明命令执行结束.
//
int  kcmThickSolid::OnInputFinished(kiInputTool *pTool)
{
	if(pTool == m_pInputEntity){
		if(m_pInputEntity->IsInputDone()){
			kiSelEntity se = m_pInputEntity->GetFirstSelect();
			NavToNextTool();
		}
	}else if(pTool == m_pInputThickness){
		if(m_pInputThickness->IsInputDone()){
			if(BuildThickSolid()){
				Done();
				Redraw();

				return KSTA_DONE;
			}else{
				EndCommand(KSTA_FAILED);
				return KSTA_FAILED;
			}
		}
	}
	return KSTA_CONTINUE;
}

int  kcmThickSolid::OnExecute()
{
	return KSTA_CONTINUE;
}

int  kcmThickSolid::OnEnd(int nCode)
{
	m_pInputEntity->ClearSel();

	return nCode;
}

// 创建必要的输入工具
BOOL  kcmThickSolid::CreateInputTools()
{
	m_optionSet.AddBoolOption("反向",'R',m_bRevDir);

	m_pInputEntity = new kiInputEntityTool(this,"选择面/壳",&m_optionSet);
	m_pInputEntity->SetOption(KCT_ENT_FACE | KCT_ENT_SHELL,1);

	m_pInputThickness = new kiInputDoubleTool(this,"输入厚度");
	m_pInputThickness->Init(&m_dThick,1e-6,1000);

	return TRUE;
}

BOOL	kcmThickSolid::DestroyInputTools()
{
	KC_SAFE_DELETE(m_pInputEntity);
	KC_SAFE_DELETE(m_pInputThickness);

	return TRUE;
}

BOOL	kcmThickSolid::InitInputToolQueue()
{
	AddInputTool(m_pInputEntity);
	AddInputTool(m_pInputThickness);

	return TRUE;
}

BOOL	kcmThickSolid::BuildThickSolid()
{
	if(m_pInputEntity->GetSelCount() <= 0)
		return FALSE;

	kiSelEntity se = m_pInputEntity->GetFirstSelect();
	TopoDS_Shape aShape = se._pEntity->GetShape();
	kcEntity *pSelEnt = se._pEntity;
	double dThick = m_dThick;
	if(m_bRevDir)
		dThick = -dThick;

	TopoDS_Shape aNS;
	try{
		BRepOffsetAPI_MakeThickSolid  aMakeThickSolid;
		aMakeThickSolid.MakeThickSolidBySimple(aShape,dThick);

		if(aMakeThickSolid.IsDone())
			aNS = aMakeThickSolid.Shape();
		if(aNS.IsNull())
			return FALSE;
	}catch(Standard_Failure){
		return FALSE;
	}

	kcEntityFactory efac;
	kcEntity *pEntity = efac.Create(aNS);
	if(pEntity)
	{
		kcModel *pModel = GetModel();
		pModel->BeginUndo(GetName());
		pModel->AddEntity(pEntity);
		pModel->DelEntity(pSelEnt);
		pModel->EndUndo();

		return TRUE;
	}

	return FALSE;
}
