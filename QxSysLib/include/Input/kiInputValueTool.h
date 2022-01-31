//////////////////////////////////////////////////////////////////////////
// 输入一个数值的工具，简单的封装。
//
#ifndef _KI_INPUT_VALUE_TOOL_H_
#define _KI_INPUT_VALUE_TOOL_H_

#include "kiInputTool.h"

class kiCommand;
class kiOptionSet;

//输入一个整数
class QXLIB_API kiInputIntTool : public kiInputTool{
public:
	kiInputIntTool(kiCommand *pCommand,const char *pszPrompt,kiOptionSet *pOptionSet = NULL);//带选项
	// 用于InputTool中的构造函数。
	kiInputIntTool(kiInputTool *pParent,const char *pPromptText,kiOptionSet *pOptionSet = NULL);
	virtual ~kiInputIntTool(void);

	bool						Init(int *pVal);
	bool						Init(int *pVal,int nMin,int nMax);

protected:
	//
	virtual int				OnBegin();

public:
	//当edit输入文本完成后，调用该函数,传入的是输入的文本信息
	virtual int				OnEditInputFinished(const char *pInputText);

protected:
	// 获取提示字符串
	virtual void				DoGetPromptText(std::string& str);

protected:
	int						*m_pIntVal;//变量指针
	int						m_nMin,m_nMax;//输入范围
};

//输入一个浮点数
class QXLIB_API kiInputDoubleTool : public kiInputTool{
public:
	kiInputDoubleTool(kiCommand *pCommand,const char *pszPrompt,kiOptionSet *pOptionSet = NULL);//带选项
	// 用于InputTool中的构造函数。
	kiInputDoubleTool(kiInputTool *pParent,const char *pPromptText,kiOptionSet *pOptionSet = NULL);
	virtual ~kiInputDoubleTool(void);

	bool						Init(double *pVal);
	bool						Init(double *pVal,double dMin,double dMax);

protected:
	//
	virtual int				OnBegin();

public:
	//当edit输入文本完成后，调用该函数,传入的是输入的文本信息
	virtual int				OnEditInputFinished(const char *pInputText);

protected:
	// 获取提示字符串
	virtual void				DoGetPromptText(std::string& str);

protected:
	double					*m_pDoubleVal;
	double					m_dMin,m_dMax;
};

#endif