// gui\ModelTreeCtrl.cpp : 实现文件
//

#include "stdafx.h"
#include "..\OcctLab.h"
#include "kcEntity.h"
#include "kcLayer.h"
#include "kcModel.h"
#include "kvCoordSystem.h"
#include "kvCoordSystemMgr.h"
#include "kiSelSet.h"
#include "LineStyleAndWidthDlg.h"
#include "OxModel.h"
#include "ModelTreeCtrl.h"


// CModelTreeCtrl

IMPLEMENT_DYNAMIC(CModelTreeCtrl, CTreeCtrl)
CModelTreeCtrl::CModelTreeCtrl()
{
	m_hModelItem = NULL;
	m_hBasePlaneRootItem = NULL;
}

CModelTreeCtrl::~CModelTreeCtrl()
{
}


BEGIN_MESSAGE_MAP(CModelTreeCtrl, CTreeCtrl)
	ON_NOTIFY_REFLECT(NM_CLICK, OnNMClick)
	ON_WM_CONTEXTMENU()
	ON_NOTIFY_REFLECT(TVN_SELCHANGED, OnTvnSelchanged)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnNMDblclk)
	ON_WM_RBUTTONDOWN()
	ON_COMMAND(ID_MENU_NEW_LAYER, &CModelTreeCtrl::OnMenuNewLayer)
	ON_COMMAND(ID_MENU_DEL_LAYER, &CModelTreeCtrl::OnMenuDelLayer)
	ON_COMMAND(ID_MENU_SET_CURLAYER, &CModelTreeCtrl::OnMenuSetCurlayer)
	ON_COMMAND(ID_MENU_LAYER_SHOW, &CModelTreeCtrl::OnMenuLayerShow)
	ON_COMMAND(ID_MENU_LAYER_HIDE, &CModelTreeCtrl::OnMenuLayerHide)
	ON_COMMAND(ID_MENU_ENTITY_SHOW, &CModelTreeCtrl::OnMenuEntityShow)
	ON_COMMAND(ID_MENU_ENTITY_HIDE, &CModelTreeCtrl::OnMenuEntityHide)
	ON_COMMAND(ID_MENU_SET_CURCS, &CModelTreeCtrl::OnMenuSetCurcs)
	ON_COMMAND(ID_LAYERMENU_SET_COLOR, &CModelTreeCtrl::OnLayermenuSetColor)
	ON_COMMAND(ID_LAYERMENU_LINE_SW, &CModelTreeCtrl::OnLayermenuLineSw)
	ON_WM_RBUTTONUP()
	ON_COMMAND(ID_ENTITYMENU_SET_COLOR, &CModelTreeCtrl::OnEntitymenuSetColor)
	ON_COMMAND(ID_ENTITYMENU_LINE_SW, &CModelTreeCtrl::OnEntitymenuLineSw)
	ON_COMMAND(ID_ENTITYMENU_MOVE_TO_LAYER, &CModelTreeCtrl::OnEntitymenuMoveToLayer)
	ON_COMMAND(ID_ENTITYMENU_COPY_TO_LAYER, &CModelTreeCtrl::OnEntitymenuCopyToLayer)
END_MESSAGE_MAP()

//使tree上一项变为粗体
void	CModelTreeCtrl::BoldTreeItem(int itemType,const char *pszItemText)
{
	HTREEITEM hItem = NULL;

	hItem = FindItemByName(itemType,pszItemText);
	if(hItem){
		DoBoldItem(hItem);
	}
}

// 将一个类型下的所有item去除重点显示
// 例如：所有图层或图层下的所有对象都去重点显示
//
void	CModelTreeCtrl::UnBoldAllItem(int itemType)
{
	HTREEITEM hRootItem = NULL,hItem = NULL,hSubItem = NULL;

	hRootItem = FindRootItem(itemType);
	if(!hRootItem){
		return;
	}
	
	//这个都是根节点下，就一级节点
	if(itemType == eLayerItem || itemType == eCoordSystemItem || itemType == eBasePlaneItem){
		hItem = this->GetChildItem(hRootItem);
		while(hItem){
			DoUnBoldItem(hItem);

			hItem = this->GetNextSiblingItem(hItem);
		}
	}else if(itemType == eEntityItem || itemType == eWorkPlaneItem){//这个有多级节点
		hItem = this->GetChildItem(hRootItem);
		while(hItem){//all layer or all cs
			hSubItem = this->GetChildItem(hItem);
			while(hSubItem){
				DoUnBoldItem(hSubItem);

				hSubItem = this->GetNextSiblingItem(hSubItem);
			}

			hItem = this->GetNextSiblingItem(hItem);
		}
	}
}

