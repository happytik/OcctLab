#include "StdAfx.h"
#include "kcModel.h"
#include "kiInputPoint.h"
#include "kiInputVector.h"
#include "kiInputEntityTool.h"
#include "kiInputAngle.h"
#include "kcEntity.h"
#include "kiSelSet.h"
#include "kvCoordSystem.h"
#include "kcBasePlane.h"
#include "kcEntityTrsf.h"
#include "kcDispObj.h"
#include "QxBaseUtils.h"
#include "kcPreviewObj.h"
#include "kcmTrsfScale.h"

#define KC_STA_PICK_ENTITY			1
#define KC_STA_INPUT_BASEPNT		2
#define KC_STA_INPUT_REFPNT			3
#define KC_STA_INPUT_SCALEPNT		4

kcmTrsfScale::kcmTrsfScale(void)
{
	m_strName = "缩放";
	m_strAlias = "Scale";
	m_strDesc = "等比缩放";

	m_bCopy = false;
	m_pSelEntity = NULL;
	m_pInputBasePnt = NULL;
	m_pInputRefPnt = NULL;
	m_pInputScalePnt = NULL;

	m_pFrmShape = NULL;
	m_pPreviewLine = NULL;
	m_pPreviewLine2 = NULL;
	m_dScale = 1.0;
	m_bInputScale = false;
}

kcmTrsfScale::~kcmTrsfScale(void)
{
}

// 表明命令可以完成,主要和Apply函数结合使用.
BOOL	kcmTrsfScale::CanFinish()//命令是否可以完成
{
	return FALSE;
}

// 当输入工具获取结果,结束时,会回调对应命令的该函数.
// 用于继续执行命令或切换输入工具.也可能返回KSTA_DONE,表明命令执行结束.
//
int	kcmTrsfScale::OnInputFinished(kiInputTool *pTool)
{
	if(pTool == m_pSelEntity)
	{
		if(m_pSelEntity->IsInputDone())
		{
			kiSelSet *pSelSet = m_pSelEntity->GetSelSet();
			kiSelEntity selEnt;
			pSelSet->InitSelected();
			while(pSelSet->MoreSelected())
			{
				selEnt = pSelSet->CurSelected();
				TopoDS_Shape aShape = selEnt._pEntity->GetShape();
				if(!aShape.IsNull())
				{
					m_pFrmShape->AddShape(aShape);
				}

				pSelSet->NextSelected();
			}

			m_nState = KC_STA_INPUT_BASEPNT;
			NavToNextTool();
		}
	}
	else if(pTool == m_pInputBasePnt)
	{
		if(m_pInputBasePnt->IsInputDone())
		{
			m_nState = KC_STA_INPUT_REFPNT;
			NavToNextTool();
		}
	}
	else if(pTool == m_pInputRefPnt)
	{
		if(m_pInputRefPnt->IsInputDone())
		{
			NavToNextTool();
			m_nState = KC_STA_INPUT_SCALEPNT;
		}
	}
	else if(pTool == m_pInputScalePnt)
	{
		if(m_pInputScalePnt->IsInputDone())
		{
			DoScale();
			Done();
			Redraw();

			return KSTA_DONE;
		}
	}
	return KSTA_CONTINUE;
}

void	kcmTrsfScale::OnOptionChanged(kiInputTool *pTool,kiOptionItem *pOptionItem)
{
	if(pTool == m_pInputScalePnt)
	{
		if(pOptionItem)
		{
			m_pInputScalePnt->SetInputState(KINPUT_DONE);
			m_bInputScale = true;
		}
	}
}

int		kcmTrsfScale::OnExecute()
{
	m_nState = KC_STA_PICK_ENTITY;
	m_pFrmShape = new kcDispFrmShape(GetModel()->GetAISContext());

	kcDocContext *pDocCtx = GetDocContext();

	m_pPreviewLine = new kcPreviewLine(GetModel()->GetAISContext(),pDocCtx);
	m_pPreviewLine->SetColor(1,1,1);

	m_pPreviewLine2 = new kcPreviewLine(GetModel()->GetAISContext(),pDocCtx);
	m_pPreviewLine2->SetColor(1,1,1);

	m_bInputScale = false;

	return KSTA_CONTINUE;
}

