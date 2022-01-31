//////////////////////////////////////////////////////////////////////////
// 命令插件的实现类.
#ifndef _QX_CMD_PLUGIN_H_
#define _QX_CMD_PLUGIN_H_

#include <string>
#include <vector>
#include <map>
#include "QxLibDef.h"
#include "QxCmdPluginImpl.h"

class QxCommandItem;
typedef std::vector<QxCommandItem *>		kuiCmdItemArray;
typedef std::map<int,QxCommandItem *>		kuiCmdItemMap;

// 插件类，管理一个程序内部的插件或者外部的动态可加载插件
// 内部插件类，从QxCmdPlugin派生，重新实现纯虚函数。
// 外部动态加载插件，基于QxDynCmdPlugin类和插件中的OxCmdPluginImpl派生类来实现。
//
// 每个插件有一个唯一的插件名称，不能相互重名。
//

class QXLIB_API QxCmdPlugin
{
public:
	QxCmdPlugin(void);
	virtual ~QxCmdPlugin(void);

	//初始化和释放
	virtual BOOL					Initialize() = 0;
	virtual void					Release();

	//根据基准ID计算每个命令对应的命令ID
	virtual int					GenCommandID(int nBaseCmdID);
		
	virtual int					GetReserveNum() const;//返回预览命令项的个数
	virtual int					GetItemCount() const;
	virtual	QxCommandItem*		GetCmdItem(int ix);

	const std::string&			GetName() const;

public:
	//根据命令命令id获取命令
	kiCommand*					GetCommand(int nCmdID);
	//释放该命令，如果非缓存命令，会被free掉
	bool							ReleaseCommand(kiCommand *pCommand);

protected:
	virtual kiCommand*			DoCreateCommand(int nLocalID) = 0;
	virtual bool					DoReleaseCommand(kiCommand *pCommand) = 0;

public:
	//
	virtual BOOL					AddCommandItem(QxCommandItem *pItem);
	virtual BOOL					AddCommandItem(int nLocalID,const char *pszTopMenu,const char *pszSubMenu,
		                                     int nMenuFlag = 0,bool bCached = false);
	virtual BOOL					AddCommandItem(int nLocalID,const char *pszTopMenu,const char *pszSubMenu,
		                                     const char *pszSubMenu2,int nMenuFlag = 0,bool bCached = false);
	virtual BOOL					AddCommandItem(int nLocalID,eAppTopMenuID topMenuId,const char *pszSubMenu,
		                                     int nMenuFlag = 0,bool bCached = false);
	virtual BOOL					AddCommandItem(int nLocalID,eAppTopMenuID topMenuId,const char *pszSubMenu,
		                                     const char *pszSubMenu2,int nMenuFlag = 0,bool bCached = false);
	QxCommandItem*				FindCommandItem(int nLocalID);

protected:
	bool							DoCheckLocalID(int nLocalID);

protected:
	std::string					_strName;
	kuiCmdItemArray				_cmdItemArray;//命令项列表
	kuiCmdItemMap					_cmdItemMap;//命令项表
	int							_nBaseCmdID;//基准命令ID，开始的命令ID
	int							_nReserveNum;//预留的个数
};

//
class QXLIB_API QxDynCmdPlugin : public QxCmdPlugin{
public:
	QxDynCmdPlugin(QxCmdPluginImpl *pImpl);
	~QxDynCmdPlugin();

	virtual BOOL					Initialize();
	virtual void					Release();

	virtual int					GetReserveNum() const;//返回预览命令项的个数

protected:
	virtual kiCommand*			DoCreateCommand(int nLocalID);
	virtual bool					DoReleaseCommand(kiCommand *pCommand);

protected:
	QxCmdPluginImpl				*pPluginImpl_;//具体的实现类
};

#endif