#include "StdAfx.h"
#include <GeomFill.hxx>
#include "kiInputEntityTool.h"
#include "kiInputVector.h"
#include "kcEntity.h"
#include "kcBasePlane.h"
#include "kcModel.h"
#include "QxBRepLib.h"
#include "kvCoordSystem.h"
#include "kcEntityFactory.h"
#include "kcmRuledSurface.h"

kcmRuledSurface::kcmRuledSurface(void)
{
	m_strName = "直纹面";
	m_strAlias = "RuledSurf";
	m_strDesc = "直纹面";

	m_pInputCurve1 = NULL;
	m_pInputCurve2 = NULL;
}

kcmRuledSurface::~kcmRuledSurface(void)
{
}

// 表明命令可以完成,主要和Apply函数结合使用.
BOOL	kcmRuledSurface::CanFinish()//命令是否可以完成
{
	return FALSE;
}

// 当输入工具获取结果,结束时,会回调对应命令的该函数.
// 用于继续执行命令或切换输入工具.也可能返回KSTA_DONE,表明命令执行结束.
//
int	kcmRuledSurface::OnInputFinished(kiInputTool *pTool)
{
	if(pTool == m_pInputCurve1)
	{
		if(m_pInputCurve1->IsInputDone())
		{
			NavToNextTool();
		}
	}
	else if(pTool == m_pInputCurve2)
	{
		if(m_pInputCurve2->IsInputDone())
		{
			if(BuildRuledSurface())
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
BOOL	kcmRuledSurface::BuildRuledSurface()
{
	if(m_pInputCurve1->GetSelCount() != 1 ||
	   m_pInputCurve2->GetSelCount() != 1)
	   return FALSE;

	kiSelEntity se1 = m_pInputCurve1->GetFirstSelect();
	kiSelEntity se2 = m_pInputCurve2->GetFirstSelect();
	TopoDS_Shape aS1 = se1.SelectShape();
	TopoDS_Shape aS2 = se2.SelectShape();

	// 通常Edge是曲线上的一部分，因此，需要获取对应的一段曲线
	// 而不能直接使用曲线生成直纹面
	kcEntityFactory efac;
	kcEntity *pEntity = NULL;
	try{
		double df1,dl1,df2,dl2;
		Handle(Geom_Curve) aCrv,aCrv1,aCrv2,aCCrv;
		//第一条曲线
		aCrv = BRep_Tool::Curve(TopoDS::Edge(aS1),df1,dl1);
		if(!aCrv.IsNull()){
			aCCrv = Handle(Geom_Curve)::DownCast(aCrv->Copy());
			if(!aCCrv.IsNull()){
				if(!aCrv->IsKind(STANDARD_TYPE(Geom_TrimmedCurve))){			
					aCrv1 = new Geom_TrimmedCurve(aCCrv,df1,dl1);
				}else{
					aCrv1 = aCCrv;
				}
			}
		}
		aCrv = BRep_Tool::Curve(TopoDS::Edge(aS2),df2,dl2);
		if(!aCrv.IsNull()){
			aCCrv = Handle(Geom_Curve)::DownCast(aCrv->Copy());
			if(!aCCrv.IsNull()){
				if(!aCrv->IsKind(STANDARD_TYPE(Geom_TrimmedCurve))){			
					aCrv2 = new Geom_TrimmedCurve(aCCrv,df2,dl2);
				}else{
					aCrv2 = aCCrv;
				}
			}
		}
		//生成直纹面
		if(!aCrv1.IsNull() && !aCrv2.IsNull()){
			//这里使用该函数
			Handle(Geom_Surface) aSurf = GeomFill::Surface(aCrv1,aCrv2);
			if(!aSurf.IsNull()){
				BRepBuilderAPI_MakeFace mf(aSurf,Precision::Confusion());
				if(mf.IsDone()){
					TopoDS_Face aF = mf.Face();
					pEntity = efac.Create(aF);
				}
			}
		}
	}catch(Standard_Failure){
		return FALSE;
	}

	if(pEntity){
		kcModel *pModel = GetModel();
		pModel->BeginUndo(GetName());
		pModel->AddEntity(pEntity);
		pModel->EndUndo();

		return TRUE;
	}

	return FALSE;
}

int		kcmRuledSurface::OnExecute()
{
	return KSTA_CONTINUE;
}

int		kcmRuledSurface::OnEnd(int nCode)
{
	return nCode;
}

// 创建必要的输入工具
BOOL	kcmRuledSurface::CreateInputTools()
{
	m_pInputCurve1 = new kiInputEntityTool(this,"选择第一条曲线");
	m_pInputCurve1->SetOption(KCT_ENT_EDGE,1);

	m_pInputCurve2 = new kiInputEntityTool(this,"选择第二条曲线");
	m_pInputCurve2->SetOption(KCT_ENT_EDGE,1);

	return TRUE;
}

BOOL	kcmRuledSurface::DestroyInputTools()
{
	KC_SAFE_DELETE(m_pInputCurve1);
	KC_SAFE_DELETE(m_pInputCurve2);

	return TRUE;
}

BOOL	kcmRuledSurface::InitInputToolQueue()
{
	AddInputTool(m_pInputCurve1);
	AddInputTool(m_pInputCurve2);

	return TRUE;
}

