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
	ResetShapeStats();
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

bool  kcShapeAttribMgr::AddColorAttrib(const TopoDS_Shape &aS,const Quantity_Color &aCol)
{
	if(aS.IsNull()){
		return false;
	}

	Handle(kcShapeAttrib) aSA;
	if(aShapeAttribMap_.FindFromKey(aS,aSA)){
		if(aSA->HasColor()){
			ASSERT(FALSE);
		}
		aSA->SetColor(aCol);
	}else{
		aSA = new kcShapeAttrib;
		aSA->SetColor(aCol);
		aShapeAttribMap_.Add(aS,aSA);
	}

	int aType = (int)aS.ShapeType();
	aShapeStats_[aType] ++;

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

void  kcShapeAttribMgr::Clear()
{
	aShapeAttribMap_.Clear();
	//
	ResetShapeStats();
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

//读取每个有颜色的face的颜色属性信息
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

		Handle(StepVisual_Colour) aSurfCol, aBoundCol, aCurveCol,aRenderCol;
		// check if it is component style
		bool bComponent = false;
		double rendTan = 0.0;
		if (!aStyles.GetColors(aStyleItem,aSurfCol,aBoundCol,aCurveCol,aRenderCol,rendTan,bComponent))
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

//读取每个有颜色的shape的颜色属性信息
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
			TopoDS_Shape aS = TransferBRep::ShapeResult(aTP,aEnt);
			if(!aS.IsNull()){
				TRACE("\n ###--- get shape that has color.");
				Quantity_Color aCol;
				if(aStyles.DecodeColor(aSurfCol,aCol)){
					AddColorAttrib(aS,aCol);
					TRACE("\n ###------ add shape color to shape attrib manager.");
					Bind(aS,aEnt);
				}
			}
		}
	}
}

bool  kcShapeAttribMgr::Bind(const TopoDS_Shape &aS,const Handle(Standard_Transient) &aEnt)
{
	if(aS.IsNull() || aEnt.IsNull()){
		return false;
	}

	if(aShapeStepObjMap_.Contains(aS)){
		TRACE("\n aShapeStepObjMap has a shape binding.");
	}else{
		aShapeStepObjMap_.Add(aS,aEnt);
	}

	return true;
}

bool  kcShapeAttribMgr::WriteToFile(const char *pszFile)
{
	ASSERT(pszFile != NULL);
	FILE *fp = NULL;
	//常用宏描述
	char *aShapeType[] = {
		"COMPOUND",
		"COMPSOLID",
		"SOLID",
		"SHELL",
		"FACE",
		"WIRE",
		"EDGE",
		"VERTEX",
		"SHAPE"
	};

	//如果没有信息，不写文件
	if(aShapeAttribMap_.IsEmpty()){
		return true;
	}

	fopen_s(&fp,pszFile,"w");
	if(fp == NULL){
		TRACE("\n open file %s failed.\n",pszFile);
		return false;
	}


	fprintf_s(fp,"#- has %d styled items.\n",aShapeAttribMap_.Size());
	//
	int ix,nMax = (int)TopAbs_SHAPE;
	for(ix = 0;ix < nMax;ix ++){
		if(aShapeStats_[ix] > 0){
			fprintf_s(fp,"  -- shape type %s has %d object that has color.\n",
				aShapeType[ix],aShapeStats_[ix]);
		}
	}

	//依次写入信息
	Handle(Standard_Type) aType;
	int nbS = aShapeAttribMap_.Extent();

	fprintf_s(fp,"\n ************************************* \n");
	for(ix = 1;ix <= nbS;ix ++){
		const TopoDS_Shape &aS = aShapeAttribMap_.FindKey(ix);
		Handle(kcShapeAttrib) aSA = aShapeAttribMap_.FindFromIndex(ix);
		//
		Handle(Standard_Transient) aEnt;
		aShapeStepObjMap_.FindFromKey(aS,aEnt);

		std::string sName = "Unknown";
		Quantity_Color aCol = aSA->GetColor();
		if(!aEnt.IsNull()){
			aType = aEnt->DynamicType();
			sName = aType->Name();
		}

		fprintf_s(fp,"  -- shape type %s ,ent type %s,has color (%f,%f,%f).\n",
			aShapeType[(int)aS.ShapeType()],sName.c_str(),aCol.Red(),aCol.Green(),aCol.Blue());
	}

	fclose(fp);
	return true;
}

void  kcShapeAttribMgr::ResetShapeStats()
{
	int ix;
	for(ix = 0;ix < 10;ix ++){
		aShapeStats_[ix] = 0;
	}
}