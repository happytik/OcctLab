//////////////////////////////////////////////////////////////////////////
// 拾取entity管理
//
#ifndef _KI_INPUT_ENTITY_TOOL_H_
#define _KI_INPUT_ENTITY_TOOL_H_


#include "kiInputTool.h"
#include "kiSelSet.h"

class kiOptionEnum;

// 对象拾取工具
// 使用7.6.0的新拾取机制，可以拾取自然对象和局部对象，可以支持多种模式下的选取
// 将逐步替代kiInputLocalEntityTool的功能
//

class QXLIB_API QxShapePickInfo{
public:
	QxShapePickInfo();
	QxShapePickInfo(const QxShapePickInfo& other);

	QxShapePickInfo&	operator=(const QxShapePickInfo& other);

public:
	bool				_bValid;
	double				_t;//拾取参数
	double				_u, _v;
	kPoint3				_p;
};

class QXLIB_API kiInputEntityTool : public kiInputTool
{
public:
	kiInputEntityTool(kiCommand *pCommand,const char *pszPrompt,kiOptionSet *pOptionSet = NULL);
	virtual ~kiInputEntityTool(void);

	//为了简化实现，如下这些设置，必须在工具处于初始化状态才能够设定。
	//一定工具开始执行，等待输入，就不能更改了

	//设定要选择的对象类型
	//掩码类型在kcEntity.h中定义，例如：KCT_ENT_FACE | KCT_ENT_SHELL
	void						SetOption(int typeMask,bool bSingleSel = false);
	//设定选择对象类型和对象个数。当选择达到个数时，会自动结束。否则右键结束选择。
	//num为-1,不限制选择的个数
	void						SetOption(int typeMask,int num);
	//设定为选择自然对象模式，即不支持局部对象选取
	void						SetNaturalMode(bool bEnable);
	//设定为类型切换模式，就是多种类型可以选择切换
	void						SetTypeSwitch(bool bEnable);
	
	// 设定全局的可交换对象，选择的子对象只能在该对象内部选取
	void						SetGlobalAISObject(const Handle(AIS_InteractiveObject) &aObj);

	//更加灵活的接口
	bool						SetEntityTypes(int typeMask);
	bool						SetEntityLimit(int nLimit);//设定拾取的个数限制
	
	void						SetSingleSelect(bool bEnable);//单选或多选
	// 是否需求点击点的信息,目前仅支持单选情况下，edge的点击点
	void						NeedPickInfo(bool bNeeded); 

	//需要点击点的信息，通常对Edge或Face而言
	void						NeedPickedPoint(int typeMask);

	void						ActivateSelectionModes();//激活选择模式
	void						DeactivateSelectionModes();//去激活选择模式

	//virtual int				Begin();
	//virtual int				End();

	void						SetHilightColor(const Quantity_NameOfColor hicol) { m_hiColor = hicol; }

	//添加全局选择集中的对象
	virtual void				AddInitSelected(bool bAutoReturn);

protected:
	//
	virtual int				OnBegin();
	virtual int				OnEnd();

	void						DoActivateSelectionModes();
	void						DoActiveDefaultSelectionMode();

public:
	//当InputEdit文本输入完成时
	virtual int				OnEditInputFinished(const char *pszText);

public:
	// 当选项改变后的调用函数
	virtual void				OnOptionChanged(kiOptionItem *pOptionItem);

public:
	//鼠标消息
	virtual void				OnLButtonDown(kuiMouseInput& mouseInput); 
	virtual int				OnLButtonUp(kuiMouseInput& mouseInput);
	virtual int				OnMouseMove(kuiMouseInput& mouseInput);
	virtual int				OnRButtonUp(kuiMouseInput& mouseInput);

public:
	//清除选择项
	void						ClearSel();
	kiSelSet*				GetSelSet();
	int						GetSelCount();
	kiSelEntity				GetFirstSelect();
	TopoDS_Shape				SelectedShape(int ix);

	bool						GetShapePickInfo(QxShapePickInfo& info);

protected:
	//获取选中项
	bool						GetSelected(kuiMouseInput& mouseInput);
	//分离选择对象类型
	void						DoSplitShapeTypes();
	//创建类型切换选项
	BOOL						CreateTypeSwitchOption();

	//
	BOOL						GetPickInfo(kuiMouseInput& mouseInput);

protected:
	kiSelSet					m_selSet;
	Quantity_NameOfColor		m_hiColor;
	Handle(AIS_InteractiveObject)	hGlobalAISObj_; // 如果存在，则只能选择该对象中的子对象
	
	int						m_nTypeMask;
	int						m_nSelLimit;
	bool						m_bSingleSelect;//只选中一个
	bool						m_bNaturalMode;//是否自然选择模式，此时不能选择局部对象
	bool						m_bNeedPickInfo;//是否需要拾取信息，包括参数等。

	int						m_nPickedPointTypeMask;//需要拾取点信息的类型
	QxShapePickInfo			m_aPickInfo; //曲线或曲面的拾取信息

protected:
	bool						m_bTypeSwitch;//是否是类型切换模式
	int						m_nOptionIdx;//选项下标
	TopAbs_ShapeEnum			m_aShapeTypes[8];//激活类型
	int						m_nShapeTypes;//类型个数
	kiOptionEnum				*pTypeOptEnum_; //用于显示多个类型的切换
};

#endif