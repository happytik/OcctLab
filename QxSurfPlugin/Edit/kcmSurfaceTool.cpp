#include "stdafx.h"
#include "kiInputEntityTool.h"
#include "kiInputVector.h"
#include "kcEntity.h"
#include "kcBasePlane.h"
#include "kcModel.h"
#include "QxBRepLib.h"
#include "kvCoordSystem.h"
#include "kcEntityFactory.h"
#include "kcmSurfaceTool.h"


kcmPrimarySurface::kcmPrimarySurface(void)
{
	m_strName = "原始面";
	m_strAlias = "PrimSurf";
	m_strDesc = "获取曲面原始面";

	m_pInputFaceTool = NULL;
}

kcmPrimarySurface::~kcmPrimarySurface(void)
{
}

// 表明命令可以完成,主要和Apply函数结合使用.
BOOL	kcmPrimarySurface::CanFinish()//命令是否可以完成
{
	return FALSE;
}

// 当输入工具获取结果,结束时,会回调对应命令的该函数.
// 用于继续执行命令或切换输入工具.也可能返回KSTA_DONE,表明命令执行结束.
//
int	kcmPrimarySurface::OnInputFinished(kiInputTool *pTool)
{
	if(pTool == m_pInputFaceTool)
	{
		if(m_pInputFaceTool->IsInputDone())
		{
			if(BuildPrimarySurface())
			{
				Done();

				return KSTA_DONE;
			}
			else
			{
				EndCommand(KSTA_CANCEL);
				return KSTA_CANCEL;
			}
		}
	}
	return kiCommand::OnInputFinished(pTool);
}

//
BOOL	kcmPrimarySurface::BuildPrimarySurface()
{
	if(m_pInputFaceTool->GetSelCount() != 1)
		return FALSE;

	TopoDS_Shape aShape = m_pInputFaceTool->SelectedShape(0);
	TopoDS_Face aFace = TopoDS::Face(aShape);
	Handle(Geom_Surface) aSurf = BRep_Tool::Surface(aFace);
	if(aSurf.IsNull())
		return FALSE;

	if(aSurf->IsKind(STANDARD_TYPE(Geom_RectangularTrimmedSurface))){
		Handle(Geom_RectangularTrimmedSurface) aTrmSurf = Handle(Geom_RectangularTrimmedSurface)::DownCast(aSurf);
		aSurf = aTrmSurf->BasisSurface();
	}

	TopoDS_Face aNF;
	try{
		BRepBuilderAPI_MakeFace mf(aSurf,1e-7);
		if(mf.IsDone())
			aNF = mf.Face();
	}catch(Standard_Failure){
		return FALSE;
	}

	kcEntityFactory efac;
	kcEntity *pEntity = efac.Create(aNF);
	if(pEntity)
	{
		kcModel *pModel = GetModel();
		pModel->BeginUndo(GetName());
		pModel->AddEntity(pEntity);
		pModel->EndUndo();

		return TRUE;
	}

	return FALSE;
}

int		kcmPrimarySurface::OnExecute()
{
	return KSTA_CONTINUE;
}

int		kcmPrimarySurface::OnEnd(int nCode)
{
	return nCode;
}

// 创建必要的输入工具
BOOL	kcmPrimarySurface::CreateInputTools()
{
	m_pInputFaceTool = new kiInputEntityTool(this,"选择一个面:");
	m_pInputFaceTool->SetOption(KCT_ENT_FACE,true);
	m_pInputFaceTool->SetNaturalMode(false);
		
	return TRUE;
}

BOOL	kcmPrimarySurface::DestroyInputTools()
{
	KC_SAFE_DELETE(m_pInputFaceTool);
	
	return TRUE;
}

BOOL	kcmPrimarySurface::InitInputToolQueue()
{
	AddInputTool(m_pInputFaceTool);

	return TRUE;
}

