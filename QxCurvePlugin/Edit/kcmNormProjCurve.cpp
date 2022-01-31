#include "StdAfx.h"
#include <BRepOffsetAPI_NormalProjection.hxx>
#include "kcEntity.h"
#include "kcModel.h"
#include "QxBRepLib.h"
#include "QxCurveLib.h"
#include "kiInputEntityTool.h"
#include "kiInputValueTool.h"
#include "kiInputPointOnCurve.h"
#include "kcEntityFactory.h"
#include "kcmNormProjCurve.h"

kcmNormProjCurve::kcmNormProjCurve(void)
{
	m_strName = "投影到面";
	m_strAlias = "PrjCrv";
	m_strDesc = "曲线投影到曲面";

	m_pPickCurveTool = NULL;
	m_pPickFaceTool = NULL;
}

kcmNormProjCurve::~kcmNormProjCurve(void)
{
}

// 表明命令可以完成,主要和Apply函数结合使用.
BOOL	kcmNormProjCurve::CanFinish()//命令是否可以完成
{
	return FALSE;
}

// 当输入工具获取结果,结束时,会回调对应命令的该函数.
// 用于继续执行命令或切换输入工具.也可能返回KSTA_DONE,表明命令执行结束.
//
int	kcmNormProjCurve::OnInputFinished(kiInputTool *pTool)
{
	if(pTool == m_pPickCurveTool)
	{
		if(pTool->IsInputDone())
		{
			NavToNextTool();
		}
	}
	else if(pTool == m_pPickFaceTool)
	{
		if(pTool->IsInputDone())
		{
			m_pPickFaceTool->End();
			//
			if(DoNormProj())
			{
				Done();
				return KSTA_DONE;
			}
			else
			{
				EndCommand(KSTA_FAILED);
				return KSTA_FAILED;
			}
		}
		else
		{
			EndCommand(KSTA_CANCEL);
			return KSTA_CANCEL;
		}
	}
	return KSTA_CONTINUE;
}

int		kcmNormProjCurve::OnExecute()
{
	return KSTA_CONTINUE;
}

int		kcmNormProjCurve::OnEnd(int nCode)
{
	return nCode;
}

// 创建必要的输入工具
BOOL	kcmNormProjCurve::CreateInputTools()
{
	m_pPickCurveTool = new kiInputEntityTool(this,"拾取要投影的曲线");
	m_pPickCurveTool->SetOption(KCT_ENT_EDGE | KCT_ENT_WIRE);
	m_pPickCurveTool->SetNaturalMode(true);

	m_pPickFaceTool = new kiInputEntityTool(this,"拾取投影到曲面");
	m_pPickFaceTool->SetOption(KCT_ENT_FACE,true);
	m_pPickFaceTool->SetNaturalMode(false); //可以选择局部对象

	return TRUE;
}

BOOL	kcmNormProjCurve::DestroyInputTools()
{
	KC_SAFE_DELETE(m_pPickCurveTool);
	KC_SAFE_DELETE(m_pPickFaceTool);

	return TRUE;
}

BOOL	kcmNormProjCurve::InitInputToolQueue()
{
	AddInputTool(m_pPickCurveTool);
	AddInputTool(m_pPickFaceTool);

	return TRUE;
}

//
BOOL	kcmNormProjCurve::DoNormProj()
{
	if(m_pPickCurveTool->GetSelCount() <= 0)
		return FALSE;
	int nbShape = m_pPickFaceTool->GetSelCount();
	if(nbShape != 1)
		return FALSE;
	TopoDS_Shape aShape = m_pPickFaceTool->SelectedShape(0);

	TopoDS_Shape aNS;
	try{
		BRepOffsetAPI_NormalProjection np(aShape);
		kiSelSet *pSelSet = m_pPickCurveTool->GetSelSet();
		pSelSet->InitSelected();
		while(pSelSet->MoreSelected())
		{
			kiSelEntity se = pSelSet->CurSelected();
			TopoDS_Shape aS = se._pEntity->GetShape();
			np.Add(aS);
			pSelSet->NextSelected();
		}
		np.SetLimit(Standard_True);
		np.Compute3d(Standard_True);
		np.Build();
		if(np.IsDone())
			aNS = np.Shape();
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

