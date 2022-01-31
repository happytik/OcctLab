#include "StdAfx.h"
#include "kiInputEntityTool.h"
#include "kiSelSet.h"
#include "kcEntity.h"
#include "kcmDispColor.h"

kcmDispColor::kcmDispColor(void)
{
	m_strName = "Set Color";
	m_strAlias = "SetCol";
	m_strDesc = "设置对象颜色";

	m_pInputEntityTool = NULL;
}

kcmDispColor::~kcmDispColor(void)
{
}

// 表明命令可以完成,主要和Apply函数结合使用.
BOOL	kcmDispColor::CanFinish()//命令是否可以完成
{
	return FALSE;
}

// 当输入工具获取结果,结束时,会回调对应命令的该函数.
// 用于继续执行命令或切换输入工具.也可能返回KSTA_DONE,表明命令执行结束.
//
int		kcmDispColor::OnInputFinished(kiInputTool *pTool)
{
	if(pTool == m_pInputEntityTool)
	{
		if(m_pInputEntityTool->IsInputDone())
		{
			kiSelSet *pSelSet = m_pInputEntityTool->GetSelSet();
			if(SetColor(pSelSet))
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

int		kcmDispColor::OnExecute()
{
	if(!IsGlobalSelSetEmpty())
	{
		kiSelSet *pSelSet = GetGlobalSelSet();
		if(SetColor(pSelSet))
		{
			ClearGlobalSelSet();

			Done();
			Redraw();

			return KSTA_DONE;
		}
	}
	return KSTA_CONTINUE;
}

int		kcmDispColor::OnEnd(int nCode)
{
	return nCode;
}

// 创建必要的输入工具
BOOL	kcmDispColor::CreateInputTools()
{
	m_pInputEntityTool = new kiInputEntityTool(this,"选择一个或多个对象:");
	m_pInputEntityTool->SetOption(KCT_ENT_ALL,false);
	m_pInputEntityTool->SetNaturalMode(true);

	return TRUE;
}

BOOL	kcmDispColor::DestroyInputTools()
{
	KC_SAFE_DELETE(m_pInputEntityTool);
	return TRUE;
}

BOOL	kcmDispColor::InitInputToolQueue()
{
	AddInputTool(m_pInputEntityTool);
	return TRUE;
}

BOOL	kcmDispColor::SetColor(kiSelSet *pSelSet)
{
	if(pSelSet->GetSelCount() <= 0)
		return FALSE;

	CColorDialog dlg;
	if(dlg.DoModal() == IDOK)
	{
		COLORREF col = dlg.GetColor();
		double r = GetRValue(col) / 256.0,g = GetGValue(col) / 256.0,b = GetBValue(col) / 256.0;
		//设置颜色
		pSelSet->InitSelected();
		while(pSelSet->MoreSelected())
		{
			kiSelEntity se = pSelSet->CurSelected();
			se._pEntity->SetColor(r,g,b,FALSE);

			pSelSet->NextSelected();
		}
	}

	return TRUE;
}