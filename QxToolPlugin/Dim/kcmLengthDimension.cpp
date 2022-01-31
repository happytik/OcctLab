#include "StdAfx.h"
#include <AIS_LengthDimension.hxx>
#include <AIS.hxx>
#include <gce_MakeDir.hxx>
#include "kiInputEntityTool.h"
#include "kiInputValueTool.h"
#include "kiInputLength.h"
#include "kcEntity.h"
#include "kcBasePlane.h"
#include "kcModel.h"
#include "kvCoordSystem.h"
#include "kcEntityFactory.h"
#include "kcPreviewObj.h"
#include "kcmLengthDimension.h"

#define KC_STA_PICK_FIRST_SHAPE		1
#define KC_STA_PICK_SECOND_SHAPE    2
#define KC_STA_INPUT_OFFSET			3

kcmLengthDimension::kcmLengthDimension(void)
{
	m_strName = "线性标注";
	m_strAlias = "LinDim";
	m_strDesc = "线性标注";

	m_pPickFirstShape = NULL;
	m_pPickSecondShape = NULL;
	m_pInputLength = NULL;
	m_nType = 0;
	m_dArrowSize = 5.0;
	m_pTypeOpt = NULL;

	m_pPreviewLine = NULL;
}

kcmLengthDimension::~kcmLengthDimension(void)
{
}

// 表明命令可以完成,主要和Apply函数结合使用.
BOOL	kcmLengthDimension::CanFinish()//命令是否可以完成
{
	return FALSE;
}