HTREEITEM  CModelTreeCtrl::FindRootItem(int itemType)
{
	if(itemType == eLayerItem || itemType == eEntityItem)
		return m_hModelItem;

	if(itemType == eBasePlaneItem)
		return m_hBasePlaneRootItem;

	return NULL;
}

bool  CModelTreeCtrl::IsItemWithType(HTREEITEM hItem,eItemType type)
{
	axItemData *pData = (axItemData *)GetItemData(hItem);
	if(pData && pData->_eType == type)
		return true;
	return false;
}

bool  CModelTreeCtrl::IsItemSelected(HTREEITEM hItem)
{
	if(hItem){
		UINT itemState = this->GetItemState(hItem,TVIS_SELECTED);
		if(itemState & TVIS_SELECTED)
			return true;
	}
	return false;
}

// 当对象被选中时，字体加粗，并设置对应的图标
void  CModelTreeCtrl::DoBoldItem(HTREEITEM hItem)
{
	if(hItem){
		this->SetItemState(hItem,TVIS_BOLD,TVIS_BOLD);
		//
		axItemData *pData = (axItemData *)GetItemData(hItem);
		if(pData){
			if(IsItemWithType(hItem,eEntityItem)){
				if(pData->_bVisible){
					this->SetItemImage(hItem,4,4);
				}else{
					this->SetItemImage(hItem,5,5);
				}
			}else if(IsItemWithType(hItem,eLayerItem)){
				if(pData->_bVisible){
					this->SetItemImage(hItem,1,1);
				}else{
					this->SetItemImage(hItem,2,2);
				}
			}
		}
	}
}

// 当对象去选时，调用该函数的处理
void	CModelTreeCtrl::DoUnBoldItem(HTREEITEM hItem)
{
	UINT itemState = 0;

	if(hItem){
		itemState = this->GetItemState(hItem,TVIS_BOLD);
		if(itemState & TVIS_BOLD){
			itemState &= ~TVIS_BOLD;
			this->SetItemState(hItem,itemState,TVIS_BOLD);
			axItemData *pData = (axItemData *)GetItemData(hItem);
			if(pData){
				if(IsItemWithType(hItem,eEntityItem)){
					if(pData->_bVisible){
						this->SetItemImage(hItem,3,3);
					}else{
						this->SetItemImage(hItem,5,5);
					}
				}else if(IsItemWithType(hItem,eLayerItem)){
					if(pData->_bVisible){
						this->SetItemImage(hItem,0,0);
					}else{
						this->SetItemImage(hItem,2,2);
					}
				}
			}
		}
	}
}

void  CModelTreeCtrl::ShowLayer(const char *pszName,BOOL bShow)
{
	
}

void	CModelTreeCtrl::ShowEntity(const char *pszName,BOOL bShow)
{
	HTREEITEM hItem = FindItemByName(eEntityItem,pszName);
	if(hItem){
		ShowEntity(hItem,bShow);
	}
}

void	CModelTreeCtrl::ShowEntity(HTREEITEM hItem,BOOL bShow)
{
	ASSERT(hItem);
	axItemData *pData = NULL;
	CString szText;

	pData = (axItemData *)GetItemData(hItem);
	if(pData){
		if(pData->_bVisible){
			if(!bShow){
				szText = DoGetItemName(hItem);
				szText += " [-]";
				SetItemText(hItem,szText);
				pData->_bVisible = FALSE;
				//
				if(pData->_eType == eLayerItem){
					this->SetItemImage(hItem,2,2);
				}else if(pData->_eType == eEntityItem){
					this->SetItemImage(hItem,5,5);
				}
			}
		}else{
			if(bShow){
				szText = DoGetItemName(hItem);
				SetItemText(hItem,szText);
				pData->_bVisible = TRUE;
				if(pData->_eType == eLayerItem){
					this->SetItemImage(hItem,0,0);
				}else if(pData->_eType == eEntityItem){
					this->SetItemImage(hItem,3,3);
				}
			}
		}
	}
}

