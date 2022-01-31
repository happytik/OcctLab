#include "StdAfx.h"
#include "kiCommand.h"
#include "QxCommanditem.h"

QxCommandItem::QxCommandItem(void)
{
	_pCommand = NULL;
	_bCached = false;
	_nCmdID = 0;
	_nLocalID = 0;
	_nMenuFlag = 0;
	_nLevel = 0;
	eTopMenuID_ = QX_TOP_MENU_UNKNOWN;
	_strMenu[0] = _strMenu[1] = _strMenu[2] = "";
}

QxCommandItem::QxCommandItem(int nLocadID,bool bCached)
{
	_pCommand = NULL;
	_nCmdID = 0;
	_nLocalID = nLocadID;
	_bCached = bCached;
	_nMenuFlag = 0;
	_nLevel = 0;
	eTopMenuID_ = QX_TOP_MENU_UNKNOWN;
	_strMenu[0] = _strMenu[1] = _strMenu[2] = "";
}

QxCommandItem::~QxCommandItem(void)
{
}

//设置菜单项文本
void QxCommandItem::SetMenuText(const char *pszText1,const char *pszText2)
{
	ASSERT(pszText1 && pszText2);
	_nLevel = 2;
	_strMenu[0] = pszText1;
	_strMenu[1] = pszText2;
	_strMenu[2] = "";
}

void QxCommandItem::SetMenuText(eAppTopMenuID topMenuId,const char *pszText2)
{
	ASSERT(pszText2);
	_nLevel = 2;
	eTopMenuID_ = topMenuId;
	_strMenu[0] = "";
	_strMenu[1] = pszText2;
	_strMenu[2] = "";
}


void QxCommandItem::SetMenuText(const char *pszText1,const char *pszText2,
									const char *pszText3)
{
	ASSERT(pszText1 && pszText2 && pszText3);
	_nLevel = 3;
	_strMenu[0] = pszText1;
	_strMenu[1] = pszText2;
	_strMenu[2] = pszText3;
}

void QxCommandItem::SetMenuText(eAppTopMenuID topMenuId,const char *pszText2,
							 const char *pszText3)
{
	ASSERT(pszText2 && pszText3);
	_nLevel = 3;
	eTopMenuID_ = topMenuId;
	_strMenu[0] = "";
	_strMenu[1] = pszText2;
	_strMenu[2] = pszText3;
}