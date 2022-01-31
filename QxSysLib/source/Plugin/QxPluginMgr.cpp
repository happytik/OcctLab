#include "StdAfx.h"
#include "QxCommandItem.h"
#include "kiCommand.h"
#include "QxCmdPlugin.h"
#include "QxPluginMgr.h"

QxPluginMgr::QxPluginMgr(void)
{
	_uiCurrentID = KI_CMD_ID_BEGIN;
}

QxPluginMgr::~QxPluginMgr(void)
{
	Destroy();
}

int  QxPluginMgr::Initialize()
{
	return 1;
}

void  QxPluginMgr::Destroy()
{
	//删除动态库对象
	QxCmdPlugin *pPlugin = NULL;
	size_t ix,isize = aPluginArray_.size();
	for(ix = 0;ix < isize;ix ++){
		pPlugin = aPluginArray_.at(ix);
		pPlugin->Release();
		delete pPlugin;
	}
	aPluginArray_.clear();

	//最后释放动态库，因为前面要用到
	QxDynPluginData *pData = NULL;
	isize = aDynPluginArray_.size();
	for(ix = 0;ix < isize;ix ++){
		pData = aDynPluginArray_[ix];
		pData->FreePluginDll();
		delete pData;
	}
	aDynPluginArray_.clear();
}

//加载一个插件,这里主要加载命令的基本信息
//
BOOL  QxPluginMgr::LoadPlugin(QxCmdPlugin *pPlugin)
{
	if(!pPlugin)
		return FALSE;

	//插件名称应当不重复
	if(FindPlugin(pPlugin->GetName()) != NULL){
		ASSERT(FALSE);
		return FALSE;
	}

	if(!pPlugin->Initialize())
		return FALSE;

	//插件生成命令对应的ID
	pPlugin->GenCommandID(_uiCurrentID);
	_uiCurrentID += pPlugin->GetReserveNum();//下一个可用值

	//将所有的item，记录到一个数组中，方便菜单创建
	int ix,iCount = pPlugin->GetItemCount();
	QxCommandItem *pCmdItem = NULL;
	for(ix = 0;ix < iCount;ix ++){
		pCmdItem = pPlugin->GetCmdItem(ix);
		_cmdItemArray.push_back(pCmdItem);
	}

	//插件记录到数组中
	aPluginArray_.push_back(pPlugin);

	return TRUE;
}

// 卸载一个插件,暂不实现
//
BOOL	QxPluginMgr::UnloadPlugin(QxCmdPlugin *pPlugin)
{
	return TRUE;
}

QxCmdPlugin*	QxPluginMgr::FindPlugin(const std::string& strName)
{
	int ix,isize = (int)aPluginArray_.size();
	for(ix = 0;ix < isize;ix ++){
		const std::string& strTempName = aPluginArray_[ix]->GetName();
		ASSERT(!strTempName.empty());
		if(strTempName == strName)
			return aPluginArray_[ix];
	}
	return NULL;
}

//根据命令id获取命令
kiCommand*  QxPluginMgr::GetCommand(int nCmdID)
{
	kiCommand *pCommand = NULL;
	kuiPluginArray::size_type ix,isize = aPluginArray_.size();

	for(ix = 0;ix < isize;ix ++){
		pCommand = aPluginArray_[ix]->GetCommand(nCmdID);
		if(pCommand)
			return pCommand;
	}

	return NULL;
}

// 释放该命令，如果非缓存命令，会被free掉
//
void  QxPluginMgr::ReleaseCommand(kiCommand *pCommand)
{
	kuiPluginArray::size_type ix,isize = aPluginArray_.size();

	for(ix = 0;ix < isize;ix ++){
		if(aPluginArray_[ix]->ReleaseCommand(pCommand))
			break;
	}
}

int  QxPluginMgr::GetItemCount() const
{
	return (int)_cmdItemArray.size();
}

QxCommandItem*  QxPluginMgr::GetCmdItem(int ix)
{
	if(ix < 0 || ix >= (int)_cmdItemArray.size())
		return NULL;
	return _cmdItemArray[ix];
}

//////////////////////////////////////////////////////////////////////////
//
QxDynPluginData::QxDynPluginData(HINSTANCE hDll)
	:pDllModule_(hDll)
{
	pMainProc_ = NULL;
	pFreeProc_ = NULL;
	pImpl_ = NULL;
}

QxDynPluginData::~QxDynPluginData()
{
	
}

void QxDynPluginData::FreePluginDll()
{
	if(pDllModule_){
		::FreeLibrary(pDllModule_);
		pDllModule_ = NULL;
	}
}

//通过动态库，加载一个插件
BOOL QxPluginMgr::LoadDynPlugin(const char *pszDllPath)
{
	if(!pszDllPath)
		return FALSE;

	HINSTANCE hDll = ::LoadLibrary(pszDllPath);
	if(!hDll || hDll == INVALID_HANDLE_VALUE)
		return FALSE;

	QxCmdPluginMainProc mainProc = (QxCmdPluginMainProc)::GetProcAddress(hDll,"QxCmdPluginMain");
	QxCmdPluginFreeProc freeProc = (QxCmdPluginFreeProc)::GetProcAddress(hDll,"QxCmdPluginFree");
	if(!mainProc || !freeProc){
		::FreeLibrary(hDll);
		return FALSE;
	}

	// 获取插件实现，并初始化
	QxCmdPluginCfg plgCfg;
	QxCmdPluginImpl *pPlgImpl = mainProc(&plgCfg);
	if(!pPlgImpl){
		::FreeLibrary(hDll);
		return FALSE;
	}

	QxDynPluginData *pData = new QxDynPluginData(hDll);
	pData->pMainProc_ = mainProc;
	pData->pFreeProc_ = freeProc;
	pData->pImpl_ = pPlgImpl;

	aDynPluginArray_.push_back(pData);

	//创建对应的plugin
	QxDynCmdPlugin *pPlugin = new QxDynCmdPlugin(pPlgImpl);
	
	return LoadPlugin(pPlugin);
}