//根据名称获取Tree上的节点
//
HTREEITEM	CModelTreeCtrl::FindItemByName(int itemType,const char *pszName)
{
	HTREEITEM hRootItem = NULL,hItem = NULL,hSubItem = NULL,hFoundItem = NULL;
	CString szText;

	hRootItem = FindRootItem(itemType);
	if(!hRootItem)
		return NULL;

	hItem = GetChildItem(hRootItem);
	while(hItem){
		if(itemType == eLayerItem || itemType == eCoordSystemItem || itemType == eBasePlaneItem){
			szText = DoGetItemName(hItem);
			if(szText.Compare(pszName) == 0){
				hFoundItem = hItem;
				break;
			}
		}else if(itemType == eEntityItem || itemType == eWorkPlaneItem){
			hSubItem = GetChildItem(hItem);
			while(hSubItem){
				szText = DoGetItemName(hSubItem);
				if(szText.Compare(pszName) == 0){
					hFoundItem = hSubItem;
					break;
				}

				hSubItem = GetNextSiblingItem(hSubItem);
			}
		}

		if(hFoundItem) break;

		hItem = GetNextSiblingItem(hItem);
	}
		

	return hFoundItem;
}

CString	CModelTreeCtrl::DoGetItemName(HTREEITEM hItem)
{
	CString szText;
	int ipos = -1;

	szText = GetItemText(hItem);
	if(!szText.IsEmpty()){
		ipos = szText.Find('[',0);
		if(ipos >= 0)
			szText = szText.Left(ipos - 1);//skip space
	}

	return szText;
}

//////////////////////////////////////////////////////////////////////////
//
//
//初始化
bool  CModelTreeCtrl::Initialize()
{
	if(!GetSafeHwnd())
		return false;

	//清除所有旧的
	DeleteAllItems();
	
	//
	m_hModelItem = InsertItem("模型管理",TVI_ROOT,TVI_LAST);
	m_hBasePlaneRootItem = InsertItem("基准面管理",TVI_ROOT,TVI_LAST);

	return true;
}

bool  CModelTreeCtrl::Clear()
{
	//清除所有旧的
	DeleteAllItems();

	m_hModelItem = NULL;
	m_hBasePlaneRootItem = NULL;

	return true;
}

//
bool	CModelTreeCtrl::AddLayer(const char *pszLayer,void *pData)
{
	if(m_hModelItem)
	{
		BOOL bHasChild = ItemHasChildren(m_hModelItem);
		HTREEITEM hItem = InsertItem(pszLayer,0,0,m_hModelItem,TVI_LAST);
		if(hItem)
		{
			axItemData *pItemData = new axItemData(eLayerItem,pData);
			SetItemData(hItem,(DWORD_PTR)pItemData);
			m_itemDataArray.push_back(pItemData);

			if(!bHasChild)
				Expand(m_hModelItem,TVE_EXPAND);
		}
	}
	return true;
}

bool	CModelTreeCtrl::RemoveLayer(const char *pszLayer)
{
	HTREEITEM hLayerItem = FindChildItem(m_hModelItem,pszLayer);
	if(!hLayerItem)
		return false;

	if(ItemHasChildren(hLayerItem))
	{
		HTREEITEM hNextItem;
		HTREEITEM hChildItem = GetChildItem(hLayerItem);

		while (hChildItem != NULL)
		{
			hNextItem = GetNextItem(hChildItem, TVGN_NEXT);
			DeleteItem(hChildItem);
			hChildItem = hNextItem;
		}
	}
	DeleteItem(hLayerItem);

	return true;
}

HTREEITEM	CModelTreeCtrl::FindChildItem(HTREEITEM hParentItem,const char *pszText)
{
	if(hParentItem == NULL)
		return NULL;

	HTREEITEM hFndItem = NULL;
	HTREEITEM hChildItem;
	CString szText;
	hChildItem = GetChildItem(hParentItem);
	while(hChildItem)
	{
		szText = GetItemText(hChildItem);
		if(szText.CompareNoCase(pszText) == 0)
		{
			hFndItem = hChildItem;
			break;
		}
		hChildItem = GetNextItem(hChildItem,TVGN_NEXT);
	}

	return hFndItem;
}

//
bool	CModelTreeCtrl::AddEntity(const char *pszLayer,const char *pszName,void *pData)
{
	HTREEITEM hLayerItem = FindChildItem(m_hModelItem,pszLayer);
	if(hLayerItem == NULL)
		return false;

	BOOL bHasChild = ItemHasChildren(hLayerItem);
	HTREEITEM hItem = InsertItem(pszName,3,4,hLayerItem);
	if(hItem)
	{
		axItemData *pItemData = new axItemData(eEntityItem,pData);
		SetItemData(hItem,(DWORD_PTR)pItemData);
		//m_wndTreeCtrl.SetItemState(hItem,TVIS_BOLD,TVIS_BOLD);
		m_itemDataArray.push_back(pItemData);

		if(!bHasChild)
			Expand(hLayerItem,TVE_EXPAND);
	}

	return true;
}

