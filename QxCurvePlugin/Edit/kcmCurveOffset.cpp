#include "StdAfx.h"
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepOffsetAPI_MakeOffset.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include "kcEntity.h"
#include "kcModel.h"
#include "QxBRepLib.h"
#include "kiInputEntityTool.h"
#include "kiInputValueTool.h"
#include "kcmCurveOffset.h"

kcmCurveOffset::kcmCurveOffset(void)
{
	m_strName = "CurveOffset";
	m_strAlias = "CurOff";
	m_strDesc = "曲线偏移";

	m_pInputEntityTool = NULL;
	m_pInputOffset = NULL;
	m_dOffset = 20;
}

kcmCurveOffset::~kcmCurveOffset(void)
{
	KC_SAFE_DELETE(m_pInputEntityTool);
}

int	kcmCurveOffset::Destroy()
{
	KC_SAFE_DELETE(m_pInputEntityTool);

	return kiCommand::Destroy();
}

//执行和结束接口
int	kcmCurveOffset::Execute()
{
	return kiCommand::Execute();
}

int	kcmCurveOffset::End(int nCode)
{
	if(m_pInputEntityTool)
	{
		m_pInputEntityTool->GetSelSet()->UnHilight(TRUE);
	}

	return kiCommand::End(nCode);
}

int	kcmCurveOffset::Apply()//实际应用
{
	return KSTA_DONE;
}

BOOL	kcmCurveOffset::CanFinish()//命令是否可以完成
{
	return FALSE;
}

//输入工具结束时
int	kcmCurveOffset::OnInputFinished(kiInputTool *pTool)
{
	if(pTool == m_pInputEntityTool)
	{
		if(m_pInputEntityTool->IsInputDone())
		{
			NavToNextTool();
		}
	}
	else if(pTool == m_pInputOffset)
	{
		if(MakeOffset())
		{
			m_pInputEntityTool->ClearSel();
			//结束了
			Done();
			Redraw();

			return KSTA_DONE;
		}
		else
		{
			AddInputTool(m_pInputOffset);
			NavToInputTool(m_pInputEntityTool);
		}
	}
	return KSTA_CONTINUE;
}

//
bool	kcmCurveOffset::MakeOffset()
{
	if(fabs(m_dOffset) < KDBL_MIN)
		return false;

	if(m_pInputEntityTool->GetSelCount() <= 0)
		return false;

	kcEntity *pEnt = NULL;
	kiSelSet *pSelSet = m_pInputEntityTool->GetSelSet();
	pSelSet->InitSelected();
	while(pSelSet->MoreSelected())
	{
		kiSelEntity selEnt = pSelSet->CurSelected();
		if(selEnt._pEntity && 
			(IS_EDGE_ENTITY(selEnt._pEntity) || IS_WIRE_ENTITY(selEnt._pEntity)))
		{
			pEnt = selEnt._pEntity;
			break;
		}

		pSelSet->NextSelected();
	}
	if(pEnt == NULL)
		return false;
	
	TopoDS_Wire aWire;
	if(IS_EDGE_ENTITY(pEnt))
	{
		kcEdgeEntity *pCurEnt = (kcEdgeEntity *)pEnt;
		TopoDS_Edge aEdge = pCurEnt->GetEdge();
		TopoDS_Edge aCopyEdge = QxBRepLib::CopyEdge(aEdge);
		if(aCopyEdge.IsNull())
			return false;

		BRepBuilderAPI_MakeWire mw(aCopyEdge);
		if(!mw.IsDone())
			return false;

		aWire = mw.Wire();
	}
	else
	{
		kcWireEntity *pWireEnt = (kcWireEntity *)pEnt;
		aWire = QxBRepLib::CopyWire(pWireEnt->GetWire());
		if(aWire.IsNull())
			return false;
	}

	BRepOffsetAPI_MakeOffset moff;
	moff.Init(GeomAbs_Arc);
	moff.AddWire(aWire);
	moff.Perform(m_dOffset);
	if(!moff.IsDone())
		return false;

	TopoDS_Shape aS = moff.Shape();
	if(aS.IsNull() || 
		(aS.ShapeType() != TopAbs_EDGE && aS.ShapeType() != TopAbs_WIRE))
	{
		return false;
	}

	kcEntity *pEntity = NULL;
	if(aS.ShapeType() == TopAbs_EDGE)
	{
		TopoDS_Edge aNE = TopoDS::Edge(aS);
		kcEdgeEntity *pCurEnt = new kcEdgeEntity;
		pCurEnt->SetEdge(aNE);
		pEntity = pCurEnt;
	}
	else
	{
		TopoDS_Wire aW = TopoDS::Wire(aS);
		kcWireEntity *pWireEnt = new kcWireEntity;
		pWireEnt->SetWire(aW);
		pEntity = pWireEnt;
	}

	kcModel *pModel = GetModel();
	pModel->BeginUndo(GetName());
	pModel->AddEntity(pEntity);
	pModel->EndUndo();

	pSelSet->UnHilight(TRUE);

	return true;
}

// 创建必要的输入工具
BOOL	kcmCurveOffset::CreateInputTools()
{
	m_pInputEntityTool = new kiInputEntityTool(this,"选择一条曲线:");
	m_pInputEntityTool->SetOption(KCT_ENT_EDGE | KCT_ENT_WIRE,1);
	m_pInputEntityTool->SetNaturalMode(true);

	m_pInputOffset = new kiInputDoubleTool(this,"输入偏移值:");
	m_pInputOffset->Init(&m_dOffset);

	return TRUE;
}

BOOL	kcmCurveOffset::InitInputToolQueue()
{
	AddInputTool(m_pInputEntityTool);
	AddInputTool(m_pInputOffset);

	return TRUE;
}
