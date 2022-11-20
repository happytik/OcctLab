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
	m_strName = "ֱ����";
	m_strAlias = "RuledSurf";
	m_strDesc = "ֱ����";

	m_pInputCurve1 = NULL;
	m_pInputCurve2 = NULL;
}

kcmRuledSurface::~kcmRuledSurface(void)
{
}

// ��������������,��Ҫ��Apply�������ʹ��.
BOOL	kcmRuledSurface::CanFinish()//�����Ƿ�������
{
	return FALSE;
}

// �����빤�߻�ȡ���,����ʱ,��ص���Ӧ����ĸú���.
// ���ڼ���ִ��������л����빤��.Ҳ���ܷ���KSTA_DONE,��������ִ�н���.
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

	// ͨ��Edge�������ϵ�һ���֣���ˣ���Ҫ��ȡ��Ӧ��һ������
	// ������ֱ��ʹ����������ֱ����
	kcEntityFactory efac;
	kcEntity *pEntity = NULL;
	try{
		double df1,dl1,df2,dl2;
		Handle(Geom_Curve) aCrv,aCrv1,aCrv2,aCCrv;
		//��һ������
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
		//����ֱ����
		if(!aCrv1.IsNull() && !aCrv2.IsNull()){
			//����ʹ�øú���
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

// ������Ҫ�����빤��
BOOL	kcmRuledSurface::CreateInputTools()
{
	m_pInputCurve1 = new kiInputEntityTool(this,"ѡ���һ������");
	m_pInputCurve1->SetOption(KCT_ENT_EDGE,1);
	m_pInputCurve1->SetNaturalMode(false);

	m_pInputCurve2 = new kiInputEntityTool(this,"ѡ��ڶ�������");
	m_pInputCurve2->SetOption(KCT_ENT_EDGE,1);
	m_pInputCurve2->SetNaturalMode(false);

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

