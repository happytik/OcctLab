#include "StdAfx.h"
#include <GeomFill_Pipe.hxx>
#include "kiInputEntityTool.h"
#include "kiInputVector.h"
#include "kiInputValueTool.h"
#include "kcEntity.h"
#include "kcBasePlane.h"
#include "kcModel.h"
#include "QxCurveLib.h"
#include "kvCoordSystem.h"
#include "kcEntityFactory.h"
#include "kcmSurfacePipe.h"

kcmSurfacePipe::kcmSurfacePipe(void)
{
	m_strName = "管道面";
	m_strAlias = "PipeSurf";
	m_strDesc = "等半径管道面";

	m_pPickPathTool = NULL;
	m_pInputRadiusTool = NULL;
	m_dRadius = 5.0;

	std::ofstream os;
}

kcmSurfacePipe::~kcmSurfacePipe(void)
{
}

// 表明命令可以完成,主要和Apply函数结合使用.
BOOL	kcmSurfacePipe::CanFinish()//命令是否可以完成
{
	return FALSE;
}

// 当输入工具获取结果,结束时,会回调对应命令的该函数.
// 用于继续执行命令或切换输入工具.也可能返回KSTA_DONE,表明命令执行结束.
//
int	kcmSurfacePipe::OnInputFinished(kiInputTool *pTool)
{
	if(pTool->IsInputDone())
	{
		if(pTool == m_pPickPathTool)
		{
			NavToNextTool();
		}
		else if(pTool == m_pInputRadiusTool)
		{
			if(BuildPipe())
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

int		kcmSurfacePipe::OnExecute()
{
	return KSTA_CONTINUE;
}

int		kcmSurfacePipe::OnEnd(int nCode)
{
	return nCode;
}

// 创建必要的输入工具
BOOL kcmSurfacePipe::CreateInputTools()
{
	m_pPickPathTool = new kiInputEntityTool(this,"拾取轨迹曲线");
	m_pPickPathTool->SetOption(KCT_ENT_EDGE,1);
	m_pPickPathTool->SetNaturalMode(false);

	m_pInputRadiusTool = new kiInputDoubleTool(this,"管道半径");
	m_pInputRadiusTool->Init(&m_dRadius,0.0001,10000);

	return TRUE;
}

BOOL kcmSurfacePipe::DestroyInputTools()
{
	KC_SAFE_DELETE(m_pPickPathTool);
	KC_SAFE_DELETE(m_pInputRadiusTool);

	return TRUE;
}

BOOL	kcmSurfacePipe::InitInputToolQueue()
{
	AddInputTool(m_pPickPathTool);
	AddInputTool(m_pInputRadiusTool);

	return TRUE;
}

//
BOOL	kcmSurfacePipe::BuildPipe()
{
	if(m_pPickPathTool->GetSelCount() != 1)
		return FALSE;
	kiSelEntity se = m_pPickPathTool->GetFirstSelect();
	TopoDS_Shape aS = se.SelectShape();
	if(aS.IsNull() || aS.ShapeType() != TopAbs_EDGE)
		return FALSE;

	double df,dl;
	TopoDS_Edge aEdge = TopoDS::Edge(aS);
	Handle(Geom_Curve) aCrv = BRep_Tool::Curve(aEdge,df,dl);
	if(aCrv.IsNull())
		return FALSE;

	Handle(Geom_Curve) aCCrv = Handle(Geom_Curve)::DownCast(aCrv->Copy());
	Handle(Geom_TrimmedCurve) aTrimCrv = new Geom_TrimmedCurve(aCCrv,df,dl);
	if(aTrimCrv.IsNull())
		return FALSE;

	TopoDS_Shape aNS;
	try{
		GeomFill_Pipe mp;
		mp.Init(aTrimCrv,m_dRadius);
		mp.Perform(1e-6,Standard_False);
		Handle(Geom_Surface) aSurf = mp.Surface();
		if(aSurf.IsNull())
			return FALSE;

		BRepBuilderAPI_MakeFace mf(aSurf,Precision::Confusion());
		mf.Build();
		if(mf.IsDone())
			aNS = mf.Face();
	}catch(Standard_Failure){
		return FALSE;
	}
	if(aNS.IsNull())
		return FALSE;

	kcEntityFactory efac;
	kcEntity *pEntity = efac.Create(aNS);
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
	
//////////////////////////////////////////////////////////////////////////
//
// 单轨扫掠
kcmSurfaceSweep1::kcmSurfaceSweep1(void)
{
	m_strName = "单轨扫掠面";
	m_strAlias = "Sweep1Surf";
	m_strDesc = "单轨扫掠面";

	m_pPickPathTool = NULL;
	m_pPickSectTool = NULL;
	m_nTrihedronType = 0;
	m_pTrihedronOpt = NULL;
}

kcmSurfaceSweep1::~kcmSurfaceSweep1(void)
{

}

// 表明命令可以完成,主要和Apply函数结合使用.
BOOL	kcmSurfaceSweep1::CanFinish()//命令是否可以完成
{
	return FALSE;
}

// 当输入工具获取结果,结束时,会回调对应命令的该函数.
// 用于继续执行命令或切换输入工具.也可能返回KSTA_DONE,表明命令执行结束.
//
int	kcmSurfaceSweep1::OnInputFinished(kiInputTool *pTool)
{
	if(pTool->IsInputDone())
	{
		if(pTool == m_pPickPathTool)
		{
			NavToNextTool();
		}
		else if(pTool == m_pPickSectTool)
		{
			if(BuildSweep())
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

int		kcmSurfaceSweep1::OnExecute()
{
	return KSTA_CONTINUE;
}

int		kcmSurfaceSweep1::OnEnd(int nCode)
{
	return nCode;
}

// 创建必要的输入工具
BOOL	kcmSurfaceSweep1::CreateInputTools()
{
	m_pTrihedronOpt = new kiOptionEnum("框架类型",'T',m_nTrihedronType);
	m_pTrihedronOpt->AddEnumItem("CorrectedFrenet",'R');
	m_pTrihedronOpt->AddEnumItem("Frenet",'F');
	m_pTrihedronOpt->AddEnumItem("Fixed",'X');
	m_pTrihedronOpt->AddEnumItem("ConstantNormal",'C');
	m_optionSet.AddOption(m_pTrihedronOpt);

	m_pPickPathTool = new kiInputEntityTool(this,"选取轨道曲线",&m_optionSet);
	m_pPickPathTool->SetOption(KCT_ENT_EDGE,1);

	m_pPickSectTool = new kiInputEntityTool(this,"选取截面曲线",&m_optionSet);
	m_pPickSectTool->SetOption(KCT_ENT_EDGE,1);

	return TRUE;
}

BOOL	kcmSurfaceSweep1::DestroyInputTools()
{
	m_optionSet.Clear();
	KC_SAFE_DELETE(m_pPickPathTool);
	KC_SAFE_DELETE(m_pPickSectTool);

	return TRUE;
}

BOOL	kcmSurfaceSweep1::InitInputToolQueue()
{
	AddInputTool(m_pPickPathTool);
	AddInputTool(m_pPickSectTool);

	return TRUE;
}


//
BOOL	kcmSurfaceSweep1::BuildSweep()
{
	if(m_pPickPathTool->GetSelCount() != 1 ||
		m_pPickSectTool->GetSelCount() != 1)
		return FALSE;

	Handle(Geom_Curve) aPathCrv,aSectCrv;

	kiSelEntity se = m_pPickPathTool->GetFirstSelect();
	TopoDS_Shape aS = se._pEntity->GetShape();
	TopoDS_Edge aEdge = TopoDS::Edge(aS);
	aPathCrv = QxCurveLib::GetCurveCopy(aEdge);
	if(aPathCrv.IsNull())
		return FALSE;

	se = m_pPickSectTool->GetFirstSelect();
	aS = se._pEntity->GetShape();
	aEdge = TopoDS::Edge(aS);
	aSectCrv = QxCurveLib::GetCurveCopy(aEdge);
	if(aSectCrv.IsNull())
		return FALSE;

	TopoDS_Shape aNS;
	try{
		GeomFill_Trihedron opt;
		switch(m_nTrihedronType){
			case 0: opt = GeomFill_IsCorrectedFrenet; break;
			case 1: opt = GeomFill_IsFrenet; break;
			case 2: opt = GeomFill_IsFixed; break;
			case 3: opt = GeomFill_IsConstantNormal; break;
			default:
				opt = GeomFill_IsCorrectedFrenet;
				break;
		}
		GeomFill_Pipe mp;
		mp.Init(aPathCrv,aSectCrv,opt);
		mp.Perform(1e-6,Standard_False);
		Handle(Geom_Surface) aSurf = mp.Surface();
		if(aSurf.IsNull())
			return FALSE;

		BRepBuilderAPI_MakeFace mf(aSurf,Precision::Confusion());
		mf.Build();
		if(mf.IsDone())
			aNS = mf.Face();

	}catch(Standard_Failure){
		return FALSE;
	}

	if(aNS.IsNull())
		return FALSE;

	kcEntityFactory efac;
	kcEntity *pEntity = efac.Create(aNS);
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

//////////////////////////////////////////////////////////////////////////
//
// 多截面线扫掠
kcmSurfaceSweepN::kcmSurfaceSweepN(void)
{
	m_strName = "多线扫掠面";
	m_strAlias = "SweepNSurf";
	m_strDesc = "多线扫掠面";

	m_pPickPathTool = NULL;
	m_pPickSectTool = NULL;
}

kcmSurfaceSweepN::~kcmSurfaceSweepN(void)
{

}

// 表明命令可以完成,主要和Apply函数结合使用.
BOOL	kcmSurfaceSweepN::CanFinish()//命令是否可以完成
{
	return FALSE;
}

// 当输入工具获取结果,结束时,会回调对应命令的该函数.
// 用于继续执行命令或切换输入工具.也可能返回KSTA_DONE,表明命令执行结束.
//
int	kcmSurfaceSweepN::OnInputFinished(kiInputTool *pTool)
{
	if(pTool->IsInputDone())
	{
		if(pTool == m_pPickPathTool)
		{
			NavToNextTool();
		}
		else if(pTool == m_pPickSectTool)
		{
			if(BuildSweep())
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

int		kcmSurfaceSweepN::OnExecute()
{
	return KSTA_CONTINUE;
}

int		kcmSurfaceSweepN::OnEnd(int nCode)
{
	return nCode;
}

// 创建必要的输入工具
BOOL	kcmSurfaceSweepN::CreateInputTools()
{
	m_pPickPathTool = new kiInputEntityTool(this,"选取轨道曲线");
	m_pPickPathTool->SetOption(KCT_ENT_EDGE,1);

	m_pPickSectTool = new kiInputEntityTool(this,"选取截面曲线");
	m_pPickSectTool->SetOption(KCT_ENT_EDGE,100);

	return TRUE;
}

BOOL	kcmSurfaceSweepN::DestroyInputTools()
{
	KC_SAFE_DELETE(m_pPickPathTool);
	KC_SAFE_DELETE(m_pPickSectTool);

	return TRUE;
}

BOOL	kcmSurfaceSweepN::InitInputToolQueue()
{
	AddInputTool(m_pPickPathTool);
	AddInputTool(m_pPickSectTool);

	return TRUE;
}


//
BOOL	kcmSurfaceSweepN::BuildSweep()
{
	if(m_pPickPathTool->GetSelCount() != 1 ||
		m_pPickSectTool->GetSelCount() < 2)
		return FALSE;

	Handle(Geom_Curve) aPathCrv,aSectCrv;
	TColGeom_SequenceOfCurve seqOfCrv;

	kiSelEntity se = m_pPickPathTool->GetFirstSelect();
	TopoDS_Shape aS = se._pEntity->GetShape();
	TopoDS_Edge aEdge = TopoDS::Edge(aS);
	aPathCrv = QxCurveLib::GetCurveCopy(aEdge);
	if(aPathCrv.IsNull())
		return FALSE;

	//获取截面线
	kiSelSet *pSelSet = m_pPickSectTool->GetSelSet();
	pSelSet->InitSelected();
	while(pSelSet->MoreSelected())
	{
		se = pSelSet->CurSelected();
		aS = se._pEntity->GetShape();
		aEdge = TopoDS::Edge(aS);
		aSectCrv = QxCurveLib::GetCurveCopy(aEdge);
		if(!aSectCrv.IsNull())
			seqOfCrv.Append(aSectCrv);

		pSelSet->NextSelected();
	}
	
	TopoDS_Shape aNS;
	try{
		Handle(Geom_Surface) aSurf;
		if(seqOfCrv.Length() == 2)
		{
			GeomFill_Pipe mp(aPathCrv,seqOfCrv.First(),seqOfCrv.Last());
			mp.Perform(1e-6,Standard_True);
			aSurf = mp.Surface();
		}
		else
		{
			GeomFill_Pipe mp(aPathCrv,seqOfCrv);
			mp.Perform(1e-6,Standard_True);
			aSurf = mp.Surface();
		}
		if(aSurf.IsNull())
			return FALSE;

		BRepBuilderAPI_MakeFace mf(aSurf,Precision::Confusion());
		mf.Build();
		if(mf.IsDone())
			aNS = mf.Face();

	}catch(Standard_Failure){
		return FALSE;
	}

	if(aNS.IsNull())
		return FALSE;

	kcEntityFactory efac;
	kcEntity *pEntity = efac.Create(aNS);
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