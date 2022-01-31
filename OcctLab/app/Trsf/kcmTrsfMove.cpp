#include "StdAfx.h"
#include "kcModel.h"
#include "kiInputPoint.h"
#include "kiInputVector.h"
#include "kiInputDirTool.h"
#include "kiInputEntityTool.h"
#include "kiInputValueTool.h"
#include "kcEntity.h"
#include "kiSelSet.h"
#include "kcEntityTrsf.h"
#include "kcDispObj.h"
#include "kcmTrsfMove.h"

#define KC_STA_SEL_ENTITY		1
#define KC_STA_GET_VECTOR		2

kcmTrsfMove::kcmTrsfMove(void)
{
	m_strName = "Move";
	m_strAlias = "Mov";
	m_strDesc = "Move Entity";

	m_nState = KC_STA_SEL_ENTITY;
	m_pSelEntity = NULL;
	m_pInputDir = NULL;
	m_pInputDistTool = NULL;
	//m_pFrmShape = NULL;
	m_bCopy = true;
	m_aDir.set(0.0,0.0,1.0);
	m_dDist = 10.0;
}

kcmTrsfMove::~kcmTrsfMove(void)
{
}

// 表明命令可以完成,主要和Apply函数结合使用.
BOOL	kcmTrsfMove::CanFinish()//命令是否可以完成
{
	return FALSE;
}

// 当输入工具获取结果,结束时,会回调对应命令的该函数.
// 用于继续执行命令或切换输入工具.也可能返回KSTA_DONE,表明命令执行结束.
//
int	kcmTrsfMove::OnInputFinished(kiInputTool *pTool)
{
	if(pTool == m_pSelEntity)
	{
		if(m_pSelEntity->IsInputDone())
		{
			//kiSelSet *pSelSet = m_pSelEntity->GetSelSet();
			//kiSelEntity selEnt;
			//pSelSet->InitSelected();
			//while(pSelSet->MoreSelected())
			//{
			//	selEnt = pSelSet->CurSelected();
			//	TopoDS_Shape aShape = selEnt._pEntity->GetShape();
			//	if(!aShape.IsNull())
			//	{
			//		m_pFrmShape->AddShape(aShape);
			//	}

			//	pSelSet->NextSelected();
			//}

			NavToNextTool();
			//m_nState = KC_STA_GET_VECTOR;
		}
		else
		{
			return KSTA_CANCEL;
		}
	}
	else if(pTool == m_pInputDir){
		if(pTool->IsInputDone()){
			NavToNextTool();
		}else{
			return KSTA_CANCEL;
		}
	}else if(pTool == m_pInputDistTool){
		m_aDir.normalize();
		m_aDir *= m_dDist;
		if(DoMove()){
			Done();
			Redraw();

			return KSTA_DONE;
		}
	}


	return kiCommand::OnInputFinished(pTool);
}

//
BOOL	kcmTrsfMove::DoMove()
{
	kVector3 vec;
	vec.set(m_aDir[0],m_aDir[1],m_aDir[2]);

	kcEntityTrsf entrsf;
	std::vector<kcEntity *> aEnt,aNewEnt;
	kcEntity *pEntity = NULL;
	kiSelSet *pSelSet = m_pSelEntity->GetSelSet();
	kiSelEntity selEnt;
	pSelSet->InitSelected();
	while(pSelSet->MoreSelected()){
		selEnt = pSelSet->CurSelected();
		//
		if(!m_bCopy)
			aEnt.push_back(selEnt._pEntity);
		//
		pEntity = entrsf.Move(selEnt._pEntity,vec);
		if(pEntity)
			aNewEnt.push_back(pEntity);

		pSelSet->NextSelected();
	}

	if(aNewEnt.empty())
		return FALSE;

	kcModel *pModel = GetModel();
	pModel->BeginUndo(GetName());
	{
		int ix,isize;
		if(!m_bCopy){
			isize = (int)aEnt.size();
			for(ix = 0;ix < isize;ix ++)
				pModel->DelEntity(aEnt[ix]);
		}
		pModel->AddEntity(aNewEnt);
	}
	pModel->EndUndo();

	return TRUE;
}

int	kcmTrsfMove::OnExecute()
{
	m_nState = KC_STA_SEL_ENTITY;

	//m_pFrmShape = new kcDispFrmShape(m_pModel->GetAISContext());

	return KSTA_CONTINUE;
}

int		kcmTrsfMove::OnEnd(int nCode)
{
	//KC_SAFE_DELETE(m_pFrmShape);

	return nCode;
}

int		kcmTrsfMove::OnMouseMove(kuiMouseInput& mouseInput)
{
	kiCommand::OnMouseMove(mouseInput);

	//if(m_nState == KC_STA_GET_VECTOR)
	//{
	//	kVector3 vec;
	//	if(m_pInputVector->GetVector(vec))
	//	{
	//		if(m_pFrmShape)
	//		{
	//			m_pFrmShape->Show();
	//			m_pFrmShape->Move(vec[0],vec[1],vec[2]);
	//		}
	//	}
	//}

	return KSTA_CONTINUE;
}

// 创建必要的输入工具,在Initialize中调用，每个命令仅仅调用一次。
BOOL	kcmTrsfMove::CreateInputTools()
{
	m_optionSet.AddBoolOption("拷贝",'c',m_bCopy);
	m_pSelEntity = new kiInputEntityTool(this,"选择要移动对象",&m_optionSet);
	m_pSelEntity->SetOption(KCT_ENT_ALL);

	m_pInputDir = new kiInputDirTool(this,"输入移动方向",NULL);
	m_pInputDir->Initialize(&m_aDir,false,false,true);

	m_pInputDistTool = new kiInputDoubleTool(this,"移动距离",NULL);
	m_pInputDistTool->Init(&m_dDist,0.0,1000.0);

	return TRUE;
}

// 销毁创建的输入工具.每个命令仅调用一次.
BOOL	kcmTrsfMove::DestroyInputTools()
{
	KC_SAFE_DELETE(m_pSelEntity);
	KC_SAFE_DELETE(m_pInputDir);
	KC_SAFE_DELETE(m_pInputDistTool);

	return TRUE;
}

// 在begin中调用，主要是初始化工具队列。
BOOL	kcmTrsfMove::InitInputToolQueue()
{
	AddInputTool(m_pSelEntity);
	AddInputTool(m_pInputDir);
	AddInputTool(m_pInputDistTool);

	return TRUE;
}