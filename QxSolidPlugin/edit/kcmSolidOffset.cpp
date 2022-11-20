#include "StdAfx.h"
#include <BRepOffsetAPI_MakeOffsetShape.hxx>
#include "kiInputEntityTool.h"
#include "kiInputValueTool.h"
#include "kiInputOptionsTool.h"
#include "kcEntity.h"
#include "kcBasePlane.h"
#include "kcModel.h"
#include "kvCoordSystem.h"
#include "kcEntityFactory.h"
#include "QxBRepLib.h"
#include "kcmSolidOffset.h"

kcmSolidOffset::kcmSolidOffset(int subCmd)
	:nSubCmd_(subCmd)
{
	m_strName = "实体偏移";
	m_strAlias = "SolidOffset";
	m_strDesc = "曲面或实体偏移";

	pOptionSet_ = NULL;
	pSideOptItem_ = NULL;

	pInputEntity_ = NULL;
	pOptionsTool_ = NULL;
	dOffset_ = 5.0;
	nOffsetMethod_ = 1;
	bOutSide_ = true;
	nJoinType_ = 0;
}

kcmSolidOffset::~kcmSolidOffset(void)
{
}

// 表明命令可以完成,主要和Apply函数结合使用.
BOOL	kcmSolidOffset::CanFinish()//命令是否可以完成
{
	return FALSE;
}

// 当输入工具获取结果,结束时,会回调对应命令的该函数.
// 用于继续执行命令或切换输入工具.也可能返回KSTA_DONE,表明命令执行结束.
//
int	kcmSolidOffset::OnInputFinished(kiInputTool *pTool)
{
	if (pTool && pTool->IsInputDone()) {
		if (pTool == pInputEntity_) {
			NavToNextTool();
		}else if(pTool == pOptionsTool_){

			if (BuildOffsetShape()) {

				Done();
				return KSTA_DONE;
			}
		}
	}
	
	return KSTA_CONTINUE;
}

int kcmSolidOffset::OnExecute()
{
	return KSTA_CONTINUE;
}

int kcmSolidOffset::OnEnd(int nCode)
{
	//pInputEntity_->ClearSel();
	return nCode;
}

// 创建必要的输入工具
BOOL	kcmSolidOffset::CreateInputTools()
{
	int nSelType = 0;
	std::string sDesc;

	//
	if (nSubCmd_ == 0) {
		nSelType = KCT_ENT_FACE;
		sDesc = "Select an Face";
	}
	else if (nSubCmd_ == 1) {
		nSelType = KCT_ENT_SHELL;
		sDesc = "Select an Shell";
	}
	else if (nSubCmd_ == 2) {
		nSelType = KCT_ENT_SOLID;
		sDesc = "Select an Solid";
	}

	pInputEntity_ = new kiInputEntityTool(this, sDesc.c_str(), NULL);
	pInputEntity_->SetOption(nSelType, 1);
	if (nSubCmd_ == 0) {
		pInputEntity_->SetNaturalMode(false);
	}
	else {
		pInputEntity_->SetNaturalMode(true);
	}

	pOptionSet_ = new kiOptionSet(this);

	kiOptionDouble *pDblItem = new kiOptionDouble("Offset", 'O', dOffset_);
	pDblItem->EnablePopup(true);
	pOptionSet_->AddOption(pDblItem);

	kiOptionEnum *pEnumItem = new kiOptionEnum("Method", 'M', nOffsetMethod_);
	pEnumItem->AddEnumItem("Simple", 'S');
	pEnumItem->AddEnumItem("Join", 'J');
	pOptionSet_->AddOption(pEnumItem);

	pEnumItem = new kiOptionEnum("JoinType", 'J', nJoinType_);
	pEnumItem->AddEnumItem("Arc", 'A');
	pEnumItem->AddEnumItem("Tangent", 'T');
	pEnumItem->AddEnumItem("Intersection", 'I');
	pOptionSet_->AddOption(pEnumItem);

	pSideOptItem_ = pOptionSet_->AddBoolOption("向外偏移", 'D', bOutSide_);
	pOptionSet_->AddApplyButtonOption();
	pOptionSet_->AddQuitButtonOption();
	
	pOptionsTool_ = new kiInputOptionsTool(this, "Options", pOptionSet_);
	pOptionsTool_->AcceptDefaultValue(true);

	return TRUE;
}

BOOL	kcmSolidOffset::DestroyInputTools()
{
	KC_SAFE_DELETE(pOptionSet_);
	KC_SAFE_DELETE(pInputEntity_);
	KC_SAFE_DELETE(pOptionsTool_);
	
	return TRUE;
}

BOOL	kcmSolidOffset::InitInputToolQueue()
{
	AddInputTool(pInputEntity_);
	AddInputTool(pOptionsTool_);
	return TRUE;
}

int kcmSolidOffset::OnApply()//实际应用
{
	if (pInputEntity_->GetSelCount() <= 0) {
		AfxMessageBox("需要选择要偏移的对象.");
		return KSTA_CONTINUE;
	}

	if (BuildOffsetShape()) {
		return KSTA_CONTINUE;
	}

	return KSTA_CONTINUE;
}

void kcmSolidOffset::OnOptionChanged(kiOptionItem *pOptionItem)
{
	if (pOptionItem == pSideOptItem_) {
		TRACE("\n side changed.");
	}
}

BOOL kcmSolidOffset::BuildOffsetShape()
{
	if(pInputEntity_->GetSelCount() <= 0)
		return FALSE;

	kiSelEntity se = pInputEntity_->GetFirstSelect();
	TopoDS_Shape aShape = se.SelectShape();
	double doff = dOffset_;
	if(!bOutSide_)
		doff = -doff;

	TopoDS_Shape aNS;
	try{
		BRepOffsetAPI_MakeOffsetShape mos;//(aShape,doff,0.001);
		if (nOffsetMethod_ == 0) {
			mos.PerformBySimple(aShape, doff);
		}
		else {
			GeomAbs_JoinType jtyp = GeomAbs_Arc;
			if (nJoinType_ == 1) jtyp = GeomAbs_Tangent;
			if (nJoinType_ == 2) jtyp = GeomAbs_Intersection;
			mos.PerformByJoin(aShape, doff, 0.001, BRepOffset_Skin, true, false, jtyp);
		}
		if (mos.IsDone()) {
			aNS = mos.Shape();
		}
		if (aNS.IsNull()) {
			AfxMessageBox("Offset Failed.");
			return FALSE;
		}
	}catch(Standard_Failure){
		return FALSE;
	}

	kcEntityFactory efac;
	kcEntity *pEntity = efac.Create(aNS);
	if(pEntity){
		kcModel *pModel = GetModel();
		pModel->BeginUndo(GetName());
		pModel->AddEntity(pEntity);
		pModel->EndUndo();

		return TRUE;
	}


	return FALSE;
}

