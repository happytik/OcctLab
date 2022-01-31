#include "StdAfx.h"
#include <BRepAlgoAPI_Section.hxx>
#include <BRepFeat_SplitShape.hxx>
#include "kiInputEntityTool.h"
#include "kiInputVector.h"
#include "kiInputValueTool.h"
#include "kcEntity.h"
#include "kcBasePlane.h"
#include "kcModel.h"
#include "QxCurveLib.h"
#include "kvCoordSystem.h"
#include "kcEntityFactory.h"
#include "kcmSurfaceSplit.h"

kcmSurfaceSplit::kcmSurfaceSplit(void)
{
	m_strName = "曲面分割";
	m_strAlias = "SplitSurf";
	m_strDesc = "分割曲面";

	m_pPickSurfaceTool = NULL;
	m_pPickShapeTool = NULL;
}

kcmSurfaceSplit::~kcmSurfaceSplit(void)
{
}

// 表明命令可以完成,主要和Apply函数结合使用.
BOOL	kcmSurfaceSplit::CanFinish()//命令是否可以完成
{
	return FALSE;
}

// 当输入工具获取结果,结束时,会回调对应命令的该函数.
// 用于继续执行命令或切换输入工具.也可能返回KSTA_DONE,表明命令执行结束.
//
int	kcmSurfaceSplit::OnInputFinished(kiInputTool *pTool)
{
	if(pTool->IsInputDone())
	{
		if(pTool == m_pPickSurfaceTool)
		{
			NavToNextTool();
		}
		else if(pTool == m_pPickShapeTool)
		{
			if(DoSplit())
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
	}
	else
	{
		EndCommand(KSTA_CANCEL);
		return KSTA_CANCEL;
	}
	return KSTA_CONTINUE;
}

int		kcmSurfaceSplit::OnExecute()
{
	return KSTA_CONTINUE;
}

int		kcmSurfaceSplit::OnEnd(int nCode)
{
	return nCode;
}

// 创建必要的输入工具
BOOL	kcmSurfaceSplit::CreateInputTools()
{
	m_pPickSurfaceTool = new kiInputEntityTool(this,"选择被分割曲面");
	m_pPickSurfaceTool->SetOption(KCT_ENT_FACE,1);
	m_pPickSurfaceTool->SetNaturalMode(true);

	m_pPickShapeTool = new kiInputEntityTool(this,"选择用于分割的对象");
	m_pPickShapeTool->SetOption(KCT_ENT_FACE | KCT_ENT_SHELL | KCT_ENT_SOLID,1);
	m_pPickShapeTool->SetNaturalMode(true);

	return TRUE;
}

BOOL	kcmSurfaceSplit::DestroyInputTools()
{
	KC_SAFE_DELETE(m_pPickSurfaceTool);
	KC_SAFE_DELETE(m_pPickShapeTool);

	return TRUE;
}

BOOL	kcmSurfaceSplit::InitInputToolQueue()
{
	AddInputTool(m_pPickSurfaceTool);
	AddInputTool(m_pPickShapeTool);

	return TRUE;
}

//
BOOL	kcmSurfaceSplit::DoSplit()
{
	if(m_pPickSurfaceTool->GetSelCount() != 1 ||
	   m_pPickShapeTool->GetSelCount() != 1)
	   return FALSE;

	kiSelEntity se = m_pPickSurfaceTool->GetFirstSelect();
	kcEntity *pFaceEnt = se._pEntity;
	TopoDS_Shape aShape = se.SelectShape();//se._pEntity->GetShape();
	
	se = m_pPickShapeTool->GetFirstSelect();
	TopoDS_Shape aShape2 = se.SelectShape() ;//._pEntity->GetShape();

	//首先求交线
	std::vector<kcEntity *> aEnt;
	try{
		BRepAlgoAPI_Section sec(aShape,aShape2,Standard_False);
		sec.ComputePCurveOn1(Standard_True);
		sec.Approximation(Standard_True);
		sec.Build();

		TopoDS_Shape aIntEdges = sec.Shape();
		if(aIntEdges.IsNull())
		{
			AfxMessageBox("没有交线.");
			return FALSE;
		}

		//分割曲面
		TopoDS_Face aFace = TopoDS::Face(aShape);
		BRepFeat_SplitShape ssplit(aFace);

		TopExp_Explorer ex;
		for(ex.Init(aIntEdges,TopAbs_EDGE);ex.More();ex.Next())
		{
			TopoDS_Edge aE = TopoDS::Edge(ex.Current());
			TopoDS_Shape aF;
			if(sec.HasAncestorFaceOn1(aE,aF))
			{
				TopoDS_Face aAncFace = TopoDS::Face(aF);
				if(aAncFace.IsSame(aFace))
					ssplit.Add(aE,aFace);
			}
		}

		ssplit.Build();

		//生成新对象
		kcEntity *pEnt = NULL;
		kcEntityFactory efac;
		const TopTools_ListOfShape& shapeList = ssplit.Modified(aFace);
		TopTools_ListIteratorOfListOfShape ite(shapeList);
		for(;ite.More();ite.Next())
		{
			TopoDS_Shape aS = ite.Value();
			pEnt = efac.Create(aS);
			if(pEnt)
				aEnt.push_back(pEnt);
		}
	}catch(Standard_Failure){
		return FALSE;
	}

	if(aEnt.empty())
		return FALSE;

	//显示
	kcModel *pModel = GetModel();
	pModel->BeginUndo(GetName());
	{
		//隐藏旧的
		pModel->DelEntity(pFaceEnt);
		pModel->AddEntity(aEnt);
	}
	pModel->EndUndo();

	return TRUE;
}
