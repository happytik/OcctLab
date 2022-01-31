#include "StdAfx.h"
#include "kcEntity.h"
#include "kcModel.h"
#include "QxBRepLib.h"
#include "QxBaseUtils.h"
#include "kiInputEntityTool.h"
#include "kiInputValueTool.h"
#include "kiInputPointOnCurve.h"
#include "kcEntityFactory.h"
#include "kcmcurvebreak.h"

//////////////////////////////////////////////////////////////////////////
//
kcmCurveBreakByPoint::kcmCurveBreakByPoint(void)
{
	m_strName = "用点打断";
	m_strAlias = "BrkByPoint";
	m_strDesc = "用点打断曲线";

	m_pInputCurve = NULL;
	m_pPickPointOnCurve = NULL;
}

kcmCurveBreakByPoint::~kcmCurveBreakByPoint(void)
{
}

// 表明命令可以完成,主要和Apply函数结合使用.
BOOL	kcmCurveBreakByPoint::CanFinish()//命令是否可以完成
{
	return FALSE;
}

// 当输入工具获取结果,结束时,会回调对应命令的该函数.
// 用于继续执行命令或切换输入工具.也可能返回KSTA_DONE,表明命令执行结束.
//
int	kcmCurveBreakByPoint::OnInputFinished(kiInputTool *pTool)
{
	if(pTool == m_pInputCurve)
	{
		if(m_pInputCurve->IsInputDone())
		{
			kiSelEntity se = m_pInputCurve->GetFirstSelect();
			TopoDS_Shape aS = se._pEntity->GetShape();
			if(aS.ShapeType() == TopAbs_EDGE)
				m_pPickPointOnCurve->Init(TopoDS::Edge(aS),6);
			else
				m_pPickPointOnCurve->Init(TopoDS::Wire(aS),6);

			NavToNextTool();
		}
	}
	else if(pTool == m_pPickPointOnCurve)
	{
		if(m_pPickPointOnCurve->IsInputDone())
		{
			if(DoBreak())
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

int		kcmCurveBreakByPoint::OnExecute()
{
	return KSTA_CONTINUE;
}

int		kcmCurveBreakByPoint::OnEnd(int nCode)
{
	m_pInputCurve->ClearSel();
	return nCode;
}

// 创建必要的输入工具
BOOL	kcmCurveBreakByPoint::CreateInputTools()
{
	m_pInputCurve = new kiInputEntityTool(this,"选择要打断的曲线");
	m_pInputCurve->SetOption(KCT_ENT_EDGE | KCT_ENT_WIRE,1);

	m_pPickPointOnCurve = new kiInputPointOnCurve(this,"拾取打断点");
    
	return TRUE;
}

BOOL	kcmCurveBreakByPoint::DestroyInputTools()
{
	KC_SAFE_DELETE(m_pInputCurve);
	KC_SAFE_DELETE(m_pPickPointOnCurve);

	return TRUE;
}

BOOL	kcmCurveBreakByPoint::InitInputToolQueue()
{
	AddInputTool(m_pInputCurve);
	AddInputTool(m_pPickPointOnCurve);

	return TRUE;
}

BOOL	kcmCurveBreakByPoint::DoBreak()
{
	if(m_pInputCurve->GetSelCount() <= 0)
		return FALSE;
	kiSelEntity se = m_pInputCurve->GetFirstSelect();
	TopoDS_Shape aShape = se._pEntity->GetShape();
	
	TopTools_SequenceOfShape seqOfShape;
	if(aShape.ShapeType() == TopAbs_EDGE)
	{
		TopoDS_Edge aE1,aE2;
		double t = m_pPickPointOnCurve->GetPickedParam();
		if(!kcgEdgeSplit::SplitEdge(TopoDS::Edge(aShape),t,aE1,aE2))
			return FALSE;
		seqOfShape.Append(aE1);
		seqOfShape.Append(aE2);
	}
	else
	{
		kPoint3 pp = m_pPickPointOnCurve->GetPickedPoint();
		if(!kcgWireSplit::SplitWire(TopoDS::Wire(aShape),KCG_GP_PNT(pp),
			1e-4,seqOfShape))
			return FALSE;
	}
	//
	kcEntityFactory efac;
	std::vector<kcEntity *> aEnt;
	kcEntity *pEntity = NULL;
	int ix,nb = seqOfShape.Length();
	for(ix = 1;ix <= nb;ix ++)
	{
		pEntity = efac.Create(seqOfShape.Value(ix));
		if(pEntity)
			aEnt.push_back(pEntity);
	}
	if(aEnt.empty())
		return FALSE;

	kcModel *pModel = GetModel();
	pModel->BeginUndo(GetName());
	{
		pModel->DelEntity(se._pEntity);//去除旧的
		nb = (int)aEnt.size();
		for(ix = 0;ix < nb;ix ++)
			pModel->AddEntity(aEnt[ix]);
	}
	pModel->EndUndo();

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
// 曲线打断
kcmCurveBreakByCurve::kcmCurveBreakByCurve()
{
	m_strName = "使用曲线";
	m_strAlias = "BreakByCrv";
	m_strDesc = "使用曲线打断";

	m_pInputCurve = NULL;
	m_pInputCurve2 = NULL;
}

kcmCurveBreakByCurve::~kcmCurveBreakByCurve()
{

}

// 表明命令可以完成,主要和Apply函数结合使用.
BOOL	kcmCurveBreakByCurve::CanFinish()//命令是否可以完成
{
	return FALSE;
}

// 当输入工具获取结果,结束时,会回调对应命令的该函数.
// 用于继续执行命令或切换输入工具.也可能返回KSTA_DONE,表明命令执行结束.
//
int	kcmCurveBreakByCurve::OnInputFinished(kiInputTool *pTool)
{
	if(pTool == m_pInputCurve)
	{
		if(m_pInputCurve->IsInputDone())
		{
			NavToNextTool();
		}
	}
	else if(pTool == m_pInputCurve2)
	{
		if(m_pInputCurve2->IsInputDone())
		{
			if(DoBreak())
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

int		kcmCurveBreakByCurve::OnExecute()
{
	return KSTA_CONTINUE;
}

int		kcmCurveBreakByCurve::OnEnd(int nCode)
{
	m_pInputCurve->ClearSel();
	m_pInputCurve2->ClearSel();

	return nCode;
}


// 创建必要的输入工具
BOOL	kcmCurveBreakByCurve::CreateInputTools()
{
	m_pInputCurve = new kiInputEntityTool(this,"选择被打断曲线");
	m_pInputCurve->SetOption(KCT_ENT_EDGE | KCT_ENT_WIRE);

	m_pInputCurve2 = new kiInputEntityTool(this,"选择分割曲线");
	m_pInputCurve2->SetOption(KCT_ENT_EDGE | KCT_ENT_WIRE,1);

	return TRUE;
}

BOOL	kcmCurveBreakByCurve::DestroyInputTools()
{
	KC_SAFE_DELETE(m_pInputCurve);
	KC_SAFE_DELETE(m_pInputCurve2);
	return TRUE;
}

BOOL	kcmCurveBreakByCurve::InitInputToolQueue()
{
	AddInputTool(m_pInputCurve);
	AddInputTool(m_pInputCurve2);
	return TRUE;
}

BOOL	kcmCurveBreakByCurve::DoBreak()
{
	if(m_pInputCurve->GetSelCount() <= 0 ||
	   m_pInputCurve2->GetSelCount() <= 0)
	   return FALSE;

	//分割用曲线
	kiSelEntity se = m_pInputCurve2->GetFirstSelect();
	TopoDS_Shape aShape = se._pEntity->GetShape();

	//被分割曲线
	TopoDS_Edge aEdge,aEdgeTool;
	TopoDS_Wire aWire,aWireTool;
	std::vector<kcEntity *> aSelEnt,aNewEnt;
	TopTools_SequenceOfShape seqOfShape,seqTmp;
	BOOL bEdgeTool = FALSE;

	if(IS_EDGE_ENTITY(se._pEntity))
	{
		aEdgeTool = TopoDS::Edge(aShape);
		bEdgeTool = TRUE;
	}
	else
		aWireTool = TopoDS::Wire(aShape);

	int ix,nC = 0;
	kiSelSet *pSelSet = m_pInputCurve->GetSelSet();
	for(pSelSet->InitSelected();pSelSet->MoreSelected();pSelSet->NextSelected())
	{
		se = pSelSet->CurSelected();

		seqTmp.Clear();
		if(IS_EDGE_ENTITY(se._pEntity))
		{
			aEdge = TopoDS::Edge(se._pEntity->GetShape());
			if(bEdgeTool)
				nC = kcgEdgeSplit::SplitEdgeByEdge(aEdge,aEdgeTool,seqTmp);
			else
				nC = kcgEdgeSplit::SplitEdgeByWire(aEdge,aWireTool,seqTmp);
		}
		else
		{
			aWire = TopoDS::Wire(se._pEntity->GetShape());
			if(bEdgeTool)
				nC = kcgWireSplit::SplitWireByEdge(aWire,aEdgeTool,seqTmp);
			else
				nC = kcgWireSplit::SplitWireByWire(aWire,aWireTool,seqTmp);
		}
		//
		if(nC > 0)
		{
			for(ix = 1;ix <= nC;ix ++)
				seqOfShape.Append(seqTmp.Value(ix));
			seqTmp.Clear();
			aSelEnt.push_back(se._pEntity);
		}
	}
	if(aSelEnt.empty())
		return FALSE;

	//创建新entity
	kcEntityFactory efac;
	kcEntity *pEntity = NULL;
	nC = seqOfShape.Length();
	for(ix = 1;ix <= nC;ix ++)
	{
		aShape = seqOfShape.Value(ix);
		pEntity = efac.Create(aShape);
		if(pEntity)
			aNewEnt.push_back(pEntity);
	}
	if(aNewEnt.empty())
		return FALSE;

	kcModel *pModel = GetModel();
	pModel->BeginUndo(GetName());
	{
		nC = (int)aSelEnt.size();
		for(ix = 0;ix < nC;ix ++)
			pModel->DelEntity(aSelEnt[ix]);
		//添加新的
		nC = (int)aNewEnt.size();
		for(ix = 0;ix < nC;ix ++)
			pModel->AddEntity(aNewEnt[ix]);
	}
	pModel->EndUndo();
	

	return TRUE;
}
