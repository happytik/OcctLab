#pragma once

#include "kiInputTool.h"

class QXLIB_API kiInputOptionsTool : public kiInputTool
{
public:
	kiInputOptionsTool(kiCommand *pCommand,const char *pPromptText,kiOptionSet *pOptionSet = NULL);
	kiInputOptionsTool(kiInputTool *pParent,const char *pPromptText,kiOptionSet *pOptionSet = NULL);
	~kiInputOptionsTool(void);

	//是否接受默认值，即当没有选项修改时，返回默认选项
	void						AcceptDefaultValue(bool bAccept);

	//当edit输入文本完成后，调用该函数,传入的是输入的文本信息
	virtual int				OnEditInputFinished(const char *pInputText);

public://OptionCallBack
	virtual void				OnOptionChanged(kiOptionItem *pOptionItem);
	virtual void				OnOptionSelected(kiOptionItem *pOptionItem);

public:
	virtual int				OnKeyDown(kuiKeyInput& keyInput);

protected:
	virtual int				OnBegin();

protected:
	bool						bAcceptDefaultValue_;//是否接受默认值,就是选项不修改，也认为输入完成

	bool						bOptionChanged_;//是否有选项发生了改变
};

