#include "StdAfx.h"
#include "kiInputPoint.h"
#include "kPoint.h"
#include "kVector.h"
#include "kcBasePlane.h"
#include "kcModel.h"
#include "kcEntity.h"
#include "kiInputLength.h"
#include "kiInputAngle.h"
#include "kiInputEntityTool.h"
#include "kvCoordSystem.h"
#include "kcmUtils.h"
#include "kcPreviewObj.h"
#include "kcgIntersect.h"
#include "kcPreviewText.h"
#include "kcmPoint.h"

kcmPoint::kcmPoint(void)
{
	m_strName = "Point";
	m_strAlias = "Pnt";
	m_strDesc = "create point";

	m_pInputPoint = NULL;
}

kcmPoint::~kcmPoint(void)
{
}

// 表明命令可以完成,主要和Apply函数结合使用.
//
BOOL	kcmPoint::CanFinish()
{
	return TRUE;
}

// 当输入工具获取结果,结束时,会回调对应命令的该函数.
// 用于继续执行命令或切换输入工具.也可能返回KSTA_DONE,表明命令执行结束.
//
int	kcmPoint::OnInputFinished(kiInputTool *pTool)
{
	if(pTool == m_pInputPoint)
	{
		if(m_pInputPoint->IsInputDone())
		{
			kcPointEntity *pPntEntity = new kcPointEntity;
			if(pPntEntity)
			{
				pPntEntity->SetPoint(m_aPoint[0],m_aPoint[1],m_aPoint[2]);

				kcModel *pModel = GetModel();
				pModel->BeginUndo(GetName());
				pModel->AddEntity(pPntEntity);
				pModel->EndUndo();

				Redraw();
			}
		}
		
		NavToInputTool(m_pInputPoint);
	}

	return KSTA_CONTINUE;
}

int	kcmPoint::OnExecute()
{
	return KSTA_CONTINUE;
}

// 创建必要的输入工具,在Initialize中调用，每个命令仅仅调用一次。
BOOL	kcmPoint::CreateInputTools()
{
	m_pInputPoint = new kiInputPoint(this,"输入点:");
	m_pInputPoint->Init(&m_aPoint);

	return TRUE;
}

// 销毁创建的输入工具.每个命令仅调用一次.
BOOL	kcmPoint::DestroyInputTools()
{
	KC_SAFE_DELETE(m_pInputPoint);

	return TRUE;
}

