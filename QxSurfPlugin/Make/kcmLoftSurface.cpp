#include "StdAfx.h"
#include <BRepOffsetAPI_ThruSections.hxx>
#include "kiInputEntityTool.h"
#include "kiInputVector.h"
#include "kcEntity.h"
#include "kcBasePlane.h"
#include "kcModel.h"
#include "QxBRepLib.h"
#include "kvCoordSystem.h"
#include "kcEntityFactory.h"
#include "kcmLoftSurface.h"

kcmLoftSurface::kcmLoftSurface(void)
{
	m_strName = "蒙面";
	m_strAlias = "Loft";
	m_strDesc = "蒙面";

	m_pInputStart = NULL;
	m_pInputThrough = NULL;
	m_pInputEnd = NULL;
}

kcmLoftSurface::~kcmLoftSurface(void)
{
}

// 表明命令可以完成,主要和Apply函数结合使用.
BOOL	kcmLoftSurface::CanFinish()//命令是否可以完成
{
	return FALSE;
}

// 当输入工具获取结果,结束时,会回调对应命令的该函数.
// 用于继续执行命令或切换输入工具.也可能返回KSTA_DONE,表明命令执行结束.
//
int	kcmLoftSurface::OnInputFinished(kiInputTool *pTool)
{
	if(pTool == m_pInputStart)
	{
		if(m_pInputStart->IsInputDone())
		{
			NavToNextTool();
		}
	}
	else if(pTool == m_pInputThrough)
	{
		NavToNextTool();
	}
	else if(pTool == m_pInputEnd)
	{
		if(m_pInputEnd->IsInputDone())
		{
			if(DoLoft())
			{
				Done();
				Redraw();

				return KSTA_DONE;
			}
		}
	}
	return kiCommand::OnInputFinished(pTool);
}

static BOOL auxAddSection(BRepOffsetAPI_ThruSections& tsec,kiSelEntity se)
{
	if(IS_POINT_ENTITY(se._pEntity))
	{
		TopoDS_Vertex aV = TopoDS::Vertex(se._pEntity->GetShape());
		tsec.AddVertex(aV);
	}
	else if(IS_EDGE_ENTITY(se._pEntity))
	{
		TopoDS_Edge aEdge = TopoDS::Edge(se._pEntity->GetShape());
		TopoDS_Wire aWire = QxBRepLib::EdgeToWire(aEdge);
		if(aWire.IsNull())
			return FALSE;
		tsec.AddWire(aWire);
	}
	else
	{
		TopoDS_Wire aWire = TopoDS::Wire(se._pEntity->GetShape());
		tsec.AddWire(aWire);
	}
	return TRUE;
}

BOOL	kcmLoftSurface::DoLoft()
{
	if(m_pInputStart->GetSelCount() != 1 ||
	   m_pInputEnd->GetSelCount() != 1)
	   return FALSE;

	TopoDS_Shape aNS;
	try{
		BRepOffsetAPI_ThruSections tsec;
		kiSelEntity se = m_pInputStart->GetFirstSelect();
		if(!auxAddSection(tsec,se))
			return FALSE;

		if(m_pInputThrough->GetSelCount() > 0)
		{
			kiSelSet *pSelSet = m_pInputThrough->GetSelSet();
			pSelSet->InitSelected();
			while(pSelSet->MoreSelected())
			{
				se = pSelSet->CurSelected();
				auxAddSection(tsec,se);
				pSelSet->NextSelected();
			}
		}

		se = m_pInputEnd->GetFirstSelect();
		auxAddSection(tsec,se);

		tsec.CheckCompatibility();
		tsec.Build();
		if(tsec.IsDone())
			aNS = tsec.Shape();
	}catch(Standard_Failure){}

	if(aNS.IsNull())
		return FALSE;

	kcEntityFactory entfac;
	kcEntity *pEnt = entfac.Create(aNS);
	if(pEnt)
	{
		kcModel *pModel = GetModel();
		pModel->BeginUndo(GetName());
		pModel->AddEntity(pEnt);
		pModel->EndUndo();

		return TRUE;
	}

	return FALSE;
}

int		kcmLoftSurface::OnExecute()
{
	m_pInputStart->ClearSel();
	m_pInputThrough->ClearSel();
	m_pInputEnd->ClearSel();

	return KSTA_CONTINUE;
}

int		kcmLoftSurface::OnEnd(int nCode)
{
	m_pInputStart->ClearSel();
	m_pInputThrough->ClearSel();
	m_pInputEnd->ClearSel();

	return nCode;
}

// 创建必要的输入工具
BOOL	kcmLoftSurface::CreateInputTools()
{
	m_pInputStart = new kiInputEntityTool(this,"拾取起始点或曲线");
	m_pInputStart->SetOption(KCT_ENT_POINT | KCT_ENT_EDGE | KCT_ENT_WIRE,1);

	m_pInputThrough = new kiInputEntityTool(this,"拾取通过曲线");
	m_pInputThrough->SetOption(KCT_ENT_EDGE | KCT_ENT_WIRE);

	m_pInputEnd = new kiInputEntityTool(this,"拾取结束点或曲线");
	m_pInputEnd->SetOption(KCT_ENT_POINT | KCT_ENT_EDGE | KCT_ENT_WIRE,1);

	return TRUE;
}

BOOL	kcmLoftSurface::DestroyInputTools()
{
	KC_SAFE_DELETE(m_pInputStart);
	KC_SAFE_DELETE(m_pInputThrough);
	KC_SAFE_DELETE(m_pInputEnd);

	return TRUE;
}

BOOL	kcmLoftSurface::InitInputToolQueue()
{
	AddInputTool(m_pInputStart);
	AddInputTool(m_pInputThrough);
	AddInputTool(m_pInputEnd);

	return TRUE;
}
