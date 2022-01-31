#include "StdAfx.h"
#include <ShapeFix_Shape.hxx>
#include "kcEntity.h"
#include "kiInputEntityTool.h"
#include "kcModel.h"
#include "kcEntityFactory.h"
#include "kcmShapeFix.h"

kcmShapeFixBase::kcmShapeFixBase(void)
{
	m_pInputEntityTool = NULL;
	m_nEntType = 0;
}

kcmShapeFixBase::~kcmShapeFixBase(void)
{
}

// 表明命令可以完成,主要和Apply函数结合使用.
BOOL	kcmShapeFixBase::CanFinish()//命令是否可以完成
{
	return FALSE;
}

// 当输入工具获取结果,结束时,会回调对应命令的该函数.
// 用于继续执行命令或切换输入工具.也可能返回KSTA_DONE,表明命令执行结束.
//
int	kcmShapeFixBase::OnInputFinished(kiInputTool *pTool)
{
	if(pTool == m_pInputEntityTool)
	{
		if(DoFix()){
			Done();
			return KSTA_DONE;
		}else{
			EndCommand(KSTA_CANCEL);
			return KSTA_CANCEL;
		}
	}
	return KSTA_CONTINUE;
}

int		kcmShapeFixBase::OnExecute()
{
	if(!IsGlobalSelSetEmpty()){
		if(DoFix())
		{
			ClearGlobalSelSet();

			Done();
			Redraw();

			return KSTA_DONE;
		}
	}
	return KSTA_CONTINUE;
}

int		kcmShapeFixBase::OnEnd(int nCode)
{
	return nCode;
}

// 创建必要的输入工具
BOOL	kcmShapeFixBase::CreateInputTools()
{
	m_pInputEntityTool = new kiInputEntityTool(this,"选择要修正的对象");
	m_pInputEntityTool->SetOption(m_nEntType,1);
	m_pInputEntityTool->SetNaturalMode(true);

	return TRUE;
}

BOOL	kcmShapeFixBase::DestroyInputTools()
{
	KC_SAFE_DELETE(m_pInputEntityTool);
	return TRUE;
}
BOOL	kcmShapeFixBase::InitInputToolQueue()
{
	AddInputTool(m_pInputEntityTool);
	return TRUE;
}

BOOL	kcmShapeFixBase::DoFix()
{
	TopoDS_Shape aShape;
	kiSelEntity se;

	if(m_pInputEntityTool->IsInputDone()){
		if(m_pInputEntityTool->GetSelCount() != 1)
			return FALSE;
		se = m_pInputEntityTool->GetFirstSelect();
		aShape = se.EntityShape();
	}else{
		//判断是否全局选择集有对象
		kiSelSet *pSelSet = GetGlobalSelSet();
		if(pSelSet->GetSelCount() > 0){
			pSelSet->InitSelected();
			se = pSelSet->CurSelected();
			aShape = se.EntityShape();
		}
	}
	if(aShape.IsNull())
		return FALSE;

	TopoDS_Shape aNS = DoFixShape(aShape);
	if(aNS.IsNull())
		return FALSE;

	kcEntityFactory entfac;
	kcEntity *pEnt = entfac.Create(aNS);
	if(pEnt)
	{
		kcModel *pModel = GetModel();
		pModel->BeginUndo(GetName());
		pModel->DelEntity(se.Entity());
		pModel->AddEntity(pEnt);
		pModel->EndUndo();

		return TRUE;
	}

	return FALSE;
}

//////////////////////////////////////////////////////////////////////////
//

kcmShapeFix::kcmShapeFix(void)
{
	m_strName = "整体修正";
	m_strAlias = "FixShape";
	m_strDesc = "修正整个对象";

	m_nEntType = KCT_ENT_ALL;
}

kcmShapeFix::~kcmShapeFix(void)
{
}

TopoDS_Shape	kcmShapeFix::DoFixShape(const TopoDS_Shape& aS)
{
	TopoDS_Shape aNS;
	if(aS.IsNull()) return aNS;

	try{
		ShapeFix_Shape fs(aS);
		fs.Perform();
		aNS = fs.Shape();
	}catch(Standard_Failure){}

	return aNS;
}
