#include "StdAfx.h"
#include "kiInputEntityTool.h"
#include "kuiInterface.h"
#include "kiSelSet.h"
#include "kcEntity.h"
#include "kcModel.h"
#include "kvGrid.h"
#include "kcBasePlane.h"
#include "FilterDispDialog.h"
#include "kcmDispVisible.h"

kcmDispHideSelect::kcmDispHideSelect(void)
{
	m_strName = "隐藏选择对象";
	m_strAlias = "HideSel";
	m_strDesc = "隐藏选择对象";

	m_pInputEntity = NULL;
}

kcmDispHideSelect::~kcmDispHideSelect(void)
{

}

// 表明命令可以完成,主要和Apply函数结合使用.
BOOL	kcmDispHideSelect::CanFinish()//命令是否可以完成
{
	return FALSE;
}

// 当输入工具获取结果,结束时,会回调对应命令的该函数.
// 用于继续执行命令或切换输入工具.也可能返回KSTA_DONE,表明命令执行结束.
//
int	kcmDispHideSelect::OnInputFinished(kiInputTool *pTool)
{
	if(pTool == m_pInputEntity)
	{
		if(m_pInputEntity->IsInputDone())
		{
			kiSelSet *pSelSet = m_pInputEntity->GetSelSet();
			if(HideSelSet(pSelSet))
			{
				Done();
				Redraw();

				return KSTA_DONE;
			}
		}
	}
	return kiCommand::OnInputFinished(pTool);
}

int		kcmDispHideSelect::OnExecute()
{
	if(!IsGlobalSelSetEmpty())
	{
		kiSelSet *pSelSet = GetGlobalSelSet();
		if(HideSelSet(pSelSet))
		{
			ClearGlobalSelSet();

			Done();
			Redraw();

			return KSTA_DONE;
		}
	}
	return KSTA_CONTINUE;
}

BOOL	kcmDispHideSelect::HideSelSet(kiSelSet *pSelSet)
{
	kcModel *pModel = GetModel();
	kuiModelTreeInterface *pIModelTree = pModel->GetModelTreeInterface();
	pSelSet->InitSelected();
	while(pSelSet->MoreSelected())
	{
		kiSelEntity se = pSelSet->CurSelected();
		se._pEntity->Display(false,FALSE);
		//
		if(pIModelTree){
			pIModelTree->ShowEntity(se._pEntity->GetName(),FALSE);
		}

		pSelSet->NextSelected();
	}

	return TRUE;
}

int		kcmDispHideSelect::OnEnd(int nCode)
{
	return nCode;
}

// 创建必要的输入工具
BOOL	kcmDispHideSelect::CreateInputTools()
{
	m_pInputEntity = new kiInputEntityTool(this,"选择要隐藏对象");
	m_pInputEntity->SetOption(KCT_ENT_ALL);

	return TRUE;
}

BOOL	kcmDispHideSelect::DestroyInputTools()
{
	KC_SAFE_DELETE(m_pInputEntity);
	return TRUE;
}

BOOL	kcmDispHideSelect::InitInputToolQueue()
{
	AddInputTool(m_pInputEntity);
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
//
kcmDispShowAll::kcmDispShowAll()
{
	m_strName = "显示隐藏对象";
	m_strAlias = "ShowAll";
	m_strDesc = "显示所有隐藏对象";
}

kcmDispShowAll::~kcmDispShowAll()
{

}

// 表明命令可以完成,主要和Apply函数结合使用.
BOOL	kcmDispShowAll::CanFinish()//命令是否可以完成
{
	return FALSE;
}

int	kcmDispShowAll::OnExecute()
{
	GetModel()->ShowAll(FALSE);
	Redraw();
	Done();

	return KSTA_DONE;
}

//////////////////////////////////////////////////////////////////////////
// 过滤显示
kcmDispTypeFilter::kcmDispTypeFilter()
{
	m_strName = "类型过滤显示";
	m_strAlias = "TypeFilterDisplay";
	m_strDesc = "过滤显示";
}

kcmDispTypeFilter::~kcmDispTypeFilter()
{
}

// 表明命令可以完成,主要和Apply函数结合使用.
BOOL	kcmDispTypeFilter::CanFinish()//命令是否可以完成
{
	return FALSE;
}

int		kcmDispTypeFilter::OnExecute()
{
	CFilterDispDialog dlg;
	if(IDOK == dlg.DoModal())
	{
		int nFlag = 0;
		if(dlg.m_bDispPoint)	nFlag |= KCT_ENT_POINT;
		if(dlg.m_bDispCurve)	nFlag |= KCT_ENT_EDGE;
		if(dlg.m_bDispWire)		nFlag |= KCT_ENT_WIRE;
		if(dlg.m_bDispSurface)	nFlag |= KCT_ENT_FACE;
		if(dlg.m_bDispShell)	nFlag |= KCT_ENT_SHELL;
		if(dlg.m_bDispSolid)	nFlag |= KCT_ENT_SOLID;
		if(dlg.m_bDispCompound)	nFlag |= KCT_ENT_COMPOUND;

		GetModel()->FilterDisplay(nFlag,FALSE);
		Redraw();
	}

	Done();
	return KSTA_DONE;
}

///////////////////////////////////////////////////////////////////////////////
//
kcmHideGridDisplay::kcmHideGridDisplay()
{
	InitSet("隐藏网格显示","HideGrid","隐藏当前工作平面的网格显示");
}

kcmHideGridDisplay::~kcmHideGridDisplay()
{
}

// 表明命令可以完成,主要和Apply函数结合使用.
BOOL kcmHideGridDisplay::CanFinish()//命令是否可以完成
{
	return FALSE;
}

int kcmHideGridDisplay::OnExecute()
{
	kcModel *pModel = GetModel();
	if(pModel != NULL){
		kcBasePlane *pBasePlane = GetCurrentBasePlane();
		if(pBasePlane != NULL){
			kvGrid *pGrid = pBasePlane->GetGrid();
			if(pGrid != NULL && pGrid->IsVisible()){
				pGrid->SetVisible(false);
				//
				Redraw();
			}
		}else{
			ASSERT(FALSE);
		}
	}
	return KSTA_DONE;
}


