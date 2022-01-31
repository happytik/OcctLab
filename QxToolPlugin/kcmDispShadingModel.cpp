#include "StdAfx.h"
#include "kiInputEntityTool.h"
#include "kiSelSet.h"
#include "kcEntity.h"
#include "kcmdispshadingmodel.h"

kcmDispShadingModel::kcmDispShadingModel(void)
{
	m_nModel = -1;
}

kcmDispShadingModel::~kcmDispShadingModel(void)
{
}

// 表明命令可以完成,主要和Apply函数结合使用.
BOOL	kcmDispShadingModel::CanFinish()//命令是否可以完成
{
	return FALSE;
}

// 当输入工具获取结果,结束时,会回调对应命令的该函数.
// 用于继续执行命令或切换输入工具.也可能返回KSTA_DONE,表明命令执行结束.
//
int	kcmDispShadingModel::OnInputFinished(kiInputTool *pTool)
{
	if(pTool == m_pInputEntityTool)
	{
		if(m_pInputEntityTool->IsInputDone())
		{
			kiSelSet *pSelSet = m_pInputEntityTool->GetSelSet();
			if(SetModel(pSelSet))
			{
				m_pInputEntityTool->ClearSel();

				Done();
				Redraw();

				return KSTA_DONE;
			}
		}
		else
		{
			return KSTA_CANCEL;
		}
	}
	return KSTA_CONTINUE;
}


int		kcmDispShadingModel::OnExecute()
{
	if(!IsGlobalSelSetEmpty())
	{
		kiSelSet *pSelSet = GetGlobalSelSet();
		if(SetModel(pSelSet))
		{
			ClearGlobalSelSet();

			Done();
			Redraw();

			return KSTA_DONE;
		}
	}
	return KSTA_CONTINUE;
}

int		kcmDispShadingModel::OnEnd(int nCode)
{
	return nCode;
}

// 创建必要的输入工具
BOOL	kcmDispShadingModel::CreateInputTools()
{
	m_pInputEntityTool = new kiInputEntityTool(this,"选择一个或多个对象:");
	m_pInputEntityTool->SetOption(KCT_ENT_ALL);

	return TRUE;
}

BOOL	kcmDispShadingModel::DestroyInputTools()
{
	KC_SAFE_DELETE(m_pInputEntityTool);
	return TRUE;
}

BOOL	kcmDispShadingModel::InitInputToolQueue()
{
	AddInputTool(m_pInputEntityTool);
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
// 框架显示
kcmDispWireFrame::kcmDispWireFrame()
{
	m_strName = "WireFrame";
	m_strAlias = "WF disp";
	m_strDesc = "框架显示";

	m_nModel = 0;
}

kcmDispWireFrame::~kcmDispWireFrame()
{

}

BOOL	kcmDispWireFrame::SetModel(kiSelSet *pSelSet)
{
	pSelSet->InitSelected();
	while(pSelSet->MoreSelected())
	{
		kiSelEntity se = pSelSet->CurSelected();
		se._pEntity->SetDisplayModel(m_nModel,FALSE);

		pSelSet->NextSelected();
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
// 渲染显示
kcmDispShading::kcmDispShading()
{
	m_strName = "Shading Display";
	m_strAlias = "SD disp";
	m_strDesc = "渲染显示";

	m_nModel = 1;
}

kcmDispShading::~kcmDispShading()
{

}

BOOL	kcmDispShading::SetModel(kiSelSet *pSelSet)
{
	pSelSet->InitSelected();
	while(pSelSet->MoreSelected())
	{
		kiSelEntity se = pSelSet->CurSelected();
		se._pEntity->SetDisplayModel(m_nModel,FALSE);

		pSelSet->NextSelected();
	}

	return TRUE;
}
