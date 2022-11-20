//////////////////////////////////////////////////////////////////////////
//
#ifndef _KI_OPTIONSET_H_
#define _KI_OPTIONSET_H_

#include <string>
#include <vector>
#include "QxLibDef.h"
#include "kiInputEdit.h"

#define KOPT_INPUT_BOOL			1 //bool型
#define KOPT_INPUT_INT				2
#define KOPT_INPUT_DOUBLE			3
#define KOPT_INPUT_ENUM			4 //多个可选项
#define KOPT_INPUT_BUTTON			5 //没有值，就是一个选项或动作。
#define KOPT_INPUT_STDAXIS			6 //标准坐标系轴

class kiOptionItem;
	class kiOptionVoid;
	class kiOptionBool;
	class kiOptionInt;
	class kiOptionDouble;
	class kiOptionEnum;
	class kiOptionButton;

class kiOptionSet;
class kiInputTool;
class kiInputEdit;
class kiInputEditCallBack;

typedef std::vector<kiOptionItem *> stdOptionItemArray;

// 这里定义系统使用button的ID，避免和其他Id冲突，可以用于命令中
#define QX_SYS_BUTTON_ID_BASE			0x0F0000
#define QX_SYS_BUTTON_APPLY			(QX_SYS_BUTTON_ID_BASE + 1) //“应用”选项按钮
#define QX_SYS_BUTTON_OK				(QX_SYS_BUTTON_ID_BASE + 2) //“确定”选项按钮
#define QX_SYS_BUTTON_CANCEL			(QX_SYS_BUTTON_ID_BASE + 3) //“退出”选项按钮

// 选项改变或选中的回调接口,调用该接口的回调函数，通知其选项已经改变或被选定（单击）
// 目前被Command和InputTool实现.
//
class kiOptionCallBack{
public:
	// 选项值被改变了，适用于有值的选项
	virtual void				OnOptionChanged(kiOptionItem *pOptItem) = 0;
	// 选项被选中了，目前主要是OptionButton被点击时，会调用该函数通知
	virtual void				OnOptionSelected(kiOptionItem *pOptItem) = 0;
};

//////////////////////////////////////////////////////////////////////////
// 一个选项，可以是多种类型
// 形如:name(key)<default value>
// 例如:距离(D)<20>,其中距离为name,D为key,20为默认值显示(如果有).
// 不同的Item根据不同的ID来区分。
//
class QXLIB_API kiOptionItem : public kiInputEditCallBack
{
public:
	//pname:选项名称 key:选项对应的快捷键。
	kiOptionItem(const char *pname,char key);
	kiOptionItem(const char *pname,char key,int id);
	virtual ~kiOptionItem();

	virtual int				GetType() const = 0;
	int						GetID() const { return m_nID;}
	void						SetID(int nID) { m_nID = nID; }
	void						EnablePopup(bool bEnable) { m_bPopupItem = bEnable; }
	//是否有效
	virtual bool				IsValid() const;

	//检查数据的有效性,当用户在该item下输入值,检查有效性,获取结果.
	virtual bool				ParseInputText(const char *lpszText) = 0;

	// 用于构造命令行显示时,使用,显示形如:距离(D)<20>:的句子,等待用户输入.
	// 仅仅复杂本身的显示,如果有sub optionSet,不处理该optionSet.
	virtual bool				GetPromptText(std::string& strPrompt) = 0;

	// 当是bool和enum时，要构造对应的enum值。
	virtual kiOptionSet*		GetSubOptionSet() = 0;
	// 主要用于bool和enum时,用于记录点击获取的值或选项.
	virtual bool				SetCurrentOption(int idx) = 0;

public://CallBack接口
	//当InputEdit输入文本改变时
	virtual int				OnEditInputChanged(const char *pszText);

	//当InputEdit文本输入完成时
	virtual int				OnEditInputFinished(const char *pszText);

	//当一个选项被选中时，例如：通过快捷字符输入或通过点击
	virtual int				OnEditOptionSelected(int idx,kiOptionItem *pItem);

public:
	// 提示选项对应的值已经被改变了,
	// 适用于选项值发生了改变时，通常关联的comand或inputtool对象进行处理
	//
	void						NotifyOptionChanged();

public:
	std::string				m_strName;//名称
	char						m_cKey;//大写保存。快捷键字符.
	int						m_nID;//唯一标识选项的id,不设则为-1.
	bool						m_bPopupItem;//是否弹出窗口项。true：弹出窗口，接受输入值.

public:
	virtual void				SetParentItem(kiOptionItem *pParentItem) { pParentItem_ = pParentItem; }
	
