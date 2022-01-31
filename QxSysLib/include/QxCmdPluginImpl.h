//
#ifndef _OX_CMD_PLUGIN_IMPL_H_
#define _OX_CMD_PLUGIN_IMPL_H_

#include "QxLibDef.h"
#include "QxCommandItem.h"

// 方便用于输出插件的主要接口函数
#define CMDPLUGIN_EXPORT extern "C" _declspec(dllexport)

class kiCommand;
class QxDynCmdPlugin;
class QxCmdPluginImpl;

// 说明：
//   每个插件可以由一系列命令组成，每个命令在插件内部有一个唯一的ID，用于插件内部命令的管理。
//   目前，仅仅支持将插件命令添加到菜单中。为了避免插件间命令的ID冲突，插件管理器内部为每个
//   插件分配了一个ID范围，每个插件有一个基本ID，插件中命令ID为插件基本ID+插件内命令的ID
//   这样，可以避免插件间命令ID冲突，并且也方便每个插件内部自己管理自己的ID。
//   

// 用于插件的配置和参数
typedef struct stQxCmdPluginCfg{
	int	stsize; //本结构的大小
	int version; //版本号
	int ctlflag; //控制标记
}QxCmdPluginCfg;

// 命令插件实现的接口类，每个插件需要实现该接口类的方法
// 同时提供一些函数，这些函数依附所在的类来实现对应的功能，同时对外部屏蔽内部的实现
// 
class QXLIB_API QxCmdPluginImpl{
public:
	QxCmdPluginImpl();
	virtual ~QxCmdPluginImpl(){}

	// 获取插件中的命令个数
	virtual int		GetCommandNum() const = 0;

	// 获取插件的名称,插件的名称要保证唯一性，不能和已经存在的插件冲突.
	virtual const char*	GetPluginName() const = 0;

	// 插件初始化和释放
	virtual bool		Initialize() = 0;
	virtual void		Release() = 0;

	// 根据命令在插件中的ID，创建命令对象
	virtual kiCommand*			DoCreateCommand(int nLocalID) = 0;
	// 释放创建的命令对象
	virtual bool					DoReleaseCommand(kiCommand *pCommand) = 0;

public:
	// 建立和内部插件类的关联
	void				SetPlugin(QxDynCmdPlugin *pPlugin);

	// 添加两级或三级菜单的命令
	virtual bool		AddCommandItem(int nLocalID,const char *pszTopMenu,const char *pszSubMenu,
		                                     int nMenuFlag = 0,bool bCached = false);
	virtual bool		AddCommandItem(int nLocalID,const char *pszTopMenu,const char *pszSubMenu,
		                                     const char *pszSubMenu2,int nMenuFlag = 0,bool bCached = false);
	virtual bool		AddCommandItem(int nLocalID,eAppTopMenuID topMenuId,const char *pszSubMenu,
		                                     int nMenuFlag = 0,bool bCached = false);
	virtual bool		AddCommandItem(int nLocalID,eAppTopMenuID topMenuId,const char *pszSubMenu,
		                                     const char *pszSubMenu2,int nMenuFlag = 0,bool bCached = false);

protected:
	bool				DoAddCommandItem(int nLocalID,const char *pszSubMenu,int nMenuFlag = 0,bool bCached = false);
	bool				DoAddCommandItem(int nLocalID,const char *pszSubMenu,
		                            const char *pszSubMenu2,int nMenuFlag = 0,bool bCached = false);

protected:
	QxDynCmdPlugin		*pPlugin_;// 这里记录了一个内部实现的插件类，用于加载从该接口派生实现的插件。
	eAppTopMenuID			eTopMenuID_; //所在顶级菜单ID
};

// NOTE:
//   插件的主要接口函数,每个插件中使用统一的名称：
//   QxCmdPluginMain和QxCmdPluginFree，方便通过动态加载获取这些接口函数

// 主要接口函数，创建并返回QxCmdPluginImpl对象
typedef QxCmdPluginImpl* (*QxCmdPluginMainProc)(QxCmdPluginCfg *pCfg);
// 释放创建的QxCmdPluginImpl对象
typedef void (*QxCmdPluginFreeProc)(QxCmdPluginImpl *pImpl);

#endif