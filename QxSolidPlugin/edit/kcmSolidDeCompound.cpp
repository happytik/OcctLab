#include "StdAfx.h"
#include "kiInputEntityTool.h"
#include "kiInputVector.h"
#include "kcEntity.h"
#include "kcBasePlane.h"
#include "kcModel.h"
#include "kvCoordSystem.h"
#include "kcEntityFactory.h"
#include "QxBRepLib.h"
#include "kcmSolidDeCompound.h"

kcmSolidDeCompound::kcmSolidDeCompound(void)
{
	m_strName = "分解";
	m_strAlias = "DeCompound";
	m_strDesc = "分解";

	m_pInputEntity = NULL;
}

kcmSolidDeCompound::~kcmSolidDeCompound(void)
{
}

// 表明命令可以完成,主要和Apply函数结合使用.
BOOL	kcmSolidDeCompound::CanFinish()//命令是否可以完成
{
	return FALSE;
}

// 当输入工具获取结果,结束时,会回调对应命令的该函数.
// 用于继续执行命令或切换输入工具.也可能返回KSTA_DONE,表明命令执行结束.
//
int		kcmSolidDeCompound::OnInputFinished(kiInputTool *pTool)
{
	if(pTool == m_pInputEntity)
	{
		if(m_pInputEntity->IsInputDone())
		{

			if(DeCompound())
			{
				Done();
		
				m_pInputEntity->ClearSel();
				Redraw();
				return KSTA_DONE;
			}
			else
			{
				m_pInputEntity->ClearSel();
				return KSTA_ERROR;
			}
		}
		else
		{
			return KSTA_CANCEL;
		}
	}
	return kiCommand::OnInputFinished(pTool);
}

// 创建必要的输入工具
BOOL	kcmSolidDeCompound::CreateInputTools()
{
	m_pInputEntity = new kiInputEntityTool(this,"选择一个组合对象");
	m_pInputEntity->SetOption(KCT_ENT_SOLID | KCT_ENT_SHELL | KCT_ENT_COMPOUND,1);
	m_pInputEntity->SetNaturalMode(true);

	return TRUE;
}

BOOL	kcmSolidDeCompound::DestroyInputTools()
{
	KC_SAFE_DELETE(m_pInputEntity);

	return TRUE;
}

BOOL	kcmSolidDeCompound::InitInputToolQueue()
{
	AddInputTool(m_pInputEntity);
	return TRUE;
}


BOOL	kcmSolidDeCompound::DeCompound()
{
	kiSelSet *pSelSet = m_pInputEntity->GetSelSet();
	if(pSelSet->GetSelCount() != 1)
		return FALSE;

	pSelSet->InitSelected();
	kiSelEntity se = pSelSet->CurSelected();
	TopoDS_Shape aShape = se.SelectShape();
	if(aShape.IsNull())
		return FALSE;
	
	TopoDS_Iterator dsit;
	kcEntityFactory entfac;
	kcEntity *pEnt = NULL;
	std::vector<kcEntity *> aEnt;

	if(aShape.ShapeType() == TopAbs_COMPOUND){
		dsit.Initialize(aShape);
		for(;dsit.More();dsit.Next())
		{
			TopoDS_Shape aS = dsit.Value();
			//TopoDS_Shape aCS = QxBRepLib::CopyShape(aS);
			//
			pEnt = entfac.Create(aS);
			if(pEnt != NULL){
				aEnt.push_back(pEnt);
			}
		}
	}else if(aShape.ShapeType() == TopAbs_SHELL || aShape.ShapeType() == TopAbs_SOLID){
		TopExp_Explorer aExp;
		for(aExp.Init(aShape,TopAbs_FACE);aExp.More();aExp.Next()){
			const TopoDS_Shape &aF = aExp.Current();
			//
			kcEntity *pEnt = entfac.Create(aF);
			if(pEnt != NULL){
				aEnt.push_back(pEnt);
			}
		}
	}

	if(!aEnt.empty())
	{
		kcModel *pModel = GetModel();
		pModel->BeginUndo(GetName());
		{
			pModel->DelEntity(se._pEntity);
			int ix,ic = (int)aEnt.size();
			for(ix = 0;ix < ic;ix ++)
				pModel->AddEntity(aEnt[ix]);
		}
		pModel->EndUndo();
	}

	return TRUE;
}