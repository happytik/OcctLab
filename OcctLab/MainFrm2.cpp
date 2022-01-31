////////////////////////////////////////////////////////
// 一些COM接口的测试功能

#include "stdafx.h"
#include "QxCommandItem.h"
#include "MainFrm.h"

typedef struct TopMenuText{
	eAppTopMenuID id;
	char		text[64];
}TopMenuText_s;
TopMenuText_s s_topMenuTextAry[] = {
	{QX_TOP_MENU_CURVE, "曲线(&C)"},
	{QX_TOP_MENU_SURFACE, "曲面(&S)"},
	{QX_TOP_MENU_SOLID, "实体(&D)"},
	{QX_TOP_MENU_UNKNOWN, ""}
};

const char* CMainFrame::DoGetTopMenuText(QxCommandItem *pItem)
{
	ASSERT(pItem);
	if(pItem->eTopMenuID_ != QX_TOP_MENU_UNKNOWN){
		int ix = 0;
		while(s_topMenuTextAry[ix].id != QX_TOP_MENU_UNKNOWN){
			if(s_topMenuTextAry[ix].id == pItem->eTopMenuID_){
				return s_topMenuTextAry[ix].text;
			}
			ix ++;
		}
		return NULL;
	}
	if(pItem->_strMenu[0].empty()){
		ASSERT(FALSE);
		return NULL;
	}
	return pItem->_strMenu[0].c_str();
}

void ComTest_Add()
{

}