#pragma once

#include <string>
#include <vector>
#include <map>
#include <NCollection_IndexedDataMap.hxx>
#include <TopTools_ShapeMapHasher.hxx>
#include <TopTools_ListOfShape.hxx>
#include <TDocStd_Document.hxx>

class kcStepMbStringMgr;
class kcStepLayer;
typedef std::vector<kcStepLayer *> stdStepLayerVector;
typedef std::map<std::string,kcStepLayer *> stdStepLayerMap;
typedef NCollection_IndexedDataMap<TopoDS_Shape,int,TopTools_ShapeMapHasher> ShapeLayerIdxMap;


//记录一个图层信息
class kcStepLayer{
public:
	kcStepLayer();
	kcStepLayer(const std::string &szName,int idx);

	void					AddShape(const TopoDS_Shape &aS);

public:
	std::string			szName_;//图层名称
	int					nIndex_;//序号
	TopTools_ListOfShape	aShapeList_;//图层包含的shape列表
};

class kcStepLayerMgr
{
public:
	kcStepLayerMgr(void);
	kcStepLayerMgr(kcStepMbStringMgr *pMbStrMgr);
	~kcStepLayerMgr(void);

	bool					ReadLayers(const Handle(TDocStd_Document) &aDoc);
	void					Clear();

	kcStepLayer*			FindLayer(const TopoDS_Shape &aFace);

protected:
	kcStepLayer*			AddLayer(const std::string &szName,int index);
	//
	void					MapFace(const TopoDS_Shape &aShape,kcStepLayer *pLayer);

protected:
	stdStepLayerVector	aLayerVec_;//图层列表
	stdStepLayerMap		aLayerMap_;//
	ShapeLayerIdxMap		aShapeLayerIdxMap_;

	kcStepMbStringMgr		*pMbStrMgr_;//名称管理
};