int		kcmTrsfScale::OnEnd(int nCode)
{
	KC_SAFE_DELETE(m_pFrmShape);
	KC_SAFE_DELETE(m_pPreviewLine);
	KC_SAFE_DELETE(m_pPreviewLine2);

	return nCode;
}

int		kcmTrsfScale::OnMouseMove(kuiMouseInput& mouseInput)
{
	kiCommand::OnMouseMove(mouseInput);

	if(m_nState == KC_STA_INPUT_REFPNT)
	{
		if(m_pPreviewLine)
		{
			m_pPreviewLine->Update(KCG_GP_PNT(m_basePoint),KCG_GP_PNT(m_refPoint));
			m_pPreviewLine->Display(TRUE);
		}
	}
	else if(m_nState == KC_STA_INPUT_SCALEPNT)
	{
		if(m_pPreviewLine2)
		{
			m_pPreviewLine2->Update(KCG_GP_PNT(m_basePoint),KCG_GP_PNT(m_aScalePoint));
			m_pPreviewLine2->Display(TRUE);
		}

		if(m_pFrmShape)
		{
			double scale = m_aScalePoint.distance(m_basePoint) /
				m_refPoint.distance(m_basePoint);
			m_pFrmShape->Scale(m_basePoint,scale);
			m_pFrmShape->Show();
		}
	}

	return KSTA_CONTINUE;
}

// 创建必要的输入工具,在Initialize中调用，每个命令仅仅调用一次。
BOOL	kcmTrsfScale::CreateInputTools()
{
	m_optionSet.AddBoolOption("拷贝对象",'C',m_bCopy);
	m_pSelEntity = new kiInputEntityTool(this,"拾取对象",&m_optionSet);
	m_pSelEntity->SetOption(KCT_ENT_ALL);

	m_pInputBasePnt = new kiInputPoint(this,"选择基点");
	m_pInputBasePnt->Init(&m_basePoint,false);

	m_pInputRefPnt = new kiInputPoint(this,"选择第一参考点");
	m_pInputRefPnt->Init(&m_refPoint,false);
	m_pInputRefPnt->SetRealtimeUpdate(true);

	m_optionSet2.AddDoubleOption("缩放比例",'S',m_dScale);
	m_pInputScalePnt = new kiInputPoint(this,"选择第二参考点",&m_optionSet2);
	m_pInputScalePnt->Init(&m_aScalePoint,false);
	m_pInputScalePnt->SetRealtimeUpdate(true);

	return TRUE;
}

// 销毁创建的输入工具.每个命令仅调用一次.
BOOL	kcmTrsfScale::DestroyInputTools()
{
	KC_SAFE_DELETE(m_pSelEntity);
	KC_SAFE_DELETE(m_pInputBasePnt);
	KC_SAFE_DELETE(m_pInputRefPnt);
	KC_SAFE_DELETE(m_pInputScalePnt);

	return TRUE;
}

// 在begin中调用，主要是初始化工具队列。
BOOL	kcmTrsfScale::InitInputToolQueue()
{
	AddInputTool(m_pSelEntity);
	AddInputTool(m_pInputBasePnt);
	AddInputTool(m_pInputRefPnt);
	AddInputTool(m_pInputScalePnt);

	return TRUE;
}

BOOL	kcmTrsfScale::DoScale()
{
	double dscale = m_dScale;
	if(!m_bInputScale)
	{
		double dist = m_refPoint.distance(m_basePoint);
		if(fabs(dist) < KDBL_MIN)
			return FALSE;
		dscale = m_aScalePoint.distance(m_basePoint) / dist;
	}

	kcEntityTrsf entrsf;
	std::vector<kcEntity *> aEnt,aNewEnt;
	kcEntity *pEntity = NULL;
	kiSelSet *pSelSet = m_pSelEntity->GetSelSet();
	kiSelEntity selEnt;
	pSelSet->InitSelected();
	while(pSelSet->MoreSelected())
	{
		selEnt = pSelSet->CurSelected();
		//
		if(!m_bCopy)
			aEnt.push_back(selEnt._pEntity);

		//
		pEntity = entrsf.Scale(selEnt._pEntity,m_basePoint,dscale);
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
		if(!m_bCopy)
		{
			isize = (int)aEnt.size();
			for(ix = 0;ix < isize;ix ++)
				pModel->DelEntity(aEnt[ix]);
		}
		pModel->AddEntity(aNewEnt);
	}
	pModel->EndUndo();

	return TRUE;
}

