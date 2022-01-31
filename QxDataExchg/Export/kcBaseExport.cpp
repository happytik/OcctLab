#include "StdAfx.h"
#include <string>
#include <TopTools_ListIteratorOfListOfShape.hxx>
#include "kcLayer.h"
#include "kcModel.h"
#include "kcEntity.h"
#include "kiSelSet.h"
#include "kcEntityFactory.h"
#include "kcBaseExport.h"

kcBaseExport::kcBaseExport(kcModel *pModel)
	:_pModel(pModel)
{
	ASSERT(pModel);
	_bExportSelShape = false;//默认不使能
	_bExportAsComp = false;
	_bHasAddedShape = false;
}

kcBaseExport::~kcBaseExport(void)
{
}

void  kcBaseExport::SetAISContext(const Handle(AIS_InteractiveContext) &aCtx)
{
	ASSERT(!aCtx.IsNull());
	hAISCtx_ = aCtx;
}

void  kcBaseExport::AddShape(const TopoDS_Shape &aShape)
{
	if(!aShape.IsNull()){
		_aShapeList.Append(aShape);
		_bHasAddedShape = true;
	}
}

//
int  kcBaseExport::GetAllShapeFromModel()
{
	kcEntity *pEntity = NULL;
	kcEntityList entList;
	kcEntityList::iterator ite;

	_aShapeList.Clear();
	_pModel->GetAllEntity(entList);
	for(ite = entList.begin();ite != entList.end();ite ++){
		pEntity = *ite;
		TopoDS_Shape aShape = pEntity->GetShape();
		if(!aShape.IsNull())
			_aShapeList.Append(aShape);
	}

	return 1;
}

int kcBaseExport::GetAllSelectedShape()
{
	ASSERT(!hAISCtx_.IsNull());
	TopoDS_Builder aBB;
	TopoDS_Compound aComp;
	kcEntity *pEntity = NULL;
	Handle(AIS_InteractiveObject) aObj;

	if(_bExportAsComp){
		aBB.MakeCompound(aComp);
	}

	hAISCtx_->InitSelected();
	for(;hAISCtx_->MoreSelected();hAISCtx_->NextSelected()){
		aObj = hAISCtx_->SelectedInteractive();
		if(!aObj.IsNull()){
			pEntity = _pModel->GetEntity(aObj);
			if(pEntity){
				TopoDS_Shape aS = pEntity->GetShape();
				if(!aS.IsNull()){
					if(_bExportAsComp){
						aBB.Add(aComp,aS);
					}else{
						_aShapeList.Append(aS);
					}
				}
			}
		}
	}

	if(_bExportAsComp){
		_aShapeList.Append(aComp);
	}

	//kiSelSet *pSelSet = ksGetGlobalSelSet();
	//ASSERT(pSelSet);
	//for(pSelSet->InitSelected();pSelSet->MoreSelected();pSelSet->NextSelected()){
	//	kiSelEntity se = pSelSet->CurSelected();
	//	TopoDS_Shape aShape = se._pEntity->GetShape();
	//	if(!aShape.IsNull())
	//		_aShapeList.Append(aShape);
	//}

	return 1;
}