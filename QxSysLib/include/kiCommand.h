//////////////////////////////////////////////////////////////////////////
// 命令接口类.
#ifndef _KI_COMMAND_H_
#define _KI_COMMAND_H_

#include <string>
#include <list>
#include <map>
#include <AIS_InteractiveContext.hxx>

#include "kBase.h"
#include "kPoint.h"
#include "kVector.h"
#include "QxLibDef.h"
#include "kiInputEdit.h"
#include "kiOptionSet.h"

class kuiMouseInput;
class kuiKeyInput;
class kcModel;
class kcEntity;
class kiInputTool;
class kiInputEdit;
class kcSnapMngr;
class kiSelSet;
class kvCoordSystem;
class kcBasePlane;
class kcDocContext;
class kiInputOptionsTool;
class QxCmdTimer;
typedef std::list<kiInputTool *>	TInputToolList;
typedef std::map<int,UINT_PTR> stdTimerIDMap;

class kiCommandMgr;

//定义执行的状态,包括命令状态,输入的状态等
//下面两个是通用状态，主要用于表示函数成功或失败
#define		KSTA_ERROR			-1
#define		KSTA_SUCCESS			0

//以下是流程状态，用于命令流程或输入流程或其他流程中
//
#define		KSTA_CONTINUE			0 //流程继续执行状态
#define		KSTA_DONE			1 //完成状态
#define		KSTA_CANCEL			2 //取消状态
#define		KSTA_FAILED			3 //执行失败

// 命令的基类。
// 从InputEditCallBack派生，允许和InputEdit进行交互，接受其响应
//
class QXLIB_API kiCommand : public kiInputEditCallBack,public kiOptionCallBack
{
public:
	kiCommand(void);
	virtual ~kiCommand(void);

	// 执行一次初始化和释放的地方。
	virtual  int				Initialize(int nCmdID,kcModel *pModel,kiCommandMgr *pCmdMgr);
	virtual  int				Destroy();

	// 开始执行命令.
	//执行和结束接口
	virtual	 int				Execute();
	// 结束命令执行.通常由外部调用,command内部不要调用该函数.
	// 可能在命令正常结束调用,也可能中途中断命令时被调用.
	// 主要负责命令中资源的释放等,保证命令回到执行前的初始状态.
	//
	virtual  int				End(int nCode);
	// 应用命令,默认是右键点击调用该函数.
	// 如果返回KSTA_DONE,将结束该命令,否则继续执行.
	//
	virtual  int				Apply();//实际应用

	// 表明命令可以完成,主要和Apply函数结合使用.
	// 当在命令中点击右键时，如果没有输入工具，则命令将根据该函数返回值
	// 进行不同处理：
	//   如果返回TRUE: 表明可以调用Apply来执行功能，并结束命令
	//   如果返回FALSE：则不会调用Apply，直接结束命令，并退出.
	// 这里的函数名称不恰当。CanApply可能更合适。
	virtual  BOOL				CanFinish() { return FALSE; }//命令是否可以完成

	// 命令是否处于运行状态
	virtual  BOOL				IsRunning() const { return m_bRunning; }	

	// 当输入工具获取结果,结束时,会回调对应命令的该函数.
	// 用于继续执行命令或切换输入工具.也可能返回KSTA_DONE,表明命令执行结束.
	//
	virtual  int				OnInputFinished(kiInputTool *pTool) { return KSTA_CONTINUE;}
	// 当输入项改变时，会调用该回调函数
	virtual  int				OnInputChanged(kiInputTool *pTool) { return KSTA_CONTINUE;}

	// 当选项改变后的调用函数
	virtual void				OnOptionChanged(kiInputTool *pTool,kiOptionItem *pOptionItem) {} //选项改变了
	virtual void				OnOptionClicked(kiInputTool *pTool,kiOptionItem *pOptionItem) {} //选项被点击

	// 结束当前的输入工具
	virtual  int				EndCurrentInputTool();

public://InputEditCallBack
	//当InputEdit输入文本改变时
	virtual int				OnEditInputChanged(const char *pszText);

	//当InputEdit文本输入完成时
	virtual int				OnEditInputFinished(const char *pszText);

	//当一个选项被选中时，例如：通过快捷字符输入或通过点击
	virtual int				OnEditOptionSelected(int idx,kiOptionItem *pItem);

public://IOptionCallBack
	//选项改变了
	virtual void				OnOptionChanged(kiOptionItem *pOptItem);
	//选项被选中了
	virtual void				OnOptionSelected(kiOptionItem *pOptItem);

protected:
	// 当有选项button被点击时，会调用该响应函数
	// nID: 是选项button对应的ID值
	virtual void				OnOptionButtonClicked(int nID);
	
protected:
	//具体的初始化和释放代码,每个子命令可以做各自的初始化和销毁操作。
	virtual int				OnInitialize();
	virtual int				OnDestroy();

	// 执行函数
	virtual int				OnExecute();
	virtual int				OnEnd(int nCode);

	// 具体的应用处理函数
	virtual int				OnApply();

	// 具体的确定处理函数
	virtual int				OnOK();

protected:
	// 这里，nId保证命令内部唯一即可
	bool						SetTimer(int nId,UINT uElapse);
	bool						KillTimer(int nId);
	bool						PauseTimer(int nId);
	bool						ResumeTimer(int nId);
	bool						HasTimer(int nId);

	// 定时器处理函数
	virtual int				OnTimer(int nId,DWORD dwTime);

	stdTimerIDMap				aTimerIdMap_;

protected:
	int						HandlerTimer(UINT_PTR nIDEvent,DWORD dwPassTime);
	friend class QxCmdTimer;

