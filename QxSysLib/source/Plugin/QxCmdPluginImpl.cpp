//
#include "stdafx.h"
#include "QxCmdPlugin.h"
#include "QxCmdPluginImpl.h"

QxCmdPluginImpl::QxCmdPluginImpl()
{
	pPlugin_ = NULL;
	eTopMenuID_ = QX_TOP_MENU_UNKNOWN; //默认未知
}

void QxCmdPluginImpl::SetPlugin(QxDynCmdPlugin *pPlugin)
{
	ASSERT(pPlugin);
	pPlugin_ = pPlugin;
}

// 添加两级或三级菜单的命令
bool QxCmdPluginImpl::AddCommandItem(int nLocalID,const char *pszTopMenu,const char *pszSubMenu,
		                                     int nMenuFlag,bool bCached)
{
	ASSERT(pPlugin_);
	BOOL bRet = pPlugin_->AddCommandItem(nLocalID,pszTopMenu,pszSubMenu,nMenuFlag,bCached);
	return (bRet ? true : false);
}

bool QxCmdPluginImpl::AddCommandItem(int nLocalID,const char *pszTopMenu,const char *pszSubMenu,
		                                     const char *pszSubMenu2,int nMenuFlag,
											 bool bCached)
{
	ASSERT(pPlugin_);
	BOOL bRet = pPlugin_->AddCommandItem(nLocalID,pszTopMenu,pszSubMenu,pszSubMenu2,nMenuFlag,bCached);
	return (bRet ? true : false);
}

bool QxCmdPluginImpl::AddCommandItem(int nLocalID,eAppTopMenuID topMenuId,const char *pszSubMenu,
		                                     int nMenuFlag,bool bCached)
{
	ASSERT(pPlugin_);
	BOOL bRet = pPlugin_->AddCommandItem(nLocalID,topMenuId,pszSubMenu,nMenuFlag,bCached);
	return (bRet ? true : false);
}

bool QxCmdPluginImpl::AddCommandItem(int nLocalID,eAppTopMenuID topMenuId,const char *pszSubMenu,
		                                     const char *pszSubMenu2,int nMenuFlag,
											 bool bCached)
{
	ASSERT(pPlugin_);
	BOOL bRet = pPlugin_->AddCommandItem(nLocalID,topMenuId,pszSubMenu,pszSubMenu2,nMenuFlag,bCached);
	return (bRet ? true : false);
}

bool QxCmdPluginImpl::DoAddCommandItem(int nLocalID,const char *pszSubMenu,int nMenuFlag,bool bCached)
{
	ASSERT(eTopMenuID_ != QX_TOP_MENU_UNKNOWN);
	return AddCommandItem(nLocalID,eTopMenuID_,pszSubMenu,nMenuFlag,bCached);
}

bool QxCmdPluginImpl::DoAddCommandItem(int nLocalID,const char *pszSubMenu,
		                            const char *pszSubMenu2,int nMenuFlag,bool bCached)
{
	ASSERT(eTopMenuID_ != QX_TOP_MENU_UNKNOWN);
	return AddCommandItem(nLocalID,eTopMenuID_,pszSubMenu,pszSubMenu2,nMenuFlag,bCached);
}