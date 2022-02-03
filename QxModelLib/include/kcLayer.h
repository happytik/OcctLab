//////////////////////////////////////////////////////////////////////////
// 保存一个图层的所有图元
#ifndef _KC_LAYER_H_
#define _KC_LAYER_H_

#include <list>
#include <string>

#include "kcEntity.h"
#include "QxLibDef.h"

class CArchive;
class kiSnapMngr;
class kcModel;
class kcEntity;

#define  KCT_ENT_LAYER			0x10000  //避免和Entity的冲突

class QXLIB_API kcLayer
{
public:
	kcLayer();
	kcLayer(const char *pszName);
	~kcLayer(void);

	int						Initialize(kcModel *pModel);
	void					Clear();
	void					Destroy();

	kcModel*				GetModel() const { return pModel_; }

	//添加 删除图元,由于可能放入undo中，信息尽量不破坏。
	BOOL					AddEntity(kcEntity *pEntity);
	kcEntity*				DelEntity(kcEntity *pEntity);
	BOOL					FindEntity(kcEntity *pEntity);
	kcEntity*				FindEntity(const char *pszName);//根据名称获取

	void					GetAllEntity(kcEntityList& entList);
	void					GetAllEntity(int entType,kcEntityList& entList);

	//显示所有对象
	BOOL					ShowAll(BOOL bUpdateView = FALSE);
	// 显示特定类型的对象
	BOOL					FilterDisplay(int nTypeFlag,BOOL bUpdateView = FALSE);

	int						GetID() const { return nId_; }
	//获取图层名称
	const std::string&		GetLayerName() const { return sName_; }
	bool					HasName(const char *pszName);

public:
	void					SetColor(double r,double g,double b,BOOL bUpdateView = TRUE);
	void					SetLineStyle(eLineStyle eStyle,BOOL bUpdateView = TRUE);
	void					SetLineWidth(double width,BOOL bUpdateView = TRUE);

public:
	//计算自动捕捉点
	BOOL					CalcSnapPoint(kiSnapMngr *pSnapMgr);

public:
	// 读取
	BOOL						Read(CArchive& ar,kcModel *pModel,UINT dwFileVer);
	// 写入
	BOOL						Write(CArchive& ar);

protected:
	void						GenHandleAndName();
	// 读取entity
	BOOL						ReadEntity(CArchive& ar);

	//
	void						UpdateCurrentViewer();

protected:
	kcModel					*pModel_;
	int						nId_;//句柄,唯一的Id
	std::string				sName_;
	kcEntityList			aEntList_;

	friend class kcModel;
};

#endif
