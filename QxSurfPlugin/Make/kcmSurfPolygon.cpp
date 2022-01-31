#include "StdAfx.h"
#include "kcModel.h"
#include "kcBasePlane.h"
#include "kcPreviewObj.h"
#include "kcPreviewText.h"
#include "kiInputPoint.h"
#include "kcgTextLib.h"
#include "kcEntityFactory.h"
#include "kcmSurfPolygon.h"

kcmSurfPolygon::kcmSurfPolygon(void)
{
	m_strName = "多边形";
	m_strAlias = "Polygon";
	m_strDesc = "平面多边形";

	m_pInputPoint = NULL;
	m_pPreviewPolyline = NULL;
	m_pPreviewLine = NULL;
	m_pPreviewLine2 = NULL;
}

kcmSurfPolygon::~kcmSurfPolygon(void)
{
}

// 表明命令可以完成,主要和Apply函数结合使用.
BOOL	kcmSurfPolygon::CanFinish()//命令是否可以完成
{
	if(m_pointArray.size() > 2)
		return TRUE;
	return FALSE;
}

// 当输入工具获取结果,结束时,会回调对应命令的该函数.
// 用于继续执行命令或切换输入工具.也可能返回KSTA_DONE,表明命令执行结束.
//
int	kcmSurfPolygon::OnInputFinished(kiInputTool *pTool)
{
	if(pTool == m_pInputPoint)
	{
		if(m_pInputPoint->IsInputDone())
		{
			BOOL bAdd = TRUE;
			if(!m_pointArray.empty())
			{
				kPoint3 pp = m_pointArray[m_pointArray.size() - 1];
				double dist = pp.distance(m_aPoint);
				if(dist < 1e-6)
					bAdd = FALSE;
			}
			if(bAdd)
			{
				m_pointArray.push_back(m_aPoint);
				//
				if(m_pPreviewPolyline)
				{
					m_pPreviewPolyline->AddPoint(m_aPoint);
					m_pPreviewPolyline->Display(TRUE);
				}
			}

			NavToInputTool(m_pInputPoint);
		}
	}
	return kiCommand::OnInputFinished(pTool);
}

int		kcmSurfPolygon::OnExecute()
{
	m_pointArray.clear();

	kcDocContext *pDocCtx = GetDocContext();

	m_pPreviewPolyline = new kcPreviewPolyline(this->GetModel()->GetAISContext(),pDocCtx);
	m_pPreviewPolyline->SetColor(1.0,1.0,1.0);

	m_pPreviewLine = new kcPreviewLine(GetModel()->GetAISContext(),pDocCtx);
	m_pPreviewLine->SetColor(1,1,1);

	m_pPreviewLine2 = new kcPreviewLine(GetModel()->GetAISContext(),pDocCtx);
	m_pPreviewLine2->SetColor(1,1,1);

	return KSTA_CONTINUE;
}

int		kcmSurfPolygon::OnEnd(int nCode)
{
	KC_SAFE_DELETE(m_pPreviewPolyline);
	KC_SAFE_DELETE(m_pPreviewLine);
	KC_SAFE_DELETE(m_pPreviewLine2);
	return nCode;
}

int		kcmSurfPolygon::OnApply()
{
	if(m_pointArray.size() > 2)
	{
		if(MakePolyline())
		{
			Done();
			Redraw();

			return KSTA_DONE;
		}
		else
		{
			EndCommand(KSTA_FAILED);

			return KSTA_FAILED;
		}
	}

	return KSTA_CONTINUE;
}

// 创建必要的输入工具
BOOL	kcmSurfPolygon::CreateInputTools()
{
	m_pInputPoint = new kiInputPoint(this,"输入一个点");
	m_pInputPoint->Init(&m_aPoint);
	m_pInputPoint->SetRealtimeUpdate(true);

	return TRUE;
}

BOOL	kcmSurfPolygon::DestroyInputTools()
{
	KC_SAFE_DELETE(m_pInputPoint);

	return TRUE;
}

BOOL	kcmSurfPolygon::InitInputToolQueue()
{
	AddInputTool(m_pInputPoint);
	return TRUE;
}

int		kcmSurfPolygon::OnMouseMove(kuiMouseInput& mouseInput)
{
	kiCommand::OnMouseMove(mouseInput);

	if(!m_pointArray.empty())
	{
		kPoint3 pp = m_pointArray[m_pointArray.size() - 1];
		if(m_pPreviewLine)
		{
			m_pPreviewLine->Update(gp_Pnt(pp[0],pp[1],pp[2]),
				gp_Pnt(m_aPoint[0],m_aPoint[1],m_aPoint[2]));
			m_pPreviewLine->Display(TRUE);
		}
		if(m_pointArray.size() > 1)
		{
			kPoint3 sp = m_pointArray[0];
			if(m_pPreviewLine2)
			{
				m_pPreviewLine2->Update(gp_Pnt(sp[0],sp[1],sp[2]),
					gp_Pnt(m_aPoint[0],m_aPoint[1],m_aPoint[2]));
				m_pPreviewLine2->Display(TRUE);
			}
		}
	}

	return KSTA_CONTINUE;
}

BOOL	kcmSurfPolygon::MakePolyline()
{
	if(m_pointArray.size() <= 2)
		return FALSE;

	TopoDS_Shape aNS;
	try{
		BRepBuilderAPI_MakePolygon mp;
		int ix,isize = (int)m_pointArray.size();
		for(ix = 0;ix < isize;ix ++)
		{
			kPoint3 cp = m_pointArray[ix];
			mp.Add(gp_Pnt(cp[0],cp[1],cp[2]));
		}
		mp.Close();
		mp.Build();
		if(mp.IsDone())
			aNS = mp.Wire();
	}catch(Standard_Failure){
		return FALSE;
	}
	if(aNS.IsNull())
		return FALSE;

	try
	{
		TopoDS_Shape aFace;
		BRepBuilderAPI_MakeFace mf(TopoDS::Wire(aNS),Standard_True);
		if(mf.IsDone())
			aNS = mf.Face();
	}
	catch (Standard_Failure){
		return FALSE;
	}
	if(aNS.IsNull())
		return FALSE;

	kcEntityFactory fac;
	kcEntity *pEntity = fac.Create(aNS);
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
