#include "StdAfx.h"
#include "kiCommand.h"
#include "QxCommandItem.h"
#include "QxCmdPlugin.h"

QxCmdPlugin::QxCmdPlugin(void)
{
	_strName = "";
	_nBaseCmdID = 0;
	_nReserveNum = 0;
}

QxCmdPlugin::~QxCmdPlugin(void)
{
	if(!_cmdItemArray.empty()){
		Release();
	}
}

void  QxCmdPlugin::Release()
{
	QxCommandItem *pCmdItem = NULL;
	int ix,isize = (int)_cmdItemArray.size();
	for(ix = 0;ix < isize;ix ++){
		pCmdItem = _cmdItemArray[ix];
		if(pCmdItem->_pCommand){
			pCmdItem->_pCommand->Destroy();
			DoReleaseCommand(pCmdItem->_pCommand);
			pCmdItem->_pCommand = NULL;
		}
		
		delete pCmdItem;
	}
	_cmdItemArray.clear();
	_cmdItemMap.clear();
}

//根据基准ID生成命令ID。
//
int  QxCmdPlugin::GenCommandID(int nBaseCmdID)
{
	ASSERT(!_cmdItemArray.empty());
	_nBaseCmdID = nBaseCmdID;

	QxCommandItem *pCmdItem = NULL;
	kuiCmdItemArray::size_type ix,isize = _cmdItemArray.size();
	for(ix = 0;ix < isize;ix ++){
		pCmdItem = _cmdItemArray[ix];
		pCmdItem->_nCmdID = pCmdItem->_nLocalID + nBaseCmdID;
	}

	return 1;
}

//返回预留命令项的个数
int  QxCmdPlugin::GetReserveNum() const
{
	return _nReserveNum;
}

int  QxCmdPlugin::GetItemCount() const
{
	return (int)_cmdItemArray.size();
}

QxCommandItem*	QxCmdPlugin::GetCmdItem(int ix) 
{
	if(ix < 0 || ix >= (int)_cmdItemArray.size())
		return NULL;
	return _cmdItemArray[ix];
}

const std::string&  QxCmdPlugin::GetName() const
{
	ASSERT(!_strName.empty());
	return _strName;
}

//
BOOL  QxCmdPlugin::AddCommandItem(QxCommandItem *pItem)
{
	ASSERT(pItem);
	int nLocalID = pItem->_nLocalID;
	if(nLocalID < 0 || nLocalID >= _nReserveNum){
		ASSERT(FALSE);
		return FALSE;
	}

	_cmdItemArray.push_back(pItem);
	//记录到表中
	_cmdItemMap.insert(kuiCmdItemMap::value_type(nLocalID,pItem));

	return TRUE;
}

BOOL  QxCmdPlugin::AddCommandItem(int nLocalID,const char *pszTopMenu,const char *pszSubMenu,
		                     int nMenuFlag,bool bCached)
{
	ASSERT(pszTopMenu && pszSubMenu);
	if(!DoCheckLocalID(nLocalID))
		return FALSE;

	QxCommandItem *pItem = new QxCommandItem(nLocalID,bCached);
	pItem->SetMenuText(pszTopMenu,pszSubMenu);
	pItem->_nMenuFlag = nMenuFlag;
	
	return AddCommandItem(pItem);
}

BOOL  QxCmdPlugin::AddCommandItem(int nLocalID,const char *pszTopMenu,const char *pszSubMenu,
		                     const char *pszSubMenu2,int nMenuFlag,bool bCached)
{
	ASSERT(pszTopMenu && pszSubMenu && pszSubMenu2);
	if(!DoCheckLocalID(nLocalID))
		return FALSE;

	QxCommandItem *pItem = new QxCommandItem(nLocalID,bCached);
	pItem->SetMenuText(pszTopMenu,pszSubMenu,pszSubMenu2);
	pItem->_nMenuFlag = nMenuFlag;

	return AddCommandItem(pItem);
}

BOOL  QxCmdPlugin::AddCommandItem(int nLocalID,eAppTopMenuID topMenuId,const char *pszSubMenu,
		                     int nMenuFlag,bool bCached)
{
	ASSERT(pszSubMenu);
	if(!DoCheckLocalID(nLocalID))
		return FALSE;

	QxCommandItem *pItem = new QxCommandItem(nLocalID,bCached);
	pItem->SetMenuText(topMenuId,pszSubMenu);
	pItem->_nMenuFlag = nMenuFlag;
	
	return AddCommandItem(pItem);
}

