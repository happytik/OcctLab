#include "StdAfx.h"
#include <BRepOffsetAPI_MakeDraft.hxx>
#include "kiInputEntityTool.h"
#include "kiInputVector.h"
#include "kiInputValueTool.h"
#include "kcEntity.h"
#include "kcBasePlane.h"
#include "kcModel.h"
#include "QxBRepLib.h"
#include "QxCurveLib.h"
#include "kvCoordSystem.h"
#include "kcEntityFactory.h"
#include "kcmSolidDraft.h"

kcmSolidDraft::kcmSolidDraft(void)
{
	m_strName = "拔模拉伸";
	m_strAlias = "SolidDraft";
	m_strDesc = "实体拔模拉伸";

	m_pPickEntityTool = NULL;
	m_pInputVectorTool = NULL;
	m_pInputDoubleTool = NULL;
	m_pPickStopEntityTool = NULL;
	m_dAngle = K_PI / 6;

	m_pDraftMode = NULL;
	m_nDraftMode = 0;
	m_pTransitionMode = NULL;
	m_nTransitionMode = 0;
	m_bInternal = false;

	m_bReady = false;
}

kcmSolidDraft::~kcmSolidDraft(void)
{
}

// 表明命令可以完成,主要和Apply函数结合使用.
BOOL	kcmSolidDraft::CanFinish()//命令是否可以完成
{
	if(m_bReady)
		return TRUE;
	return FALSE;
}

// 当输入工具获取结果,结束时,会回调对应命令的该函数.
// 用于继续执行命令或切换输入工具.也可能返回KSTA_DONE,表明命令执行结束.
//
int		kcmSolidDraft::OnInputFinished(kiInputTool *pTool)
{
	if(pTool == m_pPickEntityTool)
	{
		if(m_pPickEntityTool->IsInputDone())
		{
			NavToNextTool();
		}
	}
	else if(pTool == m_pInputVectorTool)
	{
		if(m_pInputVectorTool->IsInputDone())
		{
			NavToNextTool();
		}
	}
	else if(pTool == m_pInputDoubleTool)
	{
		if(pTool->IsInputDone())
		{
			if(m_nDraftMode == 0)
			{
				m_bReady = true;
				PromptMessage("点击右键应用或取消:");
			}
			else
			{
				if(m_nDraftMode == 1)
					m_pPickStopEntityTool->SetOption(KCT_ENT_FACE,1);
				else
					m_pPickStopEntityTool->SetOption(KCT_ENT_SHELL | KCT_ENT_SOLID,1);

				AddInputTool(m_pPickStopEntityTool);
				NavToNextTool();
			}
		}
	}
	else if(pTool == m_pPickStopEntityTool)
	{
		if(pTool->IsInputDone())
		{
			m_bReady = true;
		}
	}

	return KSTA_CONTINUE;
}

int		kcmSolidDraft::OnExecute()
{
	m_bReady = false;

	return KSTA_CONTINUE;
}

int		kcmSolidDraft::OnEnd(int nCode)
{
	return nCode;
}

int		kcmSolidDraft::OnApply()
{
	int ista = KSTA_CONTINUE;
	if(DoDraft())
	{
		Done();
		ista = KSTA_DONE;
	}
	else
	{
		EndCommand(KSTA_FAILED);
		ista = KSTA_FAILED;
	}

	return ista;
}

BOOL	kcmSolidDraft::DoDraft()
{
	if(m_pPickEntityTool->GetSelCount() != 1)
		return FALSE;
	
	kVector3 vdir;
	if(!m_pInputVectorTool->GetVector(vdir) || vdir.length() < 0.001)
		return FALSE;

	kiSelEntity se = m_pPickEntityTool->GetFirstSelect();
	TopoDS_Shape aShape = se._pEntity->GetShape();
	
	TopoDS_Shape aNS;
	try{
		BRepOffsetAPI_MakeDraft mdraft(aShape,gp_Dir(vdir[0],vdir[1],vdir[2]),m_dAngle);
		if(m_nTransitionMode == 0)
			mdraft.SetOptions(BRepBuilderAPI_RightCorner);
		else
			mdraft.SetOptions(BRepBuilderAPI_RoundCorner);
		if(m_bInternal)
			mdraft.SetDraft(Standard_True);
		else
			mdraft.SetDraft(Standard_False);

		if(m_nDraftMode == 0)
		{
			mdraft.Perform(vdir.length());
		}
		if(mdraft.IsDone())
			aNS = mdraft.Shape();
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

// 创建必要的输入工具
BOOL	kcmSolidDraft::CreateInputTools()
{
	//创建选项
	m_pDraftMode = new kiOptionEnum("拉伸方式",'M',m_nDraftMode);
	m_pDraftMode->AddEnumItem("长度拉伸",'L');
	m_pDraftMode->AddEnumItem("拉伸到面",'F');
	m_pDraftMode->AddEnumItem("拉伸到体",'S');

	m_pTransitionMode = new kiOptionEnum("转换模式",'T',m_nTransitionMode);
	m_pTransitionMode->AddEnumItem("RightCorner",'R');
	m_pTransitionMode->AddEnumItem("RoundCorner",'O');

	m_optionSet.AddBoolOption("向内",'I',m_bInternal);

	m_optionSet.AddOption(m_pDraftMode);
	m_optionSet.AddOption(m_pTransitionMode);

	m_pPickEntityTool = new kiInputEntityTool(this,"选择线、面或壳",&m_optionSet);
	m_pPickEntityTool->SetOption(KCT_ENT_WIRE | KCT_ENT_FACE | KCT_ENT_SHELL,1);

	m_pInputVectorTool = new kiInputVector(this,"输入拉伸方向",&m_optionSet);
	m_pInputVectorTool->SetType(eInputZVector);

	m_pInputDoubleTool = new kiInputDoubleTool(this,"输入角度");
	m_pInputDoubleTool->Init(&m_dAngle);

	m_pPickStopEntityTool = new kiInputEntityTool(this,"拾取终止对象");

	return TRUE;
}

BOOL	kcmSolidDraft::DestroyInputTools()
{
	KC_SAFE_DELETE(m_pPickEntityTool);
	KC_SAFE_DELETE(m_pInputVectorTool);
	KC_SAFE_DELETE(m_pInputDoubleTool);
	KC_SAFE_DELETE(m_pPickStopEntityTool);

	return TRUE;
}

BOOL	kcmSolidDraft::InitInputToolQueue()
{
	AddInputTool(m_pPickEntityTool);
	AddInputTool(m_pInputVectorTool);
	AddInputTool(m_pInputDoubleTool);
	

	return TRUE;
}