bool	CModelTreeCtrl::RemoveEntity(const char *pszLayer,const char *pszName)
{
	HTREEITEM hLayerItem = FindChildItem(m_hModelItem,pszLayer);
	if(hLayerItem == NULL)
		return false;

	HTREEITEM hItem = FindChildItem(hLayerItem,pszName);
	if(!hItem)
		return false;

	DeleteItem(hItem);

	return true;
}

bool  CModelTreeCtrl::AddBasePlane(const char *pszName,void *pData)
{
	if(m_hBasePlaneRootItem){
		BOOL bHasChild = ItemHasChildren(m_hBasePlaneRootItem);
		HTREEITEM hItem = InsertItem(pszName,m_hBasePlaneRootItem,TVI_LAST);
		if(hItem){
			axItemData *pItemData = new axItemData(eBasePlaneItem,pData);
			SetItemData(hItem,(DWORD_PTR)pItemData);
			m_itemDataArray.push_back(pItemData);

			if(!bHasChild)
				Expand(m_hBasePlaneRootItem,TVE_EXPAND);
		}
	}
	return true;
}

bool  CModelTreeCtrl::RemoveBasePlane(const char *pszName)
{
	HTREEITEM hItem = FindChildItem(m_hBasePlaneRootItem,pszName);
	if(!hItem)
		return false;

	DeleteItem(hItem);

	return true;
}


///////////////////////////////////////////////////////////
// CModelTreeCtrl 消息处理程序


void CModelTreeCtrl::OnNMClick(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
}

void CModelTreeCtrl::OnTvnSelchanged(NMHDR *pNMHDR, LRESULT *pResult)
{
	*pResult = 0;
	
	kiSelSet *pSelSet = m_pModel->GetGlobalSelSet();
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	// TODO: Add your control notification handler code here
	if (pNMTreeView->itemNew.state & TVIS_SELECTED)
	{
		KTRACE("\n CModelTreeCtrl::OnTvnSelchanged(NMHDR *pNMHDR, LRESULT *pResult)");

		//新的项
		HTREEITEM hItem = pNMTreeView->itemNew.hItem;
		axItemData *pItemData = NULL;
		if(hItem)
		{
			pItemData = (axItemData *)GetItemData(hItem);
			if(pItemData && pItemData->_eType == eEntityItem)
			{
				kcEntity *pEntity = (kcEntity *)pItemData->_pData;
				Handle(AIS_InteractiveObject) aObj = pEntity->DisplayObj();
				Handle(AIS_InteractiveContext) aCtx = m_pModel->GetAISContext();
				//aCtx->HilightWithColor(aObj,Quantity_NOC_GREEN);
				//aCtx->SetCurrentObject(aObj);
				if(pSelSet)
					pSelSet->AddSelected(aObj,TRUE);
				else
					aCtx->SetSelected(aObj,Standard_True);
				//
				DoBoldItem(hItem);
			}
		}
		
		//旧的项
		hItem = pNMTreeView->itemOld.hItem;
		if(hItem)
		{
			pItemData = (axItemData *)GetItemData(hItem);
			if(pItemData && pItemData->_eType == eEntityItem)
			{
				kcEntity *pEntity = (kcEntity *)pItemData->_pData;
				Handle(AIS_InteractiveObject) aObj = pEntity->DisplayObj();
				Handle(AIS_InteractiveContext) aCtx = m_pModel->GetAISContext();
				if(pSelSet)
					pSelSet->RemoveSelected(aObj,TRUE);
				else
					aCtx->Unhilight(aObj,Standard_True);

				DoUnBoldItem(hItem);
			}
		}
	}
	
}

// 双击
void CModelTreeCtrl::OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;

	HTREEITEM hItem = GetSelectedItem();
	if(hItem){
		axItemData *pItemData = (axItemData *)GetItemData(hItem);
		// 双击设置工作平面
		if(pItemData){
			if(pItemData->_eType == eBasePlaneItem){
				kcBasePlane *pBasePlane = (kcBasePlane *)pItemData->_pData;
				m_pModel->SetCurrentBasePlane(pBasePlane);
			}

			//图层
			if(pItemData->_eType == eLayerItem){
				kcLayer *pLayer = (kcLayer *)pItemData->_pData;
				m_pModel->SetCurrLayer(pLayer);
			}
		}
	}
}

