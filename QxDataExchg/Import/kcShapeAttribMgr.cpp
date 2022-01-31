#include "stdafx.h"
#include <vector>
#include <STEPControl_Reader.hxx>
#include <STEPCAFControl_Reader.hxx>
#include <XSControl_WorkSession.hxx>
#include <Transfer_TransientProcess.hxx>
#include <STEPConstruct_Styles.hxx>
#include <StepVisual_StyledItem.hxx>
#include <StepRepr_Representation.hxx>
#include <StepShape_AdvancedFace.hxx>
#include <XSControl_TransferReader.hxx>
#include <TransferBRep.hxx>
#include "kcShapeAttribMgr.h"


kcShapeAttribMgr::kcShapeAttribMgr(void)
{
}


kcShapeAttribMgr::~kcShapeAttribMgr(void)
{
	Clear();
}

bool  kcShapeAttribMgr::AddColorAttrib(const TopoDS_Shape &aS,double r,double g,double b)
{
	if(aS.IsNull()){
		return false;
	}
	Quantity_Color aCol(r,g,b,Quantity_TOC_RGB);
	return AddColorAttrib(aS,aCol);
}

// 记录一个shape的颜色属性,可能是一个Face对象，也可能是一个Solid等组合对象
// 需要分别进行处理
bool  kcShapeAttribMgr::AddColorAttrib(const TopoDS_Shape &aS,const Quantity_Color &aCol)
{
	if(aS.IsNull()){
		return false;
	}

	Handle(kcShapeAttrib) aSA;

	//判断是否已经处理过了
	if(aShapeAttribMap_.FindFromKey(aS,aSA)){// 对应shape已经在map中
		//存在颜色重复设置的情况.why???
		if(aSA->HasColor()){
			Quantity_Color aC = aSA->GetColor();
			TRACE("\n ###---??? shape has color (%f,%f,%f),set color is (%f,%f,%f).",
				aC.Red(),aC.Green(),aC.Blue(),aCol.Red(),aCol.Green(),aCol.Blue());
		}
		return true;
	}
	
	//单独的一个Face对象，是该face特有的颜色
	if(aS.ShapeType() == TopAbs_FACE){
		//添加一个新的记录
		aSA = new kcShapeAttrib;
		aSA->SetColor(aCol);
		aFaceSpecColorMap_.Add(aS,aSA);
		aShapeAttribMap_.Add(aS,aSA);

		return true;
	}

	//复合对象，记录每个face的颜色
	if(aS.ShapeType() < TopAbs_FACE){
		//记录一个对象
		aSA = new kcShapeAttrib;
		aSA->SetColor(aCol);

		//遍历，并记录
		TopExp_Explorer exp;
		for(exp.Init(aS,TopAbs_FACE);exp.More();exp.Next()){
			const TopoDS_Shape &aFace = exp.Current();
			//
			if(!aFaceOfShapeColorMap_.Contains(aFace)){
				aFaceOfShapeColorMap_.Add(aFace,aSA);
			}
		}

		aShapeAttribMap_.Add(aS,aSA);
		return true;
	}
	
	//曲线、曲面等对象的颜色
	aSA = new kcShapeAttrib;
	aSA->SetColor(aCol);
	aShapeAttribMap_.Add(aS,aSA);
	
	return true;
}

bool  kcShapeAttribMgr::FindColorAttrib(const TopoDS_Shape &aS,Quantity_Color &aColor)
{
	if(aS.IsNull()) return false;

	Handle(kcShapeAttrib) aSA;
	if(aShapeAttribMap_.FindFromKey(aS,aSA)){
		if(aSA->HasColor()){
			aColor = aSA->GetColor();
			return true;
		}
	}
	return false;
}

// 查找face的颜色
bool  kcShapeAttribMgr::FindFaceColor(const TopoDS_Shape &aS,Quantity_Color &aColor)
{
	Handle(kcShapeAttrib) aSA;
	if(aS.IsNull() || aS.ShapeType() != TopAbs_FACE){
		ASSERT(FALSE);
		return false;
	}

	//先查找face特有的颜色
	if(aFaceSpecColorMap_.FindFromKey(aS,aSA)){
		ASSERT(aSA->HasColor());
		aColor = aSA->GetColor();
		return true;
	}

	if(aFaceOfShapeColorMap_.FindFromKey(aS,aSA)){
		ASSERT(aSA->HasColor());
		aColor = aSA->GetColor();
		return true;
	}

	return false;
}

void  kcShapeAttribMgr::Clear()
{
	aShapeAttribMap_.Clear();
	aFaceSpecColorMap_.Clear();
	aFaceOfShapeColorMap_.Clear();
}

bool  kcShapeAttribMgr::HasColorAttribs() const
{
	return (aShapeAttribMap_.IsEmpty() ? false : true);
}

bool  kcShapeAttribMgr::HasFaceColor() const
{
	if(!aFaceSpecColorMap_.IsEmpty() ||
		!aFaceOfShapeColorMap_.IsEmpty())
		return true;
	return false;
}

bool  kcShapeAttribMgr::GetShapeList(TopTools_ListOfShape &aShapeList)
{
	if(aShapeAttribMap_.IsEmpty())
		return false;

	kcShapeAttribIdxMap::Iterator ite(aShapeAttribMap_);
	for(;ite.More();ite.Next()){
		const TopoDS_Shape &aS = ite.Key();
		aShapeList.Append(aS);
	}

	return true;
}

