#include "StdAfx.h"
#include <AIS_LengthDimension.hxx>
#include <AIS.hxx>
#include <gce_MakeDir.hxx>
#include "QxBaseUtils.h"
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
#define KC_STA_LINE_PICKED			1
#define KC_STA_PICK_SECOND_SHAPE    2
#define KC_STA_INPUT_OFFSET			3

// 0:直线长度标注
kcmLengthDimension::kcmLengthDimension(int subCmd)
	:nSubCmd_(subCmd)
{
	m_strName = "线性标注";
	m_strAlias = "LinDim";
	m_strDesc = "线性标注";

	pPickLineTool_ = NULL;
	pInputLenTool_ = NULL;
	pOptionSet_ = NULL;
	pOptionSet2_ = NULL;
	dFlyout_ = 10;
	dArrowLen_ = 2.0;
	dFontHeight_ = 16.0;

	nState_ = 0;

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
	if(pTool->IsInputDone()){
		//
		if (nSubCmd_ == 0) {
			if (pTool == pPickLineTool_) {
				if (GetPickedLineInfo()) {
					nState_ = KC_STA_LINE_PICKED;
					//
					pInputLenTool_->Set(basePnt_, offDir_, true);
					NavToNextTool();
				}
			}
			else if (pTool == pInputLenTool_) {
				if (BuildLineLenDim()) {
					Done();
					return KSTA_DONE;
				}
				else {
					EndCommand(KSTA_FAILED);
					return KSTA_FAILED;
				}
			}
		}else {
			if (pTool == m_pPickFirstShape){
				nState_ = KC_STA_PICK_SECOND_SHAPE;
				NavToNextTool();
			}else if (pTool == m_pPickSecondShape){
				//计算基准点和偏移方向
				if (!CalcPointAndDir())
					return KSTA_CONTINUE;

				m_pInputLength->Set(m_aBasePoint, m_aOffDir, true);

				nState_ = KC_STA_INPUT_OFFSET;
				NavToNextTool();
			}else if (pTool == m_pInputLength){
				if (BuildDimension()){
					Done();
					return KSTA_DONE;
				}else{
					EndCommand(KSTA_FAILED);
					return KSTA_FAILED;
				}
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

bool kcmLengthDimension::GetPickedLineInfo()
{
	QxShapePickInfo info;
	if (pPickLineTool_->GetSelCount() != 1 ||
		!pPickLineTool_->GetShapePickInfo(info))
		return false;

	kiSelEntity se = pPickLineTool_->GetFirstSelect();
	TopoDS_Shape aS, aGS;
	bool bLocalShape = false;
	if (se.IsLocalShape()) {
		aGS = se.EntityShape();
		aS = se.LocalShape();
		bLocalShape = true;
	}
	else {
		aS = se.EntityShape();
	}

	if (aS.IsNull())
		return false;

	double dF = 0.0, dL = 0.0;
	GeomAdaptor_Curve aAC;
	Handle(Geom_Curve) aCurve = BRep_Tool::Curve(TopoDS::Edge(aS), dF, dL);
	if (aCurve.IsNull())
		return false;

	aAC.Load(aCurve, dF, dL);
	if (aAC.GetType() != GeomAbs_Line) {
		return false;
	}

	gp_Vec v1;
	aCurve->D1(dF, aFirstPnt_, v1);
	aCurve->D0(dL, aSecondPnt_);

	//获取当前工作平面
	kcBasePlane *pBPln = GetCurrentBasePlane();
	ASSERT(pBPln);
	aPlane_ = pBPln->GetPlane();
	double dis1 = aPlane_.Distance(aFirstPnt_);
	double dis2 = aPlane_.Distance(aSecondPnt_);
	if (fabs(dis1) > K_DBL_EPS || fabs(dis2) > K_DBL_EPS) {
		//没有再基准面上
		if (bLocalShape) {
			//获取关联的face,并计算平面
		}
	}
	
	//计算基点和偏移方向
	info._p.get(basePnt_);
	//
	gp_Dir zd = aPlane_.Axis().Direction(),vd(v1);
	gp_Dir od = zd.Crossed(vd);
	KxFromOccDir(od, offDir_);

	return true;
}

BOOL kcmLengthDimension::CalcPointAndDir()
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

	//Standard_Boolean isOnPlane1, isOnPlane2;
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

int	kcmLengthDimension::OnExecute()
{
	if (nSubCmd_ == 0) {

	}
	else {
		nState_ = KC_STA_PICK_FIRST_SHAPE;
	}

	m_pPreviewLine = new kcPreviewLine(GetAISContext(),GetDocContext());
	m_pPreviewLine->SetColor(0.0,1.0,0.0);

	return KSTA_CONTINUE;
}

int kcmLengthDimension::OnEnd(int nCode)
{
	KC_SAFE_DELETE(m_pPreviewLine);

	return nCode;
}

// 创建必要的输入工具
BOOL kcmLengthDimension::CreateInputTools()
{
	pOptionSet_ = new kiOptionSet(this);
	pOptionSet2_ = new kiOptionSet(this);

	kiOptionDouble *pOptDbl = new kiOptionDouble("Flyout", 'F', dFlyout_);
	pOptDbl->EnablePopup(true);
	pOptionSet2_->AddOption(pOptDbl);

	pOptDbl = new kiOptionDouble("ArrowLen", 'L', dArrowLen_);
	pOptDbl->EnablePopup(true);
	pOptionSet_->AddOption(pOptDbl);

	pOptDbl = new kiOptionDouble("FontHeight", 'H', dFontHeight_);
	pOptDbl->EnablePopup(true);
	pOptionSet_->AddOption(pOptDbl);

	pOptionSet_->AddQuitButtonOption();
	pOptionSet2_->AddQuitButtonOption();

	if (nSubCmd_ == 0) {//基于直线进行标注
		pPickLineTool_ = new kiInputEntityTool(this, "pick a line", pOptionSet_);
		pPickLineTool_->SetOption(KCT_ENT_EDGE, true);
		//pPickLineTool_->SetNaturalMode(false);//允许拾取shape内部的边
		pPickLineTool_->NeedPickInfo(true);

		pInputLenTool_ = new kiInputLength(this, "input flyout", pOptionSet2_);
		pInputLenTool_->SetDefault(dFlyout_);
	}else {
		m_pTypeOpt = new kiOptionEnum("类型", 'T', m_nType);
		m_pTypeOpt->AddEnumItem("线性", 'L');
		m_pTypeOpt->AddEnumItem("垂直", 'V');
		m_pTypeOpt->AddEnumItem("水平", 'H');
		m_optionSet.AddOption(m_pTypeOpt);
		m_optionSet.AddDoubleOption("箭头大小", 'S', m_dArrowSize);

		m_pPickFirstShape = new kiInputEntityTool(this, "选择第一个对象", &m_optionSet);
		m_pPickFirstShape->SetOption(KCT_ENT_POINT, true);

		m_pPickSecondShape = new kiInputEntityTool(this, "选择第二个对象", &m_optionSet);
		m_pPickSecondShape->SetOption(KCT_ENT_POINT, true);

		m_pInputLength = new kiInputLength(this, "输入偏移距离");
		m_pInputLength->SetDefault(5.0);
	}
		

	return TRUE;
}

BOOL kcmLengthDimension::DestroyInputTools()
{
	KC_SAFE_DELETE(pPickLineTool_);
	KC_SAFE_DELETE(pInputLenTool_);
	KC_SAFE_DELETE(pOptionSet_);
	KC_SAFE_DELETE(pOptionSet2_);
	KC_SAFE_DELETE(m_pPickFirstShape);
	KC_SAFE_DELETE(m_pPickSecondShape);
	KC_SAFE_DELETE(m_pInputLength);
	return TRUE;
}

BOOL kcmLengthDimension::InitInputToolQueue()
{
	if (nSubCmd_ == 0) {
		AddInputTool(pPickLineTool_);
		AddInputTool(pInputLenTool_);
	}else {
		AddInputTool(m_pPickFirstShape);
		AddInputTool(m_pPickSecondShape);
		AddInputTool(m_pInputLength);
	}
	
	return TRUE;
}

int		kcmLengthDimension::OnMouseMove(kuiMouseInput& mouseInput)
{
	kiCommand::OnMouseMove(mouseInput);

	if (nState_ == KC_STA_LINE_PICKED) {
		double len = pInputLenTool_->GetLength();
		kPoint3 bp(aFirstPnt_.X(), aFirstPnt_.Y(), aFirstPnt_.Z());
		kPoint3 ep(aSecondPnt_.X(), aSecondPnt_.Y(), aSecondPnt_.Z());
		kVector3 vdir(offDir_);

		bp = bp + len * vdir;
		ep = ep + len * vdir;
		
		if (m_pPreviewLine){
			m_pPreviewLine->Update(bp, ep);
			m_pPreviewLine->Display(TRUE);
		}
	}

	if(nState_ == KC_STA_INPUT_OFFSET)
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
BOOL kcmLengthDimension::BuildDimension()
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

bool kcmLengthDimension::BuildLineLenDim()
{
	dFlyout_ = pInputLenTool_->GetLength();
	//
	kcLengthDimEntity *pDimEnt = new kcLengthDimEntity;
	pDimEnt->Initialize(aFirstPnt_, aSecondPnt_, aPlane_);
	pDimEnt->SetFlyout(dFlyout_);
	pDimEnt->SetArrowLength(dArrowLen_);
	pDimEnt->SetFontHeight(dFontHeight_);

	kcModel *pModel = GetModel();
	pModel->BeginUndo(GetName());
	pModel->AddEntity(pDimEnt);
	pModel->EndUndo();

	Redraw();

	return true;
}