void CModelTreeCtrl::OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/)
{

}

void CModelTreeCtrl::OnRButtonDown(UINT nFlags, CPoint point)
{
	//弹出对应的菜单
	HTREEITEM hItem = GetSelectedItem();
	if(hItem){
		CPoint scrPoint;
		GetCursorPos(&scrPoint);
		
		axItemData *pData = (axItemData *)GetItemData(hItem);
		if(pData){
			if(pData->_eType == eLayerItem){
				CMenu menu;
				if(menu.LoadMenu(IDR_MENU_MODELTREE)){
					CMenu *pPopMenu = menu.GetSubMenu(1);
					if(pPopMenu){
						pPopMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON,scrPoint.x,scrPoint.y,this);
					}
				}
			}else if(pData->_eType == eEntityItem){
				CMenu menu;
				if(menu.LoadMenu(IDR_MENU_MODELTREE)){
					CMenu *pPopMenu = menu.GetSubMenu(2);
					if(pPopMenu){
						//判断是显示还是隐藏
						if(pData->_bVisible){
							pPopMenu->RemoveMenu(0,MF_BYPOSITION);
						}else{
							pPopMenu->RemoveMenu(1,MF_BYPOSITION);
						}
						pPopMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON,scrPoint.x,scrPoint.y,this);
					}
				}
			}
		}
	}
	
	//CTreeCtrl::OnRButtonDown(nFlags, point);
}

void CModelTreeCtrl::OnRButtonUp(UINT nFlags, CPoint point)
{
	CTreeCtrl::OnRButtonUp(nFlags, point);


}

// 新建菜单
void CModelTreeCtrl::OnMenuNewLayer()
{
	ASSERT(m_pModel);
	if(m_pModel){
		m_pModel->CreateNewLayer();
	}
}


void CModelTreeCtrl::OnMenuDelLayer()
{
	// TODO: 在此添加命令处理程序代码
}


void CModelTreeCtrl::OnMenuSetCurlayer()
{
	HTREEITEM hItem = GetSelectedItem();
	if(hItem){
		axItemData *pItemData = (axItemData *)GetItemData(hItem);
		if(pItemData->_eType == eLayerItem){
			kcLayer *pLayer = (kcLayer *)pItemData->_pData;
			if(m_pModel){
				m_pModel->SetCurrLayer(pLayer);
			}
		}
	}
}


void CModelTreeCtrl::OnMenuLayerShow()
{
	DoShowLayer(TRUE);
}


void  CModelTreeCtrl::OnMenuLayerHide()
{
	DoShowLayer(FALSE);
}

void  CModelTreeCtrl::DoShowLayer(BOOL bShow)
{
	HTREEITEM hItem = GetSelectedItem();
	if(hItem){
		axItemData *pItemData = (axItemData *)GetItemData(hItem);
		if(pItemData->_eType == eLayerItem){
			axItemData *pSubItemData = NULL;
			HTREEITEM hChildItem = GetChildItem(hItem);
			while(hChildItem){
				pSubItemData = (axItemData *)GetItemData(hChildItem);
				if(pSubItemData->_bVisible != bShow){
					kcEntity *pEntity = (kcEntity *)pSubItemData->_pData;
					if(bShow){
						pEntity->Display(true,FALSE);
						ShowEntity(hChildItem,TRUE);
					}else{
						pEntity->Display(false,FALSE);
						ShowEntity(hChildItem,FALSE);
					}
				}
				hChildItem = GetNextItem(hChildItem,TVGN_NEXT);
			}
			//统一更新
			Handle(AIS_InteractiveContext) aCtx = m_pModel->GetAISContext();
			if(!aCtx.IsNull()){
				aCtx->UpdateCurrentViewer();
			}
		}
	}
}

void CModelTreeCtrl::OnMenuEntityShow()
{
	HTREEITEM hItem = GetSelectedItem();
	if(hItem){
		axItemData *pItemData = (axItemData *)GetItemData(hItem);
		if(!pItemData->_bVisible){
			kcEntity *pEntity = (kcEntity *)pItemData->_pData;
			pEntity->Display(true,TRUE);
			ShowEntity(hItem,TRUE);	
		}
	}
}