	UINT_PTR					GetTimerEventID(int nID);

protected:
	//the aux function used inside the command
	kVector3					GetWorkPlaneZDir();
	void						GetWorkPlaneCoordSystem(kvCoordSystem& cs);

public:
	UINT						GetID() const { return m_nID; }
	void						SetID(UINT nID) {  m_nID = nID; }
	const char*				GetName() const { return m_strName.c_str(); }
	const char*				GetAlias() const { return m_strAlias.c_str(); }

	kcModel*					GetModel() const { return pModel_;}
	kiCommandMgr*				GetCommandMgr() const { return pCmdMgr_; }
	Handle(AIS_InteractiveContext)	GetAISContext() const;
	kcDocContext*				GetDocContext() const;
	kiInputEdit*				GetInputEdit() const;
	const Handle(V3d_View)	GetCurrV3dView() const;
	CView*					GetActiveView() const;
	
	// 获取当前基准面
	kcBasePlane*				GetCurrentBasePlane() const;

public:
	// 清除全局选择集。
	virtual bool				ClearGlobalSelSet();
	virtual bool				IsGlobalSelSetEmpty();
	virtual kiSelSet*			GetGlobalSelSet() const; 

protected:
	//请求一个InputEdit输入
	virtual int				RequestEditInput(const char *pszPromptText,kiOptionSet *pOptionSet);
	// 将当前输入请求压栈
	virtual int				PushCurrentRequest();
	virtual int				PopRequest();
	//
	void						InitSet(const char *pszName,const char *pszAlias,const char *pszDesc);

protected:
	UINT						m_nID;//ID,系统唯一,由外部设置.
	std::string				m_strName;//命令名称
	std::string				m_strAlias;//别名
	std::string				m_strDesc;//描述

	kcSnapMngr				*m_pSnapMgr;//
	
	kiInputEdit				*m_pInputEdit;// 输入编辑框的接口指针
	BOOL						m_bRunning;//是否在运行中
	BOOL						_bInitialized;//是否初始化
	int						nRequestDepth_;//请求深度

private:
	kcDocContext				*pDocContext_; //
	kcModel					*pModel_;
	kiCommandMgr				*pCmdMgr_;//命令管理器
	kiSelSet					*pGlobalSelSet_;//全局选择集指针。

	//命令所在Doc的context对象,由初始化时传入
	Handle(AIS_InteractiveContext) hAISCtx_;

	kiOptionSet				*pMainButtonOptionSet_;  // 主要按钮的选择集
	kiInputOptionsTool		*pMainButtonOptionTool_; // 用于显示主要按钮的输入工具

protected:
	// 重新绘制
	virtual void				Redraw();

	// 结束当前命令执行。命令结束使用该函数。
	// 命令执行结束（终止或取消）要统一由cmdMgr来执行。
	// 该函数执行后,当前命令将被终止,不能再在该函数调用后,执行
	// 任何操作了.该函数内部由cmdMgr调用End函数,end函数进行最后的
	// 结束处理.
	virtual int				EndCommand(int nCode);

	// 命令执行完成。可以结束了。会置结束状态.
	virtual int				Done();

	// 显示提示信息到命令行
	virtual void				PromptMessage(const char *lpszMsg);
	
protected:
	// 计算捕捉点
	virtual BOOL				CalcSnapPoint();

	// 添加临时捕捉点
	virtual bool				AddTempSnapPoint(double x,double y,double z);

	virtual void				ClearTempSnapPoint();

protected:
	// 创建必要的输入工具,在Initialize中调用，每个命令仅仅调用一次。
	virtual	BOOL				CreateInputTools();
	// 销毁创建的输入工具.每个命令仅调用一次.
	virtual BOOL				DestroyInputTools();
	// 在begin中调用，主要是初始化工具队列。
	virtual BOOL				InitInputToolQueue();
	virtual void				ClearInputToolList();

	// 添加一个主按钮输入工具，作为菜单
	virtual void				AddMainButtonOptionTool(bool withApply=false);
	kiOptionSet*				GetMainButtonOptionSet() const;

	virtual int				AddInputTool(kiInputTool *pTool);

	// 执行toollist中下一个输入工具.假定前面的输入已经成功.
	virtual int				NavToNextTool();
	// 调到特定的工具进行执行.用于插入特殊输入时使用.
	virtual int				NavToInputTool(kiInputTool *pTool);

protected:
	TInputToolList			m_inputToolQueue;//输入tool列表
	kiInputTool			    *m_pCurInputTool;//当前输入工具.

public:
	//鼠标消息
	virtual void				OnLButtonDown(kuiMouseInput& mouseInput);
	// 当命令在这里完成时,可以返回KSTA_DONE,表示命令结束.
	virtual int				OnLButtonUp(kuiMouseInput& mouseInput);
	virtual int				OnLButtonDblClk(kuiMouseInput& mouseInput);
	virtual int				OnMouseMove(kuiMouseInput& mouseInput);
	virtual void				OnRButtonDown(kuiMouseInput& mouseInput);
	// 当命令在这里完成时,可以返回KSTA_DONE,表示命令结束.
	virtual int				OnRButtonUp(kuiMouseInput& mouseInput);

public:
	//键盘消息
	virtual int				OnKeyDown(kuiKeyInput& keyInput);
	virtual int				OnKeyUp(kuiKeyInput& keyInput);
	virtual int				OnChar(kuiKeyInput& keyInput);

protected:
	//辅助函数
	void						UAddToModel(const std::vector<kcEntity*> &aEntVec);
	//
	void						UAddToModel(kcEntity *pEntity,BOOL bUpdateView=FALSE);
	//
	void						UAddToModel(const TopoDS_Shape &aS,BOOL bUpdateView=FALSE);
};

#endif