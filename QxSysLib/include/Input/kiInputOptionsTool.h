#pragma once

#include "kiInputTool.h"

class QXLIB_API kiInputOptionsTool : public kiInputTool
{
public:
	kiInputOptionsTool(kiCommand *pCommand,const char *pPromptText,kiOptionSet *pOptionSet = NULL);
	kiInputOptionsTool(kiInputTool *pParent,const char *pPromptText,kiOptionSet *pOptionSet = NULL);
	~kiInputOptionsTool(void);

	//�Ƿ����Ĭ��ֵ������û��ѡ���޸�ʱ������Ĭ��ѡ��
	void					AcceptDefaultValue(bool bAccept);

	//��edit�����ı���ɺ󣬵��øú���,�������������ı���Ϣ
	virtual int				OnEditInputFinished(const char *pInputText);

public://OptionCallBack
	virtual void			OnOptionChanged(kiOptionItem *pOptionItem);
	virtual void			OnOptionSelected(kiOptionItem *pOptionItem);

public:
	virtual int				OnKeyDown(kuiKeyInput& keyInput);

protected:
	virtual int				OnBegin();

protected:
	bool					bAcceptDefaultValue_;//�Ƿ����Ĭ��ֵ,����ѡ��޸ģ�Ҳ��Ϊ�������

	bool					bOptionChanged_;//�Ƿ���ѡ����˸ı�
};

