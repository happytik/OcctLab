#include "stdafx.h"
#include <XCAFDoc_ShapeTool.hxx>
#include <XCAFDoc_DocumentTool.hxx>
#include <XCAFDoc_ColorTool.hxx>
#include <XCAFDoc_LayerTool.hxx>
#include "kcStepMbStringMgr.h"
#include "kcStepLayerMgr.h"

//////////////////////////////////////////////////////////////////
//
kcStepLayer::kcStepLayer()
{
	szName_.clear();
	nIndex_ = -1;
}

kcStepLayer::kcStepLayer(const std::string &szName,int idx)
{
	ASSERT(!szName.empty() && (idx >= 0));
	szName_ = szName;
	nIndex_ = idx;
}

void  kcStepLayer::AddShape(const TopoDS_Shape &aS)
{
	if(!aS.IsNull()){
		aShapeList_.Append(aS);
	}
}

//////////////////////////////////////////////////////////////////
//
kcStepLayerMgr::kcStepLayerMgr(void)
{
	pMbStrMgr_ = NULL;
}

kcStepLayerMgr::kcStepLayerMgr(kcStepMbStringMgr *pMbStrMgr)
{
	pMbStrMgr_ = pMbStrMgr;
}

kcStepLayerMgr::~kcStepLayerMgr(void)
{
	Clear();
}

bool  kcStepLayerMgr::ReadLayers(const Handle(TDocStd_Document) &aDoc)
{
	ASSERT(pMbStrMgr_ != NULL);
	Handle(XCAFDoc_LayerTool) aLyTool = XCAFDoc_DocumentTool::LayerTool(aDoc->Main());
	Handle(XCAFDoc_ShapeTool) aShapeTool = aLyTool->ShapeTool();

	//获取图层列表
	TDF_LabelSequence aLyLabs;
	aLyTool->GetLayerLabels(aLyLabs);
	if(aLyLabs.IsEmpty())//没有图层
		return true;

	kcStepLayer *pLayer = NULL;
	TCollection_ExtendedString aExtName;
	std::string szName;
	int ix,nbLayers = aLyLabs.Length();
	for(ix = 1;ix <= nbLayers;ix ++){
		TDF_Label aLayerLab = aLyLabs.Value(ix);
		//获取图层名称
		if(!aLyTool->GetLayer(aLayerLab,aExtName)){
			continue;
		}
		//
		if(!pMbStrMgr_->DecodeExtString(aExtName,szName)){
			continue;
		}

		//添加一个图层对象
		pLayer = AddLayer(szName,ix);

		//读取shape列表
		TDF_LabelSequence aShpLabs;
		aLyTool->GetShapesOfLayer(aLayerLab,aShpLabs);
		
		TopoDS_Shape aShape;
		int kx,nbShpLab = aShpLabs.Length();
		for(kx = 1;kx <= nbShpLab;kx ++){
			TDF_Label aSLab = aShpLabs.Value(kx);
			//获取对应的对象
			if(aShapeTool->GetShape(aSLab,aShape)){
				pLayer->AddShape(aShape);
				//建立shape中face和图层的对应关系
				MapFace(aShape,pLayer);
			}
		}
	}

	return true;
}

kcStepLayer*  kcStepLayerMgr::AddLayer(const std::string &szName,int index)
{
	kcStepLayer *pLayer = NULL;
	stdStepLayerMap::iterator ite = aLayerMap_.find(szName);
	if(ite != aLayerMap_.end()){
		pLayer = (*ite).second;
	}else{
		pLayer = new kcStepLayer(szName,index);
		aLayerVec_.push_back(pLayer);
	}

	return pLayer;
}

// 记录每个Face和对应的图层序号
void  kcStepLayerMgr::	MapFace(const TopoDS_Shape &aShape,kcStepLayer *pLayer)
{
	int idx = pLayer->nIndex_;
	TopExp_Explorer exp;

	for(exp.Init(aShape,TopAbs_FACE);exp.More();exp.Next()){
		const TopoDS_Shape &aFace = exp.Current();
		//
		if(!aShapeLayerIdxMap_.Contains(aFace)){
			aShapeLayerIdxMap_.Add(aFace,idx);
		}
	}
}

void  kcStepLayerMgr::	Clear()
{
	aShapeLayerIdxMap_.Clear();
	aLayerMap_.clear();

	stdStepLayerVector::size_type ix,isize = aLayerVec_.size();
	for(ix = 0;ix < isize;ix ++){
		delete aLayerVec_[ix];
	}
	aLayerVec_.clear();
}

// 根据face，查找对应的图层信息
kcStepLayer*  kcStepLayerMgr::FindLayer(const TopoDS_Shape &aFace)
{
	kcStepLayer *pLayer = NULL;
	int nIdx = -1;
	if(aShapeLayerIdxMap_.FindFromKey(aFace,nIdx)){
		stdStepLayerVector::size_type ix,isize = aLayerVec_.size();
		for(ix = 0;ix < isize;ix ++){
			if(aLayerVec_[ix]->nIndex_ == nIdx){
				pLayer = aLayerVec_[ix];
				break;
			}
		}
	}
	return pLayer;
}