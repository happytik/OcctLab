//////////////////////////////////////////////////////////////////////////
// 简单的输入接口类
#ifndef _KI_INPUT_EDIT_H_
#define _KI_INPUT_EDIT_H_

#include <string>
#include "QxLibDef.h"

class kiOptionItem;
class kiOptionSet;
class kiInputTool;
class kiInputEdit;

//定义一个kiInputEdit的回调接口，以通知需要输入的对象
class kiInputEditCallBack{
public:
	//当InputEdit输入文本改变时
	virtual int				OnEditInputChanged(const char *pszText) = 0;

	//当InputEdit文本输入完成时，通常是按下了回车或空格键。
	virtual int				OnEditInputFinished(const char *pszText) = 0;

	//当一个选项被选中时，例如：通过快捷字符输入或通过点击
	virtual int				OnEditOptionSelected(int idx,kiOptionItem *pItem) = 0;
};

class QXLIB_API kiInputEdit
{
public:
	kiInputEdit();

	//主要接口，为输入工具显示一条提示信息，可以带选项列表
	//InputTool指针不能为NULL.
	virtual int				RequestInput(const char *pPromptText,
										kiInputEditCallBack *pEditCallBack,
										kiOptionSet *pOptionSet = NULL) = 0;
	//显示一条提示信息.
	virtual void				PromptText(const char *lpszText) = 0;

	//更新所有
	virtual void				Update() = 0;

	//获取输入文本
	virtual std::string		GetInputText() const = 0;		

	//用于保存当前输入请求和恢复输入请求
	virtual int				PushRequest() = 0;
	virtual int				PopRequest() = 0;

protected:
	kiInputEditCallBack		*_pInputEditCallBack;//
	kiOptionSet				*m_pOptionSet;
};

#endif