// 读取每个有颜色的face的颜色属性信息
//
void  kcShapeAttribMgr::ReadFaceColors(STEPControl_Reader &aReader)
{
	Handle(XSControl_WorkSession) aWS = aReader.WS();
	Handle(Transfer_TransientProcess) aTP = aWS->TransferReader()->TransientProcess();

	STEPConstruct_Styles aStyles ( aWS );
	if ( ! aStyles.LoadStyles() ) {
		TRACE("\n ###--- step file no styledItems object---.");
		return;
	}

	int ix,jx,nb = aStyles.NbStyles();
	for(ix = 1;ix <= nb;ix ++){
		Handle(StepVisual_StyledItem) aStyleItem = aStyles.Style(ix);
		if(aStyleItem.IsNull())
			continue;

		Handle(StepVisual_Colour) aSurfCol, aBoundCol, aCurveCol, aRenderCol;
		// check if it is component style
		bool bComponent = false;
		double transp = 0.0;
		if (!aStyles.GetColors(aStyleItem,aSurfCol,aBoundCol,aCurveCol, aRenderCol, transp,bComponent))
			continue;

		if(aSurfCol.IsNull())
			continue;

		// find shape
		std::vector<Handle(Standard_Transient)> anItems;
		if (!aStyleItem->Item().IsNull()) {
			anItems.push_back(aStyleItem->Item());
		}
		else if (!aStyleItem->ItemAP242().Representation().IsNull()){
			//special case for AP242: item can be Reprsentation
			Handle(StepRepr_Representation) aRepr = aStyleItem->ItemAP242().Representation();
			for (jx = 1; jx <= aRepr->Items()->Length(); jx++)
				anItems.push_back(aRepr->Items()->Value(jx));
		}
		//
		if(anItems.empty())
			continue;

		std::vector<Handle(Standard_Transient)>::size_type iix,iisz = anItems.size();
		for(iix = 0;iix < iisz;iix ++){
			Handle(Standard_Transient) aEnt = anItems.at(iix);
			Handle(Standard_Type) aType = aEnt->DynamicType();
			TRACE("\n ###--- ent type is %s.",aType->Name());
			if(aEnt->IsKind(STANDARD_TYPE(StepShape_AdvancedFace))){
				TopoDS_Shape aS = TransferBRep::ShapeResult(aTP,aEnt);
				if(!aS.IsNull()){
					TRACE("\n ###--- get shape of advanced face.");
					Quantity_Color aCol;
					if(aStyles.DecodeColor(aSurfCol,aCol)){
						AddColorAttrib(aS,aCol);
						TRACE("\n ###------ add face color to shape attrib manager.");
					}
				}
			}
		}
	}
}

// 读取每个有颜色的shape的颜色属性信息
//
void  kcShapeAttribMgr::ReadShapeColors(STEPControl_Reader &aReader)
{
	Handle(XSControl_WorkSession) aWS = aReader.WS();
	Handle(Transfer_TransientProcess) aTP = aWS->TransferReader()->TransientProcess();

	STEPConstruct_Styles aStyles ( aWS );
	if ( ! aStyles.LoadStyles() ) {
		TRACE("\n ###--- step file no styledItems object---.");
		return;
	}

	int ix,jx,nb = aStyles.NbStyles();
	for(ix = 1;ix <= nb;ix ++){
		Handle(StepVisual_StyledItem) aStyleItem = aStyles.Style(ix);
		if(aStyleItem.IsNull())
			continue;

		Handle(StepVisual_Colour) aSurfCol, aBoundCol, aCurveCol, aRenderCol;
		// check if it is component style
		bool bComponent = false;
		double transp = 0.0;
		if (!aStyles.GetColors(aStyleItem,aSurfCol,aBoundCol,aCurveCol,aRenderCol,transp, bComponent))
			continue;

		// 目前仅读取曲面的颜色属性，如果没有，则不处理
		if(aSurfCol.IsNull())
			continue;

		// find shape
		std::vector<Handle(Standard_Transient)> anItems;
		if (!aStyleItem->Item().IsNull()) {
			anItems.push_back(aStyleItem->Item());
		}
		else if (!aStyleItem->ItemAP242().Representation().IsNull()){
			//special case for AP242: item can be Reprsentation
			Handle(StepRepr_Representation) aRepr = aStyleItem->ItemAP242().Representation();
			for (jx = 1; jx <= aRepr->Items()->Length(); jx++)
				anItems.push_back(aRepr->Items()->Value(jx));
		}
		//
		if(anItems.empty())
			continue;

		std::vector<Handle(Standard_Transient)>::size_type iix,iisz = anItems.size();
		for(iix = 0;iix < iisz;iix ++){
			Handle(Standard_Transient) aEnt = anItems.at(iix);
			Handle(Standard_Type) aType = aEnt->DynamicType();
			//TRACE("\n ###--- ent type is %s.",aType->Name());
			TopoDS_Shape aS = TransferBRep::ShapeResult(aTP,aEnt);
			if(!aS.IsNull()){
				//TRACE("\n ###--- get shape that has color.");
				Quantity_Color aCol;
				if(aStyles.DecodeColor(aSurfCol,aCol)){
					AddColorAttrib(aS,aCol);
					//TRACE("\n ###------ add shape color to shape attrib manager.");
				}
			}
		}
	}
}