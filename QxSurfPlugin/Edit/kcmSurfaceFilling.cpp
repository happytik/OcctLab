#include "StdAfx.h"
#include <BRepOffsetAPI_MakeFilling.hxx>
#include "kiInputEntityTool.h"
#include "kiInputValueTool.h"
#include "kcEntity.h"
#include "kcBasePlane.h"
#include "kcModel.h"
#include "kvCoordSystem.h"
#include "kcPreviewObj.h"
#include "kcEntityFactory.h"
#include "kcmSurfaceFilling.h"

kcmSurfaceFilling::kcmSurfaceFilling(void)
{
	m_strName = "补面";
	m_strAlias = "Filling";
	m_strDesc = "多边补面";

	m_pPickEdgeTool = NULL;
	m_nDegree = 3;
	m_nMaxSeg = 9;
	m_nbPtsOnCrv = 15;
	m_nbIter = 2;
	m_bBound = TRUE;
	m_nContType = 0;
	m_pOptionCont = NULL;
}

kcmSurfaceFilling::~kcmSurfaceFilling(void)
{
}

// 表明命令可以完成,主要和Apply函数结合使用.
BOOL	kcmSurfaceFilling::CanFinish()//命令是否可以完成
{
	if(m_aEdge.empty())
		return FALSE;
	return TRUE;
}

// 当输入工具获取结果,结束时,会回调对应命令的该函数.
// 用于继续执行命令或切换输入工具.也可能返回KSTA_DONE,表明命令执行结束.
//
int		kcmSurfaceFilling::OnInputFinished(kiInputTool *pTool)
{
	if(pTool == m_pPickEdgeTool)
	{
		if(m_pPickEdgeTool->IsInputDone())
		{
			TopoDS_Edge aEdge = TopoDS::Edge(m_pPickEdgeTool->SelectedShape(0));
			if(!aEdge.IsNull() && m_aShapeMap.Contains(aEdge) == Standard_False)
			{
				axCrvInfo *pInfo = new axCrvInfo;
				pInfo->_aEdge = aEdge;
				pInfo->_nCont = m_nContType;
				pInfo->_bBound = m_bBound;

				pInfo->_pPrevCrv = new kcPreviewCurve(GetModel()->GetAISContext(),GetDocContext());
				pInfo->_pPrevCrv->SetColor(0,1,0);
				pInfo->_pPrevCrv->SetWidth(2.0);
				pInfo->_pPrevCrv->Update(pInfo->_aEdge);
				pInfo->_pPrevCrv->Display(TRUE);

				m_aEdge.push_back(pInfo);
			}
			
			NavToInputTool(m_pPickEdgeTool);
		}
		//else
		//{
		//	EndCommand(KSTA_CANCEL);
		//	return KSTA_CANCEL;
		//}
	}
	return KSTA_CONTINUE;
}

int		kcmSurfaceFilling::OnExecute()
{
	ClearEdgeInfo();

	return KSTA_CONTINUE;
}

int		kcmSurfaceFilling::OnEnd(int nCode)
{
	ClearEdgeInfo();
	return nCode;
}

int		kcmSurfaceFilling::OnApply()
{
	EndCurrentInputTool();

	if(DoFilling())
	{
		Done();
		return KSTA_DONE;
	}
	else
	{
		EndCommand(KSTA_FAILED);
		return KSTA_FAILED;
	}

	return KSTA_DONE;
}

// 创建必要的输入工具
BOOL	kcmSurfaceFilling::CreateInputTools()
{
	m_optionSet.AddIntOption("次数",'D',m_nDegree);
	m_optionSet.AddIntOption("曲线上点个数",'S',m_nbPtsOnCrv);
	m_optionSet.AddIntOption("迭代次数",'I',m_nbIter);
	m_optionSet.AddIntOption("最大段数",'G',m_nMaxSeg);
	
	m_pOptionCont = new kiOptionEnum("连续条件",'C',m_nContType);
	m_pOptionCont->AddEnumItem("G0",'P');
	m_pOptionCont->AddEnumItem("G1",'T');
	m_pOptionCont->AddEnumItem("G2",'V');
	m_optionSet.AddOption(m_pOptionCont);

	m_optionSet.AddBoolOption("是否边界",'B',m_bBound);

	m_pPickEdgeTool = new kiInputEntityTool(this,"选择面经过曲线",&m_optionSet);
	m_pPickEdgeTool->SetOption(KCT_ENT_EDGE,1);
	m_pPickEdgeTool->SetNaturalMode(false);
    
	return TRUE;
}

BOOL	kcmSurfaceFilling::DestroyInputTools()
{
	KC_SAFE_DELETE(m_pPickEdgeTool);
	return TRUE;
}

BOOL	kcmSurfaceFilling::InitInputToolQueue()
{
	AddInputTool(m_pPickEdgeTool);
	return TRUE;
}

//
BOOL	kcmSurfaceFilling::DoFilling()
{
	int ix,nbShape = (int)m_aEdge.size();
	if(nbShape <= 0)
		return FALSE;

	TopoDS_Shape aNS;
	try{
		GeomAbs_Shape ct = GeomAbs_C0;
		BRepOffsetAPI_MakeFilling mf(m_nDegree,m_nbPtsOnCrv,m_nbIter);
		mf.SetApproxParam(8,m_nMaxSeg);
		for(ix = 0;ix < nbShape;ix ++)
		{
			axCrvInfo *pInfo = m_aEdge.at(ix);
			switch(pInfo->_nCont){
				case 1: ct = GeomAbs_G1; break;
				case 2: ct = GeomAbs_G2; break;
				default:
					ct = GeomAbs_C0;
					break;
			}
			if(pInfo->_bBound)
				mf.Add(pInfo->_aEdge,ct,Standard_True);
			else
				mf.Add(pInfo->_aEdge,ct,Standard_False);
		}
		mf.Build();
		if(mf.IsDone())
			aNS = mf.Shape();
	}catch(Standard_Failure){

	}

	if(aNS.IsNull())
		return FALSE;

	kcEntityFactory entfac;
	kcEntity *pEnt = entfac.Create(aNS);
	if(pEnt)
	{
		kcModel *pModel = GetModel();
		pModel->BeginUndo(GetName());
		pModel->AddEntity(pEnt);
		pModel->EndUndo();

		return TRUE;
	}

	return FALSE;
}

//
void	kcmSurfaceFilling::ClearEdgeInfo()
{
	int ix,isize = (int)m_aEdge.size();
	for(ix = 0;ix < isize;ix ++)
	{
		m_aEdge[ix]->_pPrevCrv->Display(FALSE);
		delete m_aEdge[ix]->_pPrevCrv;
		delete m_aEdge[ix];
	}
	m_aEdge.clear();

	m_aShapeMap.Clear();
}