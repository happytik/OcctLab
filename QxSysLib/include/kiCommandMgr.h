//////////////////////////////////////////////////////////////////////////
// 命令管理类,负责命令管理,命令分发等.

#ifndef _KI_COMMAND_MGR_H_
#define _KI_COMMAND_MGR_H_

#include <map>
#include <vector>
#include "QxLibDef.h"


class kuiMouseInput;
class kuiKeyInput;
class kiCommand;
typedef std::vector<kiCommand *>	kiCommandArray;
typedef std::map<UINT,kiCommand *>  kiCommandIDMap;
typedef std::map<std::string,kiCommand *>	kiCommandNameMap;

class kcModel;
class QxPluginMgr;
class kiSelSet;
class kiInputEdit;
class kcDocContext;

class QXLIB_API kiCommandMgr
{
public:
	kiCommandMgr(void);
	~kiCommandMgr(void);

	//初始化,使用模型对象和插件管理器对象
	BOOL						Initialize(kcDocContext *pDocCtx,QxPluginMgr *pPluginMgr);
	BOOL						Destroy();

	//注册和取消命令
	// 注册一个插件命令,通常是插件中的命令,命令销毁由插件负责.
	// 每个命令要求ID name alias都要是唯一的.
	BOOL						RegisterCommand(kiCommand *pCommand);
	// 注册一个系统命令,是系统的命令,有该mgr负责销毁,不用unreg.
	BOOL						RegisterSysCommand(kiCommand *pCommand);
	// 注销一个插件命令.
	BOOL						UnRegisterCommand(kiCommand *pCommand);
	kiCommand*				FindRegisterCommand(UINT nID);
	kiCommand*				FindRegisterCommand(const std::string& strName);
	kiCommand*				FindRegisterCommandByAlias(const std::string& strAlias);

protected:
	bool						_bInitialized;//是否初始化了
	kcDocContext				*pDocContext_; //Doc的主要数据记录在该对象
	kcModel					*_pModel;//关联的模型
	QxPluginMgr				*_pPluginMgr;//插件管理
	Handle(V3d_View)			aV3dView_;//关联的V3d_View对象句柄
	kiSelSet					*pGlobalSelSet_;//命令管理器对应全局选择集
	
	kiCommand				*_pExecCommand;//当前执行命令
	
	kiCommandArray			m_cmdArray;//记录所有命令对象.下标+KI_CMD_ID_BEGIN即为菜单项对应的ID.
	kiCommandIDMap			m_cmdIDMap;//ID和command的对应.
	kiCommandNameMap			m_cmdNameMap;//名称和命令对应
	kiCommandNameMap			m_cmdAliasMap;//别名和命令对应
	kiCommandArray			m_sysCmdArray;//系统命令,这些命令需要内部释放.

public:
	//执行命令
	int						ExecCommand(UINT nID);
	int						EndCommand(int nCode);

	BOOL						HasCommandRunning();

public:
	kcDocContext*				GetDocContext() const;
	// 获取全局选择集对象指针
	kiSelSet*				GetGlobalSelSet() const;

	//
	kiInputEdit*				GetInputEdit() const;

	//设置关联的v3d_View
	void						SetV3dView(const Handle(V3d_View) &aView);
	const Handle(V3d_View)	GetCurrV3dView() const;
	
public:
	//鼠标消息
	void						OnLButtonDown(kuiMouseInput& mouseInput);
	int						OnLButtonUp(kuiMouseInput& mouseInput);
	int						OnLButtonDblClk(kuiMouseInput& mouseInput);
	int						OnMouseMove(kuiMouseInput& mouseInput);
	void						OnRButtonDown(kuiMouseInput& mouseInput);
	int						OnRButtonUp(kuiMouseInput& mouseInput);

public:
	//键盘消息
	int						OnKeyDown(kuiKeyInput& keyInput);
	int						OnKeyUp(kuiKeyInput& keyInput);
	int						OnChar(kuiKeyInput& keyInput);

public:
	// 从commandbar获取的输入.
	int						ReceiveInputString(const char* pszString);
};

#endif