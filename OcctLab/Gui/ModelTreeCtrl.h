#pragma once

#include <vector>
#include "kuiInterface.h"

// 每个item记录的信息.
//
class axItemData{
public:
	axItemData();
	axItemData(eItemType eType,void *pData);

	eItemType		_eType;//1:layer 2:entity
	void				*_pData;//对layer是kcLayer，对entity是kcEntity对象指针
	BOOL				_bVisible;//是否可见
	int				_wpType;//工作平面类型 0:xy 1:yz 2:zx
};

typedef std::vector<axItemData *> TItemDataArray;

// CModelTreeCtrl

class CModelTreeCtrl : public CTreeCtrl,public kuiModelTreeInterface
{
	DECLARE_DYNAMIC(CModelTreeCtrl)

public:
	CModelTreeCtrl();
	virtual ~CModelTreeCtrl();

public:
	// 接口的实现函数
	virtual bool				Initialize();//初始化
	virtual bool				Clear();//清除
	//
	virtual bool				AddLayer(const char *pszLayer,void *pData);
	virtual bool				RemoveLayer(const char *pszLayer);
	//
	virtual bool			AddEntity(const char *pszLayer,const char *pszName,void *pData);
	virtual bool			RemoveEntity(const char *pszLayer,const char *pszName);

	virtual void			BoldTreeItem(int itemType,const char *pszItemText);
	virtual void			UnBoldAllItem(int itemType);

	virtual void			ShowLayer(const char *pszName,BOOL bShow);
	virtual void			ShowEntity(const char *pszName,BOOL bShow);

	virtual bool			AddBasePlane(const char *pszName,void *pData);
	virtual bool			RemoveBasePlane(const char *pszName);

protected:
	HTREEITEM				FindChildItem(HTREEITEM hParentItem,const char *pszText);

protected:
	void						ShowEntity(HTREEITEM hItem,BOOL bShow);
	HTREEITEM				FindItemByName(int itemType,const char *pszName);
	CString					DoGetItemName(HTREEITEM hItem);
	void						DoBoldItem(HTREEITEM hItem);
	void						DoUnBoldItem(HTREEITEM hItem);
	void						DoShowLayer(BOOL bShow);

	HTREEITEM				FindRootItem(int itemType);
	bool						IsItemWithType(HTREEITEM hItem,eItemType type);
	bool						IsItemSelected(HTREEITEM hItem);
	
protected:
	HTREEITEM				m_hModelItem;//模型的主键
	HTREEITEM				m_hBasePlaneRootItem;//基准面主键

	TItemDataArray			m_itemDataArray;

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnNMClick(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
	afx_msg void OnTvnSelchanged(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMenuNewLayer();
	afx_msg void OnMenuDelLayer();
	afx_msg void OnMenuSetCurlayer();
	afx_msg void OnMenuLayerShow();
	afx_msg void OnMenuLayerHide();
	afx_msg void OnMenuEntityShow();
	afx_msg void OnMenuEntityHide();
	afx_msg void OnMenuSetCurcs();
	afx_msg void OnLayermenuSetColor();
	afx_msg void OnLayermenuLineSw();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnEntitymenuSetColor();
	afx_msg void OnEntitymenuLineSw();
	afx_msg void OnEntitymenuMoveToLayer();
	afx_msg void OnEntitymenuCopyToLayer();
};