	// 设定回调对象
	virtual void				SetOptionCallBack(kiOptionCallBack *pCallBack);
	bool						HasOptionCallBack() const;

protected:
	virtual void				SetInputEdit(kiInputEdit *pInputEdit);
	

	friend class kiOptionSet;
	
protected:
	kiOptionItem				*pParentItem_;//父Item类
	//输入Edit接口.用于建立输入框和选项间的关联
	kiInputEdit				*pInputEdit_;

	// 回调接口,目前由Command或InputTool实现，因此可以关联对应的对象，
	// 在选项变换时，通知这些对象,目前该项需要由OptionSet对象设置
	//   可以单独设定回调对象，或者由OptionSet统一设定回调对象，提供足够的灵活性
	//   当Item不设定时，则由OptionSet设定。
	//   如果两者都不设定，则不需要回调对象,允许为空
	kiOptionCallBack			*pOptionCallBack_;
};

// 形如:name(key),例如:红色(R) 黄色(Y).没有值.
// 主要用于bool和enum中,为子项.表示一个选项
// 也可以表示一个动作，以ID区分。
// 
class QXLIB_API kiOptionButton : public kiOptionItem{
public:
	kiOptionButton(const char *pname,char key);
	kiOptionButton(const char *pname,char key,int id);

	virtual int				GetType() const { return KOPT_INPUT_BUTTON; }

	// 检查数据的有效性,输入应当是快捷键key.
	virtual bool				ParseInputText(const char *lpszText);
	// 
	virtual bool				GetPromptText(std::string& strPrompt);
	virtual kiOptionSet*		GetSubOptionSet() { return NULL; }
	virtual bool				SetCurrentOption(int idx) { return true;}

public:
	//当InputEdit文本输入完成时
	virtual int				OnEditInputFinished(const char *pszText);

	//当一个选项被选中时，例如：通过快捷字符输入或通过点击
	virtual int				OnEditOptionSelected(int idx,kiOptionItem *pItem);
};

// 形如:是否拷贝(C)<是>
class QXLIB_API kiOptionBool : public kiOptionItem{
public:
	kiOptionBool(const char *pname,char key,bool& bVar);//bVar是变量的引用，结果会保存到该变量中。
	kiOptionBool(const char *pname,char key,int id,bool& bVar);
	~kiOptionBool();

	virtual int				GetType() const { return KOPT_INPUT_BOOL; }

	//反转值.
	void						NotValue();

	//检查数据的有效性
	virtual bool				ParseInputText(const char *lpszText);
	virtual bool				GetPromptText(std::string& strPrompt);
	virtual kiOptionSet*		GetSubOptionSet() { return NULL;}
	virtual bool				SetCurrentOption(int idx) { return true;}

public:
	//当InputEdit文本输入完成时
	virtual int				OnEditInputFinished(const char *pszText);

	//当一个选项被选中时，例如：通过快捷字符输入或通过点击
	virtual int				OnEditOptionSelected(int idx,kiOptionItem *pItem);

public:
	bool&					m_boolVar;//管理向量引用。
};

// 枚举,形如:类型<单线>(单线(S) 多线(M))
//
class QXLIB_API kiOptionEnum : public kiOptionItem{
public:
	kiOptionEnum(const char *pname,char key,int& eVal);
	kiOptionEnum(const char *pname,char key,int id,int& eVal);
	virtual ~kiOptionEnum();

	virtual int				GetType() const { return KOPT_INPUT_ENUM; }

	//添加一个选项.
	bool						AddEnumItem(const char *pname,char key);

	//检查数据的有效性
	virtual bool				ParseInputText(const char *lpszText);
	virtual bool				GetPromptText(std::string& strPrompt);
	virtual kiOptionSet*		GetSubOptionSet() { return m_pOptionSet;}
	virtual bool				SetCurrentOption(int idx);

	int						GetPrevEnumValue() const { return m_oldEnumVal; }

public:
	//当InputEdit文本输入完成时
	virtual int				OnEditInputFinished(const char *pszText);

	//当一个选项被选中时，例如：通过快捷字符输入或通过点击
	virtual int				OnEditOptionSelected(int idx,kiOptionItem *pItem);

public:
	int&						m_enumVal;

protected:
	kiOptionSet				*m_pOptionSet;
	int						m_oldEnumVal;//切换前的值
};

