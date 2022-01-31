//////////////////////////////////////////////////////////////////////////
// 保存所有的图元信息.
#ifndef _KC_MODEL_H_
#define _KC_MODEL_H_

#include <vector>
#include <list>
#include <map>
#include <string>

#include <V3d_View.hxx>
#include <AIS_InteractiveObject.hxx>
#include <AIS_InteractiveContext.hxx>

#include "QxLibDef.h"

class CArchive;
class kiSnapMngr;
class kiUndoMgr;
class kcEntity;
class kcLayer;
class kiNewDelUndoItem;
class kiSelSet;
class kcHandleMgr;
class kuiModelTreeInterface;
class kcBasePlane;
class kcShapeAttribMgr;
class kcSnapMngr;

typedef std::list<kcEntity *>  kcEntityList;
typedef std::vector<kcEntity *> kcEntityArray;
typedef std::list<kcLayer *>	kcLayerList;
typedef std::list<kcBasePlane *>	kcBasePlaneList;

// 模型对象，记录和模型相关的所有信息
//
class QXLIB_API kcModel
{
public:
	//Model总是对一个AIS_Ctx的封装，和AIS_Ctx保持一一对应的关系
	kcModel(const Handle(AIS_InteractiveContext)& aCtx);
	~kcModel(void);

	//初始化
	BOOL					Initialize();
	//清空模型
	void					Clear();
	//释放所有
	void					Destroy();

	BOOL					CreateDefaultLayer();
	BOOL					CreateDefaultBasePlane(const Handle(V3d_View)& aView);

public:
	void					SetModelTreeInterface(kuiModelTreeInterface *pModelTreeInterface);
	kuiModelTreeInterface*	GetModelTreeInterface() const {  return _pModelTreeInterface; }

	//BOOL					SetAISContext(const Handle(AIS_InteractiveContext)& aCtx);
	const Handle(AIS_InteractiveContext)& GetAISContext() { return _hAISContext; }

	BOOL					SetActiveView(const Handle(V3d_View)& aView);
	const Handle(V3d_View)& GetActiveView() const { return aActiveView_; }

	kcHandleMgr*			GetHandleMgr() const { return _pHandleMgr; }
	kcShapeAttribMgr*		GetShapeAttribMgr() const { return _pShapeAttribMgr; }

	kiSelSet*			GetGlobalSelSet() const;

	void					SetSnapManager(kcSnapMngr *pMngr);
	kcSnapMngr*			GetSnapManager() const;

public://图层管理
	//创建和删除图层
	kcLayer*				CreateNewLayer();
	kcLayer*				CreateLayer(const char* szLayerName);
	BOOL					DeleteLayer(kcLayer *pLayer);
	kcLayer*				DeleteLayer(const char* szLayerName);
	kcLayer*				FindLayer(const char* szLayerName);
	BOOL					SetCurrLayer(const char* szLayerName);
	BOOL					SetCurrLayer(kcLayer *pLayer);
	kcLayer*				GetCurrLayer() const;

	const kcLayerList&	GetLayerList() const;
	int					GetLayerCount() const;

public:
	//添加和删除图元
	BOOL					AddEntity(kcEntity *pEntity,BOOL bUpdateView = FALSE);
	BOOL					AddEntity(const std::vector<kcEntity *>& aEnt,BOOL bUpdateView = FALSE);
	BOOL					AddEntity(kcEntity *pEntity,kcLayer *pLayer,BOOL bUpdateView = FALSE);
	BOOL					DelEntity(kcEntity *pEntity,BOOL bUpdateView = FALSE);
	kcEntity*			DelEntity(Handle(AIS_InteractiveObject)& aObj,BOOL bUpdateView = FALSE);
	kcEntity*			GetEntity(Handle(AIS_InteractiveObject)& aObj);

	void					GetAllEntity(kcEntityList& entList);
	void					GetAllEntity(int entType,kcEntityList& entList);

public://基准面管理
	BOOL					AddBasePlane(kcBasePlane *pBasePlane);
	BOOL					SetCurrentBasePlane(kcBasePlane *pBasePlane);
	kcBasePlane*			GetCurrentBasePlane() const;
	// 显示所以基准面的Grid
	void					ShowAllBasePlaneGrid(bool bShow);
	
public:
	//显示所有对象
	BOOL					ShowAll(BOOL bUpdateView = FALSE);
	// 显示特定类型的对象
	BOOL					FilterDisplay(int nTypeFlag,BOOL bUpdateView = FALSE);

public:
	//shape属性管理
	BOOL					AddShapeAttrib(const TopoDS_Shape &aS,const Quantity_Color &aCol);
	//查找颜色属性
	BOOL					FindShapeAttrib(const TopoDS_Shape &aS,Quantity_Color &aCol);
	//
	void					ClearAllShapeAttrib();

public:
	//和undo相关接口。开始和结束undo，在两个函数成对调用。在这两个函数间
	//AddEntity和DelEntity将自动由生成的Undo项保存。
	//
	BOOL					BeginUndo(const char *pszUndoName);
	BOOL					EndUndo();

public:
	kiUndoMgr*			GetUndoMgr();

	//计算所有捕捉点
	BOOL					CalcSnapPoint(kiSnapMngr *pSnapMgr);

public:
	// 保存和读取,dwFileVersion:是读取文件的版本
	BOOL					Read(CArchive& ar,UINT dwFileVersion);
	BOOL					Write(CArchive& ar);

protected:
	BOOL					ReadHandleMgr(CArchive& ar);
	BOOL					WriteHandleMgr(CArchive& ar);
	BOOL					ReadLayerData(CArchive& ar);
	BOOL					WriteLayerData(CArchive& ar);
	BOOL					ReadBasePlaneData(CArchive& ar);
	BOOL					WriteBasePlaneData(CArchive& ar);
	
protected://图层保存的数据
	kcHandleMgr					*_pHandleMgr;//handle管理
	kcLayerList					_layerList;//保存所有的图层信息
	kcLayer						*_pCurLayer;//当前图层

	kcShapeAttribMgr				*_pShapeAttribMgr;//对象属性管理

	kcBasePlaneList				_basePlaneList;//基准面列表
	kcBasePlane					*_pCurBasePlane;//当前获得的基准面

protected://Undo管理
	kiUndoMgr					*_pUndoMgr;//undo管理器
	kiNewDelUndoItem				*_pCurrUndoItem;//当前的UndoItem。

protected:
	//全局选择集管理对象,选择集是对AIS_CTX对象的封装，目前保持和其的一一对应关系
	kiSelSet						*pGlobalSelSet_;

protected:
	// 这里记录Doc对应的Context对象。
	Handle(AIS_InteractiveContext)	_hAISContext;
	kuiModelTreeInterface			*_pModelTreeInterface;
	Handle(V3d_View)				aActiveView_;//当前活动的view
	kcSnapMngr					*pSnapMngr_;// 这里记录的是外部管理器的指针

protected:
	UINT							dwFileVersion_;//读入文件的版本号，用于模型从文件中读取使用
};

inline kiUndoMgr*	kcModel::GetUndoMgr()
{
	ASSERT(_pUndoMgr);
	return _pUndoMgr;
}

#endif