void CModelTreeCtrl::OnMenuEntityHide()
{
	HTREEITEM hItem = GetSelectedItem();
	if(hItem){
		axItemData *pItemData = (axItemData *)GetItemData(hItem);
		if(pItemData->_bVisible){
			kcEntity *pEntity = (kcEntity *)pItemData->_pData;
			pEntity->Display(false,TRUE);
			ShowEntity(hItem,FALSE);	
		}
	}
}

void CModelTreeCtrl::OnMenuSetCurcs()
{
	// TODO: 在此添加命令处理程序代码
}

void CModelTreeCtrl::OnLayermenuSetColor()
{
	HTREEITEM hItem = GetSelectedItem();
	if(hItem){
		axItemData *pItemData = (axItemData *)GetItemData(hItem);
		if(pItemData->_eType == eLayerItem && ItemHasChildren(hItem)){
			CColorDialog dlg;
			if(dlg.DoModal() == IDOK){
				COLORREF col = dlg.GetColor();
				double r = GetRValue(col) / 255.0,g = GetGValue(col) / 255.0,b = GetBValue(col) / 255.0;
				kcLayer *pLayer = (kcLayer *)pItemData->_pData;
				pLayer->SetColor(r,g,b,TRUE);
			}
		}
	}
}

void CModelTreeCtrl::OnEntitymenuSetColor()
{
	HTREEITEM hItem = GetSelectedItem();
	if(hItem){
		axItemData *pItemData = (axItemData *)GetItemData(hItem);
		if(pItemData->_eType == eEntityItem){
			CColorDialog dlg;
			if(dlg.DoModal() == IDOK){
				COLORREF col = dlg.GetColor();
				double r = GetRValue(col) / 255.0,g = GetGValue(col) / 255.0,b = GetBValue(col) / 255.0;
				//
				kcEntity *pEnt = (kcEntity *)pItemData->_pData;
				pEnt->SetColor(r,g,b,TRUE);
			}
		}
	}
}


void CModelTreeCtrl::OnLayermenuLineSw()
{
	HTREEITEM hItem = GetSelectedItem();
	if(hItem){
		axItemData *pItemData = (axItemData *)GetItemData(hItem);
		if(pItemData->_eType == eLayerItem && ItemHasChildren(hItem)){
			CLineStyleAndWidthDlg dlg;
			if(dlg.DoModal() == IDOK){
				kcLayer *pLayer = (kcLayer *)pItemData->_pData;
				//
				pLayer->SetLineStyle((eLineStyle)dlg.m_nLineStyle,FALSE);
				pLayer->SetLineWidth(dlg.m_dLineWidth,TRUE);
			}
		}
	}
}

void CModelTreeCtrl::OnEntitymenuLineSw()
{
	HTREEITEM hItem = GetSelectedItem();
	if(hItem){
		axItemData *pItemData = (axItemData *)GetItemData(hItem);
		if(pItemData->_eType == eEntityItem){
			CLineStyleAndWidthDlg dlg;
			if(dlg.DoModal() == IDOK){
				//
				kcEntity *pEnt = (kcEntity *)pItemData->_pData;
				pEnt->SetLineStyle((eLineStyle)dlg.m_nLineStyle,FALSE);
				pEnt->SetLineWidth(dlg.m_dLineWidth,TRUE);
			}
		}
	}
}

//
void CModelTreeCtrl::OnEntitymenuMoveToLayer()
{
	ASSERT(m_pModel);
	if(m_pModel->GetLayerCount() < 2){
		AfxMessageBox("只有一个图层，无法移动");
		return;
	}
	HTREEITEM hItem = GetSelectedItem();
	if(!hItem)
		return;

	axItemData *pItemData = (axItemData *)GetItemData(hItem);
	if(pItemData->_eType != eEntityItem){
		return;
	}

	kcEntity *pEnt = (kcEntity *)pItemData->_pData;
	CWnd *pWnd = AfxGetMainWnd();
	kcLayer *pLayer = PickLayerByDlg(pWnd,m_pModel);
	if(!pLayer){
		return;
	}
	kcLayer *pEntLayer = pEnt->GetLayer();
	if(pEntLayer == pLayer){
		return;
	}

	//图层间移动
	pEntLayer->DelEntity(pEnt);
	pLayer->AddEntity(pEnt);	
}


void CModelTreeCtrl::OnEntitymenuCopyToLayer()
{
	// TODO: 在此添加命令处理程序代码
}