// 在begin中调用，主要是初始化工具队列。
BOOL	kcmPoint::InitInputToolQueue()
{
	AddInputTool(m_pInputPoint);

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// 曲线交点
kcmCurveInterPoint::kcmCurveInterPoint()
{
	m_strName = "曲线交点";
	m_strAlias = "CurIntpt";
	m_strDesc = "两曲线交点";

	m_pInputCurve1 = NULL;
	m_pInputCurve2 = NULL;
}

kcmCurveInterPoint::~kcmCurveInterPoint()
{

}

// 表明命令可以完成,主要和Apply函数结合使用.
BOOL	kcmCurveInterPoint::CanFinish()//命令是否可以完成
{
	return FALSE;
}

// 当输入工具获取结果,结束时,会回调对应命令的该函数.
// 用于继续执行命令或切换输入工具.也可能返回KSTA_DONE,表明命令执行结束.
//
int	kcmCurveInterPoint::OnInputFinished(kiInputTool *pTool)
{
	if(pTool == m_pInputCurve1)
	{
		if(m_pInputCurve1->IsInputDone())
		{
			NavToNextTool();
		}
	}
	else if(pTool == m_pInputCurve2)
	{
		if(m_pInputCurve2->IsInputDone())
		{
			if(CalcInterPoint())
			{
				m_pInputCurve1->ClearSel();
				m_pInputCurve2->ClearSel();

				Done();
				Redraw();
				return KSTA_DONE;
			}
			else
			{
				m_pInputCurve1->ClearSel();
				m_pInputCurve2->ClearSel();

				return KSTA_ERROR;
			}
		}
	}
	return kiCommand::OnInputFinished(pTool);
}

BOOL	kcmCurveInterPoint::CalcInterPoint()
{
	kiSelEntity se1,se2;
	kiSelSet *pSelSet = m_pInputCurve1->GetSelSet();
	pSelSet->InitSelected();
	se1 = pSelSet->CurSelected();

	pSelSet = m_pInputCurve2->GetSelSet();
	pSelSet->InitSelected();
	se2 = pSelSet->CurSelected();

	kcgCurveCurveInter ccinter;
	std::vector<kcEntity *> aEnt;
	if(IS_EDGE_ENTITY(se1._pEntity))
	{
		TopoDS_Edge aEdge1 = ((kcEdgeEntity *)se1._pEntity)->GetEdge();
		if(IS_EDGE_ENTITY(se2._pEntity))
		{
			TopoDS_Edge aEdge2 = ((kcEdgeEntity *)se2._pEntity)->GetEdge();
			ccinter.Intersect(aEdge1,aEdge2);
		}
		else
		{
			TopoDS_Wire aWire = ((kcWireEntity *)se2._pEntity)->GetWire();
			ccinter.Intersect(aEdge1,aWire);
		}
	}
	else
	{
		TopoDS_Wire aWire1 = ((kcWireEntity *)se1._pEntity)->GetWire();
		if(IS_EDGE_ENTITY(se2._pEntity))
		{
			TopoDS_Edge aEdge = ((kcEdgeEntity *)se2._pEntity)->GetEdge();
			ccinter.Intersect(aEdge,aWire1);
		}
		else
		{
			TopoDS_Wire aWire2 = ((kcWireEntity *)se2._pEntity)->GetWire();
			ccinter.Intersect(aWire1,aWire2);
		}
	}
	int ix;
	for(ix = 0;ix < ccinter.NbInterPoints();ix ++)
	{
		kcxCCIntpt ipt = ccinter.InterPoint(ix);
		kcPointEntity *pEnt = new kcPointEntity;
		pEnt->SetPoint(ipt._ipt1._p[0],ipt._ipt1._p[1],ipt._ipt1._p[2]);
		aEnt.push_back(pEnt);
	}

	if(!aEnt.empty())
	{
		kcModel *pModel = GetModel();
		pModel->BeginUndo(GetName());
		for(ix = 0;ix < (int)aEnt.size();ix ++)
			pModel->AddEntity(aEnt[ix]);
		pModel->EndUndo();

		return TRUE;
	}

	return FALSE;
}

int		kcmCurveInterPoint::OnExecute()
{
	return KSTA_CONTINUE;
}

int		kcmCurveInterPoint::OnEnd(int nCode)
{
	m_pInputCurve1->ClearSel();
	m_pInputCurve2->ClearSel();

	return nCode;
}

// 创建必要的输入工具
BOOL	kcmCurveInterPoint::CreateInputTools()
{
	m_pInputCurve1 = new kiInputEntityTool(this,"选择第一条曲线");
	m_pInputCurve1->SetOption(KCT_ENT_EDGE | KCT_ENT_WIRE,1);

	m_pInputCurve2 = new kiInputEntityTool(this,"选择二条曲线");
	m_pInputCurve2->SetOption(KCT_ENT_EDGE | KCT_ENT_WIRE,1);

	return TRUE;
}

BOOL	kcmCurveInterPoint::DestroyInputTools()
{
	KC_SAFE_DELETE(m_pInputCurve1);
	KC_SAFE_DELETE(m_pInputCurve2);
	return TRUE;
}

BOOL	kcmCurveInterPoint::InitInputToolQueue()
{
	AddInputTool(m_pInputCurve1);
	AddInputTool(m_pInputCurve2);
	return TRUE;
}
