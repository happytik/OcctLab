//////////////////////////////////////////////////////////////////////////
// 
#ifndef _KI_INPUT_TOOL_H_
#define _KI_INPUT_TOOL_H_

#include <string>
#include <list>
#include "kBase.h"
#include "kPoint.h"
#include "kVector.h"
#include "QxLibDef.h"
#include "kiOptionSet.h"
#include "kiInputEdit.h"

//输入状态，表明输入成功或被取消或没有输入。
//没有输入错误的情况，输入错误需要tool内部处理。

#define KINPUT_INITIAL		0  //初始化状态
#define KINPUT_RUNNING		1  //输入工具运行，等待输入
#define KINPUT_DONE		2  //输入完成
#define KINPUT_CANCEL		3  //输入取消

class kiCommand;
class kcModel;
class kuiMouseInput;
class kuiKeyInput;
class kiOptionItem;
class kiOptionSet;
class CXInputEdit;
class kcSnapMngr;
class kcBasePlane;
class kcDocContext;

// InputTool可以直接使用在kiCommand中，当输入结束，回调对应command的OnInputFinished函数。
// InputTool可以使用在kiInputTool中，即InputTool可以保护InputTool。当输入结束，调用对应
//      的OnSubInputfinished函数。
// NOTE:
// 1.  InputTool必须和Command对象关联，InputTool可以包含子InputTool，孙InputTool，但跟InputTool
//     必须和kiCommand对象关联。
// 输入工具需要提示信息，选项列表是可选项。
//
class QXLIB_API kiInputTool : public kiInputEditCallBack,public kiOptionCallBack
{
public:
	// 用于命令中的构造函数。
	kiInputTool(kiCommand *pCommand,const char *pPromptText,kiOptionSet *pOptionSet = NULL);
	// 用于InputTool中的构造函数。
	kiInputTool(kiInputTool *pParent,const char *pPromptText,kiOptionSet *pOptionSet = NULL);
	virtual ~kiInputTool(void);

	// 开始命令的执行,返回KSTA_CONTINUE等。
	// 结果为KSTA_起始的宏。下面的函数返回结果相同。
	virtual int				Begin();
	// 结束命令的执行,nCode是输入的结果.
	virtual int				End();

	//状态管理
	virtual bool			IsInputRunning() const;
	virtual bool			IsInputDone() const;
	int						GetInputState() const;
	void					SetInputState(int nState);

	// 子tool的回调函数,当子tool调用结束时,调用.
	virtual int				OnSubInputFinished(kiInputTool *pTool);

	Handle(AIS_InteractiveContext)	GetAISContext() const;
	kcDocContext*			GetDocContext() const;

	kiInputEdit*			GetInputEdit() const;

	// 获取命令对应的V3d_View对象
	const Handle(V3d_View)	GetCurrV3dView() const;

	// 获取所在模型
	kcModel*				GetModel() const;

	// 获取当前基准面
	kcBasePlane*			GetCurrentBasePlane() const;

	kcSnapMngr*				GetSnapManager() const;

protected:// 供派生类使用
	//
	kiOptionSet&			GetToolOptionSet() { return aToolOptionSet_; }
	// 获取当前使用的set对象，可能是传入的set，可能是自身的set
	kiOptionSet*			GetCurOptionSet() const;
	// 获取从参数传入的选项集
	kiOptionSet*			GetInOptionSet() const;

protected:
	//
	virtual int				OnBegin();
	virtual int				OnEnd();

	// 获取提示字符串
	virtual void			DoGetPromptText(std::string& str);
	// 出入文本完成，通常命令可以结束了
	virtual int				DoTextInputFinished(const char *pInputText);

protected:
	// 当选项改变后的调用函数
	virtual void			OnToolOptionChanged(kiOptionItem *pOptionItem);
	virtual void			OnToolOptionClicked(kiOptionItem *pOptionItem);

public://OptionCallBack
	virtual void			OnOptionChanged(kiOptionItem *pOptionItem);
	virtual void			OnOptionSelected(kiOptionItem *pOptionItem);

public://kiInputEditCallBack接口实现
	//InputEdit输入的回调函数

