//////////////////////////////////////////////////////////////////////////
//
#ifndef _KUI_INTERFACE_H_
#define _KUI_INTERFACE_H_

class kcModel;

typedef enum _tagItemType{
	eUnknowItem = 0,
	eLayerItem,
	eEntityItem,
	eCoordSystemItem,
	eWorkPlaneItem,
	eBasePlaneItem
}eItemType;

// 模型管理树和model的管理接口
class kuiModelTreeInterface{
public:
	//
	virtual bool			Initialize() = 0;//初始化
	virtual bool			Clear() = 0;//清除
	//
	virtual bool			AddLayer(const char *pszLayer,void *pData) = 0;
	virtual bool			RemoveLayer(const char *pszLayer) = 0;
	//
	virtual bool			AddEntity(const char *pszLayer,const char *pszName,void *pData) = 0;
	virtual bool			RemoveEntity(const char *pszLayer,const char *pszName) = 0;

	virtual bool			AddBasePlane(const char *pszName,void *pData) = 0;
	virtual bool			RemoveBasePlane(const char *pszName) = 0;

	virtual void			BoldTreeItem(int itemType,const char *pszItemText) = 0;
	virtual void			UnBoldAllItem(int itemType) = 0;

	virtual void			ShowLayer(const char *pszName,BOOL bShow) = 0;
	virtual void			ShowEntity(const char *pszName,BOOL bShow) = 0;

	void					SetModel(kcModel *pModel) { m_pModel = pModel;}

protected:
	kcModel					*m_pModel;
};

#endif