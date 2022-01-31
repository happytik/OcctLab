//////////////////////////////////////////////////////////////////////////
//
#ifndef _QX_COMMAND_ITEM_H_
#define _QX_COMMAND_ITEM_H_

#include <string>
#include "QxLibDef.h"

#define KUI_MENU_BEFORE_SEP		0x01 //菜单前加分隔符
#define KUI_MENU_AFTER_SEP			0x02 //菜单后加分隔符

class kiCommand;

//顶级菜单的定义，避免直接使用菜单的文本
typedef enum QxAppTopMenuID{
	QX_TOP_MENU_CURVE = 0,  //曲线菜单
	QX_TOP_MENU_SURFACE,
	QX_TOP_MENU_SOLID,
	QX_TOP_MENU_UNKNOWN,  //未知
}eAppTopMenuID;

class QXLIB_API QxCommandItem
{
public:
	QxCommandItem(void);
	QxCommandItem(int nLocadID,bool bCached);
	~QxCommandItem(void);

	//设置菜单项文本
	void						SetMenuText(const char *pszText1,const char *pszText2);
	void						SetMenuText(const char *pszText1,const char *pszText2,
										const char *pszText3);

	void						SetMenuText(eAppTopMenuID topMenuId,const char *pszText2);
	void						SetMenuText(eAppTopMenuID topMenuId,const char *pszText2,
										const char *pszText3);

public:
	kiCommand				*_pCommand;//命令对象
	int						_nCmdID;//menu对应的ID.
	int						_nLocalID;//插件内的局部ID
	bool						_bCached;//命令是否缓存
	//菜单信息
	int						_nMenuFlag;//菜单标识.
	int						_nLevel;//有几级菜单。
	eAppTopMenuID				eTopMenuID_;  //顶级菜单ID，如果设置，使用该菜单，否则使用下面的_strMenu[0]
	std::string				_strMenu[3];//最多三级菜单.
};

#endif