	//当InputEdit输入文本改变时
	virtual int				OnEditInputChanged(const char *pszText);

	//当edit输入文本完成后，调用该函数,传入的是输入的文本信息
	virtual int				OnEditInputFinished(const char *pInputText);

	//当一个选项被选中时，例如：通过快捷字符输入
	virtual int				OnEditOptionSelected(int idx,kiOptionItem *pItem);
	
protected:
	// 输入完成
	virtual int				DoNotify();
	// 选项改变通常
	virtual int				DoNotifyOptionChanged(kiOptionItem *pOptionItem);
	virtual int				DoNotifyOptionClicked(kiOptionItem *pOptionItem);

public:
	//鼠标消息
	virtual void			OnLButtonDown(kuiMouseInput& mouseInput);
	virtual int				OnLButtonUp(kuiMouseInput& mouseInput);
	virtual int				OnLButtonDblClk(kuiMouseInput& mouseInput);
	virtual int				OnMouseMove(kuiMouseInput& mouseInput);
	virtual void			OnRButtonDown(kuiMouseInput& mouseInput);
	virtual int				OnRButtonUp(kuiMouseInput& mouseInput);

protected:
	//计算当前点
	virtual void			CalcInputPoint(kuiMouseInput& mouseInput,double point[3]);

public:
	//键盘消息
	virtual int				OnKeyDown(kuiKeyInput& keyInput);
	virtual int				OnKeyUp(kuiKeyInput& keyInput);
	virtual int				OnChar(kuiKeyInput& keyInput);

protected:
	void					DoInitialize();//必要的初始化
	void					Reset();

	virtual void			CombineOptionSet(kiOptionSet *pOptSet);
	virtual void			SplitOptionSet(kiOptionSet *pOptSet);

protected:
	//输入的参数
	kiCommand				*m_pCommand;//关联的命令
	kiInputTool				*m_pParent;//父输入tool.
	std::string				m_szPromptText; //命令行的描述信息
	
	//内部获取的参数
	kiInputEdit				*m_pInputEdit;
	kcSnapMngr				*m_pSnapMgr;//拾取
	
	//当前运行的子工具,可以包含子输入工具.
	kiInputTool				*m_pCurSubTool;

private:
	// 每个InputTool有自己的选项集（内部生成），并且工具可以带上其父类（Command or InputTool）
	// 的选择集。最终，所有选项会合并到一个选择集中显示，就是toolOptionSet中.
	int						m_nInputState;//输入结果状态。

	// 这里，有两个选项集对象，工具内部自带的，和外部传入的
	// 如果工具自身有选项，可以加入到工具自带的选项集，并会合并外部输入的选项集
	// 否则，直接使用外部输入的选项集
	// pCurOptionSet_始终指向当前使用的选项集，可能是上述之一，也可能为空

	//工具的总的选项集,最终使用这个选项集，其他选项集可以并入到该选项集中。
	kiOptionSet				aToolOptionSet_;
	// 是否输工具是否有自己的选项
	bool					bHasOwnOptions_;
	//构造函数传入的选择集
	kiOptionSet				*pInOptionSet_; 
	
	//当前的选项集合，是一个指针，可以指向不同的对象
	kiOptionSet				*pCurOptionSet_;
	//运行时参数
	kiOptionItem			*pCurrentOption_;//当前的item。

	Handle(AIS_InteractiveContext) hAISCtx_;//交互对象
};

inline bool  kiInputTool::IsInputRunning() const
{
	return (m_nInputState == KINPUT_RUNNING) ? true : false;
}

inline bool	  kiInputTool::IsInputDone() const
{
	return (m_nInputState == KINPUT_DONE) ? true : false;
}


#endif
