#include "StdAfx.h"
#include "kcEntity.h"
#include "QxAnimationShape.h"
#include "kiInputEntityTool.h"
#include "kcmAnimationDemo.h"

//////////////////////////////////////////////////////////////////////////
//
kcmAnimationDemo::kcmAnimationDemo(void)
{
	m_strName = "AnimationDemo";
	m_strAlias = "AnimDemo";
	m_strDesc = "动画演示";

	m_pAnimationShape = NULL;
	m_pInputEntityTool = NULL;
	m_nTimer = 0;
	m_dAngle = 0.0;
}

kcmAnimationDemo::~kcmAnimationDemo(void)
{
}

// 表明命令可以完成,主要和Apply函数结合使用.
BOOL	kcmAnimationDemo::CanFinish()//命令是否可以完成
{
	return FALSE;
}

// 当输入工具获取结果,结束时,会回调对应命令的该函数.
// 用于继续执行命令或切换输入工具.也可能返回KSTA_DONE,表明命令执行结束.
//
int	kcmAnimationDemo::OnInputFinished(kiInputTool *pTool)
{
	if(pTool == m_pInputEntityTool)
	{
		if(m_pInputEntityTool->IsInputDone())
		{
			kiSelSet *pSelSet = m_pInputEntityTool->GetSelSet();
			pSelSet->InitSelected();
			if(pSelSet->MoreSelected())
			{
				kiSelEntity se = pSelSet->CurSelected();
				Handle(AIS_Shape) aiShape = Handle(AIS_Shape)::DownCast(se.AISObject());
				if(!aiShape.IsNull())
				{
					m_pAnimationShape = new QxAnimationShape(
						GetAISContext(),aiShape->Shape());
					m_pAnimationShape->Reset(FALSE);
					m_pAnimationShape->Display(TRUE);

					//启动定时器
					this->SetTimer(1,500);
				}
				else
				{
					Done();
					return KSTA_DONE;
				}
			}
			else
			{
				Done();

				return KSTA_DONE;
			}
		}
	}

	return KSTA_CONTINUE;
}

int	kcmAnimationDemo::OnExecute()
{
	m_dAngle = 0.0;
	return KSTA_CONTINUE;
}

int		kcmAnimationDemo::OnEnd(int nCode)
{
	return nCode;
}

// 定时器处理函数
int kcmAnimationDemo::OnTimer(int nId,DWORD dwTime)
{
	if(nId == 1){
		m_dAngle += 0.2;
		KTRACE("\n angle is %f.",m_dAngle);
		if(m_dAngle > K_PI * 2.0){
			m_pAnimationShape->Hide(TRUE);
			KillTimer(nId);
			m_nTimer = 0;

			Done();
		}else{
			gp_Trsf trsf;
			trsf.SetRotation(gp_Ax1(),m_dAngle);
			m_pAnimationShape->SetTrsf(trsf,TRUE);
		}
	}

	return 1;
}

// 创建必要的输入工具
BOOL	kcmAnimationDemo::CreateInputTools()
{
	m_pInputEntityTool = new kiInputEntityTool(this,"选择一个对象:");
	m_pInputEntityTool->SetOption(KCT_ENT_ALL,1);
	m_pInputEntityTool->SetNaturalMode(true);
	
	return TRUE;
}

BOOL	kcmAnimationDemo::DestroyInputTools()
{
	KC_SAFE_DELETE(m_pInputEntityTool);
	
	return TRUE;
}

BOOL	kcmAnimationDemo::InitInputToolQueue()
{
	AddInputTool(m_pInputEntityTool);
	return TRUE;
}
