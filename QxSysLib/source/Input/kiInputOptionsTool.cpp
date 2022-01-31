#include "stdafx.h"
#include "kiCommand.h"
#include "kiInputOptionsTool.h"


kiInputOptionsTool::kiInputOptionsTool(kiCommand *pCommand,const char *pPromptText,kiOptionSet *pOptionSet)
	:kiInputTool(pCommand,pPromptText,pOptionSet)
{
	bAcceptDefaultValue_ = false;
	bOptionChanged_ = false;
}

kiInputOptionsTool::kiInputOptionsTool(kiInputTool *pParent,const char *pPromptText,kiOptionSet *pOptionSet)
	:kiInputTool(pParent,pPromptText,pOptionSet)
{
	bAcceptDefaultValue_ = false;
	bOptionChanged_ = false;
}

kiInputOptionsTool::~kiInputOptionsTool(void)
{
}

void  kiInputOptionsTool::AcceptDefaultValue(bool bAccept)
{
	bAcceptDefaultValue_ = bAccept;
}

// 按下回车或空格键时的响应函数。
int  kiInputOptionsTool::OnEditInputFinished(const char *pInputText)
{
	//如果没有选项改变，就是全部使用默认值
	if(bAcceptDefaultValue_){
		SetInputState(KINPUT_DONE);
	}else{
		//只有修改了选项，才任务输入成功
		if(bOptionChanged_){
			SetInputState(KINPUT_DONE);
		}
	}
	//调用命令相应函数
	if(m_pCommand){
		m_pCommand->OnInputFinished(this);
	}
	//通知父对象
	if(m_pParent){
		m_pParent->OnSubInputFinished(this);
	}

	return kiInputTool::OnEditInputFinished(pInputText);
}

void  kiInputOptionsTool::OnOptionChanged(kiOptionItem *pOptionItem)
{
	bOptionChanged_ = true;

	//通知命令
	if(m_pCommand){
		m_pCommand->OnOptionChanged(this,pOptionItem);
	}
	//
	if(m_pParent){
		m_pParent->OnSubInputFinished(this);
	}

}

//当有选项被选中时，调用该函数
void  kiInputOptionsTool::OnOptionSelected(kiOptionItem *pOptionItem)
{
	if(m_pCommand){
		m_pCommand->OnOptionSelected(pOptionItem);
	}
}

int  kiInputOptionsTool::OnKeyDown(kuiKeyInput& keyInput)
{
	return kiInputTool::OnKeyDown(keyInput);
}

int  kiInputOptionsTool::OnBegin()
{
	bOptionChanged_ = false;

	return kiInputTool::OnBegin();
}