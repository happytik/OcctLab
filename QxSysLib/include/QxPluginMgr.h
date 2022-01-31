//////////////////////////////////////////////////////////////////////////
// 插件管理类

#ifndef _QX_PLUGIN_MGR_H_
#define _QX_PLUGIN_MGR_H_

#include <string>
#include <vector>
#include "QxLibDef.h"
#include "QxCmdPlugin.h"

//菜单项对应cmdID的起始范围.
#define KI_CMD_ID_BEGIN		33000
#define KI_CMD_ID_END			39000 //6000个命令,足够了.

class kiCommand;
class QxCommandItem;
class QxCmdPlugin;
typedef std::vector<QxCmdPlugin *>		kuiPluginArray;
typedef std::vector<QxCommandItem *>	kuiCmdItemArray;

//记录一个动态插件的信息
class QxDynPluginData{
public:
	QxDynPluginData(HINSTANCE hDll);
	~QxDynPluginData();

	void					FreePluginDll();

public:
	HINSTANCE 			pDllModule_;//动态库句柄
	QxCmdPluginMainProc	pMainProc_;
	QxCmdPluginFreeProc	pFreeProc_;
	QxCmdPluginImpl		*pImpl_;//具体实现类
};

class QXLIB_API QxPluginMgr
{
public:
	QxPluginMgr(void);
	~QxPluginMgr(void);

	int						Initialize();
	void						Destroy();

	//加载一个插件
	BOOL						LoadPlugin(QxCmdPlugin *pPlugin);
	//卸载一个插件
	BOOL						UnloadPlugin(QxCmdPlugin *pPlugin);

	QxCmdPlugin*				FindPlugin(const std::string& strName);

	int						GetItemCount() const;
	QxCommandItem*			GetCmdItem(int ix);

public:
	//根据命令命令id获取命令
	kiCommand*				GetCommand(int nCmdID);
	//释放该命令，如果非缓存命令，会被free掉
	//!!暂时没有用
	void						ReleaseCommand(kiCommand *pCommand);

public:
	//通过动态库，加载一个插件
	BOOL						LoadDynPlugin(const char *pszDllPath);

protected:
	std::vector<QxDynPluginData*>	aDynPluginArray_;

protected:
	kuiPluginArray			aPluginArray_;//插件列表
	kuiCmdItemArray			_cmdItemArray;//所有命令项列表.仅仅记录指针.指针释放由插件负责.
	UINT						_uiCurrentID;//当前可用的ID.
};

#endif