// 当输入工具获取结果,结束时,会回调对应命令的该函数.
// 用于继续执行命令或切换输入工具.也可能返回KSTA_DONE,表明命令执行结束.
//
int	kcmLengthDimension::OnInputFinished(kiInputTool *pTool)
{
	if(pTool->IsInputDone())
	{
		if(pTool == m_pPickFirstShape)
		{
			m_nState = KC_STA_PICK_SECOND_SHAPE;
			NavToNextTool();
		}
		else if(pTool == m_pPickSecondShape)
		{
			//计算基准点和偏移方向
			if(!CalcPointAndDir())
				return KSTA_CONTINUE;

			m_pInputLength->Set(m_aBasePoint,m_aOffDir,true);
			
			m_nState = KC_STA_INPUT_OFFSET;
			NavToNextTool();
		}
		else if(pTool == m_pInputLength)
		{
			if(BuildDimension())
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

BOOL	kcmLengthDimension::CalcPointAndDir()
{
	if(m_pPickFirstShape->GetSelCount() < 1 ||
		m_pPickSecondShape->GetSelCount() < 1)
		return FALSE;

	TopoDS_Shape aFirstShape = m_pPickFirstShape->SelectedShape(0);
	TopoDS_Shape aSecondShape = m_pPickSecondShape->SelectedShape(0);
	gp_Pnt p1,p2;

	kvCoordSystem cs;
	GetWorkPlaneCoordSystem(cs);
	m_aPlane = new Geom_Plane(gp_Pnt(cs.Org().x(),cs.Org().y(),
		cs.Org().z()),gp_Dir(cs.Z().x(),cs.Z().y(),cs.Z().z()));

	Standard_Boolean isOnPlane1, isOnPlane2;
	//AIS::ComputeGeometry(TopoDS::Vertex(aFirstShape),p1,m_aPlane,isOnPlane1);
	//AIS::ComputeGeometry(TopoDS::Vertex(aSecondShape),p2,m_aPlane,isOnPlane2);
	m_aPnt1.set(p1.X(),p1.Y(),p1.Z());
	m_aPnt2.set(p2.X(),p2.Y(),p2.Z());
	gp_Pnt mp((p1.XYZ() + p2.XYZ()) / 2.0);
	m_aBasePoint.set(mp.X(),mp.Y(),mp.Z());

	//计算方向
	if(m_nType == 0)
	{
		gp_Dir dir = gce_MakeDir(p1,p2);
		dir.Rotate(m_aPlane->Pln().Axis(),K_PI * 0.5);
		m_aOffDir.set(dir.X(),dir.Y(),dir.Z());
	}
	else if(m_nType == 1)//垂直
	{
		m_aOffDir = cs.X();
	}
	else
	{
		m_aOffDir = cs.Y();
	}

	return TRUE;
}

int		kcmLengthDimension::OnExecute()
{
	m_nState = KC_STA_PICK_FIRST_SHAPE;

	m_pPreviewLine = new kcPreviewLine(GetAISContext(),GetDocContext());
	m_pPreviewLine->SetColor(0.0,1.0,0.0);

	return KSTA_CONTINUE;
}

int		kcmLengthDimension::OnEnd(int nCode)
{
	KC_SAFE_DELETE(m_pPreviewLine);

	return nCode;
}

// 创建必要的输入工具
BOOL	kcmLengthDimension::CreateInputTools()
{
	m_pTypeOpt = new kiOptionEnum("类型",'T',m_nType);
	m_pTypeOpt->AddEnumItem("线性",'L');
	m_pTypeOpt->AddEnumItem("垂直",'V');
	m_pTypeOpt->AddEnumItem("水平",'H');
	m_optionSet.AddOption(m_pTypeOpt);
	m_optionSet.AddDoubleOption("箭头大小",'S',m_dArrowSize);

	m_pPickFirstShape = new kiInputEntityTool(this,"选择第一个对象",&m_optionSet);
	m_pPickFirstShape->SetOption(KCT_ENT_POINT,true);
	
	m_pPickSecondShape = new kiInputEntityTool(this,"选择第二个对象",&m_optionSet);
	m_pPickSecondShape->SetOption(KCT_ENT_POINT,true);
	
	m_pInputLength = new kiInputLength(this,"输入偏移距离");
	m_pInputLength->SetDefault(5.0);

	return TRUE;
}

BOOL	kcmLengthDimension::DestroyInputTools()
{
	KC_SAFE_DELETE(m_pPickFirstShape);
	KC_SAFE_DELETE(m_pPickSecondShape);
	KC_SAFE_DELETE(m_pInputLength);
	return TRUE;
}

BOOL	kcmLengthDimension::InitInputToolQueue()
{
	AddInputTool(m_pPickFirstShape);
	AddInputTool(m_pPickSecondShape);
	AddInputTool(m_pInputLength);

	return TRUE;
}

int		kcmLengthDimension::OnMouseMove(kuiMouseInput& mouseInput)
{
	kiCommand::OnMouseMove(mouseInput);

	if(m_nState == KC_STA_INPUT_OFFSET)
	{
		double len = m_pInputLength->GetLength();
		kPoint3 bp,ep;
		
		if(m_nType == 0)
		{
			bp = m_aPnt1 + len * m_aOffDir;
			ep = m_aPnt2 + len * m_aOffDir;
		}
		else if(m_nType == 1)//垂直
		{
			double dist = (m_aPnt2 - m_aPnt1) * m_aOffDir; 
			bp = m_aPnt1 + (len + dist / 2.0) * m_aOffDir;
			ep = m_aPnt2 + (len - dist / 2.0) * m_aOffDir;
		}
		else
		{
			double dist = (m_aPnt2 - m_aPnt1) * m_aOffDir; 
			bp = m_aPnt1 + (len + dist / 2.0) * m_aOffDir;
			ep = m_aPnt2 + (len - dist / 2.0) * m_aOffDir;
		}
		
		if(m_pPreviewLine)
		{
			m_pPreviewLine->Update(bp,ep);
			m_pPreviewLine->Display(TRUE);
		}
	}

	return KSTA_CONTINUE;
}

//
BOOL	kcmLengthDimension::BuildDimension()
{
	if(m_pPickFirstShape->GetSelCount() < 1 ||
		m_pPickSecondShape->GetSelCount() < 1)
	   return FALSE;

	TopoDS_Shape aFirstShape = m_pPickFirstShape->SelectedShape(0);
	TopoDS_Shape aSecondShape = m_pPickSecondShape->SelectedShape(0);

	//计算距离
	char sbuf[128];
	double dist;
	
	double len = m_pInputLength->GetLength();

	if(m_nType == 0)
		dist = m_aPnt1.distance(m_aPnt2);
	else
		dist = (m_aPnt2 - m_aPnt1) * m_aOffDir;
	sprintf_s(sbuf,128,"%.4f",dist);

	gp_Pnt pos;
	kPoint3 p = m_aBasePoint + len * m_aOffDir;
	pos.SetCoord(p[0],p[1],p[2]);

	try{
		//TCollection_ExtendedString es(sbuf);
		//Handle(AIS_LengthDimension) aLenDim = new AIS_LengthDimension(aFirstShape,
		//	aSecondShape,m_aPlane,10.0,es);

		////自己算
		//aLenDim->SetAutomaticPosition(Standard_False);
		//aLenDim->SetPosition(pos);

		//aLenDim->SetArrowSize(m_dArrowSize);
		//if(m_nType == 0)
		//	aLenDim->SetTypeOfDist(AIS_TOD_Unknown);
		//else if(m_nType == 1)
		//	aLenDim->SetTypeOfDist(AIS_TOD_Vertical);
		//else
		//	aLenDim->SetTypeOfDist(AIS_TOD_Horizontal);
		//

		//Handle(AIS_InteractiveContext) aCtx = m_pModel->GetAISContext();
		//aCtx->Display(aLenDim);
	}catch(Standard_Failure){
		return FALSE;
	}

	return TRUE;
}