//标准坐标系轴选项
class QXLIB_API kiOptionStdAxis : public kiOptionEnum{
public:
	kiOptionStdAxis(const char *pname,char key,int& eVal);
	kiOptionStdAxis(const char *pname,char key,int id,int& eVal);

	int						Initialize(bool bUseWCS,bool bOnlyZ);

protected:
	bool						_bUseWCS;
	bool						_bOnlyZ;
};

// 形如:次数(D)<3>
//
class QXLIB_API kiOptionInt : public kiOptionItem{
public:
	kiOptionInt(const char *pname,char key,int& iVal);
	kiOptionInt(const char *pname,char key,int id,int& iVal);

	virtual int				GetType() const { return KOPT_INPUT_INT; }

	//检查数据的有效性
	virtual bool				ParseInputText(const char *lpszText);
	virtual bool				GetPromptText(std::string& strPrompt);
	virtual kiOptionSet*		GetSubOptionSet() { return NULL; }
	virtual bool				SetCurrentOption(int idx) { return true;}

public:
	//当InputEdit文本输入完成时
	virtual int				OnEditInputFinished(const char *pszText);

	//当一个选项被选中时，例如：通过快捷字符输入或通过点击
	virtual int				OnEditOptionSelected(int idx,kiOptionItem *pItem);

public:
	int&						m_intVar;//
};

// 形如:距离(D)<20.000>
//
class QXLIB_API kiOptionDouble : public kiOptionItem{
public:
	kiOptionDouble(const char *pname,char key,double& dval);
	kiOptionDouble(const char *pname,char key,int id,double& dval);

	virtual int				GetType() const { return KOPT_INPUT_DOUBLE; }

	//检查数据的有效性
	virtual bool				ParseInputText(const char *lpszText);
	virtual bool				GetPromptText(std::string& strPrompt);
	virtual kiOptionSet*		GetSubOptionSet() { return NULL; }
	virtual bool				SetCurrentOption(int idx) { return true;}

public:
	//当InputEdit文本输入完成时
	virtual int				OnEditInputFinished(const char *pszText);

	//当一个选项被选中时，例如：通过快捷字符输入或通过点击
	virtual int				OnEditOptionSelected(int idx,kiOptionItem *pItem);

public:
	double&					m_doubleVal;
};

/// 一个选项集合，可以被command和inputtool使用，当选项改变或被点击时，
//  会触发对应对象的回调函数。
//  当加入InputTool时，如果OptionSet没有设定IOptionCallBack，则会设定为InputTool，
//  否则使用OptionSet设定的IOptionCallBack接口

class QXLIB_API kiOptionSet{
public:
	kiOptionSet();
	kiOptionSet(kiOptionCallBack *pOptCallBack);
	~kiOptionSet();

	// 关联输入框，这个是必须的
	void						SetInputEdit(kiInputEdit *pInputEdit);

	// 关联command或inputTool对象，如果希望选项改变时，相应对象得到通知，需要
	// 设定该关联
	void						SetOptionCallBack(kiOptionCallBack *pCallBack);
	bool						HasOptionCallBack() const;

	// 添加一个item。
	// Item对象无论在哪里分配，都会由OptionSet中来释放
	bool						AddOption(kiOptionItem* pItem);
	kiOptionItem*				AddBoolOption(const char *pname,char key,bool& bVar);
	kiOptionItem*				AddIntOption(const char *pname,char key,int& iVal);
	kiOptionItem*				AddDoubleOption(const char *pname,char key,double& dval);
	kiOptionItem*				AddButtonOption(const char *pname,char key,int nID);
	kiOptionItem*				AddSysButtonOption(int nID);
	void						AddApplyButtonOption();
	void						AddQuitButtonOption();

	//插入一个选项
	bool						InsertOptionItem(size_t idx,kiOptionItem *pItem);
	bool						RemoveOptionItem(kiOptionItem *pItem);

	void						Clear();
	bool						IsEmpty() const;

	int						GetCount() const;
	kiOptionItem*				GetOption(int ix);

	//根据键获取对应下标。
	int						FindOption(char c);
	int						IndexOfOption(char c);
	int						IndexOfOption(kiOptionItem *pItem);
	bool						HasOptionItem(kiOptionItem *pItem);

protected:
	void						DoAddItem(kiOptionItem *pItem);

protected:
	kiOptionCallBack			*pOptionCallBack_;  //关联的对象，选项修改时，调用对应的回调函数
	stdOptionItemArray		aItemArray_;
	kiInputEdit				*pInputEdit_;//关联的输入编辑接口
};


#endif