BOOL  QxCmdPlugin::AddCommandItem(int nLocalID,eAppTopMenuID topMenuId,const char *pszSubMenu,
		                     const char *pszSubMenu2,int nMenuFlag,bool bCached)
{
	ASSERT(pszSubMenu && pszSubMenu2);
	if(!DoCheckLocalID(nLocalID))
		return FALSE;
	
	QxCommandItem *pItem = new QxCommandItem(nLocalID,bCached);
	pItem->SetMenuText(topMenuId,pszSubMenu,pszSubMenu2);
	pItem->_nMenuFlag = nMenuFlag;

	return AddCommandItem(pItem);
}

bool  QxCmdPlugin::DoCheckLocalID(int nLocalID)
{
	if(FindCommandItem(nLocalID)){
		TRACE("\n###--- nLocalID %d has existed.\n",nLocalID);
		ASSERT(FALSE);
		return false;
	}
	if(nLocalID < 0 || nLocalID >= _nReserveNum){
		TRACE("\n###--- nLocalID %d large than reverse com mumber %d.\n",nLocalID, _nReserveNum);
		ASSERT(FALSE);
		return false;
	}
	return true;
}

QxCommandItem*  QxCmdPlugin::FindCommandItem(int nLocalID)
{
	kuiCmdItemMap::iterator ite = _cmdItemMap.find(nLocalID);
	if(ite != _cmdItemMap.end()){
		return (*ite).second;
	}
	return NULL;
}

// 根据命令命令id获取命令
//
kiCommand*  QxCmdPlugin::GetCommand(int nCmdID)
{
	kiCommand *pCommand = NULL;
	int nLocalID = nCmdID - _nBaseCmdID;
	if(nLocalID >= 0 && nLocalID < _nReserveNum){
		QxCommandItem *pCmdItem = FindCommandItem(nLocalID);
		if(!pCmdItem){
			ASSERT(FALSE);//不应当发生
			return NULL;
		}

		//这里，如果该command不缓存，释放掉上次的command
		//不在命令结束后，立即释放command，容易产生野指针，造成死机问题
		if(pCmdItem->_pCommand){
			if(pCmdItem->_bCached){
				pCommand = pCmdItem->_pCommand;
			}else{
				pCmdItem->_pCommand->Destroy();
				DoReleaseCommand(pCmdItem->_pCommand);
				pCmdItem->_pCommand = NULL;
			}
		}
		//没有，需要创建
		if(!pCommand){
			pCommand = DoCreateCommand(nLocalID);
			//缓存下来,以便于后续释放
			if(pCommand){
				pCmdItem->_pCommand = pCommand;
			}
		}
	}	

	return pCommand;
}

// 释放该命令，如果非缓存命令，会被free掉
//
bool  QxCmdPlugin::ReleaseCommand(kiCommand *pCommand)
{
	ASSERT(pCommand);
	int nCmdID = (int)pCommand->GetID();
	int nLocalID = nCmdID - _nBaseCmdID;
	if(nLocalID >= 0 && nLocalID < _nReserveNum){
		QxCommandItem *pCmdItem = FindCommandItem(nLocalID);
		if(pCmdItem){
			if(pCmdItem->_bCached)
				return true;

			//实际的释放
			pCommand->Destroy();
			DoReleaseCommand(pCommand);
			pCmdItem->_pCommand = NULL;
			return true;
		}
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////////
//
QxDynCmdPlugin::QxDynCmdPlugin(QxCmdPluginImpl *pImpl)
	:pPluginImpl_(pImpl)
{
	pImpl->SetPlugin(this);//建立关联
	_strName = pImpl->GetPluginName();
	_nReserveNum = pImpl->GetCommandNum();
}


QxDynCmdPlugin::~QxDynCmdPlugin()
{
}

BOOL QxDynCmdPlugin::Initialize()
{
	if(pPluginImpl_){
		return (pPluginImpl_->Initialize() ? TRUE : FALSE);
	}
	return FALSE;
}

void QxDynCmdPlugin::Release()
{
	if(pPluginImpl_){
		pPluginImpl_->Release();
	}

	QxCmdPlugin::Release();
}

int QxDynCmdPlugin::GetReserveNum() const//返回预览命令项的个数
{
	if(pPluginImpl_){
		return pPluginImpl_->GetCommandNum();
	}
	return 0;
}

kiCommand* QxDynCmdPlugin::DoCreateCommand(int nLocalID)
{
	kiCommand *pCmd = NULL;
	if(pPluginImpl_){
		pCmd = pPluginImpl_->DoCreateCommand(nLocalID);
	}
	return pCmd;
}

bool QxDynCmdPlugin::DoReleaseCommand(kiCommand *pCommand)
{
	if(pPluginImpl_){
		return pPluginImpl_->DoReleaseCommand(pCommand);
	}
	return false;
}