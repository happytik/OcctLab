#include "StdAfx.h"
#include "kcEntity.h"
#include "kcModel.h"
#include "kiSelSet.h"
#include "kiInputEntityTool.h"
#include "kcmDelete.h"

kcmDelete::kcmDelete(void)
{
	m_strName = "Delete";
	m_strAlias = "Del";
	m_strDesc = "删除对象";

	m_pInputEntity = NULL;
}

kcmDelete::~kcmDelete(void)
{
}

// 表明命令可以完成,主要和Apply函数结合使用.
BOOL	kcmDelete::CanFinish()//命令是否可以完成
{
	return FALSE;
}

// 当输入工具获取结果,结束时,会回调对应命令的该函数.
// 用于继续执行命令或切换输入工具.也可能返回KSTA_DONE,表明命令执行结束.
//
int	   kcmDelete::OnInputFinished(kiInputTool *pTool)
{
	if(pTool == m_pInputEntity)
	{
		if(m_pInputEntity->GetSelCount() > 0)
		{
			kiSelSet *pSelSet = m_pInputEntity->GetSelSet();
			//
			DeleteEntity(pSelSet);
			//
			pSelSet->Clear();

			Done();
			Redraw();

			return KSTA_DONE;
		}
	}
	return KSTA_CONTINUE;
}

int  kcmDelete::OnDestroy()
{
	KC_SAFE_DELETE(m_pInputEntity);
	return KSTA_CONTINUE;
}

// 执行函数
int  kcmDelete::OnExecute()
{
	kiSelSet *pSelSet = GetGlobalSelSet();
	if(!pSelSet->IsEmpty()){

		pSelSet->UnHilight(FALSE);

		DeleteEntity(pSelSet);
		//
		pSelSet->Clear();

		return KSTA_DONE;
	}
	return KSTA_CONTINUE;
}

// 创建必要的输入工具,在Initialize中调用，每个命令仅仅调用一次。
BOOL	kcmDelete::CreateInputTools()
{
	m_pInputEntity = new kiInputEntityTool(this,"选择要删除对象:");
	if(!m_pInputEntity)
		return FALSE;

	m_pInputEntity->SetOption(KCT_ENT_ALL);
	m_pInputEntity->SetNaturalMode(true);

	return TRUE;
}

// 在begin中调用，主要是初始化工具队列。
BOOL	kcmDelete::InitInputToolQueue()
{
	if(m_pInputEntity)
		AddInputTool(m_pInputEntity);

	return TRUE;
}

//删除
BOOL	kcmDelete::DeleteEntity(kiSelSet *pSelSet)
{
	kcModel *pModel = GetModel();
	pModel->BeginUndo(GetName());
	pSelSet->InitSelected();
	while(pSelSet->MoreSelected())
	{
		kiSelEntity se = pSelSet->CurSelected();
		if(se._pEntity)
			pModel->DelEntity(se._pEntity,FALSE);

		pSelSet->NextSelected();
	}
	pModel->EndUndo();

	return TRUE;
}