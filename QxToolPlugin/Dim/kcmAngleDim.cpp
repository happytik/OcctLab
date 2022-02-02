#include "StdAfx.h"
#include <AIS_LengthDimension.hxx>
#include <AIS_DiameterDimension.hxx>
#include <PrsDim_AngleDimension.hxx>
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
#include "kcmAngleDim.h"

#define KC_STA_PICK_FIRST_SHAPE		1
#define KC_STA_PICK_SECOND_SHAPE    2
#define KC_STA_INPUT_OFFSET			3

kcmAngleDim::kcmAngleDim(int subCmd)
	:nSubCmd_(subCmd)
{
	m_strName = "角度标注";
	m_strAlias = "AngleDim";
	m_strDesc = "角度标注";

	pPickLineTool_ = NULL;
	pPickLineTool2_ = NULL;
	pOptionSet_ = NULL;
	dFlyout_ = 10;
	dArrowLen_ = 2.0;
	dFontHeight_ = 16.0;

	nState_ = 0;
}

kcmAngleDim::~kcmAngleDim(void)
{
}

// 表明命令可以完成,主要和Apply函数结合使用.
BOOL	kcmAngleDim::CanFinish()//命令是否可以完成
{
	return FALSE;
}

// 当输入工具获取结果,结束时,会回调对应命令的该函数.
// 用于继续执行命令或切换输入工具.也可能返回KSTA_DONE,表明命令执行结束.
//
int	kcmAngleDim::OnInputFinished(kiInputTool *pTool)
{
	if(pTool->IsInputDone()){
		if (nSubCmd_ == 0) {//两条直线
			if (pTool == pPickLineTool_) {
				if (GetSelectedLine(true)) {
					NavToNextTool();
				}
			}
			else if (pTool == pPickLineTool2_) {
				//
				if (BuildDimension()) {
					Done();
					return KSTA_DONE;
				}
				else {
					EndCommand(KSTA_FAILED);
					return KSTA_FAILED;
				}
			}
		}
	}else{
		EndCommand(KSTA_CANCEL);
		return KSTA_CANCEL;
	}

	return KSTA_CONTINUE;
}

int		kcmAngleDim::OnExecute()
{

	return KSTA_CONTINUE;
}

int		kcmAngleDim::OnEnd(int nCode)
{

	return nCode;
}

// 创建必要的输入工具
BOOL kcmAngleDim::CreateInputTools()
{
	pOptionSet_ = new kiOptionSet(this);

	kiOptionDouble *pOptDbl = new kiOptionDouble("Flyout", 'F', dFlyout_);
	pOptDbl->EnablePopup(true);
	pOptionSet_->AddOption(pOptDbl);

	pOptDbl = new kiOptionDouble("ArrowLen", 'L', dArrowLen_);
	pOptDbl->EnablePopup(true);
	pOptionSet_->AddOption(pOptDbl);

	pOptDbl = new kiOptionDouble("FontHeight", 'H', dFontHeight_);
	pOptDbl->EnablePopup(true);
	pOptionSet_->AddOption(pOptDbl);

	pOptionSet_->AddQuitButtonOption();

	if (nSubCmd_ == 0) {
		pPickLineTool_ = new kiInputEntityTool(this, "select first Line", pOptionSet_);
		pPickLineTool_->SetOption(KCT_ENT_EDGE, true);
		pPickLineTool_->SetNaturalMode(false);

		pPickLineTool2_ = new kiInputEntityTool(this, "select second Line", pOptionSet_);
		pPickLineTool2_->SetOption(KCT_ENT_EDGE, true);
		pPickLineTool2_->SetNaturalMode(false);
	}
	
	return TRUE;
}

BOOL kcmAngleDim::DestroyInputTools()
{
	KC_SAFE_DELETE(pPickLineTool_);
	KC_SAFE_DELETE(pPickLineTool2_);
	KC_SAFE_DELETE(pOptionSet_);
	return TRUE;
}

BOOL	kcmAngleDim::InitInputToolQueue()
{
	AddInputTool(pPickLineTool_);
	AddInputTool(pPickLineTool2_);

	return TRUE;
}

int kcmAngleDim::OnMouseMove(kuiMouseInput& mouseInput)
{
	kiCommand::OnMouseMove(mouseInput);



	return KSTA_CONTINUE;
}

bool kcmAngleDim::GetSelectedLine(bool bFirst)
{
	kiSelEntity se;
	kiInputEntityTool *pTool = pPickLineTool_;
	if (!bFirst) {
		pTool = pPickLineTool2_;
	}
	if (pTool->GetSelCount() != 1)
		return false;

	double dF = 0.0, dL = 0.0;
	TopoDS_Shape aE = pTool->SelectedShape(0);
	Handle(Geom_Curve) aCurve = BRep_Tool::Curve(TopoDS::Edge(aE), dF, dL);
	GeomAdaptor_Curve aAC;
	aAC.Load(aCurve, dF, dL);
	if (aAC.GetType() != GeomAbs_Line) {
		return false;
	}

	if (bFirst) {
		aEdge1_ = TopoDS::Edge(aE);
	}else {
		aEdge2_ = TopoDS::Edge(aE);
	}

	return true;
}

//
bool kcmAngleDim::BuildDimension()
{
	if (!GetSelectedLine(false))
		return false;
	
	kcEntity *pEntity = NULL;
	if (nSubCmd_ == 0) {//two line
		Handle(PrsDim_AngleDimension) aDim = new PrsDim_AngleDimension(aEdge1_, aEdge2_);
		if (!aDim->IsValid())
			return false;

		kcAngleDimEntity *pDimEnt = new kcAngleDimEntity;
		pDimEnt->Initialize(aEdge1_, aEdge2_);
		//
		pDimEnt->SetFlyout(dFlyout_);
		pDimEnt->SetArrowLength(dArrowLen_);
		pDimEnt->SetFontHeight(dFontHeight_);
		pEntity = pDimEnt;
	}

	if (pEntity) {
		kcModel *pModel = GetModel();
		pModel->BeginUndo(GetName());
		pModel->AddEntity(pEntity);
		pModel->EndUndo();

		Redraw();

		return true;
	}
	
	return false;
}
